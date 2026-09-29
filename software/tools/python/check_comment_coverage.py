#!/usr/bin/env python3
"""Report Doxygen block-comment coverage for recognizable C/C++ functions.

This is a source-level heuristic, not a complete C/C++ parser. It recognizes
single-line function definitions and must not be used as the sole proof that
all functions in a translation unit are documented.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path


SOFTWARE_ROOT = Path(__file__).resolve().parents[2]
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx"}
EXCLUDED_PARTS = {".git", "build", "generated", "obj", "third_party"}
VENDOR_CH585_PATH = Path("poc1-ch585-threadx/platform/ch585/wch")
FUNCTION_RE = re.compile(
    r"^\s*(?:(?:static|inline|extern|const|volatile|__attribute__\s*\(.*?\))\s+)*"
    r"(?:[\w:]+(?:\s*<[^;{}]+>)?\s+)+[*&\s]*[\w:~]+\s*"
    r"\([^;{}]*\)\s*(?:const\s*)?(?:noexcept\s*)?\{"
)


def is_project_source(path: Path) -> bool:
    """Return whether a path is a project-owned C/C++ implementation file."""
    try:
        relative = path.resolve().relative_to(SOFTWARE_ROOT)
    except ValueError:
        return False
    return (
        path.suffix.lower() in SOURCE_SUFFIXES
        and not any(part in EXCLUDED_PARTS for part in relative.parts)
        and not relative.is_relative_to(VENDOR_CH585_PATH)
    )


def has_doxygen_block_before(lines: list[str], index: int) -> bool:
    """Check the immediately preceding comment block for a Doxygen opener."""
    cursor = index - 1
    while cursor >= 0 and not lines[cursor].strip():
        cursor -= 1
    while cursor >= 0 and (lines[cursor].lstrip().startswith("*") or "/**" in lines[cursor]):
        if "/**" in lines[cursor]:
            return True
        if "/*" in lines[cursor]:
            return False
        cursor -= 1
    return False


def inspect(path: Path) -> tuple[int, int, list[str]]:
    """Count recognizable definitions with and without a preceding Doxygen block."""
    lines = path.read_text(encoding="utf-8").splitlines()
    total = 0
    documented = 0
    missing: list[str] = []
    for index, line in enumerate(lines):
        if not FUNCTION_RE.match(line):
            continue
        total += 1
        if has_doxygen_block_before(lines, index):
            documented += 1
        else:
            missing.append(f"{path}:{index + 1}")
    return total, documented, missing


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("files", nargs="+", type=Path, help="要检查的项目自有 C/C++ 源文件")
    parser.add_argument("--threshold", type=float, default=100.0, help="覆盖率门槛，默认 100")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if not 0.0 <= args.threshold <= 100.0:
        print("--threshold 必须在 0 到 100 之间", file=sys.stderr)
        return 2
    files = [path.resolve() for path in args.files]
    invalid = [path for path in files if not is_project_source(path) or not path.is_file()]
    if invalid:
        for path in invalid:
            print(f"拒绝检查非项目自有 C/C++ 源码：{path}", file=sys.stderr)
        return 2

    total = 0
    documented = 0
    missing: list[str] = []
    for path in files:
        file_total, file_documented, file_missing = inspect(path)
        total += file_total
        documented += file_documented
        missing.extend(file_missing)
    if total == 0:
        print("未识别到单行函数定义；不能据此判定注释覆盖通过", file=sys.stderr)
        return 2

    rate = documented * 100.0 / total
    print(f"Doxygen 函数注释覆盖率：{rate:.1f}% ({documented}/{total})；门槛：{args.threshold:.1f}%")
    if missing:
        print("缺少相邻 Doxygen 块注释的函数定义：")
        for location in missing:
            print(f"- {location}")
    print("注意：这是启发式报告；多行函数声明和复杂宏/属性语法可能未被识别。")
    return 0 if rate >= args.threshold else 1


if __name__ == "__main__":
    raise SystemExit(main())
