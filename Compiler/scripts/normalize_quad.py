#!/usr/bin/env python3
"""Normalize temp/label numbers so quad outputs can be compared structurally."""
import re
import sys

TEMP_RE = re.compile(r"\bt(\d+)\b")
LABEL_RE = re.compile(r"\bL(\d+)\b")
DEF_USE_RE = re.compile(r";\s*def:\s*(.*?)\s*use:\s*(.*?)\s*$")


def sort_temp_list(text: str) -> str:
    temps = TEMP_RE.findall(text)
    if not temps:
        return text.strip()
    temps_sorted = sorted(temps, key=int)
    return " ".join(f"t{n}" for n in temps_sorted)


def normalize_line(line: str, temp_map: dict, label_map: dict, temp_id: list, label_id: list) -> str:
    if DEF_USE_RE.search(line):
        prefix, def_part, use_part = DEF_USE_RE.split(line, maxsplit=2)[0:3]
        line = prefix.rstrip()
        def_part = sort_temp_list(def_part)
        use_part = sort_temp_list(use_part)
        line += f"; def: {def_part} use: {use_part} "
        if not line.endswith("\n"):
            line += ""

    def temp_sub(m):
        key = m.group(1)
        if key not in temp_map:
            temp_map[key] = temp_id[0]
            temp_id[0] += 1
        return f"t{temp_map[key]}"

    def label_sub(m):
        key = m.group(1)
        if key not in label_map:
            label_map[key] = label_id[0]
            label_id[0] += 1
        return f"L{label_map[key]}"

    line = TEMP_RE.sub(temp_sub, line)
    line = LABEL_RE.sub(label_sub, line)
    return line


def normalize(text: str) -> str:
    lines = []
    temp_map = {}
    label_map = {}
    temp_id = [0]
    label_id = [0]

    for line in text.splitlines():
        if line.startswith("Function "):
            temp_map = {}
            label_map = {}
            temp_id = [0]
            label_id = [0]
            name = line.split("(")[0]
            lines.append(f"{name}(...):")
            continue
        lines.append(normalize_line(line, temp_map, label_map, temp_id, label_id))
    return "\n".join(lines) + "\n"


if __name__ == "__main__":
    data = sys.stdin.read()
    sys.stdout.write(normalize(data))
