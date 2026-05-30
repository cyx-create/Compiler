#include "advDFG.hh"

#include <unordered_map>
#include <vector>

namespace instr {

namespace {

static bool isExitCallStmt(const quad::QuadStm *stm) {
    if (stm == nullptr) {
        return false;
    }
    if (stm->kind == quad::QuadKind::EXTCALL) {
        auto *ext = dynamic_cast<const quad::QuadExtCall *>(stm);
        return ext != nullptr && ext->extfun == "exit";
    }
    if (stm->kind == quad::QuadKind::MOVE_EXTCALL) {
        auto *moveExt = dynamic_cast<const quad::QuadMoveExtCall *>(stm);
        return moveExt != nullptr && moveExt->extcall != nullptr &&
               moveExt->extcall->extfun == "exit";
    }
    if (stm->kind == quad::QuadKind::CALL) {
        auto *call = dynamic_cast<const quad::QuadCall *>(stm);
        return call != nullptr && call->name == "exit";
    }
    if (stm->kind == quad::QuadKind::MOVE_CALL) {
        auto *moveCall = dynamic_cast<const quad::QuadMoveCall *>(stm);
        return moveCall != nullptr && moveCall->call != nullptr &&
               moveCall->call->name == "exit";
    }
    return false;
}

static bool isBlockFinalStmt(const quad::QuadStm *stm) {
    if (stm == nullptr) {
        return false;
    }
    return stm->kind == quad::QuadKind::RETURN ||
           stm->kind == quad::QuadKind::JUMP ||
           stm->kind == quad::QuadKind::CJUMP ||
           isExitCallStmt(stm);
}

static int singleDefTempNum(const quad::QuadStm *stm) {
    if (stm == nullptr || stm->def == nullptr || stm->def->empty()) {
        return -1;
    }
    return (*stm->def->begin())->num;
}

static std::set<int> useTempNums(const quad::QuadStm *stm) {
    std::set<int> uses;
    if (stm == nullptr || stm->use == nullptr) {
        return uses;
    }
    for (auto *temp : *stm->use) {
        if (temp != nullptr) {
            uses.insert(temp->num);
        }
    }
    return uses;
}

static bool isPtrCalcStmt(const quad::QuadStm *stm) {
    return stm != nullptr && stm->kind == quad::QuadKind::PTR_CALC;
}

static bool isLoadOrStoreStmt(const quad::QuadStm *stm) {
    return stm != nullptr &&
           (stm->kind == quad::QuadKind::LOAD ||
            stm->kind == quad::QuadKind::STORE);
}

static int ptrCalcMemTempNum(const quad::QuadStm *stm) {
    auto *load = dynamic_cast<const quad::QuadLoad *>(stm);
    if (load != nullptr && load->src != nullptr &&
        load->src->kind == quad::QuadTermKind::TEMP) {
        return load->src->get_temp()->temp->num;
    }
    auto *store = dynamic_cast<const quad::QuadStore *>(stm);
    if (store != nullptr && store->dst != nullptr &&
        store->dst->kind == quad::QuadTermKind::TEMP) {
        return store->dst->get_temp()->temp->num;
    }
    return -1;
}

static void buildBlockGraph(advDFGblock *blockRecord) {
    const quad::QuadBlock *block = blockRecord->quadBlock;
    if (block == nullptr || block->quadlist == nullptr) {
        return;
    }

    advDFG &graph = blockRecord->graph;
    std::vector<const quad::QuadStm *> orderedStms;
    orderedStms.reserve(block->quadlist->size());

    for (auto *stm : *block->quadlist) {
        if (stm == nullptr || stm->kind == quad::QuadKind::PHI ||
            stm->kind == quad::QuadKind::LABEL) {
            continue;
        }
        orderedStms.push_back(stm);
    }

    if (orderedStms.empty()) {
        return;
    }

    auto *entryNode = new advDFGNode(NodeType::EntryLabel);
    if (block->entry_label != nullptr) {
        entryNode->tempDefined = block->entry_label->num;
    }
    graph.addNode(entryNode);

    std::unordered_map<int, advDFGNode *> tempDefNode;
    int nextChainId = 0;

    advDFGNode *previousNode = entryNode;
    for (auto *stm : orderedStms) {
        NodeType nodeType = isBlockFinalStmt(stm) ? NodeType::ExitStatement
                                                  : NodeType::Statement;
        auto *node = new advDFGNode(nodeType, stm);
        node->tempDefined = singleDefTempNum(stm);
        node->tempsUsed = useTempNums(stm);

        if (isPtrCalcStmt(stm)) {
            node->chainDefined = nextChainId++;
        }

        graph.addNode(node);
        graph.addEdge(previousNode, node);
        previousNode = node;

        if (node->tempDefined >= 0) {
            tempDefNode[node->tempDefined] = node;
        }
    }

    for (auto *node : graph.getNodes()) {
        if (node == nullptr || node->type != NodeType::Statement ||
            node->quadStatement == nullptr) {
            continue;
        }

        const quad::QuadStm *stm = node->quadStatement;
        if (!isLoadOrStoreStmt(stm)) {
            continue;
        }

        int memTemp = ptrCalcMemTempNum(stm);
        if (memTemp < 0) {
            continue;
        }

        auto defIt = tempDefNode.find(memTemp);
        if (defIt == tempDefNode.end()) {
            continue;
        }

        advDFGNode *defNode = defIt->second;
        if (defNode->quadStatement == nullptr ||
            !isPtrCalcStmt(defNode->quadStatement) ||
            defNode->chainDefined < 0) {
            continue;
        }

        node->chainUsed = defNode->chainDefined;
        node->tempsUsed.erase(memTemp);
        graph.addEdge(defNode, node);
    }

    for (auto *node : graph.getNodes()) {
        if (node == nullptr || node->quadStatement == nullptr) {
            continue;
        }
        if (node->type != NodeType::Statement &&
            node->type != NodeType::ExitStatement) {
            continue;
        }

        for (int useTemp : node->tempsUsed) {
            auto defIt = tempDefNode.find(useTemp);
            if (defIt != tempDefNode.end() && defIt->second != node) {
                graph.addEdge(defIt->second, node);
            }
        }
    }
}

} // namespace

advDFGprog *buildAdvDFGprog(const quad::QuadProgram *program) {
    auto *out = new advDFGprog(program);
    if (program == nullptr || program->quadFuncDeclList == nullptr) {
        return out;
    }

    for (auto *func : *program->quadFuncDeclList) {
        if (func == nullptr) {
            continue;
        }

        auto *funcRecord = new advDFGfunc(func);
        out->addFunc(funcRecord);

        if (func->quadblocklist == nullptr) {
            continue;
        }

        for (auto *block : *func->quadblocklist) {
            auto *blockRecord = new advDFGblock(block);
            buildBlockGraph(blockRecord);
            funcRecord->addBlock(blockRecord);
        }
    }

    return out;
}

} // namespace instr
