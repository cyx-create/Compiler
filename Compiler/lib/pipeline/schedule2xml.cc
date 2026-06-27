#include "schedule2xml.hh"

#include <algorithm>

#include "schedule.hh"
#include "tinyxml2.hh"

using namespace tinyxml2;

namespace instr {

namespace {

const char *kindToString(AssemInstr::Kind kind) {
    switch (kind) {
    case AssemInstr::I_LABEL: return "I_LABEL";
    case AssemInstr::I_MOVE: return "I_MOVE";
    case AssemInstr::I_CALL: return "I_CALL";
    case AssemInstr::I_EXTCALL: return "I_EXTCALL";
    case AssemInstr::I_OPER:
    default: return "I_OPER";
    }
}

void appendTemps(XMLDocument *doc, XMLElement *parent, const char *tag,
                 const std::vector<tree::Temp *> &temps) {
    XMLElement *elem = doc->NewElement(tag);
    for (size_t i = 0; i < temps.size(); ++i) {
        if (temps[i] == nullptr) continue;
        XMLElement *tempElem = doc->NewElement("Temp");
        tempElem->SetAttribute("index", static_cast<int>(i));
        tempElem->SetAttribute("num", temps[i]->num);
        tempElem->SetAttribute("name", temps[i]->str().c_str());
        elem->InsertEndChild(tempElem);
    }
    parent->InsertEndChild(elem);
}

void appendJumps(XMLDocument *doc, XMLElement *parent, const AssemTargets &jumps) {
    XMLElement *jumpsElem = doc->NewElement("Jumps");
    for (size_t i = 0; i < jumps.labels.size(); ++i) {
        if (jumps.labels[i] == nullptr) continue;
        XMLElement *labelElem = doc->NewElement("Label");
        labelElem->SetAttribute("index", static_cast<int>(i));
        labelElem->SetAttribute("num", jumps.labels[i]->num);
        labelElem->SetAttribute("name", jumps.labels[i]->str().c_str());
        jumpsElem->InsertEndChild(labelElem);
    }
    parent->InsertEndChild(jumpsElem);
}

int computeMaxTemp(const ScheduleProg *scheduleProgram) {
    int maxTemp = -1;
    for (auto *funcSchedule : scheduleProgram->funcSchedules) {
        if (funcSchedule == nullptr || funcSchedule->quadFunc == nullptr) continue;
        maxTemp = std::max(maxTemp, funcSchedule->quadFunc->last_temp_num);
        for (const auto &instr : funcSchedule->linearizedInstructions.instrs) {
            for (auto *temp : instr.dst) if (temp) maxTemp = std::max(maxTemp, temp->num);
            for (auto *temp : instr.src) if (temp) maxTemp = std::max(maxTemp, temp->num);
        }
    }
    if (scheduleProgram->quadProgram != nullptr) {
        maxTemp = std::max(maxTemp, scheduleProgram->quadProgram->last_temp_num);
    }
    return maxTemp;
}

int computeMaxLabel(const ScheduleProg *scheduleProgram) {
    int maxLabel = -1;
    for (auto *funcSchedule : scheduleProgram->funcSchedules) {
        if (funcSchedule == nullptr || funcSchedule->quadFunc == nullptr) continue;
        maxLabel = std::max(maxLabel, funcSchedule->quadFunc->last_label_num);
        for (const auto &instr : funcSchedule->linearizedInstructions.instrs) {
            if (instr.label) maxLabel = std::max(maxLabel, instr.label->num);
            for (auto *jumpLabel : instr.jumps.labels) {
                if (jumpLabel) maxLabel = std::max(maxLabel, jumpLabel->num);
            }
        }
    }
    if (scheduleProgram->quadProgram != nullptr) {
        maxLabel = std::max(maxLabel, scheduleProgram->quadProgram->last_label_num);
    }
    return maxLabel;
}

} // namespace

bool schedule2xml(const ScheduleProg *scheduleProgram, const char *filename) {
    if (scheduleProgram == nullptr || filename == nullptr) return false;

    XMLDocument doc;
    XMLElement *root = doc.NewElement("ScheduleProgram");
    root->SetAttribute("program_last_label_num", computeMaxLabel(scheduleProgram));
    root->SetAttribute("program_last_temp_num", computeMaxTemp(scheduleProgram));
    root->SetAttribute("function_count", static_cast<int>(scheduleProgram->funcSchedules.size()));
    doc.InsertEndChild(root);

    for (size_t funcIndex = 0; funcIndex < scheduleProgram->funcSchedules.size(); ++funcIndex) {
        auto *funcSchedule = scheduleProgram->funcSchedules[funcIndex];
        if (funcSchedule == nullptr || funcSchedule->quadFunc == nullptr) continue;

        XMLElement *funcElem = doc.NewElement("Function");
        funcElem->SetAttribute("index", static_cast<int>(funcIndex));
        funcElem->SetAttribute("name", funcSchedule->quadFunc->funcname.c_str());
        funcElem->SetAttribute("last_label_num", funcSchedule->quadFunc->last_label_num);
        funcElem->SetAttribute("last_temp_num", funcSchedule->quadFunc->last_temp_num);
        funcElem->SetAttribute("instruction_count",
            static_cast<int>(funcSchedule->linearizedInstructions.instrs.size()));

        for (size_t instrIndex = 0; instrIndex < funcSchedule->linearizedInstructions.instrs.size(); ++instrIndex) {
            const AssemInstr &instr = funcSchedule->linearizedInstructions.instrs[instrIndex];
            XMLElement *instrElem = doc.NewElement("Instruction");
            instrElem->SetAttribute("index", static_cast<int>(instrIndex));
            instrElem->SetAttribute("kind", kindToString(instr.kind));
            instrElem->SetAttribute("assem", instr.assem.c_str());

            if (instr.kind == AssemInstr::I_LABEL && instr.label != nullptr) {
                XMLElement *labelElem = doc.NewElement("Label");
                labelElem->SetAttribute("num", instr.label->num);
                labelElem->SetAttribute("name", instr.label->str().c_str());
                instrElem->InsertEndChild(labelElem);
            }

            appendTemps(&doc, instrElem, "Dst", instr.dst);
            appendTemps(&doc, instrElem, "Src", instr.src);
            appendJumps(&doc, instrElem, instr.jumps);
            funcElem->InsertEndChild(instrElem);
        }
        root->InsertEndChild(funcElem);
    }

    return doc.SaveFile(filename) == XML_SUCCESS;
}

} // namespace instr
