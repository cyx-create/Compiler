#ifndef COMPILER_REFLOW_HH
#define COMPILER_REFLOW_HH

#include <string>

namespace quad {
class QuadProgram;
}

bool writeQuadText(quad::QuadProgram *prog, const std::string &path, bool blocked);
bool reflowFromProgram(quad::QuadProgram *prog, const std::string &basePath);

#endif
