#include "loopinductionopt.hh"
#include "defusechain.hh"
#include <algorithm>
#include <map>

using namespace std;
using namespace quad;

static bool hasSideEffects(QuadStm* stm) {
    if (stm == nullptr) {
        return true;
    }
    switch (stm->kind) {
        case QuadKind::LABEL:
        case QuadKind::JUMP:
        case QuadKind::CJUMP:
        case QuadKind::CALL:
        case QuadKind::EXTCALL:
        case QuadKind::MOVE_CALL:
        case QuadKind::MOVE_EXTCALL:
        case QuadKind::STORE:
        case QuadKind::RETURN:
            return true;
        default:
            return false;
    }
}

static set<int> getDefTemps(QuadStm* stm) {
    set<int> defs;
    if (stm == nullptr || stm->def == nullptr) {
        return defs;
    }
    for (Temp* t : *stm->def) {
        if (t != nullptr) {
            defs.insert(t->num);
        }
    }
    return defs;
}

static set<int> getUseTemps(QuadStm* stm) {
    set<int> uses;
    if (stm == nullptr || stm->use == nullptr) {
        return uses;
    }
    for (Temp* t : *stm->use) {
        if (t != nullptr) {
            uses.insert(t->num);
        }
    }
    return uses;
}

static set<int> computeLiveTemps(QuadFuncDecl* func) {
    set<int> live;
    if (func == nullptr || func->quadblocklist == nullptr) {
        return live;
    }

    for (QuadBlock* block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) {
            continue;
        }
        for (QuadStm* stm : *block->quadlist) {
            if (hasSideEffects(stm)) {
                for (int u : getUseTemps(stm)) {
                    live.insert(u);
                }
            }
        }
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (QuadBlock* block : *func->quadblocklist) {
            if (block == nullptr || block->quadlist == nullptr) {
                continue;
            }
            for (QuadStm* stm : *block->quadlist) {
                set<int> defs = getDefTemps(stm);
                bool defLive = false;
                for (int d : defs) {
                    if (live.count(d)) {
                        defLive = true;
                        break;
                    }
                }
                if (defLive) {
                    for (int u : getUseTemps(stm)) {
                        if (!live.count(u)) {
                            live.insert(u);
                            changed = true;
                        }
                    }
                }
            }
        }
    }
    return live;
}

QuadFuncDecl* eliminateUnusedInductionVars(QuadFuncDecl* func) {
    if (func == nullptr || func->quadblocklist == nullptr) {
        return func;
    }

    bool changed = true;
    while (changed) {
        changed = false;
        set<int> live = computeLiveTemps(func);

        for (QuadBlock* block : *func->quadblocklist) {
            if (block == nullptr || block->quadlist == nullptr) {
                continue;
            }
            auto& quads = *block->quadlist;
            vector<QuadStm*> kept;
            for (QuadStm* stm : quads) {
                if (hasSideEffects(stm)) {
                    kept.push_back(stm);
                    continue;
                }
                set<int> defs = getDefTemps(stm);
                bool keep = false;
                for (int d : defs) {
                    if (live.count(d)) {
                        keep = true;
                        break;
                    }
                }
                if (keep) {
                    kept.push_back(stm);
                } else if (!defs.empty()) {
                    changed = true;
                }
            }
            if (kept.size() != quads.size()) {
                changed = true;
            }
            quads = kept;
        }
    }

    return func;
}
