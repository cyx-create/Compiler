#define DEBUG
#undef DEBUG

#include "constantPropagation.hh"
#include "MinusIntConverter.hh"

using namespace std;
using namespace fdmj;

// 表达式递归变换（clone + 常量折叠）
static Exp* transformExp(Exp* exp) {

  // 如果是 BinaryOp
  if (exp->getASTKind() == ASTKind::BinaryOp) {

    BinaryOp* b = static_cast<BinaryOp*>(exp);

    // 递归处理左右
    Exp* newLeft  = transformExp(b->left);
    Exp* newRight = transformExp(b->right);

    // 如果左右都是常量 → 折叠
    if (newLeft->getASTKind() == ASTKind::IntExp &&
        newRight->getASTKind() == ASTKind::IntExp) {

      int lval = static_cast<IntExp*>(newLeft)->val;
      int rval = static_cast<IntExp*>(newRight)->val;
      int result = 0;

      string op = b->op->op;

      if (op == "+") result = lval + rval;
      else if (op == "-") result = lval - rval;
      else if (op == "*") result = lval * rval;
      else if (op == "/") result = lval / rval;

      // 不访问 pos，使用 clone 的位置信息
      IntExp* folded = static_cast<IntExp*>(newLeft->clone());
      folded->val = result;
      return folded;
    }

    // 否则 clone 当前节点，再替换子树
    BinaryOp* newNode = static_cast<BinaryOp*>(b->clone());
    newNode->left  = newLeft;
    newNode->right = newRight;
    return newNode;
  }

  // 如果是 UnaryOp
  if (exp->getASTKind() == ASTKind::UnaryOp) {
    UnaryOp* u = static_cast<UnaryOp*>(exp);

    UnaryOp* newNode = static_cast<UnaryOp*>(u->clone());
    newNode->exp = transformExp(u->exp);
    return newNode;
  }

  // 其他类型（IntExp, IdExp, OpExp）
  return static_cast<Exp*>(exp->clone());
}

// 语句递归变换
static Stm* transformStm(Stm* stm) {

  if (stm->getASTKind() == ASTKind::Assign) {

    Assign* a = static_cast<Assign*>(stm);

    Assign* newNode = static_cast<Assign*>(a->clone());
    newNode->left = transformExp(a->left);
    newNode->exp  = transformExp(a->exp);

    return newNode;
  }

  if (stm->getASTKind() == ASTKind::Return) {

    Return* r = static_cast<Return*>(stm);

    Return* newNode = static_cast<Return*>(r->clone());
    newNode->exp = transformExp(r->exp);

    return newNode;
  }

  return static_cast<Stm*>(stm->clone());
}

// 程序递归变换
static Program* transformProgram(Program* root) {

  Program* newProgram = static_cast<Program*>(root->clone());

  MainMethod* oldMain = root->main;
  MainMethod* newMain = static_cast<MainMethod*>(oldMain->clone());

  vector<Stm*>* newStmList = new vector<Stm*>;

  for (auto stm : *(oldMain->sl)) {
    newStmList->push_back(transformStm(stm));
  }

  newMain->sl = newStmList;
  newProgram->main = newMain;

  return newProgram;
}


//入口函数
Program *constantPropagate(Program *root) {
	cout << "TODO" << endl;
	// return root;

	// 先做 minusIntRewrite（它已经 clone）
	Program* afterMinus = minusIntRewrite(root);

  	// 再做常量折叠（完全 clone 风格）
  	return transformProgram(afterMinus);
}
