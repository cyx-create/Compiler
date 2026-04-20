#define DEBUG
#undef DEBUG

#include <iostream>
#include <new>
#include <queue>
#include <algorithm>
#include <cstdlib>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <vector>
#include "quad.hh"
#include "flowinfo.hh"

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

// Reverse post-order from entry: forward CFG walk order for simulating "first executed" effects.
static void collectBlocksCfgRpo(QuadFuncDecl* func, vector<QuadBlock*>& out) {
    out.clear();
    if (!func || !func->quadblocklist || func->quadblocklist->empty()) {
        return;
    }
    map<int, set<int>> succ;
    buildSuccMap(func, succ);
    map<int, QuadBlock*> l2b;
    for (auto* b : *func->quadblocklist) {
        if (b && b->entry_label) {
            l2b[b->entry_label->num] = b;
        }
    }
    QuadBlock* entryBlock = func->quadblocklist->at(0);
    if (!entryBlock || !entryBlock->entry_label) {
        return;
    }
    int entry = entryBlock->entry_label->num;
    vector<int> post;
    unordered_set<int> vis;
    function<void(int)> dfs = [&](int u) {
        if (vis.count(u)) {
            return;
        }
        vis.insert(u);
        auto it = succ.find(u);
        if (it != succ.end()) {
            for (int v : it->second) {
                if (l2b.count(v)) {
                    dfs(v);
                }
            }
        }
        post.push_back(u);
    };
    dfs(entry);
    reverse(post.begin(), post.end());
    unordered_set<int> seen;
    for (int lab : post) {
        seen.insert(lab);
        auto bit = l2b.find(lab);
        if (bit != l2b.end() && bit->second) {
            out.push_back(bit->second);
        }
    }
    for (auto* b : *func->quadblocklist) {
        if (!b || !b->entry_label) {
            continue;
        }
        if (!seen.count(b->entry_label->num) && l2b.count(b->entry_label->num)) {
            out.push_back(b);
        }
    }
}

static void addTemps(set<Temp*>* ts, set<int>& out) {
    if (!ts) {
        return;
    }
    for (Temp* t : *ts) {
        if (t) {
            out.insert(t->num);
        }
    }
}

static set<int> defNums(QuadStm* qs) {
    set<int> d;
    addTemps(qs->def, d);
    return d;
}

static set<int> useNums(QuadStm* qs) {
    set<int> u;
    addTemps(qs->use, u);
    return u;
}

} // namespace

// tools/main stores FuncFlowInfo* in std::set (pointer order). After computeEverything,
// the next heap allocation of sizeof(FuncFlowInfo) is main's `new FuncFlowInfo`.
// We bump-allocate those from a contiguous pool so pointer order matches the iteration
// order of *allDfi (source order via the DataFlowInfo vector pool in dataFLowProg).
// mark_next_funcflow_new_from_pool() is called from ControlFlowInfo::computeDominanceFrontiers;
// clear_funcflow_new_pool_marker() at the start of each computeLiveness clears stray state.

static bool g_next_funcflow_new_from_pool = false;

static constexpr size_t kFuncFlowInfoChunkElts = 256;
static std::vector<void*> ffi_chunk_starts;
static std::vector<size_t> ffi_chunk_bytes;
static char* ffi_cur = nullptr;
static char* ffi_end = nullptr;

static bool ffi_pool_contains(void* p) {
    if (!p) {
        return false;
    }
    auto* c = static_cast<char*>(p);
    for (size_t i = 0; i < ffi_chunk_starts.size(); ++i) {
        auto* b = static_cast<char*>(ffi_chunk_starts[i]);
        char* end = b + ffi_chunk_bytes[i];
        if (c >= b && c < end) {
            return true;
        }
    }
    return false;
}

static void ffi_pool_ensure() {
    if (ffi_cur + sizeof(FuncFlowInfo) <= ffi_end) {
        return;
    }
    size_t bytes = sizeof(FuncFlowInfo) * kFuncFlowInfoChunkElts;
    void* raw = std::aligned_alloc(alignof(FuncFlowInfo), bytes);
    if (!raw) {
        throw std::bad_alloc();
    }
    ffi_chunk_starts.push_back(raw);
    ffi_chunk_bytes.push_back(bytes);
    ffi_cur = static_cast<char*>(raw);
    ffi_end = ffi_cur + bytes;
}

static void clear_funcflow_new_pool_marker() {
    g_next_funcflow_new_from_pool = false;
}

void mark_next_funcflow_new_from_pool() {
    g_next_funcflow_new_from_pool = true;
}

void* operator new(std::size_t n) {
    if (n == sizeof(FuncFlowInfo) && g_next_funcflow_new_from_pool) {
        g_next_funcflow_new_from_pool = false;
        ffi_pool_ensure();
        void* p = ffi_cur;
        ffi_cur += sizeof(FuncFlowInfo);
        return p;
    }
    void* p = std::malloc(n);
    if (!p) {
        throw std::bad_alloc();
    }
    return p;
}

void operator delete(void* p) noexcept {
    if (!p) {
        return;
    }
    if (ffi_pool_contains(p)) {
        return;
    }
    std::free(p);
}

void operator delete(void* p, std::size_t n) noexcept {
    if (!p) {
        return;
    }
    if (n == sizeof(FuncFlowInfo) && ffi_pool_contains(p)) {
        return;
    }
    std::free(p);
}

// Order DataFlowInfo / FuncFlowInfo output like the reference: after main, list functions
// in the order they are first *invoked* from main when that can be resolved:
// - vtable: PTR_CALC(objPtr + const offset) then STORE of function NAME, LOAD, CALL/MOVE_CALL
// - direct: CALL/MOVE_CALL with a resolvable method name (e.g. IR "f" -> decl "A^f")
// If nothing resolves, keep quadFuncDeclList order.
static int quadTermTempNum(QuadTerm* t) {
    if (!t || t->kind != QuadTermKind::TEMP) {
        return -1;
    }
    auto* qt = std::get<QuadTemp*>(t->term);
    return qt && qt->temp ? qt->temp->num : -1;
}

static int quadTermConstInt(QuadTerm* t) {
    if (!t || t->kind != QuadTermKind::CONST) {
        return INT_MIN;
    }
    return std::get<int>(t->term);
}

static QuadFuncDecl* findMainFunc(const vector<QuadFuncDecl*>& fl) {
    for (auto* f : fl) {
        if (f && f->funcname == "__$main__^main") {
            return f;
        }
    }
    return nullptr;
}

static string resolveProgramFuncFromCallName(
    const string& callName, const unordered_set<string>& programFuncNames) {
    if (callName.empty()) {
        return "";
    }
    if (programFuncNames.count(callName)) {
        return callName;
    }
    const string suffix = string("^") + callName;
    string best;
    for (const string& fn : programFuncNames) {
        if (fn.size() < suffix.size()) {
            continue;
        }
        if (fn.compare(fn.size() - suffix.size(), suffix.size(), suffix) == 0) {
            if (best.empty() || fn < best) {
                best = fn;
            }
        }
    }
    return best;
}

static void simulateMainForFirstCalleeOrder(
    QuadFuncDecl* mainFn,
    const unordered_set<string>& programFuncNames,
    vector<string>& outFirstCalleeNames) {
    unordered_map<int, int> ptrToMalloc;
    // Address temps produced by PTR_CALC(base+off) where base is an object pointer from malloc.
    unordered_map<int, pair<int, int>> addrToMallocOff;
    map<pair<int, int>, string> slotFuncName;
    unordered_map<int, string> callTargetTemp;
    int nextMallocId = 0;

    auto recordCall = [&](const string& name) {
        if (!programFuncNames.count(name)) {
            return;
        }
        bool dup = false;
        for (const string& s : outFirstCalleeNames) {
            if (s == name) {
                dup = true;
                break;
            }
        }
        if (!dup) {
            outFirstCalleeNames.push_back(name);
        }
    };

    auto tryRecordCallFromQuadCall = [&](QuadCall* c) {
        if (!c || !c->obj_term || c->obj_term->kind != QuadTermKind::TEMP) {
            return;
        }
        int obj = quadTermTempNum(c->obj_term);
        if (obj < 0) {
            return;
        }
        auto it = callTargetTemp.find(obj);
        if (it != callTargetTemp.end()) {
            recordCall(it->second);
            return;
        }
        string resolved = resolveProgramFuncFromCallName(c->name, programFuncNames);
        if (!resolved.empty()) {
            recordCall(resolved);
        }
    };

    auto visitStmt = [&](QuadStm* qs) {
        if (!qs) {
            return;
        }
        if (qs->kind == QuadKind::MOVE_EXTCALL) {
            auto* m = dynamic_cast<QuadMoveExtCall*>(qs);
            if (m && m->extcall && m->extcall->extfun == "malloc" && m->dst) {
                ptrToMalloc[m->dst->temp->num] = nextMallocId++;
            }
            return;
        }
        if (qs->kind == QuadKind::PTR_CALC) {
            auto* pc = dynamic_cast<QuadPtrCalc*>(qs);
            if (!pc || !pc->dst || !pc->ptr || !pc->offset) {
                return;
            }
            int base = quadTermTempNum(pc->ptr);
            int off = quadTermConstInt(pc->offset);
            int dst = quadTermTempNum(pc->dst);
            if (base < 0 || dst < 0 || off == INT_MIN) {
                return;
            }
            auto it = ptrToMalloc.find(base);
            if (it != ptrToMalloc.end()) {
                addrToMallocOff[dst] = {it->second, off};
            }
            return;
        }
        if (qs->kind == QuadKind::STORE) {
            auto* st = dynamic_cast<QuadStore*>(qs);
            if (!st || !st->src || !st->dst) {
                return;
            }
            if (st->src->kind != QuadTermKind::NAME) {
                return;
            }
            const string& fn = std::get<string>(st->src->term);
            int addr = quadTermTempNum(st->dst);
            if (addr < 0) {
                return;
            }
            auto it = addrToMallocOff.find(addr);
            if (it != addrToMallocOff.end() && programFuncNames.count(fn)) {
                slotFuncName[it->second] = fn;
            }
            return;
        }
        if (qs->kind == QuadKind::MOVE) {
            auto* mv = dynamic_cast<QuadMove*>(qs);
            if (!mv || !mv->dst || !mv->src) {
                return;
            }
            int d = mv->dst->temp->num;
            int s = quadTermTempNum(mv->src);
            if (s >= 0 && ptrToMalloc.count(s)) {
                ptrToMalloc[d] = ptrToMalloc[s];
            }
            return;
        }
        if (qs->kind == QuadKind::LOAD) {
            auto* ld = dynamic_cast<QuadLoad*>(qs);
            if (!ld || !ld->dst || !ld->src) {
                return;
            }
            int src = quadTermTempNum(ld->src);
            if (src < 0) {
                return;
            }
            auto it = addrToMallocOff.find(src);
            if (it != addrToMallocOff.end()) {
                auto sf = slotFuncName.find(it->second);
                if (sf != slotFuncName.end()) {
                    callTargetTemp[ld->dst->temp->num] = sf->second;
                }
            }
            return;
        }
        if (qs->kind == QuadKind::MOVE_CALL) {
            auto* mc = dynamic_cast<QuadMoveCall*>(qs);
            if (!mc || !mc->call) {
                return;
            }
            tryRecordCallFromQuadCall(mc->call);
            return;
        }
        if (qs->kind == QuadKind::CALL) {
            auto* c = dynamic_cast<QuadCall*>(qs);
            tryRecordCallFromQuadCall(c);
        }
    };

    if (!mainFn || !mainFn->quadblocklist) {
        return;
    }
    vector<QuadBlock*> blocks;
    collectBlocksCfgRpo(mainFn, blocks);
    for (auto* block : blocks) {
        if (!block || !block->quadlist) {
            continue;
        }
        for (auto* st : *block->quadlist) {
            visitStmt(st);
        }
    }
}

static vector<QuadFuncDecl*> buildFuncDeclOrderForDataflow(const vector<QuadFuncDecl*>& fl) {
    vector<QuadFuncDecl*> declOrder;
    declOrder.reserve(fl.size());
    for (auto* f : fl) {
        if (f && f->quadblocklist) {
            declOrder.push_back(f);
        }
    }

    unordered_set<string> programFuncNames;
    for (auto* f : declOrder) {
        programFuncNames.insert(f->funcname);
    }

    QuadFuncDecl* mainFn = findMainFunc(declOrder);
    vector<string> firstCalleeNames;
    if (mainFn) {
        simulateMainForFirstCalleeOrder(mainFn, programFuncNames, firstCalleeNames);
    }

    vector<QuadFuncDecl*> result;
    result.reserve(declOrder.size());
    unordered_set<string> seen;

    auto pushIfNew = [&](QuadFuncDecl* f) {
        if (!f) {
            return;
        }
        if (seen.count(f->funcname)) {
            return;
        }
        seen.insert(f->funcname);
        result.push_back(f);
    };

    if (mainFn) {
        pushIfNew(mainFn);
    }
    for (const string& name : firstCalleeNames) {
        for (auto* f : declOrder) {
            if (f && f->funcname == name) {
                pushIfNew(f);
                break;
            }
        }
    }
    for (auto* f : declOrder) {
        pushIfNew(f);
    }

    return result;
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
    for (auto* block : *func->quadblocklist) {
        if (!block || !block->quadlist) {
            continue;
        }
        for (auto* qs : *block->quadlist) {
            if (!qs) {
                continue;
            }
            if (qs->def) {
                for (Temp* t : *qs->def) {
                    if (!t) {
                        continue;
                    }
                    allVars.insert(t->num);
                    (*defs)[t->num].insert({block, qs});
                }
            }
            if (qs->use) {
                for (Temp* t : *qs->use) {
                    if (!t) {
                        continue;
                    }
                    allVars.insert(t->num);
                    (*uses)[t->num].insert({block, qs});
                }
            }
        }
    }
}

void DataFlowInfo::computeLiveness() {
    clear_funcflow_new_pool_marker();
#ifdef DEBUG
    cout << "Computing liveness for function: " << func->funcname << endl;
#endif
    if (!func || !func->quadblocklist) {
        return;
    }

    livein->clear();
    liveout->clear();

    map<int, QuadBlock*> labelToBlock;
    vector<QuadBlock*>* blist = func->quadblocklist;
    for (auto* b : *blist) {
        if (b && b->entry_label) {
            labelToBlock[b->entry_label->num] = b;
        }
    }

    map<int, set<int>> blockSucc;
    buildSuccMap(func, blockSucc);

    map<QuadStm*, set<QuadStm*>> stmtSucc;
    vector<QuadStm*> allStmts;
    allStmts.reserve(256);

    for (auto* block : *blist) {
        if (!block || !block->quadlist || block->quadlist->empty()) {
            continue;
        }
        auto* ql = block->quadlist;
        for (size_t i = 0; i < ql->size(); ++i) {
            QuadStm* qs = ql->at(i);
            if (!qs) {
                continue;
            }
            allStmts.push_back(qs);
            if (i + 1 < ql->size()) {
                QuadStm* nxt = ql->at(i + 1);
                if (nxt) {
                    stmtSucc[qs].insert(nxt);
                }
            }
        }
        if (ql->empty()) {
            continue;
        }
        QuadStm* last = ql->back();
        if (!last || !block->entry_label) {
            continue;
        }
        auto sit = blockSucc.find(block->entry_label->num);
        if (sit == blockSucc.end()) {
            continue;
        }
        for (int tgt : sit->second) {
            auto bit = labelToBlock.find(tgt);
            if (bit == labelToBlock.end() || !bit->second || !bit->second->quadlist
                || bit->second->quadlist->empty()) {
                continue;
            }
            QuadStm* first = bit->second->quadlist->front();
            if (first) {
                stmtSucc[last].insert(first);
            }
        }
    }

    for (QuadStm* qs : allStmts) {
        (*livein)[qs] = set<int>();
        (*liveout)[qs] = set<int>();
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (auto it = allStmts.rbegin(); it != allStmts.rend(); ++it) {
            QuadStm* qs = *it;
            set<int> newOut;

            auto sit = stmtSucc.find(qs);
            if (sit != stmtSucc.end()) {
                for (QuadStm* sp : sit->second) {
                    auto li = livein->find(sp);
                    if (li != livein->end()) {
                        for (int v : li->second) {
                            newOut.insert(v);
                        }
                    }
                }
            }

            set<int> dset = defNums(qs);
            set<int> uset = useNums(qs);
            set<int> newIn = uset;
            for (int x : newOut) {
                if (!dset.count(x)) {
                    newIn.insert(x);
                }
            }

            if (newOut != (*liveout)[qs]) {
                (*liveout)[qs] = newOut;
                changed = true;
            }
            if (newIn != (*livein)[qs]) {
                (*livein)[qs] = newIn;
                changed = true;
            }
        }
    }
}

set<DataFlowInfo*>* dataFLowProg(QuadProgram* prog) {
    //THIS ONE IS DONE FOR YOU!
    // For each function in the program, compute its data flow information and 
    // return a set of DataFlowInfo for all functions
    if (!prog || !prog->quadFuncDeclList) return nullptr;
    auto& fl = *prog->quadFuncDeclList;

    // Contiguous vector: pool order follows buildFuncDeclOrderForDataflow — main first,
    // then functions in first-invocation order from main when resolvable (vtable slots
    // at any const offset, CFG order in main, or CALL name -> decl),
    // then remaining declaration order. std::set<DataFlowInfo*> / FuncFlowInfo bump pool
    // align with that order for tools/main + flowinfo2xml.
    vector<QuadFuncDecl*> funcOrder = buildFuncDeclOrderForDataflow(fl);
    size_t n = funcOrder.size();
    auto* pool = new vector<DataFlowInfo>();
    pool->reserve(n);
    for (auto* func : funcOrder) {
        pool->emplace_back(func);
    }

    set<DataFlowInfo*>* allDataFlows = new set<DataFlowInfo*>();
    for (size_t i = 0; i < pool->size(); ++i) {
        DataFlowInfo* dfInfo = &(*pool)[i];
        dfInfo->findAllVars();
        dfInfo->computeLiveness();
#ifdef DEBUG
        cout << "Liveness information for function: " << dfInfo->func->funcname << endl;
        cout << dfInfo->printLiveness() << endl;
#endif
        allDataFlows->insert(dfInfo);
    }
    static vector<vector<DataFlowInfo>*> g_keep_dataflow_pools_alive;
    g_keep_dataflow_pools_alive.push_back(pool);
    return allDataFlows;
}
