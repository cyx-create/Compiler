#define DEBUG
#undef DEBUG

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <algorithm>
#include "temp.hh"
#include "quad.hh"
#include "flowinfo.hh"

using namespace std;
using namespace quad;

static set<int> intersectDominators(const set<int>& a, const set<int>& b) {
    set<int> result;
    set_intersection(a.begin(), a.end(), b.begin(), b.end(),
                     inserter(result, result.begin()));
    return result;
}

void ControlFlowInfo::computeAllBlocks() {
#ifdef DEBUG
    cout << "Computing all blocks in function: " << func->funcname << endl;
    cout << "#blocks = " << func->quadblocklist->size() << endl;
#endif
    if (func == nullptr || func->quadblocklist == nullptr) {
        return;
    }
    allBlocks = set<int>();
    labelToBlock = map<int, QuadBlock*>();

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
    if (entryBlock == -1 || allBlocks.empty()) {
        unreachableBlocks = allBlocks;
        return;
    }

    set<int> reachable;
    queue<int> work;
    reachable.insert(entryBlock);
    work.push(entryBlock);

    while (!work.empty()) {
        int b = work.front();
        work.pop();
        auto succIt = successors.find(b);
        if (succIt == successors.end())
            continue;
        for (int s : succIt->second) {
            if (!reachable.count(s)) {
                reachable.insert(s);
                work.push(s);
            }
        }
    }

    for (int b : allBlocks) {
        if (!reachable.count(b))
            unreachableBlocks.insert(b);
    }
}

void ControlFlowInfo::eliminateUnreachableBlocks() {
#ifdef DEBUG
    cout << "Eliminating unreachable blocks in function: " << func->funcname << endl;
#endif
    if (unreachableBlocks.empty() || func == nullptr || func->quadblocklist == nullptr)
        return;

    vector<QuadBlock*> kept;
    kept.reserve(func->quadblocklist->size());
    for (auto* block : *func->quadblocklist) {
        if (!block || !block->entry_label)
            continue;
        if (!unreachableBlocks.count(block->entry_label->num))
            kept.push_back(block);
    }
    *func->quadblocklist = kept;

    predecessors.clear();
    successors.clear();
    dominators.clear();
    immediateDominator.clear();
    dominanceFrontiers.clear();
    domTree.clear();
    unreachableBlocks.clear();
    computeAllBlocks();
}

void ControlFlowInfo::computePredecessors() {
    predecessors.clear();
    successors.clear();
    for (int b : allBlocks) {
        predecessors[b] = set<int>();
        successors[b] = set<int>();
    }

    if (!func || !func->quadblocklist)
        return;

    for (auto* block : *func->quadblocklist) {
        if (!block || !block->entry_label)
            continue;
        int bid = block->entry_label->num;
        if (!block->exit_labels)
            continue;
        for (Label* lbl : *block->exit_labels) {
            if (!lbl)
                continue;
            int sid = lbl->num;
            successors[bid].insert(sid);
            predecessors[sid].insert(bid);
        }
    }
}

void ControlFlowInfo::computeSuccessors() {
    // Successors are populated together with predecessors in computePredecessors().
    for (int b : allBlocks) {
        if (successors.find(b) == successors.end())
            successors[b] = set<int>();
    }
}

void ControlFlowInfo::computeDominators() {
#ifdef DEBUG
    std::cout << "Computing dominators for: " << func->funcname << endl;
#endif
    dominators.clear();
    if (allBlocks.empty())
        return;

    for (int b : allBlocks) {
        if (b == entryBlock)
            dominators[b] = {entryBlock};
        else
            dominators[b] = allBlocks;
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (int b : allBlocks) {
            if (b == entryBlock)
                continue;
            set<int> newDom = allBlocks;
            auto predIt = predecessors.find(b);
            if (predIt != predecessors.end() && !predIt->second.empty()) {
                bool first = true;
                for (int p : predIt->second) {
                    if (first) {
                        newDom = dominators[p];
                        first = false;
                    } else {
                        newDom = intersectDominators(newDom, dominators[p]);
                    }
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
    if (entryBlock == -1)
        return;

    immediateDominator[entryBlock] = entryBlock;

    for (int b : allBlocks) {
        if (b == entryBlock)
            continue;
        set<int> strictDoms = dominators[b];
        strictDoms.erase(b);

        int idom = entryBlock;
        for (int candidate : strictDoms) {
            bool isImmediate = true;
            for (int other : strictDoms) {
                if (other == candidate)
                    continue;
                auto domIt = dominators.find(other);
                if (domIt != dominators.end() && domIt->second.count(candidate)) {
                    isImmediate = false;
                    break;
                }
            }
            if (isImmediate) {
                idom = candidate;
                break;
            }
        }
        immediateDominator[b] = idom;
    }
}

void ControlFlowInfo::computeDomTree() {
#ifdef DEBUG
    std::cout << "Computing dominator tree for: " << func->funcname << endl;
#endif
    domTree.clear();
    for (int b : allBlocks)
        domTree[b] = set<int>();

    for (auto& kv : immediateDominator) {
        int b = kv.first;
        int idom = kv.second;
        if (b != entryBlock && idom != b)
            domTree[idom].insert(b);
    }
}

void ControlFlowInfo::computeDominanceFrontiers() {
#ifdef DEBUG
    std::cout << "Computing dominance frontier for: " << func->funcname << endl;
#endif
    dominanceFrontiers.clear();
    for (int b : allBlocks)
        dominanceFrontiers[b] = set<int>();

    for (int b : allBlocks) {
        auto succIt = successors.find(b);
        if (succIt == successors.end() || succIt->second.size() < 2)
            continue;
        for (int y : succIt->second)
            dominanceFrontiers[b].insert(y);
    }

    for (int b : allBlocks) {
        auto succIt = successors.find(b);
        if (succIt == successors.end())
            continue;
        for (int y : succIt->second) {
            int x = b;
            auto idomY = immediateDominator.find(y);
            int stop = (idomY != immediateDominator.end()) ? idomY->second : entryBlock;
            while (x != stop) {
                dominanceFrontiers[x].insert(y);
                auto idomX = immediateDominator.find(x);
                if (idomX == immediateDominator.end())
                    break;
                x = idomX->second;
            }
        }
    }
}
