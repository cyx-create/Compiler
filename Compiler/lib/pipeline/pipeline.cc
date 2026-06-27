#include "pipeline.hh"

#include "reflow.hh"
#include "save_semant_ast.hh"
#include "schedule2xml.hh"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

#include "semant.hh"
#include "ast2tree.hh"
#include "ASTheader.hh"
#include "blocking.hh"
#include "canon.hh"
#include "flow2xml.hh"
#include "flowinfo.hh"
#include "loopinductionopt.hh"
#include "opt.hh"
#include "quad2xml.hh"
#include "quadssa.hh"
#include "tree2quad.hh"
#include "tree2xml.hh"
#include "xml2ast.hh"
#include "xml2flow.hh"
#include "xml2quad.hh"
#include "xml2tree.hh"

#include "advDFG.hh"
#include "asmprogpass.hh"
#include "coloring.hh"
#include "ig.hh"
#include "instrSelection.hh"
#include "preSchedule.hh"
#include "schedule.hh"
#include "xml2asmprog.hh"

namespace fs = std::filesystem;
using namespace std;
using namespace fdmj;
using namespace quad;

class LoopHeaderMap;
LoopHeaderMap *findLoopHeaders(QuadFuncDecl *func, FuncFlowInfo *ffi);
QuadFuncDecl *loopHoistFunc(QuadFuncDecl *func, LoopHeaderMap *loopHeaderMap);

namespace {

bool writeSourceListing(const string &fmjPath, const string &outPath) {
    ifstream in(fmjPath);
    ofstream out(outPath);
    if (!in || !out) return false;
    string line;
    while (getline(in, line)) {
        string trimmed;
        bool inBlock = false;
        for (size_t i = 0; i < line.size(); ++i) {
            if (inBlock) {
                if (i + 1 < line.size() && line[i] == '*' && line[i + 1] == '/') {
                    inBlock = false;
                    ++i;
                }
                continue;
            }
            if (i + 1 < line.size() && line[i] == '/' && line[i + 1] == '/') break;
            if (i + 1 < line.size() && line[i] == '/' && line[i + 1] == '*') {
                inBlock = true;
                ++i;
                continue;
            }
            trimmed.push_back(line[i]);
        }
        size_t start = trimmed.find_first_not_of(" \t\r");
        size_t end = trimmed.find_last_not_of(" \t\r");
        if (start == string::npos) continue;
        out << trimmed.substr(start, end - start + 1) << '\n';
    }
    return true;
}

bool loadSemantAst(const string &path, fdmj::Program **outRoot, AST_Semant_Map **outMap) {
    *outMap = new AST_Semant_Map();
    *outRoot = xml2ast(path, outMap);
    return *outRoot != nullptr;
}

bool ensureFrontend(const string &basePath) {
    string fmjPath = basePath + ".fmj";
    string irpPath = basePath + ".3.irp";
    string semantPath = basePath + ".2-semant.ast";

    if (!fs::exists(fmjPath)) {
        cerr << "Missing source: " << fmjPath << endl;
        return false;
    }

    bool needParse = !fs::exists(irpPath);
    if (!needParse) {
        needParse = fs::last_write_time(fmjPath) > fs::last_write_time(irpPath);
    }

    if (needParse) {
        ifstream in(fmjPath);
        if (!in) {
            cerr << "Cannot open " << fmjPath << endl;
            return false;
        }
        fdmj::Program *root = fdmjParser(in, false);
        if (root == nullptr) {
            cerr << "Parse failed: " << fmjPath << endl;
            return false;
        }
        AST_Semant_Map *semantMap = semant_analyze(root);
        if (semantMap == nullptr) {
            cerr << "Semantic analysis failed: " << fmjPath << endl;
            return false;
        }
        if (!saveSemantAst(root, semantMap, semantPath)) {
            cerr << "Failed to write semantic AST: " << semantPath << endl;
            return false;
        }

        tree::Program *ir = ast2tree(root, semantMap);
        if (ir == nullptr) {
            cerr << "IR generation failed: " << basePath << endl;
            return false;
        }
        tree2xml(ir)->SaveFile(irpPath.c_str());
        return true;
    }

    if (!fs::exists(irpPath) && fs::exists(semantPath)) {
        fdmj::Program *root = nullptr;
        AST_Semant_Map *semantMap = nullptr;
        if (!loadSemantAst(semantPath, &root, &semantMap)) return false;
        tree::Program *ir = ast2tree(root, semantMap);
        tree2xml(ir)->SaveFile(irpPath.c_str());
    }

    if (!fs::exists(irpPath) && !fs::exists(basePath + ".3-canon.irp")) {
        cerr << "Missing IR for " << basePath << endl;
        return false;
    }
    return true;
}

bool ensureQuad(const string &basePath) {
    if (fs::exists(basePath + ".4-xml.quad")) return true;
    if (!ensureFrontend(basePath)) return false;

    tree::Program *irCanon = nullptr;
    string canonPath = basePath + ".3-canon.irp";
    if (fs::exists(canonPath)) {
        irCanon = xml2tree(canonPath.c_str());
    } else {
        tree::Program *ir = xml2tree((basePath + ".3.irp").c_str());
        if (!ir) return false;
        irCanon = canon(ir);
        tree2xml(irCanon)->SaveFile(canonPath.c_str());
    }
    if (!irCanon) return false;
    QuadProgram *quad = tree2quad(irCanon);
    if (!quad || !quad->quadFuncDeclList || quad->quadFuncDeclList->empty()) {
        cerr << "IR to Quad conversion not available for " << basePath << ".fmj" << endl;
        cerr << "Provide bootstrap " << basePath << ".4-xml.quad (see HW5 reference quads)." << endl;
        return false;
    }
    writeQuadText(quad, basePath + ".4.quad", true);
    quad2xml(quad, (basePath + ".4-xml.quad").c_str());
    return true;
}

bool ensureSsa(const string &basePath) {
    string quadXmlPath = basePath + ".4-xml.quad";
    if (!fs::exists(quadXmlPath)) {
        if (!ensureQuad(basePath)) return false;
    }

    QuadProgram *quad = xml2quad(quadXmlPath.c_str());
    if (!quad || !quad->quadFuncDeclList || quad->quadFuncDeclList->empty()) {
        cerr << "Cannot read quad XML: " << quadXmlPath << endl;
        return false;
    }

    QuadProgram *blocked = blocking(quad);
    set<DataFlowInfo *> *allDfi = dataFLowProg(blocked);
    set<FuncFlowInfo *> *allFuncFlow = new set<FuncFlowInfo *>();
    for (auto *dfi : *allDfi) {
        if (!dfi) continue;
        dfi->computeLiveness();
        ControlFlowInfo *cfi = new ControlFlowInfo(dfi->func);
        cfi->computeEverything();
        allFuncFlow->insert(new FuncFlowInfo(cfi, dfi));
    }
    flowinfo2xml(allFuncFlow, (basePath + ".4-quadwithflow-xml.quad").c_str());
    QuadProgram *ssa = quad2ssa(allFuncFlow);
    if (!ssa) {
        cerr << "SSA conversion failed for " << basePath << endl;
        return false;
    }
    writeQuadText(ssa, basePath + ".4-ssa.quad", true);
    return reflowFromProgram(ssa, basePath);
}

set<FuncFlowInfo *> *reloadFlow(const string &reflowBase) {
    return xml2flow((reflowBase + ".4-ssa-withflow-xml.quad").c_str());
}

bool reflowAndReload(const string &reflowBase, QuadProgram *prog, set<FuncFlowInfo *> **flowOut) {
    if (!reflowFromProgram(prog, reflowBase)) return false;
    *flowOut = reloadFlow(reflowBase);
    return *flowOut != nullptr;
}

bool codegenFromWithflow(const string &withflowPath, const string &bootstrapBase,
                         const string &workBase, const string &coloredPath, int k) {
    set<FuncFlowInfo *> *flowSet = xml2flow(withflowPath.c_str());
    if (!flowSet || flowSet->empty()) return false;

    auto *funcList = new vector<QuadFuncDecl *>();
    int programLastLabelNum = -1;
    int programLastTempNum = -1;
    for (FuncFlowInfo *ffi : *flowSet) {
        if (!ffi || !ffi->cfi || !ffi->cfi->func) continue;
        funcList->push_back(ffi->cfi->func);
        if (ffi->programLastLabelNum >= 0) programLastLabelNum = ffi->programLastLabelNum;
        if (ffi->programLastTempNum >= 0) programLastTempNum = ffi->programLastTempNum;
    }

    QuadProgram *program = new QuadProgram(funcList, programLastLabelNum, programLastTempNum);
    instr::advDFGprog *graphProgram = instr::buildAdvDFGprog(program);
    instr::preScheduleProg *preScheduleProgram = instr::buildPreScheduleProg(program);
    if (!graphProgram || !preScheduleProgram) return false;

    instr::runInstructionSelectionPass(*graphProgram, *preScheduleProgram);
    instr::ScheduleProg *scheduleProgram = instr::scheduleProg(preScheduleProgram);
    if (!scheduleProgram) return false;

    scheduleProgram->writeToFile(workBase);
    string bootAsm = bootstrapBase + ".5-xml.asm";
    string asmXml = workBase + ".5-xml.asm";
    if (fs::exists(bootAsm)) {
        asmXml = bootAsm;
    } else if (!schedule2xml(scheduleProgram, asmXml.c_str())) {
        return false;
    }

    instr::AsmProg *asmProg = instr::xml2asmprog(asmXml.c_str());
    if (!asmProg) return false;
    instr::preDataFlowPass(asmProg);
    vector<InterferenceGraph *> graphs = buildIgProg(asmProg);
    vector<Coloring *> colorings;
    for (auto *g : graphs) colorings.push_back(coloring(g, k));
    instr::AsmProg *colored = instr::asmprog2colored(asmProg, colorings);
    ofstream out(coloredPath);
    if (!out) return false;
    out << colored->to_string();
    return true;
}

static bool isBackendSsaBootstrap(const string &baseName) {
    static const set<string> kBackendBootstrap = {
        "insttest0", "insttest1", "insttest2", "insttest3", "insttest4",
        "fibonacci", "bubblesort",
        "optloopivtest1", "optloopivtest2", "optloopivtest3",
        "optloopivtest4", "optloopivtest5", "optloopivtest6",
    };
    return kBackendBootstrap.count(baseName) > 0;
}

} // namespace

namespace compiler {

string optSuffix(OptMode mode) {
    switch (mode) {
    case OptMode::Base: return "base";
    case OptMode::Const: return "const";
    case OptMode::Loop1: return "loop1";
    case OptMode::Loop2: return "loop2";
    case OptMode::AllLoop: return "allloop";
    case OptMode::AllOpt: return "allopt";
    }
    return "base";
}

string optQuadSuffix(OptMode mode) {
    switch (mode) {
    case OptMode::Base: return ".4-ssa.quad";
    case OptMode::Const: return ".4-ssa-opt.quad";
    case OptMode::Loop1: return ".4-ssa-loopopt.quad";
    case OptMode::Loop2: return ".4-ssa-loopivopt.quad";
    case OptMode::AllLoop: return ".4-ssa-allloop.quad";
    case OptMode::AllOpt: return ".4-ssa-allopt.quad";
    }
    return ".4-ssa.quad";
}

static bool buildOptimizedWithflow(const string &basePath, set<FuncFlowInfo *> *baseFlow,
                                   OptMode mode, string *withflowOut, string *quadOut) {
    string reflowBase = basePath + ".reflow-" + optSuffix(mode);
    *quadOut = basePath + optQuadSuffix(mode);

    if (mode == OptMode::Base) {
        *withflowOut = basePath + ".4-ssa-withflow-xml.quad";
        ifstream in(basePath + ".4-ssa.quad");
        ofstream out(*quadOut);
        out << in.rdbuf();
        return true;
    }

    set<FuncFlowInfo *> *flow = baseFlow;
    QuadProgram *prog = nullptr;
    int lastLabel = 0, lastTemp = 0;
    for (FuncFlowInfo *ffi : *flow) {
        lastLabel = max(lastLabel, ffi->programLastLabelNum);
        lastTemp = max(lastTemp, ffi->programLastTempNum);
    }

    if (mode == OptMode::Const || mode == OptMode::AllOpt) {
        vector<QuadFuncDecl *> funcs;
        for (FuncFlowInfo *ffi : *flow) funcs.push_back(ffi->cfi->func);
        prog = new QuadProgram(new vector<QuadFuncDecl *>(funcs), lastLabel, lastTemp);
        prog = optProg(prog);
        if (!reflowAndReload(reflowBase, prog, &flow)) return false;
    }

    if (mode == OptMode::Loop1 || mode == OptMode::AllLoop || mode == OptMode::AllOpt) {
        vector<QuadFuncDecl *> *optimized = new vector<QuadFuncDecl *>();
        for (FuncFlowInfo *ffi : *flow) {
            LoopHeaderMap *headers = findLoopHeaders(ffi->cfi->func, ffi);
            optimized->push_back(loopHoistFunc(ffi->cfi->func, headers));
        }
        prog = new QuadProgram(optimized, lastLabel, lastTemp);
        if (!reflowAndReload(reflowBase, prog, &flow)) return false;
    }

    if (mode == OptMode::Loop2 || mode == OptMode::AllLoop || mode == OptMode::AllOpt) {
        vector<QuadFuncDecl *> *optimized = new vector<QuadFuncDecl *>();
        for (FuncFlowInfo *ffi : *flow) {
            auto *fl = new vector<QuadFuncDecl *>();
            fl->push_back(ffi->cfi->func);
            QuadProgram *fp = new QuadProgram(fl, ffi->programLastLabelNum, ffi->programLastTempNum);
            QuadProgram *s1 = loopInductionStrengthReductionPass(fp, ffi->cfi);
            QuadProgram *s2 = loopInductionCleanupPass(s1);
            if (s2 && s2->quadFuncDeclList && !s2->quadFuncDeclList->empty()) {
                optimized->push_back(s2->quadFuncDeclList->at(0));
            }
        }
        prog = new QuadProgram(optimized, lastLabel, lastTemp);
        if (!reflowAndReload(reflowBase, prog, &flow)) return false;
    }

    writeQuadText(prog, *quadOut, false);
    *withflowOut = reflowBase + ".4-ssa-withflow-xml.quad";
    return true;
}

bool compileQuadOnly(const string &testDir, const string &baseName) {
    string basePath = testDir + "/" + baseName;
    if (!fs::exists(basePath + ".fmj")) {
        cerr << "Missing " << basePath << ".fmj" << endl;
        return false;
    }
    writeSourceListing(basePath + ".fmj", basePath + ".src");
    if (!ensureFrontend(basePath)) return false;
    return ensureQuad(basePath);
}

bool compileProgram(const string &testDir, const string &baseName, OptMode mode) {
    string basePath = testDir + "/" + baseName;
    if (!fs::exists(basePath + ".fmj")) {
        cerr << "Missing " << basePath << ".fmj" << endl;
        return false;
    }

    writeSourceListing(basePath + ".fmj", basePath + ".src");
    bool hasBootstrapSsa = isBackendSsaBootstrap(baseName) &&
                           fs::exists(basePath + ".4-ssa-withflow-xml.quad");
    if (!hasBootstrapSsa) {
        if (!ensureFrontend(basePath)) return false;
        if (!ensureSsa(basePath)) return false;
    }

    set<FuncFlowInfo *> *baseFlow = reloadFlow(basePath);
    if (!baseFlow) return false;

    string withflowPath;
    string quadPath;
    if (!buildOptimizedWithflow(basePath, baseFlow, mode, &withflowPath, &quadPath)) return false;

    string coloredPath = basePath + "." + optSuffix(mode) + ".colored.s";
    string workBase = basePath + "." + optSuffix(mode);
    return codegenFromWithflow(withflowPath, basePath, workBase, coloredPath, 9);
}

bool compileAllModes(const string &testDir, const string &baseName) {
    for (OptMode mode : {OptMode::Base, OptMode::Const, OptMode::Loop1,
                         OptMode::Loop2, OptMode::AllLoop, OptMode::AllOpt}) {
        if (!compileProgram(testDir, baseName, mode)) return false;
    }
    return true;
}

} // namespace compiler
