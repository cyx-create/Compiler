#define DEBUG
#undef DEBUG

#include <iostream>
#include <variant>
#include <map>
#include <vector>
#include <algorithm>
#include "ASTheader.hh"
#include "FDMJAST.hh"
#include "namemaps.hh"

using namespace std;
using namespace fdmj;

void AST_Name_Map_Visitor::visit(Program* node) {
#ifdef DEBUG
    std::cout << "Visiting Program" << std::endl;
#endif
    if (node == nullptr) {
         return;
    }
    if (node->main != nullptr) {
        node->main->accept(*this);
    }
    if (node->cdl != nullptr) {
        for (auto cl : *(node->cdl)) {
            cl->accept(*this);
        }
    }
}

void AST_Name_Map_Visitor::visit(MainMethod* node) {
#ifdef DEBUG
    std::cout << "Visiting MainMethod" << std::endl;
#endif
    // Main method 归属于特殊类 __$main__
    current_visiting_class = "__$main__";
    current_visiting_method = "main";

    // 确保 __$main__ 类存在
    name_maps->add_class("__$main__");

    // 添加 main 方法
    name_maps->add_method("__$main__", "main");

    // ✅ 为返回类型创建 Formal 对象
    string ret_name = "_^return^_main";
    
    // 创建 int 类型（main方法返回int）
    Type* intType = new Type(node->getPos());  // INT类型
    
    // 创建 IdExp
    IdExp* retId = new IdExp(node->getPos(), ret_name);
    
    // 创建返回类型的 Formal 对象
    Formal* returnFormal = new Formal(node->getPos(), intType, retId);
    
    // ✅ 将返回类型添加到 methodFormal 映射
    name_maps->add_method_formal("__$main__", "main", ret_name, returnFormal);

    // return 放到形参列表最后
    vector<string> formal_list;
    formal_list.push_back(ret_name); 
    name_maps->add_method_formal_list("__$main__", "main", formal_list);
    // 访问本地变量声明
    if (node->vdl) {
        for (auto v : *node->vdl)
            if (v) v->accept(*this);
    }

    // 访问语句列表
    if (node->sl) {
        for (auto s : *node->sl)
            if (s) s->accept(*this);
    }

    current_visiting_method = "";
}

void AST_Name_Map_Visitor::visit(ClassDecl* node) {
#ifdef DEBUG
    std::cout << "Visiting ClassDecl" << std::endl;
#endif

    if (!node) return;

    string cname = node->id->id;

    name_maps->add_class(cname);

    current_visiting_class = cname;

    if (node->eid) {
        name_maps->add_class_hiearchy(cname, node->eid->id);
    }

    if (node->vdl) {
        for (auto v : *node->vdl)
            if (v) v->accept(*this);
    }

    if (node->mdl) {
        for (auto m : *node->mdl)
            if (m) m->accept(*this);
    }

    current_visiting_class = "";
}

void AST_Name_Map_Visitor::visit(Type* node) {
#ifdef DEBUG
    std::cout << "Visiting Type" << std::endl;
#endif
    return;
}

void AST_Name_Map_Visitor::visit(VarDecl* node) {
#ifdef DEBUG
    std::cout << "Visiting VarDecl" << std::endl;
#endif

    if (!node) return;

    string vname = node->id->id;

    // 没有做重复检查的
    // if (current_visiting_method == "") {
    //     name_maps->add_class_var(current_visiting_class, vname, node);
    // }
    // else {
    //     name_maps->add_method_var(current_visiting_class,
    //                               current_visiting_method,
    //                               vname,
    //                               node);
    // }

    // ========== 只检查同类声明中的重复 ==========
    if (current_visiting_method == "") {
        // 类变量：检查当前类是否已有同名类变量
        if (name_maps->is_class_var(current_visiting_class, vname)) {
            std::cerr << "Error: at position ";
            if (node->getPos()) {
                std::cerr << node->getPos()->print();
            }
            std::cerr << std::endl;
            std::cerr << "Error: Variable " << vname 
                      << " is already declared in class " << current_visiting_class << std::endl;
            std::cerr << "Name mapping failed due to errors. Compilation aborted." << std::endl;
            exit(1);  // 直接退出
        } else {
            name_maps->add_class_var(current_visiting_class, vname, node);
        }
    }
    else {
        // 局部变量：只检查与其他局部变量是否重复（不检查参数）
        if (name_maps->is_method_var(current_visiting_class, 
                                     current_visiting_method, 
                                     vname)) {
            std::cerr << "Error: at position ";
            if (node->getPos()) {
                std::cerr << node->getPos()->print();
            }
            std::cerr << std::endl;
            std::cerr << "Error: Variable " << vname 
                      << " is already declared in method " << current_visiting_method 
                      << " of class " << current_visiting_class << std::endl;
            std::cerr << "Name mapping failed due to errors. Compilation aborted." << std::endl;
            exit(1);  // 直接退出
        } else {
            name_maps->add_method_var(current_visiting_class,
                                      current_visiting_method,
                                      vname,
                                      node);
        }
    }


    if (auto int_init = std::get_if<IntExp*>(&node->init)) {
        if (*int_init)
            (*int_init)->accept(*this);
    }

    if (auto arr_init = std::get_if<vector<IntExp*>*>(&node->init)) {
        if (*arr_init) {
            for (auto e : **arr_init)
                if (e) e->accept(*this);
        }
    }
}

void AST_Name_Map_Visitor::visit(MethodDecl* node) {
#ifdef DEBUG
    std::cout << "Visiting MethodDecl" << std::endl;
#endif
    if (!node) return;

    string mname = node->id->id;

    //3.15 
    // 检查同一类中方法名是否重复
    if (name_maps->is_method(current_visiting_class, mname)) {
        std::cerr << "Error: at position ";
        if (node->getPos()) {
            std::cerr << node->getPos()->print();
        }
        std::cerr << std::endl;
        std::cerr << "Error: Method " << mname 
                  << " is already declared in class " << current_visiting_class << std::endl;
        std::cerr << "Name mapping failed due to errors. Compilation aborted." << std::endl;
        exit(1);  // 直接退出
    }

    // 添加方法到当前类
    name_maps->add_method(current_visiting_class, mname);

    current_visiting_method = mname;

    vector<string> formal_list;

    // 访问形参，并添加到 Name_Maps
    if (node->fl) {
        for (auto f : *node->fl) {
            string fname = f->id->id;

            // name_maps->add_method_formal(current_visiting_class,
            //                              mname,
            //                              fname,
            //                              f);

            formal_list.push_back(fname);

            f->accept(*this);
        }
    }

    // 创建返回类型的 Formal 对象
    string ret_name = "_^return^_" + mname;
    
    // 创建一个特殊的 IdExp 来表示返回类型
    IdExp* retId = new IdExp(node->getPos(), ret_name);  // 使用节点位置
    
    // 创建返回类型的 Formal 对象
    Formal* returnFormal = new Formal(node->getPos(), node->type, retId);
    
    // 将返回类型也作为一个形参加入映射
    name_maps->add_method_formal(current_visiting_class, mname, ret_name, returnFormal);
    
    formal_list.push_back(ret_name);

    // 保存整个形参列表（包括返回类型）
    name_maps->add_method_formal_list(current_visiting_class, mname, formal_list);

    // 访问局部变量声明
    if (node->vdl) {
        for (auto v : *node->vdl)
            if (v) v->accept(*this);
    }

    // 访问语句列表
    if (node->sl) {
        for (auto s : *node->sl)
            if (s) s->accept(*this);
    }

    current_visiting_method = "";
}

//3.15
void AST_Name_Map_Visitor::visit(Formal* node) {
#ifdef DEBUG
    std::cout << "Visiting Formal" << std::endl;
#endif
    if (!node) return;
    
    string fname = node->id->id;
    
    // 检查同一方法中参数名是否重复
    if (name_maps->is_method_formal(current_visiting_class, 
                                    current_visiting_method, 
                                    fname)) {
        std::cerr << "Error: at position ";
        if (node->getPos()) {
            std::cerr << node->getPos()->print();
        }
        std::cerr << std::endl;
        std::cerr << "Error: Parameter " << fname 
                  << " is already declared in method " << current_visiting_method 
                  << " of class " << current_visiting_class << std::endl;
        std::cerr << "Name mapping failed due to errors. Compilation aborted." << std::endl;
        exit(1);  // 直接退出
    } else {
        name_maps->add_method_formal(current_visiting_class,
                                     current_visiting_method,
                                     fname,
                                     node);
    }
}

void AST_Name_Map_Visitor::visit(Nested* node) {
#ifdef DEBUG
    std::cout << "Visiting Nested" << std::endl;
#endif

    if (!node || !node->sl) return;

    for (auto s : *node->sl)
        if (s) s->accept(*this);
}

void AST_Name_Map_Visitor::visit(If* node) {
#ifdef DEBUG
    std::cout << "Visiting If" << std::endl;
#endif
    if (!node) return;

    if (node->exp) node->exp->accept(*this);
    if (node->stm1) node->stm1->accept(*this);
    if (node->stm2) node->stm2->accept(*this);
}

void AST_Name_Map_Visitor::visit(While* node) {
#ifdef DEBUG
    std::cout << "Visiting While" << std::endl;
#endif
    if (!node) return;

    if (node->exp) node->exp->accept(*this);
    if (node->stm) node->stm->accept(*this);
}

void AST_Name_Map_Visitor::visit(Assign* node) {
#ifdef DEBUG
    std::cout << "Visiting Assign" << std::endl;
#endif

    if (!node) return;

    if (node->left) node->left->accept(*this);
    if (node->exp) node->exp->accept(*this);
}

void AST_Name_Map_Visitor::visit(CallStm* node) {
#ifdef DEBUG
    std::cout << "Visiting CallStm" << std::endl;
#endif
    if (!node) return;

    if (node->obj) node->obj->accept(*this);

    if (node->par) {
        for (auto p : *node->par)
            if (p) p->accept(*this);
    }
}

void AST_Name_Map_Visitor::visit(Continue* node) { }

void AST_Name_Map_Visitor::visit(Break* node) { }

void AST_Name_Map_Visitor::visit(Return* node) {
    if (node->exp) node->exp->accept(*this);
}

void AST_Name_Map_Visitor::visit(PutInt* node) {
    if (node->exp) node->exp->accept(*this);
}

void AST_Name_Map_Visitor::visit(PutCh* node) {
    if (node->exp) node->exp->accept(*this);
}

void AST_Name_Map_Visitor::visit(PutArray* node) {

    if (!node) return;

    if (node->n) node->n->accept(*this);
    if (node->arr) node->arr->accept(*this);
}

void AST_Name_Map_Visitor::visit(Starttime* node) { }

void AST_Name_Map_Visitor::visit(Stoptime* node) { }

void AST_Name_Map_Visitor::visit(BinaryOp* node) {

    if (!node) return;

    if (node->left) node->left->accept(*this);
    if (node->right) node->right->accept(*this);
}

void AST_Name_Map_Visitor::visit(UnaryOp* node) {

    if (!node) return;

    if (node->exp) node->exp->accept(*this);
}

void AST_Name_Map_Visitor::visit(ArrayExp* node) {

    if (!node) return;

    if (node->arr) node->arr->accept(*this);
    if (node->index) node->index->accept(*this);
}

void AST_Name_Map_Visitor::visit(CallExp* node) {

    if (!node) return;

    if (node->obj) node->obj->accept(*this);

    if (node->par) {
        for (auto p : *node->par)
            if (p) p->accept(*this);
    }
}

void AST_Name_Map_Visitor::visit(ClassVar* node) {
    if (!node) return;

    // 先访问 obj（如 b）
    if (node->obj) node->obj->accept(*this);

    // 访问 id（如 a）可以记录到 NameMap（可选）
    if (node->id) node->id->accept(*this);

    // 如果需要，你可以在这里把 class->member 信息记录到 NameMap
    // auto class_name = getTypeName(node->obj);
    // name_maps->addClassVar(class_name, node->id->id, typeKind, arity);
}

void AST_Name_Map_Visitor::visit(This* node) { }

void AST_Name_Map_Visitor::visit(Length* node) {
    if (node->exp) node->exp->accept(*this);
}

void AST_Name_Map_Visitor::visit(NewArray* node) {
    if (node->size) node->size->accept(*this);
}

void AST_Name_Map_Visitor::visit(NewObject* node) { }

void AST_Name_Map_Visitor::visit(GetInt* node) { }

void AST_Name_Map_Visitor::visit(GetCh* node) { }

void AST_Name_Map_Visitor::visit(GetArray* node) {
    if (node->exp) node->exp->accept(*this);
}

void AST_Name_Map_Visitor::visit(IdExp* node) { }

void AST_Name_Map_Visitor::visit(OpExp* node) { }

void AST_Name_Map_Visitor::visit(IntExp* node) { }
