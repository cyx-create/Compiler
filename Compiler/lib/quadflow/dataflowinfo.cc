#define DEBUG
#undef DEBUG

#include <iostream>
#include <queue>
#include <algorithm>
#include <map>
#include <set>
#include "quad.hh"
#include "flowinfo.hh"

using namespace std;
using namespace quad;

static set<int> tempSetFromPtrSet(set<Temp *> *temps) {
    set<int> out;
    if (!temps) {
        return out;
    }
    for (Temp *t : *temps) {
        if (t) {
            out.insert(t->num);
        }
    }
    return out;
}

static set<int> stmtUseSet(QuadStm *stm) {
    return tempSetFromPtrSet(stm ? stm->use : nullptr);
}

static set<int> stmtDefSet(QuadStm *stm) {
    return tempSetFromPtrSet(stm ? stm->def : nullptr);
}

static set<int> setUnion(const set<int> &a, const set<int> &b) {
    set<int> out = a;
    out.insert(b.begin(), b.end());
    return out;
}

static set<int> setDiff(const set<int> &a, const set<int> &b) {
    set<int> out;
    set_difference(a.begin(), a.end(), b.begin(), b.end(),
                   inserter(out, out.begin()));
    return out;
}

void DataFlowInfo::findAllVars() {
#ifdef DEBUG
    cout << "Finding all variables in function: " << func->funcname << endl;
#endif
    allVars.clear();
    defs->clear();
    uses->clear();
    if (!func || !func->quadblocklist) {
        return;
    }

    for (QuadBlock *block : *func->quadblocklist) {
        if (!block || !block->quadlist) {
            continue;
        }
        for (QuadStm *stm : *block->quadlist) {
            if (!stm) {
                continue;
            }
            if (stm->def) {
                for (Temp *t : *stm->def) {
                    if (!t) {
                        continue;
                    }
                    allVars.insert(t->num);
                    (*defs)[t->num].insert({block, stm});
                }
            }
            if (stm->use) {
                for (Temp *t : *stm->use) {
                    if (!t) {
                        continue;
                    }
                    allVars.insert(t->num);
                    (*uses)[t->num].insert({block, stm});
                }
            }
        }
    }
}

void DataFlowInfo::computeLiveness() {
#ifdef DEBUG
    cout << "Computing liveness for function: " << func->funcname << endl;
#endif
    livein->clear();
    liveout->clear();
    if (!func || !func->quadblocklist) {
        return;
    }

    map<int, QuadBlock *> labelToBlock;
    map<int, set<int>> successors;
    for (QuadBlock *block : *func->quadblocklist) {
        if (!block || !block->entry_label) {
            continue;
        }
        int bid = block->entry_label->num;
        labelToBlock[bid] = block;
        if (block->exit_labels) {
            for (Label *lbl : *block->exit_labels) {
                if (lbl) {
                    successors[bid].insert(lbl->num);
                }
            }
        }
    }

    map<QuadBlock *, set<int>> blockLiveIn;
    map<QuadBlock *, set<int>> blockLiveOut;
    for (QuadBlock *block : *func->quadblocklist) {
        if (block) {
            blockLiveIn[block] = set<int>();
            blockLiveOut[block] = set<int>();
        }
    }

    map<QuadBlock *, set<int>> blockUse;
    map<QuadBlock *, set<int>> blockDef;
    for (QuadBlock *block : *func->quadblocklist) {
        if (!block || !block->quadlist) {
            continue;
        }
        set<int> useB;
        set<int> defB;
        for (QuadStm *stm : *block->quadlist) {
            if (!stm || stm->kind == QuadKind::LABEL) {
                continue;
            }
            useB = setUnion(useB, stmtUseSet(stm));
            defB = setUnion(defB, stmtDefSet(stm));
        }
        blockUse[block] = useB;
        blockDef[block] = defB;
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (QuadBlock *block : *func->quadblocklist) {
            if (!block || !block->entry_label) {
                continue;
            }
            set<int> newOut;
            for (int succId : successors[block->entry_label->num]) {
                auto it = labelToBlock.find(succId);
                if (it != labelToBlock.end()) {
                    newOut = setUnion(newOut, blockLiveIn[it->second]);
                }
            }
            set<int> newIn =
                setUnion(blockUse[block], setDiff(newOut, blockDef[block]));
            if (newOut != blockLiveOut[block] || newIn != blockLiveIn[block]) {
                blockLiveOut[block] = newOut;
                blockLiveIn[block] = newIn;
                changed = true;
            }
        }
    }

    for (QuadBlock *block : *func->quadblocklist) {
        if (!block || !block->quadlist || block->quadlist->empty()) {
            continue;
        }
        set<int> runningOut = blockLiveOut[block];
        for (int i = (int)block->quadlist->size() - 1; i >= 0; --i) {
            QuadStm *stm = block->quadlist->at(i);
            if (!stm) {
                continue;
            }
            if (stm->kind == QuadKind::LABEL) {
                (*liveout)[stm] = runningOut;
                (*livein)[stm] = runningOut;
                continue;
            }
            set<int> use = stmtUseSet(stm);
            set<int> def = stmtDefSet(stm);
            (*liveout)[stm] = runningOut;
            (*livein)[stm] = setUnion(use, setDiff(runningOut, def));
            runningOut = (*livein)[stm];
        }
    }
}

set<DataFlowInfo *> *dataFLowProg(QuadProgram *prog) {
    if (!prog || !prog->quadFuncDeclList) {
        return nullptr;
    }
    set<DataFlowInfo *> *allDataFlows = new set<DataFlowInfo *>();
    for (auto func : *prog->quadFuncDeclList) {
        if (!func || !func->quadblocklist) {
            continue;
        }

        DataFlowInfo *dfInfo = new DataFlowInfo(func);
        dfInfo->findAllVars();
        dfInfo->computeLiveness();
#ifdef DEBUG
        cout << "Liveness information for function: " << func->funcname << endl;
        cout << dfInfo->printLiveness() << endl;
#endif
        allDataFlows->insert(dfInfo);
    }
    return allDataFlows;
}
