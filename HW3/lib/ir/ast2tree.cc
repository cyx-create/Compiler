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
    
    // 第一遍：建立继承关系
    for (const auto& class_name : *nm->get_class_list()) {
        string parent = nm->get_parent(class_name);
        if (!parent.empty()) {
            ct->parent_map[class_name] = parent;
        }
    }
    
    // 第二遍：计算所有类的变量偏移（包括继承）
    // 收集所有类名，按拓扑排序（父类在前）
    vector<string> class_order;
    // 简单处理：确保父类在子类之前
    for (const auto& class_name : *nm->get_class_list()) {
        if (ct->parent_map.find(class_name) == ct->parent_map.end()) {
            class_order.push_back(class_name);
        }
    }
    // 添加子类
    for (const auto& class_name : *nm->get_class_list()) {
        if (ct->parent_map.find(class_name) != ct->parent_map.end()) {
            class_order.push_back(class_name);
        }
    }
    
    // 计算每个类的变量偏移
    int current_offset = 0;
    for (const auto& class_name : *nm->get_class_list()) {
        // 添加当前类的变量
        set<string>* vars = nm->get_class_var_list(class_name);
        for (const auto& v : *vars) {
            string key = class_name + "^" + v;
            ct->var_pos_map[key] = current_offset++;
        }
        delete vars;
    }

    // 记录每个类的大小（变量数量）
    for (const auto& class_name : *nm->get_class_list()) {
        set<string>* vars = nm->get_class_var_list(class_name);
        ct->class_size_map[class_name] = vars->size() + 1;  // +1 for vptr
        delete vars;
    }

    // 构建虚函数表（所有类共享同一个 vtable）
    vector<string> global_vtable;
    for (const auto& class_name : *nm->get_class_list()) {
        set<string>* methods = nm->get_method_list(class_name);
        for (const auto& m : *methods) {
            // 检查是否已存在
            bool found = false;
            for (size_t i = 0; i < global_vtable.size(); i++) {
                if (global_vtable[i] == m) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                global_vtable.push_back(m);
            }
        }
        delete methods;
    }

    // 所有类共享同一个 vtable
    for (const auto& class_name : *nm->get_class_list()) {
        ct->vtable_map[class_name] = global_vtable;
        for (size_t i = 0; i < global_vtable.size(); i++) {
            ct->method_pos_map[global_vtable[i]] = i;
        }
    }
    
    return ct;
}

Method_var_table* generate_method_var_table(string class_name, string method_name, Name_Maps* nm, Temp_map* tm, AST_Semant_Map* semant_map) {
    auto* mvt = new Method_var_table();
    if (nm == nullptr || tm == nullptr) return mvt;

    // 为非main方法添加this参数
    if (class_name != "__$main__" || method_name != "main") {
        tree::Temp* this_temp = tm->newtemp();
        (*mvt->var_temp_map)["this"] = this_temp;
        (*mvt->var_type_map)["this"] = tree::Type::PTR;
    }
    
    // 添加局部变量 - 从Name_Maps获取类型
    set<string>* locals = nm->get_method_var_list(class_name, method_name);
    for (const auto& v : *locals) {
        (*mvt->var_temp_map)[v] = tm->newtemp();
        
        // 从Name_Maps获取VarDecl来获取类型
        tree::Type var_type = tree::Type::INT; // 默认
        VarDecl* vd = nm->get_method_var(class_name, method_name, v);
        if (vd != nullptr && vd->type != nullptr) {
            if (vd->type->typeKind == fdmj::TypeKind::ARRAY || 
                vd->type->typeKind == fdmj::TypeKind::CLASS) {
                var_type = tree::Type::PTR;
            } else {
                var_type = tree::Type::INT;
            }
        }
        
        (*mvt->var_type_map)[v] = var_type;
    }
    delete locals;

    // 添加形式参数
    vector<Formal*>* formals = nm->get_method_formal_list(class_name, method_name);
    for (auto* f : *formals) {
        if (f == nullptr || f->id == nullptr) continue;
        const string fname = f->id->id;
        if (mvt->var_temp_map->find(fname) != mvt->var_temp_map->end()) {
            continue; // local overrides formal
        }
        
        // 从Name_Maps获取Formal来获取类型
        tree::Type t = tree::Type::INT;
        Formal* formal = nm->get_method_formal(class_name, method_name, fname);
        if (formal != nullptr && formal->type != nullptr) {
            if (formal->type->typeKind == fdmj::TypeKind::ARRAY || 
                formal->type->typeKind == fdmj::TypeKind::CLASS) {
                t = tree::Type::PTR;
            } else {
                t = tree::Type::INT;
            }
        }
        
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
    
    // 先生成class table
    class_table = generate_class_table(semant_map);
    
    // 收集所有的MethodDecl
    map<string, fdmj::MethodDecl*> method_decls;
    if (node->cdl != nullptr) {
        for (auto* class_decl : *node->cdl) {
            if (class_decl == nullptr) continue;
            string class_name = class_decl->id->id;
            if (class_decl->mdl != nullptr) {
                for (auto* method : *class_decl->mdl) {
                    if (method == nullptr || method->id == nullptr) continue;
                    string method_name = method->id->id;
                    string key = class_name + "^" + method_name;
                    method_decls[key] = method;
                }
            }
        }
    }
    
    // 翻译main method
    if (node->main == nullptr) {
        cerr << "Error: Program has no MainMethod" << endl;
        visit_tree_result = nullptr;
        return;
    }
    node->main->accept(*this);
    tree::Program* prog = static_cast<tree::Program*>(visit_tree_result);
    
    // 翻译所有class methods
    if (prog != nullptr && prog->funcdecllist != nullptr) {
        for (const auto& class_name : *semant_map->getNameMaps()->get_class_list()) {
            set<string>* methods = semant_map->getNameMaps()->get_method_list(class_name);
            for (const auto& method_name : *methods) {
                string key = class_name + "^" + method_name;
                if (method_decls.find(key) != method_decls.end()) {
                    current_class = class_name;
                    current_method = method_name;
                    method_decls[key]->accept(*this);
                    tree::FuncDecl* fd = static_cast<tree::FuncDecl*>(visit_tree_result);
                    if (fd != nullptr) {
                        prog->funcdecllist->push_back(fd);
                    }
                }
            }
            delete methods;
        }
    }
    
    visit_tree_result = prog;
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
                                                 method_temp_map,
                                                 semant_map);

    // 收集所有语句（变量声明 + 普通语句）
    vector<tree::Stm*>* all_stms = new vector<tree::Stm*>();
    
    // 处理变量声明
if (node->vdl != nullptr) {
    for (auto* vd : *node->vdl) {
        if (vd == nullptr) continue;
        vd->accept(*this);
        tree::Stm* init_stm = static_cast<tree::Stm*>(visit_tree_result);
        if (init_stm != nullptr) {
            // 如果是Seq，展开它
            tree::Seq* seq = dynamic_cast<tree::Seq*>(init_stm);
            if (seq != nullptr && seq->sl != nullptr) {
                for (auto* stm : *seq->sl) {
                    if (stm != nullptr) {
                        all_stms->push_back(stm);
                    }
                }
            } else {
                all_stms->push_back(init_stm);
            }
        }
    }
}
    
    // 处理普通语句
    if (node->sl != nullptr) {
        for (auto* s : *node->sl) {
            if (s == nullptr) continue;
            s->accept(*this);
            auto* stm = static_cast<tree::Stm*>(visit_tree_result);
            if (stm != nullptr) {
                all_stms->push_back(stm);
            }
        }
    }
    
    // 创建body序列
    tree::Stm* body_seq = nullptr;
    if (all_stms->empty()) {
        body_seq = new tree::Seq(nullptr);
    } else {
        // 多个语句，用Seq包装
        body_seq = new tree::Seq(all_stms);
    }

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
void ASTToTreeVisitor::visit(fdmj::VarDecl* node) {
    if (node == nullptr || node->id == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    
    string var_name = node->id->id;
    
    // 检查是否是类类型
    if (node->type != nullptr && node->type->typeKind == fdmj::TypeKind::CLASS) {
        tree::Temp* var_temp = method_var_table->get_var_temp(var_name);
        if (var_temp != nullptr) {
            // 初始化为 null (0)
            visit_tree_result = new tree::Move(
                new tree::TempExp(tree::Type::PTR, var_temp),
                new tree::Const(0)
            );
            return;
        }
    }
    
    // 检查是否是数组类型
    if (node->type != nullptr && node->type->typeKind == fdmj::TypeKind::ARRAY) {
        vector<tree::Stm*>* init_stms = new vector<tree::Stm*>();
        
        // 检查是否有初始化列表
        if (node->init.index() == 2) {
            vector<IntExp*>* init_array = get<vector<IntExp*>*>(node->init);
            if (init_array != nullptr) {
                int actual_size = init_array->size();
                
                // 获取变量的Temp
                tree::Temp* var_temp = method_var_table->get_var_temp(var_name);
                if (var_temp == nullptr) {
                    visit_tree_result = nullptr;
                    return;
                }
                
                // 计算需要分配的内存大小
                int total_bytes = (actual_size + 1) * 4;
                
                // 1. 分配内存，直接赋值给变量
                vector<tree::Exp*>* args = new vector<tree::Exp*>();
                args->push_back(new tree::Const(total_bytes));
                tree::Exp* new_array = new tree::ExtCall(tree::Type::PTR, "malloc", args);
                
                tree::Stm* assign_to_var = new tree::Move(
                    new tree::TempExp(tree::Type::PTR, var_temp),
                    new_array
                );
                init_stms->push_back(assign_to_var);
                
                // 2. 设置数组长度（使用变量地址）
                tree::Stm* set_length = new tree::Move(
                    new tree::Mem(tree::Type::INT, new tree::TempExp(tree::Type::PTR, var_temp)),
                    new tree::Const(actual_size)
                );
                init_stms->push_back(set_length);
                
                // 3. 初始化数组元素（使用变量地址）
                for (size_t i = 0; i < init_array->size(); i++) {
                    IntExp* init_val = (*init_array)[i];
                    if (init_val != nullptr) {
                        int offset = (i + 1) * 4;
                        tree::Exp* elem_addr = new tree::Binop(
                            tree::Type::PTR, 
                            "+", 
                            new tree::TempExp(tree::Type::PTR, var_temp), 
                            new tree::Const(offset)
                        );
                        tree::Stm* init_elem = new tree::Move(
                            new tree::Mem(tree::Type::INT, elem_addr),
                            new tree::Const(init_val->val)
                        );
                        init_stms->push_back(init_elem);
                    }
                }
            }
        }
        
        if (init_stms->empty()) {
            visit_tree_result = nullptr;
        } else if (init_stms->size() == 1) {
            visit_tree_result = init_stms->front();
        } else {
            visit_tree_result = new tree::Seq(init_stms);
        }
    } else {
        // 其他类型（如int）不做处理
        visit_tree_result = nullptr;
    }
}

void ASTToTreeVisitor::visit(fdmj::MethodDecl* node) {
    if (node == nullptr || node->id == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    
    string method_name = node->id->id;
    string ir_func_name = current_class + "^" + method_name;
    
    // 创建新的Temp_map和Method_var_table
    Temp_map* method_tm = new Temp_map();
    Method_var_table* mvt = new Method_var_table();
    
    // 添加this参数（所有非static方法都有this）
    tree::Temp* this_temp = method_tm->newtemp();
    (*mvt->var_temp_map)["this"] = this_temp;
    (*mvt->var_type_map)["this"] = tree::Type::PTR;
    
    // 获取Name_Maps
    Name_Maps* nm = semant_map ? semant_map->getNameMaps() : nullptr;
    
    // 添加方法参数 - 从Name_Maps获取类型
    if (node->fl != nullptr) {
        for (auto* formal : *node->fl) {
            if (formal == nullptr || formal->id == nullptr) continue;
            string param_name = formal->id->id;
            tree::Temp* param_temp = method_tm->newtemp();
            (*mvt->var_temp_map)[param_name] = param_temp;
            
            // 从Name_Maps获取Formal节点来获取类型
            tree::Type param_type = tree::Type::INT;
            if (nm != nullptr) {
                Formal* f = nm->get_method_formal(current_class, method_name, param_name);
                if (f != nullptr && f->type != nullptr) {
                    if (f->type->typeKind == fdmj::TypeKind::ARRAY || 
                        f->type->typeKind == fdmj::TypeKind::CLASS) {
                        param_type = tree::Type::PTR;
                    } else {
                        param_type = tree::Type::INT;
                    }
                }
            }
            (*mvt->var_type_map)[param_name] = param_type;
        }
    }
    
    // 添加局部变量 - 从Name_Maps获取类型
    if (node->vdl != nullptr) {
        for (auto* var_decl : *node->vdl) {
            if (var_decl == nullptr || var_decl->id == nullptr) continue;
            string var_name = var_decl->id->id;
            
            // 检查是否与参数冲突（局部变量覆盖参数）
            if (mvt->var_temp_map->find(var_name) != mvt->var_temp_map->end()) {
                (*mvt->var_temp_map)[var_name] = method_tm->newtemp();
            } else {
                (*mvt->var_temp_map)[var_name] = method_tm->newtemp();
            }
            
            // 从Name_Maps获取VarDecl来获取类型
            tree::Type var_type = tree::Type::INT;
            if (nm != nullptr) {
                VarDecl* vd = nm->get_method_var(current_class, method_name, var_name);
                if (vd != nullptr && vd->type != nullptr) {
                    if (vd->type->typeKind == fdmj::TypeKind::ARRAY || 
                        vd->type->typeKind == fdmj::TypeKind::CLASS) {
                        var_type = tree::Type::PTR;
                    } else {
                        var_type = tree::Type::INT;
                    }
                }
            }
            (*mvt->var_type_map)[var_name] = var_type;
        }
    }
    
    // 保存当前的context
    Method_var_table* old_mvt = method_var_table;
    Temp_map* old_tm = method_temp_map;
    string old_class = current_class;
    string old_method = current_method;
    
    method_var_table = mvt;
    method_temp_map = method_tm;
    current_class = this->current_class;
    current_method = method_name;
    
    // 翻译方法体
    vector<tree::Stm*>* stm_ir_list = new vector<tree::Stm*>();
    if (node->sl != nullptr) {
        for (auto* stm : *node->sl) {
            if (stm == nullptr) continue;
            stm->accept(*this);
            tree::Stm* ir_stm = static_cast<tree::Stm*>(visit_tree_result);
            if (ir_stm != nullptr) {
                stm_ir_list->push_back(ir_stm);
            }
        }
    }
    
    // 添加return语句（如果没有）
    // bool has_return = false;
    // for (auto* stm : *stm_ir_list) {
    //     if (dynamic_cast<tree::Return*>(stm) != nullptr) {
    //         has_return = true;
    //         break;
    //     }
    // }
    // if (!has_return) {
    //     stm_ir_list->push_back(new tree::Return(new tree::Const(0)));
    // }
    
    tree::Seq* body_seq = new tree::Seq(stm_ir_list);
    
    // 构建参数列表（只包含Temp*，不包含类型信息）
    vector<tree::Temp*>* params = new vector<tree::Temp*>();
    params->push_back(this_temp);  // 第一个参数是this
    
    if (node->fl != nullptr) {
        for (auto* formal : *node->fl) {
            if (formal == nullptr || formal->id == nullptr) continue;
            string param_name = formal->id->id;
            tree::Temp* param_temp = (*mvt->var_temp_map)[param_name];
            params->push_back(param_temp);
        }
    }
    
    int last_temp;
    if (stm_ir_list->empty() && node->sl == nullptr) {
        // 空方法体：使用 next_temp（下一个可用编号）
        last_temp = method_temp_map->next_temp;
    } else {
        // 非空方法体：使用最后使用的编号
        last_temp = method_temp_map->next_temp - 1;
    }
    int last_label = method_temp_map->next_label - 1;
    
    // 创建FuncDecl，return_type根据方法的返回类型设置
    tree::Type return_type = tree::Type::INT;  // 默认int
    if (node->type != nullptr) {
        return_type = fdmjTypeToTreeType(node->type->typeKind);
    }
    
    tree::FuncDecl* fd = new tree::FuncDecl(ir_func_name, params, body_seq, return_type, last_temp, last_label);
    
    // 恢复原来的context
    method_var_table = old_mvt;
    method_temp_map = old_tm;
    current_class = old_class;
    current_method = old_method;
    
    visit_tree_result = fd;
}

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

// 边界检查
tree::Exp* ASTToTreeVisitor::getArrayElementValue(tree::Exp* arr_addr, tree::Exp* index_exp) {
    // 检查索引是否是常量
    tree::Const* const_index = dynamic_cast<tree::Const*>(index_exp);
    
    if (const_index != nullptr) {
        int index_val = const_index->constVal;
        
        // 获取数组长度
        tree::Temp* length_temp = method_temp_map->newtemp();
        tree::Stm* store_length = new tree::Move(
            new tree::TempExp(tree::Type::INT, length_temp),
            new tree::Mem(tree::Type::INT, arr_addr)
        );
        
        // 标签创建顺序
        Label* label_100 = method_temp_map->newlabel();
        Label* label_101 = method_temp_map->newlabel();
        Label* label_102 = method_temp_map->newlabel();
        
        // 第一个检查：index >= 0
        tree::Cjump* check_ge = new tree::Cjump(
            ">=", 
            new tree::Const(index_val),
            new tree::Const(0),
            label_101,  // true 跳转到 101
            label_100   // false 跳转到 100
        );
        
        // 第二个检查：index >= length
        tree::Cjump* check_ge_length = new tree::Cjump(
            ">=", 
            new tree::Const(index_val),
            new tree::TempExp(tree::Type::INT, length_temp),
            label_100,  // true 跳转到 100
            label_102   // false 跳转到 102
        );
        
        // 越界处理
        vector<tree::Exp*>* exit_args = new vector<tree::Exp*>();
        exit_args->push_back(new tree::Const(-1));
        tree::ExtCall* exit_call = new tree::ExtCall(tree::Type::INT, "exit", exit_args);
        tree::Stm* exit_stm = new tree::ExpStm(exit_call);
        
        // 内层 ESeq：边界检查
        vector<tree::Stm*>* inner_stms = new vector<tree::Stm*>();
        inner_stms->push_back(store_length);
        inner_stms->push_back(check_ge);
        inner_stms->push_back(new tree::LabelStm(label_101));
        inner_stms->push_back(check_ge_length);
        inner_stms->push_back(new tree::LabelStm(label_100));
        inner_stms->push_back(exit_stm);
        inner_stms->push_back(new tree::LabelStm(label_102));
        
        tree::Seq* inner_seq = new tree::Seq(inner_stms);
        tree::Exp* inner_eseq = new tree::Eseq(tree::Type::INT, inner_seq, new tree::Const(index_val));
        
        // 计算元素地址
        tree::Exp* elem_index = new tree::Binop(tree::Type::INT, "+", inner_eseq, new tree::Const(1));
        tree::Exp* byte_offset = new tree::Binop(tree::Type::INT, "*", elem_index, new tree::Const(4));
        tree::Exp* elem_addr = new tree::Binop(tree::Type::PTR, "+", arr_addr, byte_offset);
        tree::Exp* elem_value = new tree::Mem(tree::Type::INT, elem_addr);
        
        return elem_value;  // 直接返回，不包装外层 ESeq
    } else {
        // 变量索引：需要存储到临时变量
        tree::Temp* index_temp = method_temp_map->newtemp();
        tree::Stm* store_index = new tree::Move(
            new tree::TempExp(tree::Type::INT, index_temp),
            index_exp
        );
        
        // 获取数组长度
        tree::Temp* length_temp = method_temp_map->newtemp();
        tree::Stm* store_length = new tree::Move(
            new tree::TempExp(tree::Type::INT, length_temp),
            new tree::Mem(tree::Type::INT, arr_addr)
        );
        
        // 标签创建顺序与 getArrayElementForAssign 保持一致
        Label* label_100 = method_temp_map->newlabel();
        Label* label_101 = method_temp_map->newlabel();
        Label* label_102 = method_temp_map->newlabel();
        
        // 检查 index >= 0
        tree::Cjump* check_ge = new tree::Cjump(
            ">=", 
            new tree::TempExp(tree::Type::INT, index_temp),
            new tree::Const(0),
            label_101,  // true 跳转到 101
            label_100   // false 跳转到 100
        );
        
        // 检查 index >= length
        tree::Cjump* check_ge_length = new tree::Cjump(
            ">=", 
            new tree::TempExp(tree::Type::INT, index_temp),
            new tree::TempExp(tree::Type::INT, length_temp),
            label_100,  // true 跳转到 100
            label_102   // false 跳转到 102
        );
        
        // 越界处理
        vector<tree::Exp*>* exit_args = new vector<tree::Exp*>();
        exit_args->push_back(new tree::Const(-1));
        tree::ExtCall* exit_call = new tree::ExtCall(tree::Type::INT, "exit", exit_args);
        tree::Stm* exit_stm = new tree::ExpStm(exit_call);
        
        // 内层 ESeq：边界检查，返回索引值
        vector<tree::Stm*>* inner_stms = new vector<tree::Stm*>();
        inner_stms->push_back(store_length);
        inner_stms->push_back(check_ge);
        inner_stms->push_back(new tree::LabelStm(label_101));
        inner_stms->push_back(check_ge_length);
        inner_stms->push_back(new tree::LabelStm(label_100));
        inner_stms->push_back(exit_stm);
        inner_stms->push_back(new tree::LabelStm(label_102));
        
        tree::Seq* inner_seq = new tree::Seq(inner_stms);
        tree::Exp* inner_eseq = new tree::Eseq(tree::Type::INT, inner_seq, new tree::TempExp(tree::Type::INT, index_temp));
        
        // 计算元素地址
        tree::Exp* elem_index = new tree::Binop(tree::Type::INT, "+", inner_eseq, new tree::Const(1));
        tree::Exp* byte_offset = new tree::Binop(tree::Type::INT, "*", elem_index, new tree::Const(4));
        tree::Exp* elem_addr = new tree::Binop(tree::Type::PTR, "+", arr_addr, byte_offset);
        tree::Exp* elem_value = new tree::Mem(tree::Type::INT, elem_addr);
        
        // 外层 ESeq：先存储索引，然后返回元素值
        vector<tree::Stm*>* outer_stms = new vector<tree::Stm*>();
        outer_stms->push_back(store_index);
        tree::Seq* outer_seq = new tree::Seq(outer_stms);
        
        return new tree::Eseq(tree::Type::INT, outer_seq, elem_value);
    }
}

tree::Exp* ASTToTreeVisitor::getArrayElementForAssign(tree::Exp* arr_addr, tree::Exp* index_exp) {
    // 检查索引是否需要计算（是 BinOp 表达式，如 0-123）
    tree::Binop* binop_index = dynamic_cast<tree::Binop*>(index_exp);
    
    tree::Exp* index_to_use = index_exp;
    vector<tree::Stm*>* outer_stms = nullptr;
    
    // 只有 BinOp 表达式才需要先计算并存储到临时变量
    if (binop_index != nullptr) {
        // 创建临时变量存储索引计算结果
        tree::Temp* index_temp = method_temp_map->newtemp();
        tree::Stm* store_index = new tree::Move(
            new tree::TempExp(tree::Type::INT, index_temp),
            index_exp
        );
        index_to_use = new tree::TempExp(tree::Type::INT, index_temp);
        
        outer_stms = new vector<tree::Stm*>();
        outer_stms->push_back(store_index);
    }
    
    // 获取数组长度
    tree::Temp* length_temp = method_temp_map->newtemp();
    tree::Stm* store_length = new tree::Move(
        new tree::TempExp(tree::Type::INT, length_temp),
        new tree::Mem(tree::Type::INT, arr_addr)
    );
    
    // 标签创建顺序
    Label* label_100 = method_temp_map->newlabel();
    Label* label_101 = method_temp_map->newlabel();
    Label* label_102 = method_temp_map->newlabel();
    
    // 第一个检查：index >= 0
    tree::Cjump* check_ge = new tree::Cjump(
        ">=", 
        index_to_use,
        new tree::Const(0),
        label_101,  // true 跳转到 101
        label_100   // false 跳转到 100
    );
    
    // 第二个检查：index >= length
    tree::Cjump* check_ge_length = new tree::Cjump(
        ">=", 
        index_to_use,
        new tree::TempExp(tree::Type::INT, length_temp),
        label_100,  // true 跳转到 100
        label_102   // false 跳转到 102
    );
    
    // 越界处理
    vector<tree::Exp*>* exit_args = new vector<tree::Exp*>();
    exit_args->push_back(new tree::Const(-1));
    tree::ExtCall* exit_call = new tree::ExtCall(tree::Type::INT, "exit", exit_args);
    tree::Stm* exit_stm = new tree::ExpStm(exit_call);
    
    // 内层 ESeq：边界检查，返回索引值
    vector<tree::Stm*>* inner_stms = new vector<tree::Stm*>();
    inner_stms->push_back(store_length);
    inner_stms->push_back(check_ge);
    inner_stms->push_back(new tree::LabelStm(label_101));
    inner_stms->push_back(check_ge_length);
    inner_stms->push_back(new tree::LabelStm(label_100));
    inner_stms->push_back(exit_stm);
    inner_stms->push_back(new tree::LabelStm(label_102));
    
    tree::Seq* inner_seq = new tree::Seq(inner_stms);
    tree::Exp* inner_eseq = new tree::Eseq(tree::Type::INT, inner_seq, index_to_use);
    
    // 计算元素地址
    tree::Exp* elem_index = new tree::Binop(tree::Type::INT, "+", inner_eseq, new tree::Const(1));
    tree::Exp* byte_offset = new tree::Binop(tree::Type::INT, "*", elem_index, new tree::Const(4));
    tree::Exp* elem_addr = new tree::Binop(tree::Type::PTR, "+", arr_addr, byte_offset);
    tree::Exp* elem_mem = new tree::Mem(tree::Type::INT, elem_addr);
    
    // 如果有外层语句，包装成 Eseq
    if (outer_stms != nullptr) {
        tree::Seq* outer_seq = new tree::Seq(outer_stms);
        return new tree::Eseq(tree::Type::PTR, outer_seq, elem_mem);
    }
    
    return elem_mem;
}

void ASTToTreeVisitor::visit(fdmj::Assign* node) {
    if (node == nullptr || node->left == nullptr || node->exp == nullptr) {
        visit_tree_result = nullptr;
        return;
    }

// 处理 ClassVar 赋值：a.i = 1
fdmj::ClassVar* class_var = dynamic_cast<fdmj::ClassVar*>(node->left);
if (class_var != nullptr) {
    // 翻译对象表达式
    class_var->obj->accept(*this);
    Tr_Exp* obj_tr = visit_exp_result;
    if (obj_tr == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    tree::Exp* obj_addr = obj_tr->unEx(method_temp_map)->exp;
    if (obj_addr == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    
    string var_name = class_var->id->id;
    
    // 从语义分析获取对象的实际类型
    string class_name;
    if (semant_map != nullptr) {
        AST_Semant* semant = semant_map->getSemant(class_var->obj);
        if (semant != nullptr && semant->get_kind() == AST_Semant::Kind::Value) {
            // 如果是类类型，typeKind 应该是 CLASS
            if (semant->get_type() == fdmj::TypeKind::CLASS) {
                variant<monostate, string, int> type_par = semant->get_type_par();
                if (holds_alternative<string>(type_par)) {
                    class_name = get<string>(type_par);
                }
            }
        }
    }
    
    if (class_name.empty()) {
        cerr << "Error: Cannot determine class type for expression" << endl;
        visit_tree_result = nullptr;
        return;
    }
    
    // 获取变量偏移
    int offset = class_table->get_var_pos(class_name, var_name);
    if (offset < 0) {
        cerr << "Error: Variable " << var_name << " not found in class " << class_name << endl;
        visit_tree_result = nullptr;
        return;
    }
    
    // 计算变量地址：obj_addr + offset * 4
    tree::Exp* var_addr = new tree::Binop(
        tree::Type::PTR, 
        "+", 
        obj_addr, 
        new tree::Const(offset * 4)
    );
    tree::Exp* dst = new tree::Mem(tree::Type::INT, var_addr);
    
    // 翻译右值
    node->exp->accept(*this);
    Tr_Exp* rhs_tr = visit_exp_result;
    if (rhs_tr == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    tree::Exp* src = rhs_tr->unEx(method_temp_map)->exp;
    
    visit_tree_result = new tree::Move(dst, src);
    return;
}
    
    // 处理数组赋值：a[index] = value
    fdmj::ArrayExp* array_exp = dynamic_cast<fdmj::ArrayExp*>(node->left);
    if (array_exp != nullptr) {
        // 翻译数组表达式
        array_exp->arr->accept(*this);
        Tr_Exp* arr_tr = visit_exp_result;
        if (arr_tr == nullptr) {
            visit_tree_result = nullptr;
            return;
        }
        tree::Exp* arr_addr = arr_tr->unEx(method_temp_map)->exp;
        if (arr_addr == nullptr) {
            visit_tree_result = nullptr;
            return;
        }
        
        // 翻译索引表达式
        array_exp->index->accept(*this);
        Tr_Exp* index_tr = visit_exp_result;
        if (index_tr == nullptr) {
            visit_tree_result = nullptr;
            return;
        }
        tree::Exp* index_exp = index_tr->unEx(method_temp_map)->exp;
        if (index_exp == nullptr) {
            visit_tree_result = nullptr;
            return;
        }
        
        // 翻译右值
        node->exp->accept(*this);
        Tr_Exp* rhs_tr = visit_exp_result;
        if (rhs_tr == nullptr) {
            visit_tree_result = nullptr;
            return;
        }
        tree::Exp* value = rhs_tr->unEx(method_temp_map)->exp;
        if (value == nullptr) {
            visit_tree_result = nullptr;
            return;
        }
        
        // 获取带边界检查的元素内存位置
        tree::Exp* elem_mem_with_check = getArrayElementForAssign(arr_addr, index_exp);
        
        tree::Stm* assign_stm = new tree::Move(elem_mem_with_check, value);
        visit_tree_result = assign_stm;
        return;
    }
    
    // 处理对象赋值：a = new A()
    fdmj::IdExp* id_exp = dynamic_cast<fdmj::IdExp*>(node->left);
    fdmj::NewObject* new_object = dynamic_cast<fdmj::NewObject*>(node->exp);
    
    if (id_exp != nullptr && new_object != nullptr) {
        string var_name = id_exp->id;
        tree::Temp* var_temp = method_var_table->get_var_temp(var_name);
        
        if (var_temp != nullptr) {
            new_object->accept(*this);
            Tr_Exp* new_obj_tr = visit_exp_result;
            if (new_obj_tr != nullptr) {
                tree::Exp* new_obj_addr = new_obj_tr->unEx(method_temp_map)->exp;
                visit_tree_result = new tree::Move(
                    new tree::TempExp(tree::Type::PTR, var_temp),
                    new_obj_addr
                );
                return;
            }
        }
    }
    
    // 处理数组赋值：a = new int[3]
    fdmj::NewArray* new_array = dynamic_cast<fdmj::NewArray*>(node->exp);
    
    if (id_exp != nullptr && new_array != nullptr) {
        string var_name = id_exp->id;
        tree::Temp* var_temp = method_var_table->get_var_temp(var_name);
        
        if (var_temp != nullptr) {
            new_array->accept(*this);
            Tr_Exp* new_arr_tr = visit_exp_result;
            if (new_arr_tr != nullptr) {
                tree::Exp* new_arr_addr = new_arr_tr->unEx(method_temp_map)->exp;
                visit_tree_result = new tree::Move(
                    new tree::TempExp(tree::Type::PTR, var_temp),
                    new_arr_addr
                );
                return;
            }
        }
    }
    
    // 普通赋值（IdExp = Exp）
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

void ASTToTreeVisitor::visit(fdmj::PutArray* node) {
    if (node == nullptr || node->n == nullptr || node->arr == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    
    // 翻译要输出的元素个数
    node->n->accept(*this);
    Tr_Exp* n_tr = visit_exp_result;
    if (n_tr == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    tree::Exp* n_exp = n_tr->unEx(method_temp_map)->exp;
    
    // 翻译数组
    node->arr->accept(*this);
    Tr_Exp* arr_tr = visit_exp_result;
    if (arr_tr == nullptr) {
        visit_tree_result = nullptr;
        return;
    }
    tree::Exp* arr_addr = arr_tr->unEx(method_temp_map)->exp;
    
    // 创建外部调用 putarray
    vector<tree::Exp*>* args = new vector<tree::Exp*>();
    args->push_back(n_exp);
    args->push_back(arr_addr);
    tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, "putarray", args);
    visit_tree_result = new tree::ExpStm(call);
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
    
    // 获取变量的类型
    tree::Type var_type = method_var_table->get_var_type(name);
    
    visit_exp_result = new Tr_ex(new tree::TempExp(var_type, t));
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

void ASTToTreeVisitor::visit(fdmj::ArrayExp* node) {
    if (node == nullptr || node->arr == nullptr || node->index == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    
    // 翻译数组表达式
    node->arr->accept(*this);
    Tr_Exp* arr_tr = visit_exp_result;
    if (arr_tr == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    tree::Exp* arr_addr = arr_tr->unEx(method_temp_map)->exp;
    if (arr_addr == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    
    // 翻译索引表达式
    node->index->accept(*this);
    Tr_Exp* index_tr = visit_exp_result;
    if (index_tr == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    tree::Exp* index_exp = index_tr->unEx(method_temp_map)->exp;
    if (index_exp == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    
    // 使用辅助函数生成带边界检查的元素访问
    tree::Exp* elem_value = getArrayElementValue(arr_addr, index_exp);
    
    visit_exp_result = new Tr_ex(elem_value);
}

void ASTToTreeVisitor::visit(fdmj::ClassVar* node) {
    if (node == nullptr || node->obj == nullptr || node->id == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    
    // 翻译对象表达式
    node->obj->accept(*this);
    Tr_Exp* obj_tr = visit_exp_result;
    if (obj_tr == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    tree::Exp* obj_addr = obj_tr->unEx(method_temp_map)->exp;
    if (obj_addr == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    
    string var_name = node->id->id;
    
    // 从语义分析获取对象的实际类型
    string class_name;
    if (semant_map != nullptr) {
        AST_Semant* semant = semant_map->getSemant(node->obj);
        if (semant != nullptr && semant->get_kind() == AST_Semant::Kind::Value) {
            if (semant->get_type() == fdmj::TypeKind::CLASS) {
                variant<monostate, string, int> type_par = semant->get_type_par();
                if (holds_alternative<string>(type_par)) {
                    class_name = get<string>(type_par);
                }
            }
        }
    }
    
    if (class_name.empty()) {
        cerr << "Error: Cannot determine class type for expression" << endl;
        visit_exp_result = nullptr;
        return;
    }
    
    if (class_table == nullptr) {
        cerr << "Error: class_table is null in ClassVar" << endl;
        visit_exp_result = nullptr;
        return;
    }
    
    // 获取变量偏移
    int offset = class_table->get_var_pos(class_name, var_name);
    if (offset < 0) {
        cerr << "Error: Variable " << var_name << " not found in class " << class_name << endl;
        visit_exp_result = nullptr;
        return;
    }
    
    // 计算变量地址：obj_addr + offset * 4
    tree::Exp* var_addr = new tree::Binop(
        tree::Type::PTR, 
        "+", 
        obj_addr, 
        new tree::Const(offset * 4)
    );
    tree::Exp* var_value = new tree::Mem(tree::Type::INT, var_addr);
    
    visit_exp_result = new Tr_ex(var_value);
}

void ASTToTreeVisitor::visit(fdmj::This* node) {
    if (node == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    
    if (method_var_table == nullptr) {
        cerr << "Error: 'this' used outside method" << endl;
        visit_exp_result = nullptr;
        return;
    }
    
    tree::Temp* this_temp = method_var_table->get_var_temp("this");
    if (this_temp == nullptr) {
        cerr << "Error: 'this' not found in method_var_table" << endl;
        visit_exp_result = nullptr;
        return;
    }
    
    visit_exp_result = new Tr_ex(new tree::TempExp(tree::Type::PTR, this_temp));
}

void ASTToTreeVisitor::visit(fdmj::Length* node) {
    if (node == nullptr || node->exp == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    
    // 翻译数组表达式
    node->exp->accept(*this);
    Tr_Exp* arr_tr = visit_exp_result;
    if (arr_tr == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    tree::Exp* arr_addr = arr_tr->unEx(method_temp_map)->exp;
    if (arr_addr == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    
    // 数组长度存储在数组的第一个位置（偏移0）
    // length() 不需要边界检查，直接返回长度
    tree::Exp* length = new tree::Mem(tree::Type::INT, arr_addr);
    
    visit_exp_result = new Tr_ex(length);
}
void ASTToTreeVisitor::visit(fdmj::NewArray* node) {
    if (node == nullptr || node->size == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    
    // 检查是否是多维数组：size 本身是一个 NewArray
    fdmj::NewArray* inner_new_array = dynamic_cast<fdmj::NewArray*>(node->size);
    
    if (inner_new_array != nullptr) {
        // 多维数组：先创建内层数组
        inner_new_array->accept(*this);
        Tr_Exp* inner_arr_tr = visit_exp_result;
        if (inner_arr_tr == nullptr) {
            visit_exp_result = nullptr;
            return;
        }
        tree::Exp* inner_arr_addr = inner_arr_tr->unEx(method_temp_map)->exp;
        
        // 外层数组的大小是1（只有一个元素，存储内层数组的地址）
        tree::Exp* size_exp = new tree::Const(1);
        
        // 创建临时变量存储外层数组地址
        tree::Temp* outer_array_temp = method_temp_map->newtemp();
        
        // 计算需要分配的内存大小：外层数组长度+1，每个元素是PTR类型（4字节）
        tree::Exp* total_elements = new tree::Binop(tree::Type::INT, "+", size_exp, new tree::Const(1));
        tree::Exp* total_bytes = new tree::Binop(tree::Type::INT, "*", total_elements, new tree::Const(4));
        
        // 分配外层数组内存
        vector<tree::Exp*>* args = new vector<tree::Exp*>();
        args->push_back(total_bytes);
        tree::Exp* malloc_call = new tree::ExtCall(tree::Type::PTR, "malloc", args);
        
        // 将分配的地址存入临时变量
        tree::Stm* assign_addr = new tree::Move(
            new tree::TempExp(tree::Type::PTR, outer_array_temp),
            malloc_call
        );
        
        // 设置外层数组长度
        tree::Stm* set_length = new tree::Move(
            new tree::Mem(tree::Type::INT, new tree::TempExp(tree::Type::PTR, outer_array_temp)),
            size_exp
        );
        
        // 将内层数组地址存入外层数组的第一个元素（偏移4字节）
        tree::Exp* elem_addr = new tree::Binop(
            tree::Type::PTR, 
            "+", 
            new tree::TempExp(tree::Type::PTR, outer_array_temp), 
            new tree::Const(4)
        );
        tree::Stm* store_inner = new tree::Move(
            new tree::Mem(tree::Type::PTR, elem_addr),
            inner_arr_addr
        );
        
        // 组合所有语句
        vector<tree::Stm*>* stms = new vector<tree::Stm*>();
        stms->push_back(assign_addr);
        stms->push_back(set_length);
        stms->push_back(store_inner);
        
        tree::Seq* seq = new tree::Seq(stms);
        tree::Exp* result = new tree::Eseq(tree::Type::PTR, seq, new tree::TempExp(tree::Type::PTR, outer_array_temp));
        
        visit_exp_result = new Tr_ex(result);
        return;
    }
    
    // 一维数组的处理（原有代码）
    // 翻译数组大小表达式
    node->size->accept(*this);
    Tr_Exp* size_tr = visit_exp_result;
    if (size_tr == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    tree::Exp* size_exp = size_tr->unEx(method_temp_map)->exp;
    if (size_exp == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    
    // 创建临时变量来存储数组地址
    tree::Temp* array_temp = method_temp_map->newtemp();
    
    // 计算需要分配的内存大小（字节数）
    tree::Exp* total_elements = new tree::Binop(tree::Type::INT, "+", size_exp, new tree::Const(1));
    tree::Exp* total_bytes = new tree::Binop(tree::Type::INT, "*", total_elements, new tree::Const(4));
    
    // 分配内存
    vector<tree::Exp*>* args = new vector<tree::Exp*>();
    args->push_back(total_bytes);
    tree::Exp* malloc_call = new tree::ExtCall(tree::Type::PTR, "malloc", args);
    
    // 将分配的地址存入临时变量
    tree::Stm* assign_addr = new tree::Move(
        new tree::TempExp(tree::Type::PTR, array_temp),
        malloc_call
    );
    
    // 设置数组长度（存储在数组的第一个位置，即偏移0）
    tree::Stm* set_length = new tree::Move(
        new tree::Mem(tree::Type::INT, new tree::TempExp(tree::Type::PTR, array_temp)),
        size_exp
    );
    
    // 组合：分配内存 -> 设置长度
    vector<tree::Stm*>* stms = new vector<tree::Stm*>();
    stms->push_back(assign_addr);
    stms->push_back(set_length);
    
    // 创建Eseq：先执行语句序列，然后返回数组地址
    tree::Seq* seq = new tree::Seq(stms);
    tree::Exp* result = new tree::Eseq(tree::Type::PTR, seq, new tree::TempExp(tree::Type::PTR, array_temp));
    
    visit_exp_result = new Tr_ex(result);
}

void ASTToTreeVisitor::visit(fdmj::NewObject* node) {
    if (node == nullptr || node->id == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    
    string class_name = node->id->id;
    
    if (class_table == nullptr) {
        cerr << "Error: class_table is null in NewObject" << endl;
        visit_exp_result = nullptr;
        return;
    }
    
    // 获取所有类的变量总数（用于计算 vptr 偏移）
    int total_vars = 0;
    Name_Maps* nm = semant_map->getNameMaps();
    for (const auto& c : *nm->get_class_list()) {
        set<string>* vars = nm->get_class_var_list(c);
        total_vars += vars->size();
        delete vars;
    }
    
    // 对象大小 = (变量总数 + 1) * 4
    int obj_size = total_vars + 1;
    int total_bytes = obj_size * 4;
    
    // 创建临时变量存储对象地址
    tree::Temp* obj_temp = method_temp_map->newtemp();
    
    // 分配对象内存
    vector<tree::Exp*>* args = new vector<tree::Exp*>();
    args->push_back(new tree::Const(total_bytes));
    tree::Exp* malloc_call = new tree::ExtCall(tree::Type::PTR, "malloc", args);
    
    // 将分配的地址存入临时变量
    tree::Stm* assign_addr = new tree::Move(
        new tree::TempExp(tree::Type::PTR, obj_temp),
        malloc_call
    );
    
    // vptr 存储在变量之后，偏移 = 变量总数 * 4
    int vptr_offset = total_vars * 4;
    
    // 设置vptr
    string vtable_label = class_name + "^f";
    tree::Exp* vtable_addr = new tree::Name(new tree::String_Label(vtable_label));
    
    tree::Exp* vptr_addr = new tree::Binop(
        tree::Type::PTR, 
        "+", 
        new tree::TempExp(tree::Type::PTR, obj_temp), 
        new tree::Const(vptr_offset)
    );
    tree::Stm* set_vptr = new tree::Move(
        new tree::Mem(tree::Type::PTR, vptr_addr),
        vtable_addr
    );
    
    // 组合
    vector<tree::Stm*>* stms = new vector<tree::Stm*>();
    stms->push_back(assign_addr);
    stms->push_back(set_vptr);
    
    tree::Seq* seq = new tree::Seq(stms);
    tree::Exp* result = new tree::Eseq(tree::Type::PTR, seq, new tree::TempExp(tree::Type::PTR, obj_temp));
    
    visit_exp_result = new Tr_ex(result);
}

void ASTToTreeVisitor::visit(fdmj::CallExp* node) {
    if (node == nullptr || node->name == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    
    // 如果有对象，则是方法调用
    if (node->obj != nullptr) {
        // 翻译对象
        node->obj->accept(*this);
        Tr_Exp* obj_tr = visit_exp_result;
        if (obj_tr == nullptr) {
            visit_exp_result = nullptr;
            return;
        }
        tree::Exp* obj_addr = obj_tr->unEx(method_temp_map)->exp;
        if (obj_addr == nullptr) {
            visit_exp_result = nullptr;
            return;
        }
        
        string method_name = node->name->id;
        
        // 获取对象的实际类型（需要通过语义分析）
        // 这里应该从semant_map获取node->obj的类型
        string class_name = class_var_class_name;
        if (class_name.empty()) {
            class_name = current_class;
        }
        
        // 检查class_table
        if (class_table == nullptr) {
            cerr << "Error: class_table is null in CallExp" << endl;
            visit_exp_result = nullptr;
            return;
        }
        
        // 获取方法在虚函数表中的索引
        int method_index = class_table->get_method_index(class_name, method_name);
        if (method_index < 0) {
            cerr << "Error: Method " << method_name << " not found in class " << class_name << endl;
            visit_exp_result = nullptr;
            return;
        }
        
        // 获取vptr：*(obj_addr)
        // 修正：Mem构造函数需要Type和Exp*两个参数
        tree::Exp* vptr = new tree::Mem(tree::Type::PTR, obj_addr);
        
        // 获取虚函数表：*(vptr)
        tree::Exp* vtable = new tree::Mem(tree::Type::PTR, vptr);
        
        // 获取方法地址：*(vtable + method_index * 4)
        tree::Exp* method_addr_exp = new tree::Mem(
            tree::Type::PTR,
            new tree::Binop(tree::Type::PTR, "+", vtable, new tree::Const(method_index * 4))
        );
        
        // 准备参数（包括this）
        vector<tree::Exp*>* args = new vector<tree::Exp*>();
        args->push_back(obj_addr);  // this参数
        
        if (node->par != nullptr) {
            for (auto* param : *node->par) {
                if (param == nullptr) continue;
                param->accept(*this);
                Tr_Exp* param_tr = visit_exp_result;
                if (param_tr == nullptr) {
                    visit_exp_result = nullptr;
                    return;
                }
                args->push_back(param_tr->unEx(method_temp_map)->exp);
            }
        }
        
        // 创建函数调用
        // 修正：Call构造函数需要Type, string, Exp*, vector<Exp*>* 四个参数
        // 根据treep.hh: Call(Type t, string id, Exp *obj, std::vector<tree::Exp*> *args)
        tree::Call* call = new tree::Call(tree::Type::INT, method_name, method_addr_exp, args);
        visit_exp_result = new Tr_ex(call);
    } else {
        // 外部函数调用（保持HW3的实现）
        string extfun = node->name->id;
        vector<tree::Exp*>* args = nullptr;
        if (node->par != nullptr) {
            args = new vector<tree::Exp*>();
            for (auto* p : *node->par) {
                if (p == nullptr) continue;
                p->accept(*this);
                Tr_Exp* param_tr = visit_exp_result;
                if (param_tr == nullptr) {
                    visit_exp_result = nullptr;
                    return;
                }
                args->push_back(param_tr->unEx(method_temp_map)->exp);
            }
        }
        tree::ExtCall* call = new tree::ExtCall(tree::Type::INT, extfun, args);
        visit_exp_result = new Tr_ex(call);
    }
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

void ASTToTreeVisitor::visit(fdmj::GetArray* node) {
    if (node == nullptr || node->exp == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    
    // GetArray 可能用于从数组读取元素
    // 根据你的语言定义，这可能和 ArrayExp 类似
    // 这里假设 GetArray 是读取整个数组（返回数组引用）
    node->exp->accept(*this);
    Tr_Exp* arr_tr = visit_exp_result;
    if (arr_tr == nullptr) {
        visit_exp_result = nullptr;
        return;
    }
    tree::Exp* arr_addr = arr_tr->unEx(method_temp_map)->exp;
    
    // GetArray 可能返回数组的基地址
    visit_exp_result = new Tr_ex(arr_addr);
}