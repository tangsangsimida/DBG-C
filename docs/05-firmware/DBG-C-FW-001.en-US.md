# DBG-C Firmware Architecture and PoC-1 Record

**Document ID:** DBG-C-FW-001　**Version:** V0.24　**Status:** Experimental draft; generic FIFO, CMSIS-DAP command-core, and upstream SWD-engine host checks pass; CMSIS compiler-macro mapping passes target-object compile checks; product HAL adaptation incomplete; verification-board design gate passed; board runtime and product-hardware freeze not passed

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
| ThreadX tick | Uses upstream `tx_api.h` default `TX_TIMER_TICKS_PER_SECOND`, defined as 100 ticks/s; based on the source-declared 62.4 MHz, `SysTick_Config` receives 624000 and sets compare value 623999 | This is a software configuration calculation, not a measured clock/tick; the user's reference to 1000 ticks/s conflicts with current source and needs confirmation; HPE/VTF, exception return, and scheduling semantics remain unverified |
| PoC threads | Two same-priority threads increment separate counters, record `tx_time_get()`, and call `tx_thread_sleep(1U)` | Observable values are designed for later board testing only |
| ELF build | `text=8876`, `data=8`, `bss=5564` bytes; ELF32 little-endian RISC-V, entry `_start=0x0`; `.highcode_init`/`.highcode` are linked to RAM and code/data load images to Flash | Two clean rebuilds produced identical ELF SHA-256 `d06802e345843562221f86dfb29934442e7d15d25d55de3327ca9528c961101f` and map SHA-256 `fec0082f30bbc3022aa696f7ec13f9037dc03aa8717e19bff854645d5964f936`; see `software/poc1-ch585-threadx/build-evidence.log`. This proves reproducibility in the recorded environment and consistency with the current linker script, not chip runtime |
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

Git, CMake 3.20 or newer, GNU Make, the MounRiver Linux x64 RISC-V Embedded GCC 12 toolchain, and a native C99 compiler are required. Host tests use `cc` by default; set `CC` to select another compiler executable. The recorded build host has Git 2.43.0, CMake 3.28.3, GNU Make 4.3, GCC 12.2.0, GNU assembler/linker 2.38, and Ubuntu cc 13.3.0. Ninja is optional.

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

Two clean rebuilds produced identical ELF and map hashes. ELF header, sections, program headers, symbols, and map were statically checked against the PoC startup and `ch585.ld`: `_start` is the ELF entry, application `.highcode` is linked to RAM, and linked segments fit the Flash/RAM ranges declared by the current script. The evidence log at `software/poc1-ch585-threadx/build-evidence.log` records ThreadX commit `b91b03b9e75fa523b17127f9e0eca09dca916459`, MounRiver GCC 12.2.0, GNU assembler/linker 2.38, ELF32 RISC-V, entry `0x0`, and text 8876, data 8, bss 5564 bytes. No flashing, board interrupt/scheduling test, clock measurement, or endurance run was performed.

### 6.1 Software verification and hardware-design sequence

No hardware is currently available. Apply the sequence “software static verification → limited-purpose verification-board design → board runtime verification → product-hardware freeze” with two release gates:

1. **Verification-board design gate: passed.** Two clean builds produced identical ELF/map files; ELF architecture, entry, load/run sections, symbols, and linker ranges were statically checked against the startup and linker script. Source review covered reset initialization, ThreadX context entry, exception stack switching, SysTick/VTF/PFIC/HPE setup, and ISR flow. A minimal board for verification may now be designed, with programming/recovery access and observation/measurement points for reset, system clock, SysTick, and interrupt activity. This gate does not establish CH585M runtime success and does not release product schematic or PCB freeze.
2. **Product-hardware freeze gate: not passed.** No CH585M board runtime evidence exists. ThreadX startup, thread switching/sleep/wakeup, measured SysTick/tick delivery, stack integrity, reset/sleep-wakeup, and sustained runtime require board tests. Test duration, repetitions, load, and pass thresholds must be defined in the test plan before execution. Source is set to 100 ticks/s while requirement text mentions 1000 ticks/s; resolve this conflict before freezing the board-test frequency threshold.

Static review cannot prove QingKe V3C exception stacking, VTF/HPE, interrupt return, or scheduling semantics; runtime cases remain Not run in TEST-001 and must be measured on CH585M. The upstream ThreadX `qemu_virt` example has its own QEMU virt entry and linker layout and cannot replace CH585M/WCH-path validation. QEMU, Spike, and Renode are not installed in the current environment. See OPEN-001 O19 for the software gate and verdict.

### 6.2 Verification-board design constraints

Keep the verification board minimal and provide safe programming/recovery debug access plus measurement access needed to observe reset, system clock, SysTick, and interrupt activity. Determine exact pins, probe methods, loads, and electrical protection from the CH585M datasheet, WCH EVT, and test plan; this record does not specify unchecked pins or circuitry. The verification board must not be represented as a frozen product schematic/PCB.

### 6.3 Remaining work

1. Confirm the exact CH585M board, programming/debug adapter, programming procedure, silicon revision, and boot behavior.
2. Observe `thread_a_runs`, `thread_b_runs`, and `threadx_tick_observed` changing continuously; verify `tx_thread_sleep(1U)` timeout/wakeup, same-priority rotation, and stability after reset.
3. Measure SysTick frequency and clock stability; verify VTF/PFIC/HPE behavior and exception stack integrity; retain firmware hash, board revision, programming log, and runtime log.
4. Freeze board-test duration, repetitions, load, and pass thresholds in the test plan first. Verify interrupt nesting, scheduler preemption, and sustained runtime before closing O18 and synchronizing MCU, FW, TEST, and OPEN documents.

## 7. Sources

- Eclipse ThreadX official repository and MIT license: [`software/third_party/threadx/`](../../software/third_party/threadx/); exact tag and commit are in Section 1.
- Arm official CMSIS-DAP repository: [`software/third_party/cmsis-dap/`](../../software/third_party/cmsis-dap/), pinned to commit `12636590eec66fae2d1bba4518749426ad5a4595` (checked 2026-09-29); `DAP.h` declares CMSIS-DAP firmware version 2.1.2; repository license is Apache-2.0. This is CMSIS-DAP protocol firmware source, not the complete DAPLink firmware system.
- Compiler-macro mapping basis: WCH EVT `core_riscv.h` defines GNU compiler `__ASM`/`__INLINE` after `CH585SFR.h` supplies `IRQn_Type`; official GCC documentation covers [extended inline assembly](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html) and [function attributes](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html) including `always_inline` and weak symbols; the RISC-V official [Unprivileged ISA](https://docs.riscv.org/reference/isa/unpriv/rv32.html) defines `nop`. External sources checked 2026-09-29. This supports compiler primitives only, not GPIO/SWD timing or chip runtime behavior.
- WCH CH585 EVT: [`archive notes`](../09-references/CH585EVT/README.md) and the archive entries and hashes listed above.
- MounRiver Linux x64 Toolchain V2.4.0: [official download page](https://www.mounriver.com/download); toolchain archive SHA-256 is in Section 3.

## 8. Software Module Implementation Status

Resource evidence is in [MCU-001 Section 5](../04-hardware/DBG-C-MCU-001.en-US.md): USB CDC/UART and RF paths require SRAM data buffering. The official EVT archive contains `EVT/EXAM/BLE/BLE_UART/APP/app_drv_fifo/app_drv_fifo.c` and `.h` (original SHA-256 values `312e039857300b5142f6ae6898a592b6257170d146f13a92e02bd8808bc5ec67` and `5ce231b33f9a5897a6be2abddc1901cd181f63c93c3fa00867216ac14cc8b6cc`) and `EVT/EXAM/BLE/BLE_UART/APP/peripheral.c`, showing a WCH example that buffers UART data with a byte FIFO. This example does not prove that its concurrency protection fits ThreadX; this project neither copies nor calls that FIFO directly.

| Module | Software location/resource evidence | Current implementation boundary | Status/verification |
|---|---|---|---|
| Generic byte FIFO | `software/common/byte_fifo/`; corresponds to USB CDC, UART, and RF buffering in MCU-001 | Caller-owned fixed storage; arbitrary nonzero capacity; partial reads/writes; no overwrite of unread data; clear and capacity queries; no dynamic allocation, chip registers, interrupts, or ThreadX API; not concurrency-safe, callers must serialize access | Added to the existing CMake build and cross-compiled for CH585; 3683 assertions pass in the host verification run integrated into the existing build script; not connected to USB/UART/RF and not board-tested; not a frozen product ABI |
| CMSIS-DAP / DAP command core | MCU-001 USBFS allocation; PRD CMSIS-DAP v2 target; Arm official source pinned at `software/third_party/cmsis-dap/` commit `12636590eec66fae2d1bba4518749426ad5a4595` | `DAP.h` declares `DAP_ProcessCommand()`/`DAP_ExecuteCommand()` and firmware version macro 2.1.2; upstream depends on missing `cmsis_compiler.h`, and its non-ArmCC branch uses Arm `subs` inline assembly that WCH RISC-V GCC cannot assemble directly | Eight host checks cover Info/error responses, TransferConfigure retry settings, DP read/write, WAIT retry, AP posted reads in Transfer and TransferBlock, and final `DP_RDBUFF` ordering through mocked `SWD_Transfer()`. WCH RISC-V GCC compiles SWD-enabled `DAP.c` to an ELF32 RISC-V object. Test pin macros are no-ops, JTAG is disabled, and the object is not linked into PoC. This is not product HAL/configuration and does not verify GPIO, electrical timing, USB, threads, or board behavior |
| CMSIS-DAP SWD I/O engine | Pinned upstream `Firmware/Source/SW_DP.c`; MCU-001 assigns PB5=SWDIO and PB6=SWCLK | The upstream bit-level algorithm calls I/O through `DAP_config.h` pin macros; project host tests connect them to a callback-backed line model, not WCH GPIO; fast delay is a no-op | Nine host cases with 168 assertions cover SWD read/write data and parity, WAIT/FAULT ACKs, 10-bit input/output sequences and DAP_SWD_Sequence command parsing, end-to-end DP IDCODE reads after DAP_Connect, AP posted-read data returned through DP_RDBUFF in DAP_Transfer and DAP_TransferBlock, and the two-item AP TransferBlock transaction sequence. WCH RISC-V GCC compiles upstream `SW_DP.c` to an ELF32 RISC-V object. It is not linked with product HAL or run against a Target/CH585M board; this proves no PB5/PB6 waveform, frequency, voltage, or timing |
| USBFS Device + CDC transport | MCU-001 USBFS, PB10/PB11; EVT `CH58x_usbdev.h/.c` and USB Device COM example | Archive low-level API, EP0–EP4 buffer structure, and CDC descriptors have been inspected; the example separates CDC/vendor modes and does not prove DAP + CDC composition, current-project compatibility, or ThreadX ISR synchronization | Not implemented; verify O02/O08, freeze USB-001, and complete host enumeration/transport verification |
| UART0 driver/bridge | MCU-001 UART0, PB4/PB7; EVT `CH58x_uart.h`, `CH58x_uart0.c` | Archive API names and polling implementation have been inspected; the COM example remaps UART0 to PA15/PA14, differing from MCU-001 PB4/PB7 allocation; actual PB4/PB7 init, CDC behavior, baud/flow-control, and ThreadX synchronization remain unconfirmed | Not implemented; obtain current SDK initialization evidence for PB4/PB7 and define O09 parameters plus ISR/task concurrency behavior |
| SWD Engine / Target Manager | MCU-001 PB5/PB6 SWD GPIO and PA4 Reset GPIO | SWD timing, direction switching, electrical boundaries, target voltage/protection are not executable specifications yet | Not implemented; freeze IF-001 and verify timing/electrical requirements under O11/O12 |
| BLE GATT/configuration/OTA | Integrated BLE Radio and WCH BLE examples | Product services/characteristics, authentication, MTU, pairing, and OTA image behavior are undefined | Not implemented; complete BLE-001 and O06/O07/O10/O13 first |
| Private 2.4 GHz transport | Integrated Radio and DBG-C RF Protocol target | PHY/API, frame format, sequence, retries, recovery, and latency target remain undefined | Not implemented; complete RF-001 and O01/O03/O12 first |
| CH585M ThreadX port | Current `software/poc1-ch585-threadx/platform/ch585/` | Reset, SysTick, PFIC/VTF/HPE, and context switching depend on chip hardware semantics | Experimental PoC only; startup, interrupts, tick, context switch, scheduling, sleep/wakeup have not been verified on a CH585M board; O18 remains open |

The byte FIFO only establishes a hardware-independent storage boundary. USB/UART/RF callers must not depend on its internal indices. Do not present it as an ISR-safe queue or product data protocol until synchronization and transport APIs are confirmed. Host cases in `software/common/byte_fifo/tests/test_dbgc_byte_fifo.c` are built and run by the existing `software/poc1-ch585-threadx/build.sh` using the native C99 compiler; 3683 assertions pass. This does not verify concurrency, ISR use, CH585M SRAM, or board behavior.

CMSIS-DAP tests are in `software/common/cmsis_dap_host_test/` and run through the existing `software/poc1-ch585-threadx/build.sh`. The preparation script verifies the pinned commit and unchanged upstream inputs, then changes only the delay-branch guard in a build-directory copy of `DAP.h`; it does not define `__CC_ARM` or modify the third-party submodule. Eight command-core cases use no-op pin macros and a mocked `SWD_Transfer()`. A separate host executable builds original upstream `SW_DP.c` with `DAP.c`; callback-backed pin macros connect it to a pure host bit-stream model. Nine cases check read success/parity error, write data/parity, WAIT/FAULT ACKs, 10-bit SWD input/output sequences and DAP_SWD_Sequence command parsing, end-to-end DAP_Connect plus DP IDCODE, AP DAP_Transfer, and two-item AP DAP_TransferBlock responses, direction changes, and modeled call counts. Fast delay is a no-op, so cycle count is not a clock measurement. WCH RISC-V GCC compiles test-configured `DAP.c` and `SW_DP.c` to separate ELF32 RISC-V objects, neither linked into PoC. These checks do not verify product GPIO HAL, PB5/PB6 electrical behavior/timing, USBFS integration, ThreadX, or CH585M runtime.


### 8.1 WCH USBFS and UART0 source review

This section records static review of original archive entries. It does not mean the older examples have been ported to DBG-C, compiled by the current project, or run on a CH585M board.

| Official archive entry | Original-file evidence | DBG-C conclusion |
|---|---|---|
| `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_usbdev.h`; SHA-256 `1b5dbf1304127b980d2f38f974e3372c5e55a747fe22319a835156deac1fee0f` | File header marks V1.2, 2021/11/17; declares `USB_DeviceInit()`, `USB_DevTransProcess()`, EP1–EP4 IN/OUT handlers; describes 64-byte buffers for EP0 and EP1–EP4 | Confirms WCH USBFS Device low-level interface and endpoint-buffer example exist in the archive; does not establish ThreadX ISR/task synchronization or a DAP + CDC composite device |
| `EVT/EXAM/SRC/StdPeriphDriver/CH58x_usbdev.c`; SHA-256 `dfef009a88c6c3c70670889bbab43898264967f979dc56b4c68180542b4ea540` | File header marks V1.2, 2021/11/17; `USB_DeviceInit()` configures bidirectional channels for four nonzero endpoints, DMA addresses, ACK/NAK states, and USB interrupt enables | This is a direct-register example; it is not a verified ThreadX-safe USB Service/HAL |
| `EVT/EXAM/USB/Device/COM/src/Main.c`; SHA-256 `4345ca31938ab30bf4c772e1ceeaccbd717c6a82c8a26012ef8708280fbcfda4` | File header marks V1.0, 2020/08/06; contains CDC and vendor descriptor sets; CDC uses EP4 interrupt IN and EP1/EP2 Bulk; runtime selects CDC or vendor mode through `usb_work_mode`; USB ISR accesses registers directly and masks/restores interrupts with PFIC | Confirms a CDC Device example exists in the archive; it does not show CMSIS-DAP v2 Bulk integrated in the same configuration. The two modes are not a simultaneous composite configuration in this example. Endpoint mapping, descriptors, IRQ, and ThreadX integration need redesign and measurement |
| `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_uart.h`; SHA-256 `99e419e0906b4fdaba12fd1aea3cdf58109cfd8906b1508d69e5dd884c741f70` | File header marks V1.2, 2021/11/17; declares UART0 baud, trigger level, interrupt configuration, TX/RX APIs, and status macros | Confirms archive API spellings; does not establish target-project mux initialization for PB4/PB7 or ISR-to-ThreadX synchronization safety |
| `EVT/EXAM/SRC/StdPeriphDriver/CH58x_uart0.c`; SHA-256 `f0b6b252dcb69530428f32cf2bb50cb585fbecd6f5cfd0eb57f194e257d86338` | File header marks V1.2, 2021/11/17; default initialization sets 115200 baud, a 4-byte FIFO trigger field, and TXD interrupt enable; string send/receive routines poll FIFO status/count | These are example defaults, not frozen DBG-C parameters. Polling APIs do not satisfy the undefined UART/ThreadX concurrency contract and cannot be used directly as the product bridge driver |
| `DebugInit()` in `EVT/EXAM/USB/Device/COM/src/Main.c` | Calls `GPIOPinRemap(ENABLE, RB_PIN_UART0)`, then configures PA15/PA14 and initializes UART0 | This CDC example remaps UART0 to PA15/PA14; DBG-C MCU-001 assigns PB4/PB7. Do not copy this example's pin initialization into product firmware. Obtain final SDK initialization evidence for PB4/PB7 |

The EVT GPIO header `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_gpio.h` (SHA-256 `c7f450ceaa501e4c5a912bc0429bca1540fa3c3f26e3fd55dfd908fad9b06183`) declares `GPIOB_ModeCfg(uint32_t pin, GPIOModeTypeDef mode)` and defines `GPIOB_SetBits`, `GPIOB_ResetBits`, and `GPIOB_ReadPortPin`. `EVT/EXAM/SRC/StdPeriphDriver/CH58x_gpio.c` (SHA-256 `c2c7c9b2518a2a56144f107776418911ae0849df523481ee7b999cbdfab01757`) contains the GPIOB mode-configuration implementation. Together with MCU-001's PB5/PB6 assignment, these files confirm official SDK port API symbols; they are not compiled into this project. Target voltage, protection, drive mode, and switching timing remain insufficiently specified for product HAL selection.

The generic byte FIFO remains the only hardware-independent module currently reusable as a product implementation. The pinned CMSIS-DAP command core and upstream SWD bit-level engine now have host coverage, but the callback-backed line model is a test fixture, not a WCH GPIO HAL or complete debugger port. EVT GPIO APIs were statically reviewed but not integrated or built into firmware. PB5/PB6 modes, drive strength, target voltage compatibility, external protection, and SWD timing remain open pending IF-001/O11/O12 and verification-board measurements. USBFS + CDC, UART0 driver/bridge, BLE, and private RF still lack complete interface/scheduling/electrical evidence; do not infer their parameters from example defaults. Keep O02, O08, O09, and O18 open. ThreadX and USB/UART ISR behavior still requires board verification.

### 8.3 Upstream SWD engine host line model and WCH GPIO evidence

Pinned CMSIS-DAP `Firmware/Source/SW_DP.c` implements SWJ/SWD sequences and the bit-level `SWD_Transfer()` algorithm. The host test routes `PIN_SWCLK_TCK_*` and `PIN_SWDIO_*` macros through callbacks to a model that supplies ACK/data/parity bits and records outputs, clock-high calls, and direction changes. Four host cases cover read/write data and parity plus WAIT/FAULT ACK paths. The fast-clock delay is a no-op, so the 46 clock-high calls observed in a successful read are model-call counts, not frequency or timing measurements.

The WCH EVT GPIO header `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_gpio.h` (SHA-256 `c7f450ceaa501e4c5a912bc0429bca1540fa3c3f26e3fd55dfd908fad9b06183`) declares `GPIOB_ModeCfg(uint32_t pin, GPIOModeTypeDef mode)` and defines `GPIOB_SetBits`, `GPIOB_ResetBits`, and `GPIOB_ReadPortPin`. The implementation `EVT/EXAM/SRC/StdPeriphDriver/CH58x_gpio.c` (SHA-256 `c2c7c9b2518a2a56144f107776418911ae0849df523481ee7b999cbdfab01757`) contains the GPIOB mode-configuration implementation. MCU-001 assigns PB5/PB6 to SWDIO/SWCLK. These sources establish API spellings and port calls, but are not integrated into the project. Product driver mode, target voltage/protection, and GPIO switching timing remain open pending IF-001 electrical requirements and verification-board measurements.

### 8.2 CMSIS-DAP upstream pin and adaptation gaps

The official Arm CMSIS-DAP repository is pinned as a Git submodule at commit `12636590eec66fae2d1bba4518749426ad5a4595`. `Firmware/Include/DAP.h` in that commit declares `DAP_ProcessCommand()` and `DAP_ExecuteCommand()` and firmware version 2.1.2. This source implements CMSIS-DAP firmware; it is not the complete DAPLink firmware system. `Firmware/Config/DAP_config.h` is an example-project template: its CPU clock, JTAG capability, DAP packet size, and packet-buffer count defaults must not be used as DBG-C settings.

The submodule's `DAP.h` includes `cmsis_compiler.h`; that file is absent from both the CMSIS-DAP submodule and the supplied CH585EVT ZIP. Its `PIN_DELAY_SLOW()` uses Arm `subs %0,%0,#1` inline assembly outside the `__CC_ARM` branch, which WCH RISC-V GCC cannot assemble. Do not define `__CC_ARM`; the test script changes only the branch guard in a build-directory header copy to select the upstream C loop. A local `platform/ch585/cmsis_compiler.h` maps CMSIS inline, NOP, and weak-symbol macros using the WCH core header plus GCC/RISC-V compiler primitives. WCH GCC compiles this header check and test-configured `DAP.c`/`SW_DP.c` into RISC-V objects. DAP configuration still uses no-op test pins, and the objects are not linked into PoC firmware. This verifies compiler primitives only; it is not a completed product GPIO/clock HAL or CMSIS-DAP port, and proves no PB5/PB6 waveform, electrical timing, target ACK, or CH585M runtime behavior.
