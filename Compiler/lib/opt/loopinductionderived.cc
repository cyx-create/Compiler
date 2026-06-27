#include "loopinductionderived.hh"
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

static int getConstFromTerm(QuadTerm* term) {
    if (term != nullptr && term->kind == QuadTermKind::CONST) {
        return term->get_const();
    }
    return 0;
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

static bool parseMulBinopAffine(
    QuadMoveBinop* mul,
    int basicPhiTemp,
    int basicBackedgeTemp,
    int& coeff,
    int& basicRefTemp
) {
    if (mul == nullptr || mul->binop != "*") {
        return false;
    }

    int leftTemp = getTempNumFromTerm(mul->left);
    int rightTemp = getTempNumFromTerm(mul->right);
    int leftConst = mul->left != nullptr && mul->left->kind == QuadTermKind::CONST
        ? mul->left->get_const() : 0;
    int rightConst = mul->right != nullptr && mul->right->kind == QuadTermKind::CONST
        ? mul->right->get_const() : 0;
    bool hasLeftConst = mul->left != nullptr && mul->left->kind == QuadTermKind::CONST;
    bool hasRightConst = mul->right != nullptr && mul->right->kind == QuadTermKind::CONST;

    if (hasLeftConst && rightTemp != -1) {
        coeff = leftConst;
        basicRefTemp = rightTemp;
        return rightTemp == basicPhiTemp || rightTemp == basicBackedgeTemp;
    }
    if (hasRightConst && leftTemp != -1) {
        coeff = rightConst;
        basicRefTemp = leftTemp;
        return leftTemp == basicPhiTemp || leftTemp == basicBackedgeTemp;
    }
    return false;
}

static bool tryParseAffineFromMul(
    int mulTemp,
    const DefUseChain& du,
    int basicPhiTemp,
    int basicBackedgeTemp,
    int& coeff,
    int& basicRefTemp
) {
    VarDefInfo* mulDef = du.getDef(mulTemp);
    if (mulDef == nullptr || mulDef->defStm == nullptr
        || mulDef->defStm->kind != QuadKind::MOVE_BINOP) {
        return false;
    }
    QuadMoveBinop* mul = dynamic_cast<QuadMoveBinop*>(mulDef->defStm);
    return parseMulBinopAffine(mul, basicPhiTemp, basicBackedgeTemp, coeff, basicRefTemp);
}

static bool parseLinearDerivedIV(
    QuadMoveBinop* binop,
    int basicPhiTemp,
    int basicBackedgeTemp,
    AffineIVExpr& expr,
    int& sourceTempNum,
    size_t& sourceOrder,
    const map<QuadStm*, int>& globalOrder,
    QuadStm* defStm
) {
    if (binop == nullptr || (binop->binop != "+" && binop->binop != "-")) {
        return false;
    }

    int leftTemp = getTempNumFromTerm(binop->left);
    int rightTemp = getTempNumFromTerm(binop->right);
    bool leftIsConst = binop->left != nullptr && binop->left->kind == QuadTermKind::CONST;
    bool rightIsConst = binop->right != nullptr && binop->right->kind == QuadTermKind::CONST;

    int basicRefTemp = -1;
    int constant = 0;

    if (!leftIsConst && rightIsConst) {
        basicRefTemp = leftTemp;
        constant = getConstFromTerm(binop->right);
        if (binop->binop == "-") {
            constant = -constant;
        }
    } else if (leftIsConst && !rightIsConst) {
        basicRefTemp = rightTemp;
        constant = getConstFromTerm(binop->left);
        if (binop->binop == "-") {
            constant = -constant;
        }
    } else {
        return false;
    }

    if (basicRefTemp != basicPhiTemp && basicRefTemp != basicBackedgeTemp) {
        return false;
    }

    expr.basicTempNum = basicPhiTemp;
    expr.basicCoeff = 1;
    expr.constant = constant;
    sourceTempNum = basicRefTemp;
    auto orderIt = globalOrder.find(defStm);
    sourceOrder = orderIt != globalOrder.end() ? static_cast<size_t>(orderIt->second)
        : static_cast<size_t>(-1);
    return true;
}

static bool parseDerivedIV(
    QuadStm* defStm,
    const DefUseChain& du,
    int basicPhiTemp,
    int basicBackedgeTemp,
    AffineIVExpr& expr,
    int& sourceTempNum,
    size_t& sourceOrder,
    const map<QuadStm*, int>& globalOrder
) {
    if (defStm == nullptr || defStm->kind != QuadKind::MOVE_BINOP) {
        return false;
    }
    QuadMoveBinop* binop = dynamic_cast<QuadMoveBinop*>(defStm);
    if (binop == nullptr) {
        return false;
    }

    if (binop->binop == "*") {
        int coeff = 0;
        int basicRefTemp = -1;
        if (!parseMulBinopAffine(binop, basicPhiTemp, basicBackedgeTemp, coeff, basicRefTemp)) {
            return false;
        }
        expr.basicTempNum = basicPhiTemp;
        expr.basicCoeff = coeff;
        expr.constant = 0;
        sourceTempNum = basicRefTemp;
        auto orderIt = globalOrder.find(defStm);
        sourceOrder = orderIt != globalOrder.end() ? static_cast<size_t>(orderIt->second)
            : static_cast<size_t>(-1);
        return true;
    }

    if (binop->binop != "+" && binop->binop != "-") {
        return false;
    }

    int leftTemp = getTempNumFromTerm(binop->left);
    int rightTemp = getTempNumFromTerm(binop->right);
    bool leftIsConst = binop->left != nullptr && binop->left->kind == QuadTermKind::CONST;
    bool rightIsConst = binop->right != nullptr && binop->right->kind == QuadTermKind::CONST;

    int coeff = 0;
    int constant = 0;
    int mulTemp = -1;

    if (!leftIsConst && rightIsConst) {
        mulTemp = leftTemp;
        constant = getConstFromTerm(binop->right);
        if (binop->binop == "-") {
            constant = -constant;
        }
    } else if (leftIsConst && !rightIsConst) {
        mulTemp = rightTemp;
        constant = getConstFromTerm(binop->left);
        if (binop->binop == "-") {
            constant = -constant;
        }
    } else {
        return parseLinearDerivedIV(
            binop, basicPhiTemp, basicBackedgeTemp, expr, sourceTempNum, sourceOrder, globalOrder, defStm);
    }

    if (mulTemp == -1) {
        return false;
    }

    int basicRefTemp = -1;
    if (!tryParseAffineFromMul(mulTemp, du, basicPhiTemp, basicBackedgeTemp,
                               coeff, basicRefTemp)) {
        return parseLinearDerivedIV(
            binop, basicPhiTemp, basicBackedgeTemp, expr, sourceTempNum, sourceOrder, globalOrder, defStm);
    }

    expr.basicTempNum = basicPhiTemp;
    expr.basicCoeff = coeff;
    expr.constant = constant;
    sourceTempNum = mulTemp;

    VarDefInfo* sourceDef = du.getDef(mulTemp);
    if (sourceDef != nullptr && sourceDef->defStm != nullptr) {
        auto orderIt = globalOrder.find(sourceDef->defStm);
        sourceOrder = orderIt != globalOrder.end() ? static_cast<size_t>(orderIt->second)
            : static_cast<size_t>(-1);
    } else {
        sourceOrder = static_cast<size_t>(-1);
    }
    return true;
}

static bool sourceAfterBasicUpdate(
    const DerivedInductionVar& div,
    const DefUseChain& du,
    int basicBackedgeTemp
) {
    if (div.expr.basicCoeff == 1 && div.sourceTempNum == basicBackedgeTemp) {
        return true;
    }
    if (div.expr.basicCoeff == 1 && div.sourceTempNum != basicBackedgeTemp) {
        return false;
    }

    VarDefInfo* sourceDef = du.getDef(div.sourceTempNum);
    if (sourceDef == nullptr || sourceDef->defStm == nullptr
        || sourceDef->defStm->kind != QuadKind::MOVE_BINOP) {
        return div.sourceTempNum == basicBackedgeTemp;
    }
    QuadMoveBinop* mul = dynamic_cast<QuadMoveBinop*>(sourceDef->defStm);
    if (mul == nullptr) {
        return div.sourceTempNum == basicBackedgeTemp;
    }
    int leftTemp = getTempNumFromTerm(mul->left);
    int rightTemp = getTempNumFromTerm(mul->right);
    return leftTemp == basicBackedgeTemp || rightTemp == basicBackedgeTemp;
}

map<int, vector<DerivedInductionVar>> discoverDerivedInductionVars(
    QuadFuncDecl* func,
    LoopHeaderMap* loopHeaderMap,
    const DefUseChain& du,
    const ControlFlowInfo& cfi
) {
    map<int, vector<DerivedInductionVar>> result;
    if (func == nullptr || func->quadblocklist == nullptr || loopHeaderMap == nullptr) {
        return result;
    }
    if (!loopHeaderMap->funcLoopHeaders.count(func)) {
        return result;
    }

    auto globalOrder = computeGlobalOrder(func);
    auto labelToBlock = buildLabelToBlock(func);

    map<int, vector<BasicInductionVar>> basicByHeader;
    for (LoopHeader* loopHeader : loopHeaderMap->funcLoopHeaders[func]) {
        if (loopHeader == nullptr) {
            continue;
        }
        int headerLabel = loopHeader->headerLabel;
        auto blockIt = labelToBlock.find(headerLabel);
        if (blockIt == labelToBlock.end()) {
            continue;
        }
        QuadBlock* headerBlock = blockIt->second;
        if (headerBlock == nullptr || headerBlock->quadlist == nullptr) {
            continue;
        }

        vector<BasicInductionVar> basicIVs;
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
            int initTemp = -1, backedgeTemp = -1, backedgeLabel = -1;
            for (const auto& arg : *phi->args) {
                if (arg.first == nullptr || arg.second == nullptr) {
                    continue;
                }
                if (loopHeader->bodyBlocks.count(arg.second->num)) {
                    backedgeTemp = arg.first->num;
                    backedgeLabel = arg.second->num;
                } else {
                    initTemp = arg.first->num;
                }
            }
            if (backedgeTemp == -1 || backedgeLabel == -1) {
                continue;
            }
            auto bbIt = labelToBlock.find(backedgeLabel);
            if (bbIt == labelToBlock.end()) {
                continue;
            }
            int step = 0, stepTempNum = -1;
            QuadStm* updateStm = nullptr;
            for (QuadStm* ust : *bbIt->second->quadlist) {
                if (ust->kind != QuadKind::MOVE_BINOP) continue;
                QuadMoveBinop* b = dynamic_cast<QuadMoveBinop*>(ust);
                if (b == nullptr || b->dst == nullptr || b->dst->temp == nullptr) continue;
                if (b->dst->temp->num != backedgeTemp) continue;
                int lt = getTempNumFromTerm(b->left);
                if (lt == phiTemp) {
                    updateStm = ust;
                    if (b->binop == "+") {
                        if (b->right->kind == QuadTermKind::CONST) step = b->right->get_const();
                        else stepTempNum = getTempNumFromTerm(b->right);
                    } else if (b->binop == "-") {
                        if (b->right->kind == QuadTermKind::CONST) step = -b->right->get_const();
                        else stepTempNum = -getTempNumFromTerm(b->right);
                    }
                    break;
                }
            }
            if (updateStm == nullptr) continue;
            BasicInductionVar biv(headerLabel, phiTemp, initTemp, backedgeTemp, step, phi, updateStm);
            biv.stepTempNum = stepTempNum;
            basicIVs.push_back(biv);
        }
        basicByHeader[headerLabel] = basicIVs;
    }

    for (LoopHeader* loopHeader : loopHeaderMap->funcLoopHeaders[func]) {
        if (loopHeader == nullptr) {
            continue;
        }
        int headerLabel = loopHeader->headerLabel;
        if (!basicByHeader.count(headerLabel)) {
            continue;
        }

        set<int> seenDerived;
        for (const BasicInductionVar& biv : basicByHeader[headerLabel]) {
            for (int bodyLabel : loopHeader->bodyBlocks) {
                auto blockIt = labelToBlock.find(bodyLabel);
                if (blockIt == labelToBlock.end() || blockIt->second == nullptr) {
                    continue;
                }
                QuadBlock* block = blockIt->second;
                if (block->quadlist == nullptr) {
                    continue;
                }
                for (QuadStm* stm : *block->quadlist) {
                    if (stm->kind != QuadKind::MOVE_BINOP) {
                        continue;
                    }
                    QuadMoveBinop* binop = dynamic_cast<QuadMoveBinop*>(stm);
                    if (binop == nullptr || binop->dst == nullptr || binop->dst->temp == nullptr) {
                        continue;
                    }
                    int derivedTemp = binop->dst->temp->num;
                    if (derivedTemp == biv.phiTempNum || derivedTemp == biv.backedgeTempNum) {
                        continue;
                    }
                    if (seenDerived.count(derivedTemp)) {
                        continue;
                    }

                    AffineIVExpr expr;
                    int sourceTempNum = -1;
                    size_t srcOrder = static_cast<size_t>(-1);
                    if (!parseDerivedIV(stm, du, biv.phiTempNum, biv.backedgeTempNum,
                                        expr, sourceTempNum, srcOrder, globalOrder)) {
                        continue;
                    }

                    seenDerived.insert(derivedTemp);
                    DerivedInductionVar div(
                        headerLabel, derivedTemp, sourceTempNum, srcOrder, expr, stm);
                    result[headerLabel].push_back(div);
                }
            }
        }
    }

    for (LoopHeader* loopHeader : loopHeaderMap->funcLoopHeaders[func]) {
        if (loopHeader == nullptr) {
            continue;
        }
        int headerLabel = loopHeader->headerLabel;
        if (!result.count(headerLabel)) {
            continue;
        }

        vector<DerivedInductionVar>& divs = result[headerLabel];
        vector<DerivedInductionVar> filtered;
        for (const DerivedInductionVar& d : divs) {
            bool intermediate = false;
            for (const DerivedInductionVar& other : divs) {
                if (other.tempNum == d.tempNum) {
                    continue;
                }
                if (other.sourceTempNum == d.tempNum) {
                    intermediate = true;
                    break;
                }
            }
            if (!intermediate) {
                filtered.push_back(d);
            }
        }
        divs = filtered;
    }

    return result;
}
