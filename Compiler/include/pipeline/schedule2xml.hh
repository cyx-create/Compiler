#ifndef COMPILER_SCHEDULE2XML_HH
#define COMPILER_SCHEDULE2XML_HH

namespace instr {
struct ScheduleProg;
bool schedule2xml(const ScheduleProg *scheduleProgram, const char *filename);
}

#endif
