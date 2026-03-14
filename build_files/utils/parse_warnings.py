#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 Blended Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Parse compiler warning output into a structured, actionable report.

Usage:
    # Capture warnings during build:
    make 2>&1 | tee build_warnings.log

    # Parse and summarize:
    python3 build_files/utils/parse_warnings.py build_warnings.log

    # Filter to specific subsystem:
    python3 build_files/utils/parse_warnings.py build_warnings.log --filter gpu

    # Output as CSV for spreadsheet triage:
    python3 build_files/utils/parse_warnings.py build_warnings.log --csv
"""

import argparse
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

# Matches clang/gcc warning lines:
#   /path/to/file.cc:123:45: warning: some message [-Wflag]
WARNING_RE = re.compile(
    r"^(?P<file>[^\s:]+):(?P<line>\d+):(?P<col>\d+): warning: "
    r"(?P<message>.+?)(?:\s+\[(?P<flag>-W[^\]]+)\])?\s*$"
)

# Matches emcc/emscripten warning lines:
#   emcc: warning: some message [-Wflag]
EMCC_RE = re.compile(
    r"^em(?:cc|c\+\+): warning: (?P<message>.+?)(?:\s+\[(?P<flag>-W[^\]]+)\])?\s*$"
)


def parse_warnings(log_path):
    """Parse a build log and return a list of warning dicts."""
    warnings = []
    with open(log_path, "r", errors="replace") as f:
        for raw_line in f:
            line = raw_line.strip()
            m = WARNING_RE.match(line)
            if m:
                warnings.append({
                    "file": m.group("file"),
                    "line": int(m.group("line")),
                    "col": int(m.group("col")),
                    "message": m.group("message"),
                    "flag": m.group("flag") or "unknown",
                    "raw": line,
                })
                continue
            m = EMCC_RE.match(line)
            if m:
                warnings.append({
                    "file": "(emcc)",
                    "line": 0,
                    "col": 0,
                    "message": m.group("message"),
                    "flag": m.group("flag") or "emcc",
                    "raw": line,
                })
    return warnings


def subsystem_from_path(filepath):
    """Map a source file path to a subsystem name for grouping."""
    p = filepath.replace("\\", "/")
    parts = p.split("/")

    # Find 'source/blender/XXX' pattern
    try:
        idx = parts.index("blender")
        if idx + 1 < len(parts):
            return parts[idx + 1]
    except ValueError:
        pass

    # Find 'intern/XXX' or 'extern/XXX'
    for prefix in ("intern", "extern"):
        try:
            idx = parts.index(prefix)
            if idx + 1 < len(parts):
                return f"{prefix}/{parts[idx + 1]}"
        except ValueError:
            pass

    return "other"


def print_summary(warnings, filter_subsystem=None):
    """Print a human-readable summary of warnings."""
    if filter_subsystem:
        warnings = [
            w for w in warnings
            if filter_subsystem.lower() in subsystem_from_path(w["file"]).lower()
        ]

    if not warnings:
        print("No warnings found.")
        return

    # Group by flag
    by_flag = Counter(w["flag"] for w in warnings)
    # Group by subsystem
    by_subsystem = Counter(subsystem_from_path(w["file"]) for w in warnings)
    # Group by file
    by_file = Counter(w["file"] for w in warnings)
    # Unique warnings (deduplicated by file+line+flag)
    unique = {(w["file"], w["line"], w["flag"]) for w in warnings}

    print(f"{'='*70}")
    print(f"  WARNINGS REPORT")
    print(f"  Total: {len(warnings)}  |  Unique (file:line:flag): {len(unique)}")
    print(f"{'='*70}")

    print(f"\n  BY WARNING TYPE ({len(by_flag)} categories):")
    print(f"  {'-'*50}")
    for flag, count in by_flag.most_common(25):
        bar = "#" * min(count // 5, 40)
        print(f"  {count:6d}  {flag:<35s} {bar}")

    print(f"\n  BY SUBSYSTEM ({len(by_subsystem)} subsystems):")
    print(f"  {'-'*50}")
    for sub, count in by_subsystem.most_common(25):
        bar = "#" * min(count // 5, 40)
        print(f"  {count:6d}  {sub:<35s} {bar}")

    print(f"\n  TOP 20 FILES:")
    print(f"  {'-'*50}")
    for filepath, count in by_file.most_common(20):
        # Shorten path for display
        short = filepath
        if len(short) > 55:
            short = "..." + short[-52:]
        print(f"  {count:6d}  {short}")

    print(f"\n{'='*70}")
    print(f"  Fix priority: address top warning types in top subsystems first.")
    print(f"  Re-run after fixes to verify reduction.")
    print(f"{'='*70}")


def print_csv(warnings):
    """Print warnings as CSV for spreadsheet import."""
    print("file,line,col,flag,subsystem,message")
    for w in warnings:
        sub = subsystem_from_path(w["file"])
        msg = w["message"].replace('"', '""')
        print(f'"{w["file"]}",{w["line"]},{w["col"]},"{w["flag"]}","{sub}","{msg}"')


def main():
    parser = argparse.ArgumentParser(description="Parse build warnings into a triage report.")
    parser.add_argument("logfile", help="Build log file (from make 2>&1 | tee log)")
    parser.add_argument("--filter", help="Filter to subsystem (e.g. 'gpu', 'draw', 'blenkernel')")
    parser.add_argument("--csv", action="store_true", help="Output as CSV")
    parser.add_argument("--list", action="store_true", help="List all warnings line-by-line")
    args = parser.parse_args()

    warnings = parse_warnings(args.logfile)

    if args.csv:
        print_csv(warnings)
    elif args.list:
        for w in warnings:
            print(w["raw"])
    else:
        print_summary(warnings, filter_subsystem=args.filter)


if __name__ == "__main__":
    main()
