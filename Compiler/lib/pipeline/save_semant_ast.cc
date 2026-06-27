#include "save_semant_ast.hh"

#include "ast2xml.hh"
#include "FDMJAST.hh"
#include "semant.hh"

using namespace std;
using namespace fdmj;
using namespace tinyxml2;

bool saveSemantAst(fdmj::Program *root, AST_Semant_Map *semantMap, const string &path) {
    if (root == nullptr || semantMap == nullptr) {
        return false;
    }
    XMLDocument *xml = ast2xml(root, semantMap, true, true);
    if (xml == nullptr || xml->Error()) {
        return false;
    }
    xml->SaveFile(path.c_str());
    return !xml->Error();
}
