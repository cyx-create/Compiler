#include <stack>
#include <map>
#include <set>
#include <algorithm>
#include <vector>
#include "loopheader.hh"
#include "flowinfo.hh"
#include "quad.hh"

using namespace std;
using namespace quad;

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

LoopHeaderMap* findLoopHeadersWithFlow(QuadFuncDecl* func, ControlFlowInfo* flowInfo) {
    if (func == nullptr || flowInfo == nullptr) {
        return new LoopHeaderMap();
    }

    LoopHeaderMap* loopHeaderMap = new LoopHeaderMap();
    loopHeaderMap->initFunc(func);

    map<int, set<int>> headerToBody;

    for (const auto &succEntry : flowInfo->successors) {
        int u = succEntry.first;
        const set<int> &succs = succEntry.second;
        auto domU = flowInfo->dominators.find(u);
        if (domU == flowInfo->dominators.end()) {
            continue;
        }
        const set<int> &doms = domU->second;
        for (int v : succs) {
            if (doms.count(v)) {
                set<int> body = naturalLoopForBackEdge(u, v, flowInfo);
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
