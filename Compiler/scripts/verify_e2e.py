#!/usr/bin/env python3
"""
Full end-to-end verification for Compiler/:
  1) structural (verify-quads / verify-ir)
  2) compile fdmjc → .colored.s
  3) assembly sanity checks
  4) cross-link with arm-linux-gnueabihf-gcc
  5) qemu-arm runtime vs expected stdout
  6) optimization-mode output consistency (backend)
  7) stress / boundary / timeout cases
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Optional

CRASH_EXIT = {132, 134, 139, -11, -6, 255, 251}
TIMEOUT_EXIT = 124


@dataclass
class Result:
    case_id: str
    status: str  # PASS, FAIL, SKIP
    phase: str
    detail: str = ""

    def ok(self) -> bool:
        return self.status == "PASS"


@dataclass
class Stats:
    passed: int = 0
    failed: int = 0
    skipped: int = 0
    results: list[Result] = field(default_factory=list)

    def add(self, r: Result) -> None:
        self.results.append(r)
        if r.status == "PASS":
            self.passed += 1
        elif r.status == "FAIL":
            self.failed += 1
        else:
            self.skipped += 1


def load_manifest(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def run_cmd(cmd: list[str], *, cwd: Path, stdin: bytes = b"", timeout: float = 120) -> subprocess.CompletedProcess:
    return subprocess.run(
        cmd,
        cwd=cwd,
        input=stdin,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=timeout,
    )


def normalize_stdout(raw: bytes) -> str:
    return raw.decode("utf-8", errors="replace").replace("\r", "")


def check_asm(path: Path, patterns: list[str]) -> tuple[bool, str]:
    if not path.is_file():
        return False, "missing"
    text = path.read_text(encoding="utf-8", errors="replace")
    if len(text.strip()) < 40:
        return False, "file too small or empty"
    combined = "|".join(patterns)
    if not re.search(combined, text, re.IGNORECASE | re.MULTILINE):
        return False, f"no ARM marker matched ({combined})"
    if not re.search(r"(main|_\$main|GLOBAL|\.global)", text, re.IGNORECASE):
        return False, "no main/global symbol"
    return True, "ok"


def compile_program(
    fdmjc: Path,
    test_dir: Path,
    name: str,
    mode: str,
    stats: Stats,
    *,
    force: bool,
) -> bool:
    asm = test_dir / f"{name}.{mode}.colored.s"
    if asm.is_file() and not force:
        return True
    case_id = f"compile.{name}.{mode}"
    try:
        proc = run_cmd([str(fdmjc), str(test_dir), name, mode], cwd=test_dir.parent, timeout=180)
    except subprocess.TimeoutExpired:
        stats.add(Result(case_id, "FAIL", "compile", "fdmjc timeout"))
        return False
    if proc.returncode != 0:
        err = proc.stderr.decode("utf-8", errors="replace").strip()[:400]
        stats.add(Result(case_id, "FAIL", "compile", err or f"exit {proc.returncode}"))
        return False
    if not asm.is_file():
        stats.add(Result(case_id, "FAIL", "compile", f"missing {asm.name}"))
        return False
    stats.add(Result(case_id, "PASS", "compile", asm.name))
    return True


def link_and_run(
    gcc: str,
    libsysy: Path,
    asm: Path,
    *,
    stdin: str,
    timeout_sec: float,
) -> tuple[int, str]:
    bin_path = asm.parent / f".verify_tmp_{asm.stem}"
    try:
        link = subprocess.run(
            gcc.split() + ["-o", str(bin_path), str(asm), str(libsysy)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=60,
        )
        if link.returncode != 0:
            err = link.stderr.decode("utf-8", errors="replace")[:300]
            return -1000, f"LINK_FAIL: {err}"

        run = subprocess.run(
            ["qemu-arm", str(bin_path)],
            input=stdin.encode("utf-8"),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=timeout_sec,
        )
        return run.returncode, normalize_stdout(run.stdout)
    except subprocess.TimeoutExpired:
        return TIMEOUT_EXIT, ""
    finally:
        if bin_path.is_file():
            bin_path.unlink(missing_ok=True)


def compare_stdout(got: str, expected: str, mode: str) -> bool:
    if mode == "exact":
        return got == expected
    if mode == "strip_nl":
        return got.rstrip("\n") == expected.rstrip("\n")
    if mode == "prefix":
        return got.rstrip("\n").startswith(expected.rstrip("\n"))
    return got == expected


def eval_case_result(
    case: dict[str, Any],
    ec: int,
    out: str,
) -> Result:
    cid = case["id"]
    expect = case.get("expect", "run")
    if expect == "timeout":
        if ec == TIMEOUT_EXIT:
            return Result(cid, "PASS", "run", "timeout as expected")
        return Result(cid, "FAIL", "run", f"expected timeout, ec={ec} out=[{out!r}]")

    if expect == "crash":
        if ec in CRASH_EXIT or ec not in (0, 1):
            return Result(cid, "PASS", "run", f"crash/abort ec={ec}")
        return Result(cid, "FAIL", "run", f"expected crash, ec={ec} out=[{out!r}]")

    if expect == "crash_after":
        prefix = case.get("stdout_prefix", "")
        if out.startswith(prefix) and (ec in CRASH_EXIT or ec not in (0, 1)):
            return Result(cid, "PASS", "run", f"prefix ok then crash ec={ec}")
        return Result(cid, "FAIL", "run", f"expected prefix {prefix!r} then crash; ec={ec} out=[{out!r}]")

    expected = case.get("stdout", "")
    stdout_mode = case.get("stdout_mode", "strip_nl")
    if not compare_stdout(out, expected, stdout_mode):
        return Result(
            cid,
            "FAIL",
            "run",
            f"stdout mismatch ec={ec}\n  expected=[{expected!r}]\n  got=[{out!r}]",
        )

    allow_exit = case.get("allow_exit")
    if allow_exit is not None:
        if ec in allow_exit:
            return Result(cid, "PASS", "run", f"ec={ec} allowed")
        return Result(cid, "FAIL", "run", f"stdout ok but exit {ec} not in {allow_exit}")

    if ec in CRASH_EXIT:
        return Result(cid, "FAIL", "run", f"unexpected crash ec={ec} out=[{out!r}]")

    return Result(cid, "PASS", "run", f"ec={ec}")


def eval_case(
    case: dict[str, Any],
    manifest: dict[str, Any],
    test_dir: Path,
    gcc: str,
    libsysy: Path,
) -> Result:
    cid = case["id"]
    prog = case["program"]
    mode = case.get("mode", "base")
    asm = test_dir / f"{prog}.{mode}.colored.s"
    if not asm.is_file():
        return Result(cid, "FAIL", "artifact", f"missing {asm.name}")

    ok, msg = check_asm(asm, manifest.get("asm_required_patterns", []))
    if not ok:
        return Result(cid, "FAIL", "asm_check", msg)

    stdin = case.get("stdin", "")
    timeout_sec = float(case.get("timeout_sec", 4))
    ec, out = link_and_run(gcc, libsysy, asm, stdin=stdin, timeout_sec=timeout_sec)
    return eval_case_result(case, ec, out)


def default_cases_for_mode(manifest: dict[str, Any], mode: str) -> list[dict[str, Any]]:
    """First manifest case per program for the given optimization mode."""
    seen: set[str] = set()
    out: list[dict[str, Any]] = []
    for case in manifest.get("cases", []):
        if case.get("mode", "base") != mode:
            continue
        prog = case.get("program", "")
        if not prog or prog in seen:
            continue
        seen.add(prog)
        out.append(case)
    return out


def run_structural(scripts: Path, stats: Stats, *, skip: bool) -> None:
    if skip:
        stats.add(Result("structural", "SKIP", "structural", "skipped by flag"))
        return
    for name in ("verify_quads.sh", "verify_ir.sh"):
        script = scripts / name
        if not script.is_file():
            stats.add(Result(name, "FAIL", "structural", "script missing"))
            continue
        proc = subprocess.run(["bash", str(script)], cwd=scripts.parent, capture_output=True, text=True)
        if proc.returncode == 0:
            stats.add(Result(name, "PASS", "structural", "ok"))
        else:
            tail = (proc.stdout + proc.stderr)[-600:]
            stats.add(Result(name, "FAIL", "structural", tail))


def run_opt_consistency(
    manifest: dict[str, Any],
    test_dir: Path,
    gcc: str,
    libsysy: Path,
    stats: Stats,
    modes: list[str],
) -> None:
    for item in manifest.get("opt_consistency", []):
        prog = item["program"]
        stdin = item.get("stdin", "")
        ref_mode = item.get("ref_mode", "base")
        ref_asm = test_dir / f"{prog}.{ref_mode}.colored.s"
        if not ref_asm.is_file():
            stats.add(Result(f"optconsist.{prog}", "FAIL", "opt_consistency", "ref asm missing"))
            continue
        _, ref_out = link_and_run(gcc, libsysy, ref_asm, stdin=stdin, timeout_sec=4)
        for mode in modes:
            if mode == ref_mode:
                continue
            cid = f"optconsist.{prog}.{mode}"
            asm = test_dir / f"{prog}.{mode}.colored.s"
            if not asm.is_file():
                stats.add(Result(cid, "FAIL", "opt_consistency", "asm missing"))
                continue
            _, out = link_and_run(gcc, libsysy, asm, stdin=stdin, timeout_sec=4)
            if compare_stdout(out, ref_out, "strip_nl"):
                stats.add(Result(cid, "PASS", "opt_consistency", "matches base"))
            else:
                stats.add(
                    Result(
                        cid,
                        "FAIL",
                        "opt_consistency",
                        f"base=[{ref_out!r}] {mode}=[{out!r}]",
                    )
                )


def run_stress(
    manifest: dict[str, Any],
    test_dir: Path,
    gcc: str,
    libsysy: Path,
    stats: Stats,
) -> None:
    for item in manifest.get("stress", []):
        prog = item["program"]
        mode = item.get("mode", "base")
        asm = test_dir / f"{prog}.{mode}.colored.s"
        cid = f"stress.{prog}.{mode}x{item.get('rounds', 3)}"
        if not asm.is_file():
            stats.add(Result(cid, "FAIL", "stress", "asm missing"))
            continue
        expected = item.get("stdout", "")
        rounds = int(item.get("rounds", 3))
        stdin = item.get("stdin", "")
        for i in range(1, rounds + 1):
            ec, out = link_and_run(gcc, libsysy, asm, stdin=stdin, timeout_sec=4)
            if ec == TIMEOUT_EXIT or ec in CRASH_EXIT or not compare_stdout(out, expected, "strip_nl"):
                stats.add(
                    Result(
                        cid,
                        "FAIL",
                        "stress",
                        f"round {i}/{rounds} ec={ec} out=[{out!r}]",
                    )
                )
                break
        else:
            stats.add(Result(cid, "PASS", "stress", f"{rounds} rounds ok"))


def filter_cases(manifest: dict[str, Any], tier: str) -> list[dict[str, Any]]:
    programs = manifest.get("programs", {})
    cases = manifest.get("cases", [])
    if tier == "all":
        return cases
    allowed = {n for n, meta in programs.items() if meta.get("tier") == tier}
    return [c for c in cases if c.get("program") in allowed]


def main() -> int:
    parser = argparse.ArgumentParser(description="Compiler full E2E verification")
    parser.add_argument("--compile", action="store_true", help="force recompile via fdmjc")
    parser.add_argument("--compile-missing", action="store_true", default=True,
                        help="compile when .colored.s missing (default)")
    parser.add_argument("--no-compile-missing", dest="compile_missing", action="store_false")
    parser.add_argument("--skip-structural", action="store_true")
    parser.add_argument("--tier", choices=["all", "backend", "quad", "ir"], default="all")
    parser.add_argument("--modes", default="base", help="comma-separated opt modes to compile")
    parser.add_argument("--with-opt-consistency", action="store_true",
                        help="compare outputs across all opt modes (backend)")
    parser.add_argument("--with-stress", action="store_true")
    parser.add_argument("--quick", action="store_true",
                        help="backend tier only, base mode, skip opt/stress")
    args = parser.parse_args()

    compiler_dir = Path(__file__).resolve().parent.parent
    scripts_dir = compiler_dir / "scripts"
    test_dir = compiler_dir / "test"
    manifest_path = scripts_dir / "test_manifest.json"
    fdmjc = compiler_dir / "build" / "tools" / "main" / "fdmjc"
    libsysy = compiler_dir / "vendor" / "libsysy" / "libsysy32.s"
    gcc = "arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -static -Wl,-z,noexecstack -lm"

    if args.quick:
        args.tier = "backend"
        args.modes = "base"
        args.with_opt_consistency = False
        args.with_stress = False

    if not fdmjc.is_file():
        print("ERROR: fdmjc not built — run: make build", file=sys.stderr)
        return 2

    manifest = load_manifest(manifest_path)
    stats = Stats()
    modes = [m.strip() for m in args.modes.split(",") if m.strip()]
    tier = args.tier

    # setup fixtures
    setup = scripts_dir / "setup_test.sh"
    if setup.is_file():
        subprocess.run(["bash", str(setup)], cwd=compiler_dir, check=False, capture_output=True)

    print("========== Phase 1: Structural verification ==========")
    run_structural(scripts_dir, stats, skip=args.skip_structural)

    print("\n========== Phase 2: Compile → .colored.s ==========")
    programs_meta = manifest.get("programs", {})
    if tier != "all":
        program_names = sorted(n for n, m in programs_meta.items() if m.get("tier") == tier)
    else:
        program_names = sorted(programs_meta.keys())

    force_compile = args.compile
    for name in program_names:
        fmj = test_dir / f"{name}.fmj"
        if not fmj.is_file():
            stats.add(Result(f"fixture.{name}", "SKIP", "fixture", "no .fmj"))
            continue
        for mode in modes:
            asm = test_dir / f"{name}.{mode}.colored.s"
            need = force_compile or (args.compile_missing and not asm.is_file())
            compile_program(fdmjc, test_dir, name, mode, stats, force=need)

    print("\n========== Phase 3: Runtime / logic verification ==========")
    cases = filter_cases(manifest, tier)
    for case in cases:
        stats.add(eval_case(case, manifest, test_dir, gcc, libsysy))

    if args.with_opt_consistency and tier in ("all", "backend"):
        print("\n========== Phase 4: Optimization output consistency ==========")
        all_modes = manifest.get("opt_modes", ["base"])
        run_opt_consistency(manifest, test_dir, gcc, libsysy, stats, all_modes)

    if args.with_stress and tier in ("all", "backend"):
        print("\n========== Phase 5: Stress tests ==========")
        run_stress(manifest, test_dir, gcc, libsysy, stats)

    print("\n========== SUMMARY ==========")
    fails = [r for r in stats.results if r.status == "FAIL"]
    for r in stats.results:
        if r.status != "PASS":
            print(f"  [{r.status}] {r.phase}:{r.case_id} — {r.detail[:120]}")

    print(
        f"\nPASS={stats.passed} FAIL={stats.failed} SKIP={stats.skipped} "
        f"TOTAL={stats.passed + stats.failed + stats.skipped}"
    )
    by_phase: dict[str, list[Result]] = {}
    for r in stats.results:
        by_phase.setdefault(r.phase, []).append(r)
    print("\nBy phase:")
    for phase, rs in sorted(by_phase.items()):
        p = sum(1 for x in rs if x.status == "PASS")
        f = sum(1 for x in rs if x.status == "FAIL")
        print(f"  {phase}: {p} pass, {f} fail")

    if stats.failed == 0:
        print("\nVERDICT: ALL_PASSED")
        return 0
    print(f"\nVERDICT: {stats.failed} FAILURE(S)")
    return 1


if __name__ == "__main__":
    sys.exit(main())
