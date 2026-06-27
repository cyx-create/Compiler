#ifndef _SAVE_SEMANT_AST_HH
#define _SAVE_SEMANT_AST_HH

#include <string>

namespace fdmj {
class Program;
}
class AST_Semant_Map;

bool saveSemantAst(fdmj::Program *root, AST_Semant_Map *semantMap, const std::string &path);

#endif
