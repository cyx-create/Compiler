#include "loopstrengthreduction.hh"
#include "loopinductionopt.hh"
#include "defusechain.hh"
#include <cstdlib>
#include <map>
#include <set>
#include <algorithm>

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

static bool sourceAfterBasicUpdate(
    int sourceTempNum,
    const DefUseChain& du,
    int basicBackedgeTemp
) {
    VarDefInfo* sourceDef = du.getDef(sourceTempNum);
    if (sourceDef == nullptr || sourceDef->defStm == nullptr
        || sourceDef->defStm->kind != QuadKind::MOVE_BINOP) {
        return false;
    }
    QuadMoveBinop* mul = dynamic_cast<QuadMoveBinop*>(sourceDef->defStm);
    if (mul == nullptr) {
        return false;
    }
    int leftTemp = getTempNumFromTerm(mul->left);
    int rightTemp = getTempNumFromTerm(mul->right);
    return leftTemp == basicBackedgeTemp || rightTemp == basicBackedgeTemp;
}

static const BasicInductionVar* findBasicIV(
    const map<int, vector<BasicInductionVar>>& basicIVsByHeader,
    int headerLabel,
    int basicTempNum
) {
    auto it = basicIVsByHeader.find(headerLabel);
    if (it == basicIVsByHeader.end()) {
        return nullptr;
    }
    for (const BasicInductionVar& biv : it->second) {
        if (biv.phiTempNum == basicTempNum) {
            return &biv;
        }
    }
    return nullptr;
}

static int findPreheaderLabel(
    LoopHeaderMap* loopHeaderMap,
    QuadFuncDecl* func,
    int headerLabel
) {
    if (loopHeaderMap == nullptr || !loopHeaderMap->funcLoopHeaders.count(func)) {
        return -1;
    }
    set<int> bodyBlocks;
    for (LoopHeader* lh : loopHeaderMap->funcLoopHeaders[func]) {
        if (lh != nullptr && lh->headerLabel == headerLabel) {
            bodyBlocks = lh->bodyBlocks;
            break;
        }
    }
    auto labelToBlock = buildLabelToBlock(func);
    auto blockIt = labelToBlock.find(headerLabel);
    if (blockIt == labelToBlock.end() || blockIt->second == nullptr) {
        return -1;
    }
    QuadBlock* headerBlock = blockIt->second;
    if (headerBlock->quadlist == nullptr) {
        return -1;
    }
    for (QuadStm* stm : *headerBlock->quadlist) {
        if (stm->kind != QuadKind::PHI) {
            continue;
        }
        QuadPhi* phi = dynamic_cast<QuadPhi*>(stm);
        if (phi == nullptr || phi->args == nullptr) {
            continue;
        }
        for (const auto& arg : *phi->args) {
            if (arg.second == nullptr) {
                continue;
            }
            if (!bodyBlocks.count(arg.second->num)) {
                return arg.second->num;
            }
        }
    }
    return -1;
}

static int findBackedgeLabel(
    const BasicInductionVar& biv,
    LoopHeaderMap* loopHeaderMap,
    QuadFuncDecl* func
) {
    auto labelToBlock = buildLabelToBlock(func);
    auto blockIt = labelToBlock.find(biv.headerLabel);
    if (blockIt == labelToBlock.end()) {
        return -1;
    }
    QuadBlock* headerBlock = blockIt->second;
    if (headerBlock == nullptr || headerBlock->quadlist == nullptr) {
        return -1;
    }
    set<int> bodyBlocks;
    if (loopHeaderMap != nullptr && loopHeaderMap->funcLoopHeaders.count(func)) {
        for (LoopHeader* lh : loopHeaderMap->funcLoopHeaders[func]) {
            if (lh != nullptr && lh->headerLabel == biv.headerLabel) {
                bodyBlocks = lh->bodyBlocks;
                break;
            }
        }
    }
    for (QuadStm* stm : *headerBlock->quadlist) {
        if (stm->kind != QuadKind::PHI) {
            continue;
        }
        QuadPhi* phi = dynamic_cast<QuadPhi*>(stm);
        if (phi == nullptr || phi->temp_exp == nullptr || phi->temp_exp->temp == nullptr) {
            continue;
        }
        if (phi->temp_exp->temp->num != biv.phiTempNum || phi->args == nullptr) {
            continue;
        }
        for (const auto& arg : *phi->args) {
            if (arg.second != nullptr && bodyBlocks.count(arg.second->num)) {
                return arg.second->num;
            }
        }
    }
    return -1;
}

static set<QuadStm*> collectDerivedDefChain(QuadStm* defStm, const DefUseChain& du) {
    set<QuadStm*> chain;
    if (defStm == nullptr) {
        return chain;
    }
    chain.insert(defStm);
    set<int> uses = du.getUsesBy(defStm);
    for (int usedTemp : uses) {
        VarDefInfo* usedDef = du.getDef(usedTemp);
        if (usedDef != nullptr && usedDef->defStm != nullptr
            && usedDef->defStm->kind == QuadKind::MOVE_BINOP) {
            chain.insert(usedDef->defStm);
        }
    }
    return chain;
}

static set<int> collectUsedTemps(QuadFuncDecl* func) {
    set<int> used;
    if (func == nullptr || func->quadblocklist == nullptr) {
        return used;
    }
    for (QuadBlock* block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) {
            continue;
        }
        for (QuadStm* stm : *block->quadlist) {
            if (stm->def != nullptr) {
                for (Temp* t : *stm->def) {
                    if (t != nullptr) {
                        used.insert(t->num);
                    }
                }
            }
            if (stm->use != nullptr) {
                for (Temp* t : *stm->use) {
                    if (t != nullptr) {
                        used.insert(t->num);
                    }
                }
            }
        }
    }
    return used;
}

static int allocateTemp(set<int>& usedTemps, int startFrom) {
    int t = startFrom;
    while (usedTemps.count(t)) {
        ++t;
    }
    usedTemps.insert(t);
    return t;
}

StrengthReductionPlan generateStrengthReductionPlan(
    QuadFuncDecl* func,
    const map<int, vector<DerivedInductionVar>>& derivedIVsByHeader,
    const map<int, vector<BasicInductionVar>>& basicIVsByHeader,
    LoopHeaderMap* loopHeaderMap
) {
    StrengthReductionPlan plan;

    if (func == nullptr || loopHeaderMap == nullptr || derivedIVsByHeader.empty()) {
        return plan;
    }

    DefUseChain du(func);
    set<int> usedTemps = collectUsedTemps(func);

    for (const auto& entry : derivedIVsByHeader) {
        int headerLabel = entry.first;
        for (const DerivedInductionVar& div : entry.second) {
            const BasicInductionVar* biv = findBasicIV(
                basicIVsByHeader, headerLabel, div.expr.basicTempNum);
            if (biv == nullptr) {
                continue;
            }

            StrengthReductionPlan::ReplacementIV repl;
            repl.map.headerLabel = headerLabel;
            repl.map.oldTempNum = div.tempNum;
            repl.map.basicTempNum = biv->phiTempNum;

            int nextStart = max(div.tempNum + 1, func->last_temp_num + 1);
            repl.map.newPhiTemp = allocateTemp(usedTemps, nextStart);
            repl.initTemps.newInitTemp = allocateTemp(usedTemps, repl.map.newPhiTemp + 1);
            repl.map.newBackedgeTemp = allocateTemp(usedTemps, repl.initTemps.newInitTemp + 1);
            repl.sourceOrder = div.sourceOrder;

            repl.initExpr.basicCoeff = div.expr.basicCoeff;
            repl.initExpr.constant = div.expr.constant;
            repl.initExpr.initTempNum = biv->initTempNum;
            repl.initExpr.sourceAfterBasicUpdate = sourceAfterBasicUpdate(
                div.sourceTempNum, du, biv->backedgeTempNum);
            repl.initExpr.basicStepValue = biv->step;
            repl.initExpr.basicStepTempNum = biv->stepTempNum;

            if (repl.initExpr.sourceAfterBasicUpdate) {
                repl.initTemps.newInitAdjustedSourceTemp =
                    allocateTemp(usedTemps, repl.map.newBackedgeTemp + 1);
            }
            int nextAux = repl.initTemps.newInitAdjustedSourceTemp != -1
                ? repl.initTemps.newInitAdjustedSourceTemp + 1
                : repl.map.newBackedgeTemp + 1;
            if (abs(div.expr.basicCoeff) != 1) {
                repl.initTemps.newInitIntermediateTemp =
                    allocateTemp(usedTemps, nextAux);
                nextAux = repl.initTemps.newInitIntermediateTemp + 1;
            }

            if (biv->stepTempNum != -1) {
                repl.stepExpr.stepSourceTempNum = abs(biv->stepTempNum);
                repl.stepExpr.stepTempScaleFactor = abs(div.expr.basicCoeff);
                repl.stepExpr.newStepTemp = allocateTemp(usedTemps, nextAux);
                repl.stepExpr.stepIncrementTempNum = repl.stepExpr.newStepTemp;
                repl.stepExpr.stepIncrementNegative = true;
            } else {
                repl.stepExpr.stepIncrementValue = div.expr.basicCoeff * biv->step;
            }

            repl.placement.initLabel = findPreheaderLabel(loopHeaderMap, func, headerLabel);
            repl.placement.backedgeLabel = findBackedgeLabel(*biv, loopHeaderMap, func);

            plan.replacements.push_back(repl);
            plan.tempReplacement[div.tempNum] = repl.map.newPhiTemp;

            set<QuadStm*> chain = collectDerivedDefChain(div.defStm, du);
            for (QuadStm* s : chain) {
                plan.stmtsToRemove.insert(s);
            }
        }
    }

    return plan;
}

static set<Temp*>* makeDefSet(int tempNum) {
    auto* s = new set<Temp*>();
    s->insert(new Temp(tempNum));
    return s;
}

static set<Temp*>* makeUseSet(initializer_list<int> temps) {
    auto* s = new set<Temp*>();
    for (int t : temps) {
        s->insert(new Temp(t));
    }
    return s;
}

static QuadTemp* makeQuadTemp(int tempNum) {
    return new QuadTemp(new Temp(tempNum), QuadType::INT);
}

static QuadTerm* makeTempTerm(int tempNum) {
    return new QuadTerm(makeQuadTemp(tempNum));
}

static QuadTerm* makeConstTerm(int value) {
    return new QuadTerm(value);
}

static QuadMoveBinop* makeBinop(int dst, int leftTemp, const string& op, int rightTemp) {
    return new QuadMoveBinop(
        makeQuadTemp(dst),
        makeTempTerm(leftTemp),
        op,
        makeTempTerm(rightTemp),
        makeDefSet(dst),
        makeUseSet({leftTemp, rightTemp}));
}

static QuadMoveBinop* makeBinopConst(int dst, int leftTemp, const string& op, int constant) {
    return new QuadMoveBinop(
        makeQuadTemp(dst),
        makeTempTerm(leftTemp),
        op,
        makeConstTerm(constant),
        makeDefSet(dst),
        makeUseSet({leftTemp}));
}

static QuadMoveBinop* makeConstBinop(int dst, int constant, const string& op, int rightTemp) {
    return new QuadMoveBinop(
        makeQuadTemp(dst),
        makeConstTerm(constant),
        op,
        makeTempTerm(rightTemp),
        makeDefSet(dst),
        makeUseSet({rightTemp}));
}

static void replaceTempInTerm(QuadTerm* term, int oldTemp, int newTemp) {
    if (term == nullptr) {
        return;
    }
    if (term->kind == QuadTermKind::TEMP) {
        QuadTemp* qt = term->get_temp();
        if (qt != nullptr && qt->temp != nullptr && qt->temp->num == oldTemp) {
            qt->temp = new Temp(newTemp);
        }
    }
}

static void replaceTempInStm(QuadStm* stm, int oldTemp, int newTemp) {
    if (stm == nullptr) {
        return;
    }
    auto updateSet = [&](set<Temp*>* temps) {
        if (temps == nullptr) {
            return;
        }
        set<Temp*> updated;
        for (Temp* t : *temps) {
            if (t != nullptr && t->num == oldTemp) {
                updated.insert(new Temp(newTemp));
            } else {
                updated.insert(t);
            }
        }
        *temps = updated;
    };
    updateSet(stm->def);
    updateSet(stm->use);

    switch (stm->kind) {
        case QuadKind::MOVE: {
            QuadMove* m = dynamic_cast<QuadMove*>(stm);
            if (m != nullptr) {
                replaceTempInTerm(m->src, oldTemp, newTemp);
            }
            break;
        }
        case QuadKind::MOVE_BINOP: {
            QuadMoveBinop* b = dynamic_cast<QuadMoveBinop*>(stm);
            if (b != nullptr) {
                replaceTempInTerm(b->left, oldTemp, newTemp);
                replaceTempInTerm(b->right, oldTemp, newTemp);
            }
            break;
        }
        case QuadKind::CJUMP: {
            QuadCJump* cj = dynamic_cast<QuadCJump*>(stm);
            if (cj != nullptr) {
                replaceTempInTerm(cj->left, oldTemp, newTemp);
                replaceTempInTerm(cj->right, oldTemp, newTemp);
            }
            break;
        }
        case QuadKind::PHI: {
            QuadPhi* phi = dynamic_cast<QuadPhi*>(stm);
            if (phi != nullptr && phi->args != nullptr) {
                for (auto& arg : *phi->args) {
                    if (arg.first != nullptr && arg.first->num == oldTemp) {
                        arg.first = new Temp(newTemp);
                    }
                }
            }
            break;
        }
        case QuadKind::EXTCALL: {
            QuadExtCall* ec = dynamic_cast<QuadExtCall*>(stm);
            if (ec != nullptr && ec->args != nullptr) {
                for (QuadTerm* arg : *ec->args) {
                    replaceTempInTerm(arg, oldTemp, newTemp);
                }
            }
            break;
        }
        case QuadKind::RETURN: {
            QuadReturn* ret = dynamic_cast<QuadReturn*>(stm);
            if (ret != nullptr) {
                replaceTempInTerm(ret->exp, oldTemp, newTemp);
            }
            break;
        }
        default:
            break;
    }
}

static void replaceTempInFunc(QuadFuncDecl* func, int oldTemp, int newTemp) {
    if (func == nullptr || func->quadblocklist == nullptr) {
        return;
    }
    for (QuadBlock* block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) {
            continue;
        }
        for (QuadStm* stm : *block->quadlist) {
            replaceTempInStm(stm, oldTemp, newTemp);
        }
    }
}

static vector<QuadStm*> buildInitStmts(const StrengthReductionPlan::ReplacementIV& repl) {
    vector<QuadStm*> stmts;
    int coeff = repl.initExpr.basicCoeff;
    int constant = repl.initExpr.constant;
    int curTemp = repl.initExpr.initTempNum;
    int finalInit = repl.initTemps.newInitTemp;

    if (repl.initExpr.sourceAfterBasicUpdate) {
        int adjusted = repl.initTemps.newInitAdjustedSourceTemp;
        if (repl.initExpr.basicStepTempNum != -1) {
            stmts.push_back(makeBinop(adjusted, curTemp, "-", abs(repl.initExpr.basicStepTempNum)));
        } else if (repl.initExpr.basicStepValue < 0) {
            stmts.push_back(makeBinopConst(adjusted, curTemp, "-", -repl.initExpr.basicStepValue));
        } else {
            stmts.push_back(makeBinopConst(adjusted, curTemp, "+", repl.initExpr.basicStepValue));
        }
        curTemp = adjusted;
    }

    int absCoeff = abs(coeff);
    if (absCoeff != 1) {
        int mulTemp = repl.initTemps.newInitIntermediateTemp;
        stmts.push_back(makeBinopConst(mulTemp, curTemp, "*", absCoeff));
        curTemp = mulTemp;
    } else if (coeff == -1) {
        int negTemp = repl.initTemps.newInitIntermediateTemp;
        stmts.push_back(makeBinopConst(negTemp, 0, "-", curTemp));
        curTemp = negTemp;
    }

    if (constant >= 0) {
        stmts.push_back(makeBinopConst(finalInit, curTemp, "+", constant));
    } else {
        stmts.push_back(makeBinopConst(finalInit, curTemp, "-", -constant));
    }
    return stmts;
}

static QuadPhi* makePhi(
    int phiTemp,
    int backedgeTemp,
    int backedgeLabel,
    int initTemp,
    int initLabel
) {
    auto* args = new vector<pair<Temp*, Label*>>();
    args->push_back({new Temp(backedgeTemp), new Label(backedgeLabel)});
    args->push_back({new Temp(initTemp), new Label(initLabel)});
    return new QuadPhi(
        makeQuadTemp(phiTemp),
        args,
        makeDefSet(phiTemp),
        makeUseSet({backedgeTemp, initTemp}));
}

static int computeCJumpThreshold(
    const StrengthReductionPlan::ReplacementIV& repl,
    int cjumpConst
) {
    int a = repl.initExpr.basicCoeff;
    int b = repl.initExpr.constant;
    if (repl.initExpr.sourceAfterBasicUpdate) {
        if (repl.initExpr.basicStepTempNum != -1) {
            return b;
        }
        return b + a * repl.initExpr.basicStepValue;
    }
    return a * cjumpConst + b;
}

QuadFuncDecl* applyStrengthReduction(
    QuadFuncDecl* func,
    const StrengthReductionPlan& plan
) {
    if (func == nullptr || func->quadblocklist == nullptr) {
        return func;
    }

    if (plan.tempReplacement.empty()) {
        return func;
    }

    auto labelToBlock = buildLabelToBlock(func);
    int nextTemp = func->last_temp_num;

    for (const auto& repl : plan.replacements) {
        nextTemp = max(nextTemp, repl.map.newBackedgeTemp);
        if (repl.initTemps.newInitAdjustedSourceTemp > nextTemp) {
            nextTemp = repl.initTemps.newInitAdjustedSourceTemp;
        }
        if (repl.initTemps.newInitIntermediateTemp > nextTemp) {
            nextTemp = repl.initTemps.newInitIntermediateTemp;
        }
        if (repl.stepExpr.newStepTemp > nextTemp) {
            nextTemp = repl.stepExpr.newStepTemp;
        }
    }

    for (const auto& repl : plan.replacements) {
        auto preIt = labelToBlock.find(repl.placement.initLabel);
        if (preIt != labelToBlock.end() && preIt->second != nullptr) {
            QuadBlock* preheader = preIt->second;
            vector<QuadStm*> initStmts = buildInitStmts(repl);

            if (repl.stepExpr.stepIncrementTempNum != -1 && repl.stepExpr.newStepTemp != -1) {
                initStmts.push_back(makeBinopConst(
                    repl.stepExpr.newStepTemp,
                    repl.stepExpr.stepSourceTempNum,
                    "*",
                    repl.stepExpr.stepTempScaleFactor));
            }

            if (preheader->quadlist != nullptr) {
                auto insertPos = preheader->quadlist->end();
                for (auto it = preheader->quadlist->begin(); it != preheader->quadlist->end(); ++it) {
                    if ((*it)->kind == QuadKind::JUMP) {
                        insertPos = it;
                        break;
                    }
                }
                preheader->quadlist->insert(insertPos, initStmts.begin(), initStmts.end());
            }
        }

        auto headerIt = labelToBlock.find(repl.map.headerLabel);
        if (headerIt != labelToBlock.end() && headerIt->second != nullptr) {
            QuadBlock* headerBlock = headerIt->second;
            QuadPhi* newPhi = makePhi(
                repl.map.newPhiTemp,
                repl.map.newBackedgeTemp,
                repl.placement.backedgeLabel,
                repl.initTemps.newInitTemp,
                repl.placement.initLabel);
            if (headerBlock->quadlist != nullptr) {
                auto insertPos = headerBlock->quadlist->begin();
                if (insertPos != headerBlock->quadlist->end()) {
                    ++insertPos;
                }
                for (auto it = headerBlock->quadlist->begin(); it != headerBlock->quadlist->end(); ++it) {
                    if ((*it)->kind == QuadKind::PHI) {
                        insertPos = it + 1;
                    }
                }
                headerBlock->quadlist->insert(insertPos, newPhi);
            }
        }

        auto backIt = labelToBlock.find(repl.placement.backedgeLabel);
        if (backIt != labelToBlock.end() && backIt->second != nullptr) {
            QuadBlock* backBlock = backIt->second;
            QuadStm* updateStm = nullptr;
            if (repl.stepExpr.stepIncrementTempNum != -1) {
                if (repl.stepExpr.stepIncrementNegative) {
                    updateStm = makeBinop(
                        repl.map.newBackedgeTemp,
                        repl.map.newPhiTemp,
                        "-",
                        repl.stepExpr.newStepTemp);
                } else {
                    updateStm = makeBinop(
                        repl.map.newBackedgeTemp,
                        repl.map.newPhiTemp,
                        "+",
                        repl.stepExpr.newStepTemp);
                }
            } else {
                int stepVal = repl.stepExpr.stepIncrementValue;
                if (stepVal >= 0) {
                    updateStm = makeBinopConst(
                        repl.map.newBackedgeTemp, repl.map.newPhiTemp, "+", stepVal);
                } else {
                    updateStm = makeBinopConst(
                        repl.map.newBackedgeTemp, repl.map.newPhiTemp, "-", -stepVal);
                }
            }

            if (backBlock->quadlist != nullptr) {
                auto insertPos = backBlock->quadlist->end();
                for (auto it = backBlock->quadlist->begin(); it != backBlock->quadlist->end(); ++it) {
                    if ((*it)->kind == QuadKind::JUMP) {
                        insertPos = it;
                        break;
                    }
                }
                backBlock->quadlist->insert(insertPos, updateStm);
            }
        }

        replaceTempInFunc(func, repl.map.oldTempNum, repl.map.newPhiTemp);

        for (QuadBlock* block : *func->quadblocklist) {
            if (block == nullptr || block->quadlist == nullptr) {
                continue;
            }
            for (QuadStm* stm : *block->quadlist) {
                if (stm->kind != QuadKind::CJUMP) {
                    continue;
                }
                QuadCJump* cj = dynamic_cast<QuadCJump*>(stm);
                if (cj == nullptr) {
                    continue;
                }
                int leftTemp = getTempNumFromTerm(cj->left);
                if (leftTemp != repl.map.basicTempNum) {
                    continue;
                }
                if (cj->right == nullptr || cj->right->kind != QuadTermKind::CONST) {
                    continue;
                }
                int oldConst = cj->right->get_const();
                int newThreshold = computeCJumpThreshold(repl, oldConst);
                cj->left = makeTempTerm(repl.map.newPhiTemp);
                cj->right = makeConstTerm(newThreshold);
                if (cj->use != nullptr) {
                    cj->use->clear();
                    cj->use->insert(new Temp(repl.map.newPhiTemp));
                }
            }
        }
    }

    for (QuadBlock* block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) {
            continue;
        }
        auto& quads = *block->quadlist;
        quads.erase(
            remove_if(quads.begin(), quads.end(),
                [&](QuadStm* stm) { return plan.stmtsToRemove.count(stm) > 0; }),
            quads.end());
    }

    return func;
}
