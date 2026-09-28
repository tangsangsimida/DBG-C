#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_dir="$(cd -- "${project_dir}/../.." && pwd)"
build_dir="${project_dir}/build"
toolchain_file="${project_dir}/cmake/wch-riscv32.cmake"

cmake -S "${project_dir}" -B "${build_dir}" \
    -DCMAKE_TOOLCHAIN_FILE="${toolchain_file}" \
    -DCMAKE_BUILD_TYPE=Debug
cmake --build "${build_dir}" --parallel
"${HOME}/.local/share/DBG-C/toolchains/MRS-2.4.0/RISC-V Embedded GCC12/bin/riscv-wch-elf-size" \
    "${build_dir}/dbgc_poc1.elf"
