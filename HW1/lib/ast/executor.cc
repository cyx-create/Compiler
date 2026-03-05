#define DEBUG
#undef DEBUG

#include "executor.hh"
#include <iostream>

using namespace std;
using namespace fdmj;


// 表达式求值
static int evalExp(Exp* exp, map<string,int>& env) {

  switch (exp->getASTKind()) {

    case ASTKind::IntExp: {
      IntExp* i = static_cast<IntExp*>(exp);
      return i->val;
    }

case ASTKind::IdExp: {
    IdExp* id = static_cast<IdExp*>(exp);

    // 检查变量是否在 env 中
    auto it = env.find(id->id);
    if (it == env.end()) {
        // 未定义变量：假设为0，同时打印错误信息到 stderr
        cerr << "Warning: variable '" << id->id
             << "' used before definition at line "
             << id->getPos()->eline << ", column "
             << id->getPos()->ecolumn << endl;
        env[id->id] = 0;  // 假设值为0
        return 0;
    }
    return it->second;
}

    case ASTKind::BinaryOp: {
      BinaryOp* b = static_cast<BinaryOp*>(exp);

      int l = evalExp(b->left, env);
      int r = evalExp(b->right, env);

      string op = b->op->op;

      if (op == "+") return l + r;
      if (op == "-") return l - r;
      if (op == "*") return l * r;
      if (op == "/") return l / r;
      if (op == "%") return l % r;

      if (op == "==") return l == r;
      if (op == "!=") return l != r;
      if (op == "<") return l < r;
      if (op == "<=") return l <= r;
      if (op == ">") return l > r;
      if (op == ">=") return l >= r;

      cout << "Unknown binary operator: " << op << endl;
      exit(1);
    }

    case ASTKind::UnaryOp: {
      UnaryOp* u = static_cast<UnaryOp*>(exp);
      int val = evalExp(u->exp, env);
      string op = u->op->op;

      if (op == "-") return -val;
      if (op == "!") return !val;

      cout << "Unknown unary operator: " << op << endl;
      exit(1);
    }

    default:
      cout << "Unsupported expression type" << endl;
      exit(1);
  }

  return 0;
}

// 语句执行
static bool execStmt(Stm* stm,
                     map<string,int>& env,
                     int& retVal) {

  switch (stm->getASTKind()) {

    case ASTKind::Assign: {
      Assign* a = static_cast<Assign*>(stm);

      // 左边必须是 IdExp
      if (a->left->getASTKind() != ASTKind::IdExp) {
        cout << "Left side of assignment must be identifier." << endl;
        exit(1);
      }

      IdExp* id = static_cast<IdExp*>(a->left);
      int val = evalExp(a->exp, env);
      env[id->id] = val;

      return false;
    }

    case ASTKind::Return: {
      Return* r = static_cast<Return*>(stm);
      retVal = evalExp(r->exp, env);
      return true;  // 遇到 return
    }

    default:
      cout << "Unsupported statement type" << endl;
      exit(1);
  }

  return false;
}

// 主执行入口
int execute(Program *root) {
  cout << "TODO" << endl;
  // return 0;

  map<string,int> env;
  int retVal = 0;

  MainMethod* mainMethod = root->main;

  for (Stm* stm : *(mainMethod->sl)) {
    if (execStmt(stm, env, retVal)) {
      return retVal;
    }
  }

  return retVal;
}
