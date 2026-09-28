# DBG-C Firmware Architecture and PoC-1 Record

**Document ID:** DBG-C-FW-001　**Version:** V0.1　**Status:** Experimental draft; build verified, runtime, tick, and preemption on CH585M not verified

## 1. Scope and status

This stage checks whether the Eclipse ThreadX release sources can be referenced from a CH585M project and whether the firmware ELF can be built with the current MounRiver RISC-V toolchain. PoC source is in [`software/poc1-ch585-threadx/`](../../software/poc1-ch585-threadx/); the upstream repository is a Git submodule at [`software/third_party/threadx/`](../../software/third_party/threadx/). This is not DBG-C V1 firmware and does not implement USB, CMSIS-DAP, SWD, CDC, BLE, private 2.4 GHz, or OTA.

| Item | Established status | Evidence boundary |
|---|---|---|
| ThreadX | Release tag `v6.5.1.202602a_rel`, commit `b91b03b9e75fa523b17127f9e0eca09dca916459` | Submodule checkout; upstream repository includes MIT `LICENSE.txt` |
| ThreadX port | Build uses upstream generic `ports/risc-v32/gnu` | Compilation does not prove compatibility with QingKe V3C exception entry, hardware stack push, or context switching |
| WCH chip inputs | Startup assembly and linker script from the FreeRTOS example inside archived `docs/09-references/CH585EVT/CH585EVT.ZIP` | Only the files actually used were copied; WCH copyright/use notices are retained |
| Toolchain | MounRiver Studio Linux x64 Toolchain V2.4.0, RISC-V Embedded GCC 12; GCC 12.2.0, GNU assembler/linker 2.38 | Observed executable prefix is `riscv-wch-elf-`; do not treat `riscv-none-embed-` in EVT `.cproject` as this PoC's actual prefix |
| Build | `dbgc_poc1.elf` linked successfully; text 7124, data 8, bss 5560 bytes | One build on this host with this toolchain; not target-board runtime evidence |
| ThreadX tick | Not connected | WCH startup's default `SysTick_Handler` loops forever; the PoC does not configure SysTick or connect `_tx_timer_interrupt` |
| CH585M board scheduling | Not run | No connected CH585M development board was identified; host `lsusb` did not show a device identifiable as a CH585 debug/programming interface |

The two same-priority PoC threads increment `thread_a_runs` and `thread_b_runs`, then call `tx_thread_relinquish()`. The `volatile` counters are observable with a debugger, but they do not establish that the threads have run on CH585M while the platform interrupt and startup adaptation is incomplete.

## 2. Software layers and current layout

```text
Application: PoC main and two test threads
ThreadX common kernel: official submodule common/
CPU port: official RISC-V32/GNU assembly sources
CH585 platform: WCH startup and linker inputs plus local adapter area
Build: CMake and MounRiver WCH RISC-V GCC 12
```

The build references the upstream generic RISC-V32/GNU files through ThreadX's official CMake custom-port entry; no upstream ThreadX source is copied or modified. `platform/ch585/threadx_port/CMakeLists.txt` only selects source files and is not a completed CH585 port. The WCH startup assembly comes from its EVT FreeRTOS project. `highcode_init()` is currently a local empty implementation, and the SysTick vector retains the WCH file's default halt handler.

## 3. Host environment setup

### 3.1 Dependencies

Git, CMake 3.20 or newer, GNU Make, and MounRiver's Linux x64 RISC-V Embedded GCC 12 toolchain are required. The current build host reports Git 2.43.0, CMake 3.28.3, GNU Make 4.3, GCC 12.2.0, and GNU assembler/linker 2.38. Ninja is optional.

### 3.2 Install the MounRiver toolchain

Download `MRS_Toolchain_Linux_X64_V240.tar.xz` from the [official MounRiver download page](https://www.mounriver.com/download). The page is dynamic; verify the downloaded filename and SHA-256 below before extracting:

```text
1fae593d27e24466f17c2df0fd00f746143f587fe33e912a78e35142fef82a6d
```

The commands selectively extract the required compiler directory without installing the full MounRiver IDE:

```bash
archive="/path/to/MRS_Toolchain_Linux_X64_V240.tar.xz"
printf '%s  %s\n' \
  '1fae593d27e24466f17c2df0fd00f746143f587fe33e912a78e35142fef82a6d' \
  "$archive" | sha256sum -c -
mkdir -p "$HOME/.cache/DBG-C/MRS-2.4.0"
tar -xJf "$archive" -C "$HOME/.cache/DBG-C/MRS-2.4.0" \
  'Toolchain/RISC-V Embedded GCC12'
mkdir -p "$HOME/.local/share/DBG-C/toolchains/MRS-2.4.0"
cp -a "$HOME/.cache/DBG-C/MRS-2.4.0/Toolchain/RISC-V Embedded GCC12" \
  "$HOME/.local/share/DBG-C/toolchains/MRS-2.4.0/"
"$HOME/.local/share/DBG-C/toolchains/MRS-2.4.0/RISC-V Embedded GCC12/bin/riscv-wch-elf-gcc" --version
```

The archive is approximately 411 MB. The build configuration uses this per-user location by default. To use a different location, set `DBGC_WCH_TOOLCHAIN_ROOT` to the actual `bin` directory during CMake configuration.

### 3.3 Fetch and build

```bash
git clone --recurse-submodules <DBG-C repository URL>
cd DBG-C
git submodule update --init --recursive
software/poc1-ch585-threadx/build.sh
```

For a different toolchain location:

```bash
cmake -S software/poc1-ch585-threadx \
  -B software/poc1-ch585-threadx/build \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/software/poc1-ch585-threadx/cmake/wch-riscv32.cmake" \
  -DDBGC_WCH_TOOLCHAIN_ROOT="/actual/path/RISC-V Embedded GCC12/bin" \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build software/poc1-ch585-threadx/build --parallel
```

Build outputs stay in `software/poc1-ch585-threadx/build/` and are excluded by the PoC-local `.gitignore`; the repository-root `.gitignore` does not need changes.

## 4. Chip startup and resource boundaries

The copied WCH files come from these exact archive entries:

| Repository file | Original EVT path | Original archive SHA-256 |
|---|---|---|
| `software/poc1-ch585-threadx/platform/ch585/startup/startup_CH585.S` | `EVT/EXAM/FreeRTOS/Startup/Startup_CH585_FreeRTOS.S` | `48248d39086a92c9a5c5d57bf5dce4c64cf3ce83d6de0ef14c14e1d6a9bdef49` |
| `software/poc1-ch585-threadx/platform/ch585/ld/ch585.ld` | `EVT/EXAM/FreeRTOS/Ld/Link.ld` | `a5e7454144693e4b1af34cc5f04c7a7c9277664102285f08f731fd9f93551598` |

The repository copies normalize line endings and whitespace only; the listed SHA-256 values are for the original files inside the ZIP archive.

The linker script from this WCH example defines FLASH at `0x00000000` with a length of 448 KiB and RAM at `0x20000000` with a length of 128 KiB. These are the linked region values in the referenced EVT sample, not new validation for a specific DBG-C PCB, package/silicon revision, or all available RAM.

The WCH EVT FreeRTOS startup configures PFIC/VTF, `mtvec`, and QingKe CSRs; its FreeRTOS port also uses a software interrupt for task switching and adjusts HPE during context save. The generic ThreadX upstream port assumes standard RISC-V M-mode exception entry and software stack frames. Their interrupt-entry and context models remain an unverified integration boundary and cannot be assumed compatible because linking succeeds. The current empty `highcode_init()` also does not set the system clock; do not present an SDK default or sample frequency as this PoC's actual runtime frequency.

## 5. Acceptance record and remaining work

Only host cross-compilation has passed: CMake configuration succeeded, ThreadX common sources and RISC-V assembly compiled, and the ELF linked. Output size is text 7124, data 8, bss 5560 bytes. Build command: `software/poc1-ch585-threadx/build.sh`. No flash operation or hardware test was run.

Before completing PoC-1:

1. Implement and verify ThreadX startup, system-stack/unused-memory bounds, exception/interrupt frame, HPE setup, and context save/restore using the CH585 official reference manual and EVT source; retain third-party copyright notices.
2. Configure SysTick as the ThreadX system clock and define an ISR entry that does not conflict with ThreadX scheduling. Verify the exact register/API names and interrupt-clear path against the EVT/reference manual first.
3. Replace the WCH startup's weak default SysTick infinite loop, configure a verified system clock, and give ISR/system stacks and both thread stacks explicit linker boundaries.
4. On a CH585M board, observe both thread counters, ThreadX ticks, sleep timeout/wakeup, preemption, and sustained operation. Record board revision, oscillator/clock setup, programming method, firmware hash, test logs, and reset results.
5. Only after board validation, mark the port as runtime-verified and synchronize MCU, FW, TEST, and OPEN documents.

## 6. Sources

- Eclipse ThreadX official repository and MIT license: [`software/third_party/threadx/`](../../software/third_party/threadx/), tag `v6.5.1.202602a_rel`, commit `b91b03b9e75fa523b17127f9e0eca09dca916459`.
- WCH official EVT archive: [`CH585EVT archive notes`](../09-references/CH585EVT/README.md); implementation evidence is limited to the startup and linker entries listed above.
- MounRiver official download page: [MounRiver Studio Downloads](https://www.mounriver.com/download), Linux x64 Toolchain V2.4.0; the package SHA-256 is recorded above.
