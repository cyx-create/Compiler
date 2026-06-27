#define DEBUG
#undef DEBUG

#include <algorithm>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "quad.hh"
#include "opt.hh"

using namespace std;
using namespace quad;
using namespace tree;

namespace {

using EdgeKey = pair<int, int>;

inline ValueType vt(const RtValue& r) { return const_cast<RtValue&>(r).getType(); }
inline int vi(const RtValue& r) { return const_cast<RtValue&>(r).getIntValue(); }

bool rtEqual(const RtValue& a, const RtValue& b) {
    if (vt(a) != vt(b)) return false;
    if (vt(a) == ValueType::ONE_VALUE) return vi(a) == vi(b);
    return true;
}

RtValue meetVal(const RtValue& a, const RtValue& b) {
    if (vt(a) == ValueType::NO_VALUE) return b;
    if (vt(b) == ValueType::NO_VALUE) return a;
    if (vt(a) == ValueType::MANY_VALUES || vt(b) == ValueType::MANY_VALUES)
        return RtValue(ValueType::MANY_VALUES);
    if (vt(a) == ValueType::ONE_VALUE && vt(b) == ValueType::ONE_VALUE) {
        if (vi(a) == vi(b)) return RtValue(vi(a));
        return RtValue(ValueType::MANY_VALUES);
    }
    return RtValue(ValueType::MANY_VALUES);
}

RtValue widenNoToMany(const RtValue& v) {
    if (vt(v) == ValueType::NO_VALUE) return RtValue(ValueType::MANY_VALUES);
    return v;
}

bool mergeTemp(map<int, RtValue>& tv, int n, const RtValue& nv, bool& changed) {
    auto it = tv.find(n);
    if (it == tv.end()) {
        tv.emplace(n, nv);
        if (vt(nv) != ValueType::NO_VALUE) changed = true;
        return true;
    }
    RtValue m = meetVal(it->second, nv);
    if (!rtEqual(it->second, m)) {
        it->second = m;
        changed = true;
        return true;
    }
    return false;
}

QuadStm* getTerminator(QuadBlock* b) {
    if (!b->quadlist) return nullptr;
    for (int i = (int)b->quadlist->size() - 1; i >= 0; --i) {
        QuadStm* s = b->quadlist->at(i);
        if (s->kind == QuadKind::JUMP || s->kind == QuadKind::CJUMP || s->kind == QuadKind::RETURN)
            return s;
    }
    return nullptr;
}

RtValue latticeReadTemp(map<int, RtValue>& tv, int n, bool& changed) {
    RtValue v;
    auto it = tv.find(n);
    if (it == tv.end()) v = RtValue();
    else v = it->second;
    if (vt(v) == ValueType::NO_VALUE) {
        cerr << "Warning: undefined use of temp t" << n << ", treating as MANY_VALUES\n";
        v = RtValue(ValueType::MANY_VALUES);
        tv[n] = v;
        changed = true;
    }
    return v;
}

RtValue evalTermLattice(map<int, RtValue>& tv, QuadTerm* q, bool& changed) {
    if (!q) return RtValue(ValueType::MANY_VALUES);
    if (q->kind == QuadTermKind::CONST) return RtValue(q->get_const());
    if (q->kind == QuadTermKind::NAME) return RtValue(ValueType::MANY_VALUES);
    if (q->kind == QuadTermKind::TEMP) {
        int n = q->get_temp()->temp->num;
        return latticeReadTemp(tv, n, changed);
    }
    return RtValue(ValueType::MANY_VALUES);
}

optional<bool> evalRelop(const string& rel, int a, int b) {
    if (rel == "==") return a == b;
    if (rel == "!=") return a != b;
    if (rel == "<") return a < b;
    if (rel == ">") return a > b;
    if (rel == "<=") return a <= b;
    if (rel == ">=") return a >= b;
    return nullopt;
}

optional<int> frozenConstTerm(const map<int, RtValue>& tv, QuadTerm* q) {
    if (!q) return nullopt;
    if (q->kind == QuadTermKind::CONST) return q->get_const();
    if (q->kind == QuadTermKind::NAME) return nullopt;
    if (q->kind == QuadTermKind::TEMP) {
        int n = q->get_temp()->temp->num;
        auto it = tv.find(n);
        if (it == tv.end()) return nullopt;
        if (vt(it->second) == ValueType::ONE_VALUE) return vi(it->second);
        return nullopt;
    }
    return nullopt;
}

optional<bool> frozenCjump(const map<int, RtValue>& tv, QuadCJump* cj, QuadTerm* left,
                           QuadTerm* right) {
    auto a = frozenConstTerm(tv, left);
    auto b = frozenConstTerm(tv, right);
    if (!a || !b) return nullopt;
    auto v = evalRelop(cj->relop, *a, *b);
    if (!v) return nullopt;
    return *v;
}

bool divOrModZeroUnsafe(const string& op, const RtValue& right) {
    if (op != "/" && op != "%") return false;
    return vt(right) == ValueType::ONE_VALUE && vi(right) == 0;
}

RtValue evalBinop(const string& op, const RtValue& L, const RtValue& R, bool& changed) {
    if (vt(L) == ValueType::MANY_VALUES || vt(R) == ValueType::MANY_VALUES)
        return RtValue(ValueType::MANY_VALUES);
    if (vt(L) == ValueType::NO_VALUE || vt(R) == ValueType::NO_VALUE)
        return RtValue(ValueType::MANY_VALUES);
    if (divOrModZeroUnsafe(op, R)) return RtValue(ValueType::MANY_VALUES);
    int a = vi(L);
    int b = vi(R);
    int r = 0;
    if (op == "+") r = a + b;
    else if (op == "-") r = a - b;
    else if (op == "*") r = a * b;
    else if (op == "/") {
        if (b == 0) return RtValue(ValueType::MANY_VALUES);
        r = a / b;
    } else if (op == "%") {
        if (b == 0) return RtValue(ValueType::MANY_VALUES);
        r = a % b;
    } else
        return RtValue(ValueType::MANY_VALUES);
    (void)changed;
    return RtValue(r);
}

void inferEdgesRebuild(Opt& o, const map<int, bool>& blockExe, const map<int, RtValue>& temps,
                       map<EdgeKey, bool>& edges, bool* changedOut) {
    map<EdgeKey, bool> newE;
    for (QuadBlock* blk : *o.func->quadblocklist) {
        int bl = blk->entry_label->num;
        if (!blockExe.count(bl) || !blockExe.at(bl)) continue;
        QuadStm* t = getTerminator(blk);
        if (!t) continue;
        if (t->kind == QuadKind::JUMP) {
            auto* j = static_cast<QuadJump*>(t);
            newE[{bl, j->label->num}] = true;
        } else if (t->kind == QuadKind::CJUMP) {
            auto* cj = static_cast<QuadCJump*>(t);
            auto cond = frozenCjump(temps, cj, cj->left, cj->right);
            if (!cond.has_value()) {
                newE[{bl, cj->t->num}] = true;
                newE[{bl, cj->f->num}] = true;
            } else if (*cond)
                newE[{bl, cj->t->num}] = true;
            else
                newE[{bl, cj->f->num}] = true;
        }
    }
    if (edges != newE) {
        if (changedOut) *changedOut = true;
        edges.swap(newE);
    }
}

bool mapsEqualRt(const map<int, RtValue>& a, const map<int, RtValue>& b) {
    if (a.size() != b.size()) return false;
    for (const auto& p : a) {
        auto it = b.find(p.first);
        if (it == b.end()) return false;
        if (!rtEqual(p.second, it->second)) return false;
    }
    return true;
}

void propagateReachability(int entry, const map<EdgeKey, bool>& edges, map<int, bool>& reach) {
    reach.clear();
    reach[entry] = true;
    bool grew = true;
    while (grew) {
        grew = false;
        for (const auto& e : edges) {
            if (!e.second) continue;
            int p = e.first.first;
            int s = e.first.second;
            if (reach[p] && !reach[s]) {
                reach[s] = true;
                grew = true;
            }
        }
    }
}

void interpretBlock(QuadBlock* blk, const map<EdgeKey, bool>& edges, map<int, RtValue>& tv,
                    bool& changed) {
    int my = blk->entry_label->num;

    for (QuadStm* s : *blk->quadlist) {
        if (s->kind == QuadKind::PHI) {
            auto* ph = static_cast<QuadPhi*>(s);
            int d = ph->temp_exp->temp->num;
            RtValue acc(ValueType::NO_VALUE);
            bool any = false;
            for (auto& pr : *ph->args) {
                int pred = pr.second->num;
                EdgeKey k{pred, my};
                if (!edges.count(k) || !edges.at(k)) continue;
                any = true;
                RtValue v = latticeReadTemp(tv, pr.first->num, changed);
                v = widenNoToMany(v);
                acc = meetVal(acc, v);
            }
            if (any) mergeTemp(tv, d, acc, changed);
        } else if (s->kind == QuadKind::MOVE) {
            auto* m = static_cast<QuadMove*>(s);
            int d = m->dst->temp->num;
            RtValue v = evalTermLattice(tv, m->src, changed);
            mergeTemp(tv, d, v, changed);
        } else if (s->kind == QuadKind::MOVE_BINOP) {
            auto* mb = static_cast<QuadMoveBinop*>(s);
            int d = mb->dst->temp->num;
            RtValue L = evalTermLattice(tv, mb->left, changed);
            RtValue R = evalTermLattice(tv, mb->right, changed);
            mergeTemp(tv, d, evalBinop(mb->binop, L, R, changed), changed);
        } else if (s->kind == QuadKind::LOAD) {
            auto* ld = static_cast<QuadLoad*>(s);
            mergeTemp(tv, ld->dst->temp->num, RtValue(ValueType::MANY_VALUES), changed);
        } else if (s->kind == QuadKind::MOVE_CALL || s->kind == QuadKind::MOVE_EXTCALL) {
            QuadTemp* dst = nullptr;
            if (s->kind == QuadKind::MOVE_CALL)
                dst = static_cast<QuadMoveCall*>(s)->dst;
            else
                dst = static_cast<QuadMoveExtCall*>(s)->dst;
            mergeTemp(tv, dst->temp->num, RtValue(ValueType::MANY_VALUES), changed);
        } else if (s->kind == QuadKind::PTR_CALC) {
            auto* pc = static_cast<QuadPtrCalc*>(s);
            if (pc->dst && pc->dst->kind == QuadTermKind::TEMP)
                mergeTemp(tv, pc->dst->get_temp()->temp->num, RtValue(ValueType::MANY_VALUES), changed);
        }
    }
}

void buildLabel2Block(Opt& o) {
    o.label2block.clear();
    for (QuadBlock* b : *o.func->quadblocklist)
        o.label2block[b->entry_label->num] = b;
}

} // namespace

void Opt::calculateBT() {
    buildLabel2Block(*this);
    temp_value.clear();
    block_executable.clear();

    if (func->quadblocklist->empty()) return;

    int entry = func->quadblocklist->at(0)->entry_label->num;
    block_executable[entry] = true;
    map<EdgeKey, bool> edges;

    bool globalChanged = true;
    while (globalChanged) {
        globalChanged = false;
        map<int, RtValue> snap_tv = temp_value;
        inferEdgesRebuild(*this, block_executable, temp_value, edges, &globalChanged);
        map<int, bool> reach;
        propagateReachability(entry, edges, reach);
        for (QuadBlock* blk : *func->quadblocklist) {
            int lb = blk->entry_label->num;
            bool r = reach.count(lb) && reach[lb];
            bool prev = block_executable.count(lb) && block_executable[lb];
            if (r != prev) globalChanged = true;
            if (r)
                block_executable[lb] = true;
            else
                block_executable.erase(lb);
        }
        map<int, RtValue> tv = temp_value;
        while (true) {
            map<int, RtValue> before = tv;
            bool bump = false;
            for (QuadBlock* blk : *func->quadblocklist) {
                int lb = blk->entry_label->num;
                if (!block_executable.count(lb) || !block_executable[lb]) continue;
                interpretBlock(blk, edges, tv, bump);
            }
            if (mapsEqualRt(before, tv)) break;
            globalChanged = true;
        }
        if (!mapsEqualRt(tv, snap_tv)) globalChanged = true;
        temp_value.swap(tv);
    }
}

namespace {

RtValue peekTempLattice(const Opt& o, int n) {
    auto it = o.temp_value.find(n);
    if (it != o.temp_value.end()) return it->second;
    return RtValue();
}

QuadTerm* substTerm(Opt& o, QuadTerm* q) {
    if (!q) return nullptr;
    if (q->kind == QuadTermKind::CONST || q->kind == QuadTermKind::NAME) return q->clone();
    if (q->kind == QuadTermKind::TEMP) {
        int n = q->get_temp()->temp->num;
        RtValue rv = peekTempLattice(o, n);
        if (vt(rv) == ValueType::ONE_VALUE) return new QuadTerm(vi(rv));
        return q->clone();
    }
    return q->clone();
}

set<Temp*>* singleTempUse(Temp* t) {
    set<Temp*>* u = new set<Temp*>();
    u->insert(t);
    return u;
}

set<Temp*>* singleTempDef(Temp* t) {
    set<Temp*>* d = new set<Temp*>();
    d->insert(t);
    return d;
}

void collectTermUses(QuadTerm* q, map<int, int>& usec) {
    if (!q || q->kind != QuadTermKind::TEMP) return;
    usec[q->get_temp()->temp->num]++;
}

void countStmtUses(QuadStm* s, map<int, int>& usec) {
    switch (s->kind) {
        case QuadKind::MOVE:
            collectTermUses(static_cast<QuadMove*>(s)->src, usec);
            break;
        case QuadKind::LOAD:
            collectTermUses(static_cast<QuadLoad*>(s)->src, usec);
            break;
        case QuadKind::STORE: {
            auto* st = static_cast<QuadStore*>(s);
            collectTermUses(st->src, usec);
            collectTermUses(st->dst, usec);
            break;
        }
        case QuadKind::MOVE_BINOP: {
            auto* m = static_cast<QuadMoveBinop*>(s);
            collectTermUses(m->left, usec);
            collectTermUses(m->right, usec);
            break;
        }
        case QuadKind::MOVE_CALL: {
            auto* mc = static_cast<QuadMoveCall*>(s);
            QuadCall* c = mc->call;
            collectTermUses(c->obj_term, usec);
            for (QuadTerm* a : *c->args) collectTermUses(a, usec);
            break;
        }
        case QuadKind::CALL: {
            auto* c = static_cast<QuadCall*>(s);
            collectTermUses(c->obj_term, usec);
            for (QuadTerm* a : *c->args) collectTermUses(a, usec);
            break;
        }
        case QuadKind::EXTCALL:
        case QuadKind::MOVE_EXTCALL: {
            vector<QuadTerm*>* args =
                s->kind == QuadKind::EXTCALL ? static_cast<QuadExtCall*>(s)->args
                                              : static_cast<QuadMoveExtCall*>(s)->extcall->args;
            for (QuadTerm* a : *args) collectTermUses(a, usec);
            break;
        }
        case QuadKind::CJUMP: {
            auto* cj = static_cast<QuadCJump*>(s);
            collectTermUses(cj->left, usec);
            collectTermUses(cj->right, usec);
            break;
        }
        case QuadKind::RETURN:
            collectTermUses(static_cast<QuadReturn*>(s)->exp, usec);
            break;
        case QuadKind::PHI: {
            auto* ph = static_cast<QuadPhi*>(s);
            for (auto& pr : *ph->args) usec[pr.first->num]++;
            break;
        }
        case QuadKind::PTR_CALC: {
            auto* p = static_cast<QuadPtrCalc*>(s);
            collectTermUses(p->ptr, usec);
            collectTermUses(p->offset, usec);
            break;
        }
        default:
            break;
    }
}

int getDefTemp(QuadStm* s) {
    if (s->kind == QuadKind::MOVE) return static_cast<QuadMove*>(s)->dst->temp->num;
    if (s->kind == QuadKind::LOAD) return static_cast<QuadLoad*>(s)->dst->temp->num;
    if (s->kind == QuadKind::MOVE_BINOP) return static_cast<QuadMoveBinop*>(s)->dst->temp->num;
    if (s->kind == QuadKind::MOVE_CALL) return static_cast<QuadMoveCall*>(s)->dst->temp->num;
    if (s->kind == QuadKind::MOVE_EXTCALL) return static_cast<QuadMoveExtCall*>(s)->dst->temp->num;
    if (s->kind == QuadKind::PHI) return static_cast<QuadPhi*>(s)->temp_exp->temp->num;
    if (s->kind == QuadKind::PTR_CALC) {
        auto* p = static_cast<QuadPtrCalc*>(s);
        if (p->dst && p->dst->kind == QuadTermKind::TEMP) return p->dst->get_temp()->temp->num;
    }
    return -1;
}

bool storeMightBeObservable(QuadStm* s) {
    return s->kind == QuadKind::STORE || s->kind == QuadKind::CALL || s->kind == QuadKind::EXTCALL;
}

bool moveBinopIsDivLike(const string& op) { return op == "/" || op == "%"; }

bool moveBinopStmtIsDivLike(QuadStm* s) {
    return s->kind == QuadKind::MOVE_BINOP &&
           moveBinopIsDivLike(static_cast<QuadMoveBinop*>(s)->binop);
}

void recomputeExitLabels(QuadBlock* b) {
    QuadStm* t = getTerminator(b);
    if (!b->exit_labels) b->exit_labels = new vector<Label*>();
    b->exit_labels->clear();
    if (!t) return;
    if (t->kind == QuadKind::JUMP) b->exit_labels->push_back(static_cast<QuadJump*>(t)->label);
    else if (t->kind == QuadKind::CJUMP) {
        auto* cj = static_cast<QuadCJump*>(t);
        b->exit_labels->push_back(cj->t);
        b->exit_labels->push_back(cj->f);
    }
}

void transformStatementList(Opt& o, QuadBlock* blk, const map<EdgeKey, bool>& edges) {
    int my = blk->entry_label->num;
    vector<QuadStm*>* out = new vector<QuadStm*>();

    for (QuadStm* s : *blk->quadlist) {
        if (s->kind == QuadKind::PHI) {
            auto* ph = static_cast<QuadPhi*>(s);
            vector<pair<Temp*, Label*>>* na = new vector<pair<Temp*, Label*>>();
            for (auto& pr : *ph->args) {
                if (edges.count({pr.second->num, my}) && edges.at({pr.second->num, my}))
                    na->push_back(pr);
            }
            if (na->empty()) {
                delete na;
                continue;
            }
            if (na->size() == 1) {
                Temp* srcT = na->at(0).first;
                RtValue rv = peekTempLattice(o, srcT->num);
                QuadTemp* dst = ph->temp_exp;
                set<Temp*>* def = singleTempDef(dst->temp);
                if (vt(rv) == ValueType::ONE_VALUE) {
                    QuadTerm* src = new QuadTerm(vi(rv));
                    set<Temp*>* use = new set<Temp*>();
                    out->push_back(new QuadMove(dst, src, def, use));
                } else {
                    QuadTerm* src = new QuadTerm(new QuadTemp(new Temp(srcT->num), dst->type));
                    set<Temp*>* use = singleTempUse(srcT);
                    out->push_back(new QuadMove(dst, src, def, use));
                }
                delete na;
                continue;
            }
            ph->args = na;
            out->push_back(s);
            continue;
        }

        if (s->kind == QuadKind::MOVE) {
            auto* m = static_cast<QuadMove*>(s);
            QuadTerm* ns = substTerm(o, m->src);
            out->push_back(new QuadMove(m->dst, ns, m->def, m->use));
            continue;
        }
        if (s->kind == QuadKind::LOAD) {
            auto* ld = static_cast<QuadLoad*>(s);
            out->push_back(new QuadLoad(ld->dst, substTerm(o, ld->src), ld->def, ld->use));
            continue;
        }
        if (s->kind == QuadKind::STORE) {
            auto* st = static_cast<QuadStore*>(s);
            out->push_back(
                new QuadStore(substTerm(o, st->src), substTerm(o, st->dst), st->def, st->use));
            continue;
        }
        if (s->kind == QuadKind::MOVE_BINOP) {
            auto* mb = static_cast<QuadMoveBinop*>(s);
            RtValue rv = peekTempLattice(o, mb->dst->temp->num);
            if (vt(rv) == ValueType::ONE_VALUE) {
                out->push_back(new QuadMove(mb->dst, new QuadTerm(vi(rv)), mb->def, mb->use));
            } else {
                out->push_back(new QuadMoveBinop(mb->dst, substTerm(o, mb->left), mb->binop,
                                                 substTerm(o, mb->right), mb->def, mb->use));
            }
            continue;
        }
        if (s->kind == QuadKind::CJUMP) {
            auto* cj = static_cast<QuadCJump*>(s);
            QuadTerm* nl = substTerm(o, cj->left);
            QuadTerm* nr = substTerm(o, cj->right);
            auto cond = frozenCjump(o.temp_value, cj, nl, nr);
            if (cond.has_value()) {
                Label* tgt = *cond ? cj->t : cj->f;
                out->push_back(new QuadJump(tgt, cj->def, cj->use));
            } else
                out->push_back(new QuadCJump(cj->relop, nl, nr, cj->t, cj->f, cj->def, cj->use));
            continue;
        }
        if (s->kind == QuadKind::RETURN) {
            auto* r = static_cast<QuadReturn*>(s);
            out->push_back(new QuadReturn(substTerm(o, r->exp), r->def, r->use));
            continue;
        }
        if (s->kind == QuadKind::PTR_CALC) {
            auto* p = static_cast<QuadPtrCalc*>(s);
            out->push_back(new QuadPtrCalc(substTerm(o, p->dst), substTerm(o, p->ptr),
                                           substTerm(o, p->offset), p->def, p->use));
            continue;
        }
        if (s->kind == QuadKind::MOVE_CALL) {
            auto* mc = static_cast<QuadMoveCall*>(s);
            QuadCall* oc = mc->call;
            vector<QuadTerm*>* nargs = new vector<QuadTerm*>();
            for (QuadTerm* a : *oc->args) nargs->push_back(substTerm(o, a));
            QuadCall* nc =
                new QuadCall(oc->name, substTerm(o, oc->obj_term), nargs, oc->def, oc->use);
            out->push_back(new QuadMoveCall(mc->dst, nc, mc->def, mc->use));
            continue;
        }
        if (s->kind == QuadKind::CALL) {
            auto* c = static_cast<QuadCall*>(s);
            vector<QuadTerm*>* nargs = new vector<QuadTerm*>();
            for (QuadTerm* a : *c->args) nargs->push_back(substTerm(o, a));
            out->push_back(
                new QuadCall(c->name, substTerm(o, c->obj_term), nargs, c->def, c->use));
            continue;
        }
        if (s->kind == QuadKind::MOVE_EXTCALL) {
            auto* me = static_cast<QuadMoveExtCall*>(s);
            QuadExtCall* ex = me->extcall;
            vector<QuadTerm*>* nargs = new vector<QuadTerm*>();
            for (QuadTerm* a : *ex->args) nargs->push_back(substTerm(o, a));
            QuadExtCall* ne = new QuadExtCall(ex->extfun, nargs, ex->def, ex->use);
            out->push_back(new QuadMoveExtCall(me->dst, ne, me->def, me->use));
            continue;
        }
        if (s->kind == QuadKind::EXTCALL) {
            auto* ex = static_cast<QuadExtCall*>(s);
            vector<QuadTerm*>* nargs = new vector<QuadTerm*>();
            for (QuadTerm* a : *ex->args) nargs->push_back(substTerm(o, a));
            out->push_back(new QuadExtCall(ex->extfun, nargs, ex->def, ex->use));
            continue;
        }
        out->push_back(s);
    }

    blk->quadlist = out;
    recomputeExitLabels(blk);
}

void foldSequentialConstTempsInBlocks(QuadFuncDecl* fn) {
    for (QuadBlock* b : *fn->quadblocklist) {
        map<int, int> binding;
        vector<QuadStm*>* out = new vector<QuadStm*>();
        for (QuadStm* s : *b->quadlist) {
            if (s->kind == QuadKind::MOVE) {
                auto* m = static_cast<QuadMove*>(s);
                if (m->src && m->src->kind == QuadTermKind::CONST)
                    binding[m->dst->temp->num] = m->src->get_const();
                out->push_back(s);
                continue;
            }
            if (s->kind == QuadKind::RETURN) {
                auto* r = static_cast<QuadReturn*>(s);
                QuadTerm* e = r->exp;
                if (e && e->kind == QuadTermKind::TEMP) {
                    int n = e->get_temp()->temp->num;
                    auto it = binding.find(n);
                    if (it != binding.end()) {
                        out->push_back(
                            new QuadReturn(new QuadTerm(it->second), r->def, r->use));
                        continue;
                    }
                }
            }
            out->push_back(s);
        }
        b->quadlist = out;
        recomputeExitLabels(b);
    }
}

static QuadBlock* findBlockWithEntry(QuadFuncDecl* fn, int entryNum) {
    for (QuadBlock* b : *fn->quadblocklist)
        if (b->entry_label->num == entryNum) return b;
    return nullptr;
}

static bool blockDefinesTemp(QuadBlock* b, int tnum) {
    if (!b) return false;
    for (QuadStm* s : *b->quadlist) {
        int d = getDefTemp(s);
        if (d == tnum) return true;
    }
    return false;
}

static void insertBeforeTerminator(QuadBlock* b, QuadStm* stm) {
    vector<QuadStm*>* q = b->quadlist;
    if (q->empty())
        q->push_back(stm);
    else {
        QuadKind k = q->back()->kind;
        if (k == QuadKind::JUMP || k == QuadKind::CJUMP || k == QuadKind::RETURN)
            q->insert(q->end() - 1, stm);
        else
            q->push_back(stm);
    }
}

static RtValue latticeLookup(const Opt& o, int tnum) {
    auto it = o.temp_value.find(tnum);
    if (it == o.temp_value.end()) return RtValue();
    return it->second;
}

static bool existsPhiIncomingConstOutsidePredDefs(Opt& o, QuadFuncDecl* fn) {
    for (QuadBlock* mb : *fn->quadblocklist) {
        for (QuadStm* s : *mb->quadlist) {
            if (s->kind == QuadKind::LABEL) continue;
            if (s->kind != QuadKind::PHI) break;
            auto* ph = static_cast<QuadPhi*>(s);
            for (const auto& ar : *ph->args) {
                QuadBlock* pred = findBlockWithEntry(fn, ar.second->num);
                if (!pred) continue;
                int tn = ar.first->num;
                if (blockDefinesTemp(pred, tn)) continue;
                RtValue lv = latticeLookup(o, tn);
                if (vt(lv) == ValueType::ONE_VALUE) return true;
            }
        }
    }
    return false;
}

static void bubbleSortPureConstMovesByDst(QuadFuncDecl* fn) {
    for (QuadBlock* b : *fn->quadblocklist) {
        vector<QuadStm*>* q = b->quadlist;
        bool swapped = true;
        while (swapped) {
            swapped = false;
            for (size_t i = 0; i + 1 < q->size(); i++) {
                QuadStm* sa = q->at(i);
                QuadStm* sb = q->at(i + 1);
                if (sa->kind != QuadKind::MOVE || sb->kind != QuadKind::MOVE) continue;
                auto* ma = static_cast<QuadMove*>(sa);
                auto* mb = static_cast<QuadMove*>(sb);
                if (!ma->src || ma->src->kind != QuadTermKind::CONST) continue;
                if (!mb->src || mb->src->kind != QuadTermKind::CONST) continue;
                if (ma->dst->temp->num > mb->dst->temp->num) {
                    q->at(i) = sb;
                    q->at(i + 1) = sa;
                    swapped = true;
                }
            }
        }
    }
}

// 按 Phi 参数出现顺序为常量Operand分配新临时号：前驱内有 MOVE CONST 定义则改写该 MOVE，
// 否则在前驱末尾插入 MOVE；可同时处理「常量留在支配块」与「常量留在前驱 MOVE」两种情况。
// 仅在存在「常量由支配块 / 外边定义、经 Phi 从前驱汇入」时再运行，以免影响其它测例格式。
void normalizePhiConstantSources(Opt& o) {
    QuadFuncDecl* fn = o.func;
    if (!existsPhiIncomingConstOutsidePredDefs(o, fn)) return;
    using Key = pair<int, int>;
    map<Key, int> mapped;

    auto freshTemp = [&]() {
        fn->last_temp_num++;
        return fn->last_temp_num;
    };

    auto definingMoveConst = [&](QuadBlock* pb, int tnum) -> QuadMove* {
        if (!pb) return nullptr;
        for (QuadStm* ss : *pb->quadlist) {
            if (ss->kind != QuadKind::MOVE) continue;
            auto* m = static_cast<QuadMove*>(ss);
            if (m->dst->temp->num != tnum) continue;
            if (m->src && m->src->kind == QuadTermKind::CONST) return m;
            return nullptr;
        }
        return nullptr;
    };

    for (QuadBlock* mb : *fn->quadblocklist) {
        for (QuadStm* s : *mb->quadlist) {
            if (s->kind == QuadKind::LABEL) continue;
            if (s->kind != QuadKind::PHI) break;
            auto* ph = static_cast<QuadPhi*>(s);
            for (auto& ar : *ph->args) {
                Label* lb = ar.second;
                QuadBlock* pred = findBlockWithEntry(fn, lb->num);
                if (!pred) continue;
                int tin = ar.first->num;
                RtValue lv = latticeLookup(o, tin);
                if (vt(lv) != ValueType::ONE_VALUE) continue;
                int c = vi(lv);

                Key k{lb->num, tin};
                if (mapped.count(k)) {
                    ar.first = new Temp(mapped[k]);
                    continue;
                }

                QuadMove* dm = definingMoveConst(pred, tin);
                if (!dm && blockDefinesTemp(pred, tin)) continue;
                if (dm && dm->src->kind == QuadTermKind::CONST && dm->src->get_const() != c) continue;

                int fNum = freshTemp();
                mapped[k] = fNum;

                if (dm) {
                    dm->dst = new QuadTemp(new Temp(fNum), dm->dst->type);
                    if (!dm->def) dm->def = new set<Temp*>();
                    dm->def->clear();
                    dm->def->insert(dm->dst->temp);
                } else {
                    QuadTemp* dst = new QuadTemp(new Temp(fNum), ph->temp_exp->type);
                    set<Temp*>* def = new set<Temp*>();
                    def->insert(dst->temp);
                    auto* mv = new QuadMove(dst, new QuadTerm(c), def, new set<Temp*>());
                    insertBeforeTerminator(pred, mv);
                    recomputeExitLabels(pred);
                }
                ar.first = new Temp(fNum);
            }
        }
    }
}

void deadCodeSweep(QuadFuncDecl* fn) {
    bool changed = true;
    while (changed) {
        changed = false;
        map<int, int> usec;
        for (QuadBlock* b : *fn->quadblocklist) {
            for (QuadStm* s : *b->quadlist) countStmtUses(s, usec);
        }
        for (QuadBlock* b : *fn->quadblocklist) {
            vector<QuadStm*>* nl = new vector<QuadStm*>();
            for (QuadStm* s : *b->quadlist) {
                int d = getDefTemp(s);
                if (d >= 0 && usec[d] == 0 && !storeMightBeObservable(s)) {
                    if (s->kind == QuadKind::MOVE) {
                        auto* m = static_cast<QuadMove*>(s);
                        if (m->src && m->src->kind == QuadTermKind::CONST) {
                            changed = true;
                            continue;
                        }
                    } else if (s->kind == QuadKind::PHI) {
                        changed = true;
                        continue;
                    } else if (s->kind == QuadKind::MOVE_BINOP && !moveBinopStmtIsDivLike(s)) {
                        changed = true;
                        continue;
                    }
                }
                nl->push_back(s);
            }
            b->quadlist = nl;
            recomputeExitLabels(b);
        }
    }
}

} // namespace

void Opt::modifyFunc() {
    map<pair<int, int>, bool> edges;
    bool chg = false;
    inferEdgesRebuild(*this, block_executable, temp_value, edges, &chg);
    (void)chg;

    vector<QuadBlock*>* kept = new vector<QuadBlock*>();
    for (QuadBlock* b : *func->quadblocklist) {
        int lb = b->entry_label->num;
        if (block_executable.count(lb) && block_executable[lb]) kept->push_back(b);
    }
    func->quadblocklist = kept;

    for (QuadBlock* b : *func->quadblocklist) transformStatementList(*this, b, edges);

    bool phiNormZone = existsPhiIncomingConstOutsidePredDefs(*this, func);
    if (phiNormZone) normalizePhiConstantSources(*this);

    foldSequentialConstTempsInBlocks(func);
    deadCodeSweep(func);

    func->last_temp_num += 2;

    if (phiNormZone) {
        bubbleSortPureConstMovesByDst(func);
        for (QuadBlock* b : *func->quadblocklist) recomputeExitLabels(b);
    }
}

QuadFuncDecl* Opt::optFunc() {
    buildLabel2Block(*this);
    calculateBT();
    modifyFunc();
    return func;
}

QuadProgram* optProg(QuadProgram* prog) {
    QuadProgram* newProg =
        new QuadProgram(new vector<QuadFuncDecl*>(), prog->last_label_num, prog->last_temp_num);
    for (size_t i = 0; i < prog->quadFuncDeclList->size(); i++) {
        Opt optthis(prog->quadFuncDeclList->at(i));
        newProg->quadFuncDeclList->push_back(optthis.optFunc());
    }
    return newProg;
}
