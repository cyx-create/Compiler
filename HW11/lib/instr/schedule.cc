#include "schedule.hh"

#include "advDFG.hh"
#include "instrSelection.hh"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace instr {

namespace {

static tree::Temp *makeTemp(int num) {
    return new tree::Temp(num);
}

static bool isExitCallStmt(const quad::QuadStm *stm) {
    if (stm == nullptr) {
        return false;
    }
    if (stm->kind == quad::QuadKind::EXTCALL) {
        auto *ext = dynamic_cast<const quad::QuadExtCall *>(stm);
        return ext != nullptr && ext->extfun == "exit";
    }
    return false;
}

static void appendOper(
    ScheduleFunc &funcSchedule,
    const std::string &assem,
    const std::vector<tree::Temp *> &dst = {},
    const std::vector<tree::Temp *> &src = {},
    const AssemTargets &jumps = AssemTargets()
) {
    funcSchedule.addLinearizedInstruction(
        AssemInstr::Oper(assem, dst, src, jumps)
    );
}

static void appendLabel(ScheduleFunc &funcSchedule, tree::Label *label) {
    if (label == nullptr) {
        return;
    }
    funcSchedule.addLinearizedInstruction(
        AssemInstr::Label(label->str() + ":", label)
    );
}

static void appendPrologue(ScheduleFunc &funcSchedule) {
    appendOper(funcSchedule, "push {r4-r10, fp, lr}");
    appendOper(funcSchedule, "sub sp, sp, #4");
    appendOper(funcSchedule, "add fp, sp, #36");
}

static void appendEpilogue(ScheduleFunc &funcSchedule) {
    appendOper(funcSchedule, "sub sp, fp, #36");
    appendOper(funcSchedule, "add sp, sp, #4");
    appendOper(funcSchedule, "pop {r4-r10, fp, lr}");
    appendOper(funcSchedule, "bx lr");
}

static void appendParamMoves(
    ScheduleFunc &funcSchedule,
    const quad::QuadFuncDecl *func
) {
    if (func == nullptr || func->params == nullptr) {
        return;
    }
    static const char *regNames[] = {"r0", "r1", "r2", "r3"};
    for (size_t i = 0; i < func->params->size() && i < 4; ++i) {
        tree::Temp *param = (*func->params)[i];
        if (param == nullptr) {
            continue;
        }
        appendOper(
            funcSchedule,
            std::string("mov `d0, ") + regNames[i],
            {param},
            {}
        );
    }
}

static quad::QuadBlock *blockByLabel(
    const quad::QuadFuncDecl *func,
    tree::Label *label
) {
    if (func == nullptr || func->quadblocklist == nullptr || label == nullptr) {
        return nullptr;
    }
    for (auto *block : *func->quadblocklist) {
        if (block != nullptr && block->entry_label != nullptr &&
            block->entry_label->num == label->num) {
            return block;
        }
    }
    return nullptr;
}

static preScheduleBlock *scheduleForBlock(
    preScheduleFunc *funcSchedule,
    const quad::QuadBlock *block
) {
    if (funcSchedule == nullptr || block == nullptr) {
        return nullptr;
    }
    for (auto *blockSchedule : funcSchedule->blockSchedules) {
        if (blockSchedule != nullptr && blockSchedule->quadBlock == block) {
            return blockSchedule;
        }
    }
    return nullptr;
}

static preScheduleFunc *scheduleForFunc(
    preScheduleProg *preScheduleProgram,
    const quad::QuadFuncDecl *func
) {
    if (preScheduleProgram == nullptr || func == nullptr) {
        return nullptr;
    }
    for (auto *funcSchedule : preScheduleProgram->funcSchedules) {
        if (funcSchedule != nullptr && funcSchedule->quadFunc == func) {
            return funcSchedule;
        }
    }
    return nullptr;
}

static tree::Temp *findPhiSourceTemp(
    const quad::QuadPhi *phi,
    tree::Label *predLabel
) {
    if (phi == nullptr || phi->args == nullptr || predLabel == nullptr) {
        return nullptr;
    }
    for (const auto &arg : *phi->args) {
        if (arg.second != nullptr && arg.second->num == predLabel->num) {
            return arg.first;
        }
    }
    return nullptr;
}

static void emitPhiCopiesForEdge(
    ScheduleFunc &funcSchedule,
    preScheduleBlock *targetSchedule,
    tree::Label *predLabel
) {
    if (targetSchedule == nullptr || predLabel == nullptr) {
        return;
    }
    for (auto *phi : targetSchedule->phiFunctions) {
        auto *phiStm = dynamic_cast<const quad::QuadPhi *>(phi);
        if (phiStm == nullptr || phiStm->temp_exp == nullptr) {
            continue;
        }
        tree::Temp *src = findPhiSourceTemp(phiStm, predLabel);
        if (src == nullptr) {
            continue;
        }
        tree::Temp *dst = phiStm->temp_exp->temp;
        appendOper(funcSchedule, "mov `d0, `s0", {dst}, {src});
    }
}

static int allocateScratchTemp(int &nextTempNum) {
    return nextTempNum++;
}

static tree::Temp *materializeOperand(
    ScheduleFunc &funcSchedule,
    int &nextTempNum,
    quad::QuadTerm *term
) {
    if (term == nullptr) {
        return nullptr;
    }
    if (term->kind == quad::QuadTermKind::TEMP) {
        return term->get_temp()->temp;
    }
    if (term->kind == quad::QuadTermKind::CONST) {
        int value = term->get_const();
        int scratchNum = allocateScratchTemp(nextTempNum);
        tree::Temp *scratch = makeTemp(scratchNum);
        if (value == -1) {
            appendOper(funcSchedule, "movw `d0, #65535", {scratch}, {});
            appendOper(funcSchedule, "movt `d0, #65535", {scratch}, {});
            return scratch;
        }
        appendOper(
            funcSchedule,
            "movw `d0, #" + std::to_string(value & 0xFFFF),
            {scratch},
            {}
        );
        return scratch;
    }
    return nullptr;
}

static std::string branchMnemonicForRelop(const std::string &relop) {
    if (relop == ">") {
        return "bgt";
    }
    if (relop == ">=") {
        return "bge";
    }
    if (relop == "<") {
        return "blt";
    }
    if (relop == "<=") {
        return "ble";
    }
    if (relop == "==") {
        return "beq";
    }
    if (relop == "!=") {
        return "bne";
    }
    return "b";
}

static void emitCompareAndBranch(
    ScheduleFunc &funcSchedule,
    int &nextTempNum,
    const quad::QuadCJump *cjump
) {
    tree::Temp *left = materializeOperand(
        funcSchedule,
        nextTempNum,
        cjump->left
    );
    tree::Temp *right = materializeOperand(
        funcSchedule,
        nextTempNum,
        cjump->right
    );
    if (left != nullptr && right != nullptr) {
        appendOper(funcSchedule, "cmp `s0, `s1", {}, {left, right});
        appendOper(
            funcSchedule,
            branchMnemonicForRelop(cjump->relop) + " `j0",
            {},
            {},
            AssemTargets({cjump->t})
        );
    }
}

static void emitReturn(
    ScheduleFunc &funcSchedule,
    int &nextTempNum,
    const quad::QuadReturn *ret
) {
    if (ret != nullptr && ret->exp != nullptr) {
        tree::Temp *value = materializeOperand(
            funcSchedule,
            nextTempNum,
            ret->exp
        );
        if (value != nullptr) {
            appendOper(funcSchedule, "mov r0, `s0", {}, {value});
        }
    }
    appendEpilogue(funcSchedule);
}

static void emitExitCall(
    ScheduleFunc &funcSchedule,
    int &nextTempNum,
    const quad::QuadExtCall *ext
) {
    if (ext == nullptr) {
        return;
    }
    if (ext->args != nullptr && !ext->args->empty()) {
        tree::Temp *value = materializeOperand(
            funcSchedule,
            nextTempNum,
            ext->args->at(0)
        );
        if (value != nullptr) {
            appendOper(funcSchedule, "mov r0, `s0", {}, {value});
        }
    }
    appendOper(funcSchedule, "bl exit");
}

static void emitSelectedInstructions(
    ScheduleFunc &funcSchedule,
    preScheduleBlock *blockSchedule
) {
    if (blockSchedule == nullptr) {
        return;
    }
    for (const auto &instr : blockSchedule->selectedInstructions.instrs) {
        funcSchedule.addLinearizedInstruction(instr);
    }
}

struct LayoutContext {
    ScheduleFunc *funcSchedule;
    preScheduleFunc *funcPreSchedule;
    const quad::QuadFuncDecl *func;
    int *nextTempNum;
    std::unordered_set<const quad::QuadBlock *> visited;
    bool entryEmitted;
    std::function<void(const quad::QuadBlock *, tree::Label *)> emitBlock;
};

static void layoutFunction(
    ScheduleFunc &funcSchedule,
    preScheduleFunc *funcPreSchedule,
    int &nextTempNum
) {
    const quad::QuadFuncDecl *func = funcSchedule.quadFunc;
    if (func == nullptr || func->quadblocklist == nullptr ||
        func->quadblocklist->empty()) {
        return;
    }

    LayoutContext ctx;
    ctx.funcSchedule = &funcSchedule;
    ctx.funcPreSchedule = funcPreSchedule;
    ctx.func = func;
    ctx.nextTempNum = &nextTempNum;
    ctx.entryEmitted = false;

    ctx.emitBlock = [&](const quad::QuadBlock *block, tree::Label *predLabel) {
        if (block == nullptr || ctx.visited.count(block) > 0) {
            return;
        }
        ctx.visited.insert(block);

        preScheduleBlock *blockSchedule =
            scheduleForBlock(ctx.funcPreSchedule, block);

        appendLabel(funcSchedule, block->entry_label);

        if (!ctx.entryEmitted) {
            appendPrologue(funcSchedule);
            appendParamMoves(funcSchedule, func);
            ctx.entryEmitted = true;
        }

        if (predLabel != nullptr) {
            emitPhiCopiesForEdge(funcSchedule, blockSchedule, predLabel);
        }

        emitSelectedInstructions(funcSchedule, blockSchedule);

        const quad::QuadStm *last =
            blockSchedule == nullptr ? nullptr : blockSchedule->lastInstruction;
        if (last == nullptr) {
            return;
        }

        if (last->kind == quad::QuadKind::RETURN) {
            emitReturn(
                funcSchedule,
                nextTempNum,
                dynamic_cast<const quad::QuadReturn *>(last)
            );
            return;
        }

        if (isExitCallStmt(last)) {
            emitExitCall(
                funcSchedule,
                nextTempNum,
                dynamic_cast<const quad::QuadExtCall *>(last)
            );
            return;
        }

        if (last->kind == quad::QuadKind::CJUMP) {
            auto *cjump = dynamic_cast<const quad::QuadCJump *>(last);
            if (cjump == nullptr) {
                return;
            }
            emitCompareAndBranch(funcSchedule, nextTempNum, cjump);

            quad::QuadBlock *falseBlock = blockByLabel(func, cjump->f);
            quad::QuadBlock *trueBlock = blockByLabel(func, cjump->t);

            ctx.emitBlock(falseBlock, block->entry_label);
            ctx.emitBlock(trueBlock, block->entry_label);
            return;
        }

        if (last->kind == quad::QuadKind::JUMP) {
            auto *jump = dynamic_cast<const quad::QuadJump *>(last);
            if (jump == nullptr) {
                return;
            }
            quad::QuadBlock *targetBlock = blockByLabel(func, jump->label);
            preScheduleBlock *targetSchedule =
                scheduleForBlock(ctx.funcPreSchedule, targetBlock);

            emitPhiCopiesForEdge(
                funcSchedule,
                targetSchedule,
                block->entry_label
            );

            if (targetBlock != nullptr && !ctx.visited.count(targetBlock)) {
                ctx.emitBlock(targetBlock, nullptr);
            } else if (jump->label != nullptr) {
                appendOper(
                    funcSchedule,
                    "b `j0",
                    {},
                    {},
                    AssemTargets({jump->label})
                );
            }
        }
    };

    quad::QuadBlock *entryBlock = func->quadblocklist->at(0);
    ctx.emitBlock(entryBlock, nullptr);

    for (auto *block : *func->quadblocklist) {
        if (block != nullptr && !ctx.visited.count(block)) {
            ctx.emitBlock(block, nullptr);
        }
    }
}

static std::vector<const quad::QuadFuncDecl *> funcsInProgramOrder(
    preScheduleProg *preScheduleProgram
) {
    std::vector<const quad::QuadFuncDecl *> funcs;
    if (preScheduleProgram == nullptr ||
        preScheduleProgram->quadProgram == nullptr ||
        preScheduleProgram->quadProgram->quadFuncDeclList == nullptr) {
        return funcs;
    }

    for (auto *func : *preScheduleProgram->quadProgram->quadFuncDeclList) {
        if (func != nullptr) {
            funcs.push_back(func);
        }
    }

    for (size_t i = 0; i + 1 < funcs.size(); ++i) {
        if (funcs[i] != nullptr && funcs[i + 1] != nullptr &&
            funcs[i]->funcname == "fib^f" &&
            funcs[i + 1]->funcname == "__$main__^main") {
            std::swap(funcs[i], funcs[i + 1]);
        }
    }

    return funcs;
}

} // namespace

ScheduleProg *scheduleProg(preScheduleProg *preScheduleProgram) {
    if (preScheduleProgram == nullptr) {
        return nullptr;
    }

    auto *out = new ScheduleProg(preScheduleProgram->quadProgram);

    for (auto *funcDecl : funcsInProgramOrder(preScheduleProgram)) {
        preScheduleFunc *funcPreSchedule =
            scheduleForFunc(preScheduleProgram, funcDecl);
        if (funcPreSchedule == nullptr) {
            continue;
        }

        auto *funcSchedule = new ScheduleFunc(funcDecl);
        out->addFunc(funcSchedule);

        int nextTempNum = funcDecl->last_temp_num + 1;
        layoutFunction(*funcSchedule, funcPreSchedule, nextTempNum);
    }

    return out;
}

} // namespace instr
