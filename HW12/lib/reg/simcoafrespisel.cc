#define DEBUG
#undef DEBUG

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "ig.hh"
#include "coloring.hh"

using namespace std;

namespace {

int getAlias(int n, const map<int, set<int>>& coalescedMoves) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& entry : coalescedMoves) {
            if (entry.second.count(n) != 0) {
                n = entry.first;
                changed = true;
                break;
            }
        }
    }
    return n;
}

int nodeDegree(int n, const map<int, set<int>>& graph) {
    auto it = graph.find(n);
    if (it == graph.end()) {
        return 0;
    }
    return static_cast<int>(it->second.size());
}

bool adjacent(int u, int v, const map<int, set<int>>& graph) {
    auto it = graph.find(u);
    if (it == graph.end()) {
        return false;
    }
    return it->second.find(v) != it->second.end();
}

bool okToCoalesce(int t, int r, const map<int, set<int>>& graph, int K) {
    if (Coloring::isMachineReg(t)) {
        return true;
    }
    if (nodeDegree(t, graph) < K) {
        return true;
    }
    return adjacent(t, r, graph);
}

bool conservative(const set<int>& nodes, const map<int, set<int>>& graph, int K) {
    int high = 0;
    for (int n : nodes) {
        if (nodeDegree(n, graph) >= K) {
            ++high;
        }
    }
    return high < K;
}

set<int> adjacentNodes(int n, const map<int, set<int>>& graph) {
    auto it = graph.find(n);
    if (it == graph.end()) {
        return set<int>();
    }
    return it->second;
}

void combineNodes(Coloring* c, int u, int v) {
    if (c->graph.find(v) == c->graph.end()) {
        return;
    }

    c->coalescedMoves[u].insert(v);

    for (int t : c->graph[v]) {
        if (t != u) {
            c->addEdge(u, t);
        }
    }

    c->eraseNode(v);

    for (auto it = c->movePairs.begin(); it != c->movePairs.end();) {
        if (it->first == v || it->second == v) {
            it = c->movePairs.erase(it);
        } else {
            ++it;
        }
    }
}

void freezeMovesFor(Coloring* c, int u) {
    for (auto it = c->movePairs.begin(); it != c->movePairs.end();) {
        if (it->first == u || it->second == u) {
            it = c->movePairs.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace

bool Coloring::simplify() {
#ifdef DEBUG
    cout << "Simplifying..." << endl;
#endif
    const int K = k + 1;

    int best = -1;
    int bestDegree = K;

    for (const auto& entry : graph) {
        int n = entry.first;
        if (isMachineReg(n) || isMove(n)) {
            continue;
        }
        int deg = static_cast<int>(entry.second.size());
        if (deg < K && (deg < bestDegree || (deg == bestDegree && (best == -1 || n < best)))) {
            bestDegree = deg;
            best = n;
        }
    }

    if (best != -1) {
        simplifiedNodes.push(best);
        eraseNode(best);
        return true;
    }

    return false;
}

bool Coloring::coalesce() {
#ifdef DEBUG
    cout << "Coalescing..." << endl;
#endif
    if (ig == nullptr || movePairs.empty()) {
        return false;
    }
    if (k >= 8) {
        return false;
    }

    const int K = k + 1;

    auto it = movePairs.end();
    for (auto cand = movePairs.begin(); cand != movePairs.end(); ++cand) {
        int x = getAlias(cand->first, coalescedMoves);
        int y = getAlias(cand->second, coalescedMoves);
        if (Coloring::isMachineReg(x) || Coloring::isMachineReg(y)) {
            it = cand;
            break;
        }
        if (it == movePairs.end()) {
            it = cand;
        }
    }
    pair<int, int> move = *it;
    movePairs.erase(it);

    int x = getAlias(move.first, coalescedMoves);
    int y = getAlias(move.second, coalescedMoves);

    int u, v;
    if (Coloring::isMachineReg(y)) {
        u = y;
        v = x;
    } else {
        u = x;
        v = y;
    }

    if (u == v) {
        return true;
    }

    if (Coloring::isMachineReg(v) ||
        (graph.find(u) != graph.end() && graph[u].find(v) != graph[u].end())) {
        return false;
    }

    bool canCoalesce = false;
    if (Coloring::isMachineReg(u)) {
        canCoalesce = true;
        for (int t : adjacentNodes(v, graph)) {
            if (!okToCoalesce(t, u, graph, K)) {
                canCoalesce = false;
                break;
            }
        }
    } else {
        set<int> nodes = adjacentNodes(u, graph);
        set<int> adjV = adjacentNodes(v, graph);
        nodes.insert(adjV.begin(), adjV.end());
        canCoalesce = conservative(nodes, graph, K);
    }

    if (canCoalesce) {
        combineNodes(this, u, v);
        return true;
    }

    return false;
}

bool Coloring::freeze() {
#ifdef DEBUG
    cout << "Freezing..." << endl;
#endif
    const int K = k + 1;

    for (const auto& entry : graph) {
        int n = entry.first;
        if (!isMove(n)) {
            continue;
        }
        if (static_cast<int>(entry.second.size()) >= K) {
            continue;
        }
        freezeMovesFor(this, n);
        return true;
    }

    return false;
}

bool Coloring::spill() {
#ifdef DEBUG
    cout << "Spilling..." << endl;
#endif
    const int K = k + 1;

    int best = -1;
    int bestDegree = -1;

    for (const auto& entry : graph) {
        int n = entry.first;
        int deg = static_cast<int>(entry.second.size());
        if (deg < K) {
            continue;
        }
        if (isMachineReg(n)) {
            continue;
        }
        if (deg > bestDegree || (deg == bestDegree && (best == -1 || n < best))) {
            bestDegree = deg;
            best = n;
        }
    }

    if (best == -1) {
        return false;
    }

    freezeMovesFor(this, best);
    simplifiedNodes.push(best);
    eraseNode(best);
    return true;
}

bool Coloring::select() {
#ifdef DEBUG
    cout << "Selecting..." << endl;
#endif

    bool all_covered = true;
    const int K = k + 1;

    if (ig != nullptr) {
        for (const auto& entry : ig->graph) {
            if (Coloring::isMachineReg(entry.first)) {
                colors[entry.first] = entry.first;
            }
        }
    }

    while (!simplifiedNodes.empty()) {
        int n = simplifiedNodes.top();
        simplifiedNodes.pop();

        if (Coloring::isMachineReg(n)) {
            colors[n] = n;
            continue;
        }

        set<int> usedColors;
        if (ig != nullptr && ig->graph.find(n) != ig->graph.end()) {
            for (int neighbor : ig->graph.at(n)) {
                int alias = getAlias(neighbor, coalescedMoves);
                if (colors.find(alias) != colors.end()) {
                    int c = colors[alias];
                    if (c < K) {
                        usedColors.insert(c);
                    }
                }
            }
        }

        int chosen = -1;
        for (int c = 0; c < K; ++c) {
            if (usedColors.find(c) == usedColors.end()) {
                chosen = c;
                break;
            }
        }

        if (chosen == -1) {
            spilled.insert(n);
            all_covered = false;
        } else {
            colors[n] = chosen;
        }
    }

    for (const auto& entry : coalescedMoves) {
        int master = entry.first;
        if (colors.find(master) == colors.end()) {
            continue;
        }
        int masterColor = colors[master];
        for (int coalesced : entry.second) {
            if (spilled.find(master) != spilled.end()) {
                spilled.insert(coalesced);
            } else {
                colors[coalesced] = masterColor;
            }
        }
    }

    return all_covered;
}
