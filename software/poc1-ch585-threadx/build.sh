#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_dir="$(cd -- "${project_dir}/../.." && pwd)"
toolchain_bin="${DBGC_WCH_TOOLCHAIN_ROOT:-${HOME}/.local/share/DBG-C/toolchains/MRS-2.4.0/RISC-V Embedded GCC12/bin}"
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
    cmake --version | head -n 1
    "${toolchain_bin}/riscv-wch-elf-gcc" --version | head -n 1
    "${toolchain_bin}/riscv-wch-elf-as" --version | head -n 1
    "${toolchain_bin}/riscv-wch-elf-ld" --version | head -n 1
    cmake -S "${project_dir}" -B "${build_dir}" \
        -DCMAKE_TOOLCHAIN_FILE="${project_dir}/cmake/wch-riscv32.cmake" \
        -DDBGC_WCH_TOOLCHAIN_ROOT="${toolchain_bin}" \
        -DCMAKE_BUILD_TYPE=Debug
    cmake --build "${build_dir}" --parallel
    "${toolchain_bin}/riscv-wch-elf-size" "${build_dir}/dbgc_poc1.elf"
    "${toolchain_bin}/riscv-wch-elf-readelf" -h "${build_dir}/dbgc_poc1.elf" | \
        rg 'Class:|Machine:|Entry point address:'
} 2>&1 | tee "${project_dir}/build-evidence.log"
