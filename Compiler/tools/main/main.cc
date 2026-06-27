#include <iostream>
#include <string>

#include "pipeline.hh"

using namespace std;
using namespace compiler;

static OptMode parseMode(const string &arg) {
    if (arg == "base") return OptMode::Base;
    if (arg == "const") return OptMode::Const;
    if (arg == "loop1") return OptMode::Loop1;
    if (arg == "loop2") return OptMode::Loop2;
    if (arg == "allloop") return OptMode::AllLoop;
    if (arg == "allopt") return OptMode::AllOpt;
    return OptMode::Base;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <testdir> <basename> [mode|all]" << endl;
        return EXIT_FAILURE;
    }

    string testDir = argv[1];
    string baseName = argv[2];
    string modeArg = argc >= 4 ? argv[3] : "all";

    if (modeArg == "all") {
        return compileAllModes(testDir, baseName) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    if (modeArg == "quad-only") {
        return compileQuadOnly(testDir, baseName) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    return compileProgram(testDir, baseName, parseMode(modeArg)) ? EXIT_SUCCESS : EXIT_FAILURE;
}
