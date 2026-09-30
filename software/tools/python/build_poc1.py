#!/usr/bin/env python3
"""Configure, build, run host checks, and inspect the CH585M ThreadX PoC."""

from __future__ import annotations

import argparse
import os
import re
import shlex
import shutil
import subprocess
import sys
from pathlib import Path


SOFTWARE_ROOT = Path(__file__).resolve().parents[2]
SOFTWARE_BUILD_ROOT = SOFTWARE_ROOT / "build"
PROJECT_DIR = SOFTWARE_ROOT / "poc1-ch585-threadx"
REPO_DIR = SOFTWARE_ROOT.parent
HOST_FLAGS = ["-std=c99", "-Wall", "-Wextra", "-Werror", "-pedantic"]
TARGET_FLAGS = [
    "-std=gnu99",
    "-Wall",
    "-Wextra",
    "-Werror",
    "-march=rv32imac",
    "-mabi=ilp32",
    "-mcmodel=medany",
]


class BuildFailure(Exception):
    """A required command or evidence check failed."""


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--build-dir",
        type=Path,
        help="构建目录，必须位于 software/build/ 内；默认 software/build/poc1-ch585-threadx/",
    )
    parser.add_argument("--toolchain-root", type=Path, help="WCH 工具链目录；也可设置 DBGC_WCH_TOOLCHAIN_ROOT 或将工具加入 PATH")
    parser.add_argument("--host-cc", help="主机 C 编译器；也可设置 CC")
    parser.add_argument("--generator", help="可选的 CMake generator 名称")
    return parser.parse_args()


def resolve_build_directory(path: Path) -> Path:
    """Resolve build paths under software/build, accepting the former build/ prefix."""
    if not path.is_absolute():
        if path.parts and path.parts[0] == "build":
            path = Path(*path.parts[1:])
        path = SOFTWARE_BUILD_ROOT / path
    resolved = path.resolve()
    try:
        relative = resolved.relative_to(SOFTWARE_BUILD_ROOT.resolve())
    except ValueError as exc:
        raise BuildFailure(f"构建目录必须位于 {SOFTWARE_BUILD_ROOT} 下：{resolved}") from exc
    if not relative.parts:
        raise BuildFailure(f"构建目录不能是 software/build 根目录：{resolved}")
    return resolved


def display_command(command: list[str]) -> str:
    if os.name == "nt":
        return subprocess.list2cmdline(command)
    return shlex.join(command)


def run(
    command: list[str],
    log,
    *,
    cwd: Path = REPO_DIR,
    first_line: bool = False,
    match: str | None = None,
) -> str:
    """Run one command, stream its output to the console and evidence log."""
    rendered = display_command(command)
    print(f"\n$ {rendered}", flush=True)
    log.write(f"\n$ {rendered}\n")
    log.flush()
    try:
        process = subprocess.Popen(
            command,
            cwd=cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            encoding="utf-8",
            errors="replace",
            bufsize=1,
        )
    except OSError as exc:
        raise BuildFailure(f"无法启动命令 {command[0]}：{exc}") from exc

    output_lines: list[str] = []
    selected_lines: list[str] = []
    matcher = re.compile(match) if match else None
    assert process.stdout is not None
    for line in process.stdout:
        output_lines.append(line)
        log.write(line)
        if matcher is None or matcher.search(line):
            if not first_line or not selected_lines:
                print(line, end="", flush=True)
            selected_lines.append(line)
    return_code = process.wait()
    log.flush()
    if return_code:
        raise BuildFailure(f"命令失败，退出码 {return_code}：{rendered}")
    if matcher and not selected_lines:
        raise BuildFailure(f"命令输出没有匹配到所需信息“{match}”：{rendered}")
    if first_line and not selected_lines:
        raise BuildFailure(f"命令没有输出版本信息：{rendered}")
    return "".join(output_lines)


def select_host_compiler(requested: str | None) -> list[str]:
    """Resolve the configured host C compiler executable."""
    compiler = requested or os.environ.get("CC")
    if compiler:
        parts = shlex.split(compiler, posix=os.name != "nt")
        if len(parts) != 1:
            raise BuildFailure("--host-cc 或 CC 必须是单个编译器命令，不接受附加参数")
        executable = parts[0].strip('"')
        if not shutil.which(executable) and not Path(executable).is_file():
            raise BuildFailure(f"找不到主机 C 编译器：{executable}")
        return [executable]

    names = ("cc", "gcc", "clang") if os.name != "nt" else ("gcc", "clang")
    for name in names:
        executable = shutil.which(name)
        if executable:
            return [executable]
    raise BuildFailure("找不到主机 C 编译器；请安装 GCC/Clang 或设置 CC")


def tool_path(toolchain_root: Path, name: str) -> str:
    suffix = ".exe" if os.name == "nt" else ""
    path = toolchain_root / f"riscv-wch-elf-{name}{suffix}"
    if not path.is_file():
        raise BuildFailure(f"工具链程序不存在：{path}")
    return str(path)


def host_test_command(
    compiler: list[str],
    name: str,
    sources: list[Path],
    includes: list[Path],
    defines: tuple[str, ...] = (),
    extra_flags: tuple[str, ...] = (),
) -> list[str]:
    output = BUILD_DIR / "host-tests" / (name + (".exe" if os.name == "nt" else ""))
    command = compiler + HOST_FLAGS + list(extra_flags)
    command.extend(f"-D{define}" for define in defines)
    command.extend(f"-I{path}" for path in includes)
    command.extend(str(path) for path in sources)
    command.extend(["-o", str(output)])
    return command


def target_compile_command(
    gcc: str,
    name: str,
    source: Path,
    includes: list[Path],
    defines: tuple[str, ...] = (),
    extra_flags: tuple[str, ...] = (),
) -> list[str]:
    output = BUILD_DIR / "target-tests" / name
    command = [gcc] + TARGET_FLAGS + list(extra_flags)
    command.extend(f"-D{define}" for define in defines)
    command.extend(f"-I{path}" for path in includes)
    command.extend(["-c", str(source), "-o", str(output)])
    return command


def run_host_checks(log, host_cc: list[str], upstream_dir: Path) -> None:
    common = REPO_DIR / "software/common"
    platform = PROJECT_DIR / "platform/ch585"
    fifo_inc = common / "byte_fifo/include"
    fifo_src = common / "byte_fifo/src/dbgc_byte_fifo.c"
    byte_stream_inc = common / "byte_stream_bridge/include"
    byte_stream_src = common / "byte_stream_bridge/src/dbgc_byte_stream_bridge.c"
    duplex_inc = common / "byte_duplex_bridge/include"
    duplex_src = common / "byte_duplex_bridge/src/dbgc_byte_duplex_bridge.c"
    reset_inc = common / "target_reset_sequence/include"
    reset_src = common / "target_reset_sequence/src/dbgc_target_reset_sequence.c"
    update_inc = common / "update_manager/include"
    update_src = common / "update_manager/src/dbgc_update_manager.c"
    cmsis_inc = common / "cmsis_dap_host_test"
    bounds_inc = common / "cmsis_dap_bounds/include"
    dap_src = upstream_dir / "src/DAP.c"

    tests = [
        ("test_dbgc_byte_fifo", [fifo_src, common / "byte_fifo/tests/test_dbgc_byte_fifo.c"], [fifo_inc], (), ()),
        ("test_dbgc_packet_queue", [common / "packet_queue/src/dbgc_packet_queue.c", common / "packet_queue/tests/test_dbgc_packet_queue.c"], [common / "packet_queue/include"], (), ()),
        ("test_dbgc_byte_stream_bridge", [fifo_src, byte_stream_src, common / "byte_stream_bridge/tests/test_dbgc_byte_stream_bridge.c"], [fifo_inc, byte_stream_inc], (), ()),
        ("test_dbgc_byte_duplex_bridge", [fifo_src, byte_stream_src, duplex_src, common / "byte_duplex_bridge/tests/test_dbgc_byte_duplex_bridge.c"], [fifo_inc, byte_stream_inc, duplex_inc], (), ()),
        ("test_dbgc_target_reset_sequence", [reset_src, common / "target_reset_sequence/tests/test_dbgc_target_reset_sequence.c"], [reset_inc], (), ()),
        ("test_dbgc_update_manager", [update_src, common / "update_manager/tests/test_dbgc_update_manager.c"], [update_inc], (), ()),
        ("test_ch585_target_reset_sequence", [platform / "dbgc_ch585_target_reset_gpio.c", platform / "dbgc_ch585_target_reset_sequence.c", reset_src, common / "ch585_target_reset_gpio_host_test/test_ch585_target_reset_sequence.c"], [common / "ch585_target_reset_gpio_host_test", reset_inc, platform], ("DBGC_CH585_TARGET_RESET_GPIO_HOST_TEST",), ()),
        ("test_ch585_uart0", [platform / "dbgc_ch585_uart0.c", platform / "dbgc_ch585_uart0_bridge_adapter.c", fifo_src, byte_stream_src, duplex_src, common / "ch585_uart0_host_test/test_ch585_uart0.c"], [common / "ch585_uart0_host_test", fifo_inc, byte_stream_inc, duplex_inc, platform], ("DBGC_CH585_UART0_HOST_TEST",), ()),
        ("test_ch585_swd_gpio", [platform / "dbgc_ch585_swd_gpio.c", common / "ch585_swd_gpio_host_test/test_ch585_swd_gpio.c"], [common / "ch585_swd_gpio_host_test", platform], ("DBGC_CH585_SWD_GPIO_HOST_TEST",), ()),
        ("test_ch585_jtag_gpio", [platform / "dbgc_ch585_jtag_gpio.c", common / "ch585_jtag_gpio_host_test/test_ch585_jtag_gpio.c"], [common / "ch585_jtag_gpio_host_test", platform], ("DBGC_CH585_JTAG_GPIO_HOST_TEST",), ()),
        ("test_ch585_target_reset_gpio", [platform / "dbgc_ch585_target_reset_gpio.c", common / "ch585_target_reset_gpio_host_test/test_ch585_target_reset_gpio.c"], [common / "ch585_target_reset_gpio_host_test", platform], ("DBGC_CH585_TARGET_RESET_GPIO_HOST_TEST",), ()),
        ("test_ch585_target_power_gpio", [platform / "dbgc_ch585_target_power_gpio.c", common / "ch585_target_power_gpio_host_test/test_ch585_target_power_gpio.c"], [common / "ch585_target_power_gpio_host_test", platform], ("DBGC_CH585_TARGET_POWER_GPIO_HOST_TEST",), ()),
        ("test_ch585_ui_gpio", [platform / "dbgc_ch585_ui_gpio.c", common / "ch585_ui_gpio_host_test/test_ch585_ui_gpio.c"], [common / "ch585_ui_gpio_host_test", platform], ("DBGC_CH585_UI_GPIO_HOST_TEST",), ()),
        ("test_ch585_uid", [platform / "dbgc_ch585_uid.c", common / "ch585_uid_host_test/test_ch585_uid.c"], [common / "ch585_uid_host_test", platform], ("DBGC_CH585_UID_HOST_TEST",), ()),
        ("test_ch585_vtref_adc", [platform / "dbgc_ch585_vtref_adc.c", common / "ch585_vtref_adc_host_test/test_ch585_vtref_adc.c"], [common / "ch585_vtref_adc_host_test", platform], ("DBGC_CH585_VTREF_ADC_HOST_TEST",), ()),
        ("test_ch585_spi1_config", [platform / "dbgc_ch585_spi1_config.c", common / "ch585_spi1_config_host_test/test_ch585_spi1_config.c"], [common / "ch585_spi1_config_host_test", platform], ("DBGC_CH585_SPI1_CONFIG_HOST_TEST",), ()),
        ("test_ch585_spi1_gpio", [platform / "dbgc_ch585_spi1_gpio.c", common / "ch585_spi1_gpio_host_test/test_ch585_spi1_gpio.c"], [common / "ch585_spi1_gpio_host_test", platform], ("DBGC_CH585_SPI1_GPIO_HOST_TEST",), ()),
        ("test_dbgc_cmsis_dap_service", [common / "packet_queue/src/dbgc_packet_queue.c", common / "cmsis_dap_bounds/src/dbgc_cmsis_dap_bounds.c", common / "cmsis_dap_service/src/dbgc_cmsis_dap_service.c", common / "cmsis_dap_service/tests/test_dbgc_cmsis_dap_service.c"], [common / "packet_queue/include", bounds_inc, common / "cmsis_dap_service/include", cmsis_inc, upstream_dir / "include"], ("DBGC_CMSIS_DAP_TEST_C_LOOP",), ()),
        ("test_cmsis_dap_bounds", [common / "cmsis_dap_bounds/src/dbgc_cmsis_dap_bounds.c", common / "cmsis_dap_bounds/tests/test_cmsis_dap_bounds.c"], [bounds_inc, cmsis_inc, upstream_dir / "include"], ("DBGC_CMSIS_DAP_TEST_C_LOOP",), ()),
        ("test_cmsis_dap_commands", [common / "cmsis_dap_bounds/src/dbgc_cmsis_dap_bounds.c", dap_src, common / "cmsis_dap_host_test/test_dap_commands.c"], [bounds_inc, cmsis_inc, upstream_dir / "include"], ("DBGC_CMSIS_DAP_TEST_C_LOOP",), ("-Wno-unused-parameter", "-Wno-unused-variable")),
        ("test_cmsis_dap_swd_engine", [platform / "dbgc_ch585_swd_gpio.c", dap_src, REPO_DIR / "software/third_party/cmsis-dap/Firmware/Source/SW_DP.c", common / "cmsis_dap_host_test/test_swd_engine.c"], [common / "ch585_swd_gpio_host_test", cmsis_inc, platform, upstream_dir / "include"], ("DBGC_CMSIS_DAP_TEST_C_LOOP", "DBGC_CMSIS_DAP_SWD_ENGINE_TEST", "DBGC_CH585_SWD_GPIO_HOST_TEST"), ("-Wno-unused-parameter", "-Wno-unused-variable")),
        ("test_cmsis_dap_jtag_engine", [platform / "dbgc_ch585_jtag_gpio.c", dap_src, REPO_DIR / "software/third_party/cmsis-dap/Firmware/Source/SW_DP.c", REPO_DIR / "software/third_party/cmsis-dap/Firmware/Source/JTAG_DP.c", common / "cmsis_dap_host_test/test_jtag_engine.c"], [common / "ch585_jtag_gpio_host_test", cmsis_inc, platform, upstream_dir / "include"], ("DBGC_CMSIS_DAP_TEST_C_LOOP", "DBGC_CMSIS_DAP_JTAG_ENGINE_TEST", "DBGC_CH585_JTAG_GPIO_HOST_TEST"), ("-Wno-unused-parameter", "-Wno-unused-variable")),
    ]
    (BUILD_DIR / "host-tests").mkdir(parents=True, exist_ok=True)
    for name, sources, includes, defines, extra_flags in tests:
        run(host_test_command(host_cc, name, sources, includes, defines, extra_flags), log)
        executable = BUILD_DIR / "host-tests" / (name + (".exe" if os.name == "nt" else ""))
        run([str(executable)], log)


def run_target_checks(log, toolchain_root: Path, upstream_dir: Path) -> None:
    common = REPO_DIR / "software/common"
    platform = PROJECT_DIR / "platform/ch585"
    vendor = REPO_DIR / "software/third_party/wch/ch585"
    gcc = tool_path(toolchain_root, "gcc")
    readelf = tool_path(toolchain_root, "readelf")
    linker = tool_path(toolchain_root, "ld")
    nm = tool_path(toolchain_root, "nm")
    objdump = tool_path(toolchain_root, "objdump")
    (BUILD_DIR / "target-tests").mkdir(parents=True, exist_ok=True)

    checks = [
        ("cmsis_dap_bounds.o", common / "cmsis_dap_bounds/src/dbgc_cmsis_dap_bounds.c", [common / "cmsis_dap_bounds/include", platform, platform / "wch", common / "cmsis_dap_host_test", upstream_dir / "include"], (), ()),
        ("dbgc_update_manager.o", common / "update_manager/src/dbgc_update_manager.c", [common / "update_manager/include"], (), ()),
        ("dbgc_cmsis_compiler_check.o", platform / "dbgc_cmsis_compiler_check.c", [platform, platform / "wch"], (), ()),
        ("dbgc_ch585_swd_gpio.o", platform / "dbgc_ch585_swd_gpio.c", [platform, platform / "wch"], (), ()),
        ("dbgc_ch585_target_reset_gpio.o", platform / "dbgc_ch585_target_reset_gpio.c", [platform, platform / "wch"], (), ()),
        ("dbgc_ch585_target_power_gpio.o", platform / "dbgc_ch585_target_power_gpio.c", [platform, platform / "wch"], (), ()),
        ("dbgc_ch585_ui_gpio.o", platform / "dbgc_ch585_ui_gpio.c", [platform, platform / "wch"], (), ()),
        ("dbgc_ch585_uart0_bridge_adapter.o", platform / "dbgc_ch585_uart0_bridge_adapter.c", [common / "byte_fifo/include", common / "byte_stream_bridge/include", common / "byte_duplex_bridge/include", platform, platform / "wch"], (), ()),
        ("dbgc_ch585_uid.o", platform / "dbgc_ch585_uid.c", [platform, platform / "wch", vendor], (), ()),
        ("dbgc_ch585_vtref_adc.o", platform / "dbgc_ch585_vtref_adc.c", [platform, platform / "wch"], (), ()),
        ("dbgc_ch585_spi1_config.o", platform / "dbgc_ch585_spi1_config.c", [platform, platform / "wch"], (), ()),
        ("dbgc_ch585_spi1_gpio.o", platform / "dbgc_ch585_spi1_gpio.c", [platform, platform / "wch"], (), ()),
        ("dbgc_ch585_uart0.o", platform / "dbgc_ch585_uart0.c", [platform, platform / "wch"], (), ()),
        ("cmsis_dap_command_core.o", upstream_dir / "src/DAP.c", [platform, platform / "wch", common / "cmsis_dap_host_test", upstream_dir / "include"], ("DBGC_CMSIS_DAP_TEST_C_LOOP",), ("-Wno-unused-parameter", "-Wno-unused-variable")),
        ("cmsis_dap_swd_engine.o", REPO_DIR / "software/third_party/cmsis-dap/Firmware/Source/SW_DP.c", [platform, platform / "wch", common / "cmsis_dap_host_test", upstream_dir / "include"], ("DBGC_CMSIS_DAP_TEST_C_LOOP", "DBGC_CMSIS_DAP_SWD_ENGINE_TEST"), ("-Wno-unused-parameter", "-Wno-unused-variable")),
        ("cmsis_dap_jtag_command_core.o", upstream_dir / "src/DAP.c", [platform, platform / "wch", common / "cmsis_dap_host_test", upstream_dir / "include"], ("DBGC_CMSIS_DAP_TEST_C_LOOP", "DBGC_CMSIS_DAP_JTAG_ENGINE_TEST"), ("-Wno-unused-parameter", "-Wno-unused-variable")),
        ("cmsis_dap_jtag_engine.o", REPO_DIR / "software/third_party/cmsis-dap/Firmware/Source/JTAG_DP.c", [platform, platform / "wch", common / "cmsis_dap_host_test", upstream_dir / "include"], ("DBGC_CMSIS_DAP_TEST_C_LOOP", "DBGC_CMSIS_DAP_JTAG_ENGINE_TEST"), ("-Wno-unused-parameter", "-Wno-unused-variable")),
    ]
    for name, source, includes, defines, extra_flags in checks:
        run(target_compile_command(gcc, name, source, includes, defines, extra_flags), log)

    uid_object = BUILD_DIR / "target-tests/dbgc_ch585_uid.o"
    uid_link = BUILD_DIR / "target-tests/dbgc_ch585_uid_link.o"
    run([linker, "-r", str(uid_object), str(vendor / "libISP585.a"), "-o", str(uid_link)], log)
    undefined = run([nm, "-u", str(uid_link)], log)
    if re.search(r"GET_UNIQUE_ID|FLASH_EEPROM_CMD", undefined):
        raise BuildFailure("UID target link check left a vendor UID/flash symbol unresolved")

    for name in (
        "cmsis_dap_bounds.o",
        "dbgc_ch585_target_reset_gpio.o",
        "dbgc_ch585_target_power_gpio.o",
        "dbgc_ch585_ui_gpio.o",
        "dbgc_ch585_uart0.o",
        "dbgc_ch585_uart0_bridge_adapter.o",
        "dbgc_ch585_uid.o",
        "dbgc_ch585_vtref_adc.o",
        "dbgc_ch585_spi1_config.o",
        "dbgc_ch585_spi1_gpio.o",
        "dbgc_ch585_swd_gpio.o",
        "cmsis_dap_command_core.o",
        "cmsis_dap_swd_engine.o",
        "cmsis_dap_jtag_command_core.o",
        "cmsis_dap_jtag_engine.o",
    ):
        run([readelf, "-h", str(BUILD_DIR / "target-tests" / name)], log, match=r"Class:|Machine:")
    run([readelf, "-Ws", str(BUILD_DIR / "target-tests/dbgc_cmsis_compiler_check.o")], log, match=r"WEAK.*dbgc_cmsis_compiler_weak_check")
    run([objdump, "-d", str(BUILD_DIR / "target-tests/dbgc_cmsis_compiler_check.o")], log, match=r"nop")
    run([nm, "-u", str(uid_link)], log)
    elf = BUILD_DIR / "dbgc_poc1.elf"
    run([tool_path(toolchain_root, "size"), str(elf)], log)
    run([readelf, "-h", str(elf)], log, match=r"Class:|Machine:|Entry point address:")


def run_build(args: argparse.Namespace) -> None:
    global BUILD_DIR
    requested_build_dir = args.build_dir or Path(
        os.environ.get("DBGC_BUILD_DIR", "poc1-ch585-threadx")
    )
    BUILD_DIR = resolve_build_directory(requested_build_dir)
    BUILD_DIR.mkdir(parents=True, exist_ok=True)
    evidence_log = BUILD_DIR / "build-evidence.log"

    requested_toolchain = args.toolchain_root or os.environ.get("DBGC_WCH_TOOLCHAIN_ROOT")
    if requested_toolchain:
        toolchain_root = Path(requested_toolchain).expanduser().resolve()
    else:
        gcc = shutil.which("riscv-wch-elf-gcc")
        if not gcc:
            raise BuildFailure("未找到 WCH RISC-V GCC；请将工具链加入 PATH 或设置 DBGC_WCH_TOOLCHAIN_ROOT")
        toolchain_root = Path(gcc).resolve().parent
    host_cc = select_host_compiler(args.host_cc)

    with evidence_log.open("a", encoding="utf-8", newline="") as log:
        run(["git", "-C", str(REPO_DIR), "rev-parse", "HEAD"], log, first_line=True)
        threadx = REPO_DIR / "software/third_party/threadx"
        cmsis = REPO_DIR / "software/third_party/cmsis-dap"
        run(["git", "-C", str(threadx), "rev-parse", "HEAD"], log, first_line=True)
        run(["git", "-C", str(threadx), "describe", "--tags", "--exact-match", "HEAD"], log, first_line=True)
        run(["git", "-C", str(cmsis), "rev-parse", "HEAD"], log, first_line=True)
        cmake = os.environ.get("CMAKE", "cmake")
        run([cmake, "--version"], log, first_line=True)
        for name in ("gcc", "as", "ld"):
            run([tool_path(toolchain_root, name), "--version"], log, first_line=True)
        run(host_cc + ["--version"], log, first_line=True)

        configure = [
            cmake, "-S", str(PROJECT_DIR), "-B", str(BUILD_DIR),
            f"-DCMAKE_TOOLCHAIN_FILE={PROJECT_DIR / 'cmake/wch-riscv32.cmake'}",
            f"-DDBGC_WCH_TOOLCHAIN_ROOT={toolchain_root}",
            "-DCMAKE_BUILD_TYPE=Debug",
        ]
        if args.generator:
            configure[1:1] = ["-G", args.generator]
        run(configure, log)
        run(["cmake", "--build", str(BUILD_DIR), "--parallel"], log)

        upstream_dir = BUILD_DIR / "host-tests/cmsis-dap"
        upstream_dir.parent.mkdir(parents=True, exist_ok=True)
        run(
            [
                sys.executable,
                str(SOFTWARE_ROOT / "tools/python/prepare_cmsis_dap_upstream.py"),
                str(cmsis),
                str(upstream_dir),
            ],
            log,
        )
        run_host_checks(log, host_cc, upstream_dir)
        run_target_checks(log, toolchain_root, upstream_dir)


def main() -> int:
    try:
        run_build(parse_args())
    except BuildFailure as exc:
        print(f"构建失败：{exc}", file=sys.stderr)
        return 1
    print(f"构建和软件检查完成。证据日志：{BUILD_DIR / 'build-evidence.log'}")
    return 0


BUILD_DIR = SOFTWARE_BUILD_ROOT / "poc1-ch585-threadx"


if __name__ == "__main__":
    raise SystemExit(main())
