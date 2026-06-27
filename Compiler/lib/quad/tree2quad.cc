#define DEBUG
#undef DEBUG

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "quad.hh"
#include "tree2quad.hh"
#include "treep.hh"

using namespace std;
using namespace tree;
using namespace quad;

namespace {

set<Temp *> *emptySet() { return new set<Temp *>(); }

set<Temp *> *defSet(Temp *t) {
    auto *s = emptySet();
    if (t) s->insert(t);
    return s;
}

void addTermUse(QuadTerm *term, set<Temp *> &use) {
    if (term == nullptr || term->kind != QuadTermKind::TEMP) return;
    use.insert(term->get_temp()->temp);
}

set<Temp *> *useSet(QuadTerm *a) {
    set<Temp *> u;
    addTermUse(a, u);
    return new set<Temp *>(u);
}

set<Temp *> *useSet(QuadTerm *a, QuadTerm *b) {
    set<Temp *> u;
    addTermUse(a, u);
    addTermUse(b, u);
    return new set<Temp *>(u);
}

set<Temp *> *useSet(const vector<QuadTerm *> *args, QuadTerm *obj) {
    set<Temp *> u;
    addTermUse(obj, u);
    if (args) {
        for (auto *arg : *args) addTermUse(arg, u);
    }
    return new set<Temp *>(u);
}

QuadType toQuadType(Type ty) { return ty == Type::PTR ? QuadType::PTR : QuadType::INT; }

bool isPtrType(Type ty) { return ty == Type::PTR; }

} // namespace

QuadProgram *tree2quad(Program *prog) {
#ifdef DEBUG
    cout << "in Tree2Quad::Converting IR to Quad" << endl;
#endif
    if (prog == nullptr || prog->funcdecllist == nullptr) {
        return nullptr;
    }

    Tree2Quad visitor;
    visitor.quadprog = nullptr;
    visitor.quadfuncdecllist = new vector<QuadFuncDecl *>();
    visitor.prog_last_label = 0;
    visitor.prog_last_temp = 0;
    prog->accept(visitor);

    if (visitor.quadfuncdecllist->empty()) {
        return nullptr;
    }
    return new QuadProgram(visitor.quadfuncdecllist, visitor.prog_last_label,
                           visitor.prog_last_temp);
}

void Tree2Quad::emit(QuadStm *stm) {
    if (visit_result == nullptr) visit_result = new vector<QuadStm *>();
    visit_result->push_back(stm);
}

QuadTemp *Tree2Quad::makeTemp(Temp *t, Type ty) {
    return new QuadTemp(temp_map->named_temp(t->num), toQuadType(ty));
}

QuadTerm *Tree2Quad::makeConst(int value) { return new QuadTerm(value); }

QuadTerm *Tree2Quad::makeName(const string &name) { return new QuadTerm(name); }

Label *Tree2Quad::labelOf(Label *label) {
    return temp_map->named_label(label->num);
}

QuadTerm *Tree2Quad::compileExp(Exp *exp) {
    output_term = nullptr;
    if (exp == nullptr) return nullptr;
    exp->accept(*this);
    return output_term;
}

vector<QuadTerm *> *Tree2Quad::compileArgs(vector<Exp *> *args) {
    auto *terms = new vector<QuadTerm *>();
    if (args == nullptr) return terms;
    for (Exp *arg : *args) {
        terms->push_back(compileExp(arg));
    }
    return terms;
}

void Tree2Quad::compileCallTo(Call *call, QuadTemp *dst) {
    QuadTerm *objTerm = compileExp(call->obj);
    vector<QuadTerm *> *args = compileArgs(call->args);
    auto *use = useSet(args, objTerm);
    auto *callStm = new QuadCall(call->id, objTerm, args, emptySet(), use);
    if (dst == nullptr) {
        emit(callStm);
    } else {
        auto *def = defSet(dst->temp);
        for (Temp *t : *use) def->erase(t);
        emit(new QuadMoveCall(dst, callStm, def, use));
    }
}

void Tree2Quad::compileExtCallTo(ExtCall *call, QuadTemp *dst) {
    vector<QuadTerm *> *args = compileArgs(call->args);
    auto *use = useSet(args, nullptr);
    auto *ext = new QuadExtCall(call->extfun, args, emptySet(), use);
    if (dst == nullptr) {
        emit(ext);
    } else {
        emit(new QuadMoveExtCall(dst, ext, defSet(dst->temp), use));
    }
}

QuadTerm *Tree2Quad::compileMemAddress(Exp *memExp) {
    if (auto *te = dynamic_cast<TempExp *>(memExp)) {
        return new QuadTerm(makeTemp(te->temp, te->type));
    }
    if (auto *bin = dynamic_cast<Binop *>(memExp)) {
        if (bin->op == "+") {
            QuadTerm *left = compileExp(bin->left);
            QuadTerm *right = compileExp(bin->right);
            Temp *dst = temp_map->newtemp();
            QuadTemp *dstTemp = makeTemp(dst, Type::PTR);
            emit(new QuadPtrCalc(new QuadTerm(dstTemp), left, right, defSet(dst),
                                 useSet(left, right)));
            return new QuadTerm(dstTemp);
        }
    }
    return compileExp(memExp);
}

void Tree2Quad::compileMoveToMem(Mem *dstMem, Exp *src) {
    QuadTerm *addr = compileMemAddress(dstMem->mem);
    QuadTerm *srcTerm = compileExp(src);
    emit(new QuadStore(srcTerm, addr, emptySet(), useSet(addr, srcTerm)));
}

void Tree2Quad::compileMoveToTemp(TempExp *dst, Exp *src) {
    QuadTemp *dstTemp = makeTemp(dst->temp, dst->type);

    if (auto *c = dynamic_cast<Const *>(src)) {
        emit(new QuadMove(dstTemp, makeConst(c->constVal), defSet(dst->temp), emptySet()));
        return;
    }
    if (auto *te = dynamic_cast<TempExp *>(src)) {
        QuadTerm *srcTerm = new QuadTerm(makeTemp(te->temp, te->type));
        emit(new QuadMove(dstTemp, srcTerm, defSet(dst->temp), useSet(srcTerm)));
        return;
    }
    if (auto *bin = dynamic_cast<Binop *>(src)) {
        QuadTerm *left = compileExp(bin->left);
        QuadTerm *right = compileExp(bin->right);
        emit(new QuadMoveBinop(dstTemp, left, bin->op, right, defSet(dst->temp),
                               useSet(left, right)));
        return;
    }
    if (auto *mem = dynamic_cast<Mem *>(src)) {
        QuadTerm *addr = compileMemAddress(mem->mem);
        emit(new QuadLoad(dstTemp, addr, defSet(dst->temp), useSet(addr)));
        return;
    }
    if (auto *call = dynamic_cast<Call *>(src)) {
        compileCallTo(call, dstTemp);
        return;
    }
    if (auto *ext = dynamic_cast<ExtCall *>(src)) {
        compileExtCallTo(ext, dstTemp);
        return;
    }
    if (auto *eseq = dynamic_cast<Eseq *>(src)) {
        if (eseq->stm) eseq->stm->accept(*this);
        compileMoveToTemp(dst, eseq->exp);
        return;
    }
    if (auto *name = dynamic_cast<Name *>(src)) {
        QuadTerm *srcTerm = nullptr;
        if (name->sname) srcTerm = makeName(name->sname->name);
        else if (name->name) srcTerm = makeName("L" + to_string(name->name->num));
        emit(new QuadMove(dstTemp, srcTerm, defSet(dst->temp), emptySet()));
        return;
    }

    compileExp(src);
    if (output_term) {
        emit(new QuadMove(dstTemp, output_term, defSet(dst->temp), useSet(output_term)));
    }
}

void Tree2Quad::visit(Program *prog) {
    for (FuncDecl *func : *prog->funcdecllist) {
        if (func == nullptr) continue;
        func->accept(*this);
        prog_last_label = max(prog_last_label, current_func_last_label);
        prog_last_temp = max(prog_last_temp, current_func_last_temp);
    }
}

void Tree2Quad::visit(FuncDecl *func) {
    temp_map = new Temp_map();
    temp_map->next_temp = func->last_temp_num + 2;
    temp_map->next_label = func->last_label_num + 1;
    for (int i = 0; i <= func->last_temp_num; ++i) temp_map->t_map[i] = true;
    for (int i = 0; i <= func->last_label_num; ++i) temp_map->l_map[i] = true;
    if (func->args) {
        for (Temp *arg : *func->args) temp_map->t_map[arg->num] = true;
    }

    visit_result = new vector<QuadStm *>();
    Label *entry = temp_map->named_label(func->last_label_num + 1);
    emit(new QuadLabel(entry, emptySet(), emptySet()));

    if (func->stm) func->stm->accept(*this);

    if (visit_result == nullptr || visit_result->empty() ||
        visit_result->back()->kind != QuadKind::RETURN) {
        emit(new QuadReturn(makeConst(0), emptySet(), emptySet()));
    }

    vector<Label *> *exitLabels = nullptr;
    vector<QuadBlock *> *blocks = new vector<QuadBlock *>();
    blocks->push_back(new QuadBlock(visit_result, entry, exitLabels));

    current_func_last_label = max(func->last_label_num + 1, temp_map->next_label - 1);
    if (temp_map->next_temp == func->last_temp_num + 2) {
        current_func_last_temp = func->last_temp_num + 1;
    } else {
        current_func_last_temp = temp_map->next_temp - 1;
    }

    quadfuncdecllist->push_back(new QuadFuncDecl(
        func->name, func->args, blocks, current_func_last_label, current_func_last_temp));
}

void Tree2Quad::visit(Seq *seq) {
    if (seq->sl == nullptr) return;
    for (Stm *stm : *seq->sl) {
        if (stm) stm->accept(*this);
    }
}

void Tree2Quad::visit(LabelStm *labelstm) {
    emit(new QuadLabel(labelOf(labelstm->label), emptySet(), emptySet()));
}

void Tree2Quad::visit(Jump *jump) {
    emit(new QuadJump(labelOf(jump->label), emptySet(), emptySet()));
}

void Tree2Quad::visit(Cjump *cjump) {
    QuadTerm *left = compileExp(cjump->left);
    QuadTerm *right = compileExp(cjump->right);
    emit(new QuadCJump(cjump->relop, left, right, labelOf(cjump->t), labelOf(cjump->f),
                       emptySet(), useSet(left, right)));
}

void Tree2Quad::visit(Move *move) {
    if (auto *dstMem = dynamic_cast<Mem *>(move->dst)) {
        compileMoveToMem(dstMem, move->src);
        return;
    }
    if (auto *dstTemp = dynamic_cast<TempExp *>(move->dst)) {
        compileMoveToTemp(dstTemp, move->src);
        return;
    }
}

void Tree2Quad::visit(Return *ret) {
    QuadTerm *val = compileExp(ret->exp);
    emit(new QuadReturn(val, emptySet(), useSet(val)));
}

void Tree2Quad::visit(ExpStm *exp) {
    if (auto *call = dynamic_cast<Call *>(exp->exp)) {
        compileCallTo(call, nullptr);
        return;
    }
    if (auto *ext = dynamic_cast<ExtCall *>(exp->exp)) {
        compileExtCallTo(ext, nullptr);
        return;
    }
    compileExp(exp->exp);
}

void Tree2Quad::visit(Binop *binop) {
    if (binop->op == "+" && (isPtrType(binop->left->type) || isPtrType(binop->right->type))) {
        QuadTerm *left = compileExp(binop->left);
        QuadTerm *right = compileExp(binop->right);
        Temp *dst = temp_map->newtemp();
        Type dstTy = isPtrType(binop->left->type) ? binop->left->type : binop->right->type;
        QuadTemp *dstTemp = makeTemp(dst, dstTy);
        emit(new QuadPtrCalc(new QuadTerm(dstTemp), left, right, defSet(dst),
                             useSet(left, right)));
        output_term = new QuadTerm(dstTemp);
        return;
    }

    QuadTerm *left = compileExp(binop->left);
    QuadTerm *right = compileExp(binop->right);
    Temp *dst = temp_map->newtemp();
    QuadTemp *dstTemp = makeTemp(dst, binop->type);
    emit(new QuadMoveBinop(dstTemp, left, binop->op, right, defSet(dst), useSet(left, right)));
    output_term = new QuadTerm(dstTemp);
}

void Tree2Quad::visit(Mem *mem) {
    QuadTerm *addr = compileMemAddress(mem->mem);
    Temp *dst = temp_map->newtemp();
    QuadTemp *dstTemp = makeTemp(dst, mem->type);
    emit(new QuadLoad(dstTemp, addr, defSet(dst), useSet(addr)));
    output_term = new QuadTerm(dstTemp);
}

void Tree2Quad::visit(TempExp *tempexp) {
    output_term = new QuadTerm(makeTemp(tempexp->temp, tempexp->type));
}

void Tree2Quad::visit(Eseq *eseq) {
    if (eseq->stm) eseq->stm->accept(*this);
    compileExp(eseq->exp);
}

void Tree2Quad::visit(Name *name) {
    if (name->sname) output_term = makeName(name->sname->name);
    else if (name->name) output_term = makeName("L" + to_string(name->name->num));
    else output_term = makeConst(0);
}

void Tree2Quad::visit(Const *constant) { output_term = makeConst(constant->constVal); }

void Tree2Quad::visit(Call *call) {
    Temp *dst = temp_map->newtemp();
    compileCallTo(call, makeTemp(dst, call->type));
    output_term = new QuadTerm(makeTemp(dst, call->type));
}

void Tree2Quad::visit(ExtCall *extcall) {
    Temp *dst = temp_map->newtemp();
    compileExtCallTo(extcall, makeTemp(dst, extcall->type));
    output_term = new QuadTerm(makeTemp(dst, extcall->type));
}
