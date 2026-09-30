# DBG-C Verification Specification

**Document ID:** DBG-C-TEST-001　**Version:** V0.71　**Status:** Test-plan draft; PB8/PB9 UI GPIO adapter passes 56 modeled-register host checks; SPI1 configuration BSP passes 19 modeled-register checks and PA0–PA3 GPIO modes and raw PA3 chip-select BSP pass 104 modeled-register checks; SPI transfer is not implemented; SWD-engine host line model now runs through the CH585 GPIO BSP modeled-register path with 3321 assertions; the generic Target Reset sequence service passes 24 callback host checks and the PB5 reset-sequence adapter passes 37 modeled-register host checks; the PB5 GPIO BSP passes 69 modeled-register host checks; the CH585 JTAG GPIO BSP passes 116 modeled-register checks, and upstream CMSIS-DAP JTAG Sequence, IDCODE, and DP Transfer write/posted-read paths through the BSP pass 1610 cumulative modeled-register host checks; fixed-slot packet queue (54 host checks), update transaction manager (57 host checks), FIFO, single/duplex byte-stream bridges, CMSIS-DAP command-core, upstream SWD-engine host model, bounded dispatch, bounds preflight, CMSIS compiler mapping, and CH585 SWD GPIO, Target Reset/PB6 Target Power GPIO, UART0 and UID-read adapter host and target relocatable-link checks executed; PoC ThreadX creation-status/tick-observation code is included in the target cross-build; the CH585 BSP/adapter static library is included in the PoC cross-build; product UART/command bounds, silicon UID, and ThreadX board tests have not run

## 1. Pass Criteria

Record DUT hardware/firmware version, Host OS/tool version, Target board/MCU, wiring/cable, environment, steps, expected and actual results, logs/captures, and Pass/Fail/Blocked for every case. Mark unrun cases “Not run”; do not mark Pass without evidence. Quantitative limits remain open until the relevant specifications are frozen.

## 2. USB

Verify enumeration, descriptors, CMSIS-DAP v2 transport, CDC Control/Data, hot-plug/reconnect, and identification of multiple connected DBG-C units. Cover Windows/WinUSB/CDC, Linux/udev, and macOS. VID/PID, endpoint layout, packet sizes, and descriptors remain for USB specification/implementation freeze.

## 3. SWD and Target

MCU-001 V0.11 assigns Target SWD/JTAG to PB0–PB3 and Target_nRESET to PB5. The PoC BSP and modeled-register tests now map SWDIO/SWCLK to PB1/PB0 and Target_nRESET to PB5. These checks do not verify physical pins, waveforms, voltage, or reset effect; no board is available and board testing has not run.

Cover Connect, ID read, memory read/write, erase, Program/Verify, Reset, breakpoints, single-step, registers, and memory access. Select at least one concrete STM32 and GD32 part/board; exact parts remain to be fixed with test fixtures. Check SWD frequency range, signal quality, target power loss/removal, and error reporting.

### 3.1 JTAG

Using the final CMSIS-DAP configuration and MCU-001 PB0–PB3 nets, verify JTAG connect, TAP reset/sequences, DP/AP reads/writes, program/verify, Target Reset, breakpoints, single-step, and error recovery. Use frozen STM32 and GD32 target models. No target board is currently available; these cases have not run.

### 3.2 SWO, VTref, and Target Power

- SWO: Use a programmable SWO source over the frozen encoding/rate range; verify capture, host stream output, overflow indication, and concurrent DAP/UART traffic. Baud range and loss thresholds are not frozen.
- VTref/level adaptation: Apply calibrated 1.8 V and 3.3 V Target domains, covering valid thresholds, abnormal voltage, and all four Probe/Target power combinations. Check voltage, direction, isolation, back-feed current, and maximum SWD/JTAG/UART/SWO rate on every Target digital line. Record ADC readings, error, protection nodes, and waveforms. Component-level thresholds/accuracy remain TBD; board tests have not run.
- Target power control: Verify startup default, switching, no-load/rated-load/short-circuit conditions, reverse power, hotplug, and fault recovery. Keep pending until thresholds and circuit are frozen.

### 3.3 USB Self-Update and External SPI NOR

USBHS self-update covers valid images, version rejection, corruption, interrupted transfer, power loss before/after commit, Boot recovery, and old-image startup. External NOR tests cover identification, erase/write boundaries, image integrity, power-loss state, and media faults. Partition, part, and image format tests follow actual link maps, part selection, and Update Manager specification.

### 3.4 Generic Update Transaction Manager Host Checks

The test target is `software/common/update_manager/`. Its backend uses host-model callbacks and does not access CH585M Flash, external SPI NOR, USB, RF, BLE, ThreadX, or interrupts. The 57 assertions belong to one host-test suite; the rows below classify coverage and do not represent separate test processes.

| Case | Check | Pass condition | Status |
|---|---|---|---|
| UPDATE-HOST-01 | Write a complete image at contiguous offsets, then verify and commit | State transitions through RECEIVING, VERIFIED, and COMMITTED; backend receives correct offsets, lengths, and byte order | Pass (covered by the 57-assertion host suite) |
| UPDATE-HOST-02 | Empty image, zero-length chunk, null data, skipped offset, and chunk beyond declared image length | Matching argument/range error is returned; invalid chunk does not call backend write and received length is unchanged | Pass (covered by the 57-assertion host suite) |
| UPDATE-HOST-03 | Invoke disallowed operations before begin, before complete reception, and after verified/committed/aborted/failed states | Invalid transitions are rejected without repeating backend phase callbacks | Pass (covered by the 57-assertion host suite) |
| UPDATE-HOST-04 | Fail each backend prepare, write, verify, commit, and abort operation | State and status match the API contract; failed chunk is not replayed; failed commit enters COMMIT_UNCERTAIN and rejects abort/new transactions | Pass (covered by the 57-assertion host suite) |

These checks verify transaction orchestration and callback contracts only. They do not verify image algorithms, Flash erase/program behavior, commit atomicity, power-loss recovery, rollback, OTA transport, or board behavior. Hardware and product acceptance must still run the USB self-update and external SPI NOR cases above.

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
| TX-POC-04 | Measure SysTick externally and compare with `TX_TIMER_TICKS_PER_SECOND` | Measured frequency matches the decided 1000 ticks/s PoC target and the tolerance frozen in the test plan before execution | Not run |
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

This FIFO has no concurrency-safety guarantee, so no multi-thread/ISR concurrency pass case is defined. The seven host case groups are integrated into the existing `python3 software/tools/python/build_poc1.py`; the run reported `PASS: 3683 byte FIFO checks`, with command and toolchain evidence in the local build-directory log, which is not version-controlled. This proves only pure C FIFO data behavior on the current host, not ThreadX concurrency, ISR safety, CH585M SRAM behavior, or board behavior.

## 8.1 Scheduler-Free Byte-Stream Bridge Host Checks

The test target is `software/common/byte_stream_bridge/`. It reuses the generic byte FIFO and represents source and sink endpoints through explicit one-byte callbacks. Each service call is bounded by a maximum forwarded-byte count. The tests use no USB stack, UART registers, interrupts, or ThreadX.

| Case | Check | Pass condition | Status |
|---|---|---|---|
| BYTE-BRIDGE-HOST-01 | Check source-to-sink order, service budget, pending-byte retention and FIFO buffering under sink backpressure, source/sink errors, zero budget, and invalid initialization arguments | 15 callback-model checks pass; bytes not accepted by the sink remain in the channel or FIFO | Pass (host callback model only) |

The channel is a caller-polled primitive for one serialized execution context. It provides no concurrency protection and is not connected to a product CDC/UART path. The target static library compiles in the existing PoC build, but the PoC application does not call the channel. Host checks do not establish USB CDC or UART behavior, product throughput under sustained backpressure, or ThreadX/ISR integration.

Reproduction command from the repository root: `python3 software/tools/python/build_poc1.py --build-dir build/byte-stream-bridge`.

### 8.2 Scheduler-free Duplex Byte-Stream Bridge Host Checks

The test target is `software/common/byte_duplex_bridge/`, which composes two existing single-channel streams. Endpoints are test callbacks; directions use independent FIFOs and caller-provided service budgets. The service processes directions in a fixed order, and backpressure on one path does not prevent servicing the other. It provides no scheduler, concurrency protection, or USB/UART driver.

| Case | Check | Pass condition | Status |
|---|---|---|---|
| BYTE-DUPLEX-HOST-01 | Check duplex initialization, independent budgets, byte order in both directions, one-way write backpressure and recovery, zero budgets, fail-fast endpoint errors, null object, and shared-FIFO rejection | 15 callback-model checks pass | Pass (host callback model only) |

Reproduction command from the repository root: `python3 software/tools/python/build_poc1.py --build-dir build/duplex-bridge`. This command also cross-builds the ThreadX PoC, but the PoC application does not call the duplex service; neither the build nor host model proves CH585M interrupts, scheduling, USB CDC, UART, electrical, or board behavior.


## 8.3 Generic Fixed-Slot Packet Queue Host Checks

The test target is `software/common/packet_queue/`. Slot size, count, and backing memory are supplied by the caller; this module selects no product transport packet size or queue depth. Tests cover pure C queue semantics only, not concurrency, ISR, USB/RF/BLE, or MCU SRAM.

| Case | Check | Pass condition | Status |
|---|---|---|---|
| PACKET-QUEUE-HOST-01 | Initialization boundaries, multiplication overflow, empty queue, FIFO order, wraparound, full queue, oversized records, record preservation on undersized output, zero-length records, and clear | All 54 native host checks pass | Pass (native C99 host only) |

The queue is not thread-safe; callers must serialize all access. Reproduction command: `python3 software/tools/python/build_poc1.py --build-dir build/packet-queue-final`. Host checks do not prove product capacity selection, Transport integration, ThreadX/ISR behavior, or board behavior.

## 9. CMSIS-DAP Command-Core Host Checks

The test target is `Firmware/Source/DAP.c` from the pinned upstream commit, called through `DAP_ExecuteCommand()`. The configuration under `software/common/cmsis_dap_host_test/` is test-only and is not the DBG-C product configuration; SWD command processing is enabled and JTAG is disabled. SWD pin operations are no-ops, and `SWD_Transfer()` is mocked. The build copies `DAP.h` into `software/build/archive/poc1-ch585-threadx/host-tests/` and changes only its delay-branch selection condition. It does not define `__CC_ARM` or modify the third-party submodule.

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

The test compiles pinned upstream `Firmware/Source/SW_DP.c` with its `DAP.c` dependency. Test pin macros call callbacks that invoke the actual `dbgc_ch585_swd_gpio.c` BSP API; modeled GPIOB registers supply PB1 input and check PB1/PB0 write masks. The bit-stream model supplies ACK, data, and parity inputs and records output bits, clock-high calls, and direction changes. `DBGC_CH585_GPIO_OUTPUT_PP_5MA` and `DBGC_CH585_GPIO_INPUT_FLOATING` are explicit host-fixture modes only, not a product electrical policy. The model fast delay is a no-op; call counts are not clock-frequency or timing measurements.

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
| SWD-ENG-10 | DP `DAP_Transfer` write of `0x89ABCDEF` after `DAP_Connect` selects SWD; the script supplies write ACK and final RDBUFF response | DAP response succeeds; SWD write-data bit order, parity, request length, and bus-direction callbacks match fixture expectations | Pass (host model) |

WCH RISC-V GCC also compiles upstream `SW_DP.c` with the test configuration to an ELF32 RISC-V object. The ten host cases contain 3321 assertions and call the CH585 SWD GPIO BSP source against ordinary host variables modeling GPIOB registers. The target object is not linked into product firmware. These checks cover the software call path through the BSP and modeled register mapping; they do not verify silicon GPIO, voltage, PB1/PB0 waveform, SWD frequency, setup/hold timing, Target electrical behavior, USB, ThreadX, or CH585M board runtime. Reproduction command: `python3 software/tools/python/build_poc1.py --build-dir build/ch585-gpio-swd-integration`.

## 9.2 CMSIS-DAP Upstream JTAG Sequence Host Register Model

The test compiles pinned upstream `Firmware/Source/DAP.c` and `JTAG_DP.c` and executes `ID_DAP_JTAG_Sequence` through `DAP_ExecuteCommand()`. Its test-only configuration disables SWD, enables JTAG, and reserves one JTAG device configuration slot. Test callbacks call the `dbgc_ch585_jtag_gpio.c` BSP API against ordinary host variables modeling GPIOB registers. The fixture explicitly configures PB0/TCK, PB1/TMS, and PB2/TDI as push-pull 5 mA outputs and PB3/TDO as a floating input; these are test modes, not product electrical policy.

| Case | Check | Pass condition | Status |
|---|---|---|---|
| JTAG-ENG-01 | One 8-bit `ID_DAP_JTAG_Sequence` with TMS high and TDO capture, TDI request `0xA5`, and scripted TDO `0x96` | CMSIS-DAP response length/status/captured data, TDI bit order, PB0–PB3 masks, and eight clock rising/falling calls match fixture expectations | Pass (88 host checks) |
| JTAG-ENG-02 | `DAP_Connect` selects JTAG, configures one device with a 4-bit IR, then issues `ID_DAP_JTAG_IDCODE`; TDO queue supplies scripted value `0x2BA01477` | Returns the IDCODE command success status and scripted value in little-endian order; exactly 32 TDO bits are read; the Sequence/IDCODE subset passed 385 checks | Pass (host model) |
| JTAG-ENG-03 | Send one DP write and one read through `ID_DAP_Transfer`; the TDO queue supplies ACK, posted data, and RDBUFF data | Response status and little-endian data match the script; TDI write-data bit order is correct; posted-read/RDBUFF completes; the full JTAG host model passes 1610 cumulative checks | Pass (host model) |

WCH RISC-V GCC also compiles the JTAG-enabled test configurations of upstream `DAP.c` and `JTAG_DP.c` into ELF32 RISC-V objects; they are not linked into product firmware. This verifies target-object compilation for the test profile. IDCODE and DP responses come from scripted queues rather than a physical device. The host model checks DP-write TDI ordering and the posted-read/RDBUFF command response path; it does not prove actual DP/AP access, a physical TAP chain, package pads, target electrical behavior, waveform/timing, USB Transport, or CH585M board runtime. The complete host/cross-build check passes 1610 cumulative JTAG assertions with the pinned source and toolchain. Reproduce from fish with: `set -gx DBGC_WCH_TOOLCHAIN_ROOT /path/to/RISC-V-Embedded-GCC12/bin; python3 software/tools/python/build_poc1.py --toolchain-root "$DBGC_WCH_TOOLCHAIN_ROOT" --build-dir build/jtag-transfer-host-model`.

## 10. Current Execution Status

The repository contains the CH585M datasheet, CH585EVT archive, and ThreadX PoC-1. The PoC host cross-build and toolchain summary is recorded in FW-001; raw build logs stay in local build directories and are not version-controlled; the user confirms no hardware is currently available, so no CH585M programming/debug or runtime evidence exists. DBG-C product firmware, cable samples, and captures are also unavailable. All product-level verification cases remain **Not run**. The fixed-slot packet queue passes 54 host checks. The seven FIFO host case groups, eight CMSIS-DAP command-core host checks, five bounded-dispatch host cases, ten CMSIS-DAP command-to-SWD-engine host line-model cases (including DP write, connected to the CH585 GPIO BSP modeled-register path), two CMSIS compiler-mapping target-object checks, 57 SWD GPIO BSP modeled-register host checks, 69 Target Reset GPIO BSP modeled-register host checks, 94 UART0 BSP/duplex-adapter modeled-register and callback checks, 31 UID-read adapter mock checks, 15 single-channel and 15 duplex byte-stream-bridge host checks, 24 generic Target Reset sequence callback checks, and the UID ROM-library relocatable-link, BSP/adapter target-object compile checks, and `dbgc_ch585_platform` static-library build passed, but none count as board or product-function tests; neither does the PoC cross-build count as board runtime evidence. Two clean builds at a fixed path for the 1000 ticks/s configuration produced matching ELF/map hashes; ELF/link resource review and source static review are complete, so the verification-board software gate is re-reviewed and passed. This gate requires reproducible clean builds, ELF/link/resource checks, and static review of startup, context, and interrupt paths; after that review, it only permits a verification board and does not release product schematic or PCB freeze. Board runtime remains Not run, and product-hardware freeze is not released. ThreadX startup, interrupt entry/exit, SysTick frequency and tick delivery, thread switch/sleep/wakeup, stack integrity, reset recovery, and sustained-runtime cases remain Not run. Define duration, repetitions, load, and pass thresholds in the specific test plan before execution. See OPEN-001 O18/O19 for the release gates. This document defines coverage and is not a board verification report.

### 10.1 CMSIS and CH585 GPIO Host/Target Checks

| Case | Check | Pass condition | Status |
|---|---|---|---|
| CMSIS-COMP-01 | Compile the `platform/ch585/cmsis_compiler.h` check object with WCH RISC-V GCC 12.2.0 and inspect its ELF symbol table and disassembly | Object is ELF32 RISC-V; the weak-symbol check function is marked `WEAK`; disassembly contains RISC-V `nop` | Pass |
| CMSIS-COMP-02 | Compile test-configured pinned upstream `DAP.c` and `SW_DP.c` with WCH RISC-V GCC and this compiler header | Both separate objects are ELF32 RISC-V, with no Arm ISA assembly in the object-compilation path | Pass |
| CH585-GPIO-HOST-01 | Compile and run `dbgc_ch585_swd_gpio.c` against modeled registers; check all SWDIO/SWCLK modes, reads/writes, and invalid arguments | Modeled register bits match the WCH EVT GPIOB mode implementation; 57 checks pass | Pass (host register model only) |
| CH585-GPIO-OBJ-01 | Compile `platform/ch585/dbgc_ch585_swd_gpio.c` with WCH RISC-V GCC and inspect the object ELF header | `-Werror` compile succeeds and the target object is ELF32 RISC-V | Pass (target-object compile only) |
| CH585-RESET-HOST-01 | Compile and run the Target Reset GPIO BSP against modeled registers; check five modes, raw-level reads/writes, and invalid arguments | Raw modes and reads/writes plus CMSIS-DAP nRESET bit mapping, low-latch-before-output ordering, caller-selected release mode, and invalid-argument rejection match the modeled WCH EVT GPIOB operations; 69 checks pass | Pass (host register model only) |
| CH585-RESET-OBJ-01 | Compile `platform/ch585/dbgc_ch585_target_reset_gpio.c` with WCH RISC-V GCC and inspect its ELF header | `-Werror` compile succeeds and the target object is ELF32 RISC-V | Pass (target-object compile only) |

The SWD and Target Reset GPIO host checks use ordinary variables to model GPIOB registers and verify the BSP source bit operations; they do not verify CH585M register/pin behavior, electrical output, target reset effect, or timing. The raw Target Reset GPIO API passes through high/low levels. A separate PB5 primitive maps CMSIS-DAP nRESET bit 0 to low assertion and bit 1 to input-mode release, with caller-selected push-pull and floating/pull-up input modes. This is software direction-switching emulation only; it does not prove external pull-up, level translation, electrical behavior, reset pulse, or board timing. Target-object checks do not execute the objects. The CMSIS-DAP and GPIO objects are not linked into PoC, and these cases do not prove product DAP configuration, USB, ThreadX, or CH585M board behavior. To compile CMSIS-DAP, the build script selects the upstream C-loop delay branch in a build-directory copy of `DAP.h`; the SWD timing of that C loop has not been calibrated on hardware.

Reproduction command from the repository root: `python3 software/tools/python/build_poc1.py --build-dir build/uart0-duplex-adapter`. This existing build entry runs the SWD GPIO, Target Reset GPIO, and UART0 host register models and compiles all three BSPs and the UART0 bridge adapter to RISC-V target objects; none of those objects is linked into PoC.

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

Reproduction command from the repository root: `python3 software/tools/python/build_poc1.py --build-dir build/uid-vendor-link`. These checks do not execute the chip ROM command or prove actual CH585M UID bytes, uniqueness, or stability. They define no product Device ID, authentication, or pairing-storage behavior.

### 10.2 CMSIS-DAP Request-Length Safety Checks

| Check | Required coverage | Status |
|---|---|---|
| DAP-GUARD-01 | Product-enabled pinned command and vendor-path input boundaries, truncation, integer ranges, and USB packet limit | Product-level Not run; product configuration/USB-length contract is not frozen, and vendor commands fail closed |
| DAP-GUARD-02 | Worst-case response writes and response-buffer limits under actual product callbacks/command configuration | Product-level Not run; product profile, callback bounds, and buffer capacities are undefined |
| DAP-BOUNDS-HOST-01 | Hardware-independent preflight: fixed/variable commands, Transfer/TransferBlock, SWD/JTAG Sequence, ExecuteCommands, truncation, capacity failure, packed-length limits, and fail-closed paths | Pass; 102 checks with an explicit test profile, not product configuration |
| DAP-DISPATCH-HOST-01 | Use pinned upstream `DAP_ExecuteCommand()` through bounded dispatch: reject truncated input and insufficient capacity, valid Connect, detect a length-contract mismatch, execute a multi-command packet, and consume the full SWD Transfer request after FAULT | Pass; five host cases with test profile, not product USB integration |
| DAP-BOUNDS-TARGET-OBJ-01 | Compile the bounds preflight with WCH RISC-V GCC and inspect object architecture | Pass; ELF32 RISC-V object, not linked into PoC/DAP |

Reproduction command from the repository root: `python3 software/tools/python/build_poc1.py --build-dir build/cmsis-dap-bounds-dispatch`. This existing build entry passes 102 preflight checks and five bounded-dispatch host cases, and compiles the module to an ELF32 RISC-V object; tests provide an explicit host profile. The dispatch host cases call pinned upstream `DAP_ExecuteCommand()`, but the wrapper is not connected to the product USB receive path. Upstream vendor, SWO, and CMSIS-DAP UART commands are rejected; Info checks depend on the caller supplying the actual maximum write size. These results do not prove product call-path coverage, actual USB packet length, callback behavior, or CH585M runtime. O21 remains open and product-level cases remain Not run.

### 10.3 CMSIS-DAP queued service host checks

The test covers `software/common/cmsis_dap_service/` with caller-provided fixed-slot request/response queues and buffers. Queue operations are serialized in one host-test context.

| Case | Coverage | Pass criteria | Status |
|---|---|---|---|
| DAP-SERVICE-HOST-01 | Complete request processing, empty queue, invalid objects, request retention when response queue is full, malformed/oversized request rejection, insufficient response capacity, upstream error, and dequeue after success | 40 host checks pass | Pass (host model only) |

The test does not include real USB reception, CMSIS-DAP Transport, ThreadX concurrency, or CH585M peripheral execution. Reproduce with `python3 software/tools/python/build_poc1.py --build-dir build/goal-dap-service`.

### 10.4 Generic Target Reset Sequence Host Checks

The test covers `software/common/target_reset_sequence/`. It only orders caller-provided assert, hold, and release callbacks; it accesses no GPIO and defines no PB5 level, reset polarity, hold duration, delay unit, or scheduling context.

| Check | Pass condition | Status |
|---|---|---|
| Success path | Callbacks run in assert, hold, release order and return success | Pass |
| Assertion failure | Skip hold, still call release, and return the assertion error | Pass |
| Hold failure | Still call release and return the hold error | Pass |
| Release failure | Return the release error; it takes precedence when hold and release both fail | Pass |
| Invalid operation table | Return an error without invoking callbacks | Pass |

All 24 host checks pass. This verifies only hardware-independent callback order and error propagation; it does not verify PB5, Target_nRESET levels, pulse width, GPIO electrical behavior, or CH585M board runtime. Reproduction command: `python3 software/tools/python/build_poc1.py --build-dir build/target-reset-sequence`.


### 10.5 CH585 PB5 Target Reset Sequence Integration Host Checks

The test covers `software/poc1-ch585-threadx/platform/ch585/dbgc_ch585_target_reset_sequence.c`, the PB5 GPIO BSP, and the generic sequence service. The host test calls the actual BSP source with ordinary variables modeling GPIOB registers. The hold callback checks that the assertion write occurred first; after return, the test checks the release write. Active-high and active-low are fixture parameters, not a product polarity decision; push-pull 5 mA is also only a fixture mode.

| Check | Pass condition | Status |
|---|---|---|
| Explicit asserted level | Both high and low fixture settings write the asserted value and then its inverse after the hold callback | Pass |
| Release after hold error | A hold callback error still results in release and is returned unchanged | Pass |
| Configuration validation | Null configuration, missing hold callback, or raw level outside 0/1 returns an error without GPIO access | Pass |

All 37 integration checks pass. They verify source call ordering and modeled-register operations only, not the physical PB5 pin, drive behavior, Target_nRESET polarity, pulse width, or CH585M board behavior. Before calling, the caller must place PB5 in the released output state under an approved electrical policy. The host test does not verify GPIO initialization transitions or target reset behavior. The caller supplies hold policy and scheduling context. Reproduction command: `python3 software/tools/python/build_poc1.py --build-dir build/reset-sequence-pb5-repro`.


### 10.6 CH585 Target JTAG GPIO BSP Host Checks

The test covers `software/poc1-ch585-threadx/platform/ch585/dbgc_ch585_jtag_gpio.c`. PB0/TCK, PB1/TMS, PB2/TDI, and PB3/TDO mappings come from MCU-001 V0.11 and CH585M datasheet Table 1-1; register operations follow the WCH EVT GPIOB implementation. Ordinary variables model GPIOB registers.

| Check | Coverage | Status |
|---|---|---|
| JTAG-GPIO-HOST-01 | Five input/output modes per signal, PB bit-mask reads/writes, and invalid signal/mode/null-output arguments | 116 modeled-register host checks pass |

This test does not select product electrical modes or verify physical pins, target voltage, electrical protection, JTAG waveform/timing, or CH585M board runtime. The BSP is not connected to the CMSIS-DAP JTAG command engine. Reproduce with `python3 software/tools/python/build_poc1.py --build-dir build/goal-jtag-gpio`.


### 10.7 CH585 PA4/A0 VTref ADC Raw-Sampling Host Checks

The test target is `software/poc1-ch585-threadx/platform/ch585/dbgc_ch585_vtref_adc.c`. Ordinary variables model PA4, ADC configuration, conversion status, and data registers using WCH EVT ADC/GPIO register definitions. The caller explicitly supplies sample clock and PGA gain; the poll limit counts status-register reads per stage and is not a time value. The test covers rejection of sample-clock option 0 when `R16_CLK_SYS_CFG[9]` is 1.

| Check | Coverage | Status |
|---|---|---|
| VTREF-ADC-HOST-01 | Uninitialized/invalid arguments, PA4 floating analog-input setup, ADC A0 setup, sample-clock constraint, first-conversion discard after initialization, continuation after bounded polling, and raw-data masking | 44 modeled-register host checks pass; the host model does not represent silicon ADC conversion |

This check does not verify ADC accuracy, actual sample-clock frequency, input range, divider/protection circuit, millivolt conversion, calibration, VTref thresholds, or board behavior. Product parameters remain unfrozen; no board is available and silicon sampling has not run. Reproduce with: `python3 software/tools/python/build_poc1.py --build-dir build/vtref-adc`.

### 10.8 CH585 SPI1 Configuration BSP Host Checks

The test target is `software/poc1-ch585-threadx/platform/ch585/dbgc_ch585_spi1_config.c`. Register and configuration semantics come from CH585/CH584 datasheet §§10.1.1/10.2 and the 2026.08 EVT `CH58x_spi1.c`. Ordinary variables model SPI1 control, configuration, clock-divider, and status registers.

| Check | Coverage | Status |
|---|---|---|
| SPI1-CONFIG-HOST-01 | Modes 0/3, MSB/LSB bit order, divider limits 2/254, input-delay bit, invalid arguments, and busy rejection | 19 modeled-register host checks pass |

The configuration BSP in this section configures controller registers only; PA0–PA3 GPIO modes and raw PA3 levels are covered by the separate BSP in Section 10.9. Neither BSP transfers data or implements Flash commands. The host checks do not prove divider-related waveforms, SPI electrical behavior, or CH585M board behavior. EVT transfer routines contain unbounded polling; datasheet Table 10-2 defines `RB_SPI_FIFO_READY` and `RB_SPI_FREE` only as FIFO-ready and SPI-idle status; it does not define an operation to abort shifting data or reset a transaction. `RB_SPI_ALL_CLEAR` clears FIFO, counters, and interrupt flags, which does not prove timeout recovery, so transfer support remains unimplemented pending that evidence. Reproduce with: `python3 software/tools/python/build_poc1.py --build-dir build/spi1-config`.

### 10.9 CH585 SPI1 GPIO and Raw Chip-Select BSP Host Checks

The test target is `software/poc1-ch585-threadx/platform/ch585/dbgc_ch585_spi1_gpio.c`. PA0/SCK1, PA1/MOSI1, and PA2/MISO1 come from datasheet Table 1-1; PA3 is the GPIO CS allocated by MCU-001. Mode-register writes follow WCH EVT `CH58x_gpio.c` `GPIOA_ModeCfg()`. The SPI1 branch in EVT `EVT/EXAM/SPI/src/Main.c`, currently excluded by `#if 1`, uses PA12 as CS; it does not verify the project PA3 chip-select configuration or transfer behavior.

| Check | Coverage | Status |
|---|---|---|
| SPI1-GPIO-HOST-01 | Mapping for all four GPIOs; five modes per pin; raw PA3 high/low writes; invalid pin and mode rejection | 104 modeled-register host checks pass |

The check covers raw PA3 output-latch high/low writes but does not define active polarity, default state, or timing. It does not prove board pin behavior, electrical compatibility, or SPI signal timing. Reproduce with: `python3 software/tools/python/build_poc1.py --build-dir build/spi1-gpio-cs`.

### 10.10 CH585 PB6 TARGET_PWR_EN Raw GPIO Host Checks

The test target is `software/poc1-ch585-threadx/platform/ch585/dbgc_ch585_target_power_gpio.c`. PB6/QFN48 pin 18 comes from MCU-001; GPIOB mode bits and `R32_PB_SET`, `R32_PB_CLR`, and `R32_PB_PIN` access follow WCH EVT `CH58x_gpio.c` and local `CH585SFR.h`. Ordinary variables model the registers.

| Check | Coverage | Status |
|---|---|---|
| TARGET-POWER-GPIO-HOST-01 | Five caller-selected modes, preservation of unrelated GPIO bits, raw PB6 high/low writes and reads, invalid mode, and null output pointer | 36 checks pass |

This test does not define active power polarity, default state, load-switch circuit, or protection policy, and does not verify the physical PB6 pin, power switch, target supply behavior, or CH585M board runtime. Reproduce with `python3 software/tools/python/build_poc1.py --toolchain-root /path/to/riscv-wch-elf/bin --build-dir build/target-power-gpio`; omit `--toolchain-root` if the toolchain is already available through PATH.

### 10.11 CH585 PB8/PB9 User-Interface GPIO Host Checks

The test target is `software/poc1-ch585-threadx/platform/ch585/dbgc_ch585_ui_gpio.c`. PB8=KEY_MODE and PB9=LED_TARGET come from MCU-001 and datasheet Table 1-1; GPIOB masks and mode-register operations follow WCH EVT `CH58x_gpio.h` and `CH58x_gpio.c`.

| Check | Coverage | Status |
|---|---|---|
| UI-GPIO-HOST-01 | PB8/PB9 mapping, register-bit isolation across five modes, raw high/low reads and writes, invalid signal/mode, and null output pointer | 56 checks pass |

This check does not define button polarity, debounce/wakeup policy, LED polarity/default state, or verify external circuitry or CH585M board runtime. PB18/PB19 are excluded because the EVT RF antenna-switch mux remains unresolved. Reproduce with `python3 software/tools/python/build_poc1.py --toolchain-root /path/to/riscv-wch-elf/bin --build-dir build/ui-gpio`.

## 11. Validation Board Hardware Bring-up and Gates

This section adds board-verification planning from HW-001 V0.3; it does not mean any board test has run. No board is currently available, so all items below are **Not run**.

| Stage/case | Observation or verification | Passing evidence | Current status |
|---|---|---|---|
| Pre-power inspection | Supply nets, shorts, polarity, DNI/0R state, recovery jumpers, Target isolation | Checklist, schematic/BOM revision, measurement record | Not run |
| Power/clocks | VBUS and MCU rails; 32 MHz oscillation; optional 32 kHz; reset startup | Scope/power readings, chip startup log, crystal/supply part numbers | Not run |
| Chip recovery | PB22/BOOT and PB23/RST sequence; USBFS and official UART recovery | Measurements matching WCH instructions, download log, cold-start recovery record | Not run; entry timing awaits official evidence |
| USBHS/USBFS | Independent enumeration, hot plug, recovery entry, dual-controller conflict | Host logs, descriptor/interface records, recovery with power loss/corrupt app | Not run; concurrency unconfirmed |
| Target interface | Isolated test access on both sides of the VTref-associated level-adaptation paths for PB0–PB5, PB20, and PA4 sensing; no direct CH585M-to-Target operating path | Pin/net continuity, power-state waveforms, protection/misconnection records | Not run; do not connect incompatible voltage |
| Target voltage/VTref/power | Cover 1.8 V/3.3 V and four Probe/Target power combinations; measure adaptation and powered-off isolation on each Target digital line plus VTref error, back-feed, current limit, short, and hot plug | Calibrated supply readings, scope traces, powered-off injection current, fault recovery, component temperature | Not run; translators, thresholds, and numerical criteria TBD |
| RF mux/RF | PB16–PB21 mux state; antenna network; separate BLE/RF mode operation | Configuration/register evidence, RF logs, RF measurements | Not run; prebuilt-library pin use unresolved |
| SPI NOR | SPI waveforms, device identification, boundary read/write, power-loss recovery | Part number, logic-analyzer record, image-integrity log | Not run; part/transfer implementation TBD |
| Product acceptance gate | ThreadX tick/interrupt/scheduling, DAP, SWD/JTAG/UART, OTA, and system cases | Complete versioned evidence under applicable sections and HW-001 gates | Not run; schematic completion cannot substitute |

### Schematic Gate Status

**Validation Board schematic design may start/continue**: the MCU-001 pin baseline and software-design gate are established. The MCU core, USBHS PHY, independent recovery interface, direct Target test access, and isolatable modules may be drawn. Keep Target voltage front end/power, BOOT/recovery timing, PB16–PB21 RF mux, RF matching, and custom Type-C interface isolated or pending where evidence is insufficient. PCB-layout and fabrication gates have not passed. This is not a Product Board freeze.
