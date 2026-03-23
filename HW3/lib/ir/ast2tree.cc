#define DEBUG
// #undef DEBUG

#include <iostream>  // IWYU pragma: keep
#include <string>
#include <map>        // IWYU pragma: keep
#include <vector>
#include <algorithm>  // IWYU pragma: keep

#include "config.hh"    // IWYU pragma: keep
#include "ASTheader.hh"  // IWYU pragma: keep
#include "FDMJAST.hh"    // IWYU pragma: keep
#include "treep.hh"      // IWYU pragma: keep
#include "temp.hh"       // IWYU pragma: keep
#include "ast2tree.hh"   // IWYU pragma: keep

using namespace std;

static inline tree::Type fdmjTypeToTreeType(fdmj::TypeKind tk) {
    switch (tk) {
        case fdmj::TypeKind::INT: return tree::Type::INT;
        default: return tree::Type::PTR;
    }
}

Class_table* generate_class_table(AST_Semant_Map* semant_map) {
    if (semant_map == nullptr || semant_map->getNameMaps() == nullptr) {
        return new Class_table();
    }
    Name_Maps* nm = semant_map->getNameMaps();
    auto* ct = new Class_table();

    // HW3 assumes no classes in Program (only main), but NameMaps may still contain __$main__.
    int var_off = 0;
    for (const auto& c : *nm->get_class_list()) {
        set<string>* vars = nm->get_class_var_list(c);
        for (const auto& v : *vars) {
            ct->var_pos_map[c + "^" + v] = var_off++;
        }
        delete vars;

        // method_pos_map is only indexed by method name in this simplified Class_table.
        set<string>* methods = nm->get_method_list(c);
        for (const auto& m : *methods) {
            if (ct->method_pos_map.find(m) == ct->method_pos_map.end()) {
                ct->method_pos_map[m] = static_cast<int>(ct->method_pos_map.size());
            }
        }
        delete methods;
    }
    return ct;
}

Method_var_table* generate_method_var_table(string class_name, string method_name, Name_Maps* nm, Temp_map* tm) {
    auto* mvt = new Method_var_table();
    if (nm == nullptr || tm == nullptr) return mvt;

    // In HW3, only integer variables exist; nevertheless we keep the type map.
    // Temp numbering must be deterministic to match expected IR outputs.
    set<string>* locals = nm->get_method_var_list(class_name, method_name);
    for (const auto& v : *locals) {
        (*mvt->var_temp_map)[v] = tm->newtemp();
        (*mvt->var_type_map)[v] = tree::Type::INT;
    }
    delete locals;

    // Allocate remaining formals (if any). If a formal name conflicts with a local,
    // local temp should override formal temp; we skip allocating such formals.
    vector<Formal*>* formals = nm->get_method_formal_list(class_name, method_name);
    for (auto* f : *formals) {
        if (f == nullptr || f->id == nullptr) continue;
        const string fname = f->id->id;
        if (mvt->var_temp_map->find(fname) != mvt->var_temp_map->end()) {
            continue; // local overrides formal
        }
        tree::Type t = fdmjTypeToTreeType(f->type->typeKind);
        (*mvt->var_temp_map)[fname] = tm->newtemp();
        (*mvt->var_type_map)[fname] = t;
    }
    delete formals;

    return mvt;
}

tree::Program* ast2tree(fdmj::Program* prog, AST_Semant_Map* semant_map) {
    ASTToTreeVisitor v;
    v.semant_map = semant_map;
    v.class_table = generate_class_table(semant_map);
    prog->accept(v);

    auto* t = v.getTree();
    return static_cast<tree::Program*>(t);
}

void ASTToTreeVisitor::visit(fdmj::Program* node) {
    if (node == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    if (node->main == nullptr) {
        cerr << "Error: Program has no MainMethod" << endl;
        visit_tree_result = nullptr;
        return;
    }
    node->main->accept(*this);
    // MainMethod sets visit_tree_result to the final IR Program.
}

void ASTToTreeVisitor::visit(fdmj::MainMethod* node) {
    current_class = "__$main__";
    current_method = "main";
    if (semant_map != nullptr && semant_map->getNameMaps() != nullptr) {
        Name_Maps* nm = semant_map->getNameMaps();
        if (!nm->is_class(current_class)) {
            // fallback: pick any existing class
            auto* classes = nm->get_class_list();
            if (classes != nullptr && !classes->empty()) {
                current_class = *classes->begin();
            }
            delete classes;
        }
        if (!nm->is_method(current_class, current_method)) {
            set<string>* methods = nm->get_method_list(current_class);
            if (methods != nullptr && !methods->empty()) {
                current_method = *methods->begin();
            }
            delete methods;
        }
    }

    method_temp_map = new Temp_map();
    method_var_table = generate_method_var_table(current_class, current_method,
                                                 semant_map ? semant_map->getNameMaps() : nullptr,
                                                 method_temp_map);

    // Translate statements.
    vector<tree::Stm*>* stm_ir_list = new vector<tree::Stm*>();
    if (node->sl != nullptr) {
        for (auto* s : *node->sl) {
            if (s == nullptr) continue;
            s->accept(*this);
            auto* stm = static_cast<tree::Stm*>(visit_tree_result);
            if (stm != nullptr) stm_ir_list->push_back(stm);
        }
    }
    tree::Seq* body_seq = new tree::Seq(stm_ir_list);

    const string ir_func_name = current_class + "^" + current_method;

    // last_temp/last_label are the last allocated numbers (tm counters - 1).
    int last_temp = method_temp_map->next_temp - 1;
    int last_label = method_temp_map->next_label - 1;

    tree::FuncDecl* fd = new tree::FuncDecl(ir_func_name, nullptr, body_seq, tree::Type::INT, last_temp, last_label);
    vector<tree::FuncDecl*>* fdl = new vector<tree::FuncDecl*>();
    fdl->push_back(fd);
    visit_tree_result = new tree::Program(fdl);

    // Avoid deleting labels referenced by IR nodes (visitor destructor currently deletes continue/break labels).
    continue_label = nullptr;
    break_label = nullptr;
}

void ASTToTreeVisitor::visit(fdmj::ClassDecl* /*node*/) { visit_tree_result = nullptr; }
void ASTToTreeVisitor::visit(fdmj::Type* /*node*/) { visit_tree_result = nullptr; }
void ASTToTreeVisitor::visit(fdmj::VarDecl* /*node*/) { visit_tree_result = nullptr; }
void ASTToTreeVisitor::visit(fdmj::MethodDecl* /*node*/) { visit_tree_result = nullptr; }
void ASTToTreeVisitor::visit(fdmj::Formal* /*node*/) { visit_tree_result = nullptr; }
void ASTToTreeVisitor::visit(fdmj::Nested* node) {
    if (node == nullptr || node->sl == nullptr || node->sl->size() == 0) {
        visit_tree_result = nullptr;
        return;
    }
    vector<tree::Stm*>* sl_ir = new vector<tree::Stm*>();
    for (auto* s : *node->sl) {
        if (s == nullptr) continue;
        s->accept(*this);
        auto* stm = static_cast<tree::Stm*>(visit_tree_result);
        if (stm != nullptr) sl_ir->push_back(stm);
    }
    visit_tree_result = new tree::Seq(sl_ir);
}

void ASTToTreeVisitor::visit(fdmj::If* node) { 
    if (!node || !node->exp || !node->stm1) {
        visit_tree_result = nullptr;
        return;
    }

    // 1️⃣ 先翻译 condition
    node->exp->accept(*this);
    Tr_cx* cond_cx = visit_exp_result->unCx(method_temp_map);

    // 2️⃣ 先翻译 then / else（关键！！）
    node->stm1->accept(*this);
    tree::Stm* then_stm = static_cast<tree::Stm*>(visit_tree_result);

    tree::Stm* else_stm = nullptr;
    if (node->stm2) {
        node->stm2->accept(*this);
        else_stm = static_cast<tree::Stm*>(visit_tree_result);
    }

    // 3️⃣ 最后才分配 label（核心！！）
    Label* then_label = method_temp_map->newlabel();
    Label* else_label = method_temp_map->newlabel();
    Label* join_label = method_temp_map->newlabel();

    cond_cx->true_list->patch(then_label);
    cond_cx->false_list->patch(else_label);

    // 4️⃣ 拼 IR
    vector<tree::Stm*>* sl = new vector<tree::Stm*>();

    sl->push_back(cond_cx->stm);
    sl->push_back(new tree::LabelStm(then_label));
    if (then_stm) sl->push_back(then_stm);

    if (node->stm2) {
        sl->push_back(new tree::Jump(join_label));
        sl->push_back(new tree::LabelStm(else_label));
        if (else_stm) sl->push_back(else_stm);
        sl->push_back(new tree::LabelStm(join_label));
    } else {
        sl->push_back(new tree::LabelStm(else_label));
    }

    visit_tree_result = new tree::Seq(sl);
}

void ASTToTreeVisitor::visit(fdmj::While* node) {
    if (!node || !node->exp) {
        visit_tree_result = nullptr;
        return;
    }

    Label* old_continue = continue_label;
    Label* old_break = break_label;

    // ✅ 先翻译 cond（关键！！）
    node->exp->accept(*this);
    Tr_cx* cond_cx = visit_exp_result->unCx(method_temp_map);

    // ✅ 再申请 label
    Label* start_label = method_temp_map->newlabel();
    Label* body_label  = method_temp_map->newlabel();
    Label* exit_label  = method_temp_map->newlabel();

    continue_label = start_label;
    break_label = exit_label;

    cond_cx->true_list->patch(body_label);
    cond_cx->false_list->patch(exit_label);

    // body
    tree::Stm* body_stm = nullptr;
    if (node->stm) {
        node->stm->accept(*this);
        body_stm = static_cast<tree::Stm*>(visit_tree_result);
    }

    vector<tree::Stm*>* sl = new vector<tree::Stm*>();

    sl->push_back(new tree::LabelStm(start_label));
    sl->push_back(cond_cx->stm);
    sl->push_back(new tree::LabelStm(body_label));

    if (body_stm) sl->push_back(body_stm);

    sl->push_back(new tree::Jump(start_label));
    sl->push_back(new tree::LabelStm(exit_label));

    visit_tree_result = new tree::Seq(sl);

    continue_label = old_continue;
    break_label = old_break;
}

void ASTToTreeVisitor::visit(fdmj::Assign* node) {
    if (node == nullptr || node->left == nullptr || node->exp == nullptr) {
        visit_tree_result = nullptr;
        return;
    }

    node->left->accept(*this);
    Tr_Exp* left_tr = visit_exp_result;
    if (left_tr == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    Tr_ex* left_ex = left_tr->unEx(method_temp_map);
    tree::Exp* dst = left_ex->exp;

    node->exp->accept(*this);
    Tr_Exp* rhs_tr = visit_exp_result;
    if (rhs_tr == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    Tr_ex* rhs_ex = rhs_tr->unEx(method_temp_map);
    tree::Exp* src = rhs_ex->exp;

    visit_tree_result = new tree::Move(dst, src);
}

void ASTToTreeVisitor::visit(fdmj::Continue* node) {
    if (node == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    if (continue_label == nullptr) {
        cerr << "Error: continue used outside a loop" << endl;
        visit_tree_result = nullptr;
        return;
    }
    visit_tree_result = new tree::Jump(continue_label);
}

void ASTToTreeVisitor::visit(fdmj::Break* node) {
    if (node == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    if (break_label == nullptr) {
        cerr << "Error: break used outside a loop" << endl;
        visit_tree_result = nullptr;
        return;
    }
    visit_tree_result = new tree::Jump(break_label);
}

void ASTToTreeVisitor::visit(fdmj::Return* node) {
    if (node == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    if (node->exp == nullptr) {
        visit_tree_result = new tree::Return(new tree::Const(0));
        return;
    }
    node->exp->accept(*this);
    Tr_Exp* ret_tr = visit_exp_result;
    Tr_ex* ret_ex = ret_tr->unEx(method_temp_map);
    visit_tree_result = new tree::Return(ret_ex->exp);
}

void ASTToTreeVisitor::visit(fdmj::PutInt* node) {
    if (node == nullptr || node->exp == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    node->exp->accept(*this);
    Tr_Exp* e_tr = visit_exp_result;
    tree::Exp* arg_exp = e_tr->unEx(method_temp_map)->exp;

    vector<tree::Exp*>* args = new vector<tree::Exp*>();
    args->push_back(arg_exp);
    tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, "putint", args);
    visit_tree_result = new tree::ExpStm(call);
}

void ASTToTreeVisitor::visit(fdmj::PutCh* node) {
    if (node == nullptr || node->exp == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    node->exp->accept(*this);
    Tr_Exp* e_tr = visit_exp_result;
    tree::Exp* arg_exp = e_tr->unEx(method_temp_map)->exp;

    vector<tree::Exp*>* args = new vector<tree::Exp*>();
    args->push_back(arg_exp);
    tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, "putch", args);
    visit_tree_result = new tree::ExpStm(call);
}

void ASTToTreeVisitor::visit(fdmj::PutArray* /*node*/) {
    cerr << "Error: PutArray not supported in HW3" << endl;
    visit_tree_result = nullptr;
}

void ASTToTreeVisitor::visit(fdmj::Starttime* /*node*/) {
    vector<tree::Exp*>* args = nullptr;
    tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, "starttime", args);
    visit_tree_result = new tree::ExpStm(call);
}

void ASTToTreeVisitor::visit(fdmj::Stoptime* /*node*/) {
    vector<tree::Exp*>* args = nullptr;
    tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, "stoptime", args);
    visit_tree_result = new tree::ExpStm(call);
}

// Statements with generic call nodes (HW3 mainly uses PutInt/PutCh/GetInt special nodes).
void ASTToTreeVisitor::visit(fdmj::CallStm* node) {
    if (node == nullptr || node->name == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    if (node->obj != nullptr) {
        cerr << "Error: CallStm on object not supported in HW3" << endl;
        visit_tree_result = nullptr;
        return;
    }

    string extfun = node->name->id;
    vector<tree::Exp*>* args = nullptr;
    if (node->par != nullptr) {
        args = new vector<tree::Exp*>();
        for (auto* p : *node->par) {
            if (p == nullptr) continue;
            p->accept(*this);
            args->push_back(visit_exp_result->unEx(method_temp_map)->exp);
        }
    }
    tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, extfun, args);
    visit_tree_result = new tree::ExpStm(call);
}

void ASTToTreeVisitor::visit(fdmj::BinaryOp* node) {
    if (node == nullptr || node->op == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    const string op = node->op->op;

    if (op == "+" || op == "-" || op == "*" || op == "/") {
        node->left->accept(*this);
        Tr_Exp* l = visit_exp_result;
        node->right->accept(*this);
        Tr_Exp* r = visit_exp_result;
        tree::Exp* le = l->unEx(method_temp_map)->exp;
        tree::Exp* re = r->unEx(method_temp_map)->exp;
        visit_exp_result = new Tr_ex(new tree::Binop(tree::Type::INT, op, le, re));
        return;
    }

    // Short-circuit boolean operators.
    if (op == "&&" || op == "||") {
        node->left->accept(*this);
        Tr_Exp* l = visit_exp_result;
        node->right->accept(*this);
        Tr_Exp* r = visit_exp_result;

        Tr_cx* l_cx = l->unCx(method_temp_map);
        Tr_cx* r_cx = r->unCx(method_temp_map);

        if (op == "&&") {
            Label* mid = method_temp_map->newlabel();
            // If left is true, jump to mid to evaluate right; if left is false, fall through to false_list.
            l_cx->true_list->patch(mid);

            Patch_list* false_list = new Patch_list();
            false_list->add(l_cx->false_list);
            false_list->add(r_cx->false_list);

            vector<tree::Stm*>* sl = new vector<tree::Stm*>();
            sl->push_back(l_cx->stm);
            sl->push_back(new tree::LabelStm(mid));
            sl->push_back(r_cx->stm);

            visit_exp_result = new Tr_cx(r_cx->true_list, false_list, new tree::Seq(sl));
        } else {
            Label* mid = method_temp_map->newlabel();
            // If left is false, jump to mid to evaluate right; if left is true, it should go to true_list.
            l_cx->false_list->patch(mid);

            Patch_list* true_list = new Patch_list();
            true_list->add(l_cx->true_list);
            true_list->add(r_cx->true_list);

            vector<tree::Stm*>* sl = new vector<tree::Stm*>();
            sl->push_back(l_cx->stm);
            sl->push_back(new tree::LabelStm(mid));
            sl->push_back(r_cx->stm);

            visit_exp_result = new Tr_cx(true_list, r_cx->false_list, new tree::Seq(sl));
        }
        return;
    }

    // Relational operators.
    if (op == ">" || op == "<" || op == "==" || op == "!=" || op == ">=" || op == "<=") {
        node->left->accept(*this);
        Tr_Exp* l = visit_exp_result;
        node->right->accept(*this);
        Tr_Exp* r = visit_exp_result;

        tree::Exp* le = l->unEx(method_temp_map)->exp;
        tree::Exp* re = r->unEx(method_temp_map)->exp;

        Patch_list* t = new Patch_list();
        Patch_list* f = new Patch_list();
        Label* tl = method_temp_map->newlabel();
        Label* fl = method_temp_map->newlabel();
        t->add_patch(tl);
        f->add_patch(fl);
        tree::Cjump* cj = new tree::Cjump(op, le, re, tl, fl);
        visit_exp_result = new Tr_cx(t, f, cj);
        return;
    }

    cerr << "Error: Unsupported BinaryOp operator: " << op << endl;
    visit_exp_result = nullptr;
}

void ASTToTreeVisitor::visit(fdmj::UnaryOp* node) {
    if (node == nullptr || node->op == nullptr || node->exp == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    const string op = node->op->op;
    node->exp->accept(*this);
    Tr_Exp* inner = visit_exp_result;

    if (op == "-") {
        tree::Exp* e = inner->unEx(method_temp_map)->exp;
        visit_exp_result = new Tr_ex(new tree::Binop(tree::Type::INT, "-", new tree::Const(0), e));
        return;
    }

    if (op == "!") {
        // Not required by your request, but implemented for completeness:
        // !e is translated as a boolean negation of (e != 0).
        Tr_cx* cx = inner->unCx(method_temp_map);
        Patch_list* tmp = cx->true_list;
        cx->true_list = cx->false_list;
        cx->false_list = tmp;
        visit_exp_result = cx;
        return;
    }

    cerr << "Error: Unsupported UnaryOp operator: " << op << endl;
    visit_exp_result = nullptr;
}

void ASTToTreeVisitor::visit(fdmj::IdExp* node) {
    if (node == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    if (method_var_table == nullptr) {
        cerr << "Error: IdExp visited without method_var_table" << endl;
        visit_exp_result = nullptr;
        return;
    }

    const string name = node->id;
    tree::Temp* t = method_var_table->get_var_temp(name);
    if (t == nullptr) {
        cerr << "Error: Unknown identifier in IdExp: " << name << endl;
        // Fail fast instead of generating invalid IR.
        exit(1);
    }
    visit_exp_result = new Tr_ex(new tree::TempExp(tree::Type::INT, t));
}

void ASTToTreeVisitor::visit(fdmj::IntExp* node) {
    if (node == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    visit_exp_result = new Tr_ex(new tree::Const(node->val));
}

void ASTToTreeVisitor::visit(fdmj::OpExp* /*node*/) {
    // OpExp is always handled by BinaryOp/UnaryOp directly.
    visit_exp_result = nullptr;
}

void ASTToTreeVisitor::visit(fdmj::ArrayExp* /*node*/) { cerr << "Error: ArrayExp not supported in HW3" << endl; visit_exp_result = nullptr; }
void ASTToTreeVisitor::visit(fdmj::ClassVar* /*node*/) { cerr << "Error: ClassVar not supported in HW3" << endl; visit_exp_result = nullptr; }
void ASTToTreeVisitor::visit(fdmj::This* /*node*/) { cerr << "Error: This not supported in HW3" << endl; visit_exp_result = nullptr; }
void ASTToTreeVisitor::visit(fdmj::Length* /*node*/) { cerr << "Error: Length not supported in HW3" << endl; visit_exp_result = nullptr; }
void ASTToTreeVisitor::visit(fdmj::NewArray* /*node*/) { cerr << "Error: NewArray not supported in HW3" << endl; visit_exp_result = nullptr; }
void ASTToTreeVisitor::visit(fdmj::NewObject* /*node*/) { cerr << "Error: NewObject not supported in HW3" << endl; visit_exp_result = nullptr; }

void ASTToTreeVisitor::visit(fdmj::CallExp* node) {
    if (node == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    if (node->name == nullptr) {
        cerr << "Error: CallExp missing method name" << endl;
        visit_exp_result = nullptr;
        return;
    }
    if (node->obj != nullptr) {
        cerr << "Error: CallExp on object not supported in HW3" << endl;
        visit_exp_result = nullptr;
        return;
    }

    string extfun = node->name->id;
    vector<tree::Exp*>* args = nullptr;
    if (node->par != nullptr) {
        args = new vector<tree::Exp*>();
        for (auto* p : *node->par) {
            if (p == nullptr) continue;
            p->accept(*this);
            args->push_back(visit_exp_result->unEx(method_temp_map)->exp);
        }
    }
    tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, extfun, args);
    visit_exp_result = new Tr_ex(call);
}

void ASTToTreeVisitor::visit(fdmj::GetInt* /*node*/) {
    vector<tree::Exp*>* args = nullptr;
    tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, "getint", args);
    visit_exp_result = new Tr_ex(call);
}

void ASTToTreeVisitor::visit(fdmj::GetCh* /*node*/) {
    vector<tree::Exp*>* args = nullptr;
    tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, "getch", args);
    visit_exp_result = new Tr_ex(call);
}

void ASTToTreeVisitor::visit(fdmj::GetArray* /*node*/) {
    cerr << "Error: GetArray not supported in HW3" << endl;
    visit_exp_result = nullptr;
}

