#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_dir="$(cd -- "${project_dir}/../.." && pwd)"
toolchain_bin="${DBGC_WCH_TOOLCHAIN_ROOT:-${HOME}/.local/share/DBG-C/toolchains/MRS-2.4.0/RISC-V Embedded GCC12/bin}"
host_cc="${CC:-cc}"
build_dir="${DBGC_BUILD_DIR:-${project_dir}/build}"
if [[ "${build_dir}" != /* ]]; then
    build_dir="${project_dir}/${build_dir}"
fi
build_dir="$(realpath -m -- "${build_dir}")"
case "${build_dir}" in
    "${project_dir}"/*) ;;
    *)
        printf 'Build directory must remain under %s\n' "${project_dir}" >&2
        exit 2
        ;;
esac

{
    printf 'Repository HEAD: '
    git -C "${repo_dir}" rev-parse HEAD
    printf 'ThreadX commit: '
    git -C "${repo_dir}/software/third_party/threadx" rev-parse HEAD
    printf 'ThreadX tag: '
    git -C "${repo_dir}/software/third_party/threadx" describe --tags --exact-match HEAD
    printf 'CMSIS-DAP commit: '
    git -C "${repo_dir}/software/third_party/cmsis-dap" rev-parse HEAD
    cmake --version | head -n 1
    "${toolchain_bin}/riscv-wch-elf-gcc" --version | head -n 1
    "${toolchain_bin}/riscv-wch-elf-as" --version | head -n 1
    "${toolchain_bin}/riscv-wch-elf-ld" --version | head -n 1
    printf 'Host C compiler: '
    "${host_cc}" --version | head -n 1
    cmake -S "${project_dir}" -B "${build_dir}" \
        -DCMAKE_TOOLCHAIN_FILE="${project_dir}/cmake/wch-riscv32.cmake" \
        -DDBGC_WCH_TOOLCHAIN_ROOT="${toolchain_bin}" \
        -DCMAKE_BUILD_TYPE=Debug
    cmake --build "${build_dir}" --parallel
    mkdir -p "${build_dir}/host-tests"
    "${host_cc}" -std=c99 -Wall -Wextra -Werror -pedantic \
        -I"${repo_dir}/software/common/byte_fifo/include" \
        "${repo_dir}/software/common/byte_fifo/src/dbgc_byte_fifo.c" \
        "${repo_dir}/software/common/byte_fifo/tests/test_dbgc_byte_fifo.c" \
        -o "${build_dir}/host-tests/test_dbgc_byte_fifo"
    "${build_dir}/host-tests/test_dbgc_byte_fifo"
    "${host_cc}" -std=c99 -Wall -Wextra -Werror -pedantic \
        -I"${repo_dir}/software/common/byte_fifo/include" \
        -I"${repo_dir}/software/common/byte_stream_bridge/include" \
        "${repo_dir}/software/common/byte_fifo/src/dbgc_byte_fifo.c" \
        "${repo_dir}/software/common/byte_stream_bridge/src/dbgc_byte_stream_bridge.c" \
        "${repo_dir}/software/common/byte_stream_bridge/tests/test_dbgc_byte_stream_bridge.c" \
        -o "${build_dir}/host-tests/test_dbgc_byte_stream_bridge"
    "${build_dir}/host-tests/test_dbgc_byte_stream_bridge"
    "${host_cc}" -std=c99 -Wall -Wextra -Werror -pedantic \
        -I"${repo_dir}/software/common/byte_fifo/include" \
        -I"${repo_dir}/software/common/byte_stream_bridge/include" \
        -I"${repo_dir}/software/common/byte_duplex_bridge/include" \
        "${repo_dir}/software/common/byte_fifo/src/dbgc_byte_fifo.c" \
        "${repo_dir}/software/common/byte_stream_bridge/src/dbgc_byte_stream_bridge.c" \
        "${repo_dir}/software/common/byte_duplex_bridge/src/dbgc_byte_duplex_bridge.c" \
        "${repo_dir}/software/common/byte_duplex_bridge/tests/test_dbgc_byte_duplex_bridge.c" \
        -o "${build_dir}/host-tests/test_dbgc_byte_duplex_bridge"
    "${build_dir}/host-tests/test_dbgc_byte_duplex_bridge"
    "${host_cc}" -std=c99 -Wall -Wextra -Werror -pedantic \
        -DDBGC_CH585_UART0_HOST_TEST \
        -I"${repo_dir}/software/common/ch585_uart0_host_test" \
        -I"${repo_dir}/software/common/byte_fifo/include" \
        -I"${repo_dir}/software/common/byte_stream_bridge/include" \
        -I"${repo_dir}/software/common/byte_duplex_bridge/include" \
        -I"${project_dir}/platform/ch585" \
        "${project_dir}/platform/ch585/dbgc_ch585_uart0.c" \
        "${project_dir}/platform/ch585/dbgc_ch585_uart0_bridge_adapter.c" \
        "${repo_dir}/software/common/byte_fifo/src/dbgc_byte_fifo.c" \
        "${repo_dir}/software/common/byte_stream_bridge/src/dbgc_byte_stream_bridge.c" \
        "${repo_dir}/software/common/byte_duplex_bridge/src/dbgc_byte_duplex_bridge.c" \
        "${repo_dir}/software/common/ch585_uart0_host_test/test_ch585_uart0.c" \
        -o "${build_dir}/host-tests/test_ch585_uart0"
    "${build_dir}/host-tests/test_ch585_uart0"
    "${host_cc}" -std=c99 -Wall -Wextra -Werror -pedantic \
        -DDBGC_CH585_SWD_GPIO_HOST_TEST \
        -I"${repo_dir}/software/common/ch585_swd_gpio_host_test" \
        -I"${project_dir}/platform/ch585" \
        "${project_dir}/platform/ch585/dbgc_ch585_swd_gpio.c" \
        "${repo_dir}/software/common/ch585_swd_gpio_host_test/test_ch585_swd_gpio.c" \
        -o "${build_dir}/host-tests/test_ch585_swd_gpio"
    "${build_dir}/host-tests/test_ch585_swd_gpio"
    "${host_cc}" -std=c99 -Wall -Wextra -Werror -pedantic \
        -DDBGC_CH585_TARGET_RESET_GPIO_HOST_TEST \
        -I"${repo_dir}/software/common/ch585_target_reset_gpio_host_test" \
        -I"${project_dir}/platform/ch585" \
        "${project_dir}/platform/ch585/dbgc_ch585_target_reset_gpio.c" \
        "${repo_dir}/software/common/ch585_target_reset_gpio_host_test/test_ch585_target_reset_gpio.c" \
        -o "${build_dir}/host-tests/test_ch585_target_reset_gpio"
    "${build_dir}/host-tests/test_ch585_target_reset_gpio"
    "${host_cc}" -std=c99 -Wall -Wextra -Werror -pedantic \
        -I"${project_dir}/platform/ch585" \
        "${project_dir}/platform/ch585/dbgc_ch585_uid.c" \
        "${repo_dir}/software/common/ch585_uid_host_test/test_ch585_uid.c" \
        -o "${build_dir}/host-tests/test_ch585_uid"
    "${build_dir}/host-tests/test_ch585_uid"
    cmsis_dap_host_dir="${build_dir}/host-tests/cmsis-dap"
    python3 "${repo_dir}/software/common/cmsis_dap_host_test/prepare_upstream.py" \
        "${repo_dir}/software/third_party/cmsis-dap" "${cmsis_dap_host_dir}"
    "${host_cc}" -std=c99 -Wall -Wextra -Werror -pedantic \
        -DDBGC_CMSIS_DAP_TEST_C_LOOP \
        -I"${repo_dir}/software/common/cmsis_dap_bounds/include" \
        -I"${repo_dir}/software/common/cmsis_dap_host_test" \
        -I"${cmsis_dap_host_dir}/include" \
        "${repo_dir}/software/common/cmsis_dap_bounds/src/dbgc_cmsis_dap_bounds.c" \
        "${repo_dir}/software/common/cmsis_dap_bounds/tests/test_cmsis_dap_bounds.c" \
        -o "${build_dir}/host-tests/test_cmsis_dap_bounds"
    "${build_dir}/host-tests/test_cmsis_dap_bounds"
    "${host_cc}" -std=c99 -Wall -Wextra -Werror \
        -Wno-unused-parameter -Wno-unused-variable -pedantic \
        -DDBGC_CMSIS_DAP_TEST_C_LOOP \
        -I"${repo_dir}/software/common/cmsis_dap_bounds/include" \
        -I"${repo_dir}/software/common/cmsis_dap_host_test" \
        -I"${cmsis_dap_host_dir}/include" \
        "${repo_dir}/software/common/cmsis_dap_bounds/src/dbgc_cmsis_dap_bounds.c" \
        "${cmsis_dap_host_dir}/src/DAP.c" \
        "${repo_dir}/software/common/cmsis_dap_host_test/test_dap_commands.c" \
        -o "${build_dir}/host-tests/test_cmsis_dap_commands"
    "${build_dir}/host-tests/test_cmsis_dap_commands"
    "${host_cc}" -std=c99 -Wall -Wextra -Werror \
        -Wno-unused-parameter -Wno-unused-variable -pedantic \
        -DDBGC_CMSIS_DAP_TEST_C_LOOP \
        -DDBGC_CMSIS_DAP_SWD_ENGINE_TEST \
        -I"${repo_dir}/software/common/cmsis_dap_host_test" \
        -I"${cmsis_dap_host_dir}/include" \
        "${cmsis_dap_host_dir}/src/DAP.c" \
        "${repo_dir}/software/third_party/cmsis-dap/Firmware/Source/SW_DP.c" \
        "${repo_dir}/software/common/cmsis_dap_host_test/test_swd_engine.c" \
        -o "${build_dir}/host-tests/test_cmsis_dap_swd_engine"
    "${build_dir}/host-tests/test_cmsis_dap_swd_engine"
    mkdir -p "${build_dir}/target-tests"
    "${toolchain_bin}/riscv-wch-elf-gcc" -std=gnu99 -Wall -Wextra -Werror \
        -march=rv32imac -mabi=ilp32 -mcmodel=medany \
        -I"${repo_dir}/software/common/cmsis_dap_bounds/include" \
        -I"${project_dir}/platform/ch585" \
        -I"${project_dir}/platform/ch585/wch" \
        -I"${repo_dir}/software/common/cmsis_dap_host_test" \
        -I"${cmsis_dap_host_dir}/include" \
        -c "${repo_dir}/software/common/cmsis_dap_bounds/src/dbgc_cmsis_dap_bounds.c" \
        -o "${build_dir}/target-tests/cmsis_dap_bounds.o"
    "${toolchain_bin}/riscv-wch-elf-readelf" -h \
        "${build_dir}/target-tests/cmsis_dap_bounds.o" | \
        rg 'Class:|Machine:'
    "${toolchain_bin}/riscv-wch-elf-gcc" -std=gnu99 -Wall -Wextra -Werror \
        -march=rv32imac -mabi=ilp32 -mcmodel=medany \
        -I"${project_dir}/platform/ch585" \
        -I"${project_dir}/platform/ch585/wch" \
        -c "${project_dir}/platform/ch585/dbgc_cmsis_compiler_check.c" \
        -o "${build_dir}/target-tests/dbgc_cmsis_compiler_check.o"
    "${toolchain_bin}/riscv-wch-elf-gcc" -std=gnu99 -Wall -Wextra -Werror \
        -march=rv32imac -mabi=ilp32 -mcmodel=medany \
        -I"${project_dir}/platform/ch585" \
        -I"${project_dir}/platform/ch585/wch" \
        -c "${project_dir}/platform/ch585/dbgc_ch585_swd_gpio.c" \
        -o "${build_dir}/target-tests/dbgc_ch585_swd_gpio.o"
    "${toolchain_bin}/riscv-wch-elf-gcc" -std=gnu99 -Wall -Wextra -Werror \
        -march=rv32imac -mabi=ilp32 -mcmodel=medany \
        -I"${project_dir}/platform/ch585" \
        -I"${project_dir}/platform/ch585/wch" \
        -c "${project_dir}/platform/ch585/dbgc_ch585_target_reset_gpio.c" \
        -o "${build_dir}/target-tests/dbgc_ch585_target_reset_gpio.o"
    "${toolchain_bin}/riscv-wch-elf-gcc" -std=gnu99 -Wall -Wextra -Werror \
        -march=rv32imac -mabi=ilp32 -mcmodel=medany \
        -I"${repo_dir}/software/common/byte_fifo/include" \
        -I"${repo_dir}/software/common/byte_stream_bridge/include" \
        -I"${repo_dir}/software/common/byte_duplex_bridge/include" \
        -I"${project_dir}/platform/ch585" \
        -I"${project_dir}/platform/ch585/wch" \
        -c "${project_dir}/platform/ch585/dbgc_ch585_uart0_bridge_adapter.c" \
        -o "${build_dir}/target-tests/dbgc_ch585_uart0_bridge_adapter.o"
    "${toolchain_bin}/riscv-wch-elf-gcc" -std=gnu99 -Wall -Wextra -Werror \
        -march=rv32imac -mabi=ilp32 -mcmodel=medany \
        -I"${project_dir}/platform/ch585" \
        -c "${project_dir}/platform/ch585/dbgc_ch585_uid.c" \
        -o "${build_dir}/target-tests/dbgc_ch585_uid.o"
    "${toolchain_bin}/riscv-wch-elf-gcc" -std=gnu99 -Wall -Wextra -Werror \
        -march=rv32imac -mabi=ilp32 -mcmodel=medany \
        -I"${project_dir}/platform/ch585" \
        -I"${project_dir}/platform/ch585/wch" \
        -c "${project_dir}/platform/ch585/dbgc_ch585_uart0.c" \
        -o "${build_dir}/target-tests/dbgc_ch585_uart0.o"
    "${toolchain_bin}/riscv-wch-elf-readelf" -h \
        "${build_dir}/target-tests/dbgc_ch585_target_reset_gpio.o" | \
        rg 'Class:|Machine:'
    "${toolchain_bin}/riscv-wch-elf-readelf" -h \
        "${build_dir}/target-tests/dbgc_ch585_uart0.o" | \
        rg 'Class:|Machine:'
    "${toolchain_bin}/riscv-wch-elf-readelf" -h \
        "${build_dir}/target-tests/dbgc_ch585_uart0_bridge_adapter.o" | \
        rg 'Class:|Machine:'
    "${toolchain_bin}/riscv-wch-elf-readelf" -h \
        "${build_dir}/target-tests/dbgc_ch585_uid.o" | \
        rg 'Class:|Machine:'
    "${toolchain_bin}/riscv-wch-elf-readelf" -h \
        "${build_dir}/target-tests/dbgc_ch585_swd_gpio.o" | \
        rg 'Class:|Machine:'
    "${toolchain_bin}/riscv-wch-elf-readelf" -Ws \
        "${build_dir}/target-tests/dbgc_cmsis_compiler_check.o" | \
        rg -o 'WEAK.*dbgc_cmsis_compiler_weak_check'
    "${toolchain_bin}/riscv-wch-elf-objdump" -d \
        "${build_dir}/target-tests/dbgc_cmsis_compiler_check.o" | \
        rg 'nop'
    "${toolchain_bin}/riscv-wch-elf-gcc" -std=gnu99 -Wall -Wextra -Werror \
        -Wno-unused-parameter -Wno-unused-variable \
        -march=rv32imac -mabi=ilp32 -mcmodel=medany \
        -DDBGC_CMSIS_DAP_TEST_C_LOOP \
        -I"${project_dir}/platform/ch585" \
        -I"${project_dir}/platform/ch585/wch" \
        -I"${repo_dir}/software/common/cmsis_dap_host_test" \
        -I"${cmsis_dap_host_dir}/include" \
        -c "${cmsis_dap_host_dir}/src/DAP.c" \
        -o "${build_dir}/target-tests/cmsis_dap_command_core.o"
    "${toolchain_bin}/riscv-wch-elf-gcc" -std=gnu99 -Wall -Wextra -Werror \
        -Wno-unused-parameter -Wno-unused-variable \
        -march=rv32imac -mabi=ilp32 -mcmodel=medany \
        -DDBGC_CMSIS_DAP_TEST_C_LOOP \
        -DDBGC_CMSIS_DAP_SWD_ENGINE_TEST \
        -I"${project_dir}/platform/ch585" \
        -I"${project_dir}/platform/ch585/wch" \
        -I"${repo_dir}/software/common/cmsis_dap_host_test" \
        -I"${cmsis_dap_host_dir}/include" \
        -c "${repo_dir}/software/third_party/cmsis-dap/Firmware/Source/SW_DP.c" \
        -o "${build_dir}/target-tests/cmsis_dap_swd_engine.o"
    "${toolchain_bin}/riscv-wch-elf-readelf" -h \
        "${build_dir}/target-tests/cmsis_dap_command_core.o" | \
        rg 'Class:|Machine:'
    "${toolchain_bin}/riscv-wch-elf-readelf" -h \
        "${build_dir}/target-tests/cmsis_dap_swd_engine.o" | \
        rg 'Class:|Machine:'
    "${toolchain_bin}/riscv-wch-elf-size" "${build_dir}/dbgc_poc1.elf"
    "${toolchain_bin}/riscv-wch-elf-readelf" -h "${build_dir}/dbgc_poc1.elf" | \
        rg 'Class:|Machine:|Entry point address:'
} 2>&1 | tee -a "${project_dir}/build-evidence.log"
