# DBG-C Firmware Architecture and PoC-1 Record

**Document ID:** DBG-C-FW-001　**Version:** V0.3　**Status:** Experimental draft; host cross-build passes, user confirms no board is currently available, board runtime is pending

## 1. Scope and status

This phase checks whether Eclipse ThreadX can be cross-built in a CH585M project and implements experimental startup, clock, ThreadX timebase, and two-thread sleep/wakeup paths. The PoC is in [`software/poc1-ch585-threadx/`](../../software/poc1-ch585-threadx/); ThreadX is in [`software/third_party/threadx/`](../../software/third_party/threadx/). This is not DBG-C V1 firmware and includes no USB, CMSIS-DAP, SWD, CDC, BLE, private 2.4 GHz, or OTA.

| Item | Current record | Evidence boundary |
|---|---|---|
| ThreadX | Tag `v6.5.1.202602a_rel`, commit `b91b03b9e75fa523b17127f9e0eca09dca916459`; upstream includes `LICENSE.txt` under MIT | Exact submodule checkout |
| WCH EVT | Archive index is dated `2026.08`; its FreeRTOS example identifies FreeRTOS-Kernel V11.3.0 and CH585/CH584 QingKe V3C | Information from the WCH example; this PoC does not use FreeRTOS |
| EVT build configuration | `.cproject` selects its GCC12 RISC-V Compiler configuration, RV32I with M/C, ilp32 ABI, and executable prefix `riscv-none-embed-` | `.cproject` does not identify a GCC patch version; its prefix differs from the installed MounRiver toolchain |
| Installed toolchain | MounRiver Studio Linux x64 Toolchain V2.4.0; GCC 12.2.0, GNU assembler/linker 2.38; actual prefix `riscv-wch-elf-` | Measured on this host; not a claim about the EVT compiler version |
| System clock initialization | `highcode_init()` derives from the reset-time clock sequence in WCH `CH58x_sys.c` and selects HSI PLL 62.4 MHz | Source correspondence checked; frequency and stability have not been measured on hardware |
| ThreadX low-level setup | Sets `_tx_initialize_unused_memory` to the aligned location after linker symbol `_end`; configures WCH VTF SysTick entry, PFIC priority, and SysTick | Host compile/link passes; memory boundary and interrupt behavior are untested on board |
| ThreadX tick | Uses upstream `tx_api.h` default `TX_TIMER_TICKS_PER_SECOND`, defined as 100 ticks/s; SysTick ISR calls `_tx_thread_context_save` and `_tx_timer_interrupt`, then clears SysTick `SR` as defined by the EVT header | ISR assembly builds; HPE/VTF, exception return, and scheduling semantics remain unverified |
| PoC threads | Two same-priority threads increment separate counters, record `tx_time_get()`, and call `tx_thread_sleep(1U)` | Observable values are designed for later board testing only |
| ELF build | `text=8876`, `data=8`, `bss=5564` bytes; ELF32 RISC-V, entry `0x0` | One cross-build on this host; log: `software/poc1-ch585-threadx/build-evidence.log`; not flash or runtime evidence |
| CH585M board runtime | Not run; pending | User confirms no board is currently available; no programming/debug or runtime evidence exists |

## 2. Software layers and layout

```text
PoC application: src/main.c
ThreadX common kernel: software/third_party/threadx/common
ThreadX generic RISC-V32 context routines: upstream ports/risc-v32/gnu
CH585M adapter and startup: platform/ch585
Build: CMake and MounRiver WCH RISC-V GCC 12
```

The PoC keeps ThreadX upstream RISC-V32/GNU thread context save/restore, system-return, and interrupt-control routines. Local `tx_initialize_low_level.S`, `tx_initialize_low_level.c`, and `tx_systick.S` connect chip initialization and SysTick. `highcode_init.c` derives its clock sequence from WCH EVT `CH58x_sys.c`; the WCH startup and linker files come from its EVT FreeRTOS example. This is an experimental port and does not establish compatibility with QingKe V3C hardware stack push, VTF, or `mret` semantics.

## 3. Linux host setup

### 3.1 Required tools

Git, CMake 3.20 or newer, GNU Make, and the MounRiver Linux x64 RISC-V Embedded GCC 12 toolchain are required. The recorded build host has Git 2.43.0, CMake 3.28.3, GNU Make 4.3, GCC 12.2.0, and GNU assembler/linker 2.38. Ninja is optional.

### 3.2 Install the MounRiver toolchain

Download `MRS_Toolchain_Linux_X64_V240.tar.xz` from the [official MounRiver download page](https://www.mounriver.com/download). The page is dynamic; verify this archive SHA-256 before extraction:

```text
1fae593d27e24466f17c2df0fd00f746143f587fe33e912a78e35142fef82a6d
```

The following commands selectively extract the compiler directory without installing the full IDE:

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

The build script uses this per-user directory by default. Set `DBGC_WCH_TOOLCHAIN_ROOT` to the actual `bin` directory when the toolchain is elsewhere.

### 3.3 Fetch and build

```bash
git clone --recurse-submodules <DBG-C repository URL>
cd DBG-C
git submodule update --init --recursive
software/poc1-ch585-threadx/build.sh
```

Custom toolchain and build directory:

```bash
DBGC_WCH_TOOLCHAIN_ROOT="/actual/path/RISC-V Embedded GCC12/bin" \
DBGC_BUILD_DIR="build-local" \
software/poc1-ch585-threadx/build.sh
```

The build directory must stay under the PoC directory and defaults to `build/`; `software/poc1-ch585-threadx/.gitignore` excludes only `/build/`. The script records tool versions, repository and ThreadX revisions, build output, ELF size, and ELF header summary in `build-evidence.log`. The repository-root `.gitignore` does not need build-output changes.

## 4. WCH source provenance

The original material remains in the official EVT archive described in [`CH585EVT archive notes`](../09-references/CH585EVT/README.md). Hashes below are for the original ZIP entries. Repository copies of WCH headers only normalize line endings and trailing whitespace; the startup assembly and linker script also normalize indentation whitespace and trailing blank lines. Comparison confirms these are whitespace-only changes. The clock initialization file derives its function body from the listed WCH source and retains copyright/use notices.

| PoC file | Original WCH EVT entry | Original SHA-256 |
|---|---|---|
| `platform/ch585/startup/startup_CH585.S` | `EVT/EXAM/FreeRTOS/Startup/Startup_CH585_FreeRTOS.S` | `48248d39086a92c9a5c5d57bf5dce4c64cf3ce83d6de0ef14c14e1d6a9bdef49` |
| `platform/ch585/ld/ch585.ld` | `EVT/EXAM/FreeRTOS/Ld/Link.ld` | `a5e7454144693e4b1af34cc5f04c7a7c9277664102285f08f731fd9f93551598` |
| `platform/ch585/wch/CH585SFR.h` | `EVT/EXAM/SRC/StdPeriphDriver/inc/CH585SFR.h` | `b1ee193c9037813758d72f47bfecb02046aee0edcd0129e0f8e73bd806788de3` |
| `platform/ch585/wch/CH58x_clk.h` | `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_clk.h` | `7e5de3bbd5edb11c1d6fff2bff1b33de5725777dc7128f29dee6c7624db207da` |
| `platform/ch585/wch/CH58x_sys.h` | `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_sys.h` | `1ce59c4b44c30d59dea204be0ae907b6dc66938f0bfdb3ce3567bb9c46eb1a51` |
| `platform/ch585/wch/core_riscv.h` | `EVT/EXAM/SRC/RVMSIS/core_riscv.h` | `fd0099eb7fce96c1ae1c0c5c58fd5da0d8f6b6b167e2c3fb335d5910b0a5503e` |
| `platform/ch585/highcode_init.c` clock sequence | `EVT/EXAM/SRC/StdPeriphDriver/CH58x_sys.c` | `954a8b489c8b333d482ed65e0665a1260c5f7976398ef4878f4f928251f62772` |

Version and project-format evidence: `EVT/CH585_List_EN.txt` (SHA-256 `112318f05aa8764f591dcea84333fba3a0e978e234c9c2d4f9086dc915c27fc9`) is dated `2026.08`. This index and the FreeRTOS project metadata below do not declare a separate WCH SDK semantic version, so the date must not be described as an SDK version. The example `EVT/EXAM/FreeRTOS/readme.txt` (SHA-256 `ae231c158445d7d68ef7b0e534100c24afcccac903bf2475f5647c9903d1d353`) identifies a FreeRTOS-Kernel V11.3.0 port for CH585/CH584 QingKe V3C and describes hardware stack push, interrupt-stack, and fast SysTick interrupt-entry constraints. This is porting reference material, not proof of ThreadX compatibility. `EVT/EXAM/FreeRTOS/.project` (SHA-256 `e98d5f026b4f06d2cb6a46933c5aa85425ccdeb5a4941fbb8a40a7a0b94c8b41`) is an Eclipse CDT project description with project name `FreeRTOS`; `.cproject` (SHA-256 `158d9e4ff583f3bff39be61a5b19270f976ecc1d10dd5ce17170281c0d1bab81`) is a CDT managed-build configuration selecting the GCC12 RISC-V compiler option, RV32I with M/C, ilp32, and configured prefix `riscv-none-embed-`, but it does not state a GCC patch version. The sample's `EVT/EXAM/FreeRTOS/FreeRTOS/FreeRTOSConfig.h` sets a 500 Hz tick; this PoC does not reuse that value. ThreadX uses the upstream header's default of 100 ticks/s.

## 5. Linker, clock, and interrupt boundaries

The WCH example linker script defines FLASH at `0x00000000`, length 448 KiB, and RAM at `0x20000000`, length 128 KiB. These are the linked regions in the referenced EVT example; applicability to the board's silicon revision and actual runtime memory use still require confirmation.

The WCH sample's `highcode_init()` initializes HSI PLL to 62.4 MHz and configures related safe-access and Flash-clock fields. The PoC derives this function body into `highcode_init.c`; this does not establish that the board is running at 62.4 MHz.

The current SysTick ISR assembly entry is placed in the highcode region used by WCH startup. It creates the generic ThreadX 32-word interrupt frame, clears the HPE control bit following the WCH FreeRTOS port approach, calls ThreadX context-save and timer-interrupt functions, clears the SysTick status register defined by WCH `core_riscv.h`, and jumps to the ThreadX context-restore entry. Whether QingKe exception entry already pushes state, VTF entry/return behavior, HPE effects on saved state, frame compatibility with upstream ThreadX, and scheduler return are all subject to board verification. A host link cannot prove these behaviors.

## 6. Build result and remaining acceptance work

The host cross-build passed. The evidence log at `software/poc1-ch585-threadx/build-evidence.log` records ThreadX commit `b91b03b9e75fa523b17127f9e0eca09dca916459`, ELF32 RISC-V, entry `0x0`, and text 8876, data 8, bss 5564 bytes. No flashing, board interrupt/scheduling test, clock measurement, or endurance run was performed.

### 6.1 Software verification before hardware design

The user confirms that no hardware is currently available and requires software verification first, followed by hardware design and then verification on actual hardware. Only the host cross-build has passed so far; this does not prove CH585M interrupt, tick, or thread-scheduling behavior. The pre-hardware software verification gate and its pass criteria have not yet been defined; see OPEN-001 O19. Do not mark this PoC software-verified or board-verified until the criteria are agreed and met.

### 6.2 Remaining work

1. Confirm the exact CH585M board, programming/debug adapter, programming procedure, silicon revision, and boot behavior.
2. Observe `thread_a_runs`, `thread_b_runs`, and `threadx_tick_observed` changing continuously; verify `tx_thread_sleep(1U)` timeout/wakeup, same-priority rotation, and stability after reset.
3. Measure SysTick frequency and clock stability; verify VTF/PFIC/HPE behavior and exception stack integrity; retain firmware hash, board revision, programming log, and runtime log.
4. Verify interrupt nesting, scheduler preemption, and sustained runtime before marking the port board-verified and synchronizing MCU, FW, TEST, and OPEN documents.

## 7. Sources

- Eclipse ThreadX official repository and MIT license: [`software/third_party/threadx/`](../../software/third_party/threadx/); exact tag and commit are in Section 1.
- WCH CH585 EVT: [`archive notes`](../09-references/CH585EVT/README.md) and the archive entries and hashes listed above.
- MounRiver Linux x64 Toolchain V2.4.0: [official download page](https://www.mounriver.com/download); toolchain archive SHA-256 is in Section 3.
