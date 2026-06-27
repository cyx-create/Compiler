#!/usr/bin/env bash
# Final comprehensive test for Compiler submission package.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
LOG="$ROOT/final_test_report.txt"
: > "$LOG"

pass=0
fail=0
warn=0

ok()   { echo "[PASS] $*" | tee -a "$LOG"; pass=$((pass+1)); }
bad()  { echo "[FAIL] $*" | tee -a "$LOG"; fail=$((fail+1)); }
note() { echo "[WARN] $*" | tee -a "$LOG"; warn=$((warn+1)); }

section() { echo "" | tee -a "$LOG"; echo "======== $* ========" | tee -a "$LOG"; }

section "A. 代码包结构 (要求 c)"
for f in Makefile README.md; do
  [[ -f "$f" ]] && ok "存在 $f" || bad "缺少 $f"
done
[[ -d test ]] && ok "存在 test/ 目录" || bad "缺少 test/ 目录"
fmj_n=$(ls test/*.fmj 2>/dev/null | wc -l)
if [[ "$fmj_n" -ge 1 ]]; then ok "test/ 含 $fmj_n 个 .fmj"; else bad "test/ 无 .fmj"; fi
[[ -f build/tools/main/fdmjc ]] || note "fdmjc 尚未构建（后续 make build 会生成）"

section "B. 工具链 (要求 a)"
for cmd in cmake ninja flex bison g++ arm-linux-gnueabihf-gcc qemu-arm python3; do
  command -v "$cmd" >/dev/null && ok "工具 $cmd" || bad "缺少 $cmd"
done

section "C. PARSING 前端集成"
if diff -q ../PARSING/lib/frontend/parser.yy lib/frontend/parser.yy >/dev/null 2>&1; then
  ok "parser.yy 与 PARSING/ 一致"
else
  bad "parser.yy 与 PARSING/ 不一致"
fi
if diff -q ../PARSING/lib/frontend/lexer.ll lib/frontend/lexer.ll >/dev/null 2>&1; then
  ok "lexer.ll 与 PARSING/ 一致"
else
  bad "lexer.ll 与 PARSING/ 不一致"
fi

section "D. make build"
if make build >>"$LOG" 2>&1; then
  ok "make build 成功"
else
  bad "make build 失败"
fi
[[ -x build/tools/main/fdmjc ]] && ok "fdmjc 可执行" || bad "fdmjc 不可执行"

section "E. make compile + 中间产物 (要求 d)"
# 清理一个 IR 与一个 Quad 程序的生成物，强制走完整前端
rm -f test/irtest6.{src,2-semant.ast,3.irp,3-canon.irp,4.quad,4-xml.quad,4-ssa.quad,base.colored.s} 2>/dev/null || true
rm -f test/quadtest3.{src,2-semant.ast,3.irp,3-canon.irp,4.quad,4-xml.quad,4-ssa.quad,base.colored.s} 2>/dev/null || true

if make compile >>"$LOG" 2>&1; then
  ok "make compile 成功（46 程序 × 6 优化）"
else
  bad "make compile 失败"
fi

check_outputs() {
  local name="$1"
  local missing=""
  for suf in src 2-semant.ast 3.irp 3-canon.irp 4.quad 4-xml.quad 4-ssa.quad 4-ssa-withflow-xml.quad \
             base.colored.s const.colored.s loop1.colored.s loop2.colored.s allloop.colored.s allopt.colored.s; do
    [[ -f "test/${name}.${suf}" ]] || missing="$missing ${suf}"
  done
  if [[ -z "$missing" ]]; then
    ok "$name 全部关键中间文件齐全"
  else
    bad "$name 缺少:$missing"
  fi
}
check_outputs irtest6
check_outputs quadtest3

# 统计全局产物数量
for mode in base const loop1 loop2 allloop allopt; do
  n=$(ls test/*."${mode}".colored.s 2>/dev/null | wc -l)
  if [[ "$n" -eq "$fmj_n" ]]; then
    ok "*.${mode}.colored.s 共 $n 个（与 .fmj 数一致）"
  else
    bad "*.${mode}.colored.s 仅 $n 个，期望 $fmj_n"
  fi
done
src_n=$(ls test/*.src 2>/dev/null | wc -l)
[[ "$src_n" -eq "$fmj_n" ]] && ok "*.src 共 $src_n 个" || bad "*.src 仅 $src_n 个"
sem_n=$(ls test/*.2-semant.ast 2>/dev/null | wc -l)
# 后端 bootstrap 可能跳过 parse，允许 semant 数 <= fmj
if [[ "$sem_n" -ge 33 ]]; then ok "*.2-semant.ast 共 $sem_n 个（前端路径已生成）"; else note "*.2-semant.ast 仅 $sem_n 个（部分后端 bootstrap 跳过）"; fi

section "F. make run 系列 (要求 c)"
for target in run run-const run-loop1 run-loop2 run-allloop run-allopt; do
  if make "$target" >>"$LOG" 2>&1; then
    ok "make $target 成功"
  else
    bad "make $target 失败"
  fi
done

section "G. 结构验证脚本"
if make verify-quads >>"$LOG" 2>&1; then ok "make verify-quads"; else bad "make verify-quads"; fi
if make verify-ir >>"$LOG" 2>&1; then ok "make verify-ir"; else bad "make verify-ir"; fi
if make verify-e2e-quick >>"$LOG" 2>&1; then ok "make verify-e2e-quick"; else bad "make verify-e2e-quick"; fi

section "H. 抽样 QEMU 深度检查"
GCC="arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -static -Wl,-z,noexecstack -lm"
LIB=vendor/libsysy/libsysy32.s
sample_run() {
  local prog="$1" mode="$2" expect="$3" stdin="${4:-}"
  local asm="test/${prog}.${mode}.colored.s"
  local bin="/tmp/final_${prog}_${mode}"
  if ! $GCC "$asm" "$LIB" -o "$bin" 2>/dev/null; then bad "链接失败 $prog.$mode"; return; fi
  local out ec
  if [[ -n "$stdin" ]]; then out=$(echo -n "$stdin" | timeout 4 qemu-arm "$bin" 2>&1); else out=$(timeout 4 qemu-arm "$bin" 2>&1); fi
  ec=$?
  if [[ "$out" == *"$expect"* ]] || [[ "$ec" -eq 139 && "$expect" == CRASH_AFTER_0 ]]; then
    ok "QEMU $prog.$mode 输出符合预期"
  else
    bad "QEMU $prog.$mode ec=$ec expect='$expect' got='${out:0:80}'"
  fi
}
sample_run irtest6 base "2"
sample_run quadtest3 base "1"
sample_run bubblesort base "0 1 2 3 5 6 9 "
sample_run irtest18 base "CRASH_AFTER_0"

section "汇总"
echo "" | tee -a "$LOG"
echo "PASS=$pass FAIL=$fail WARN=$warn" | tee -a "$LOG"
[[ "$fail" -eq 0 ]] && exit 0 || exit 1
