#define DEBUG
#undef DEBUG

#include "executor.hh"
#include <iostream>

using namespace std;
using namespace fdmj;

// ======== 表达式求值辅助函数 ========
static int evalBinaryOp(const string &op, int l, int r) {
    if (op == "+") return l + r;
    if (op == "-") return l - r;
    if (op == "*") return l * r;
    if (op == "/") return l / r;
    if (op == "%") return l % r;

    if (op == "==") return l == r;
    if (op == "!=") return l != r;
    if (op == "<")  return l < r;
    if (op == "<=") return l <= r;
    if (op == ">")  return l > r;
    if (op == ">=") return l >= r;

    cerr << "Unknown binary operator: " << op << endl;
    exit(1);
}

static int evalUnaryOp(const string &op, int val) {
    if (op == "-") return -val;
    if (op == "!") return !val;
    cerr << "Unknown unary operator: " << op << endl;
    exit(1);
}

// ======== Visitor实现 ========

void Executor::visit(Program *node) {
    if (!node || !node->main) return;
    node->main->accept(*this);
}

void Executor::visit(MainMethod *node) {
    if (!node || !node->sl) return;
    for (Stm *stm : *(node->sl)) {
        stm->accept(*this);
        // 如果 retVal 被设置了，就直接停止执行
        if (retVal != 0) break;
    }
}

void Executor::visit(Assign *node) {
    if (!node || !node->left || !node->exp) return;

    // 左边必须是 IdExp
    if (node->left->getASTKind() != ASTKind::IdExp) {
        cerr << "Left side of assignment must be identifier." << endl;
        exit(1);
    }

    node->exp->accept(*this);          // 先计算右值
    int val = retVal;                  // 当前计算结果
    IdExp *id = static_cast<IdExp*>(node->left);

    env[id->id] = val;                 // 更新环境
    retVal = 0;                        // 清空临时 retVal
}

void Executor::visit(Return *node) {
    if (!node || !node->exp) return;

    node->exp->accept(*this);
    // 执行 return
    // retVal 已经在 visit(IntExp/IdExp/BinaryOp/UnaryOp)里被设置
    // 保留当前值
}

void Executor::visit(IntExp *node) {
    if (!node) return;
    retVal = node->val;
}

void Executor::visit(IdExp *node) {
    if (!node) return;
    auto it = env.find(node->id);
    if (it == env.end()) {
        cerr << "Warning: variable '" << node->id
             << "' used before definition at line "
             << node->getPos()->sline << ", column "
             << node->getPos()->scolumn << endl;
        env[node->id] = 0;
        retVal = 0;
        return;
    }
    retVal = it->second;
}

void Executor::visit(BinaryOp *node) {
    if (!node || !node->left || !node->right || !node->op) return;

    node->left->accept(*this);
    int l = retVal;

    node->right->accept(*this);
    int r = retVal;

    retVal = evalBinaryOp(node->op->op, l, r);
}

void Executor::visit(UnaryOp *node) {
    if (!node || !node->exp || !node->op) return;

    node->exp->accept(*this);
    int val = retVal;

    retVal = evalUnaryOp(node->op->op, val);
}

void Executor::visit(OpExp *node) {
    // OpExp本身不需要求值
    retVal = 0;
}

int execute(Program *root) {
    Executor v;
    root->accept(v);
    return v.retVal;
}