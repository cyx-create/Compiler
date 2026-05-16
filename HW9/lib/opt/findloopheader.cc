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

// Defined in loophoistfunc.cc — flow program key + findLoop call counter for multi-func order.
void loopOptNoteNewFlowProgram(FuncFlowInfo *ffi);
void loopOptAfterFindLoopHeaders();

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
    loopOptNoteNewFlowProgram(ffi);

    LoopHeaderMap *loopHeaderMap = new LoopHeaderMap();
    if (ffi == nullptr || ffi->cfi == nullptr) {
        if (func != nullptr) {
            loopHeaderMap->initFunc(func);
        }
        return loopHeaderMap;
    }
    loopOptAfterFindLoopHeaders();

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
