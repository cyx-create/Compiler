#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DST="$(cd "$(dirname "$0")/.." && pwd)"

# Drop previously synced sources (keep CMakeLists.txt, pipeline/, tree2quad, ast2tree, schedule).
for d in ast ir quad quadflow opt instr reg util; do
    if [ "$d" = quad ]; then
        find "$DST/lib/$d" -maxdepth 1 -type f -name '*.cc' ! -name 'tree2quad.cc' -delete 2>/dev/null || true
        find "$DST/include/$d" -maxdepth 1 -type f -name '*.hh' ! -name 'tree2quad.hh' -delete 2>/dev/null || true
    elif [ "$d" = ir ]; then
        find "$DST/lib/$d" -maxdepth 1 -type f -name '*.cc' ! -name 'ast2tree.cc' -delete 2>/dev/null || true
        find "$DST/include/$d" -maxdepth 1 -type f -name '*.hh' -delete 2>/dev/null || true
    elif [ "$d" = instr ]; then
        find "$DST/lib/$d" -maxdepth 1 -type f -name '*.cc' ! -name 'schedule.cc' ! -name 'selectInstr.cc' ! -name 'asmprog2colored.cc' -delete 2>/dev/null || true
        find "$DST/include/$d" -maxdepth 1 -type f -name '*.hh' -delete 2>/dev/null || true
    elif [ "$d" = ast ]; then
        find "$DST/lib/$d" -maxdepth 1 -type f -name '*.cc' ! -name 'semantanlyzer.cc' ! -name 'setnamemaps.cc' ! -name 'namemaps.cc' -delete 2>/dev/null || true
        find "$DST/include/$d" -maxdepth 1 -type f -name '*.hh' ! -name 'semant.hh' ! -name 'namemaps.hh' -delete 2>/dev/null || true
    elif [ "$d" = quadflow ]; then
        find "$DST/lib/$d" -maxdepth 1 -type f -name '*.cc' ! -name 'controlflowinfo.cc' ! -name 'flowinfo.cc' ! -name 'dataflowinfo.cc' -delete 2>/dev/null || true
        find "$DST/include/$d" -maxdepth 1 -type f -name '*.hh' -delete 2>/dev/null || true
    else
        find "$DST/lib/$d" -maxdepth 1 -type f \( -name '*.cc' -o -name '*.hh' \) -delete 2>/dev/null || true
        find "$DST/include/$d" -maxdepth 1 -type f -name '*.hh' -delete 2>/dev/null || true
    fi
done
mkdir -p "$DST/lib/frontend" "$DST/include/frontend" "$DST/lib" "$DST/include"

copy() {
    local src="$1"
    local rel="$2"
    mkdir -p "$(dirname "$DST/$rel")"
    cp "$ROOT/$src" "$DST/$rel"
}

# frontend — teacher-provided PARSING parser (FDMJ 2026)
copy PARSING/lib/frontend/lexer.ll lib/frontend/lexer.ll
copy PARSING/lib/frontend/parser.yy lib/frontend/parser.yy
copy PARSING/lib/frontend/CMakeLists.txt lib/frontend/CMakeLists.txt
copy PARSING/include/frontend/ASTLexer.hh include/frontend/ASTLexer.hh
copy PARSING/include/frontend/ast_location.hh include/frontend/ast_location.hh

# ast (semant/namemaps kept in Compiler for parse+semant pipeline)
copy HW3/lib/ast/FDMJAST.cc lib/ast/FDMJAST.cc
# copy HW3/lib/ast/namemaps.cc lib/ast/namemaps.cc
# copy HW3/include/ast/semant.hh include/ast/semant.hh
# copy HW3/include/ast/namemaps.hh include/ast/namemaps.hh
copy HW3/include/ast/FDMJAST.hh include/ast/FDMJAST.hh
copy HW3/include/ast/ASTheader.hh include/ast/ASTheader.hh

# ir (ast2tree.cc kept in Compiler for implicit-return handling)
# copy HW3/lib/ir/ast2tree.cc lib/ir/ast2tree.cc
copy HW3/lib/ir/tr_exp.cc lib/ir/tr_exp.cc
copy HW5/lib/ir/canon.cc lib/ir/canon.cc
copy HW5/lib/ir/treep.cc lib/ir/treep.cc
copy HW3/include/ir/ast2tree.hh include/ir/ast2tree.hh
copy HW3/include/ir/tr_exp.hh include/ir/tr_exp.hh
copy HW5/include/ir/canon.hh include/ir/canon.hh
copy HW5/include/ir/treep.hh include/ir/treep.hh
copy HW12/include/ir/temp.hh include/ir/temp.hh

# quad (tree2quad is maintained in Compiler/, not synced from HW5 stub)
copy HW11/lib/quad/quad.cc lib/quad/quad.cc
copy HW6/lib/quad/blocking.cc lib/quad/blocking.cc
copy HW7/lib/quad/quadssa.cc lib/quad/quadssa.cc
copy HW7/lib/quad/quadssa_diag.cc lib/quad/quadssa_diag.cc
copy HW11/include/quad/quad.hh include/quad/quad.hh
copy HW6/include/quad/blocking.hh include/quad/blocking.hh
copy HW7/include/quad/quadssa.hh include/quad/quadssa.hh
copy HW7/include/quad/quadssa_diag.hh include/quad/quadssa_diag.hh
# tree2quad.{cc,hh} — kept in Compiler repo; do not overwrite from HW5

# quadflow (controlflow/dataflow/flowinfo kept in Compiler; HW7 stubs not synced)
# copy HW11/lib/quadflow/controlflowinfo.cc lib/quadflow/controlflowinfo.cc
# copy HW7/lib/quadflow/dataflowinfo.cc lib/quadflow/dataflowinfo.cc
# copy HW11/lib/quadflow/flowinfo.cc lib/quadflow/flowinfo.cc
copy HW11/include/quadflow/flowinfo.hh include/quadflow/flowinfo.hh

# opt
copy HW8/lib/opt/opt.cc lib/opt/opt.cc
copy HW9/lib/opt/findloopheader.cc lib/opt/findloopheader.cc
copy HW9/lib/opt/loophoistfunc.cc lib/opt/loophoistfunc.cc
copy HW10/lib/opt/loopheaderwithflow.cc lib/opt/loopheaderwithflow.cc
copy HW10/lib/opt/loopinductionelimination.cc lib/opt/loopinductionelimination.cc
copy HW10/include/opt/loopheader.hh include/opt/loopheader.hh
for f in defusechain loopinductionbasic loopinductionderived \
    loopinductionopt loopoptanalysis loopoptreport loopstrengthreduction; do
    copy "HW10/lib/opt/${f}.cc" "lib/opt/${f}.cc"
    copy "HW10/include/opt/${f}.hh" "include/opt/${f}.hh"
done
copy HW8/include/opt/opt.hh include/opt/opt.hh
copy HW9/include/opt/loopopt.hh include/opt/loopopt.hh

# instr (schedule.cc kept in Compiler for implicit-return epilogue)
# copy HW11/lib/instr/schedule.cc lib/instr/schedule.cc
for f in advDFG preSchedule; do
    copy "HW11/lib/instr/${f}.cc" "lib/instr/${f}.cc"
    copy "HW11/include/instr/${f}.hh" "include/instr/${f}.hh"
done
for f in advDFG preSchedule schedule; do
    copy "HW11/include/instr/${f}.hh" "include/instr/${f}.hh"
done
for f in buildAdvDFG buildPreSchedule fileSchedule preSchedule; do
    copy "HW11/lib/instr/${f}.cc" "lib/instr/${f}.cc"
done
# selectInstr.cc kept in Compiler for IO call ordering during instruction selection
# copy HW11/lib/instr/selectInstr.cc lib/instr/selectInstr.cc
copy HW11/include/instr/instrSelection.hh include/instr/instrSelection.hh
copy HW11/include/instr/assem.hh include/instr/assem.hh
for f in asmdataflow asmprog; do
    copy "HW12/lib/instr/${f}.cc" "lib/instr/${f}.cc"
    copy "HW12/include/instr/${f}.hh" "include/instr/${f}.hh"
done
# asmprog2colored.cc kept in Compiler for spill scratch register fix (r11/r12)
# copy HW12/lib/instr/asmprog2colored.cc lib/instr/asmprog2colored.cc
copy HW12/lib/instr/predataflowpass.cc lib/instr/predataflowpass.cc
copy HW12/include/instr/asmprogpass.hh include/instr/asmprogpass.hh

# reg
copy HW12/lib/reg/buildIg.cc lib/reg/buildIg.cc
copy HW12/lib/reg/simcoafrespisel.cc lib/reg/simcoafrespisel.cc
for f in coloring ig; do
    copy "HW12/lib/reg/${f}.cc" "lib/reg/${f}.cc"
    copy "HW12/include/reg/${f}.hh" "include/reg/${f}.hh"
done

# util
copy HW3/lib/util/ast2xml.cc lib/util/ast2xml.cc
copy HW3/lib/util/xml2ast.cc lib/util/xml2ast.cc
copy HW5/lib/util/tree2xml.cc lib/util/tree2xml.cc
copy HW5/lib/util/xml2tree.cc lib/util/xml2tree.cc
copy HW11/lib/util/quad2xml.cc lib/util/quad2xml.cc
copy HW11/lib/util/xml2quad.cc lib/util/xml2quad.cc
copy HW11/lib/util/xml2flow.cc lib/util/xml2flow.cc
copy HW7/lib/util/flow2xml.cc lib/util/flow2xml.cc
copy HW11/lib/util/print_config.cc lib/util/print_config.cc
copy HW12/lib/util/xml2asmprog.cc lib/util/xml2asmprog.cc
copy HW3/include/util/ast2xml.hh include/util/ast2xml.hh
copy HW3/include/util/xml2ast.hh include/util/xml2ast.hh
copy HW5/include/util/tree2xml.hh include/util/tree2xml.hh
copy HW5/include/util/xml2tree.hh include/util/xml2tree.hh
copy HW11/include/util/quad2xml.hh include/util/quad2xml.hh
copy HW11/include/util/xml2quad.hh include/util/xml2quad.hh
copy HW11/include/util/xml2flow.hh include/util/xml2flow.hh
copy HW7/include/util/flow2xml.hh include/util/flow2xml.hh
copy HW11/include/util/flowinfo.hh include/util/flowinfo.hh
copy HW11/include/util/config.hh include/util/config.hh
copy HW12/include/util/xml2asmprog.hh include/util/xml2asmprog.hh

# vendor
rm -rf "$DST/vendor/tinyxml2"
cp -r "$ROOT/HW12/vendor/tinyxml2" "$DST/vendor/tinyxml2"
# libsysy kept in Compiler for putint fflush (irtest18 crash_after)
# rm -rf "$DST/vendor/libsysy"
# cp -r "$ROOT/HW12/vendor/libsysy" "$DST/vendor/libsysy"

echo "Synced HW sources into $DST"
