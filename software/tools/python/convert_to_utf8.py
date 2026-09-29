#!/usr/bin/env python3
"""Convert one project-owned text file to UTF-8 using an explicit source encoding."""

from __future__ import annotations

import argparse
import hashlib
import os
import sys
import tempfile
from pathlib import Path


SOFTWARE_ROOT = Path(__file__).resolve().parents[2]
EXCLUDED_PARTS = {".git", "build", "generated", "obj", "third_party"}
TEXT_SUFFIXES = {
    ".c", ".h", ".cc", ".cpp", ".cxx", ".hh", ".hpp", ".hxx", ".s", ".S",
    ".md", ".txt", ".py", ".sh", ".cmake", ".yml", ".yaml", ".json", ".ld",
}
VENDOR_CH585_PATH = Path("poc1-ch585-threadx/platform/ch585/wch")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("file", type=Path, help="software/ 下单个项目自有文本文件")
    parser.add_argument("--from-encoding", required=True, help="源文件的精确 Python codec 名称")
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--dry-run", action="store_true", help="只预览转换，不写入")
    mode.add_argument("--write", action="store_true", help="将文件原子替换为 UTF-8 无 BOM")
    return parser.parse_args()


def source_is_allowed(path: Path) -> bool:
    """Reject paths outside software/ and known vendor/generated trees."""
    try:
        relative = path.resolve().relative_to(SOFTWARE_ROOT)
    except ValueError:
        return False
    return (
        path.suffix in TEXT_SUFFIXES
        and not any(part in EXCLUDED_PARTS for part in relative.parts)
        and not relative.is_relative_to(VENDOR_CH585_PATH)
    )


def main() -> int:
    args = parse_args()
    if args.file.is_symlink():
        print(f"拒绝处理符号链接：{args.file}", file=sys.stderr)
        return 2
    path = args.file.resolve()
    if not source_is_allowed(path) or not path.is_file():
        print(f"拒绝处理非项目自有文件或不存在的文件：{path}", file=sys.stderr)
        return 2

    original = path.read_bytes()
    try:
        text = original.decode(args.from_encoding, errors="strict")
    except (LookupError, UnicodeDecodeError) as exc:
        print(f"无法按指定编码解码 {path}：{exc}", file=sys.stderr)
        return 2
    if text.startswith("\ufeff"):
        text = text[1:]
    converted = text.encode("utf-8")
    print(f"文件：{path}")
    print(f"源编码：{args.from_encoding}；目标编码：UTF-8，无 BOM")
    print(f"SHA-256：{hashlib.sha256(original).hexdigest()} -> {hashlib.sha256(converted).hexdigest()}")
    if args.dry_run:
        print("预览完成，文件未修改。")
        return 0

    mode = path.stat().st_mode
    temporary_name: str | None = None
    try:
        with tempfile.NamedTemporaryFile(dir=path.parent, delete=False) as temporary:
            temporary_name = temporary.name
            temporary.write(converted)
            temporary.flush()
            os.fsync(temporary.fileno())
        os.chmod(temporary_name, mode)
        os.replace(temporary_name, path)
    except OSError as exc:
        if temporary_name is not None:
            Path(temporary_name).unlink(missing_ok=True)
        print(f"转换失败：{exc}", file=sys.stderr)
        return 1
    print("转换完成。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
