#include "reflow.hh"

#include <fstream>
#include <iostream>
#include <string>

#include "blocking.hh"
#include "flow2xml.hh"
#include "flowinfo.hh"
#include "quad2xml.hh"

using namespace std;
using namespace quad;

bool writeQuadText(QuadProgram *prog, const string &path, bool blocked) {
    ofstream out(path);
    if (!out) {
        cerr << "Error opening file: " << path << endl;
        return false;
    }
    string text;
    text.reserve(100000);
    prog->print(text, 0, blocked);
    out << text;
    return true;
}

bool reflowFromProgram(QuadProgram *prog, const string &basePath) {
    if (prog == nullptr) {
        return false;
    }

    QuadProgram *blocked = blocking(prog);
    if (blocked == nullptr) {
        cerr << "Error blocking quad program during reflow" << endl;
        return false;
    }

    set<DataFlowInfo *> *allDfi = dataFLowProg(blocked);
    if (allDfi == nullptr) {
        cerr << "Error computing data flow during reflow" << endl;
        return false;
    }

    set<FuncFlowInfo *> *allFuncFlow = new set<FuncFlowInfo *>();
    for (auto *dfItem : *allDfi) {
        if (dfItem == nullptr) {
            continue;
        }
        dfItem->computeLiveness();
        ControlFlowInfo *cfi = new ControlFlowInfo(dfItem->func);
        cfi->computeEverything();
        allFuncFlow->insert(new FuncFlowInfo(cfi, dfItem));
    }

    string withflowPath = basePath + ".4-ssa-withflow-xml.quad";
    string ssaXmlPath = basePath + ".4-ssa-xml.quad";

    if (!flowinfo2xml(allFuncFlow, withflowPath.c_str())) {
        cerr << "Error writing flow XML: " << withflowPath << endl;
        return false;
    }
    if (!quad2xml(blocked, ssaXmlPath.c_str())) {
        cerr << "Error writing quad XML: " << ssaXmlPath << endl;
        return false;
    }
    return true;
}
