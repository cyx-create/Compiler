// #define DEBUG
#undef DEBUG

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "temp.hh"
#include "ig.hh"
#include "asmdataflow.hh"
#include "asmprog.hh"

using namespace std;
using namespace instr;

static void addIgEdge(map<int, set<int>>& graph, int u, int v) {
    if (u == v) {
        return;
    }
    graph[u].insert(v);
    graph[v].insert(u);
}

InterferenceGraph* buildIg(instr::AsmFunction* asmFunc, instr::AsmDataFlowInfo* flowInfo) {
    map<int, set<int>> graph;
    set<pair<int, int>> movePairs;

    if (asmFunc == nullptr || flowInfo == nullptr) {
        return new InterferenceGraph(graph, movePairs);
    }

    const vector<AssemInstr>& instrs = asmFunc->instructions;
    size_t n = instrs.size();

    auto addNode = [&](int t) {
        if (graph.find(t) == graph.end()) {
            graph[t] = set<int>();
        }
    };

    for (size_t i = 0; i < n; ++i) {
        const AssemInstr& instr = instrs[i];
        for (tree::Temp* t : instr.dst) {
            if (t != nullptr) {
                addNode(t->num);
            }
        }
        for (tree::Temp* t : instr.src) {
            if (t != nullptr) {
                addNode(t->num);
            }
        }
    }

    for (size_t i = n; i > 0; --i) {
        size_t idx = i - 1;
        const AssemInstr& instr = instrs[idx];
        set<int> live = flowInfo->liveout[idx];

        if (instr.kind == AssemInstr::I_MOVE && instr.dst.size() == 1 && instr.src.size() == 1 &&
            instr.dst[0] != nullptr && instr.src[0] != nullptr) {
            int dst = instr.dst[0]->num;
            int src = instr.src[0]->num;
            movePairs.insert({src, dst});
            live.erase(src);
        }

        set<int> defs = flowInfo->getDef(idx);
        for (int d : defs) {
            for (int l : live) {
                addIgEdge(graph, l, d);
            }
        }

        set<int> uses = flowInfo->getUse(idx);
        set<int> newLive = uses;
        for (int l : live) {
            if (defs.find(l) == defs.end()) {
                newLive.insert(l);
            }
        }
        live = newLive;
    }

    return new InterferenceGraph(graph, movePairs);
}

vector<InterferenceGraph*> buildIgProg(instr::AsmProg* program) {
    vector<InterferenceGraph*> graphs;

    if (program == nullptr || program->functions.empty()) {
        return graphs;
    }

#ifdef DEBUG
    cout << "Building interference graphs for program with " << program->functions.size()
         << " functions" << endl;
#endif

    for (AsmFunction& func : program->functions) {
        AsmDataFlowInfo flowInfo(&func);
        flowInfo.computeLiveness();
        graphs.push_back(buildIg(&func, &flowInfo));
    }

    return graphs;
}
