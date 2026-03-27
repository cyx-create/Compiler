#define DEBUG
#undef DEBUG

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <variant>

#include "namemaps.hh"
#include "semant.hh"

using namespace std;
using namespace fdmj;

// TODO

AST_Semant_Map *semant_analyze(Program *node) {
  // std::cout << "TODO" << std::endl;
  // return nullptr;
  std::cerr << "Start Semantic Analysis" << std::endl;
  if (node == nullptr) {
    return nullptr;
  }
  Name_Maps *name_maps = makeNameMaps(node);
  AST_Semant_Visitor semant_visitor(name_maps);
  semant_visitor.visit(node);

  //检查所有错误，同时没有生成AST
    if (semant_visitor.mode == -1) {
        std::cerr << "Semantic Analysis failed due to errors." << std::endl;
        exit(1);
    }

  std::cerr << "Semantic Analysis Done" << std::endl;
  return semant_visitor.getSemantMap();
}

void AST_Semant_Visitor::visit(Program *node) {
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

// rest of the code

//main的return部分是不是也放在Return里面去处理？？？
void AST_Semant_Visitor::visit(MainMethod* node) {
#ifdef DEBUG
    std::cout << "Visiting MainMethod" << std::endl;
#endif
    // 设置正确的上下文
    current_visiting_class = "__$main__";  // main方法属于特殊类
    current_visiting_method = "main";

    // 分析局部变量声明
    if (node->vdl) {
        for (auto v : *node->vdl) {
            if (v) v->accept(*this);
        }
    }

   // 分析语句列表
    bool hasReturn = false;
    if (node->sl) {
        for (auto s : *node->sl) {
            if (!s) continue;
            
            // 访问语句（这会递归调用相应的 visit 方法）
            s->accept(*this);
            
            // 检查是否有 Return 语句
            if (dynamic_cast<Return*>(s)) {
                hasReturn = true;
            }
        }
    }

    // 检查 main 方法必须有 return 语句
    // 不需要
    // if (!hasReturn) {
    //     std::cerr << "Error: main method must have a return statement";
    //     if (node->getPos()) {
    //         std::cerr << " at ";
    //         std::cerr << node->getPos()->print();
    //     }
    //     std::cerr << std::endl;
    //     mode = -1;  // 标记错误
    // }

    // 清理上下文
    current_visiting_method = "";
    current_visiting_class = "";
}

// 新加一个辅助函数
// 检查循环继承
static bool hasCircularInheritance(Name_Maps* maps, const string& className) {
    string current = className;
    set<string> visited;
    
    while (!current.empty()) {
        if (visited.find(current) != visited.end()) {
            return true;
        }
        visited.insert(current);
        current = maps->get_parent(current);
    }
    return false;
}

//暂时copy
void AST_Semant_Visitor::visit(ClassDecl* node) {
#ifdef DEBUG
    std::cout << "Visiting ClassDecl: " << node->id->id << std::endl;
#endif

    if (node == nullptr) return;

    string className = node->id->id;
    
    // ========== 1. 继承关系验证 ==========
    if (node->eid != nullptr) {  // 如果有父类
        string parentName = node->eid->id;
        
        // 1.1 检查父类是否存在
        if (!name_maps->is_class(parentName)) {
            std::cerr << "Error: Parent class " << parentName 
                      << " does not exist";
            if (node->eid->getPos()) {
                std::cerr << " at ";
                std::cerr << node->eid->getPos()->print();
            }
            std::cerr << std::endl;
            mode = -1;
            return;
            // exit(1);
        }
        
        // 1.2 检查类不能继承自身
        if (className == parentName) {
            std::cerr << "Error: Class cannot extend itself";
            if (node->eid->getPos()) {
                std::cerr << " at ";
                std::cerr << node->eid->getPos()->print();
            }
            std::cerr << std::endl;
            mode = -1;
            return;
            // exit(1);
        }
        
        // 1.3 检查单层继承（FDMJ2026限制）
        string grandParent = name_maps->get_parent(parentName);
        if (!grandParent.empty()) {
            std::cerr << "Error: FDMJ2026 only allows single-level inheritance. "
                      << "Class " << className << " cannot extend " << parentName 
                      << " which already extends " << grandParent;
            if (node->eid->getPos()) {
                std::cerr << " at ";
                std::cerr << node->eid->getPos()->print();
            }
            std::cerr << std::endl;
            mode = -1;
            return;
            // exit(1);
        }
        
        // 1.4 检查循环继承（间接）
        if (hasCircularInheritance(name_maps, className)) {
            std::cerr << "Error: Circular inheritance detected in class " 
                      << className;
            if (node->getPos()) {
                std::cerr << " at ";
                std::cerr << node->getPos()->print();
            }
            std::cerr << std::endl;
            mode = -1;
            return;
            // exit(1);
        }
    }
    
    // ========== 2. 设置当前上下文 ==========
    current_visiting_class = className;

    // ========== 3. 分析成员变量 ==========
    if (node->vdl != nullptr) {
        for (auto var : *(node->vdl)) {
            if (var) var->accept(*this);  // 重复检查在 VarDecl 中处理
        }
    }

    if (node->mdl != nullptr) {
        for (auto method : *(node->mdl)) {
            if (method) {
                current_visiting_method = method->id->id;
                method->accept(*this);
            }
        }
    }

    // ========== 5. 清理上下文 ==========
    current_visiting_class = "";
    current_visiting_method = "";  // 确保方法名也被清理
}

// 据说这里要做类型检查？但是现在还没做
void AST_Semant_Visitor::visit(VarDecl *node) {
#ifdef DEBUG
    std::cout << "Visiting VarDecl: " << node->id->id << std::endl;
#endif
    if (node == nullptr) return;
    
    string var_name = node->id->id;
    
    // ========== 1. 检查变量类型是否存在 ==========
    if (node->type != nullptr) {
        // 如果是类类型，检查类是否存在
        if (node->type->typeKind == TypeKind::CLASS) {
            string class_name = node->type->cid ? node->type->cid->id : "";
            if (!name_maps->is_class(class_name)) {
                std::cerr << "Error: Variable " << var_name 
                          << " has undefined class type " << class_name;
                if (node->getPos()) {
                    std::cerr << " at ";
                    std::cerr << node->getPos()->print();
                }
                std::cerr << std::endl;
                mode = -1;
                return;
                // exit(1);
            }
        }
        node->type->accept(*this);
    }
    // ========== 3. 处理初始化表达式 ==========
    if (holds_alternative<IntExp*>(node->init)) {
        IntExp* e = get<IntExp*>(node->init);
        if (e) {
            e->accept(*this);
            
            // 检查初始化类型是否匹配
            if (node->type->typeKind != TypeKind::INT) {
                std::cerr << "Error: Cannot initialize non-int variable with integer";
                if (e->getPos()) {
                    std::cerr << " at ";
                    std::cerr << e->getPos()->print();
                }
                std::cerr << std::endl;
                mode = -1;
                return;
                // exit(1);
            }
        }
    }
    
    if (holds_alternative<vector<IntExp*>*>(node->init)) {
        vector<IntExp*>* arr = get<vector<IntExp*>*>(node->init);
        if (arr) {
            // 检查变量类型是否为数组
            if (node->type->typeKind != TypeKind::ARRAY) {
                std::cerr << "Error: Cannot initialize non-array variable with array initializer";
                if (node->getPos()) {
                    std::cerr << " at ";
                    std::cerr << node->getPos()->print();
                }
                std::cerr << std::endl;
                mode = -1;
                return;
                // exit(1);
            }
            
            for (auto e : *arr) {
                if (e) e->accept(*this);
            }
        }
    }
    
    // ========== 4. 为变量设置语义信息 ==========
    variant<monostate, string, int> type_par;
    if (node->type->typeKind == TypeKind::CLASS) {
        type_par = node->type->cid ? node->type->cid->id : "";
    } else if (node->type->typeKind == TypeKind::ARRAY) {
        // 数组的 arity 从 type 节点获取
        type_par = node->type->arity ? node->type->arity->val : 0;
    } else {
        type_par = monostate{};
    }
    
    AST_Semant *varSem = new AST_Semant(
        AST_Semant::Kind::Value,
        node->type->typeKind,
        type_par,
        true  // 变量是左值
    );
    semant_map->setSemant(node, varSem);
}

void AST_Semant_Visitor::visit(MethodDecl *node) {
#ifdef DEBUG
    std::cout << "Visiting MethodDecl" << std::endl;
#endif
    if (node == nullptr) {
        return;
    }
    
    // 访问返回类型
    if (node->type != nullptr) {
        node->type->accept(*this);
    }
    
    // 保存当前方法名
    string old_method = current_visiting_method;
    current_visiting_method = node->id->id;
    
    // 访问形参
    if (node->fl != nullptr) {
        for (auto formal : *(node->fl)) {
            formal->accept(*this);
        }
    }
    
    // 访问局部变量声明
    if (node->vdl != nullptr) {
        for (auto var : *(node->vdl)) {
            var->accept(*this);
        }
    }
    
    // 访问语句列表
    if (node->sl != nullptr) {
        for (auto stmt : *(node->sl)) {
            stmt->accept(*this);
        }
    }
    
    // ========== 检查是否重写/重载了父类的方法 ==========
    // 获取当前类的所有祖先类
    string current = current_visiting_class;
    set<string> ancestors;
    while (!current.empty()) {
        ancestors.insert(current);
        current = name_maps->get_parent(current);
    }
    
    for (auto ancestor : ancestors) {
        if (ancestor == current_visiting_class) {
            continue;  // 跳过自己
        }
        
        // 如果在祖先类中发现同名方法
        if (name_maps->is_method(ancestor, node->id->id)) {
            // 获取祖先类方法的形参列表 (返回的是 vector<Formal*>*)
            vector<Formal*>* ancestor_formals = 
                name_maps->get_method_formal_list(ancestor, node->id->id);
            
            if (!ancestor_formals) continue;
            
            size_t ancestor_param_count = ancestor_formals->size() - 1;  // 减掉返回类型
            
            // 获取当前方法的形参个数
            size_t current_param_count = node->fl ? node->fl->size() : 0;
            
            // 检查参数个数是否相同
            if (ancestor_param_count != current_param_count) {
                std::cerr << "Error: Method " << node->id->id
                          << " has wrong number of parameters";
                if (node->getPos()) {
                    std::cerr << " at " << node->getPos()->print();
                }
                std::cerr << std::endl;
                mode = -1;
                delete ancestor_formals;
                current_visiting_method = old_method;
                // 下面两位二选一
                return;
                // exit(1);
            }
            
         // 检查每个参数类型是否相同
            bool match = true;
            if (node->fl != nullptr) {
                for (size_t i = 0; i < current_param_count; i++) {
                    // 获取祖先类的形参
                    Formal* ancestor_formal = (*ancestor_formals)[i];
                    
                    // 获取当前方法的形参
                    Formal* current_formal = (*node->fl)[i];
                    
                    // 比较类型
                    if (ancestor_formal->type->typeKind != current_formal->type->typeKind) {
                        match = false;
                        break;
                    }
                    
                    // 根据类型进行详细比较
                    switch (ancestor_formal->type->typeKind) {
                        case TypeKind::CLASS: {
                            string ancestor_class = ancestor_formal->type->cid->id;
                            string current_class = current_formal->type->cid->id;
                            if (ancestor_class != current_class) {
                                match = false;
                                break;
                            }
                            break;
                        }
                        case TypeKind::ARRAY: {
                            // 数组类型直接匹配（FDMJ只有一维数组）
                            // 如果需要检查维度，可以在这里添加
                            break;
                        }
                        case TypeKind::INT:
                        default:
                            // INT 类型已经通过 typeKind 比较
                            break;
                    }
                    
                    if (!match) break;
                }
            }
            
            // 检查返回类型是否匹配
            if (match) {
                // 最后一个元素是返回类型
                Formal* ancestor_return = (*ancestor_formals)[ancestor_formals->size() - 1];
                
                // 比较返回类型
                if (ancestor_return->type->typeKind != node->type->typeKind) {
                    match = false;
                } else {
                    // 根据类型进行详细比较
                    switch (ancestor_return->type->typeKind) {
                        case TypeKind::CLASS: {
                            string ancestor_class = ancestor_return->type->cid->id;
                            string current_class = node->type->cid->id;
                            if (ancestor_class != current_class) {
                                match = false;
                            }
                            break;
                        }
                        case TypeKind::ARRAY: {
                            // 数组类型直接匹配
                            break;
                        }
                        case TypeKind::INT:
                        default:
                            // INT 类型已经通过 typeKind 比较
                            break;
                    }
                }
            }
            
            if (!match) {
                std::cerr << "Error: Method " << node->id->id
                          << " has wrong parameter type or return type and can not be overloaded";
                if (node->getPos()) {
                    std::cerr << " at " << node->getPos()->print();
                }
                std::cerr << std::endl;
                mode = -1;
                delete ancestor_formals;
                current_visiting_method = old_method;
                return;
                // exit(1);
            }
            
            delete ancestor_formals;
        }
    }
    
    // 恢复当前方法名
    current_visiting_method = old_method;
}

void AST_Semant_Visitor::visit(Formal* node) {
#ifdef DEBUG
  std::cout << "Visiting Formal" << std::endl;
#endif
  if (node == nullptr) {
    return;
  }
  if (node->type != nullptr) {
    node->type->accept(*this);
  }
  if (node->id != nullptr) {
    node->id->accept(*this);
  }
}

void AST_Semant_Visitor::visit(Nested* node) {
#ifdef DEBUG
  std::cout << "Visiting Nested" << std::endl;
#endif
  if (node == nullptr) {
    return;
  }
  if (node->sl != nullptr) {
    for (auto stmt : *(node->sl)) {
      stmt->accept(*this);
    }
  }
}

void AST_Semant_Visitor::visit(If *node) {
#ifdef DEBUG
    std::cout << "Visiting If" << std::endl;
#endif
    if (node == nullptr) {
        return;
    }
    
    // ========== 1. 检查条件表达式 ==========
    if (node->exp != nullptr) {
        node->exp->accept(*this);
        
        // 获取条件表达式的语义信息
        AST_Semant* condSem = semant_map->getSemant(node->exp);
        if (condSem) {
            // 条件表达式必须是 int 类型（0 为假，非0 为真）
            if (condSem->get_type() != TypeKind::INT) {
                std::cerr << "Error: If condition must be int expression";
                if (node->exp->getPos()) {
                    std::cerr << " at " << node->exp->getPos()->print();
                }
                std::cerr << std::endl;
                mode = -1;
                return;
                // exit(1);
            }
        } else {
            std::cerr << "Error: No semantic information for if condition";
            if (node->exp->getPos()) {
                std::cerr << " at " << node->exp->getPos()->print();
            }
            std::cerr << std::endl;
            mode = -1;
            return;
            // exit(1);
        }
    } else {
        std::cerr << "Error: If statement missing condition";
        if (node->getPos()) {
            std::cerr << " at " << node->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // ========== 2. 分析 then 分支 ==========
    if (node->stm1 != nullptr) {
        node->stm1->accept(*this);
    } else {
        std::cerr << "Error: If statement missing then branch";
        if (node->getPos()) {
            std::cerr << " at " << node->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // ========== 3. 分析 else 分支（如果存在） ==========
    if (node->stm2 != nullptr) {
        node->stm2->accept(*this);
    }
    
    // ========== 4. If 语句本身不需要语义信息 ==========
    // If 语句不是表达式，没有类型，所以不需要设置语义信息
}

void AST_Semant_Visitor::visit(While *node) {
#ifdef DEBUG
    std::cout << "Visiting While" << std::endl;
#endif
    if (node == nullptr) return;
    
    // ========== 1. 进入循环，增加嵌套计数 ==========
    in_a_while_loop++;
    
    // ========== 2. 检查循环条件 ==========
    if (node->exp != nullptr) {
        node->exp->accept(*this);
        
        // 获取条件表达式的语义信息
        AST_Semant* condSem = semant_map->getSemant(node->exp);
        if (condSem) {
            // 条件表达式必须是 int 类型（0 为假，非0 为真）
            if (condSem->get_type() != TypeKind::INT) {
                std::cerr << "Error: While condition must be int expression";
                if (node->exp->getPos()) {
                    std::cerr << " at " << node->exp->getPos()->print();
                }
                std::cerr << std::endl;
                mode = -1;
                return;
                // exit(1);
            }
        } else {
            std::cerr << "Error: No semantic information for while condition";
            if (node->exp->getPos()) {
                std::cerr << " at " << node->exp->getPos()->print();
            }
            std::cerr << std::endl;
            mode = -1;
            return;
            // exit(1);
        }
    } else {
        std::cerr << "Error: While statement missing condition";
        if (node->getPos()) {
            std::cerr << " at " << node->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        in_a_while_loop--;
        return;
        // exit(1);
    }
    
    // ========== 3. 分析循环体 ==========
    if (node->stm != nullptr) {
        node->stm->accept(*this);
    }
    
    // ========== 4. 退出循环，减少嵌套计数 ==========
    in_a_while_loop--;
}

void AST_Semant_Visitor::visit(Assign *node) {
#ifdef DEBUG
    std::cout << "Visiting Assign" << std::endl;
#endif
    if (node == nullptr) {
        return;
    }
    
    // ========== 1. 分析左右子表达式 ==========
    if (node->left != nullptr) {
        node->left->accept(*this);
    }
    if (node->exp != nullptr) {
        node->exp->accept(*this);
    }
    
    // ========== 2. 获取语义信息 ==========
    AST_Semant *leftSemant = semant_map->getSemant(node->left);
    AST_Semant *expSemant = semant_map->getSemant(node->exp);
    
    // ========== 3. 左值检查 ==========
    if (!leftSemant || !leftSemant->is_lvalue()) {
        std::cerr << "Error: Left-hand side of assignment must be an lvalue";
        if (node->left && node->left->getPos()) {
            std::cerr << " at " << node->left->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // ========== 4. 类型存在性检查 ==========
    if (!leftSemant || !expSemant) {
        std::cerr << "Error: Missing type information in assignment";
        if (node->getPos()) {
            std::cerr << " at " << node->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // ========== 5. 基本类型匹配检查 ==========
    if (leftSemant->get_type() != expSemant->get_type()) {
        std::cerr << "Error: Type mismatch in assignment";
        if (node->left->getPos()) {
            std::cerr << " at " << node->left->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // ========== 6. 类类型的向上转型检查（单层继承）==========
    if (leftSemant->get_type() == TypeKind::CLASS) {
        string left_class = std::get<string>(leftSemant->get_type_par());
        string right_class = std::get<string>(expSemant->get_type_par());
        
        // 情况1：相同类
        if (left_class == right_class) {
            // 允许
        }
        // 情况2：右边是左边的子类（单层继承）
        else if (name_maps->get_parent(right_class) == left_class) {
            // 允许：子类赋给父类
        }
        // 情况3：其他情况都不允许
        else {
            std::cerr << "Error: Type mismatch in assignment: cannot assign "
                      << right_class << " to " << left_class;
            if (node->left->getPos()) {
                std::cerr << " at " << node->left->getPos()->print();
            }
            std::cerr << std::endl;
            mode = -1;
            return;
            // exit(1);
        }
    }
    
    
}

void AST_Semant_Visitor::visit(CallStm *node) {
#ifdef DEBUG
    std::cout << "Visiting CallStm" << std::endl;
#endif
    if (node == nullptr) return;
    
    // 保存上下文
    string old_class = current_visiting_class;
    string old_method = current_visiting_method;
    
    // ========== 1. 分析对象表达式 ==========
    if (node->obj) {
        node->obj->accept(*this);
    }
    
    // ========== 2. 获取对象类型 ==========
    AST_Semant *objSem = semant_map->getSemant(node->obj);
    if (!objSem) {
        std::cerr << "Error: No semantic information for method call object";
        if (node->obj && node->obj->getPos()) {
            std::cerr << " at " << node->obj->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    if (objSem->get_type() != TypeKind::CLASS) {
        std::cerr << "Error: Method call on non-class object";
        if (node->obj && node->obj->getPos()) {
            std::cerr << " at " << node->obj->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    string class_name = std::get<string>(objSem->get_type_par());
    string method_name = node->name->id;
    
    // ========== 3. 在继承链中查找方法 ==========
    bool found = false;
    string defining_class = class_name;
    while (!defining_class.empty()) {
        if (name_maps->is_method(defining_class, method_name)) {
            found = true;
            break;
        }
        defining_class = name_maps->get_parent(defining_class);
    }
    
    if (!found) {
        std::cerr << "Error: Method " << method_name << " not found in class " << class_name;
        if (node->name && node->name->getPos()) {
            std::cerr << " at " << node->name->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // ========== 4. 获取方法的形参列表 ==========
    vector<Formal*>* formals = name_maps->get_method_formal_list(defining_class, method_name);
    if (!formals) {
        std::cerr << "Error: Cannot find formal list for method " << method_name;
        if (node->name && node->name->getPos()) {
            std::cerr << " at " << node->name->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    size_t param_count = formals->size() - 1;  // 减掉返回类型
    size_t arg_count = node->par ? node->par->size() : 0;
    
    // ========== 5. 检查参数个数 ==========
    if (param_count != arg_count) {
        std::cerr << "Error: Method " << method_name << " expects " << param_count 
                  << " arguments but got " << arg_count;
        if (node->name && node->name->getPos()) {
            std::cerr << " at " << node->name->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        delete formals;
        return;
        // exit(1);
    }
    
    // ========== 6. 检查每个参数类型 ==========
    if (node->par) {
        for (size_t i = 0; i < arg_count; i++) {
            // 分析实参表达式
            (*node->par)[i]->accept(*this);
            AST_Semant* argSem = semant_map->getSemant((*node->par)[i]);
            if (!argSem) {
                std::cerr << "Error: No semantic information for argument " << i+1;
                if ((*node->par)[i]->getPos()) {
                    std::cerr << " at " << (*node->par)[i]->getPos()->print();
                }
                std::cerr << std::endl;
                mode = -1;
                delete formals;
                return;
                // exit(1);
            }
            
            // 获取对应的形参
            Formal* formal = (*formals)[i];
            
            // ========== 类型匹配检查（内联） ==========
            bool valid = false;
            
            // 基本类型必须相同
            if (formal->type->typeKind == argSem->get_type()) {
                // int 类型直接匹配
                if (formal->type->typeKind == TypeKind::INT) {
                    valid = true;
                }
                // 数组类型直接匹配
                else if (formal->type->typeKind == TypeKind::ARRAY) {
                    valid = true;
                }
                // 类类型：允许子类对象作为参数
                else if (formal->type->typeKind == TypeKind::CLASS) {
                    string formal_class = formal->type->cid->id;
                    string arg_class = std::get<string>(argSem->get_type_par());
                    
                    // 相同类
                    if (formal_class == arg_class) {
                        valid = true;
                    } else {
                        // 检查 arg_class 是否是 formal_class 的子类（单层继承）
                        string parent = name_maps->get_parent(arg_class);
                        if (parent == formal_class) {
                            valid = true;
                        }
                    }
                }
            }
            
            if (!valid) {
                std::cerr << "Error: Argument " << i+1 << " type mismatch in method " << method_name;
                if ((*node->par)[i]->getPos()) {
                    std::cerr << " at " << (*node->par)[i]->getPos()->print();
                }
                std::cerr << std::endl;
                mode = -1;
                delete formals;
                return;
                // exit(1);
            }
        }
    }
    
    delete formals;
    
    // ========== 7. CallStm 本身不需要语义信息 ==========
    // CallStm 是语句，不是表达式，所以不设置语义信息
    
    // 恢复上下文
    current_visiting_class = old_class;
    current_visiting_method = old_method;
}

void AST_Semant_Visitor::visit(Continue *node) {
#ifdef DEBUG
  std::cout << "Visiting Continue" << std::endl;
#endif
  // cout << "in_while_loops: " << in_while_loops << endl;
  if (in_a_while_loop == 0) {
    cerr << "Error: Continue statement outside of a loop" << endl;
    cerr << "Pos: " << node->getPos()->print() << endl;
    mode = -1;
    return;
    // exit(1);
  }
}

void AST_Semant_Visitor::visit(Break *node) {
#ifdef DEBUG
  std::cout << "Visiting Break" << std::endl;
#endif
  // cout << "in_while_loops: " << in_while_loops << endl;
  if (in_a_while_loop == 0) {
    cerr << "Error: Break statement outside of a loop" << endl;
    cerr << "Pos: " << node->getPos()->print() << endl;
    mode = -1;
    return;
    // exit(1);
  }
}

void AST_Semant_Visitor::visit(Return *node) {
#ifdef DEBUG
    std::cout << "Visiting Return" << std::endl;
#endif
    if (node == nullptr) return;
    
    // 1. 分析返回值表达式
    if (node->exp != nullptr) {
        node->exp->accept(*this);
    }

    // 2. 获取表达式的语义信息
    AST_Semant *expSemant = semant_map->getSemant(node->exp);
    if (!expSemant) {
        std::cerr << "Error: No semantic information for return expression" << std::endl;
        mode = -1;
        return;
        // exit(1);
    }

    // 3. 获取当前方法的期望返回类型
    string ret_name = "_^return^_" + current_visiting_method;
    Formal *ret_formal = name_maps->get_method_formal(
        current_visiting_class, 
        current_visiting_method, 
        ret_name
    );
    
    if (!ret_formal) {
        // 如果是 main 方法，特殊处理
        if (current_visiting_method == "main") {
            // main 方法总是返回 int
            bool match = (expSemant->get_type() == TypeKind::INT);
            if (!match) {
                std::cerr << "Error: main method must return int" << std::endl;
                std::cerr << "Pos: " << node->exp->getPos()->print() << std::endl;
                mode = -1;
                return;
                // exit(1);
            }
            
            // 设置 Return 节点的语义信息
            AST_Semant *returnSemant = new AST_Semant(
                AST_Semant::Kind::Value,
                expSemant->get_type(),
                expSemant->get_type_par(),
                false
            );
            semant_map->setSemant(node, returnSemant);
            return;
        }
        
        std::cerr << "Error: Cannot find return type for method " 
                  << current_visiting_method << std::endl;
        mode = -1;
        return;
        // exit(1);
    }

    // 4. 检查返回类型是否匹配（原有代码继续）
    bool match = false;
    Type *ret_type = ret_formal->type;
    TypeKind return_typeKind = ret_type->typeKind;

    // 4.1 基本类型匹配
    if (expSemant->get_type() == return_typeKind) {
        if (return_typeKind == TypeKind::INT ||
            return_typeKind == TypeKind::ARRAY) {
            match = true;
        }
    }

    // 4.2 类类型向上转型
    if (return_typeKind == TypeKind::CLASS &&
        expSemant->get_type() == TypeKind::CLASS) {
        
        string return_class = ret_type->cid->id;
        string exp_class = std::get<string>(expSemant->get_type_par());
        
        string current = exp_class;
        while (!current.empty()) {
            if (current == return_class) {
                match = true;
                break;
            }
            current = name_maps->get_parent(current);
        }
    }

    if (!match) {
        std::cerr << "Error: Return type mismatch in method " 
                  << current_visiting_method << " at ";
        std::cerr << node->exp->getPos()->print() << std::endl;
        mode = -1;
        return;
        // exit(1);
    }

    // 5. 创建 Return 节点的语义信息
    AST_Semant *returnSemant = new AST_Semant(
        AST_Semant::Kind::Value,
        expSemant->get_type(),
        expSemant->get_type_par(),
        false
    );

    semant_map->setSemant(node, returnSemant);
}

void AST_Semant_Visitor::visit(PutInt *node) {
#ifdef DEBUG
  std::cout << "Visiting PutInt" << std::endl;
#endif
  if (node == nullptr) {
    return;
  }
  if (node->exp != nullptr) {
    node->exp->accept(*this);
  }

  // CHECK: if expression is an int
  AST_Semant *expSemant = semant_map->getSemant(node->exp);
  if (!expSemant || expSemant->get_type() != TypeKind::INT) {
    cerr << "Error: PutInt expression must be an int" << endl;
    cerr << "Pos: " << node->exp->getPos()->print() << endl;
    mode = -1;
    return;
    // exit(1);
  }
}

void AST_Semant_Visitor::visit(PutCh *node) {
#ifdef DEBUG
  std::cout << "Visiting PutCh" << std::endl;
#endif
  if (node == nullptr) {
    return;
  }
  if (node->exp != nullptr) {
    node->exp->accept(*this);
  }

  // CHECK: if expression is an int
  AST_Semant *expSemant = semant_map->getSemant(node->exp);
  if (!expSemant || expSemant->get_type() != TypeKind::INT) {
    cerr << "Error: PutCh expression must be an int" << endl;
    cerr << "Pos: " << node->exp->getPos()->print() << endl;
    mode = -1;
    return;
    // exit(1);
  }
}

void AST_Semant_Visitor::visit(PutArray *node) {
#ifdef DEBUG
  std::cout << "Visiting PutArray" << std::endl;
#endif
  if (node == nullptr) {
    return;
  }
  if (node->n != nullptr) {
    node->n->accept(*this);
  }
  if (node->arr != nullptr) {
    node->arr->accept(*this);
  }

  // CHECK: if n is an int and arr is an array
  AST_Semant *nSemant = semant_map->getSemant(node->n);
  AST_Semant *arrSemant = semant_map->getSemant(node->arr);

  if (!nSemant || nSemant->get_type() != TypeKind::INT) {
    cerr << "Error: PutArray n must be an int" << endl;
    cerr << "Pos: " << node->n->getPos()->print() << endl;
    mode = -1;
    return;
    // exit(1);
  }

  if (!arrSemant || arrSemant->get_type() != TypeKind::ARRAY) {
    cerr << "Error: PutArray arr must be an array" << endl;
    cerr << "Pos: " << node->arr->getPos()->print() << endl;
    mode = -1;
    return;
    // exit(1);
  }
}

void AST_Semant_Visitor::visit(Starttime* node) {}
void AST_Semant_Visitor::visit(Stoptime* node) {}

void AST_Semant_Visitor::visit(BinaryOp* node) {

    node->left->accept(*this);

    node->right->accept(*this);

    semant_map->setSemant(
        node,
        new AST_Semant(
            AST_Semant::Value,
            TypeKind::INT,
            monostate{},
            false
        )
    );
}


void AST_Semant_Visitor::visit(UnaryOp* node) {

    node->exp->accept(*this);

    semant_map->setSemant(
        node,
        new AST_Semant(
            AST_Semant::Value,
            TypeKind::INT,
            monostate{},
            false
        )
    );
}


void AST_Semant_Visitor::visit(ArrayExp *node) {
#ifdef DEBUG
    std::cout << "Visiting ArrayExp" << std::endl;
#endif
    if (node == nullptr) return;
    
    // 1. 分析子表达式
    if (node->arr) node->arr->accept(*this);
    if (node->index) node->index->accept(*this);
    
    // 2. 获取语义信息
    AST_Semant *arrSem = semant_map->getSemant(node->arr);
    AST_Semant *idxSem = semant_map->getSemant(node->index);

    // 3. 检查数组类型
    if (!arrSem) {
        std::cerr << "Error: No semantic information for array expression";
        if (node->arr && node->arr->getPos()) {
            std::cerr << " at " << node->arr->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }

    if (arrSem->get_type() != TypeKind::ARRAY) {
        std::cerr << "Error: Array access on non-array type";
        if (node->arr && node->arr->getPos()) {
            std::cerr << " at " << node->arr->getPos()->print();
        }
        // 打印实际类型
        if (arrSem->get_type() == TypeKind::INT)
            std::cerr << " (got int)";
        else if (arrSem->get_type() == TypeKind::CLASS)
            std::cerr << " (got class " << std::get<string>(arrSem->get_type_par()) << ")";
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // 4. 检查索引类型
    if (!idxSem) {
        std::cerr << "Error: No semantic information for array index";
        if (node->index && node->index->getPos()) {
            std::cerr << " at " << node->index->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    if (idxSem->get_type() != TypeKind::INT) {
        std::cerr << "Error: Array index must be int";
        if (node->index && node->index->getPos()) {
            std::cerr << " at " << node->index->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    // 6. 检查常量索引是否为负数（警告）
    if (IntExp* intIdx = dynamic_cast<IntExp*>(node->index)) {
        if (intIdx->val < 0) {
            std::cerr << "Warning: Array index is negative (will cause runtime error)";
            if (node->index->getPos()) {
                std::cerr << " at " << node->index->getPos()->print();
            }
            std::cerr << std::endl;
            // 不设置 mode = -1，因为这是运行时错误
        }
    }
    
    // 7. 设置数组元素的语义信息
    // 注意：在FDMJ中，数组元素总是int类型
    AST_Semant *elemSem = new AST_Semant(
        AST_Semant::Kind::Value,
        TypeKind::INT,
        monostate{},
        true  // 数组元素是左值
    );
    
    semant_map->setSemant(node, elemSem);
}


void AST_Semant_Visitor::visit(CallExp *node) {
#ifdef DEBUG
    std::cout << "Visiting CallExp" << std::endl;
#endif
    if (node == nullptr) return;
    
    // 保存上下文
    string old_class = current_visiting_class;
    string old_method = current_visiting_method;
    
    // ========== 1. 先分析对象表达式 ==========
    if (node->obj) {
        node->obj->accept(*this);
    } else {
        std::cerr << "Error: Method call on null object";
        if (node->getPos()) {
            std::cerr << " at " << node->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // ========== 2. 获取对象类型 ==========
    AST_Semant *objSem = semant_map->getSemant(node->obj);
    if (!objSem) {
        std::cerr << "Error: No semantic information for method call object";
        if (node->obj->getPos()) {
            std::cerr << " at " << node->obj->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    if (objSem->get_type() != TypeKind::CLASS) {
        std::cerr << "Error: Method call on non-class object";
        if (node->obj->getPos()) {
            std::cerr << " at " << node->obj->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // 获取类名
    string class_name;
    try {
        class_name = std::get<string>(objSem->get_type_par());
    } catch (const std::bad_variant_access &) {
        std::cerr << "Error: Invalid class name in object";
        if (node->obj->getPos()) {
            std::cerr << " at " << node->obj->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    string method_name = node->name->id;
    
    // ========== 3. 在继承链中查找方法 ==========
    bool found = false;
    string defining_class = class_name;
    while (!defining_class.empty()) {
        if (name_maps->is_method(defining_class, method_name)) {
            found = true;
            break;
        }
        defining_class = name_maps->get_parent(defining_class);
    }
    
    if (!found) {
        std::cerr << "Error: Method " << method_name << " not found in class " << class_name;
        if (node->name && node->name->getPos()) {
            std::cerr << " at " << node->name->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // ========== 4. 获取方法的形参列表 ==========
    vector<Formal*>* formals = name_maps->get_method_formal_list(defining_class, method_name);
    if (!formals) {
        std::cerr << "Error: Cannot find formal list for method " << method_name;
        if (node->name && node->name->getPos()) {
            std::cerr << " at " << node->name->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    size_t param_count = formals->size() - 1;  // 减掉返回类型
    size_t arg_count = node->par ? node->par->size() : 0;
    
    // ========== 5. 检查参数个数 ==========
    if (param_count != arg_count) {
        std::cerr << "Error: Method " << method_name << " expects " << param_count 
                  << " arguments but got " << arg_count;
        if (node->name && node->name->getPos()) {
            std::cerr << " at " << node->name->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        delete formals;
        return;
        // exit(1);
    }
    
    // ========== 6. 分析参数表达式（现在再分析参数）==========
    if (node->par) {
        for (auto exp : *(node->par)) {
            if (exp) exp->accept(*this);
        }
    }
    
    // ========== 7. 检查每个参数类型 ==========
    if (node->par) {
        for (size_t i = 0; i < arg_count; i++) {
            AST_Semant* argSem = semant_map->getSemant((*node->par)[i]);
            if (!argSem) {
                std::cerr << "Error: No semantic information for argument " << i+1;
                if ((*node->par)[i]->getPos()) {
                    std::cerr << " at " << (*node->par)[i]->getPos()->print();
                }
                std::cerr << std::endl;
                mode = -1;
                delete formals;
                return;
                // exit(1);
            }
            
            Formal* formal = (*formals)[i];
            bool valid = false;
            
            // 类型匹配检查
            if (formal->type->typeKind == argSem->get_type()) {
                if (formal->type->typeKind == TypeKind::INT) {
                    valid = true;
                }
                // 数组类型直接匹配
                else if (formal->type->typeKind == TypeKind::ARRAY) {
                    valid = true;
                }
                else if (formal->type->typeKind == TypeKind::CLASS) {
                    string formal_class = formal->type->cid->id;
                    string arg_class = std::get<string>(argSem->get_type_par());
                    
                    // 相同类
                    if (formal_class == arg_class) {
                        valid = true;
                    } else {
                        // 检查 arg_class 是否是 formal_class 的子类（单层继承）
                        string parent = name_maps->get_parent(arg_class);
                        if (parent == formal_class) {
                            valid = true;
                        }
                    }
                }
            }
            
            if (!valid) {
                std::cerr << "Error: Argument " << i+1 << " type mismatch in method " << method_name;
                if ((*node->par)[i]->getPos()) {
                    std::cerr << " at " << (*node->par)[i]->getPos()->print();
                }
                std::cerr << std::endl;
                mode = -1;
                delete formals;
                return;
                // exit(1);
            }
        }
    }
    
    // ========== 8. 获取返回类型 ==========
    string ret_name = "_^return^_" + method_name;
    Formal* ret_formal = name_maps->get_method_formal(defining_class, method_name, ret_name);
    if (!ret_formal) {
        std::cerr << "Error: Method " << method_name << " has no return type";
        if (node->name && node->name->getPos()) {
            std::cerr << " at " << node->name->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        delete formals;
        return;
        // exit(1);
    }
    
    // ========== 9. 设置 CallExp 的语义信息 ==========
    Type* ret_type = ret_formal->type;
    variant<monostate, string, int> type_par;
    
    if (ret_type->typeKind == TypeKind::CLASS) {
        type_par = ret_type->cid->id;
    } else if (ret_type->typeKind == TypeKind::ARRAY) {
        // type_par = ret_type->arity ? ret_type->arity->val : 0;
        type_par = 0;
    } else {
        type_par = monostate{};
    }
    
    AST_Semant *callSem = new AST_Semant(
        AST_Semant::Kind::Value,
        ret_type->typeKind,
        type_par,
        false  // 方法返回值不是左值
    );
    semant_map->setSemant(node, callSem);
    
    delete formals;
    
    // 恢复上下文
    current_visiting_class = old_class;
    current_visiting_method = old_method;
}

void AST_Semant_Visitor::visit(ClassVar *node) {
#ifdef DEBUG
    std::cout << "Visiting ClassVar" << std::endl;
#endif
    if (node == nullptr) return;
    
    // ========== 1. 分析对象表达式 ==========
    if (node->obj) {
        node->obj->accept(*this);
    }
    
    // ========== 2. 获取对象类型 ==========
    AST_Semant *objSem = semant_map->getSemant(node->obj);
    if (!objSem) {
        std::cerr << "Error: No semantic information for object in ClassVar";
        if (node->obj && node->obj->getPos()) {
            std::cerr << " at " << node->obj->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // 检查对象是否为类类型
    if (objSem->get_type() != TypeKind::CLASS) {
        std::cerr << "Error: ClassVar object must be a class";
        if (node->obj && node->obj->getPos()) {
            std::cerr << " at " << node->obj->getPos()->print();
        }
        // 打印实际类型
        if (objSem->get_type() == TypeKind::INT)
            std::cerr << " (got int)";
        else if (objSem->get_type() == TypeKind::ARRAY)
            std::cerr << " (got array)";
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // 获取对象的类名
    string class_name;
    try {
        class_name = std::get<string>(objSem->get_type_par());
    } catch (const std::bad_variant_access &) {
        std::cerr << "Error: Invalid class name in object";
        if (node->obj && node->obj->getPos()) {
            std::cerr << " at " << node->obj->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    string field_name = node->id->id;
    
    // ========== 3. 在继承链中查找字段 ==========
    VarDecl *fieldDecl = nullptr;
    string defining_class = class_name;
    
    while (!defining_class.empty()) {
        if (name_maps->is_class_var(defining_class, field_name)) {
            fieldDecl = name_maps->get_class_var(defining_class, field_name);
            break;
        }
        defining_class = name_maps->get_parent(defining_class);
    }
    
    if (!fieldDecl) {
        std::cerr << "Error: Field " << field_name << " not found in class " << class_name;
        if (node->id && node->id->getPos()) {
            std::cerr << " at " << node->id->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // ========== 4. 获取字段的类型信息 ==========
    Type *type = fieldDecl->type;
    TypeKind typeKind = type->typeKind;
    variant<monostate, string, int> type_par;
    
    if (typeKind == TypeKind::CLASS) {
        type_par = type->cid ? type->cid->id : "";
    } else if (typeKind == TypeKind::ARRAY) {
        // 一维数组，arity 设为 0
        // type_par = 0;
        type_par = type->arity ? type->arity->val : 0;
    } else {
        type_par = monostate{};
    }
    
    // ========== 5. 设置语义信息 ==========
    AST_Semant *fieldSem = new AST_Semant(
        AST_Semant::Kind::Value,
        typeKind,
        type_par,
        true  // 字段是左值
    );
    
    semant_map->setSemant(node, fieldSem);
    
    // 同时也为 id 设置语义信息
    if (node->id) {
        AST_Semant *idSem = new AST_Semant(
            AST_Semant::Kind::Value,
            typeKind,
            type_par,
            true  // 字段名作为左值
        );
        semant_map->setSemant(node->id, idSem);
    }
}


void AST_Semant_Visitor::visit(This *node) {
#ifdef DEBUG
    std::cout << "Visiting This" << std::endl;
#endif
    if (node == nullptr) return;
    
    // 检查 this 的使用上下文
    // if (current_visiting_class.empty() || current_visiting_method.empty()) {
    //     std::cerr << "Error: 'this' used outside of class method";
    //     if (node->getPos()) {
    //         std::cerr << " at " << node->getPos()->print();
    //     }
    //     std::cerr << std::endl;
    //     mode = -1;
    //     return;
    // }
    
    // this 的类型是当前类，不是左值
    AST_Semant *thisSem = new AST_Semant(
        AST_Semant::Kind::Value,
        TypeKind::CLASS,
        current_visiting_class,
        false
    );
    
    semant_map->setSemant(node, thisSem);
}

void AST_Semant_Visitor::visit(Length *node) {
#ifdef DEBUG
    std::cout << "Visiting Length" << std::endl;
#endif
    if (node == nullptr) return;
    
    // 分析数组表达式
    if (node->exp) {
        node->exp->accept(*this);
    }
    
    // 获取数组表达式的语义信息
    AST_Semant *arrSem = semant_map->getSemant(node->exp);
    if (!arrSem) {
        std::cerr << "Error: No semantic information for length expression";
        if (node->exp && node->exp->getPos()) {
            std::cerr << " at " << node->exp->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // 检查是否为数组类型
    if (arrSem->get_type() != TypeKind::ARRAY) {
        std::cerr << "Error: length() can only be applied to arrays";
        if (node->exp && node->exp->getPos()) {
            std::cerr << " at " << node->exp->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // length() 返回 int
    AST_Semant *lenSem = new AST_Semant(
        AST_Semant::Kind::Value,
        TypeKind::INT,
        monostate{},
        false
    );
    
    semant_map->setSemant(node, lenSem);
}


void AST_Semant_Visitor::visit(NewArray *node) {
#ifdef DEBUG
    std::cout << "Visiting NewArray" << std::endl;
#endif
    if (node == nullptr) return;
    
    // 分析大小表达式
    if (node->size) {
        node->size->accept(*this);
    }
    
    // 获取大小表达式的语义信息
    AST_Semant *sizeSem = semant_map->getSemant(node->size);
    if (!sizeSem) {
        std::cerr << "Error: No semantic information for array size";
        if (node->size && node->size->getPos()) {
            std::cerr << " at " << node->size->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // 检查大小是否为 int
    if (sizeSem->get_type() != TypeKind::INT) {
        std::cerr << "Error: Array size must be int";
        if (node->size && node->size->getPos()) {
            std::cerr << " at " << node->size->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // new int[size] 返回 int[] 类型
    AST_Semant *newArrSem = new AST_Semant(
        AST_Semant::Kind::Value,
        TypeKind::ARRAY,
        0,  // 一维数组
        false
    );
    
    semant_map->setSemant(node, newArrSem);
}

void AST_Semant_Visitor::visit(NewObject *node) {
#ifdef DEBUG
    std::cout << "Visiting NewObject" << std::endl;
#endif
    if (node == nullptr) return;
    
    // 获取类名
    if (!node->id) {
        std::cerr << "Error: new object missing class name";
        if (node->getPos()) {
            std::cerr << " at " << node->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    string class_name = node->id->id;
    
    // 检查类是否存在
    if (!name_maps->is_class(class_name)) {
        std::cerr << "Error: Unknown class '" << class_name << "'";
        if (node->id->getPos()) {
            std::cerr << " at " << node->id->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // new ClassName() 返回 class 类型
    AST_Semant *newObjSem = new AST_Semant(
        AST_Semant::Kind::Value,
        TypeKind::CLASS,
        class_name,
        false
    );
    
    semant_map->setSemant(node, newObjSem);
}


void AST_Semant_Visitor::visit(GetInt* node) {

    semant_map->setSemant(
        node,
        new AST_Semant(
            AST_Semant::Value,
            TypeKind::INT,
            monostate{},
            false
        )
    );

    // AST_Semant *getIntSemant = new AST_Semant(AST_Semant::Kind::Value,
    //                                         TypeKind::INT, monostate{}, false);
    // semant_map->setSemant(node, getIntSemant);
}


void AST_Semant_Visitor::visit(GetCh* node) {

    semant_map->setSemant(
        node,
        new AST_Semant(
            AST_Semant::Value,
            TypeKind::INT,
            monostate{},
            false
        )
    );
}


void AST_Semant_Visitor::visit(GetArray *node) {
#ifdef DEBUG
    std::cout << "Visiting GetArray" << std::endl;
#endif
    if (node == nullptr) return;
    
    // 分析数组表达式
    if (node->exp) {
        node->exp->accept(*this);
    }
    
    // 获取数组表达式的语义信息
    AST_Semant *arrSem = semant_map->getSemant(node->exp);
    if (!arrSem) {
        std::cerr << "Error: No semantic information for getarray argument";
        if (node->exp && node->exp->getPos()) {
            std::cerr << " at " << node->exp->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // 检查是否为数组类型
    if (arrSem->get_type() != TypeKind::ARRAY) {
        std::cerr << "Error: getarray() argument must be an array";
        if (node->exp && node->exp->getPos()) {
            std::cerr << " at " << node->exp->getPos()->print();
        }
        std::cerr << std::endl;
        mode = -1;
        return;
        // exit(1);
    }
    
    // GetArray 语句本身不需要设置语义信息
}


void AST_Semant_Visitor::visit(IdExp *node) {
#ifdef DEBUG
    std::cout << "Visiting IdExp: " << node->id << std::endl;
#endif
    if (node == nullptr) return;
    
    string id = node->id;
     
    // ========== 1. 查找局部变量 ==========
    if (!current_visiting_method.empty()) {
        if (name_maps->is_method_var(current_visiting_class, 
                                     current_visiting_method, 
                                     id)) {
            VarDecl* vd = name_maps->get_method_var(current_visiting_class,
                                                    current_visiting_method,
                                                    id);
            if (vd && vd->type) {
                variant<monostate, string, int> type_par;
                if (vd->type->typeKind == TypeKind::CLASS) {
                    type_par = vd->type->cid ? vd->type->cid->id : "";
                } else if (vd->type->typeKind == TypeKind::ARRAY) {
                    // type_par = 0;
                    type_par = vd->type->arity ? vd->type->arity->val : 0;
                } else {
                    type_par = monostate{};
                }
                
                AST_Semant *sem = new AST_Semant(
                    AST_Semant::Kind::Value,
                    vd->type->typeKind,
                    type_par,
                    true
                );
                semant_map->setSemant(node, sem);
                return;
            }
        }
    }
    
    // ========== 2. 查找形参 ==========
    if (!current_visiting_method.empty()) {
        if (name_maps->is_method_formal(current_visiting_class,
                                        current_visiting_method,
                                        id)) {
            Formal* f = name_maps->get_method_formal(current_visiting_class,
                                                      current_visiting_method,
                                                      id);
            if (f && f->type) {
                variant<monostate, string, int> type_par;
                if (f->type->typeKind == TypeKind::CLASS) {
                    type_par = f->type->cid ? f->type->cid->id : "";
                } else if (f->type->typeKind == TypeKind::ARRAY) {
                    type_par = 0;
                } else {
                    type_par = monostate{};
                }
                
                AST_Semant *sem = new AST_Semant(
                    AST_Semant::Kind::Value,
                    f->type->typeKind,
                    type_par,
                    true
                );
                semant_map->setSemant(node, sem);
                return;
            }
        }
    }
    
    // ========== 3. 查找类变量（当前类）- 但跳过 __$main__ ==========
    if (!current_visiting_class.empty() && current_visiting_class != "__$main__") {
        if (name_maps->is_class_var(current_visiting_class, id)) {
            VarDecl* vd = name_maps->get_class_var(current_visiting_class, id);
            if (vd && vd->type) {
                variant<monostate, string, int> type_par;
                if (vd->type->typeKind == TypeKind::CLASS) {
                    type_par = vd->type->cid ? vd->type->cid->id : "";
                } else if (vd->type->typeKind == TypeKind::ARRAY) {
                    type_par = 0;
                } else {
                    type_par = monostate{};
                }
                
                AST_Semant *sem = new AST_Semant(
                    AST_Semant::Kind::Value,
                    vd->type->typeKind,
                    type_par,
                    true
                );
                semant_map->setSemant(node, sem);
                return;
            }
        }
    }
    
    // ========== 4. 查找父类的类变量 ==========
    if (!current_visiting_class.empty() && current_visiting_class != "__$main__") {
        string parent = name_maps->get_parent(current_visiting_class);
        while (!parent.empty()) {
            if (name_maps->is_class_var(parent, id)) {
                VarDecl* vd = name_maps->get_class_var(parent, id);
                if (vd && vd->type) {
                    variant<monostate, string, int> type_par;
                    if (vd->type->typeKind == TypeKind::CLASS) {
                        type_par = vd->type->cid ? vd->type->cid->id : "";
                    } else if (vd->type->typeKind == TypeKind::ARRAY) {
                        type_par = 0;
                    } else {
                        type_par = monostate{};
                    }
                    
                    AST_Semant *sem = new AST_Semant(
                        AST_Semant::Kind::Value,
                        vd->type->typeKind,
                        type_par,
                        true
                    );
                    semant_map->setSemant(node, sem);
                    return;
                }
            }
            parent = name_maps->get_parent(parent);
        }
    }
    
    // ========== 5. 未找到，但不报错,只是返回一个默认的语义信息
    
    // 创建一个默认的语义信息
    AST_Semant *defaultSem = new AST_Semant(
        AST_Semant::Kind::Value,
        TypeKind::INT,
        monostate{},
        true
    );
    semant_map->setSemant(node, defaultSem);
}


void AST_Semant_Visitor::visit(OpExp* node) {}


void AST_Semant_Visitor::visit(IntExp* node) {

    semant_map->setSemant(
        node,
        new AST_Semant(
            AST_Semant::Value,
            TypeKind::INT,
            monostate{},
            false
        )
    );
}

void AST_Semant_Visitor::visit(Type* node) {
    if (node == nullptr) return;

    // 根据 typeKind 来判断类型
    switch (node->typeKind) {
        case TypeKind::INT: {
            semant_map->setSemant(node, new AST_Semant(
                AST_Semant::Value,
                TypeKind::INT,
                monostate{},
                false
            ));
            break;
        }
        case TypeKind::ARRAY: {
            int arity = node->arity ? node->arity->val : 0; // 假设 IntExp 有 value
            semant_map->setSemant(node, new AST_Semant(
                AST_Semant::Value,
                TypeKind::ARRAY,
                arity,
                false
            ));

            // 如果数组有 arity 节点，也访问一下
            if (node->arity) node->arity->accept(*this);
            break;
        }
        case TypeKind::CLASS: {
            std::string class_name = node->cid ? node->cid->id : "";
            semant_map->setSemant(node, new AST_Semant(
                AST_Semant::Value,
                TypeKind::CLASS,
                class_name,
                false
            ));

            // 访问类名 IdExp 节点
            if (node->cid) node->cid->accept(*this);
            break;
        }
        default: {
            cerr << "Error: unknown type in AST_Semant_Visitor::visit(Type)" << endl;
            mode = -1;
            break;
            // exit(1);
        }
    }
}