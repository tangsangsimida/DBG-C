#!/usr/bin/env python3
"""Format or check project-owned C/C++ files using software/.clang-format."""

from __future__ import annotations

import argparse
import concurrent.futures
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import time
from datetime import datetime
from pathlib import Path


SOFTWARE_ROOT = Path(__file__).resolve().parents[2]
CONFIG_FILE = SOFTWARE_ROOT / ".clang-format"
C_SUFFIXES = {".c", ".h"}
CPP_SUFFIXES = {".cc", ".cpp", ".cxx", ".hh", ".hpp", ".hxx", ".h"}
EXCLUDED_PARTS = {".git", "build", "generated", "obj", "third_party"}
VENDOR_CH585_PATH = Path("poc1-ch585-threadx/platform/ch585/wch")
CACHE_FILE = SOFTWARE_ROOT / "build/.cache/format_cache.json"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=__doc__,
        epilog=(
            "示例：\n"
            "  python3 software/tools/python/format_code.py --all\n"
            "  python3 software/tools/python/format_code.py --check --all\n"
            "  python3 software/tools/python/format_code.py --all --dirs software/common\n"
            "  python3 software/tools/python/format_code.py software/common/packet_queue/src/dbgc_packet_queue.c"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    targets = parser.add_mutually_exclusive_group()
    targets.add_argument("--c", action="store_true", help="扫描 C 文件和 C 头文件")
    targets.add_argument("--cpp", action="store_true", help="扫描 C++ 文件和头文件")
    targets.add_argument("--all", action="store_true", help="扫描 C 和 C++ 项目源码")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--check", action="store_true", help="只检查，不修改文件")
    mode.add_argument("--write", action="store_true", help="执行格式化；省略时也是默认模式")
    parser.add_argument("--dirs", nargs="+", help="指定 software/ 内的扫描目录")
    parser.add_argument("--workers", type=int, default=4, help="并行任务数，默认 4")
    parser.add_argument("--report", type=Path, help="保存格式化报告")
    parser.add_argument(
        "--clang-format",
        default=os.environ.get("CLANG_FORMAT", "clang-format"),
        help="clang-format 命令；也可设置 CLANG_FORMAT",
    )
    parser.add_argument("--no-cache", action="store_true", help="禁用写入模式的哈希缓存")
    parser.add_argument("-v", "--verbose", action="store_true", help="显示未变化文件和缓存信息")
    parser.add_argument("files", nargs="*", type=Path, help="只处理指定的项目自有源文件")
    return parser.parse_args()


def allowed_relative(path: Path) -> Path | None:
    """Return a path relative to software/ if it is in the owned source scope."""
    try:
        relative = path.resolve().relative_to(SOFTWARE_ROOT)
    except ValueError:
        return None
    if any(part in EXCLUDED_PARTS for part in relative.parts):
        return None
    if relative.is_relative_to(VENDOR_CH585_PATH):
        return None
    return relative


def target_suffixes(args: argparse.Namespace) -> set[str]:
    if args.c:
        return C_SUFFIXES
    if args.cpp:
        return CPP_SUFFIXES
    return C_SUFFIXES | CPP_SUFFIXES


def resolve_user_path(path: Path) -> Path:
    """Resolve paths passed from either the repository or software directory."""
    if path.is_absolute():
        return path.resolve()
    from_cwd = path.resolve()
    try:
        from_cwd.relative_to(SOFTWARE_ROOT)
        return from_cwd
    except ValueError:
        return (SOFTWARE_ROOT / path).resolve()


def resolve_scan_dirs(values: list[str] | None) -> list[Path]:
    if not values:
        return [SOFTWARE_ROOT]
    directories: list[Path] = []
    for value in values:
        path = resolve_user_path(Path(value))
        relative = allowed_relative(path)
        if relative is None or not path.is_dir():
            raise ValueError(f"拒绝扫描目录（需为 software/ 下的项目目录）：{value}")
        directories.append(path)
    return directories


def find_files(args: argparse.Namespace) -> list[Path]:
    suffixes = target_suffixes(args)
    if args.files:
        if args.all or args.dirs:
            raise ValueError("指定文件时不能同时使用 --all 或 --dirs")
        files = [resolve_user_path(path) for path in args.files]
    else:
        directories = resolve_scan_dirs(args.dirs)
        files = []
        for directory in directories:
            for current, child_dirs, names in os.walk(directory):
                current_path = Path(current)
                relative = current_path.relative_to(SOFTWARE_ROOT)
                child_dirs[:] = [
                    name
                    for name in child_dirs
                    if name not in EXCLUDED_PARTS
                    and not (relative / name).is_relative_to(VENDOR_CH585_PATH)
                ]
                files.extend(
                    current_path / name
                    for name in names
                    if (current_path / name).suffix.lower() in suffixes
                )

    files = sorted(set(path.resolve() for path in files))
    invalid = [
        path
        for path in files
        if allowed_relative(path) is None or not path.is_file() or path.suffix.lower() not in suffixes
    ]
    if invalid:
        raise ValueError("拒绝处理非项目自有源码或不支持的路径：" + ", ".join(map(str, invalid)))
    return files


def file_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load_cache() -> dict[str, str]:
    try:
        value = json.loads(CACHE_FILE.read_text(encoding="utf-8"))
        return value if isinstance(value, dict) else {}
    except (OSError, UnicodeError, json.JSONDecodeError):
        return {}


def format_one(
    path: Path,
    relative: str,
    args: argparse.Namespace,
    tool: str,
    cache: dict[str, str],
    cache_prefix: str,
) -> tuple[str, str, str | None]:
    """Return status, relative path, and an optional diagnostic."""
    key = cache_prefix + relative
    before_hash = file_hash(path)
    if not args.check and not args.no_cache and cache.get(key) == before_hash:
        return "cached", relative, None

    try:
        if args.check:
            command = [tool, "--dry-run", "--Werror", str(path)]
            result = subprocess.run(command, capture_output=True, text=True, timeout=30)
            if result.returncode:
                return "needs-format", relative, result.stderr.strip() or result.stdout.strip()
            return "unchanged", relative, None

        result = subprocess.run([tool, "-i", str(path)], capture_output=True, text=True, timeout=30)
        if result.returncode:
            return "failed", relative, result.stderr.strip() or result.stdout.strip()
        after_hash = file_hash(path)
        return ("modified" if before_hash != after_hash else "unchanged"), relative, None
    except subprocess.TimeoutExpired:
        return "failed", relative, "格式化超时"
    except OSError as exc:
        return "failed", relative, str(exc)


def tool_version(tool: str) -> str:
    result = subprocess.run([tool, "--version"], capture_output=True, text=True, timeout=10)
    if result.returncode:
        raise ValueError(f"无法读取 clang-format 版本：{result.stderr.strip()}")
    output = result.stdout.strip() or result.stderr.strip()
    match = re.search(r"version\s+(\d+)(?:\.\d+)*", output, re.IGNORECASE)
    if match and int(match.group(1)) < 11:
        raise ValueError(f"要求 clang-format 11 或更新版本，当前为：{output.splitlines()[0]}")
    return output.splitlines()[0] if output else "未知版本"


def save_report(
    destination: Path,
    mode: str,
    version: str,
    started_at: datetime,
    elapsed: float,
    results: list[tuple[str, str, str | None]],
) -> None:
    counts = {key: sum(result[0] == key for result in results) for key in ("modified", "unchanged", "cached", "needs-format", "failed")}
    lines = [
        "DBG-C 源码格式化报告",
        f"时间：{started_at.astimezone().isoformat(timespec='seconds')}",
        f"模式：{mode}",
        f"工具：{version}",
        f"文件数：{len(results)}",
        f"已修改：{counts['modified']}",
        f"无变化：{counts['unchanged']}",
        f"缓存跳过：{counts['cached']}",
        f"需格式化：{counts['needs-format']}",
        f"失败：{counts['failed']}",
        f"耗时：{elapsed:.2f}s",
    ]
    for status, path, error in results:
        if status in {"modified", "needs-format", "failed"}:
            lines.append(f"{status}: {path}" + (f" — {error}" if error else ""))
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    args = parse_args()
    if not (args.c or args.cpp or args.all or args.files):
        print("请指定 --c、--cpp、--all 或一个以上源文件路径。", file=sys.stderr)
        return 2
    if args.workers < 1:
        print("--workers 必须大于零。", file=sys.stderr)
        return 2
    if not CONFIG_FILE.is_file():
        print(f"找不到格式配置：{CONFIG_FILE}", file=sys.stderr)
        return 2

    try:
        tool = shutil.which(args.clang_format) or (args.clang_format if Path(args.clang_format).is_file() else None)
        if not tool:
            raise ValueError(f"找不到 clang-format：{args.clang_format}")
        version = tool_version(tool)
        files = find_files(args)
        if not files:
            raise ValueError("扫描范围内没有符合条件的 C/C++ 文件")
    except (OSError, ValueError, subprocess.SubprocessError) as exc:
        print(f"格式化工具错误：{exc}", file=sys.stderr)
        return 2

    started = datetime.now().astimezone()
    start_time = time.monotonic()
    mode = "检查" if args.check else "格式化"
    cache = load_cache() if not args.check and not args.no_cache else {}
    prefix = f"{version}|{file_hash(CONFIG_FILE)}|"
    print(f"[{mode}] {version}；扫描 {len(files)} 个文件；并行数 {args.workers}")
    results: list[tuple[str, str, str | None]] = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.workers) as executor:
        futures = {
            executor.submit(
                format_one,
                path,
                path.relative_to(SOFTWARE_ROOT).as_posix(),
                args,
                tool,
                cache,
                prefix,
            ): path
            for path in files
        }
        for future in concurrent.futures.as_completed(futures):
            result = future.result()
            results.append(result)
            status, relative, error = result
            if status in {"modified", "unchanged"} and not args.check and not args.no_cache:
                cache[prefix + relative] = file_hash(SOFTWARE_ROOT / relative)
            if status in {"modified", "needs-format", "failed"}:
                print(f"{status}: {relative}" + (f" — {error}" if error else ""))
            elif args.verbose:
                print(f"{status}: {relative}")
    results.sort(key=lambda result: result[1])
    elapsed = time.monotonic() - start_time
    counts = {key: sum(result[0] == key for result in results) for key in ("modified", "unchanged", "cached", "needs-format", "failed")}
    print(
        f"统计：总计 {len(results)}，修改 {counts['modified']}，无变化 {counts['unchanged']}，"
        f"缓存跳过 {counts['cached']}，需格式化 {counts['needs-format']}，失败 {counts['failed']}，"
        f"耗时 {elapsed:.2f}s"
    )

    if not args.check and not args.no_cache:
        CACHE_FILE.parent.mkdir(parents=True, exist_ok=True)
        CACHE_FILE.write_text(json.dumps(cache, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    if args.report:
        report_path = resolve_user_path(args.report)
        save_report(report_path, mode, version, started, elapsed, results)
        print(f"报告已保存：{report_path}")

    if counts["failed"] or (args.check and counts["needs-format"]):
        return 1
    print("格式检查通过。" if args.check else "格式化完成。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
