#!/usr/bin/env python3
"""Run compiled .colored.s tests (make run / make run-*)."""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

# Allow importing verify_e2e helpers from the same directory.
sys.path.insert(0, str(Path(__file__).resolve().parent))
from verify_e2e import (  # noqa: E402
    default_cases_for_mode,
    eval_case_result,
    link_and_run,
    load_manifest,
)


def main() -> int:
    parser = argparse.ArgumentParser(description="Run Compiler test programs under QEMU")
    parser.add_argument("mode", nargs="?", default="base", help="optimization mode suffix")
    parser.add_argument(
        "--tier",
        choices=["all", "backend", "quad", "ir"],
        default="all",
        help="only run programs in this manifest tier (default: all)",
    )
    args = parser.parse_args()

    compiler_dir = Path(__file__).resolve().parent.parent
    test_dir = compiler_dir / "test"
    manifest = load_manifest(compiler_dir / "scripts" / "test_manifest.json")
    libsysy = compiler_dir / "vendor" / "libsysy" / "libsysy32.s"
    gcc = "arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -static -Wl,-z,noexecstack -lm"
    mode = args.mode

    cases = default_cases_for_mode(manifest, mode)
    if args.tier != "all":
        allowed = {
            name
            for name, meta in manifest.get("programs", {}).items()
            if meta.get("tier") == args.tier
        }
        cases = [c for c in cases if c.get("program") in allowed]
    case_by_prog = {c["program"]: c for c in cases}

    asm_files = sorted(test_dir.glob(f"*.{mode}.colored.s"))
    if args.tier != "all":
        allowed = {
            name
            for name, meta in manifest.get("programs", {}).items()
            if meta.get("tier") == args.tier
        }
        asm_files = [
            p
            for p in asm_files
            if p.name[: -len(f".{mode}.colored.s")] in allowed
        ]
    if not asm_files:
        print(f"Missing test/*.{mode}.colored.s — run make compile first", file=sys.stderr)
        return 1

    failed = False
    for asm in asm_files:
        prog = asm.name[: -len(f".{mode}.colored.s")]
        case = case_by_prog.get(prog)
        print(f"==== {prog} ({mode}) ====", flush=True)

        if case is None:
            ec, out = link_and_run(gcc, libsysy, asm, stdin="", timeout_sec=4)
            sys.stdout.write(out)
            if ec in (132, 134, 139):
                print(f"make run: {prog} crashed unexpectedly (ec={ec})", file=sys.stderr)
                failed = True
            continue

        stdin = case.get("stdin", "")
        timeout_sec = float(case.get("timeout_sec", 4))
        ec, out = link_and_run(gcc, libsysy, asm, stdin=stdin, timeout_sec=timeout_sec)
        sys.stdout.write(out)
        result = eval_case_result(case, ec, out)
        if not result.ok():
            print(f"make run: {prog} failed — {result.detail}", file=sys.stderr)
            failed = True

    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
