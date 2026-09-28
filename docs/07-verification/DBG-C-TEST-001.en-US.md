# DBG-C Verification Specification

**Document ID:** DBG-C-TEST-001　**Version:** V0.27　**Status:** Test-plan draft; FIFO, CMSIS-DAP command-core, upstream SWD-engine host model, CMSIS compiler mapping, and CH585 SWD GPIO and Target Reset GPIO host register-model/target-object checks executed; upstream command-request bounds have not been verified; ThreadX board tests have not run

## 1. Pass Criteria

Record DUT hardware/firmware version, Host OS/tool version, Target board/MCU, wiring/cable, environment, steps, expected and actual results, logs/captures, and Pass/Fail/Blocked for every case. Mark unrun cases “Not run”; do not mark Pass without evidence. Quantitative limits remain open until the relevant specifications are frozen.

## 2. USB

Verify enumeration, descriptors, CMSIS-DAP v2 transport, CDC Control/Data, hot-plug/reconnect, and identification of multiple connected DBG-C units. Cover Windows/WinUSB/CDC, Linux/udev, and macOS. VID/PID, endpoint layout, packet sizes, and descriptors remain for USB specification/implementation freeze.

## 3. SWD and Target

Cover Connect, ID read, memory read/write, erase, Program/Verify, Reset, breakpoints, single-step, registers, and memory access. Select at least one concrete STM32 and GD32 part/board; exact parts remain to be fixed with test fixtures. Check SWD frequency range, signal quality, target power loss/removal, and error reporting.

## 4. UART

Verify RX, TX, full-duplex, multiple baud rates, sustained high load, overflow/flow-control behavior, and concurrency with wireless Debug/UART. Baud rates and throughput limits remain for review.

## 5. 2.4 GHz

Pair/unpair/re-pair, role setup, reconnect; shielding/interference/distance changes, loss/duplicate/reorder/power loss; measure latency distribution, throughput, retries, and error rate; download, online Debug, concurrent UART, and sustained operation. After protocol freeze, test field boundaries, CRC/authentication, versions, and malformed frames.

## 6. BLE

Discovery, device information, Pair/Unpair, configuration, status, OTA, interrupted-transfer recovery, compatibility, and authorization. Cover the selected PC Bluetooth adapter/OS matrix; matrix remains open.

## 7. Stress and Fault Injection

Continuous programming/Debug, long-duration RF, USB plug/unplug, target plug/unplug, RF loss, target power loss, and DBG-C reboot. Check for no silent false success or dangerous command replay and recovery to the defined state. Duration, repetitions, and thresholds remain open.

## 8. Generic Byte FIFO Host Test Plan

Test `software/common/byte_fifo/` against the public header contract. These cases cover pure C data behavior only; they do not cover concurrency, ISR, USB/UART/RF, or actual MCU SRAM.

| Case | Check | Pass condition | Status |
|---|---|---|---|
| FIFO-01 Initialization boundaries | Null FIFO, null storage, zero capacity, capacity 1 initialization plus single-byte fill/reject/readback, and a non-power-of-two capacity | Invalid inputs return nonzero and leave the object unusable; valid capacities initialize successfully; capacity 1 rejects an extra byte when full and reads the stored byte back correctly | Pass |
| FIFO-02 Writes and full capacity | Write to empty, exact fill, write beyond capacity, and write while full | Return the accepted byte count; partial write is allowed when space is limited; unread bytes are never overwritten | Pass |
| FIFO-03 Reads and empty capacity | Read from empty, partial read, and read to empty | Return the number actually read and preserve FIFO byte order; empty reads do not change state | Pass |
| FIFO-04 Wraparound | Use a non-power-of-two capacity and operations that wrap both read and write indices | FIFO order/count remain correct and the guard bytes around the backing storage remain intact | Pass |
| FIFO-05 Invalid data pointers | Nonzero length with a null read/write data pointer | Return 0 and leave FIFO state unchanged | Pass |
| FIFO-06 Clear and queries | Clear empty/nonempty FIFO; query capacity, used count, and free space; zero-length read/write | Queries match state; clear discards contents but preserves capacity; zero-length operations do not change state | Pass |
| FIFO-07 Deterministic state sequence | Run 512 deterministic write, read, query, and periodic-clear iterations with capacity 7 | Per-step accepted/read counts, byte order, used/free counts match a reference queue; backing-storage guard bytes remain intact | Pass |

This FIFO has no concurrency-safety guarantee, so no multi-thread/ISR concurrency pass case is defined. The seven host case groups are integrated into the existing `software/poc1-ch585-threadx/build.sh`; the run reported `PASS: 3683 byte FIFO checks`, with command and toolchain evidence in `software/poc1-ch585-threadx/build-evidence.log`. This proves only pure C FIFO data behavior on the current host, not ThreadX concurrency, ISR safety, CH585M SRAM behavior, or board behavior.

## 9. CMSIS-DAP Command-Core Host Checks

The test target is `Firmware/Source/DAP.c` from the pinned upstream commit, called through `DAP_ExecuteCommand()`. The configuration under `software/common/cmsis_dap_host_test/` is test-only and is not the DBG-C product configuration; SWD command processing is enabled and JTAG is disabled. SWD pin operations are no-ops, and `SWD_Transfer()` is mocked. The build copies `DAP.h` into `software/poc1-ch585-threadx/build/host-tests/` and changes only its delay-branch selection condition. It does not define `__CC_ARM` or modify the third-party submodule.

| Case | Request/check | Pass condition | Status |
|---|---|---|---|
| DAP-HOST-01 | `ID_DAP_Info` for `DAP_ID_DAP_FW_VER` | Response command, length, and firmware-version string match the pinned upstream implementation | Pass |
| DAP-HOST-02 | `ID_DAP_Info` for undefined information identifier `0xA0` | Response length is 2 and information length is 0 | Pass |
| DAP-HOST-03 | Unsupported command `0x30` | Returns `ID_DAP_Invalid` with response length 1 | Pass |
| DAP-HOST-04 | Single DP read; mock returns WAIT twice, then OK and data | Upstream command layer retries twice and returns correct count, status, and little-endian data | Pass |
| DAP-HOST-05 | DP write followed by write-completion check | Write value reaches the transaction mock, followed by `DP_RDBUFF | DAP_TRANSFER_RnW`, with correct response count/status | Pass |
| DAP-HOST-06 | Two consecutive AP reads; mock returns different data for posted read and RDBUFF | Data returns in request order; call sequence contains both AP reads and final `DP_RDBUFF | DAP_TRANSFER_RnW` | Pass |
| DAP-HOST-07 | Configure `retry_count=1`, then return WAIT twice for a DP read | Configure responds OK; following read performs one retry, returns WAIT, and calls the transaction mock twice | Pass |
| DAP-HOST-08 | Execute AP `ID_DAP_TransferBlock` with two read transfers | Return two little-endian values in request order; transaction sequence ends with `DP_RDBUFF | DAP_TRANSFER_RnW` | Pass |

These eight cases cover the pinned command layer, including TransferConfigure retry-count application, AP posted reads in both Transfer and TransferBlock, DP/AP read/write handling, and WAIT retries across a mocked physical-transaction boundary. They do not verify GPIO, SWD electrical timing, DAP packet bounds, CMSIS-DAP v2 USB transport, or CH585M behavior. The build script separately compiles the SWD-enabled test configuration to an ELF32 RISC-V object with WCH RISC-V GCC; pin macros are no-ops and transactions are mocked. The object is not linked into PoC or run on CH585M. These checks do not replace product cases in Sections 2–7.

## 9.1 CMSIS-DAP Upstream SWD-Engine Host Line Model

The test compiles pinned upstream `Firmware/Source/SW_DP.c` with its `DAP.c` dependency. Test pin macros call a pure host bit-stream model that supplies ACK, data, and parity inputs and records output bits, clock-high calls, and direction changes. The model fast delay is a no-op; call counts are not clock-frequency or timing measurements.

| Case | Check | Pass condition | Status |
|---|---|---|---|
| SWD-ENG-01 | SWD read request, ACK=OK, 32-bit data, and valid parity | Request bit order, returned data, direction changes, final clock-line state, and model call counts match fixture expectations | Pass |
| SWD-ENG-02 | SWD read returns invalid parity | Engine reports `DAP_TRANSFER_ERROR`; captured data, direction changes, and model call counts match fixture expectations | Pass |
| SWD-ENG-03 | SWD write `0x89ABCDEF` | Request bit order, 32-bit LSB-first data, data parity, direction changes, and model call counts match fixture expectations | Pass |
| SWD-ENG-04 | SWD read returns WAIT and FAULT ACKs | Both ACKs are returned unchanged; model input consumption, direction changes, idle-line state, and call counts match fixture expectations | Pass |
| SWD-ENG-05 | 10-bit SWD output and input sequences | Output is least-significant-bit first; input is packed into two bytes by the upstream algorithm; bit count and modeled clock calls match expectations | Pass |
| SWD-ENG-06 | Single DP IDCODE `DAP_Transfer` read after `DAP_Connect` selects SWD | End-to-end upstream command and bit-level engine model returns the expected response length, command, transfer status, and little-endian data | Pass |
| SWD-ENG-07 | Single AP `DAP_Transfer` read after `DAP_Connect` selects SWD, with different modeled AP-posted and later RDBUFF data | Both SWD transactions are consumed by the model; the DAP response contains RDBUFF-stage data with correct length, status, and little-endian order | Pass |
| SWD-ENG-08 | Two AP reads through `DAP_TransferBlock` after `DAP_Connect` selects SWD | The model consumes the initial posted read, subsequent AP read, and final RDBUFF transaction; DAP returns both values in little-endian order with the expected count and status | Pass |
| SWD-ENG-09 | `DAP_SWD_Sequence` command with a 10-bit SWD output sequence and a 10-bit input sequence | Response length and success status, output bit order, input-byte packing, modeled clock calls, and direction-change callbacks match expectations | Pass |

WCH RISC-V GCC also compiles upstream `SW_DP.c` with the test configuration to an ELF32 RISC-V object. The nine host cases contain 168 assertions. Neither these cases nor target-object compilation uses WCH GPIO, USB, ThreadX, or CH585M; the object is not linked into product firmware. These checks cover pinned upstream bit-level logic against a simulated line model. They do not verify PB5/PB6 waveforms, voltage, SWD frequency, setup/hold timing, or Target electrical behavior.

## 10. Current Execution Status

The repository contains the CH585M datasheet, CH585EVT archive, and ThreadX PoC-1. A host cross-build log for the PoC is recorded in FW-001; the user confirms no hardware is currently available, so no CH585M programming/debug or runtime evidence exists. DBG-C product firmware, cable samples, and captures are also unavailable. All product-level verification cases remain **Not run**. The seven FIFO host case groups, eight CMSIS-DAP command-core host checks, nine CMSIS-DAP command-to-SWD-engine integration line-model checks, two CMSIS compiler-mapping target-object checks, 57 SWD GPIO BSP modeled-register host checks, 33 Target Reset GPIO BSP modeled-register host checks, and two GPIO BSP target-object compile checks passed, but none count as board or product-function tests; neither does the PoC cross-build count as board runtime evidence. Build, ELF/link checks, and source static review are complete; the verification-board design gate has passed for limited-purpose verification hardware. This gate requires reproducible clean builds, ELF/link/resource checks, and static review of startup, context, and interrupt paths; it only permits a verification board and does not release product schematic or PCB freeze. Board runtime remains Not run, and product-hardware freeze is not released. ThreadX startup, interrupt entry/exit, SysTick frequency and tick delivery, thread switch/sleep/wakeup, stack integrity, reset recovery, and sustained-runtime cases remain Not run. Define duration, repetitions, load, and pass thresholds in the specific test plan before execution. See OPEN-001 O18/O19 for the release gates. This document defines coverage and is not a board verification report.

### 10.1 CMSIS and CH585 GPIO Host/Target Checks

| Case | Check | Pass condition | Status |
|---|---|---|---|
| CMSIS-COMP-01 | Compile the `platform/ch585/cmsis_compiler.h` check object with WCH RISC-V GCC 12.2.0 and inspect its ELF symbol table and disassembly | Object is ELF32 RISC-V; the weak-symbol check function is marked `WEAK`; disassembly contains RISC-V `nop` | Pass |
| CMSIS-COMP-02 | Compile test-configured pinned upstream `DAP.c` and `SW_DP.c` with WCH RISC-V GCC and this compiler header | Both separate objects are ELF32 RISC-V, with no Arm ISA assembly in the object-compilation path | Pass |
| CH585-GPIO-HOST-01 | Compile and run `dbgc_ch585_swd_gpio.c` against modeled registers; check all SWDIO/SWCLK modes, reads/writes, and invalid arguments | Modeled register bits match the WCH EVT GPIOB mode implementation; 57 checks pass | Pass (host register model only) |
| CH585-GPIO-OBJ-01 | Compile `platform/ch585/dbgc_ch585_swd_gpio.c` with WCH RISC-V GCC and inspect the object ELF header | `-Werror` compile succeeds and the target object is ELF32 RISC-V | Pass (target-object compile only) |
| CH585-RESET-HOST-01 | Compile and run the Target Reset GPIO BSP against modeled registers; check five modes, raw-level reads/writes, and invalid arguments | PA4 modeled-register changes match the WCH EVT GPIOA implementation; 33 checks pass | Pass (host register model only) |
| CH585-RESET-OBJ-01 | Compile `platform/ch585/dbgc_ch585_target_reset_gpio.c` with WCH RISC-V GCC and inspect its ELF header | `-Werror` compile succeeds and the target object is ELF32 RISC-V | Pass (target-object compile only) |

The SWD and Target Reset GPIO host checks use ordinary variables to model GPIOB/GPIOA registers and verify the BSP source bit operations; they do not verify CH585M register/pin behavior, electrical output, target reset effect, or timing. The Target Reset API passes through raw high/low levels and implements no active-level mapping or pulse policy. Target-object checks do not execute the objects. The CMSIS-DAP and GPIO objects are not linked into PoC, and these cases do not prove product DAP configuration, USB, ThreadX, or CH585M board behavior. To compile CMSIS-DAP, the build script selects the upstream C-loop delay branch in a build-directory copy of `DAP.h`; the SWD timing of that C loop has not been calibrated on hardware.

Reproduction command from the repository root: `DBGC_BUILD_DIR=build/target-reset-host-model software/poc1-ch585-threadx/build.sh`. The `host-tests/test_ch585_swd_gpio` and `host-tests/test_ch585_target_reset_gpio` executables run the SWD GPIO and Target Reset GPIO modeled-register checks; `target-tests/dbgc_ch585_swd_gpio.o` and `target-tests/dbgc_ch585_target_reset_gpio.o` are not linked into PoC.

### 10.2 CMSIS-DAP Request-Length Safety Checks

| Check | Required coverage | Status |
|---|---|---|
| DAP-GUARD-01 | Minimum/maximum input lengths for all enabled commands and vendor path, `DAP_ExecuteCommands` count/subcommand truncation, variable-field truncation, integer overflow, and packet-capacity bounds | Not run; bounds adapter is not implemented, and DAP_PACKET_SIZE/USB receive contract are not frozen |
| DAP-GUARD-02 | Worst-case response/write lengths and output-buffer bounds for read `DAP_Transfer`, `DAP_TransferBlock`, `DAP_SWD_Sequence`, `DAP_ExecuteCommands`, vendor commands, and each Info string callback | Not run; product response capacity and maximum string-info callback output are undefined |

The upstream command APIs receive no request length or response capacity; `DAP_ExecuteCommands` follows a request count, vendor handling can be overridden, and Info string callbacks receive no capacity. Host-fixture configuration does not define product configuration. The existing eight command checks use complete arrays and do not prove truncated-packet safety. Freeze the product command/callback/USB-buffer contract and implement the bounds adapter before executing these cases; record exact capacities. See OPEN-001 O21.
