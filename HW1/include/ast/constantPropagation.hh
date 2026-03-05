#ifndef _CONSTANTPROPAGATION_H
#define _CONSTANTPROPAGATION_H

#include "ASTheader.hh"
#include "FDMJAST.hh"
#include <map>
using namespace std;
using namespace fdmj;

Program *constantPropagate(Program *root);

class ConstantPropagation : public ASTVisitor {
public:
    map<string,int> constTable;
    AST *newNode =
      nullptr; // newNode is the root of a clone of the original AST (sub) tree
    ConstantPropagation(AST *newNode) : newNode(newNode) {} 

   void visit(Program *node) override;
   void visit(MainMethod *node) override;
   void visit(Assign *node) override;
   void visit(Return *node) override;
   void visit(BinaryOp *node) override;
   void visit(UnaryOp *node) override;
   void visit(IdExp *node) override;
   void visit(OpExp *node) override;
   void visit(IntExp *node) override;
};

Program *constantPropagate(Program *root);
#endif