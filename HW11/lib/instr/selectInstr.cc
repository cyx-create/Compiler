#include "instrSelection.hh"

#include <cstdint>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

namespace instr {

namespace {

static tree::Temp *makeTemp(int num) {
    return new tree::Temp(num);
}

static bool fitsMovImmediate(int value) {
    uint32_t u = static_cast<uint32_t>(value);
    for (int shift = 0; shift < 32; shift += 2) {
        if ((u & ~0xFFu) == 0) {
            return true;
        }
        u = (u << 2) | (u >> 30);
    }
    return false;
}

static bool isMoveExtCallStmt(const quad::QuadStm *stm) {
    return stm != nullptr && stm->kind == quad::QuadKind::MOVE_EXTCALL;
}

static bool isExtCallLikeStmt(const quad::QuadStm *stm) {
    return stm != nullptr &&
           (stm->kind == quad::QuadKind::MOVE_EXTCALL ||
            stm->kind == quad::QuadKind::EXTCALL);
}

struct SelectionContext {
    int &nextTempNum;
    const advDFG &graph;
    const std::vector<const advDFGNode *> *blockNodes = nullptr;
};

struct StatementBundle {
    const quad::QuadStm *stm = nullptr;
    const advDFGNode *node = nullptr;
    AssemInstrList instrs;
    int tempDefined = -1;
    std::set<int> tempsUsed;
    size_t quadIndex = 0;
    bool isPtrCalcBundle = false;
};

static void appendOper(
    StatementBundle &bundle,
    const std::string &assem,
    const std::vector<tree::Temp *> &dst = {},
    const std::vector<tree::Temp *> &src = {},
    const AssemTargets &jumps = AssemTargets()
) {
    bundle.instrs.append(AssemInstr::Oper(assem, dst, src, jumps));
}

static void appendMove(
    StatementBundle &bundle,
    const std::string &assem,
    const std::vector<tree::Temp *> &dst,
    const std::vector<tree::Temp *> &src
) {
    bundle.instrs.append(AssemInstr::Move(assem, dst, src));
}

static int allocateScratchTemp(SelectionContext &ctx) {
    return ctx.nextTempNum++;
}

static tree::Temp *materializeConstMovw(
    SelectionContext &ctx,
    StatementBundle &bundle,
    int value
) {
    int tempNum = allocateScratchTemp(ctx);
    tree::Temp *temp = makeTemp(tempNum);
    appendOper(
        bundle,
        "movw `d0, #" + std::to_string(value & 0xFFFF),
        {temp},
        {}
    );
    return temp;
}

static const advDFGNode *findChainDefNode(
    const advDFG &graph,
    int chainId
) {
    for (auto *node : graph.getNodes()) {
        if (node != nullptr && node->chainDefined == chainId) {
            return node;
        }
    }
    return nullptr;
}

static int ptrCalcConstOffset(const quad::QuadPtrCalc *ptrCalc) {
    if (ptrCalc == nullptr || ptrCalc->offset == nullptr ||
        ptrCalc->offset->kind != quad::QuadTermKind::CONST) {
        return -1;
    }
    return ptrCalc->offset->get_const();
}

static int ptrCalcBaseTempNum(const quad::QuadPtrCalc *ptrCalc) {
    if (ptrCalc == nullptr || ptrCalc->ptr == nullptr ||
        ptrCalc->ptr->kind != quad::QuadTermKind::TEMP) {
        return -1;
    }
    return ptrCalc->ptr->get_temp()->temp->num;
}

static int ptrCalcOffsetTempNum(const quad::QuadPtrCalc *ptrCalc) {
    if (ptrCalc == nullptr || ptrCalc->offset == nullptr ||
        ptrCalc->offset->kind != quad::QuadTermKind::TEMP) {
        return -1;
    }
    return ptrCalc->offset->get_temp()->temp->num;
}

static int ptrCalcDstTempNum(const quad::QuadPtrCalc *ptrCalc) {
    if (ptrCalc == nullptr || ptrCalc->dst == nullptr) {
        return -1;
    }
    return ptrCalc->dst->get_temp()->temp->num;
}

static bool chainUsedOnlyByLoad(const advDFG &graph, int chainId) {
    bool hasUser = false;
    for (auto *node : graph.getNodes()) {
        if (node == nullptr || node->chainUsed != chainId ||
            node->quadStatement == nullptr) {
            continue;
        }
        hasUser = true;
        if (node->quadStatement->kind != quad::QuadKind::LOAD) {
            return false;
        }
    }
    return hasUser;
}

static bool storeNeedsExplicitChain(
    const advDFG &graph,
    const quad::QuadStore *store,
    const quad::QuadPtrCalc *ptrCalc
) {
    if (store == nullptr) {
        return false;
    }
    if (store->src->kind == quad::QuadTermKind::TEMP) {
        return true;
    }
    if (store->src->kind == quad::QuadTermKind::NAME) {
        return ptrCalcConstOffset(ptrCalc) != 0;
    }
    return false;
}

static bool hasTempStoreChainAtOffsetZero(
    const advDFG &graph,
    int baseTempNum
) {
    for (auto *node : graph.getNodes()) {
        if (node == nullptr || node->quadStatement == nullptr ||
            node->quadStatement->kind != quad::QuadKind::PTR_CALC ||
            node->chainDefined < 0) {
            continue;
        }
        auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc *>(node->quadStatement);
        if (ptrCalc == nullptr || ptrCalcConstOffset(ptrCalc) != 0 ||
            ptrCalcBaseTempNum(ptrCalc) != baseTempNum) {
            continue;
        }
        for (auto *user : graph.getNodes()) {
            if (user == nullptr || user->chainUsed != node->chainDefined ||
                user->quadStatement == nullptr ||
                user->quadStatement->kind != quad::QuadKind::STORE) {
                continue;
            }
            auto *store = dynamic_cast<const quad::QuadStore *>(user->quadStatement);
            if (store != nullptr && store->src->kind == quad::QuadTermKind::TEMP) {
                return true;
            }
        }
    }
    return false;
}

static bool isChainCompanionPtrCalc(
    const advDFG &graph,
    const advDFGNode *node
) {
    if (node == nullptr || node->quadStatement == nullptr ||
        node->quadStatement->kind != quad::QuadKind::PTR_CALC) {
        return false;
    }
    auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc *>(node->quadStatement);
    if (ptrCalc == nullptr || ptrCalcConstOffset(ptrCalc) == 0) {
        return false;
    }
    int baseNum = ptrCalcBaseTempNum(ptrCalc);
    if (!hasTempStoreChainAtOffsetZero(graph, baseNum)) {
        return false;
    }
    for (auto *user : graph.getNodes()) {
        if (user == nullptr || user->chainUsed != node->chainDefined ||
            user->quadStatement == nullptr ||
            user->quadStatement->kind != quad::QuadKind::STORE) {
            continue;
        }
        auto *store = dynamic_cast<const quad::QuadStore *>(user->quadStatement);
        if (store == nullptr) {
            continue;
        }
        if (store->src->kind == quad::QuadTermKind::NAME ||
            store->src->kind == quad::QuadTermKind::TEMP) {
            return true;
        }
    }
    return false;
}

static bool ptrCalcUsesCompanionChainBarrier(
    const advDFG &graph,
    const advDFGNode *node
) {
    if (isChainCompanionPtrCalc(graph, node)) {
        return true;
    }
    if (node == nullptr || node->quadStatement == nullptr ||
        node->quadStatement->kind != quad::QuadKind::PTR_CALC) {
        return false;
    }
    auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc *>(node->quadStatement);
    if (ptrCalc == nullptr || ptrCalcConstOffset(ptrCalc) != 0) {
        return false;
    }
    int baseNum = ptrCalcBaseTempNum(ptrCalc);
    if (!hasTempStoreChainAtOffsetZero(graph, baseNum)) {
        return false;
    }
    for (auto *user : graph.getNodes()) {
        if (user == nullptr || user->chainUsed != node->chainDefined ||
            user->quadStatement == nullptr ||
            user->quadStatement->kind != quad::QuadKind::STORE) {
            continue;
        }
        auto *store = dynamic_cast<const quad::QuadStore *>(user->quadStatement);
        if (store != nullptr && store->src->kind == quad::QuadTermKind::TEMP) {
            return true;
        }
    }
    return false;
}

static bool tempDefinedInBlock(
    const std::vector<const advDFGNode *> &nodes,
    int tempNum
) {
    if (tempNum < 0) {
        return false;
    }
    for (auto *node : nodes) {
        if (node != nullptr && node->tempDefined == tempNum) {
            return true;
        }
    }
    return false;
}

static bool blockHasMulBinop(
    const std::vector<const advDFGNode *> &nodes
) {
    for (auto *node : nodes) {
        if (node == nullptr || node->quadStatement == nullptr ||
            node->quadStatement->kind != quad::QuadKind::MOVE_BINOP) {
            continue;
        }
        auto *binop = dynamic_cast<const quad::QuadMoveBinop *>(node->quadStatement);
        if (binop != nullptr && binop->binop == "*") {
            return true;
        }
    }
    return false;
}

static bool movePreparesDynamicPtrCalcBase(
    const advDFG &graph,
    int dstTempNum
) {
    if (dstTempNum < 0) {
        return false;
    }
    for (auto *node : graph.getNodes()) {
        if (node == nullptr || node->quadStatement == nullptr ||
            node->quadStatement->kind != quad::QuadKind::PTR_CALC) {
            continue;
        }
        auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc *>(node->quadStatement);
        if (ptrCalc != nullptr &&
            ptrCalcBaseTempNum(ptrCalc) == dstTempNum &&
            ptrCalcConstOffset(ptrCalc) < 0) {
            return true;
        }
    }
    return false;
}

static int nodeIndexInBlock(
    const std::vector<const advDFGNode *> &nodes,
    const advDFGNode *target
) {
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i] == target) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

static bool hasInterleavedMulBetween(
    const std::vector<const advDFGNode *> &nodes,
    size_t startIndex,
    size_t endIndex
) {
    if (startIndex >= endIndex) {
        return false;
    }
    for (size_t i = startIndex + 1; i < endIndex && i < nodes.size(); ++i) {
        if (nodes[i] == nullptr || nodes[i]->quadStatement == nullptr ||
            nodes[i]->quadStatement->kind != quad::QuadKind::MOVE_BINOP) {
            continue;
        }
        auto *binop = dynamic_cast<const quad::QuadMoveBinop *>(nodes[i]->quadStatement);
        if (binop != nullptr && binop->binop == "*") {
            return true;
        }
    }
    return false;
}

static int callObjectTempForStmt(const quad::QuadStm *stm) {
    if (stm == nullptr) {
        return -1;
    }
    if (stm->kind == quad::QuadKind::CALL) {
        auto *call = dynamic_cast<const quad::QuadCall *>(stm);
        if (call != nullptr && call->obj_term != nullptr &&
            call->obj_term->kind == quad::QuadTermKind::TEMP) {
            return call->obj_term->get_temp()->temp->num;
        }
    }
    if (stm->kind == quad::QuadKind::MOVE_CALL) {
        auto *moveCall = dynamic_cast<const quad::QuadMoveCall *>(stm);
        if (moveCall != nullptr && moveCall->call != nullptr &&
            moveCall->call->obj_term != nullptr &&
            moveCall->call->obj_term->kind == quad::QuadTermKind::TEMP) {
            return moveCall->call->obj_term->get_temp()->temp->num;
        }
    }
    return -1;
}

static int nextCallUsingObjectAfter(
    const std::vector<const advDFGNode *> &nodes,
    int objectTempNum,
    size_t afterIndex
) {
    if (objectTempNum < 0) {
        return -1;
    }
    for (size_t i = afterIndex + 1; i < nodes.size(); ++i) {
        if (nodes[i] == nullptr || nodes[i]->quadStatement == nullptr) {
            continue;
        }
        if (callObjectTempForStmt(nodes[i]->quadStatement) == objectTempNum) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

static bool isSubConstBinop(const quad::QuadStm *stm);

static bool isConstMoveStmt(const quad::QuadStm *stm) {
    if (stm == nullptr || stm->kind != quad::QuadKind::MOVE) {
        return false;
    }
    auto *move = dynamic_cast<const quad::QuadMove *>(stm);
    return move != nullptr && move->src != nullptr &&
           move->src->kind == quad::QuadTermKind::CONST;
}

static bool extCallWinsOverAdjacentConstMove(size_t movIndex, size_t extIndex) {
    return extIndex == movIndex + 1;
}

static bool stmtUsesTempAsCallArg(
    const quad::QuadStm *stm,
    int tempNum
) {
    if (stm == nullptr || tempNum < 0) {
        return false;
    }

    const quad::QuadCall *call = nullptr;
    if (stm->kind == quad::QuadKind::CALL) {
        call = dynamic_cast<const quad::QuadCall *>(stm);
    } else if (stm->kind == quad::QuadKind::MOVE_CALL) {
        auto *moveCall = dynamic_cast<const quad::QuadMoveCall *>(stm);
        call = moveCall == nullptr ? nullptr : moveCall->call;
    }
    if (call == nullptr || call->args == nullptr) {
        return false;
    }

    for (auto *arg : *call->args) {
        if (arg != nullptr && arg->kind == quad::QuadTermKind::TEMP &&
            arg->get_temp()->temp->num == tempNum) {
            return true;
        }
    }
    return false;
}

static bool moveUsedAsCallArgumentAfter(
    const std::vector<const advDFGNode *> &nodes,
    int tempNum,
    size_t afterIndex
) {
    if (tempNum < 0) {
        return false;
    }
    for (size_t i = afterIndex + 1; i < nodes.size(); ++i) {
        if (nodes[i] == nullptr || nodes[i]->quadStatement == nullptr) {
            continue;
        }
        if (stmtUsesTempAsCallArg(nodes[i]->quadStatement, tempNum)) {
            return true;
        }
    }
    return false;
}

static int firstPendingNameStoreIndex(
    const advDFG &graph,
    const std::vector<const advDFGNode *> &nodes,
    const quad::QuadMove *move
) {
    if (move == nullptr || move->src == nullptr ||
        move->src->kind != quad::QuadTermKind::TEMP) {
        return -1;
    }
    int srcNum = move->src->get_temp()->temp->num;
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i] == nullptr || nodes[i]->quadStatement == nullptr ||
            nodes[i]->quadStatement->kind != quad::QuadKind::STORE) {
            continue;
        }
        auto *store = dynamic_cast<const quad::QuadStore *>(nodes[i]->quadStatement);
        if (store == nullptr || store->src == nullptr ||
            store->src->kind != quad::QuadTermKind::NAME) {
            continue;
        }
        if (nodes[i]->chainUsed < 0) {
            if (store->dst != nullptr &&
                store->dst->kind == quad::QuadTermKind::TEMP &&
                store->dst->get_temp()->temp->num == srcNum) {
                return static_cast<int>(i);
            }
            continue;
        }
        auto *chainNode = findChainDefNode(graph, nodes[i]->chainUsed);
        auto *ptrCalc = chainNode == nullptr
                            ? nullptr
                            : dynamic_cast<const quad::QuadPtrCalc *>(
                                  chainNode->quadStatement);
        if (ptrCalc != nullptr && ptrCalcBaseTempNum(ptrCalc) == srcNum) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

static bool pendingNameStoreToMoveSrc(
    const advDFG &graph,
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled,
    const quad::QuadMove *move
) {
    if (move == nullptr || move->src == nullptr ||
        move->src->kind != quad::QuadTermKind::TEMP) {
        return false;
    }
    int srcNum = move->src->get_temp()->temp->num;
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (scheduled[i] || nodes[i] == nullptr ||
            nodes[i]->quadStatement == nullptr ||
            nodes[i]->quadStatement->kind != quad::QuadKind::STORE) {
            continue;
        }
        auto *store = dynamic_cast<const quad::QuadStore *>(nodes[i]->quadStatement);
        if (store == nullptr || store->src == nullptr ||
            store->src->kind != quad::QuadTermKind::NAME) {
            continue;
        }
        if (nodes[i]->chainUsed < 0) {
            if (store->dst != nullptr &&
                store->dst->kind == quad::QuadTermKind::TEMP &&
                store->dst->get_temp()->temp->num == srcNum) {
                return true;
            }
            continue;
        }
        auto *chainNode = findChainDefNode(graph, nodes[i]->chainUsed);
        auto *ptrCalc = chainNode == nullptr
                            ? nullptr
                            : dynamic_cast<const quad::QuadPtrCalc *>(
                                  chainNode->quadStatement);
        if (ptrCalc != nullptr && ptrCalcBaseTempNum(ptrCalc) == srcNum) {
            return true;
        }
    }
    return false;
}

static bool isMallocMoveExtCall(const quad::QuadStm *stm) {
    if (stm == nullptr || stm->kind != quad::QuadKind::MOVE_EXTCALL) {
        return false;
    }
    auto *moveExt = dynamic_cast<const quad::QuadMoveExtCall *>(stm);
    return moveExt != nullptr && moveExt->extcall != nullptr &&
           moveExt->extcall->extfun == "malloc";
}

static bool allMallocExtCallsScheduledInBlock(
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled
) {
    bool hasMalloc = false;
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i] == nullptr || nodes[i]->quadStatement == nullptr ||
            !isMallocMoveExtCall(nodes[i]->quadStatement)) {
            continue;
        }
        hasMalloc = true;
        if (!scheduled[i]) {
            return false;
        }
    }
    return hasMalloc;
}

static int countMallocExtCallsInBlock(
    const std::vector<const advDFGNode *> &nodes
) {
    int count = 0;
    for (auto *node : nodes) {
        if (node == nullptr || node->quadStatement == nullptr ||
            !isMallocMoveExtCall(node->quadStatement)) {
            continue;
        }
        ++count;
    }
    return count;
}

static bool tempIsMallocResultInBlock(
    const std::vector<const advDFGNode *> &nodes,
    int tempNum
) {
    if (tempNum < 0) {
        return false;
    }
    for (auto *node : nodes) {
        if (node == nullptr || node->quadStatement == nullptr ||
            !isMallocMoveExtCall(node->quadStatement)) {
            continue;
        }
        auto *moveExt = dynamic_cast<const quad::QuadMoveExtCall *>(
            node->quadStatement);
        if (moveExt != nullptr && moveExt->dst != nullptr &&
            moveExt->dst->temp->num == tempNum) {
            return true;
        }
    }
    return false;
}

static int countCallsInBlock(const std::vector<const advDFGNode *> &nodes) {
    int count = 0;
    for (auto *node : nodes) {
        if (node == nullptr || node->quadStatement == nullptr) {
            continue;
        }
        if (node->quadStatement->kind == quad::QuadKind::CALL ||
            node->quadStatement->kind == quad::QuadKind::MOVE_CALL) {
            ++count;
        }
    }
    return count;
}

static bool loadResultFeedsCallObjectInBlock(
    const std::vector<const advDFGNode *> &nodes,
    size_t loadIndex
) {
    if (loadIndex >= nodes.size() || nodes[loadIndex] == nullptr ||
        nodes[loadIndex]->quadStatement == nullptr ||
        nodes[loadIndex]->quadStatement->kind != quad::QuadKind::LOAD) {
        return false;
    }
    auto *load = dynamic_cast<const quad::QuadLoad *>(nodes[loadIndex]->quadStatement);
    if (load == nullptr || load->dst == nullptr) {
        return false;
    }
    int dstNum = load->dst->temp->num;
    for (size_t i = loadIndex + 1; i < nodes.size(); ++i) {
        if (nodes[i] == nullptr || nodes[i]->quadStatement == nullptr) {
            continue;
        }
        if (callObjectTempForStmt(nodes[i]->quadStatement) == dstNum) {
            return true;
        }
        if (nodes[i]->quadStatement->kind == quad::QuadKind::MOVE) {
            auto *move = dynamic_cast<const quad::QuadMove *>(nodes[i]->quadStatement);
            if (move != nullptr && move->src != nullptr &&
                move->src->kind == quad::QuadTermKind::TEMP &&
                move->src->get_temp()->temp->num == dstNum &&
                move->dst != nullptr) {
                int moveDst = move->dst->temp->num;
                for (size_t j = i + 1; j < nodes.size(); ++j) {
                    if (nodes[j] != nullptr &&
                        callObjectTempForStmt(nodes[j]->quadStatement) == moveDst) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

static bool blockHasUnscheduledLoadFedCall(
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled
) {
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (scheduled[i] || nodes[i] == nullptr ||
            nodes[i]->quadStatement == nullptr ||
            nodes[i]->quadStatement->kind != quad::QuadKind::LOAD) {
            continue;
        }
        if (loadResultFeedsCallObjectInBlock(nodes, i)) {
            return true;
        }
    }
    return false;
}

static bool tempDefinedByMoveInBlock(
    const std::vector<const advDFGNode *> &nodes,
    int tempNum
) {
    if (tempNum < 0) {
        return false;
    }
    for (auto *node : nodes) {
        if (node == nullptr || node->quadStatement == nullptr ||
            node->quadStatement->kind != quad::QuadKind::MOVE) {
            continue;
        }
        auto *move = dynamic_cast<const quad::QuadMove *>(node->quadStatement);
        if (move != nullptr && move->dst != nullptr &&
            move->dst->temp->num == tempNum) {
            return true;
        }
    }
    return false;
}

static bool moveDstUsedByUnscheduledLoadInBlock(
    const advDFG &graph,
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled,
    int dstNum
) {
    if (dstNum < 0) {
        return false;
    }
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (scheduled[i] || nodes[i] == nullptr ||
            nodes[i]->quadStatement == nullptr ||
            nodes[i]->quadStatement->kind != quad::QuadKind::LOAD) {
            continue;
        }
        auto *load = dynamic_cast<const quad::QuadLoad *>(nodes[i]->quadStatement);
        if (load == nullptr || load->src == nullptr) {
            continue;
        }
        if (load->src->kind == quad::QuadTermKind::TEMP &&
            load->src->get_temp()->temp->num == dstNum) {
            return true;
        }
    }
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (scheduled[i] || nodes[i] == nullptr ||
            nodes[i]->chainUsed < 0) {
            continue;
        }
        auto *chainNode = findChainDefNode(graph, nodes[i]->chainUsed);
        auto *ptrCalc = chainNode == nullptr
                            ? nullptr
                            : dynamic_cast<const quad::QuadPtrCalc *>(
                                  chainNode->quadStatement);
        if (ptrCalc == nullptr || ptrCalcBaseTempNum(ptrCalc) != dstNum) {
            continue;
        }
        if (nodes[i]->quadStatement != nullptr &&
            nodes[i]->quadStatement->kind == quad::QuadKind::LOAD) {
            return true;
        }
    }
    return false;
}

static int loadDefiningTempBefore(
    const std::vector<const advDFGNode *> &nodes,
    int tempNum,
    size_t beforeIndex
) {
    if (tempNum < 0) {
        return -1;
    }
    for (size_t i = 0; i < beforeIndex && i < nodes.size(); ++i) {
        if (nodes[i] == nullptr || nodes[i]->quadStatement == nullptr ||
            nodes[i]->quadStatement->kind != quad::QuadKind::LOAD) {
            continue;
        }
        auto *load = dynamic_cast<const quad::QuadLoad *>(nodes[i]->quadStatement);
        if (load != nullptr && load->dst != nullptr &&
            load->dst->temp->num == tempNum) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

static bool hasUnscheduledMoveExtCallBetween(
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled,
    size_t startExclusive,
    size_t endExclusive
) {
    for (size_t i = startExclusive + 1; i < endExclusive && i < nodes.size(); ++i) {
        if (scheduled[i] || nodes[i] == nullptr ||
            nodes[i]->quadStatement == nullptr) {
            continue;
        }
        if (isMoveExtCallStmt(nodes[i]->quadStatement)) {
            return true;
        }
    }
    return false;
}

static int companionTempStoreNodeIndex(
    const advDFG &graph,
    const std::vector<const advDFGNode *> &nodes,
    const advDFGNode *ptrCalcNode
) {
    if (ptrCalcNode == nullptr || ptrCalcNode->chainDefined < 0) {
        return -1;
    }
    int baseNum = -1;
    auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc *>(ptrCalcNode->quadStatement);
    if (ptrCalc != nullptr) {
        baseNum = ptrCalcBaseTempNum(ptrCalc);
    }
    for (size_t i = 0; i < nodes.size(); ++i) {
        auto *node = nodes[i];
        if (node == nullptr || node->quadStatement == nullptr ||
            node->chainDefined < 0) {
            continue;
        }
        auto *chainPtrCalc = dynamic_cast<const quad::QuadPtrCalc *>(node->quadStatement);
        if (chainPtrCalc == nullptr || ptrCalcConstOffset(chainPtrCalc) != 0 ||
            ptrCalcBaseTempNum(chainPtrCalc) != baseNum) {
            continue;
        }
        for (auto *user : graph.getNodes()) {
            if (user == nullptr || user->chainUsed != node->chainDefined ||
                user->quadStatement == nullptr ||
                user->quadStatement->kind != quad::QuadKind::STORE) {
                continue;
            }
            auto *store = dynamic_cast<const quad::QuadStore *>(user->quadStatement);
            if (store == nullptr || store->src->kind != quad::QuadTermKind::TEMP) {
                continue;
            }
            for (size_t j = 0; j < nodes.size(); ++j) {
                if (nodes[j] == user) {
                    return static_cast<int>(j);
                }
            }
        }
    }
    return -1;
}

static bool allMoveExtCallsBeforeScheduled(
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled,
    size_t boundIndex
) {
    for (size_t j = 0; j < boundIndex && j < nodes.size(); ++j) {
        if (scheduled[j] || nodes[j] == nullptr ||
            nodes[j]->quadStatement == nullptr) {
            continue;
        }
        if (isMoveExtCallStmt(nodes[j]->quadStatement)) {
            return false;
        }
    }

    return true;
}

static bool hasAddConstBinopInBlock(
    const std::vector<const advDFGNode *> &nodes
) {
    for (auto *node : nodes) {
        if (node == nullptr || node->quadStatement == nullptr ||
            node->quadStatement->kind != quad::QuadKind::MOVE_BINOP) {
            continue;
        }
        auto *binop = dynamic_cast<const quad::QuadMoveBinop *>(node->quadStatement);
        if (binop != nullptr && binop->binop == "+" &&
            binop->right->kind == quad::QuadTermKind::CONST) {
            return true;
        }
    }
    return false;
}

static bool storeConstChainNeedsExplicitPtrCalc(
    const quad::QuadPtrCalc *ptrCalc
) {
    (void)ptrCalc;
    return false;
}

static void emitLoadWithChain(
    StatementBundle &bundle,
    int dstNum,
    const quad::QuadPtrCalc *ptrCalc
) {
    int baseNum = ptrCalcBaseTempNum(ptrCalc);
    tree::Temp *dst = makeTemp(dstNum);
    tree::Temp *base = makeTemp(baseNum);
    int constOff = ptrCalcConstOffset(ptrCalc);

    if (constOff >= 0) {
        if (constOff == 0) {
            appendOper(bundle, "ldr `d0, [`s0]", {dst}, {base});
        } else {
            appendOper(
                bundle,
                "ldr `d0, [`s0, #" + std::to_string(constOff) + "]",
                {dst},
                {base}
            );
        }
        return;
    }

    int offNum = ptrCalcOffsetTempNum(ptrCalc);
    appendOper(
        bundle,
        "ldr `d0, [`s0, `s1]",
        {dst},
        {base, makeTemp(offNum)}
    );
}

static void emitLoadWithExplicitPtr(
    StatementBundle &bundle,
    int dstNum,
    int ptrTempNum
) {
    appendOper(
        bundle,
        "ldr `d0, [`s0]",
        {makeTemp(dstNum)},
        {makeTemp(ptrTempNum)}
    );
}

static void emitStoreWithChain(
    SelectionContext &ctx,
    StatementBundle &bundle,
    const quad::QuadStore *store,
    const quad::QuadPtrCalc *ptrCalc,
    bool ptrTempAlreadyDefined
) {
    int baseNum = ptrCalcBaseTempNum(ptrCalc);
    int ptrTempNum = ptrCalcDstTempNum(ptrCalc);
    int constOff = ptrCalcConstOffset(ptrCalc);
    tree::Temp *srcTemp = nullptr;

    if (store->src->kind == quad::QuadTermKind::TEMP) {
        srcTemp = makeTemp(store->src->get_temp()->temp->num);
    } else if (store->src->kind == quad::QuadTermKind::CONST) {
        srcTemp = materializeConstMovw(ctx, bundle, store->src->get_const());
    } else if (store->src->kind == quad::QuadTermKind::NAME) {
        int scratchNum = allocateScratchTemp(ctx);
        tree::Temp *scratch = makeTemp(scratchNum);
        appendOper(bundle, "adr `d0, " + store->src->get_name(), {scratch}, {});
        srcTemp = scratch;
    }

    if (srcTemp == nullptr) {
        return;
    }

    if (store->src->kind == quad::QuadTermKind::NAME &&
        ptrCalcConstOffset(ptrCalc) == 0) {
        appendOper(
            bundle,
            "str `d0, [`s0]",
            {srcTemp},
            {makeTemp(baseNum)}
        );
        return;
    }

    if (store->src->kind == quad::QuadTermKind::TEMP &&
        ptrCalcConstOffset(ptrCalc) == 0) {
        if (ptrTempAlreadyDefined) {
            appendOper(
                bundle,
                "str `d0, [`s0]",
                {srcTemp},
                {makeTemp(ptrTempNum)}
            );
        } else {
            tree::Temp *ptrTemp = makeTemp(ptrTempNum);
            tree::Temp *base = makeTemp(baseNum);
            appendMove(bundle, "mov `d0, `s0", {ptrTemp}, {base});
            appendOper(bundle, "str `d0, [`s0]", {srcTemp}, {ptrTemp});
        }
        return;
    }

    if (ptrTempAlreadyDefined &&
        (storeNeedsExplicitChain(ctx.graph, store, ptrCalc) ||
         ptrCalcConstOffset(ptrCalc) < 0 ||
         storeConstChainNeedsExplicitPtrCalc(ptrCalc))) {
        appendOper(
            bundle,
            "str `d0, [`s0]",
            {srcTemp},
            {makeTemp(ptrTempNum)}
        );
        return;
    }

    tree::Temp *base = makeTemp(baseNum);
    if (constOff >= 0) {
        if (constOff == 0) {
            appendOper(bundle, "str `d0, [`s0]", {srcTemp}, {base});
        } else {
            appendOper(
                bundle,
                "str `d0, [`s0, #" + std::to_string(constOff) + "]",
                {srcTemp},
                {base}
            );
        }
        return;
    }

    int offNum = ptrCalcOffsetTempNum(ptrCalc);
    appendOper(
        bundle,
        "str `d0, [`s0, `s1]",
        {srcTemp},
        {base, makeTemp(offNum)}
    );
}

static void emitStoreDirect(
    SelectionContext &ctx,
    StatementBundle &bundle,
    const quad::QuadStore *store
) {
    if (store->dst == nullptr || store->dst->kind != quad::QuadTermKind::TEMP) {
        return;
    }

    tree::Temp *base = makeTemp(store->dst->get_temp()->temp->num);
    tree::Temp *srcTemp = nullptr;

    if (store->src->kind == quad::QuadTermKind::TEMP) {
        srcTemp = makeTemp(store->src->get_temp()->temp->num);
    } else if (store->src->kind == quad::QuadTermKind::CONST) {
        srcTemp = materializeConstMovw(ctx, bundle, store->src->get_const());
    } else if (store->src->kind == quad::QuadTermKind::NAME) {
        int scratchNum = allocateScratchTemp(ctx);
        tree::Temp *scratch = makeTemp(scratchNum);
        appendOper(bundle, "adr `d0, " + store->src->get_name(), {scratch}, {});
        srcTemp = scratch;
    }

    if (srcTemp != nullptr) {
        appendOper(bundle, "str `d0, [`s0]", {srcTemp}, {base});
    }
}

static void emitPtrCalcBundle(
    StatementBundle &bundle,
    const quad::QuadPtrCalc *ptrCalc
) {
    int dstNum = ptrCalcDstTempNum(ptrCalc);
    int baseNum = ptrCalcBaseTempNum(ptrCalc);
    int constOff = ptrCalcConstOffset(ptrCalc);
    tree::Temp *dst = makeTemp(dstNum);
    tree::Temp *base = makeTemp(baseNum);

    if (constOff >= 0) {
        if (constOff == 0) {
            appendMove(bundle, "mov `d0, `s0", {dst}, {base});
        } else {
            appendOper(
                bundle,
                "add `d0, `s0, #" + std::to_string(constOff),
                {dst},
                {base}
            );
        }
    } else {
        int offNum = ptrCalcOffsetTempNum(ptrCalc);
        appendOper(
            bundle,
            "add `d0, `s0, `s1",
            {dst},
            {base, makeTemp(offNum)}
        );
    }
}

static void emitExtCallArgs(
    SelectionContext &ctx,
    StatementBundle &bundle,
    const std::vector<quad::QuadTerm *> &args
) {
    static const char *regNames[] = {"r0", "r1", "r2", "r3"};
    for (size_t i = 0; i < args.size() && i < 4; ++i) {
        quad::QuadTerm *arg = args[i];
        if (arg == nullptr) {
            continue;
        }

        tree::Temp *src = nullptr;
        if (arg->kind == quad::QuadTermKind::TEMP) {
            src = makeTemp(arg->get_temp()->temp->num);
        } else if (arg->kind == quad::QuadTermKind::CONST) {
            src = materializeConstMovw(ctx, bundle, arg->get_const());
        }

        if (src != nullptr) {
            appendOper(
                bundle,
                std::string("mov ") + regNames[i] + ", `s0",
                {},
                {src}
            );
        }
    }
}

static void emitCallArgs(
    SelectionContext &ctx,
    StatementBundle &bundle,
    const std::vector<quad::QuadTerm *> &args
) {
    static const char *regNames[] = {"r0", "r1", "r2", "r3"};
    std::vector<std::pair<std::string, tree::Temp *>> pending;

    for (size_t i = 0; i < args.size() && i < 4; ++i) {
        quad::QuadTerm *arg = args[i];
        if (arg == nullptr) {
            continue;
        }

        if (arg->kind == quad::QuadTermKind::TEMP) {
            pending.push_back(
                {regNames[i], makeTemp(arg->get_temp()->temp->num)}
            );
        } else if (arg->kind == quad::QuadTermKind::CONST) {
            pending.push_back(
                {regNames[i], materializeConstMovw(ctx, bundle, arg->get_const())}
            );
        }
    }

    for (const auto &entry : pending) {
        appendOper(
            bundle,
            std::string("mov ") + entry.first + ", `s0",
            {},
            {entry.second}
        );
    }
}

static bool ptrCalcNeedsExplicitBundle(
    const advDFG &graph,
    const advDFGNode *node,
    const std::vector<const advDFGNode *> *nodes
) {
    if (node == nullptr || node->quadStatement == nullptr ||
        node->quadStatement->kind != quad::QuadKind::PTR_CALC ||
        node->chainDefined < 0) {
        return false;
    }

    if (chainUsedOnlyByLoad(graph, node->chainDefined)) {
        auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc *>(node->quadStatement);
        if (ptrCalc != nullptr) {
            if (ptrCalcConstOffset(ptrCalc) >= 8) {
                return true;
            }
            if (ptrCalcConstOffset(ptrCalc) == 0 && nodes != nullptr &&
                countCallsInBlock(*nodes) >= 2) {
                int loadIdx = -1;
                for (size_t i = 0; i < nodes->size(); ++i) {
                    if ((*nodes)[i] != nullptr &&
                        (*nodes)[i]->chainUsed == node->chainDefined &&
                        (*nodes)[i]->quadStatement != nullptr &&
                        (*nodes)[i]->quadStatement->kind == quad::QuadKind::LOAD) {
                        loadIdx = static_cast<int>(i);
                        break;
                    }
                }
                if (loadIdx >= 0) {
                    for (size_t i = 0; i < nodes->size(); ++i) {
                        if ((*nodes)[i] == nullptr ||
                            (*nodes)[i]->quadStatement == nullptr) {
                            continue;
                        }
                        auto kind = (*nodes)[i]->quadStatement->kind;
                        if (kind != quad::QuadKind::CALL &&
                            kind != quad::QuadKind::MOVE_CALL) {
                            continue;
                        }
                        if (static_cast<int>(i) < loadIdx) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc *>(node->quadStatement);
    if (ptrCalc == nullptr) {
        return false;
    }

    if (ptrCalcConstOffset(ptrCalc) < 0 && nodes != nullptr) {
        bool onlyStore = true;
        for (auto *chainUser : graph.getNodes()) {
            if (chainUser == nullptr ||
                chainUser->chainUsed != node->chainDefined ||
                chainUser->quadStatement == nullptr) {
                continue;
            }
            if (chainUser->quadStatement->kind != quad::QuadKind::STORE) {
                onlyStore = false;
                break;
            }
        }
        if (onlyStore) {
            int ptrCalcIndex = nodeIndexInBlock(*nodes, node);
            if (ptrCalcIndex >= 0) {
                for (auto *chainUser : graph.getNodes()) {
                    if (chainUser == nullptr ||
                        chainUser->chainUsed != node->chainDefined ||
                        chainUser->quadStatement == nullptr ||
                        chainUser->quadStatement->kind != quad::QuadKind::STORE) {
                        continue;
                    }
                    int storeIndex = nodeIndexInBlock(*nodes, chainUser);
                    if (storeIndex > ptrCalcIndex &&
                        hasInterleavedMulBetween(
                            *nodes,
                            static_cast<size_t>(ptrCalcIndex),
                            static_cast<size_t>(storeIndex))) {
                        return true;
                    }
                }
            }
            return false;
        }
    }

    for (auto *user : graph.getNodes()) {
        if (user == nullptr || user->chainUsed != node->chainDefined ||
            user->quadStatement == nullptr) {
            continue;
        }
        if (user->quadStatement->kind == quad::QuadKind::LOAD) {
            continue;
        }
        if (user->quadStatement->kind == quad::QuadKind::STORE) {
            auto *store = dynamic_cast<const quad::QuadStore *>(user->quadStatement);
            if (store == nullptr) {
                continue;
            }
            if (store->src->kind == quad::QuadTermKind::NAME &&
                ptrCalcConstOffset(ptrCalc) != 0) {
                if (hasTempStoreChainAtOffsetZero(
                        graph, ptrCalcBaseTempNum(ptrCalc))) {
                    return true;
                }
                continue;
            }
            if (store->src->kind == quad::QuadTermKind::TEMP &&
                ptrCalcConstOffset(ptrCalc) != 0 &&
                hasTempStoreChainAtOffsetZero(
                    graph, ptrCalcBaseTempNum(ptrCalc))) {
                return true;
            }
            if (storeNeedsExplicitChain(graph, store, ptrCalc)) {
                return true;
            }
            if (storeConstChainNeedsExplicitPtrCalc(ptrCalc)) {
                return true;
            }
        }
    }

    return false;
}

static StatementBundle buildBundle(
    SelectionContext &ctx,
    const advDFGNode *node,
    size_t quadIndex,
    const std::set<int> &definedTempsAtBuild
) {
    StatementBundle bundle;
    bundle.node = node;
    bundle.quadIndex = quadIndex;
    if (node == nullptr || node->quadStatement == nullptr) {
        return bundle;
    }

    const quad::QuadStm *stm = node->quadStatement;
    bundle.stm = stm;
    bundle.tempDefined = node->tempDefined;
    bundle.tempsUsed = node->tempsUsed;

    int chainUsed = node->chainUsed;

    switch (stm->kind) {
        case quad::QuadKind::LOAD: {
            auto *load = dynamic_cast<const quad::QuadLoad *>(stm);
            if (load == nullptr) {
                break;
            }
            if (chainUsed >= 0) {
                auto *chainNode = findChainDefNode(ctx.graph, chainUsed);
                auto *ptrCalc = chainNode == nullptr
                                    ? nullptr
                                    : dynamic_cast<const quad::QuadPtrCalc *>(
                                          chainNode->quadStatement);
                if (ptrCalc != nullptr) {
                    int ptrTempNum = ptrCalcDstTempNum(ptrCalc);
                    if (definedTempsAtBuild.count(ptrTempNum) > 0) {
                        emitLoadWithExplicitPtr(
                            bundle,
                            load->dst->temp->num,
                            ptrTempNum
                        );
                    } else {
                        emitLoadWithChain(bundle, load->dst->temp->num, ptrCalc);
                    }
                    return bundle;
                }
            }
            if (load->src->kind == quad::QuadTermKind::TEMP) {
                tree::Temp *dst = makeTemp(load->dst->temp->num);
                tree::Temp *src = makeTemp(load->src->get_temp()->temp->num);
                appendOper(bundle, "ldr `d0, [`s0]", {dst}, {src});
            }
            return bundle;
        }
        case quad::QuadKind::STORE: {
            auto *store = dynamic_cast<const quad::QuadStore *>(stm);
            if (store == nullptr) {
                break;
            }
            if (chainUsed >= 0) {
                auto *chainNode = findChainDefNode(ctx.graph, chainUsed);
                auto *ptrCalc = chainNode == nullptr
                                    ? nullptr
                                    : dynamic_cast<const quad::QuadPtrCalc *>(
                                          chainNode->quadStatement);
                if (ptrCalc != nullptr) {
                    bool ptrDefined =
                        definedTempsAtBuild.count(ptrCalcDstTempNum(ptrCalc)) > 0;
                    emitStoreWithChain(
                        ctx,
                        bundle,
                        store,
                        ptrCalc,
                        ptrDefined
                    );
                    return bundle;
                }
            }
            emitStoreDirect(ctx, bundle, store);
            return bundle;
        }
        case quad::QuadKind::PTR_CALC: {
            auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc *>(stm);
            if (ptrCalc == nullptr || !ptrCalcNeedsExplicitBundle(
                    ctx.graph, node, ctx.blockNodes)) {
                return bundle;
            }
            emitPtrCalcBundle(bundle, ptrCalc);
            bundle.isPtrCalcBundle = true;
            return bundle;
        }
        case quad::QuadKind::MOVE: {
            auto *move = dynamic_cast<const quad::QuadMove *>(stm);
            if (move == nullptr) {
                break;
            }
            tree::Temp *dst = makeTemp(move->dst->temp->num);
            if (move->src->kind == quad::QuadTermKind::TEMP) {
                tree::Temp *src = makeTemp(move->src->get_temp()->temp->num);
                appendMove(bundle, "mov `d0, `s0", {dst}, {src});
            } else if (move->src->kind == quad::QuadTermKind::CONST) {
                int value = move->src->get_const();
                if (fitsMovImmediate(value)) {
                    appendOper(
                        bundle,
                        "mov `d0, #" + std::to_string(value),
                        {dst},
                        {}
                    );
                } else {
                    tree::Temp *src = materializeConstMovw(ctx, bundle, value);
                    appendMove(bundle, "mov `d0, `s0", {dst}, {src});
                }
            }
            return bundle;
        }
        case quad::QuadKind::MOVE_BINOP: {
            auto *binop = dynamic_cast<const quad::QuadMoveBinop *>(stm);
            if (binop == nullptr) {
                break;
            }

            tree::Temp *dst = makeTemp(binop->dst->temp->num);
            int leftNum = -1;
            int leftConst = -1;
            int rightConst = -1;
            int rightNum = -1;

            if (binop->left->kind == quad::QuadTermKind::TEMP) {
                leftNum = binop->left->get_temp()->temp->num;
            } else if (binop->left->kind == quad::QuadTermKind::CONST) {
                leftConst = binop->left->get_const();
            }
            if (binop->right->kind == quad::QuadTermKind::CONST) {
                rightConst = binop->right->get_const();
            } else if (binop->right->kind == quad::QuadTermKind::TEMP) {
                rightNum = binop->right->get_temp()->temp->num;
            }

            if (binop->binop == "+" && rightConst >= 0 && leftNum >= 0) {
                appendOper(
                    bundle,
                    "add `d0, `s0, #" + std::to_string(rightConst),
                    {dst},
                    {makeTemp(leftNum)}
                );
            } else if (binop->binop == "-" && rightConst >= 0 && leftNum >= 0) {
                appendOper(
                    bundle,
                    "sub `d0, `s0, #" + std::to_string(rightConst),
                    {dst},
                    {makeTemp(leftNum)}
                );
            } else if (binop->binop == "-" && leftConst >= 0 && rightConst >= 0) {
                tree::Temp *leftTemp = materializeConstMovw(ctx, bundle, leftConst);
                tree::Temp *rightTemp = materializeConstMovw(ctx, bundle, rightConst);
                appendOper(
                    bundle,
                    "sub `d0, `s0, `s1",
                    {dst},
                    {leftTemp, rightTemp}
                );
            } else if (binop->binop == "*" && rightConst >= 0 && leftNum >= 0) {
                tree::Temp *constTemp = materializeConstMovw(ctx, bundle, rightConst);
                appendOper(
                    bundle,
                    "mul `d0, `s0, `s1",
                    {dst},
                    {makeTemp(leftNum), constTemp}
                );
            } else if (binop->binop == "*" && leftConst >= 0 && rightNum >= 0) {
                tree::Temp *constTemp = materializeConstMovw(ctx, bundle, leftConst);
                appendOper(
                    bundle,
                    "mul `d0, `s0, `s1",
                    {dst},
                    {constTemp, makeTemp(rightNum)}
                );
            } else if (binop->binop == "*" && rightNum >= 0 && leftNum >= 0) {
                appendOper(
                    bundle,
                    "mul `d0, `s0, `s1",
                    {dst},
                    {makeTemp(leftNum), makeTemp(rightNum)}
                );
            } else if (binop->binop == "+" && rightNum >= 0 && leftNum >= 0) {
                appendOper(
                    bundle,
                    "add `d0, `s0, `s1",
                    {dst},
                    {makeTemp(leftNum), makeTemp(rightNum)}
                );
            } else if (binop->binop == "-" && rightNum >= 0 && leftNum >= 0) {
                appendOper(
                    bundle,
                    "sub `d0, `s0, `s1",
                    {dst},
                    {makeTemp(leftNum), makeTemp(rightNum)}
                );
            }
            return bundle;
        }
        case quad::QuadKind::MOVE_EXTCALL: {
            auto *moveExt = dynamic_cast<const quad::QuadMoveExtCall *>(stm);
            if (moveExt == nullptr || moveExt->extcall == nullptr) {
                break;
            }
            if (moveExt->extcall->args != nullptr) {
                emitExtCallArgs(ctx, bundle, *moveExt->extcall->args);
            }
            appendOper(bundle, "bl " + moveExt->extcall->extfun, {}, {});
            tree::Temp *dst = makeTemp(moveExt->dst->temp->num);
            appendOper(bundle, "mov `d0, r0", {dst}, {});
            return bundle;
        }
        case quad::QuadKind::EXTCALL: {
            auto *ext = dynamic_cast<const quad::QuadExtCall *>(stm);
            if (ext == nullptr) {
                break;
            }
            if (ext->args != nullptr) {
                emitExtCallArgs(ctx, bundle, *ext->args);
            }
            appendOper(bundle, "bl " + ext->extfun, {}, {});
            return bundle;
        }
        case quad::QuadKind::MOVE_CALL: {
            auto *moveCall = dynamic_cast<const quad::QuadMoveCall *>(stm);
            if (moveCall == nullptr || moveCall->call == nullptr) {
                break;
            }
            auto *call = moveCall->call;
            if (call->args != nullptr) {
                emitCallArgs(ctx, bundle, *call->args);
            }
            if (call->obj_term != nullptr &&
                call->obj_term->kind == quad::QuadTermKind::TEMP) {
                tree::Temp *target =
                    makeTemp(call->obj_term->get_temp()->temp->num);
                appendOper(bundle, "blx `s0", {}, {target});
            } else {
                appendOper(bundle, "bl " + call->name, {}, {});
            }
            tree::Temp *dst = makeTemp(moveCall->dst->temp->num);
            appendOper(bundle, "mov `d0, r0", {dst}, {});
            return bundle;
        }
        case quad::QuadKind::CALL: {
            auto *call = dynamic_cast<const quad::QuadCall *>(stm);
            if (call == nullptr) {
                break;
            }
            if (call->args != nullptr) {
                emitCallArgs(ctx, bundle, *call->args);
            }
            if (call->obj_term != nullptr &&
                call->obj_term->kind == quad::QuadTermKind::TEMP) {
                tree::Temp *target =
                    makeTemp(call->obj_term->get_temp()->temp->num);
                appendOper(bundle, "blx `s0", {}, {target});
            } else {
                appendOper(bundle, "bl " + call->name, {}, {});
            }
            return bundle;
        }
        default:
            break;
    }

    return bundle;
}

static bool moveCopiesChainBase(
    const advDFG &graph,
    const quad::QuadMove *move
);

static bool tempIsChainCopyBase(
    const advDFG &graph,
    int tempNum
) {
    if (tempNum < 0) {
        return false;
    }
    for (auto *node : graph.getNodes()) {
        if (node == nullptr || node->quadStatement == nullptr ||
            node->quadStatement->kind != quad::QuadKind::MOVE) {
            continue;
        }
        auto *move = dynamic_cast<const quad::QuadMove *>(node->quadStatement);
        if (move == nullptr || move->dst == nullptr ||
            move->dst->temp->num != tempNum) {
            continue;
        }
        if (moveCopiesChainBase(graph, move)) {
            return true;
        }
    }
    return false;
}

static int ptrCalcSchedulePriority(
    const advDFG &graph,
    const advDFGNode *node,
    const std::vector<const advDFGNode *> &nodes
) {
    if (node == nullptr || node->quadStatement == nullptr ||
        node->quadStatement->kind != quad::QuadKind::PTR_CALC) {
        return 3;
    }
    auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc *>(node->quadStatement);
    if (ptrCalc == nullptr) {
        return 3;
    }
    if (tempIsChainCopyBase(graph, ptrCalcBaseTempNum(ptrCalc))) {
        return 2;
    }
    int off = ptrCalcConstOffset(ptrCalc);
    int baseNum = ptrCalcBaseTempNum(ptrCalc);
    if (off == 0 && !tempDefinedInBlock(nodes, baseNum) &&
        blockHasMulBinop(nodes)) {
        return 1;
    }
    for (auto *user : graph.getNodes()) {
        if (user == nullptr || user->chainUsed != node->chainDefined ||
            user->quadStatement == nullptr ||
            user->quadStatement->kind != quad::QuadKind::STORE) {
            continue;
        }
        auto *store = dynamic_cast<const quad::QuadStore *>(user->quadStatement);
        if (store == nullptr) {
            continue;
        }
        if (store->src->kind == quad::QuadTermKind::TEMP) {
            if (ptrCalcConstOffset(ptrCalc) == 0) {
                return 2;
            }
            if (hasTempStoreChainAtOffsetZero(
                    graph, ptrCalcBaseTempNum(ptrCalc))) {
                return 2;
            }
            continue;
        }
        if (store->src->kind == quad::QuadTermKind::NAME &&
            hasTempStoreChainAtOffsetZero(graph, ptrCalcBaseTempNum(ptrCalc))) {
            return 2;
        }
    }
    return 3;
}

static bool mulBinopScheduledInBlock(
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled
) {
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (!scheduled[i] || nodes[i] == nullptr ||
            nodes[i]->quadStatement == nullptr) {
            continue;
        }
        if (nodes[i]->quadStatement->kind != quad::QuadKind::MOVE_BINOP) {
            continue;
        }
        auto *binop = dynamic_cast<const quad::QuadMoveBinop *>(nodes[i]->quadStatement);
        if (binop != nullptr && binop->binop == "*") {
            return true;
        }
    }
    return false;
}

static bool nodePreparesUpcomingCallOperand(
    const std::vector<const advDFGNode *> &nodes,
    size_t nodeIndex,
    size_t callIndex
) {
    if (callIndex >= nodes.size() || nodeIndex >= callIndex ||
        nodes[callIndex] == nullptr ||
        nodes[callIndex]->quadStatement == nullptr) {
        return false;
    }
    const quad::QuadStm *callStm = nodes[callIndex]->quadStatement;
    const quad::QuadCall *call = nullptr;
    if (callStm->kind == quad::QuadKind::CALL) {
        call = dynamic_cast<const quad::QuadCall *>(callStm);
    } else if (callStm->kind == quad::QuadKind::MOVE_CALL) {
        auto *moveCall = dynamic_cast<const quad::QuadMoveCall *>(callStm);
        call = moveCall == nullptr ? nullptr : moveCall->call;
    }
    if (call == nullptr || call->args == nullptr) {
        return false;
    }

    auto *nodeStm = nodes[nodeIndex]->quadStatement;
    if (nodeStm == nullptr) {
        return false;
    }

    if (nodeStm->kind == quad::QuadKind::MOVE_BINOP) {
        auto *binop = dynamic_cast<const quad::QuadMoveBinop *>(nodeStm);
        if (binop == nullptr || binop->dst == nullptr) {
            return false;
        }
        int dstNum = binop->dst->temp->num;
        for (auto *arg : *call->args) {
            if (arg != nullptr && arg->kind == quad::QuadTermKind::TEMP &&
                arg->get_temp()->temp->num == dstNum) {
                return true;
            }
        }
        return false;
    }

    if (nodeStm->kind == quad::QuadKind::LOAD) {
        auto *load = dynamic_cast<const quad::QuadLoad *>(nodeStm);
        if (load == nullptr || load->dst == nullptr) {
            return false;
        }
        int dstNum = load->dst->temp->num;
        for (auto *arg : *call->args) {
            if (arg != nullptr && arg->kind == quad::QuadTermKind::TEMP &&
                arg->get_temp()->temp->num == dstNum) {
                return true;
            }
        }
        return false;
    }

    if (nodeStm->kind == quad::QuadKind::MOVE) {
        auto *move = dynamic_cast<const quad::QuadMove *>(nodeStm);
        if (move == nullptr || move->dst == nullptr) {
            return false;
        }
        int dstNum = move->dst->temp->num;
        if (call->obj_term != nullptr &&
            call->obj_term->kind == quad::QuadTermKind::TEMP &&
            call->obj_term->get_temp()->temp->num == dstNum) {
            return false;
        }
        for (auto *arg : *call->args) {
            if (arg != nullptr && arg->kind == quad::QuadTermKind::TEMP &&
                arg->get_temp()->temp->num == dstNum) {
                return true;
            }
        }
    }
    return false;
}

static bool loadFedCallObjectShouldDefer(
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled,
    size_t moveIndex,
    const std::set<int> &definedTemps
) {
    auto *move = dynamic_cast<const quad::QuadMove *>(
        nodes[moveIndex]->quadStatement);
    if (move == nullptr || move->dst == nullptr) {
        return false;
    }
    int callIdx = nextCallUsingObjectAfter(
        nodes, move->dst->temp->num, moveIndex);
    if (callIdx <= static_cast<int>(moveIndex) ||
        nodes[static_cast<size_t>(callIdx)]->quadStatement == nullptr) {
        return false;
    }
    const quad::QuadKind callKind =
        nodes[static_cast<size_t>(callIdx)]->quadStatement->kind;
    if (callKind != quad::QuadKind::CALL &&
        callKind != quad::QuadKind::MOVE_CALL) {
        return false;
    }

    for (size_t j = moveIndex + 1; j < static_cast<size_t>(callIdx); ++j) {
        if (scheduled[j] || !nodePreparesUpcomingCallOperand(nodes, j, callIdx)) {
            continue;
        }
        return true;
    }

    int loadIdx = loadDefiningTempBefore(
        nodes, move->src->get_temp()->temp->num, moveIndex);
    if (loadIdx >= 0 && countCallsInBlock(nodes) >= 2) {
        for (size_t j = 0; j < nodes.size(); ++j) {
            if (scheduled[j] || j == moveIndex) {
                continue;
            }
            if (isSubConstBinop(nodes[j]->quadStatement)) {
                return true;
            }
            if (nodes[j]->quadStatement != nullptr &&
                nodes[j]->quadStatement->kind == quad::QuadKind::MOVE) {
                auto *otherMove = dynamic_cast<const quad::QuadMove *>(
                    nodes[j]->quadStatement);
                if (otherMove != nullptr && otherMove->src != nullptr &&
                    otherMove->src->kind == quad::QuadTermKind::TEMP) {
                    int otherLoadIdx = loadDefiningTempBefore(
                        nodes, otherMove->src->get_temp()->temp->num, j);
                    if (otherLoadIdx < 0) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

static bool moveDstIsDirectStorePointer(
    const advDFG &graph,
    int dstTempNum
) {
    if (dstTempNum < 0) {
        return false;
    }
    for (auto *node : graph.getNodes()) {
        if (node == nullptr || node->quadStatement == nullptr ||
            node->quadStatement->kind != quad::QuadKind::STORE ||
            node->chainUsed >= 0) {
            continue;
        }
        auto *store = dynamic_cast<const quad::QuadStore *>(node->quadStatement);
        if (store != nullptr && store->dst != nullptr &&
            store->dst->kind == quad::QuadTermKind::TEMP &&
            store->dst->get_temp()->temp->num == dstTempNum) {
            return true;
        }
    }
    return false;
}

static int moveTempSchedulePriority(
    const advDFG &graph,
    const quad::QuadMove *move,
    size_t quadIndex,
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled
) {
    if (move == nullptr || move->src->kind != quad::QuadTermKind::TEMP) {
        return 3;
    }
    int srcNum = move->src->get_temp()->temp->num;
    int dstNum = move->dst->temp->num;

    if (moveDstIsDirectStorePointer(graph, dstNum) &&
        !tempDefinedInBlock(nodes, srcNum) &&
        blockHasMulBinop(nodes)) {
        return 1;
    }

    int loadIdx = loadDefiningTempBefore(nodes, srcNum, quadIndex);
    if (loadIdx >= 0 && loadIdx + 1 == static_cast<int>(quadIndex)) {
        int callIdx = nextCallUsingObjectAfter(nodes, dstNum, quadIndex);
        if (callIdx >= 0 &&
            !hasUnscheduledMoveExtCallBetween(
                nodes,
                scheduled,
                quadIndex,
                static_cast<size_t>(callIdx))) {
            return 2;
        }
    }

    if (!tempDefinedInBlock(nodes, srcNum) && blockHasMulBinop(nodes)) {
        return 2;
    }

    if (movePreparesDynamicPtrCalcBase(graph, dstNum)) {
        return 2;
    }

    if (tempIsChainCopyBase(graph, srcNum)) {
        return 2;
    }

    if (quadIndex > 0 && scheduled[quadIndex - 1] &&
        nodes[quadIndex - 1] != nullptr &&
        nodes[quadIndex - 1]->quadStatement != nullptr &&
        nodes[quadIndex - 1]->quadStatement->kind == quad::QuadKind::PTR_CALC) {
        auto *prevPtrCalc = dynamic_cast<const quad::QuadPtrCalc *>(
            nodes[quadIndex - 1]->quadStatement);
        if (prevPtrCalc != nullptr &&
            ptrCalcBaseTempNum(prevPtrCalc) == srcNum &&
            ptrCalcSchedulePriority(graph, nodes[quadIndex - 1], nodes) <= 2) {
            return 1;
        }
    }
    for (auto *node : graph.getNodes()) {
        if (node == nullptr || node->quadStatement == nullptr ||
            node->quadStatement->kind != quad::QuadKind::PTR_CALC ||
            node->chainDefined < 0) {
            continue;
        }
        auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc *>(node->quadStatement);
        if (ptrCalc == nullptr || ptrCalcBaseTempNum(ptrCalc) != srcNum) {
            continue;
        }
        for (auto *user : graph.getNodes()) {
            if (user == nullptr || user->chainUsed != node->chainDefined ||
                user->quadStatement == nullptr ||
                user->quadStatement->kind != quad::QuadKind::STORE) {
                continue;
            }
            auto *store = dynamic_cast<const quad::QuadStore *>(user->quadStatement);
            if (store != nullptr &&
                store->src->kind == quad::QuadTermKind::NAME &&
                hasTempStoreChainAtOffsetZero(graph, ptrCalcBaseTempNum(ptrCalc))) {
                return 2;
            }
        }
        for (auto *user : graph.getNodes()) {
            if (user == nullptr || user->chainUsed != node->chainDefined ||
                user->quadStatement == nullptr ||
                user->quadStatement->kind != quad::QuadKind::STORE) {
                continue;
            }
            auto *store = dynamic_cast<const quad::QuadStore *>(user->quadStatement);
            if (store != nullptr && store->src->kind == quad::QuadTermKind::TEMP &&
                ptrCalcConstOffset(ptrCalc) == 0) {
                return 2;
            }
        }
    }
    return 3;
}

static bool isSubConstBinop(const quad::QuadStm *stm) {
    if (stm == nullptr || stm->kind != quad::QuadKind::MOVE_BINOP) {
        return false;
    }
    auto *binop = dynamic_cast<const quad::QuadMoveBinop *>(stm);
    return binop != nullptr && binop->binop == "-" &&
           binop->right->kind == quad::QuadTermKind::CONST;
}

static bool isMulBinopStmt(const quad::QuadStm *stm) {
    if (stm == nullptr || stm->kind != quad::QuadKind::MOVE_BINOP) {
        return false;
    }
    auto *binop = dynamic_cast<const quad::QuadMoveBinop *>(stm);
    return binop != nullptr && binop->binop == "*";
}

static bool isAddConstBinop(const quad::QuadStm *stm) {
    if (stm == nullptr || stm->kind != quad::QuadKind::MOVE_BINOP) {
        return false;
    }
    auto *binop = dynamic_cast<const quad::QuadMoveBinop *>(stm);
    return binop != nullptr && binop->binop == "+" &&
           binop->right->kind == quad::QuadTermKind::CONST;
}

static bool isAddBinop(const quad::QuadStm *stm) {
    if (stm == nullptr || stm->kind != quad::QuadKind::MOVE_BINOP) {
        return false;
    }
    auto *binop = dynamic_cast<const quad::QuadMoveBinop *>(stm);
    return binop != nullptr && binop->binop == "+";
}

static bool hasUnscheduledMulBefore(
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled,
    size_t beforeIndex
) {
    for (size_t i = 0; i < beforeIndex && i < nodes.size(); ++i) {
        if (scheduled[i] || nodes[i] == nullptr ||
            nodes[i]->quadStatement == nullptr) {
            continue;
        }
        if (isMulBinopStmt(nodes[i]->quadStatement)) {
            return true;
        }
    }
    return false;
}

static bool hasUnscheduledMulInBlock(
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled
) {
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (scheduled[i] || nodes[i] == nullptr ||
            nodes[i]->quadStatement == nullptr) {
            continue;
        }
        if (isMulBinopStmt(nodes[i]->quadStatement)) {
            return true;
        }
    }
    return false;
}

static bool hasUnscheduledMulAfter(
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled,
    size_t afterIndex
) {
    for (size_t i = afterIndex + 1; i < nodes.size(); ++i) {
        if (scheduled[i] || nodes[i] == nullptr ||
            nodes[i]->quadStatement == nullptr) {
            continue;
        }
        if (isMulBinopStmt(nodes[i]->quadStatement)) {
            return true;
        }
    }
    return false;
}

static bool blockHasUnscheduledCallArgMoveBefore(
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled,
    size_t beforeIndex,
    int exceptTempNum
) {
    for (size_t i = 0; i < beforeIndex && i < nodes.size(); ++i) {
        if (scheduled[i] || nodes[i] == nullptr ||
            nodes[i]->quadStatement == nullptr ||
            nodes[i]->quadStatement->kind != quad::QuadKind::MOVE) {
            continue;
        }
        auto *move = dynamic_cast<const quad::QuadMove *>(nodes[i]->quadStatement);
        if (move == nullptr || move->dst == nullptr ||
            move->dst->temp->num == exceptTempNum) {
            continue;
        }
        if (moveUsedAsCallArgumentAfter(
                nodes, move->dst->temp->num, i)) {
            return true;
        }
    }
    return false;
}

static int firstCallIndexInBlock(
    const std::vector<const advDFGNode *> &nodes
) {
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i] == nullptr || nodes[i]->quadStatement == nullptr) {
            continue;
        }
        auto kind = nodes[i]->quadStatement->kind;
        if (kind == quad::QuadKind::CALL ||
            kind == quad::QuadKind::MOVE_CALL) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

static int schedulePriority(
    const advDFG &graph,
    const StatementBundle &bundle,
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled
);

static bool storeShouldRunBeforePtrCalc(
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled,
    size_t storeIndex,
    size_t ptrCalcIndex,
    const std::set<int> &definedTemps
) {
    if (storeIndex >= ptrCalcIndex ||
        nodes[storeIndex] == nullptr ||
        nodes[ptrCalcIndex] == nullptr ||
        nodes[storeIndex]->quadStatement == nullptr ||
        nodes[storeIndex]->quadStatement->kind != quad::QuadKind::STORE ||
        nodes[ptrCalcIndex]->quadStatement == nullptr ||
        nodes[ptrCalcIndex]->quadStatement->kind != quad::QuadKind::PTR_CALC) {
        return false;
    }

    if (nodes[storeIndex]->chainUsed >= 0) {
        return false;
    }

    for (int temp : nodes[storeIndex]->tempsUsed) {
        if (definedTemps.count(temp) == 0) {
            return false;
        }
    }

    auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc *>(
        nodes[ptrCalcIndex]->quadStatement);
    if (ptrCalc == nullptr) {
        return false;
    }
    int ptrDst = ptrCalcDstTempNum(ptrCalc);
    if (nodes[storeIndex]->tempsUsed.count(ptrDst) > 0) {
        return false;
    }

    return true;
}

static bool moveCopiesChainBase(
    const advDFG &graph,
    const quad::QuadMove *move
) {
    if (move == nullptr || move->src == nullptr ||
        move->src->kind != quad::QuadTermKind::TEMP) {
        return false;
    }
    return hasTempStoreChainAtOffsetZero(
        graph, move->src->get_temp()->temp->num);
}

static int chainBaseCompanionStoreIndex(
    const advDFG &graph,
    const std::vector<const advDFGNode *> &nodes,
    int baseTempNum
) {
    for (auto *node : graph.getNodes()) {
        if (node == nullptr || node->quadStatement == nullptr ||
            node->quadStatement->kind != quad::QuadKind::PTR_CALC ||
            node->chainDefined < 0) {
            continue;
        }
        auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc *>(node->quadStatement);
        if (ptrCalc == nullptr || ptrCalcConstOffset(ptrCalc) != 0 ||
            ptrCalcBaseTempNum(ptrCalc) != baseTempNum) {
            continue;
        }
        return companionTempStoreNodeIndex(graph, nodes, node);
    }
    return -1;
}

static int storeOffsetOnBase(
    const advDFG &graph,
    const advDFGNode *storeNode
) {
    if (storeNode == nullptr || storeNode->quadStatement == nullptr ||
        storeNode->quadStatement->kind != quad::QuadKind::STORE) {
        return -1;
    }
    if (storeNode->chainUsed >= 0) {
        auto *chainNode = findChainDefNode(graph, storeNode->chainUsed);
        auto *ptrCalc = chainNode == nullptr
                            ? nullptr
                            : dynamic_cast<const quad::QuadPtrCalc *>(
                                  chainNode->quadStatement);
        if (ptrCalc == nullptr) {
            return -1;
        }
        return ptrCalcConstOffset(ptrCalc);
    }
    return 0;
}

static bool ptrCalcHasPendingInterleavedMul(
    const advDFG &graph,
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled,
    size_t ptrCalcIndex
) {
    if (nodes[ptrCalcIndex] == nullptr ||
        nodes[ptrCalcIndex]->quadStatement == nullptr ||
        nodes[ptrCalcIndex]->quadStatement->kind != quad::QuadKind::PTR_CALC) {
        return false;
    }
    auto *ptrCalcNode = dynamic_cast<const quad::QuadPtrCalc *>(
        nodes[ptrCalcIndex]->quadStatement);
    if (ptrCalcNode == nullptr ||
        ptrCalcConstOffset(ptrCalcNode) >= 0) {
        return false;
    }
    for (auto *user : graph.getNodes()) {
        if (user == nullptr ||
            user->chainUsed != nodes[ptrCalcIndex]->chainDefined ||
            user->quadStatement == nullptr ||
            user->quadStatement->kind != quad::QuadKind::STORE) {
            continue;
        }
        int storeIndex = nodeIndexInBlock(nodes, user);
        if (storeIndex <= static_cast<int>(ptrCalcIndex)) {
            continue;
        }
        for (size_t j = ptrCalcIndex + 1;
             j < static_cast<size_t>(storeIndex) && j < nodes.size(); ++j) {
            if (scheduled[j] || nodes[j] == nullptr ||
                nodes[j]->quadStatement == nullptr ||
                nodes[j]->quadStatement->kind != quad::QuadKind::MOVE_BINOP) {
                continue;
            }
            auto *binop = dynamic_cast<const quad::QuadMoveBinop *>(
                nodes[j]->quadStatement);
            if (binop != nullptr && binop->binop == "*") {
                return true;
            }
        }
    }
    return false;
}

static bool chainCopyBaseSetupShouldDefer(
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled,
    size_t index,
    const std::set<int> &definedTemps
) {
    (void)definedTemps;
    for (size_t extIdx = 0; extIdx < index && extIdx < nodes.size(); ++extIdx) {
        if (nodes[extIdx] == nullptr ||
            nodes[extIdx]->quadStatement == nullptr ||
            !isMoveExtCallStmt(nodes[extIdx]->quadStatement)) {
            continue;
        }
        if (!scheduled[extIdx]) {
            return true;
        }
        int allocTemp = nodes[extIdx]->tempDefined;
        if (allocTemp < 0) {
            continue;
        }
        for (size_t storeIdx = extIdx + 1;
             storeIdx < index && storeIdx < nodes.size(); ++storeIdx) {
            if (scheduled[storeIdx] || nodes[storeIdx] == nullptr ||
                nodes[storeIdx]->quadStatement == nullptr ||
                nodes[storeIdx]->quadStatement->kind != quad::QuadKind::STORE ||
                nodes[storeIdx]->chainUsed >= 0) {
                continue;
            }
            auto *store = dynamic_cast<const quad::QuadStore *>(
                nodes[storeIdx]->quadStatement);
            if (store == nullptr || store->dst == nullptr ||
                store->dst->kind != quad::QuadTermKind::TEMP ||
                store->dst->get_temp()->temp->num != allocTemp) {
                continue;
            }
            bool storeReady = true;
            for (int temp : nodes[storeIdx]->tempsUsed) {
                if (definedTemps.count(temp) == 0) {
                    storeReady = false;
                    break;
                }
            }
            if (storeReady) {
                return true;
            }
        }
    }
    return false;
}

static bool chainCopyBasePtrCalcShouldDefer(
    const advDFG &graph,
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled,
    size_t index,
    const std::set<int> &definedTemps
) {
    if (nodes[index] == nullptr ||
        nodes[index]->quadStatement == nullptr ||
        nodes[index]->quadStatement->kind != quad::QuadKind::PTR_CALC) {
        return false;
    }
    auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc *>(
        nodes[index]->quadStatement);
    if (ptrCalc == nullptr ||
        !tempIsChainCopyBase(graph, ptrCalcBaseTempNum(ptrCalc))) {
        return false;
    }
    return chainCopyBaseSetupShouldDefer(
        nodes, scheduled, index, definedTemps);
}

static bool ptrCalcHasUnscheduledInterleavedMulBeforeStore(
    const advDFG &graph,
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled,
    size_t ptrCalcIndex
) {
    if (ptrCalcIndex >= nodes.size() || nodes[ptrCalcIndex] == nullptr) {
        return false;
    }
    if (!ptrCalcNeedsExplicitBundle(
            graph, nodes[ptrCalcIndex], &nodes)) {
        return false;
    }
    auto *ptrCalc = dynamic_cast<const quad::QuadPtrCalc *>(
        nodes[ptrCalcIndex]->quadStatement);
    if (ptrCalc == nullptr || ptrCalcConstOffset(ptrCalc) >= 0) {
        return false;
    }
    for (auto *user : graph.getNodes()) {
        if (user == nullptr ||
            user->chainUsed != nodes[ptrCalcIndex]->chainDefined ||
            user->quadStatement == nullptr ||
            user->quadStatement->kind != quad::QuadKind::STORE) {
            continue;
        }
        int storeIndex = nodeIndexInBlock(nodes, user);
        if (storeIndex <= static_cast<int>(ptrCalcIndex) ||
            !hasInterleavedMulBetween(
                nodes,
                ptrCalcIndex,
                static_cast<size_t>(storeIndex))) {
            continue;
        }
        for (size_t i = ptrCalcIndex + 1;
             i < static_cast<size_t>(storeIndex) && i < nodes.size(); ++i) {
            if (scheduled[i] || nodes[i] == nullptr ||
                nodes[i]->quadStatement == nullptr) {
                continue;
            }
            if (isMulBinopStmt(nodes[i]->quadStatement)) {
                return true;
            }
        }
    }
    return false;
}

static int effectiveSchedulePriority(
    const advDFG &graph,
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled,
    size_t index,
    const std::set<int> &definedTemps
) {
    StatementBundle probe;
    probe.stm = nodes[index]->quadStatement;
    probe.node = nodes[index];
    probe.quadIndex = index;
    int priority = schedulePriority(graph, probe, nodes, scheduled);

    if (priority == 0) {
        return priority;
    }

    if (nodes[index]->quadStatement != nullptr &&
        nodes[index]->quadStatement->kind == quad::QuadKind::LOAD &&
        nodes[index]->chainUsed >= 0) {
        auto *chainNode = findChainDefNode(graph, nodes[index]->chainUsed);
        if (chainNode != nullptr &&
            ptrCalcNeedsExplicitBundle(graph, chainNode, &nodes) &&
            countCallsInBlock(nodes) >= 2) {
            int firstCallIdx = firstCallIndexInBlock(nodes);
            if (firstCallIdx >= 0 &&
                !scheduled[static_cast<size_t>(firstCallIdx)]) {
                return 4;
            }
        }
    }

    if (nodes[index]->quadStatement != nullptr &&
        nodes[index]->quadStatement->kind == quad::QuadKind::PTR_CALC &&
        ptrCalcHasUnscheduledInterleavedMulBeforeStore(
            graph, nodes, scheduled, index)) {
        return 4;
    }

    if (priority <= 2 && nodes[index]->quadStatement != nullptr &&
        nodes[index]->quadStatement->kind == quad::QuadKind::MOVE) {
        auto *move = dynamic_cast<const quad::QuadMove *>(nodes[index]->quadStatement);
        if (move != nullptr && move->src != nullptr &&
            move->src->kind == quad::QuadTermKind::TEMP) {
            int loadIdx = loadDefiningTempBefore(
                nodes, move->src->get_temp()->temp->num, index);
            if (loadIdx >= 0 && loadIdx + 1 == static_cast<int>(index) &&
                loadFedCallObjectShouldDefer(
                    nodes, scheduled, index, definedTemps)) {
                return 3;
            }
        }
        if (move != nullptr && move->src != nullptr &&
            move->src->kind == quad::QuadTermKind::TEMP &&
            tempIsChainCopyBase(graph, move->src->get_temp()->temp->num) &&
            chainCopyBaseSetupShouldDefer(
                nodes, scheduled, index, definedTemps)) {
            return 3;
        }
        if (move != nullptr && move->dst != nullptr) {
            int callIdx = nextCallUsingObjectAfter(
                nodes, move->dst->temp->num, index);
            if (callIdx > static_cast<int>(index) &&
                hasUnscheduledMoveExtCallBetween(
                    nodes,
                    scheduled,
                    index,
                    static_cast<size_t>(callIdx))) {
                return 3;
            }
        }
    }

    if (priority <= 1) {
        return priority;
    }

    if (nodes[index]->quadStatement != nullptr &&
        nodes[index]->quadStatement->kind == quad::QuadKind::PTR_CALC &&
        chainCopyBasePtrCalcShouldDefer(
            graph, nodes, scheduled, index, definedTemps)) {
        return 3;
    }

    if (nodes[index]->quadStatement != nullptr &&
        nodes[index]->quadStatement->kind == quad::QuadKind::PTR_CALC &&
        ptrCalcHasPendingInterleavedMul(
            graph, nodes, scheduled, index)) {
        return 3;
    }

    if (priority == 2 && nodes[index]->quadStatement != nullptr) {
        if (nodes[index]->quadStatement->kind == quad::QuadKind::PTR_CALC ||
            nodes[index]->quadStatement->kind == quad::QuadKind::MOVE) {
            for (size_t j = 0; j < nodes.size(); ++j) {
                if (scheduled[j] || j == index) {
                    continue;
                }
                StatementBundle otherProbe;
                otherProbe.stm = nodes[j]->quadStatement;
                otherProbe.node = nodes[j];
                if (schedulePriority(graph, otherProbe, nodes, scheduled) != 3 ||
                    nodes[j]->quadStatement == nullptr ||
                    nodes[j]->quadStatement->kind != quad::QuadKind::STORE) {
                    continue;
                }
                if (nodes[index]->quadStatement->kind == quad::QuadKind::PTR_CALC &&
                    storeShouldRunBeforePtrCalc(
                        nodes, scheduled, j, index, definedTemps)) {
                    return 3;
                }
                if (nodes[index]->quadStatement->kind == quad::QuadKind::MOVE &&
                    j < index && nodes[j]->chainUsed < 0) {
                    bool storeReady = true;
                    for (int temp : nodes[j]->tempsUsed) {
                        if (definedTemps.count(temp) == 0) {
                            storeReady = false;
                            break;
                        }
                    }
                    if (storeReady) {
                        return 3;
                    }
                }
            }
        }
    }

    return priority;
}

static int schedulePriority(
    const advDFG &graph,
    const StatementBundle &bundle,
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled
) {
    if (bundle.stm == nullptr) {
        return 100;
    }

    switch (bundle.stm->kind) {
        case quad::QuadKind::MOVE: {
            auto *move = dynamic_cast<const quad::QuadMove *>(bundle.stm);
            if (move == nullptr) {
                return 3;
            }
            if (move->src->kind == quad::QuadTermKind::CONST) {
                return 0;
            }
            if (move->dst != nullptr &&
                move->src->kind == quad::QuadTermKind::TEMP) {
                bool callArg = moveUsedAsCallArgumentAfter(
                    nodes, move->dst->temp->num, bundle.quadIndex);
                bool pendingName = pendingNameStoreToMoveSrc(
                    graph, nodes, scheduled, move);
                if (callArg) {
                    if (pendingName) {
                        if (tempIsMallocResultInBlock(
                                nodes, move->src->get_temp()->temp->num)) {
                            int nameStoreIdx = firstPendingNameStoreIndex(
                                graph, nodes, move);
                            bool loadInBlock = moveDstUsedByUnscheduledLoadInBlock(
                                graph, nodes, scheduled,
                                move->dst->temp->num);
                            if (nameStoreIdx >= 0 &&
                                !scheduled[static_cast<size_t>(nameStoreIdx)] &&
                                loadInBlock &&
                                blockHasUnscheduledCallArgMoveBefore(
                                    nodes,
                                    scheduled,
                                    static_cast<size_t>(nameStoreIdx),
                                    move->dst->temp->num)) {
                                return 3;
                            }
                            if (!loadInBlock) {
                                return 0;
                            }
                        } else {
                            return 3;
                        }
                    } else if (allMallocExtCallsScheduledInBlock(nodes, scheduled) &&
                        !tempDefinedByMoveInBlock(
                            nodes, move->src->get_temp()->temp->num)) {
                        return 0;
                    }
                } else if (pendingName &&
                           tempIsMallocResultInBlock(
                               nodes, move->src->get_temp()->temp->num)) {
                    int nameStoreIdx = firstPendingNameStoreIndex(
                        graph, nodes, move);
                    bool loadInBlock = moveDstUsedByUnscheduledLoadInBlock(
                        graph, nodes, scheduled, move->dst->temp->num);
                    if (nameStoreIdx >= 0 &&
                        !scheduled[static_cast<size_t>(nameStoreIdx)] &&
                        loadInBlock &&
                        (blockHasUnscheduledCallArgMoveBefore(
                             nodes,
                             scheduled,
                             static_cast<size_t>(nameStoreIdx),
                             move->dst->temp->num) ||
                         (static_cast<int>(bundle.quadIndex) > nameStoreIdx &&
                          countMallocExtCallsInBlock(nodes) <= 1))) {
                        return 3;
                    }
                    if (!loadInBlock) {
                        return 0;
                    }
                }
            }
            return moveTempSchedulePriority(
                graph, move, bundle.quadIndex, nodes, scheduled);
        }
        case quad::QuadKind::PTR_CALC:
            return ptrCalcSchedulePriority(graph, bundle.node, nodes);
        case quad::QuadKind::LOAD: {
            if (bundle.node != nullptr && bundle.node->chainUsed >= 0) {
                auto *chainNode = findChainDefNode(
                    graph, bundle.node->chainUsed);
                auto *ptrCalc = chainNode == nullptr
                                    ? nullptr
                                    : dynamic_cast<const quad::QuadPtrCalc *>(
                                          chainNode->quadStatement);
                if (ptrCalc != nullptr &&
                    ptrCalcNeedsExplicitBundle(graph, chainNode, &nodes) &&
                    countCallsInBlock(nodes) >= 2) {
                    int firstCallIdx = firstCallIndexInBlock(nodes);
                    if (firstCallIdx >= 0 &&
                        !scheduled[static_cast<size_t>(firstCallIdx)]) {
                        return 4;
                    }
                    if (firstCallIdx >= 0 &&
                        scheduled[static_cast<size_t>(firstCallIdx)]) {
                        return 0;
                    }
                }
                if (ptrCalc != nullptr &&
                    ptrCalcConstOffset(ptrCalc) == 0 &&
                    !tempDefinedInBlock(
                        nodes, ptrCalcBaseTempNum(ptrCalc)) &&
                    blockHasMulBinop(nodes)) {
                    return 2;
                }
            }
            return 3;
        }
        case quad::QuadKind::MOVE_BINOP: {
            auto *binop = dynamic_cast<const quad::QuadMoveBinop *>(bundle.stm);
            if (binop == nullptr) {
                return 3;
            }
            if (binop->binop == "+" &&
                binop->right->kind == quad::QuadTermKind::CONST) {
                if (blockHasUnscheduledLoadFedCall(nodes, scheduled)) {
                    return 3;
                }
                return 1;
            }
            if (binop->binop == "+" &&
                binop->right->kind == quad::QuadTermKind::TEMP &&
                binop->left->kind == quad::QuadTermKind::TEMP &&
                hasUnscheduledMulBefore(
                    nodes, scheduled, bundle.quadIndex)) {
                return 4;
            }
            if (binop->binop == "+" &&
                hasUnscheduledMulAfter(
                    nodes, scheduled, bundle.quadIndex)) {
                return 4;
            }
            if (binop->binop == "+" &&
                binop->right->kind == quad::QuadTermKind::TEMP) {
                if (blockHasUnscheduledLoadFedCall(nodes, scheduled)) {
                    return 3;
                }
                return 1;
            }
            if (binop->binop == "-") {
                if (mulBinopScheduledInBlock(nodes, scheduled)) {
                    return 1;
                }
            }
            return 3;
        }
        default:
            return 3;
    }
}

static bool hasEarlierMoveExtCall(
    const std::vector<const advDFGNode *> &nodes,
    size_t index
) {
    for (size_t j = 0; j < index; ++j) {
        if (nodes[j] != nullptr && nodes[j]->quadStatement != nullptr &&
            isMoveExtCallStmt(nodes[j]->quadStatement)) {
            return true;
        }
    }
    return false;
}

static bool anyMoveExtCallScheduled(
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled
) {
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (!scheduled[i] || nodes[i] == nullptr ||
            nodes[i]->quadStatement == nullptr) {
            continue;
        }
        if (isMoveExtCallStmt(nodes[i]->quadStatement)) {
            return true;
        }
    }
    return false;
}

static bool moveExtCallBarrierAllows(
    size_t candidateIndex,
    int candidatePriority,
    const advDFG &graph,
    const std::vector<const advDFGNode *> &nodes,
    const std::vector<bool> &scheduled,
    const std::set<int> &definedTemps
) {
    if (candidatePriority == 0 &&
        candidateIndex == 0 &&
        isConstMoveStmt(nodes[candidateIndex]->quadStatement) &&
        candidateIndex + 1 < nodes.size() &&
        nodes[candidateIndex + 1] != nullptr &&
        nodes[candidateIndex + 1]->quadStatement != nullptr &&
        isExtCallLikeStmt(nodes[candidateIndex + 1]->quadStatement) &&
        !scheduled[candidateIndex + 1]) {
        return false;
    }

    if (candidatePriority == 0 &&
        hasEarlierMoveExtCall(nodes, candidateIndex) &&
        !anyMoveExtCallScheduled(nodes, scheduled)) {
        return false;
    }

    if (nodes[candidateIndex] != nullptr &&
        nodes[candidateIndex]->quadStatement != nullptr &&
        nodes[candidateIndex]->quadStatement->kind == quad::QuadKind::MOVE) {
        auto *move = dynamic_cast<const quad::QuadMove *>(
            nodes[candidateIndex]->quadStatement);
        if (move != nullptr && move->dst != nullptr) {
            int callIdx = nextCallUsingObjectAfter(
                nodes, move->dst->temp->num, candidateIndex);
            if (callIdx > static_cast<int>(candidateIndex) &&
                hasUnscheduledMoveExtCallBetween(
                    nodes,
                    scheduled,
                    candidateIndex,
                    static_cast<size_t>(callIdx))) {
                return false;
            }
        }
        if (move != nullptr && move->src != nullptr &&
            move->src->kind == quad::QuadTermKind::TEMP) {
            int loadIdx = loadDefiningTempBefore(
                nodes, move->src->get_temp()->temp->num, candidateIndex);
            if (loadIdx >= 0 &&
                loadIdx + 1 == static_cast<int>(candidateIndex) &&
                loadFedCallObjectShouldDefer(
                    nodes, scheduled, candidateIndex, definedTemps)) {
                return false;
            }
        }
        if (move != nullptr && moveCopiesChainBase(graph, move)) {
            int bound = chainBaseCompanionStoreIndex(
                graph,
                nodes,
                move->src->get_temp()->temp->num);
            if (bound >= 0 &&
                !allMoveExtCallsBeforeScheduled(
                    nodes, scheduled, static_cast<size_t>(bound))) {
                return false;
            }
            return true;
        }
    }

    if (candidatePriority >= 2 &&
        nodes[candidateIndex] != nullptr &&
        ptrCalcUsesCompanionChainBarrier(graph, nodes[candidateIndex])) {
        int bound = companionTempStoreNodeIndex(graph, nodes, nodes[candidateIndex]);
        if (bound >= 0 &&
            !allMoveExtCallsBeforeScheduled(nodes, scheduled,
                                            static_cast<size_t>(bound))) {
            return false;
        }
        return true;
    }

    if (nodes[candidateIndex] != nullptr &&
        nodes[candidateIndex]->quadStatement != nullptr &&
        nodes[candidateIndex]->quadStatement->kind == quad::QuadKind::LOAD &&
        nodes[candidateIndex]->chainUsed >= 0) {
        auto *chainNode = findChainDefNode(
            graph, nodes[candidateIndex]->chainUsed);
        if (chainNode != nullptr &&
            ptrCalcNeedsExplicitBundle(graph, chainNode, &nodes) &&
            countCallsInBlock(nodes) >= 2) {
            int firstCallIdx = firstCallIndexInBlock(nodes);
            if (firstCallIdx >= 0 &&
                !scheduled[static_cast<size_t>(firstCallIdx)]) {
                return false;
            }
        }
    }

    return true;
}

static std::vector<const advDFGNode *> statementNodesInOrder(
    const advDFG &graph
) {
    std::vector<const advDFGNode *> result;
    for (auto *node : graph.getNodes()) {
        if (node != nullptr && node->type == NodeType::Statement) {
            result.push_back(node);
        }
    }
    return result;
}

} // namespace

void selectInstructionsForBlock(
    const advDFGblock &blockGraph,
    preScheduleBlock &schedBlock,
    int &nextTempNum
) {
    if (blockGraph.graph.getNodes().empty()) {
        return;
    }

    SelectionContext ctx{nextTempNum, blockGraph.graph, {}};
    auto nodes = statementNodesInOrder(blockGraph.graph);
    ctx.blockNodes = &nodes;

    std::vector<bool> scheduled(nodes.size(), false);
    std::set<int> blockDefined;
    for (auto *node : nodes) {
        if (node != nullptr && node->tempDefined >= 0) {
            blockDefined.insert(node->tempDefined);
        }
    }

    std::set<int> definedTemps;
    for (auto *node : nodes) {
        if (node == nullptr) {
            continue;
        }
        for (int temp : node->tempsUsed) {
            if (blockDefined.count(temp) == 0) {
                definedTemps.insert(temp);
            }
        }
    }

    size_t scheduledCount = 0;
    while (scheduledCount < nodes.size()) {
        int bestIndex = -1;
        int bestPriority = 1000;
        size_t bestQuadIndex = 0;
        bool preferHighQuadIndex = false;

        for (size_t i = 0; i < nodes.size(); ++i) {
            if (scheduled[i] || nodes[i] == nullptr) {
                continue;
            }

            StatementBundle probe;
            probe.stm = nodes[i]->quadStatement;
            probe.node = nodes[i];
            probe.quadIndex = i;
            probe.isPtrCalcBundle =
                probe.stm != nullptr &&
                probe.stm->kind == quad::QuadKind::PTR_CALC &&
                ptrCalcNeedsExplicitBundle(blockGraph.graph, nodes[i], &nodes);

            std::set<int> probeUses = nodes[i]->tempsUsed;
            bool ready = true;
            for (int temp : probeUses) {
                if (definedTemps.count(temp) == 0) {
                    ready = false;
                    break;
                }
            }
            if (!ready) {
                continue;
            }

            int priority = effectiveSchedulePriority(
                blockGraph.graph, nodes, scheduled, i, definedTemps);
            if (!moveExtCallBarrierAllows(
                    i, priority, blockGraph.graph, nodes, scheduled, definedTemps)) {
                continue;
            }

            bool highQuad = probe.isPtrCalcBundle &&
                            ptrCalcSchedulePriority(blockGraph.graph, nodes[i], nodes) <= 1;
            size_t quadIndex = i;

            bool candidateSub = isSubConstBinop(probe.stm);
            bool bestSub = bestIndex >= 0 &&
                           isSubConstBinop(nodes[bestIndex]->quadStatement);
            bool candidateMul = isMulBinopStmt(probe.stm);
            bool bestMul = bestIndex >= 0 &&
                           isMulBinopStmt(nodes[bestIndex]->quadStatement);
            bool mulScheduled = mulBinopScheduledInBlock(nodes, scheduled);

            bool better = false;
            if (bestIndex < 0) {
                better = true;
            } else if (priority < bestPriority) {
                better = true;
            } else if (priority == bestPriority) {
                bool candidateConst = isConstMoveStmt(probe.stm);
                bool bestConst = bestIndex >= 0 &&
                                 isConstMoveStmt(nodes[bestIndex]->quadStatement);
                bool candidateExt = isExtCallLikeStmt(probe.stm);
                bool bestExt = bestIndex >= 0 &&
                               isExtCallLikeStmt(nodes[bestIndex]->quadStatement);

                if (candidateExt && bestConst &&
                    extCallWinsOverAdjacentConstMove(bestQuadIndex, quadIndex)) {
                    better = true;
                } else if (candidateConst && bestExt &&
                           extCallWinsOverAdjacentConstMove(quadIndex, bestQuadIndex)) {
                    better = false;
                } else if (candidateMul && bestSub && !mulScheduled) {
                    better = true;
                } else if (candidateSub && bestMul && !mulScheduled) {
                    better = false;
                } else if (isAddBinop(probe.stm) &&
                           hasUnscheduledMulAfter(
                               nodes, scheduled, quadIndex)) {
                    better = false;
                } else if (candidateMul && bestIndex >= 0 &&
                           isAddBinop(
                               nodes[bestIndex]->quadStatement) &&
                           hasUnscheduledMulAfter(
                               nodes, scheduled, bestQuadIndex)) {
                    better = true;
                } else if (isAddBinop(probe.stm) && bestMul &&
                           hasUnscheduledMulBefore(
                               nodes, scheduled, quadIndex)) {
                    better = false;
                } else if (candidateMul && bestIndex >= 0 &&
                           isAddBinop(
                               nodes[bestIndex]->quadStatement) &&
                           hasUnscheduledMulBefore(
                               nodes, scheduled, bestQuadIndex)) {
                    better = true;
                } else if (isAddConstBinop(probe.stm) && bestMul &&
                           hasUnscheduledMulBefore(
                               nodes, scheduled, quadIndex)) {
                    better = false;
                } else if (candidateMul && bestIndex >= 0 &&
                           isAddConstBinop(
                               nodes[bestIndex]->quadStatement) &&
                           hasUnscheduledMulBefore(
                               nodes, scheduled, bestQuadIndex)) {
                    better = true;
                } else if (candidateSub && !bestSub && mulScheduled) {
                    better = quadIndex > bestQuadIndex;
                } else if (!candidateSub && bestSub) {
                    better = false;
                } else if (highQuad) {
                    better = quadIndex < bestQuadIndex;
                } else if (preferHighQuadIndex) {
                    better = false;
                } else {
                    better = quadIndex < bestQuadIndex;
                }
            }

            if (better) {
                bestPriority = priority;
                bestQuadIndex = quadIndex;
                preferHighQuadIndex = highQuad;
                bestIndex = static_cast<int>(i);
            }
        }

        if (bestIndex < 0) {
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (!scheduled[i]) {
                    bestIndex = static_cast<int>(i);
                    break;
                }
            }
        }

        if (bestIndex < 0) {
            break;
        }

        scheduled[bestIndex] = true;
        ++scheduledCount;

        StatementBundle bundle =
            buildBundle(ctx, nodes[bestIndex], static_cast<size_t>(bestIndex), definedTemps);
        if (!bundle.instrs.instrs.empty()) {
            schedBlock.selectedInstructions.extend(bundle.instrs);
        }
        if (bundle.tempDefined >= 0 && !bundle.instrs.instrs.empty()) {
            definedTemps.insert(bundle.tempDefined);
        }
    }
}

void runInstructionSelectionPass(
    const advDFGprog &graphProgram,
    preScheduleProg &preScheduleProgram
) {
    size_t funcCount = std::min(
        graphProgram.fungraph.size(),
        preScheduleProgram.funcSchedules.size()
    );
    for (size_t funcIndex = 0; funcIndex < funcCount; ++funcIndex) {
        auto *funcGraph = graphProgram.fungraph[funcIndex];
        auto *funcSchedule = preScheduleProgram.funcSchedules[funcIndex];
        if (funcGraph == nullptr || funcSchedule == nullptr ||
            funcSchedule->quadFunc == nullptr) {
            continue;
        }

        int nextTempNum = funcSchedule->quadFunc->last_temp_num + 1;
        size_t blockCount = std::min(
            funcGraph->blockgraph.size(),
            funcSchedule->blockSchedules.size()
        );
        for (size_t blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
            auto *blockGraph = funcGraph->blockgraph[blockIndex];
            auto *blockSchedule = funcSchedule->blockSchedules[blockIndex];
            if (blockGraph == nullptr || blockSchedule == nullptr) {
                continue;
            }

            selectInstructionsForBlock(*blockGraph, *blockSchedule, nextTempNum);
        }

        auto *mutableFunc =
            const_cast<quad::QuadFuncDecl *>(funcSchedule->quadFunc);
        mutableFunc->last_temp_num = nextTempNum - 1;
    }
}

} // namespace instr
