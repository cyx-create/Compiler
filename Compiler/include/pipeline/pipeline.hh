#ifndef COMPILER_PIPELINE_HH
#define COMPILER_PIPELINE_HH

#include <string>

namespace compiler {

enum class OptMode {
    Base,
    Const,
    Loop1,
    Loop2,
    AllLoop,
    AllOpt
};

std::string optSuffix(OptMode mode);
std::string optQuadSuffix(OptMode mode);

// Compile one test program under testDir; baseName has no directory prefix.
// Generates intermediate artifacts and testDir/baseName.<suffix>.colored.s for the given mode.
bool compileProgram(const std::string &testDir, const std::string &baseName, OptMode mode);

// Stop after IR→Quad (for HW5 structural verification).
bool compileQuadOnly(const std::string &testDir, const std::string &baseName);

// Emit all course-required artifacts for every optimization mode.
bool compileAllModes(const std::string &testDir, const std::string &baseName);

} // namespace compiler

#endif
