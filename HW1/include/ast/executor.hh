#ifndef _EXECUTOR_H
#define _EXECUTOR_H

#include "ASTheader.hh"
#include "FDMJAST.hh"
#include <map>

using namespace std;
using namespace fdmj;

int execute(Program *root);

class Executor : public ASTVisitor {
public:
    map<string,int> env;   // 变量环境
    int retVal = 0;        // return 值
    AST *newNode = nullptr; // 访问接口保留

    Executor() {}
    ~Executor() {}

    // Visitor接口
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

#endif