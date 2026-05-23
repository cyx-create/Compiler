#include "loopinductionbasic.hh"
#include "defusechain.hh"
#include "loopinductionopt.hh"
#include <map>

using namespace std;
using namespace quad;

static int getTempNumFromTerm(QuadTerm* term) {
    if (term == nullptr) return -1;
    if (term->kind == QuadTermKind::TEMP) {
        QuadTemp* qt = term->get_temp();
        if (qt != nullptr && qt->temp != nullptr) {
            return qt->temp->num;
        }
    }
    return -1;
}

static map<QuadStm*, int> computeGlobalOrder(QuadFuncDecl* func) {
    map<QuadStm*, int> order;
    int idx = 0;
    if (func == nullptr || func->quadblocklist == nullptr) {
        return order;
    }
    for (QuadBlock* block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) {
            continue;
        }
        for (QuadStm* stm : *block->quadlist) {
            if (stm->kind == QuadKind::LABEL) {
                continue;
            }
            order[stm] = ++idx;
        }
    }
    return order;
}

static bool isLoopInvariant(int tempNum, const set<int>& bodyBlocks, const DefUseChain& du) {
    VarDefInfo* defInfo = du.getDef(tempNum);
    if (defInfo == nullptr || defInfo->defBlock == nullptr) {
        return true;
    }
    int defLabel = defInfo->defBlock->entry_label != nullptr
        ? defInfo->defBlock->entry_label->num : -1;
    return bodyBlocks.count(defLabel) == 0;
}

static QuadStm* findBackedgeUpdate(
    int backedgeTemp,
    int phiTemp,
    QuadBlock* backedgeBlock,
    int& step,
    int& stepTempNum
) {
    if (backedgeBlock == nullptr || backedgeBlock->quadlist == nullptr) {
        return nullptr;
    }
    for (QuadStm* stm : *backedgeBlock->quadlist) {
        if (stm->kind != QuadKind::MOVE_BINOP) {
            continue;
        }
        QuadMoveBinop* binop = dynamic_cast<QuadMoveBinop*>(stm);
        if (binop == nullptr || binop->dst == nullptr || binop->dst->temp == nullptr) {
            continue;
        }
        if (binop->dst->temp->num != backedgeTemp) {
            continue;
        }
        int leftTemp = getTempNumFromTerm(binop->left);
        int rightTemp = getTempNumFromTerm(binop->right);
        int leftConst = binop->left != nullptr && binop->left->kind == QuadTermKind::CONST
            ? binop->left->get_const() : -1;
        int rightConst = binop->right != nullptr && binop->right->kind == QuadTermKind::CONST
            ? binop->right->get_const() : -1;

        if (leftTemp == phiTemp && binop->binop == "+") {
            if (rightConst != -1) {
                step = rightConst;
                stepTempNum = -1;
                return stm;
            }
            if (rightTemp != -1) {
                step = 0;
                stepTempNum = rightTemp;
                return stm;
            }
        }
        if (leftTemp == phiTemp && binop->binop == "-") {
            if (rightConst != -1) {
                step = -rightConst;
                stepTempNum = -1;
                return stm;
            }
            if (rightTemp != -1) {
                step = 0;
                stepTempNum = -rightTemp;
                return stm;
            }
        }
        if (rightTemp == phiTemp && binop->binop == "+") {
            if (leftConst != -1) {
                step = leftConst;
                stepTempNum = -1;
                return stm;
            }
            if (leftTemp != -1) {
                step = 0;
                stepTempNum = leftTemp;
                return stm;
            }
        }
    }
    return nullptr;
}

map<int, vector<BasicInductionVar>> discoverBasicInductionVars(
    QuadFuncDecl* func,
    LoopHeaderMap* loopHeaderMap,
    const DefUseChain& du,
    const ControlFlowInfo& cfi
) {
    map<int, vector<BasicInductionVar>> result;
    if (func == nullptr || func->quadblocklist == nullptr || loopHeaderMap == nullptr) {
        return result;
    }
    if (!loopHeaderMap->funcLoopHeaders.count(func)) {
        return result;
    }

    auto globalOrder = computeGlobalOrder(func);
    auto labelToBlock = buildLabelToBlock(func);

    for (LoopHeader* loopHeader : loopHeaderMap->funcLoopHeaders[func]) {
        if (loopHeader == nullptr) {
            continue;
        }
        int headerLabel = loopHeader->headerLabel;
        const set<int>& bodyBlocks = loopHeader->bodyBlocks;

        auto blockIt = labelToBlock.find(headerLabel);
        if (blockIt == labelToBlock.end() || blockIt->second == nullptr) {
            continue;
        }
        QuadBlock* headerBlock = blockIt->second;
        if (headerBlock->quadlist == nullptr) {
            continue;
        }

        for (QuadStm* stm : *headerBlock->quadlist) {
            if (stm->kind != QuadKind::PHI) {
                continue;
            }
            QuadPhi* phi = dynamic_cast<QuadPhi*>(stm);
            if (phi == nullptr || phi->temp_exp == nullptr || phi->temp_exp->temp == nullptr
                || phi->args == nullptr || phi->args->size() != 2) {
                continue;
            }

            int phiTemp = phi->temp_exp->temp->num;
            int initTemp = -1;
            int initLabel = -1;
            int backedgeTemp = -1;
            int backedgeLabel = -1;

            for (const auto& arg : *phi->args) {
                if (arg.first == nullptr || arg.second == nullptr) {
                    continue;
                }
                int argLabel = arg.second->num;
                if (bodyBlocks.count(argLabel)) {
                    backedgeTemp = arg.first->num;
                    backedgeLabel = argLabel;
                } else {
                    initTemp = arg.first->num;
                    initLabel = argLabel;
                }
            }

            if (initTemp == -1 || backedgeTemp == -1 || backedgeLabel == -1) {
                continue;
            }

            auto backBlockIt = labelToBlock.find(backedgeLabel);
            if (backBlockIt == labelToBlock.end()) {
                continue;
            }

            int step = 0;
            int stepTempNum = -1;
            QuadStm* updateStm = findBackedgeUpdate(
                backedgeTemp, phiTemp, backBlockIt->second, step, stepTempNum);
            if (updateStm == nullptr) {
                continue;
            }

            if (stepTempNum != -1 && !isLoopInvariant(abs(stepTempNum), bodyBlocks, du)) {
                continue;
            }

            BasicInductionVar biv(
                headerLabel, phiTemp, initTemp, backedgeTemp, step, phi, updateStm);
            biv.stepTempNum = stepTempNum;
            auto orderIt = globalOrder.find(updateStm);
            biv.updateOrder = orderIt != globalOrder.end() ? orderIt->second : -1;

            if (stepTempNum != -1) {
                biv.addRelatedTemp(abs(stepTempNum));
            }

            result[headerLabel].push_back(biv);
        }
    }

    return result;
}

static bool isBackedgeFamilyStatement(QuadStm* useStm, const BasicInductionVar& iv) {
    if (useStm == iv.phiStm || useStm == iv.updateStm) {
        return true;
    }
    return false;
}

void classifyRelatedTemps(
    QuadFuncDecl* func,
    map<int, vector<BasicInductionVar>>& basicByHeader,
    const DefUseChain& du
) {
    if (func == nullptr || func->quadblocklist == nullptr) {
        return;
    }

    for (auto& entry : basicByHeader) {
        for (BasicInductionVar& iv : entry.second) {
            for (int tempNum : iv.relatedTemps) {
                if (tempNum == iv.phiTempNum) {
                    continue;
                }

                bool onlyBackedgeUses = true;
                VarDefInfo* defInfo = du.getDef(tempNum);
                if (defInfo != nullptr) {
                    for (const auto& useLoc : defInfo->useSet) {
                        QuadStm* useStm = useLoc.second;
                        if (!isBackedgeFamilyStatement(useStm, iv)) {
                            onlyBackedgeUses = false;
                            break;
                        }
                    }
                } else {
                    for (QuadBlock* block : *func->quadblocklist) {
                        if (block == nullptr || block->quadlist == nullptr) {
                            continue;
                        }
                        for (QuadStm* stm : *block->quadlist) {
                            if (stm == iv.phiStm || stm == iv.updateStm) {
                                continue;
                            }
                            set<int> uses = du.getUsesBy(stm);
                            if (uses.count(tempNum)) {
                                onlyBackedgeUses = false;
                                break;
                            }
                        }
                        if (!onlyBackedgeUses) {
                            break;
                        }
                    }
                }

                if (onlyBackedgeUses) {
                    iv.markUseless(tempNum);
                } else {
                    iv.markUseful(tempNum);
                }
            }
        }
    }
}
