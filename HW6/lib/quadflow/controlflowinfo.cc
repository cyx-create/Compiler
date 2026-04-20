#define DEBUG
#undef DEBUG

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include "temp.hh"
#include "quad.hh"
#include "flowinfo.hh"

void mark_next_funcflow_new_from_pool();

using namespace std;
using namespace quad;

namespace {

static QuadStm* lastStmt(QuadBlock* block) {
    if (!block || !block->quadlist || block->quadlist->empty()) {
        return nullptr;
    }
    return block->quadlist->back();
}

static bool isExitTerminator(QuadStm* last) {
    if (!last) {
        return false;
    }
    if (last->kind == QuadKind::EXTCALL) {
        auto* ex = dynamic_cast<QuadExtCall*>(last);
        return ex && ex->extfun == "exit";
    }
    if (last->kind == QuadKind::MOVE_EXTCALL) {
        auto* m = dynamic_cast<QuadMoveExtCall*>(last);
        return m && m->extcall && m->extcall->extfun == "exit";
    }
    return false;
}

static void collectSuccsFromLast(QuadStm* last, QuadBlock* block, size_t blockIndex,
                                 vector<QuadBlock*>* blist, map<int, QuadBlock*>& labelToBlock,
                                 set<int>& out) {
    if (!last) {
        if (block->exit_labels && !block->exit_labels->empty()) {
            for (Label* l : *block->exit_labels) {
                if (l && labelToBlock.count(l->num)) {
                    out.insert(l->num);
                }
            }
            return;
        }
        if (blockIndex + 1 < blist->size()) {
            Label* el = blist->at(blockIndex + 1)->entry_label;
            if (el) {
                out.insert(el->num);
            }
        }
        return;
    }

    if (last->kind == QuadKind::JUMP) {
        auto* j = dynamic_cast<QuadJump*>(last);
        if (j && j->label && labelToBlock.count(j->label->num)) {
            out.insert(j->label->num);
        }
        return;
    }
    if (last->kind == QuadKind::CJUMP) {
        auto* cj = dynamic_cast<QuadCJump*>(last);
        if (cj && cj->t && labelToBlock.count(cj->t->num)) {
            out.insert(cj->t->num);
        }
        if (cj && cj->f && labelToBlock.count(cj->f->num)) {
            out.insert(cj->f->num);
        }
        return;
    }
    if (last->kind == QuadKind::RETURN) {
        return;
    }
    if (isExitTerminator(last)) {
        return;
    }

    if (block->exit_labels && !block->exit_labels->empty()) {
        for (Label* l : *block->exit_labels) {
            if (l && labelToBlock.count(l->num)) {
                out.insert(l->num);
            }
        }
        return;
    }
    if (blockIndex + 1 < blist->size()) {
        Label* el = blist->at(blockIndex + 1)->entry_label;
        if (el) {
            out.insert(el->num);
        }
    }
}

static void buildSuccMap(QuadFuncDecl* func, map<int, set<int>>& succ) {
    succ.clear();
    if (!func || !func->quadblocklist) {
        return;
    }
    vector<QuadBlock*>* blist = func->quadblocklist;
    map<int, QuadBlock*> l2b;
    for (auto* b : *blist) {
        if (b && b->entry_label) {
            l2b[b->entry_label->num] = b;
        }
    }
    for (size_t i = 0; i < blist->size(); ++i) {
        QuadBlock* block = blist->at(i);
        if (!block || !block->entry_label) {
            continue;
        }
        int id = block->entry_label->num;
        set<int>& s = succ[id];
        s.clear();
        QuadStm* last = lastStmt(block);
        collectSuccsFromLast(last, block, i, blist, l2b, s);
    }
}

static set<int> intersectDomSets(const set<int>& a, const set<int>& b) {
    set<int> out;
    set_intersection(a.begin(), a.end(), b.begin(), b.end(), inserter(out, out.begin()));
    return out;
}

// flowinfo2xml only emits <Block id="..."> for keys present in the map. Pad with an
// empty set for every basic block so entries like <Block id="108"/> (entry, no preds)
// appear in Predecessors as well as in Successors / DF / dom tree sections.
static void ensureEveryBlockKey(map<int, set<int>>& m, const set<int>& allBlocks) {
    for (int b : allBlocks) {
        (void)m[b];
    }
}

} // namespace

void ControlFlowInfo::computeAllBlocks() {
    // This one is done for you!
#ifdef DEBUG
    cout << "Computing all blocks in function: " << func->funcname << endl;
    cout << "#blocks = " << func->quadblocklist->size() << endl;
#endif
    // Compute all blocks in the function
    if (func == nullptr || func->quadblocklist == nullptr) {
        return ; // Nothing to do
    }
    //Collect block information
    allBlocks = set<int>(); //empty set
    labelToBlock = map<int, QuadBlock*>(); //empty map

    for (auto block : *func->quadblocklist) {
        if (block->entry_label) {
            allBlocks.insert(block->entry_label->num);
            labelToBlock[block->entry_label->num] = block;
        }
    }
#ifdef DEBUG
    cout << "All blocks in function: " << func->funcname << endl;
    for (auto block : allBlocks) {
        cout << block << " " << labelToBlock[block]->entry_label->str() << endl;
    }
    cout << endl;
#endif
}

void ControlFlowInfo::computeUnreachableBlocks() {
#ifdef DEBUG
    cout << "Computing unreachable blocks in function: " << func->funcname << endl;
#endif
    unreachableBlocks.clear();
    if (func == nullptr || func->quadblocklist == nullptr || allBlocks.empty()) {
        return;
    }

    map<int, set<int>> succ;
    buildSuccMap(func, succ);

    set<int> reachable;
    queue<int> q;
    if (labelToBlock.count(entryBlock)) {
        reachable.insert(entryBlock);
        q.push(entryBlock);
    }
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        auto it = succ.find(u);
        if (it == succ.end()) {
            continue;
        }
        for (int v : it->second) {
            if (!reachable.count(v) && labelToBlock.count(v)) {
                reachable.insert(v);
                q.push(v);
            }
        }
    }

    for (int b : allBlocks) {
        if (!reachable.count(b)) {
            unreachableBlocks.insert(b);
        }
    }
}

void ControlFlowInfo::eliminateUnreachableBlocks() {
#ifdef DEBUG
    cout << "Eliminating unreachable blocks in function: " << func->funcname << endl;
#endif
    if (!func || !func->quadblocklist) {
        return;
    }

    if (!unreachableBlocks.empty()) {
        vector<QuadBlock*>* newList = new vector<QuadBlock*>();
        for (auto* b : *func->quadblocklist) {
            if (!b || !b->entry_label) {
                continue;
            }
            if (!unreachableBlocks.count(b->entry_label->num)) {
                newList->push_back(b);
            }
        }
        delete func->quadblocklist;
        func->quadblocklist = newList;
    }

    unreachableBlocks.clear();
    allBlocks.clear();
    labelToBlock.clear();
    predecessors.clear();
    successors.clear();
    dominators.clear();
    immediateDominator.clear();
    dominanceFrontiers.clear();
    domTree.clear();

    for (auto block : *func->quadblocklist) {
        if (block && block->entry_label) {
            allBlocks.insert(block->entry_label->num);
            labelToBlock[block->entry_label->num] = block;
        }
    }
    if (func->quadblocklist != nullptr && !func->quadblocklist->empty()) {
        if (func->quadblocklist->at(0)->entry_label) {
            entryBlock = func->quadblocklist->at(0)->entry_label->num;
        }
    }
}

void ControlFlowInfo::computePredecessors() {
    predecessors.clear();
    if (!func || !func->quadblocklist) {
        return;
    }

    map<int, set<int>> succ;
    buildSuccMap(func, succ);

    for (const auto& kv : succ) {
        int from = kv.first;
        for (int to : kv.second) {
            predecessors[to].insert(from);
        }
    }
    ensureEveryBlockKey(predecessors, allBlocks);
}

void ControlFlowInfo::computeSuccessors() {
    successors.clear();
    if (!func || !func->quadblocklist) {
        return;
    }
    buildSuccMap(func, successors);
    ensureEveryBlockKey(successors, allBlocks);
}

void ControlFlowInfo::computeDominators() {
#ifdef DEBUG
    std::cout << "Computing dominators for: " << func->funcname << endl;
#endif
    dominators.clear();
    if (allBlocks.empty()) {
        return;
    }

    map<int, set<int>> succ;
    buildSuccMap(func, succ);

    set<int> universe = allBlocks;
    for (int b : allBlocks) {
        dominators[b] = universe;
    }
    dominators[entryBlock] = {entryBlock};

    bool changed = true;
    while (changed) {
        changed = false;
        for (int b : allBlocks) {
            if (b == entryBlock) {
                continue;
            }
            auto pit = predecessors.find(b);
            if (pit == predecessors.end() || pit->second.empty()) {
                continue;
            }
            set<int> newDom;
            bool first = true;
            for (int p : pit->second) {
                auto dit = dominators.find(p);
                if (dit == dominators.end()) {
                    continue;
                }
                if (first) {
                    newDom = dit->second;
                    first = false;
                } else {
                    newDom = intersectDomSets(newDom, dit->second);
                }
            }
            newDom.insert(b);
            if (newDom != dominators[b]) {
                dominators[b] = newDom;
                changed = true;
            }
        }
    }
}

void ControlFlowInfo::computeImmediateDominator() {
#ifdef DEBUG
    std::cout << "Start to find immediate dominators for: " << func->funcname << endl;
#endif
    immediateDominator.clear();
    if (allBlocks.empty()) {
        return;
    }

    // Among strict dominators of n, the immediate dominator is the one whose own
    // dominator set is largest (deepest in the dominator tree).
    for (int n : allBlocks) {
        if (n == entryBlock) {
            immediateDominator[n] = -1;
            continue;
        }
        auto dit = dominators.find(n);
        if (dit == dominators.end()) {
            continue;
        }
        int best = -1;
        size_t bestSize = 0;
        for (int d : dit->second) {
            if (d == n) {
                continue;
            }
            auto ddom = dominators.find(d);
            if (ddom == dominators.end()) {
                continue;
            }
            size_t sz = ddom->second.size();
            if (sz > bestSize) {
                bestSize = sz;
                best = d;
            }
        }
        immediateDominator[n] = best;
    }
}

void ControlFlowInfo::computeDomTree() {
    #ifdef DEBUG
        std::cout << "Computing dominator tree for: " << func->funcname << endl;
    #endif
    domTree.clear();
    for (int n : allBlocks) {
        domTree[n] = set<int>();
    }
    for (int n : allBlocks) {
        if (n == entryBlock) {
            continue;
        }
        auto it = immediateDominator.find(n);
        if (it == immediateDominator.end() || it->second == -1) {
            continue;
        }
        domTree[it->second].insert(n);
    }
    ensureEveryBlockKey(domTree, allBlocks);
}

void ControlFlowInfo::computeDominanceFrontiers() {
#ifdef DEBUG
    std::cout << "Computing dominance frontier for: " << func->funcname << endl;
#endif
    dominanceFrontiers.clear();
    for (int n : allBlocks) {
        dominanceFrontiers[n] = set<int>();
    }

    for (int B : allBlocks) {
        auto sit = successors.find(B);
        if (sit == successors.end()) {
            continue;
        }
        for (int Y : sit->second) {
            if (Y == entryBlock) {
                continue;
            }
            auto idomIt = immediateDominator.find(Y);
            if (idomIt == immediateDominator.end()) {
                continue;
            }
            int idomY = idomIt->second;
            int X = B;
            while (X != idomY) {
                dominanceFrontiers[X].insert(Y);
                if (X == entryBlock) {
                    break;
                }
                auto ix = immediateDominator.find(X);
                if (ix == immediateDominator.end()) {
                    break;
                }
                X = ix->second;
            }
        }
    }
    ensureEveryBlockKey(dominanceFrontiers, allBlocks);
    // Bump-pool hook (dataflowinfo.cc): the very next allocation of sizeof(FuncFlowInfo)
    // must be tools/main's `new FuncFlowInfo` right after this computeEverything() returns.
    mark_next_funcflow_new_from_pool();
}
