#!/usr/bin/env python3
"""Check or format DBG-C-owned C and C++ files under software/."""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
from pathlib import Path


SOFTWARE_ROOT = Path(__file__).resolve().parents[2]
SOURCE_SUFFIXES = {".c", ".h", ".cc", ".cpp", ".cxx", ".hh", ".hpp", ".hxx"}
EXCLUDED_PARTS = {".git", "build", "generated", "obj", "third_party"}
VENDOR_CH585_PATH = Path("poc1-ch585-threadx/platform/ch585/wch")


def is_project_source(path: Path) -> bool:
    """Return whether a path is a project-owned C/C++ source file."""
    try:
        relative = path.resolve().relative_to(SOFTWARE_ROOT)
    except ValueError:
        return False
    if path.suffix.lower() not in SOURCE_SUFFIXES:
        return False
    if any(part in EXCLUDED_PARTS for part in relative.parts):
        return False
    return not relative.is_relative_to(VENDOR_CH585_PATH)


def discover_sources() -> list[Path]:
    """Find project-owned C/C++ files without traversing excluded trees."""
    sources: list[Path] = []
    for current, directories, filenames in os.walk(SOFTWARE_ROOT):
        current_path = Path(current)
        relative = current_path.relative_to(SOFTWARE_ROOT)
        directories[:] = [
            name
            for name in directories
            if name not in EXCLUDED_PARTS
            and not (relative / name).is_relative_to(VENDOR_CH585_PATH)
        ]
        for filename in filenames:
            path = current_path / filename
            if is_project_source(path):
                sources.append(path)
    return sorted(sources)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true", help="检查格式，不修改文件")
    mode.add_argument("--write", action="store_true", help="使用 clang-format 修改文件")
    parser.add_argument("--all", action="store_true", help="处理 software/ 下项目自有 C/C++ 文件")
    parser.add_argument("files", nargs="*", type=Path, help="处理指定的项目自有文件")
    parser.add_argument("--clang-format", default="clang-format", help="clang-format 可执行文件")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.all == bool(args.files):
        print("必须且只能指定 --all 或一个以上文件路径", file=sys.stderr)
        return 2
    files = discover_sources() if args.all else [path.resolve() for path in args.files]
    invalid = [path for path in files if not is_project_source(path)]
    if invalid:
        for path in invalid:
            print(f"拒绝处理非项目自有 C/C++ 路径：{path}", file=sys.stderr)
        return 2
    if not files:
        print("没有找到可处理的 C/C++ 文件", file=sys.stderr)
        return 2

    command = [args.clang_format]
    command.extend(["--dry-run", "--Werror"] if args.check else ["-i"])
    command.extend(str(path) for path in files)
    try:
        result = subprocess.run(command, cwd=SOFTWARE_ROOT, check=False)
    except FileNotFoundError:
        print(f"找不到格式化工具：{args.clang_format}", file=sys.stderr)
        return 2
    if result.returncode:
        return result.returncode
    action = "格式检查通过" if args.check else "格式化完成"
    print(f"{action}：{len(files)} 个文件")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
