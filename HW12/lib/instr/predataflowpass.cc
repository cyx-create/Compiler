#include "asmprogpass.hh"
#include "temp.hh"

namespace instr {

void preDataFlowPass(AsmProg* prog) {
    if (prog == nullptr) {
        return;
    }

    for (AsmFunction& func : prog->functions) {
        for (AssemInstr& instr : func.instructions) {
            if (instr.kind != AssemInstr::I_OPER) {
                continue;
            }
            const std::string& a = instr.assem;
            bool isCall = (a.rfind("bl ", 0) == 0) || (a.rfind("blx ", 0) == 0);
            if (!isCall) {
                continue;
            }

            std::set<int> existing;
            for (tree::Temp* t : instr.dst) {
                if (t != nullptr) {
                    existing.insert(t->num);
                }
            }

            for (int reg = 0; reg <= 3; ++reg) {
                if (existing.find(reg) == existing.end()) {
                    instr.dst.push_back(new tree::Temp(reg));
                    existing.insert(reg);
                }
            }
        }
    }
}

} // namespace instr
