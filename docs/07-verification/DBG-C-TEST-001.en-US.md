# DBG-C Verification Specification

**Document ID:** DBG-C-TEST-001　**Version:** V0.41　**Status:** Test-plan draft; FIFO, single/duplex byte-stream bridges, CMSIS-DAP command-core, upstream SWD-engine host model, bounded dispatch, bounds preflight, CMSIS compiler mapping, and CH585 SWD GPIO, Target Reset GPIO, UART0 and UID-read adapter host and target relocatable-link checks executed; PoC ThreadX creation-status/tick-observation code is included in the target cross-build; the CH585 BSP/adapter static library is included in the PoC cross-build; product UART/command bounds, silicon UID, and ThreadX board tests have not run

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

### 7.1 ThreadX PoC Board Runtime Checks

Run these checks on a CH585M verification board. Record the PoC ELF SHA-256, board revision, silicon identifier, toolchain/programmer versions, wiring, environment, sampling duration, repetitions, load, debugger observations, and reset/interrupt logs. Review and freeze sampling duration and pass thresholds before execution; these are currently undefined, so every case remains Not run.

| Case | Check | Pass condition | Status |
|---|---|---|---|
| TX-POC-01 | Inspect `thread_a_create_status` and `thread_b_create_status` | Both equal ThreadX `TX_SUCCESS`; record exact values | Not run |
| TX-POC-02 | Observe `thread_a_runs`, `thread_b_runs`, `thread_a_last_tick`, `thread_b_last_tick`, and `threadx_tick_observed` | Both run counters and per-thread tick samples advance within the pre-frozen duration/repetition thresholds, without unexplained stalls | Not run |
| TX-POC-03 | Observe thread resume after `tx_thread_sleep(1U)` and same-priority rotation | Both threads sleep, resume, and rotate under the pre-frozen observation criteria, without unexplained lost ticks or hangs | Not run |
| TX-POC-04 | Measure SysTick externally and compare with `TX_TIMER_TICKS_PER_SECOND` | Measured frequency matches the decided/frozen target and tolerance; resolve the source setting of 100 ticks/s versus the 1000 ticks/s requirement before freezing the target/tolerance | Not run |
| TX-POC-05 | Check interrupt entry/exit, context restore, and stack integrity during reset, sleep/wakeup, and sustained runtime | Duration, repetitions, load, and observation method are frozen first; no unexplained exception, stack damage, reset, or stall | Not run |

Observation symbols expose software state; they do not self-certify interrupt, tick, or stack correctness. A host cross-build proves target ELF build/link only and must not be recorded as a pass for any board case above.

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

## 8.1 Scheduler-Free Byte-Stream Bridge Host Checks

The test target is `software/common/byte_stream_bridge/`. It reuses the generic byte FIFO and represents source and sink endpoints through explicit one-byte callbacks. Each service call is bounded by a maximum forwarded-byte count. The tests use no USB stack, UART registers, interrupts, or ThreadX.

| Case | Check | Pass condition | Status |
|---|---|---|---|
| BYTE-BRIDGE-HOST-01 | Check source-to-sink order, service budget, pending-byte retention and FIFO buffering under sink backpressure, source/sink errors, zero budget, and invalid initialization arguments | 15 callback-model checks pass; bytes not accepted by the sink remain in the channel or FIFO | Pass (host callback model only) |

The channel is a caller-polled primitive for one serialized execution context. It provides no concurrency protection and is not connected to a product CDC/UART path. The target static library compiles in the existing PoC build, but the PoC application does not call the channel. Host checks do not establish USB CDC or UART behavior, product throughput under sustained backpressure, or ThreadX/ISR integration.

Reproduction command from the repository root: `DBGC_BUILD_DIR=build/byte-stream-bridge software/poc1-ch585-threadx/build.sh`.

### 8.2 Scheduler-free Duplex Byte-Stream Bridge Host Checks

The test target is `software/common/byte_duplex_bridge/`, which composes two existing single-channel streams. Endpoints are test callbacks; directions use independent FIFOs and caller-provided service budgets. The service processes directions in a fixed order, and backpressure on one path does not prevent servicing the other. It provides no scheduler, concurrency protection, or USB/UART driver.

| Case | Check | Pass condition | Status |
|---|---|---|---|
| BYTE-DUPLEX-HOST-01 | Check duplex initialization, independent budgets, byte order in both directions, one-way write backpressure and recovery, zero budgets, fail-fast endpoint errors, null object, and shared-FIFO rejection | 15 callback-model checks pass | Pass (host callback model only) |

Reproduction command from the repository root: `DBGC_BUILD_DIR=build/duplex-bridge software/poc1-ch585-threadx/build.sh`. This command also cross-builds the ThreadX PoC, but the PoC application does not call the duplex service; neither the build nor host model proves CH585M interrupts, scheduling, USB CDC, UART, electrical, or board behavior.


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

These eight cases cover the pinned command layer, including TransferConfigure retry-count application, AP posted reads in both Transfer and TransferBlock, DP/AP read/write handling, and WAIT retries across a mocked physical-transaction boundary. They do not verify GPIO, SWD electrical timing, CMSIS-DAP v2 USB transport, or CH585M behavior. Bounded-dispatch host cases are listed in Section 10.2. The build script separately compiles the SWD-enabled test configuration to an ELF32 RISC-V object with WCH RISC-V GCC; pin macros are no-ops and transactions are mocked. The object is not linked into PoC or run on CH585M. These checks do not replace product cases in Sections 2–7.

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

The repository contains the CH585M datasheet, CH585EVT archive, and ThreadX PoC-1. A host cross-build log for the PoC is recorded in FW-001; the user confirms no hardware is currently available, so no CH585M programming/debug or runtime evidence exists. DBG-C product firmware, cable samples, and captures are also unavailable. All product-level verification cases remain **Not run**. The seven FIFO host case groups, eight CMSIS-DAP command-core host checks, five bounded-dispatch host cases, nine CMSIS-DAP command-to-SWD-engine integration line-model checks, two CMSIS compiler-mapping target-object checks, 57 SWD GPIO BSP modeled-register host checks, 33 Target Reset GPIO BSP modeled-register host checks, 94 UART0 BSP/duplex-adapter modeled-register and callback checks, 31 UID-read adapter mock checks, 15 single-channel and 15 duplex byte-stream-bridge host checks, and the UID ROM-library relocatable-link, BSP/adapter target-object compile checks, and `dbgc_ch585_platform` static-library build passed, but none count as board or product-function tests; neither does the PoC cross-build count as board runtime evidence. Build, ELF/link checks, and source static review are complete; the verification-board design gate has passed for limited-purpose verification hardware. This gate requires reproducible clean builds, ELF/link/resource checks, and static review of startup, context, and interrupt paths; it only permits a verification board and does not release product schematic or PCB freeze. Board runtime remains Not run, and product-hardware freeze is not released. ThreadX startup, interrupt entry/exit, SysTick frequency and tick delivery, thread switch/sleep/wakeup, stack integrity, reset recovery, and sustained-runtime cases remain Not run. Define duration, repetitions, load, and pass thresholds in the specific test plan before execution. See OPEN-001 O18/O19 for the release gates. This document defines coverage and is not a board verification report.

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

Reproduction command from the repository root: `DBGC_BUILD_DIR=build/uart0-duplex-adapter software/poc1-ch585-threadx/build.sh`. This existing build entry runs the SWD GPIO, Target Reset GPIO, and UART0 host register models and compiles all three BSPs and the UART0 bridge adapter to RISC-V target objects; none of those objects is linked into PoC.

### CH585-UART0 Host Register Model and Target-Object Checks

| Case | Check | Pass condition | Status |
|---|---|---|---|
| CH585-UART0-HOST-01 | Compile and run the UART0 BSP against modeled registers; check explicit clock/baud divisor, PB4/PB7 mode and mux, all four FIFO trigger values, 40 valid LCR field combinations, invalid format values, eight pure WCH EVT divisor-function checks, IER/DIV setup, UART0 one-byte callback mapping, duplex transport-input-to-UART0-TX and UART0-RX-to-transport-writer composition, and nonblocking TX/RX | 94 checks match WCH EVT GPIO/UART source and local SFR definitions; bridge endpoints use mock callbacks; invalid initialization inputs are rejected before register writes | Pass (host register model only) |
| CH585-UART0-OBJ-01 | Compile `platform/ch585/dbgc_ch585_uart0.c` with WCH RISC-V GCC and inspect its object ELF header | `-Werror` compile succeeds and target object is ELF32 RISC-V | Pass (target-object compile only) |
| CH585-UART0-ADAPTER-OBJ-01 | Compile the UART0 bridge callback adapter with WCH RISC-V GCC and inspect the object ELF header | -Werror compile succeeds and the object is ELF32 RISC-V; it is not executed or linked into product firmware | Pass (target-object compile only) |

The register model does not simulate UART hardware FIFO read side effects; it checks register selection/writes, the receive-data read path, and full/empty branches only. It does not prove UART interrupts, real FIFO behavior, baud error, PB4/PB7 electrical behavior, serial data transfer, or ThreadX/CDC concurrency. The transport callback is a host-test substitute, not USB CDC. Product UART parameters and end-to-end tests in Section 4 remain Not run.

### CH585 UID-Read Adapter Host and Target-Object Checks

Source evidence is `EVT/EXAM/SRC/StdPeriphDriver/inc/ISP585.h`, `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_flash.h`, and `EVT/EXAM/SRC/StdPeriphDriver/CH58x_flash.c` in the WCH EVT archive. `ISP585.h` declares `uint32_t FLASH_EEPROM_CMD(uint8_t cmd, uint32_t StartAddr, void *Buffer, uint32_t Length)`, documents zero success/nonzero failure, and requires an aligned RAM Buffer. WCH `GET_UNIQUE_ID()` calls `CMD_GET_ROM_INFO` at `ROM_CFG_MAC_ADDR` with length zero, preserves the first six bytes, and writes a 16-bit sum into the final two bytes. That wrapper returns `void` and ignores the low-level status; DBG-C calls the status-returning ROM command directly.

| Case | Check | Pass condition | Status |
|---|---|---|---|
| CH585-UID-HOST-01 | Mock `FLASH_EEPROM_CMD()` to check command/address/length, destination alignment, six data bytes and the WCH checksum, rejection of null/short buffers, and unchanged caller output after command failure | 31 checks pass; ROM-command failure maps to an error return and caller memory remains unchanged | Pass (host mock) |
| CH585-UID-OBJ-01 | Compile the UID adapter with WCH RISC-V GCC and use `ld -r` with EVT `libISP585.a` to check symbol resolution | `-Werror` succeeds, target object is ELF32 RISC-V, and the included EVT library resolves `FLASH_EEPROM_CMD`; only `memcpy` remains for final C-library resolution | Pass (target-object and relocatable-link check only) |

Reproduction command from the repository root: `DBGC_BUILD_DIR=build/uid-vendor-link software/poc1-ch585-threadx/build.sh`. These checks do not execute the chip ROM command or prove actual CH585M UID bytes, uniqueness, or stability. They define no product Device ID, authentication, or pairing-storage behavior.

### 10.2 CMSIS-DAP Request-Length Safety Checks

| Check | Required coverage | Status |
|---|---|---|
| DAP-GUARD-01 | Product-enabled pinned command and vendor-path input boundaries, truncation, integer ranges, and USB packet limit | Product-level Not run; product configuration/USB-length contract is not frozen, and vendor commands fail closed |
| DAP-GUARD-02 | Worst-case response writes and response-buffer limits under actual product callbacks/command configuration | Product-level Not run; product profile, callback bounds, and buffer capacities are undefined |
| DAP-BOUNDS-HOST-01 | Hardware-independent preflight: fixed/variable commands, Transfer/TransferBlock, SWD/JTAG Sequence, ExecuteCommands, truncation, capacity failure, packed-length limits, and fail-closed paths | Pass; 102 checks with an explicit test profile, not product configuration |
| DAP-DISPATCH-HOST-01 | Use pinned upstream `DAP_ExecuteCommand()` through bounded dispatch: reject truncated input and insufficient capacity, valid Connect, detect a length-contract mismatch, execute a multi-command packet, and consume the full SWD Transfer request after FAULT | Pass; five host cases with test profile, not product USB integration |
| DAP-BOUNDS-TARGET-OBJ-01 | Compile the bounds preflight with WCH RISC-V GCC and inspect object architecture | Pass; ELF32 RISC-V object, not linked into PoC/DAP |

Reproduction command from the repository root: `DBGC_BUILD_DIR=build/cmsis-dap-bounds-dispatch software/poc1-ch585-threadx/build.sh`. This existing build entry passes 102 preflight checks and five bounded-dispatch host cases, and compiles the module to an ELF32 RISC-V object; tests provide an explicit host profile. The dispatch host cases call pinned upstream `DAP_ExecuteCommand()`, but the wrapper is not connected to the product USB receive path. Upstream vendor, SWO, and CMSIS-DAP UART commands are rejected; Info checks depend on the caller supplying the actual maximum write size. These results do not prove product call-path coverage, actual USB packet length, callback behavior, or CH585M runtime. O21 remains open and product-level cases remain Not run.
