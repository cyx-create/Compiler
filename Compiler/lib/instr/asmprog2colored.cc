#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <regex>
#include <sstream>
#include <algorithm>
#include "asmprogpass.hh"
#include "temp.hh"
#include "coloring.hh"

using namespace std;
using namespace tree;

namespace instr {

namespace {

int resolveAlias(int tempNum, const Coloring* coloring) {
    int n = tempNum;
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& entry : coloring->coalescedMoves) {
            if (entry.second.find(n) != entry.second.end()) {
                n = entry.first;
                changed = true;
                break;
            }
        }
    }
    return n;
}

bool isSpilledTemp(int tempNum, const Coloring* coloring) {
    return coloring->spilled.find(resolveAlias(tempNum, coloring)) != coloring->spilled.end();
}

string cleanFuncName(string funcName) {
    size_t pos = funcName.find('^');
    while (pos != string::npos) {
        funcName.replace(pos, 1, "$");
        pos = funcName.find('^', pos + 1);
    }
    if (funcName.substr(0, 3) == "__$") {
        funcName = funcName.substr(3);
        size_t underscores = funcName.find("__$");
        if (underscores != string::npos) {
            funcName = funcName.substr(0, underscores);
        }
    }
    return funcName;
}

void replaceAll(string& text, const string& from, const string& to) {
    if (from.empty()) {
        return;
    }
    size_t pos = 0;
    while ((pos = text.find(from, pos)) != string::npos) {
        text.replace(pos, from.length(), to);
        pos += to.length();
    }
}

string formatColoredAssem(
    const AssemInstr& instr,
    const string& funcName,
    const Coloring* coloring,
    const map<int, string>& spilledSrcRegs
) {
    if (instr.kind == AssemInstr::I_LABEL) {
        string label = instr.assem;
        size_t pos = label.find('^');
        while (pos != string::npos) {
            label.replace(pos, 1, "$");
            pos = label.find('^', pos + 1);
        }
        return label;
    }

    string result = instr.assem;
    size_t pos = result.find('^');
    while (pos != string::npos) {
        result.replace(pos, 1, "$");
        pos = result.find('^', pos + 1);
    }

    for (size_t i = 0; i < instr.dst.size(); ++i) {
        string placeholder = "`d" + to_string(i);
        string regName;
        if (instr.dst[i] != nullptr) {
            if (isSpilledTemp(instr.dst[i]->num, coloring)) {
                regName = "r10";
            } else {
                regName = getTempRegName(instr.dst[i]->num, coloring);
            }
            replaceAll(result, placeholder, regName);
        }
    }

    for (size_t i = 0; i < instr.src.size(); ++i) {
        string placeholder = "`s" + to_string(i);
        string regName;
        if (instr.src[i] != nullptr) {
            if (isSpilledTemp(instr.src[i]->num, coloring)) {
                int alias = resolveAlias(instr.src[i]->num, coloring);
                auto it = spilledSrcRegs.find(alias);
                regName = (it != spilledSrcRegs.end()) ? it->second : "r12";
            } else {
                regName = getTempRegName(instr.src[i]->num, coloring);
            }
            replaceAll(result, placeholder, regName);
        }
    }

    for (size_t i = 0; i < instr.jumps.labels.size(); ++i) {
        string placeholder = "`j" + to_string(i);
        if (instr.jumps.labels[i] != nullptr) {
            string labelName = funcName + "$L" + to_string(instr.jumps.labels[i]->num);
            replaceAll(result, placeholder, labelName);
        }
    }

    regex adrLiteral(R"(adr\s+\w+,\s+([A-Za-z_]\w*))");
    smatch match;
    string::const_iterator searchStart(result.cbegin());
    while (regex_search(searchStart, result.cend(), match, adrLiteral)) {
        string labelName = match[1].str();
        if (!labelName.empty() && labelName[0] == 'L' && labelName.find('$') == string::npos) {
            string prefixed = funcName + "$" + labelName;
            size_t labelPos = distance(result.cbegin(), match[1].first);
            result = result.substr(0, labelPos) + prefixed + result.substr(labelPos + labelName.length());
            searchStart = result.cbegin() + labelPos + prefixed.length();
        } else {
            searchStart = match[0].second;
        }
    }

    result = regex_replace(
        result,
        regex(R"(adr\s+(\w+),\s+(\S*\$\S*))"),
        "ldr $1, =$2"
    );

    return result;
}

AssemInstr makeOper(const string& assem) {
    return AssemInstr::Oper(assem, {}, {}, AssemTargets());
}

void patchStackInstruction(AssemInstr& instr, int extraBytes) {
    if (extraBytes == 0) {
        return;
    }

    regex subSp(R"(sub\s+sp,\s+sp,\s+#(\d+))");
    regex addSp(R"(add\s+sp,\s+sp,\s+#(\d+))");
    regex addFp(R"(add\s+fp,\s+sp,\s+#(\d+))");
    regex subFp(R"(sub\s+sp,\s+fp,\s+#(\d+))");

    smatch match;
    if (regex_search(instr.assem, match, subSp) && match.size() > 1) {
        int oldVal = stoi(match[1].str());
        instr.assem = regex_replace(instr.assem, subSp, "sub sp, sp, #" + to_string(oldVal + extraBytes));
    } else if (regex_search(instr.assem, match, addSp) && match.size() > 1) {
        int oldVal = stoi(match[1].str());
        instr.assem = regex_replace(instr.assem, addSp, "add sp, sp, #" + to_string(oldVal + extraBytes));
    } else if (regex_search(instr.assem, match, addFp) && match.size() > 1) {
        int oldVal = stoi(match[1].str());
        instr.assem = regex_replace(instr.assem, addFp, "add fp, sp, #" + to_string(oldVal + extraBytes));
    } else if (regex_search(instr.assem, match, subFp) && match.size() > 1) {
        int oldVal = stoi(match[1].str());
        instr.assem = regex_replace(instr.assem, subFp, "sub sp, fp, #" + to_string(oldVal + extraBytes));
    }
}

vector<AssemInstr> expandInstruction(
    const AssemInstr& instr,
    const string& funcName,
    const Coloring* coloring,
    const map<int, int>& spillOffsets
) {
    vector<AssemInstr> expanded;

    map<int, string> spilledSrcRegs;
    vector<int> srcOrder;
    for (tree::Temp* t : instr.src) {
        if (t != nullptr) {
            int alias = resolveAlias(t->num, coloring);
            if (isSpilledTemp(t->num, coloring) &&
                find(srcOrder.begin(), srcOrder.end(), alias) == srcOrder.end()) {
                srcOrder.push_back(alias);
            }
        }
    }
    for (size_t i = 0; i < srcOrder.size(); ++i) {
        spilledSrcRegs[srcOrder[i]] = (i == 0) ? "r12" : "r10";
    }

    if (instr.kind == AssemInstr::I_LABEL) {
        AssemInstr label = instr;
        label.assem = formatColoredAssem(instr, funcName, coloring, spilledSrcRegs);
        label.dst.clear();
        label.src.clear();
        expanded.push_back(label);
        return expanded;
    }

    if (instr.kind == AssemInstr::I_MOVE && instr.dst.size() == 1 && instr.src.size() == 1 &&
        instr.dst[0] != nullptr && instr.src[0] != nullptr) {
        int dstAlias = resolveAlias(instr.dst[0]->num, coloring);
        int srcAlias = resolveAlias(instr.src[0]->num, coloring);
        bool dstSpill = isSpilledTemp(instr.dst[0]->num, coloring);
        bool srcSpill = isSpilledTemp(instr.src[0]->num, coloring);

        if (dstAlias == srcAlias) {
            return expanded;
        }

        if (dstSpill && srcSpill) {
            expanded.push_back(makeOper(
                "ldr r12, [fp, #" + to_string(spillOffsets.at(srcAlias)) + "]"));
            expanded.push_back(makeOper(
                "str r12, [fp, #" + to_string(spillOffsets.at(dstAlias)) + "]"));
            return expanded;
        }

        if (dstSpill && !srcSpill) {
            expanded.push_back(makeOper(
                "mov r10, " + getTempRegName(instr.src[0]->num, coloring)));
            expanded.push_back(makeOper(
                "str r10, [fp, #" + to_string(spillOffsets.at(dstAlias)) + "]"));
            return expanded;
        }

        if (!dstSpill && srcSpill) {
            expanded.push_back(makeOper(
                "ldr r12, [fp, #" + to_string(spillOffsets.at(srcAlias)) + "]"));
            expanded.push_back(makeOper(
                "mov " + getTempRegName(instr.dst[0]->num, coloring) + ", r12"));
            return expanded;
        }

        if (getTempRegName(instr.dst[0]->num, coloring) ==
            getTempRegName(instr.src[0]->num, coloring)) {
            return expanded;
        }

        string formatted = formatColoredAssem(instr, funcName, coloring, spilledSrcRegs);
        AssemInstr colored = instr;
        colored.assem = formatted;
        colored.dst.clear();
        colored.src.clear();
        colored.jumps = AssemTargets();
        expanded.push_back(colored);
        return expanded;
    }

    set<int> spilledDsts;
    for (tree::Temp* t : instr.dst) {
        if (t != nullptr && isSpilledTemp(t->num, coloring)) {
            spilledDsts.insert(resolveAlias(t->num, coloring));
        }
    }

    for (size_t i = 0; i < srcOrder.size(); ++i) {
        int off = spillOffsets.at(srcOrder[i]);
        string reg = (i == 0) ? "r12" : "r10";
        expanded.push_back(makeOper("ldr " + reg + ", [fp, #" + to_string(off) + "]"));
    }

    string formatted = formatColoredAssem(instr, funcName, coloring, spilledSrcRegs);
    AssemInstr colored = instr;
    colored.assem = formatted;
    colored.dst.clear();
    colored.src.clear();
    colored.jumps = AssemTargets();
    expanded.push_back(colored);

    vector<int> dstOrder;
    for (tree::Temp* t : instr.dst) {
        if (t != nullptr) {
            int alias = resolveAlias(t->num, coloring);
            if (spilledDsts.count(alias) && find(dstOrder.begin(), dstOrder.end(), alias) == dstOrder.end()) {
                dstOrder.push_back(alias);
            }
        }
    }

    for (int alias : dstOrder) {
        int off = spillOffsets.at(alias);
        expanded.push_back(makeOper("str r10, [fp, #" + to_string(off) + "]"));
    }

    return expanded;
}

} // namespace

string getRegName(int colorNum) {
    return "r" + to_string(colorNum);
}

string getTempRegName(int tempNum, const Coloring* coloring) {
    int alias = resolveAlias(tempNum, coloring);
    if (coloring->spilled.find(alias) != coloring->spilled.end()) {
        return "r10";
    }
    if (Coloring::isMachineReg(alias)) {
        return "r" + to_string(alias);
    }
    auto it = coloring->colors.find(alias);
    if (it == coloring->colors.end()) {
        return "r0";
    }
    return getRegName(it->second);
}

AsmProg* asmprog2colored(AsmProg* program, const vector<Coloring*>& colorings) {
    AsmProg* colored = new AsmProg();

    if (program == nullptr) {
        return colored;
    }

    for (size_t fi = 0; fi < program->functions.size(); ++fi) {
        const AsmFunction& func = program->functions[fi];
        Coloring* coloring = (fi < colorings.size()) ? colorings[fi] : nullptr;
        string funcName = cleanFuncName(func.name);

        map<int, int> spillOffsets;
        int extraStack = 0;
        if (coloring != nullptr) {
            vector<int> spillList(coloring->spilled.begin(), coloring->spilled.end());
            sort(spillList.begin(), spillList.end());
            for (size_t i = 0; i < spillList.size(); ++i) {
                spillOffsets[spillList[i]] = -40 - static_cast<int>(i) * 4;
            }
            extraStack = static_cast<int>(spillList.size()) * 4;
        }

        AsmFunction newFunc(func.name);
        for (const AssemInstr& instr : func.instructions) {
            if (coloring == nullptr) {
                newFunc.instructions.push_back(instr);
                continue;
            }

            vector<AssemInstr> expanded = expandInstruction(instr, funcName, coloring, spillOffsets);
            for (AssemInstr& ins : expanded) {
                patchStackInstruction(ins, extraStack);
                newFunc.instructions.push_back(ins);
            }
        }

        colored->functions.push_back(newFunc);
    }

    return colored;
}

} // namespace instr
