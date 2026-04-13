#define DEBUG
#undef DEBUG

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "treep.hh"
#include "quad.hh"
#include "tree2quad.hh"

using namespace std;
using namespace tree;
using namespace quad;

/*
We use an instruction selection method (pattern matching) to convert the IR tree to Quad.
*/

namespace {

static map<int, Temp*> g_canonical_temps;

QuadType to_quad_type(Type t) {
    return (t == Type::PTR) ? QuadType::PTR : QuadType::INT;
}

Temp* canonical_temp(Temp* t) {
    if (t == nullptr) return nullptr;
    auto it = g_canonical_temps.find(t->num);
    if (it != g_canonical_temps.end()) return it->second;
    Temp* normalized = new Temp(t->num);
    g_canonical_temps[t->num] = normalized;
    return normalized;
}

set<Temp*>* new_temp_set() {
    return new set<Temp*>();
}

void add_term_use(set<Temp*>* uses, QuadTerm* term) {
    if (uses == nullptr || term == nullptr) return;
    if (term->kind != QuadTermKind::TEMP) return;
    QuadTemp* qt = term->get_temp();
    if (qt == nullptr || qt->temp == nullptr) return;
    uses->insert(canonical_temp(qt->temp));
}

void append_quads(vector<QuadStm*>* dst, vector<QuadStm*>* src) {
    if (dst == nullptr || src == nullptr) return;
    dst->insert(dst->end(), src->begin(), src->end());
}

QuadTerm* temp_term_from_temp_exp(TempExp* temp_exp) {
    return new QuadTerm(new QuadTemp(temp_exp->temp, to_quad_type(temp_exp->type)));
}

QuadCall* build_quad_call(Tree2Quad* ctx, Call* call, vector<QuadStm*>* out_quads) {
    call->obj->accept(*ctx);
    append_quads(out_quads, ctx->visit_result);
    QuadTerm* obj_term = ctx->output_term;

    vector<QuadTerm*>* arg_terms = new vector<QuadTerm*>();
    if (call->args != nullptr) {
        for (auto* arg : *call->args) {
            arg->accept(*ctx);
            append_quads(out_quads, ctx->visit_result);
            arg_terms->push_back(ctx->output_term);
        }
    }

    set<Temp*>* def = new_temp_set();
    set<Temp*>* use = new_temp_set();
    add_term_use(use, obj_term);
    for (auto* arg_term : *arg_terms) add_term_use(use, arg_term);
    return new QuadCall(call->id, obj_term, arg_terms, def, use);
}

QuadExtCall* build_quad_extcall(Tree2Quad* ctx, ExtCall* extcall, vector<QuadStm*>* out_quads) {
    vector<QuadTerm*>* arg_terms = new vector<QuadTerm*>();
    if (extcall->args != nullptr) {
        for (auto* arg : *extcall->args) {
            arg->accept(*ctx);
            append_quads(out_quads, ctx->visit_result);
            arg_terms->push_back(ctx->output_term);
        }
    }

    set<Temp*>* def = new_temp_set();
    set<Temp*>* use = new_temp_set();
    for (auto* arg_term : *arg_terms) add_term_use(use, arg_term);
    return new QuadExtCall(extcall->extfun, arg_terms, def, use);
}

}

QuadProgram* tree2quad(Program* prog) {
#ifdef DEBUG
    cout << "in Tree2Quad::Converting IR to Quad" << endl;
#endif
    if (prog == nullptr) {
        return nullptr;
    }

    g_canonical_temps.clear();
    Tree2Quad v;
    prog->accept(v);
    return v.quadprog;
}

void Tree2Quad::visit(tree::Program *prog) {
    vector<QuadFuncDecl*>* funcs = new vector<QuadFuncDecl*>();
    quadprog = new QuadProgram(funcs, 0, 0);
    if (prog == nullptr || prog->funcdecllist == nullptr) return;

    for (auto* func : *prog->funcdecllist) {
        if (func == nullptr) continue;
        func->accept(*this);
    }
}

void Tree2Quad::visit(tree::FuncDecl *func) {
    if (func == nullptr || quadprog == nullptr) return;

    temp_map = new Temp_map();
    temp_map->next_temp = func->last_temp_num + 1;
    temp_map->next_label = func->last_label_num + 1;

    vector<QuadStm*>* quads = new vector<QuadStm*>();
    Label* entry = temp_map->newlabel();
    quads->push_back(new QuadLabel(entry, new_temp_set(), new_temp_set()));

    if (func->stm != nullptr) {
        func->stm->accept(*this);
        append_quads(quads, visit_result);
    }

    vector<QuadBlock*>* blocks = new vector<QuadBlock*>();
    blocks->push_back(new QuadBlock(quads, entry, nullptr));

    QuadFuncDecl* qf = new QuadFuncDecl(
        func->name,
        func->args,
        blocks,
        temp_map->next_label - 1,
        temp_map->next_temp - 1
    );

    quadprog->quadFuncDeclList->push_back(qf);
    quadprog->last_label_num = max(quadprog->last_label_num, qf->last_label_num);
    quadprog->last_temp_num = max(quadprog->last_temp_num, qf->last_temp_num);
}

void Tree2Quad::visit(tree::Jump *jump) {
    visit_result = new vector<QuadStm*>();
    if (jump == nullptr) return;
    visit_result->push_back(new QuadJump(jump->label, new_temp_set(), new_temp_set()));
}

void Tree2Quad::visit(tree::Cjump *cjump) {
    vector<QuadStm*>* result = new vector<QuadStm*>();
    if (cjump == nullptr) return;

    cjump->left->accept(*this);
    append_quads(result, this->visit_result);
    QuadTerm* left_term = output_term;

    cjump->right->accept(*this);
    append_quads(result, this->visit_result);
    QuadTerm* right_term = output_term;

    set<Temp*>* def = new_temp_set();
    set<Temp*>* use = new_temp_set();
    add_term_use(use, left_term);
    add_term_use(use, right_term);
    result->push_back(new QuadCJump(cjump->relop, left_term, right_term, cjump->t, cjump->f, def, use));
    visit_result = result;
}

void Tree2Quad::visit(tree::Move *move) {
    vector<QuadStm*>* result = new vector<QuadStm*>();
    if (move == nullptr || move->dst == nullptr || move->src == nullptr) return;

    if (move->dst->getTreeKind() == Kind::TEMPEXP) {
        TempExp* dst_temp_exp = static_cast<TempExp*>(move->dst);
        QuadTemp* dst_temp = new QuadTemp(dst_temp_exp->temp, to_quad_type(dst_temp_exp->type));

        if (move->src->getTreeKind() == Kind::CALL) {
            vector<QuadStm*>* pre = new vector<QuadStm*>();
            QuadCall* call = build_quad_call(this, static_cast<Call*>(move->src), pre);
            append_quads(result, pre);
            set<Temp*>* def = new_temp_set();
            set<Temp*>* use = new_temp_set();
            def->insert(canonical_temp(dst_temp_exp->temp));
            for (auto* t : *call->use) use->insert(t);
            result->push_back(new QuadMoveCall(dst_temp, call, def, use));
            visit_result = result;
            return;
        }

        if (move->src->getTreeKind() == Kind::EXTCALL) {
            vector<QuadStm*>* pre = new vector<QuadStm*>();
            QuadExtCall* extcall = build_quad_extcall(this, static_cast<ExtCall*>(move->src), pre);
            append_quads(result, pre);
            set<Temp*>* def = new_temp_set();
            set<Temp*>* use = new_temp_set();
            def->insert(canonical_temp(dst_temp_exp->temp));
            for (auto* t : *extcall->use) use->insert(t);
            result->push_back(new QuadMoveExtCall(dst_temp, extcall, def, use));
            visit_result = result;
            return;
        }

        if (move->src->getTreeKind() == Kind::MEM) {
            Mem* src_mem = static_cast<Mem*>(move->src);
            src_mem->mem->accept(*this);
            append_quads(result, this->visit_result);
            QuadTerm* addr_term = output_term;

            set<Temp*>* def = new_temp_set();
            set<Temp*>* use = new_temp_set();
            def->insert(canonical_temp(dst_temp_exp->temp));
            add_term_use(use, addr_term);
            result->push_back(new QuadLoad(dst_temp, addr_term, def, use));
            visit_result = result;
            return;
        }

        if (move->src->getTreeKind() == Kind::BINOP) {
            Binop* src_binop = static_cast<Binop*>(move->src);
            src_binop->left->accept(*this);
            append_quads(result, this->visit_result);
            QuadTerm* left_term = output_term;

            src_binop->right->accept(*this);
            append_quads(result, this->visit_result);
            QuadTerm* right_term = output_term;

            set<Temp*>* def = new_temp_set();
            set<Temp*>* use = new_temp_set();
            def->insert(canonical_temp(dst_temp_exp->temp));
            add_term_use(use, left_term);
            add_term_use(use, right_term);

            if (src_binop->op == "+" && src_binop->type == Type::PTR) {
                result->push_back(new QuadPtrCalc(new QuadTerm(dst_temp), left_term, right_term, def, use));
            } else {
                result->push_back(new QuadMoveBinop(dst_temp, left_term, src_binop->op, right_term, def, use));
            }
            visit_result = result;
            return;
        }

        move->src->accept(*this);
        append_quads(result, this->visit_result);
        QuadTerm* src_term = output_term;

        set<Temp*>* def = new_temp_set();
        set<Temp*>* use = new_temp_set();
        def->insert(canonical_temp(dst_temp_exp->temp));
        add_term_use(use, src_term);
        result->push_back(new QuadMove(dst_temp, src_term, def, use));
        visit_result = result;
        return;
    }

    if (move->dst->getTreeKind() == Kind::MEM) {
        Mem* dst_mem = static_cast<Mem*>(move->dst);
        dst_mem->mem->accept(*this);
        append_quads(result, this->visit_result);
        QuadTerm* dst_addr = output_term;

        QuadTerm* src_term = nullptr;
        if (move->src->getTreeKind() == Kind::CALL) {
            vector<QuadStm*>* pre = new vector<QuadStm*>();
            QuadCall* call = build_quad_call(this, static_cast<Call*>(move->src), pre);
            append_quads(result, pre);

            Temp* tmp = temp_map->newtemp();
            QuadType qt = to_quad_type(move->src->type);
            QuadTemp* dst_tmp = new QuadTemp(tmp, qt);
            set<Temp*>* call_def = new_temp_set();
            set<Temp*>* call_use = new_temp_set();
            call_def->insert(canonical_temp(tmp));
            for (auto* t : *call->use) call_use->insert(t);
            result->push_back(new QuadMoveCall(dst_tmp, call, call_def, call_use));
            src_term = new QuadTerm(dst_tmp);
        } else if (move->src->getTreeKind() == Kind::EXTCALL) {
            vector<QuadStm*>* pre = new vector<QuadStm*>();
            QuadExtCall* extcall = build_quad_extcall(this, static_cast<ExtCall*>(move->src), pre);
            append_quads(result, pre);

            Temp* tmp = temp_map->newtemp();
            QuadType qt = to_quad_type(move->src->type);
            QuadTemp* dst_tmp = new QuadTemp(tmp, qt);
            set<Temp*>* call_def = new_temp_set();
            set<Temp*>* call_use = new_temp_set();
            call_def->insert(canonical_temp(tmp));
            for (auto* t : *extcall->use) call_use->insert(t);
            result->push_back(new QuadMoveExtCall(dst_tmp, extcall, call_def, call_use));
            src_term = new QuadTerm(dst_tmp);
        } else {
            move->src->accept(*this);
            append_quads(result, this->visit_result);
            src_term = output_term;
        }

        set<Temp*>* def = new_temp_set();
        set<Temp*>* use = new_temp_set();
        add_term_use(use, dst_addr);
        add_term_use(use, src_term);
        result->push_back(new QuadStore(src_term, dst_addr, def, use));
        visit_result = result;
    }
}

void Tree2Quad::visit(tree::Seq *seq) {
    vector<QuadStm*>* result = new vector<QuadStm*>();
    if (seq == nullptr || seq->sl == nullptr) return;
    for (auto* stm : *seq->sl) {
        if (stm == nullptr) continue;
        stm->accept(*this);
        append_quads(result, this->visit_result);
    }
    visit_result = result;
}

void Tree2Quad::visit(tree::LabelStm *labelstm) {
    visit_result = new vector<QuadStm*>();
    if (labelstm == nullptr) return;
    visit_result->push_back(new QuadLabel(labelstm->label, new_temp_set(), new_temp_set()));
}

void Tree2Quad::visit(tree::Return *ret) {
    vector<QuadStm*>* result = new vector<QuadStm*>();
    if (ret == nullptr || ret->exp == nullptr) return;
    ret->exp->accept(*this);
    append_quads(result, this->visit_result);
    QuadTerm* value = output_term;
    set<Temp*>* def = new_temp_set();
    set<Temp*>* use = new_temp_set();
    add_term_use(use, value);
    result->push_back(new QuadReturn(value, def, use));
    visit_result = result;
}

void Tree2Quad::visit(tree::ExpStm *exp) {
    vector<QuadStm*>* result = new vector<QuadStm*>();
    if (exp == nullptr || exp->exp == nullptr) return;

    if (exp->exp->getTreeKind() == Kind::CALL) {
        vector<QuadStm*>* pre = new vector<QuadStm*>();
        QuadCall* call = build_quad_call(this, static_cast<Call*>(exp->exp), pre);
        append_quads(result, pre);
        result->push_back(call);
        visit_result = result;
        return;
    }

    if (exp->exp->getTreeKind() == Kind::EXTCALL) {
        vector<QuadStm*>* pre = new vector<QuadStm*>();
        QuadExtCall* extcall = build_quad_extcall(this, static_cast<ExtCall*>(exp->exp), pre);
        append_quads(result, pre);
        result->push_back(extcall);
        visit_result = result;
        return;
    }

    exp->exp->accept(*this);
    append_quads(result, this->visit_result);
    visit_result = result;
}

void Tree2Quad::visit(tree::Binop *binop) {
    vector<QuadStm*>* result = new vector<QuadStm*>();
    if (binop == nullptr) return;

    binop->left->accept(*this);
    append_quads(result, this->visit_result);
    QuadTerm* left_term = output_term;

    binop->right->accept(*this);
    append_quads(result, this->visit_result);
    QuadTerm* right_term = output_term;

    Temp* t = temp_map->newtemp();
    QuadType qt = to_quad_type(binop->type);
    QuadTemp* dst_temp = new QuadTemp(t, qt);
    QuadTerm* dst_term = new QuadTerm(dst_temp);

    set<Temp*>* def = new_temp_set();
    set<Temp*>* use = new_temp_set();
    def->insert(canonical_temp(t));
    add_term_use(use, left_term);
    add_term_use(use, right_term);

    if (binop->op == "+" && binop->type == Type::PTR) {
        result->push_back(new QuadPtrCalc(dst_term, left_term, right_term, def, use));
    } else {
        result->push_back(new QuadMoveBinop(dst_temp, left_term, binop->op, right_term, def, use));
    }
    visit_result = result;
    output_term = dst_term;
}

void Tree2Quad::visit(tree::Mem *mem) {
    vector<QuadStm*>* result = new vector<QuadStm*>();
    if (mem == nullptr || mem->mem == nullptr) return;

    mem->mem->accept(*this);
    append_quads(result, this->visit_result);
    QuadTerm* addr_term = output_term;

    Temp* t = temp_map->newtemp();
    QuadTemp* dst = new QuadTemp(t, to_quad_type(mem->type));
    QuadTerm* dst_term = new QuadTerm(dst);

    set<Temp*>* def = new_temp_set();
    set<Temp*>* use = new_temp_set();
    def->insert(canonical_temp(t));
    add_term_use(use, addr_term);
    result->push_back(new QuadLoad(dst, addr_term, def, use));
    visit_result = result;
    output_term = dst_term;
}

void Tree2Quad::visit(tree::TempExp *tempexp) {
    visit_result = new vector<QuadStm*>();
    if (tempexp == nullptr) {
        output_term = nullptr;
        return;
    }
    output_term = temp_term_from_temp_exp(tempexp);
}

void Tree2Quad::visit(tree::Eseq *eseq) {
    vector<QuadStm*>* result = new vector<QuadStm*>();
    if (eseq == nullptr) return;

    if (eseq->stm != nullptr) {
        eseq->stm->accept(*this);
        append_quads(result, this->visit_result);
    }
    if (eseq->exp != nullptr) {
        eseq->exp->accept(*this);
        append_quads(result, this->visit_result);
        output_term = this->output_term;
    } else {
        output_term = nullptr;
    }
    visit_result = result;
}

void Tree2Quad::visit(tree::Name *name) {
    visit_result = new vector<QuadStm*>();
    if (name == nullptr) {
        output_term = nullptr;
        return;
    }
    if (name->sname != nullptr) {
        output_term = new QuadTerm(name->sname->name);
        return;
    }
    if (name->name != nullptr) {
        output_term = new QuadTerm(to_string(name->name->num));
        return;
    }
    output_term = nullptr;
}

void Tree2Quad::visit(tree::Const *const_exp) {
    visit_result = new vector<QuadStm*>();
    if (const_exp == nullptr) {
        output_term = nullptr;
        return;
    }
    output_term = new QuadTerm(const_exp->constVal);
}

void Tree2Quad::visit(tree::Call *call) {
    visit_result = new vector<QuadStm*>();
    if (call == nullptr) {
        output_term = nullptr;
        return;
    }

    QuadCall* qcall = build_quad_call(this, call, visit_result);
    Temp* t = temp_map->newtemp();
    QuadTemp* dst = new QuadTemp(t, to_quad_type(call->type));
    set<Temp*>* def = new_temp_set();
    set<Temp*>* use = new_temp_set();
    def->insert(canonical_temp(t));
    for (auto* u : *qcall->use) use->insert(u);
    visit_result->push_back(new QuadMoveCall(dst, qcall, def, use));
    output_term = new QuadTerm(dst);
}

void Tree2Quad::visit(tree::ExtCall *extcall) {
    visit_result = new vector<QuadStm*>();
    if (extcall == nullptr) {
        output_term = nullptr;
        return;
    }

    QuadExtCall* qext = build_quad_extcall(this, extcall, visit_result);
    Temp* t = temp_map->newtemp();
    QuadTemp* dst = new QuadTemp(t, to_quad_type(extcall->type));
    set<Temp*>* def = new_temp_set();
    set<Temp*>* use = new_temp_set();
    def->insert(canonical_temp(t));
    for (auto* u : *qext->use) use->insert(u);
    visit_result->push_back(new QuadMoveExtCall(dst, qext, def, use));
    output_term = new QuadTerm(dst);
}
