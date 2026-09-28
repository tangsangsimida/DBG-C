#!/usr/bin/env python3
"""Copy the pinned CMSIS-DAP command core and adapt one host-only delay guard."""

from __future__ import annotations

import hashlib
import pathlib
import subprocess
import sys


def copy_verified(source: pathlib.Path, destination: pathlib.Path) -> bytes:
    data = source.read_bytes()
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(data)
    return data


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: prepare_upstream.py CMSIS_DAP_ROOT OUTPUT_DIR", file=sys.stderr)
        return 2

    source_root = pathlib.Path(sys.argv[1])
    output_root = pathlib.Path(sys.argv[2])
    pinned_commit = "12636590eec66fae2d1bba4518749426ad5a4595"
    actual_commit = subprocess.check_output(
        ["git", "-C", str(source_root), "rev-parse", "HEAD"], text=True
    ).strip()
    if actual_commit != pinned_commit:
        raise RuntimeError(
            f"Expected CMSIS-DAP commit {pinned_commit}, got {actual_commit}"
        )
    subprocess.run(
        [
            "git",
            "-C",
            str(source_root),
            "diff",
            "--quiet",
            "HEAD",
            "--",
            "Firmware/Include/DAP.h",
            "Firmware/Source/DAP.c",
        ],
        check=True,
    )
    include_source = source_root / "Firmware/Include/DAP.h"
    source_source = source_root / "Firmware/Source/DAP.c"
    include_output = output_root / "include/DAP.h"
    source_output = output_root / "src/DAP.c"

    header = copy_verified(include_source, include_output)
    copy_verified(source_source, source_output)

    arm_guard = b"#if defined(__CC_ARM)\n"
    host_guard = b"#if defined(DBGC_CMSIS_DAP_TEST_C_LOOP)\n"
    if header.count(arm_guard) != 1:
        raise RuntimeError("Pinned DAP.h no longer has exactly one expected delay guard")
    include_output.write_bytes(header.replace(arm_guard, host_guard, 1))

    print(f"DAP.h source SHA-256: {hashlib.sha256(header).hexdigest()}")
    print(f"DAP.c source SHA-256: {hashlib.sha256(source_source.read_bytes()).hexdigest()}")
    print(f"CMSIS-DAP commit: {actual_commit}")
    print("Test adaptation: select upstream C-loop delay branch without defining __CC_ARM")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
