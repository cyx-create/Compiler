#include <stack>
#include <map>
#include <set>
#include <algorithm>
#include <utility>
#include "loopopt.hh"
#include "flowinfo.hh"
#include "quad.hh"

using namespace std;
using namespace quad;

// Defined in loophoistfunc.cc — clears per-function visit list when a new program is read.
void loopOptClearVisitOrder();

// Natural loop for back edge m -> n (n is header): classic Dragon-book stack algorithm.
static set<int> naturalLoopForBackEdge(int m, int n, ControlFlowInfo *cfi) {
    set<int> loop = {n};
    stack<int> st;
    st.push(m);
    while (!st.empty()) {
        int x = st.top();
        st.pop();
        if (!loop.count(x)) {
            loop.insert(x);
            auto it = cfi->predecessors.find(x);
            if (it != cfi->predecessors.end()) {
                for (int p : it->second) {
                    st.push(p);
                }
            }
        }
    }
    return loop;
}

LoopHeaderMap *findLoopHeaders(QuadFuncDecl *func, FuncFlowInfo *ffi) {
    // New flow XML file uses a new (last_label, last_temp) pair on the program root.
    if (ffi != nullptr) {
        pair<int, int> key(ffi->programLastLabelNum, ffi->programLastTempNum);
        static pair<int, int> s_lastKey{-1, -1};
        if (key != s_lastKey) {
            s_lastKey = key;
            loopOptClearVisitOrder();
        }
    }

    LoopHeaderMap *loopHeaderMap = new LoopHeaderMap();
    ControlFlowInfo *cfi = ffi->cfi;

    map<int, set<int>> headerToBody;

    for (const auto &succEntry : cfi->successors) {
        int u = succEntry.first;
        const set<int> &succs = succEntry.second;
        auto domU = cfi->dominators.find(u);
        if (domU == cfi->dominators.end()) {
            continue;
        }
        const set<int> &doms = domU->second;
        for (int v : succs) {
            if (doms.count(v)) {
                // u -> v is a back edge; v is a loop header.
                set<int> body = naturalLoopForBackEdge(u, v, cfi);
                for (int b : body) {
                    headerToBody[v].insert(b);
                }
            }
        }
    }

    set<LoopHeader *> lhset;
    for (const auto &hb : headerToBody) {
        lhset.insert(new LoopHeader(hb.first, hb.second));
    }

    loopHeaderMap->addLoopHeader(func, lhset);
    return loopHeaderMap;
}
