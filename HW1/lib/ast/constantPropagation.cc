#define DEBUG
#undef DEBUG

#include "constantPropagation.hh"
#include "MinusIntConverter.hh"
#include <iostream>

using namespace std;
using namespace fdmj;

#define StmList vector<Stm *>

Program *constantPropagate(Program *root) {
  if (root == nullptr)
    return nullptr;

  // 先做 minus rewrite
  Program *afterMinus = minusIntRewrite(root);

  ConstantPropagation v(nullptr);
  afterMinus->accept(v);

  return dynamic_cast<Program *>(v.newNode);
}

template <typename T>
static vector<T *> *visitList(ConstantPropagation &v, vector<T *> *tl) {

  if (tl == nullptr || tl->size() == 0)
    return nullptr;

  vector<T *> *vt = new vector<T *>();

  for (T *x : *tl) {

    if (x == nullptr)
      continue;

    x->accept(v);

    if (v.newNode == nullptr)
      continue;

    vt->push_back(static_cast<T *>(v.newNode));
  }

  if (vt->size() == 0) {
    delete vt;
    vt = nullptr;
  }

  return vt;
}

void ConstantPropagation::visit(Program *node) {

#ifdef DEBUG
  cerr << "Rewriting Program...\n";
#endif

  if (node == nullptr) {
    newNode = nullptr;
    return;
  }

  MainMethod *m = nullptr;

  if (node->main != nullptr) {

    node->main->accept(*this);

    if (newNode != nullptr)
      m = static_cast<MainMethod *>(newNode);
  }

  newNode = new Program(node->getPos()->clone(), m);
}

void ConstantPropagation::visit(MainMethod *node) {

#ifdef DEBUG
  cerr << "Rewriting MainMethod...\n";
#endif

  if (node == nullptr) {
    newNode = nullptr;
    return;
  }

  StmList *sl = nullptr;

  if (node->sl != nullptr)
    sl = visitList<Stm>(*this, node->sl);

  newNode = new MainMethod(node->getPos()->clone(), sl);
}

void ConstantPropagation::visit(Assign *node) {

#ifdef DEBUG
  cerr << "Rewriting Assign...\n";
#endif

  if (node == nullptr) {
    newNode = nullptr;
    return;
  }

  Exp *l = nullptr;
  Exp *r = nullptr;

  if (node->left != nullptr) {
    node->left->accept(*this);
    l = static_cast<Exp *>(newNode);
  }

  if (node->exp != nullptr) {
    node->exp->accept(*this);
    r = static_cast<Exp *>(newNode);
  }

  newNode = new Assign(node->getPos()->clone(), l, r);
}

void ConstantPropagation::visit(Return *node) {

#ifdef DEBUG
  cerr << "Rewriting Return...\n";
#endif

  if (node == nullptr) {
    newNode = nullptr;
    return;
  }

  Exp *e = nullptr;

  if (node->exp != nullptr) {
    node->exp->accept(*this);
    e = static_cast<Exp *>(newNode);
  }

  newNode = new Return(node->getPos()->clone(), e);
}

void ConstantPropagation::visit(BinaryOp *node) {

#ifdef DEBUG
  cerr << "Rewriting BinaryOp...\n";
#endif

  if (node == nullptr) {
    newNode = nullptr;
    return;
  }

  Exp *l = nullptr;
  Exp *r = nullptr;

  if (node->left != nullptr) {
    node->left->accept(*this);
    l = static_cast<Exp *>(newNode);
  }

  if (node->right != nullptr) {
    node->right->accept(*this);
    r = static_cast<Exp *>(newNode);
  }

  // 常量折叠
  if (l != nullptr && r != nullptr &&
      l->getASTKind() == ASTKind::IntExp &&
      r->getASTKind() == ASTKind::IntExp) {

    int lv = static_cast<IntExp *>(l)->val;
    int rv = static_cast<IntExp *>(r)->val;

    string op = node->op->op;

    int result = 0;

    if (op == "+")
      result = lv + rv;
    else if (op == "-")
      result = lv - rv;
    else if (op == "*")
      result = lv * rv;
    else if (op == "/")
      result = lv / rv;

    newNode = new IntExp(node->getPos()->clone(), result);
    return;
  }

  newNode = new BinaryOp(node->getPos()->clone(), l, node->op->clone(), r);
}

void ConstantPropagation::visit(UnaryOp *node) {

#ifdef DEBUG
  cerr << "Rewriting UnaryOp...\n";
#endif

  if (node == nullptr) {
    newNode = nullptr;
    return;
  }

  Exp *e = nullptr;

  if (node->exp != nullptr) {
    node->exp->accept(*this);
    e = static_cast<Exp *>(newNode);
  }

  newNode = new UnaryOp(node->getPos()->clone(), node->op->clone(), e);
}

void ConstantPropagation::visit(IdExp *node) {

#ifdef DEBUG
  cerr << "Rewriting IdExp...\n";
#endif

  newNode = (node == nullptr) ? nullptr : static_cast<IdExp *>(node->clone());
}

void ConstantPropagation::visit(IntExp *node) {

#ifdef DEBUG
  cerr << "Rewriting IntExp...\n";
#endif

  newNode = (node == nullptr) ? nullptr : static_cast<IntExp *>(node->clone());
}

void ConstantPropagation::visit(OpExp *node) {

#ifdef DEBUG
  cerr << "Rewriting OpExp...\n";
#endif

  newNode = (node == nullptr) ? nullptr : static_cast<OpExp *>(node->clone());
}