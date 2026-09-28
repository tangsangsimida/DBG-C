# DBG-C Verification Specification

**Document ID:** DBG-C-TEST-001　**Version:** V0.5　**Status:** Test-plan draft; FIFO host behavior tests and board tests have not run

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
| FIFO-01 Initialization boundaries | Null FIFO, null storage, zero capacity, capacity 1, and a non-power-of-two capacity | Invalid inputs return nonzero and leave the object unusable; valid capacities initialize successfully | Not run |
| FIFO-02 Writes and full capacity | Write to empty, exact fill, write beyond capacity, and write while full | Return the accepted byte count; partial write is allowed when space is limited; unread bytes are never overwritten | Not run |
| FIFO-03 Reads and empty capacity | Read from empty, partial read, and read to empty | Return the number actually read and preserve FIFO byte order; empty reads do not change state | Not run |
| FIFO-04 Wraparound | Use a non-power-of-two capacity and split reads/writes so both indices pass the end | Order and count remain correct across wraparound with no out-of-bounds access | Not run |
| FIFO-05 Invalid data pointers | Nonzero length with a null read/write data pointer | Return 0 and leave FIFO state unchanged | Not run |
| FIFO-06 Clear and queries | Clear empty/nonempty FIFO; query capacity, used count, and free space; zero-length read/write | Queries match state; clear discards contents but preserves capacity; zero-length operations do not change state | Not run |

This FIFO has no concurrency-safety guarantee, so no multi-thread/ISR concurrency pass case is defined. The repository has no existing host test runner. This turn followed the requirement to run only existing relevant build commands, so these cases were not executed; run them later in an established host-verification flow and record the command and results.

## 9. Current Execution Status

The repository contains the CH585M datasheet, CH585EVT archive, and ThreadX PoC-1. A host cross-build log for the PoC is recorded in FW-001; the user confirms no hardware is currently available, so no CH585M programming/debug or runtime evidence exists. DBG-C product firmware, cable samples, and captures are also unavailable. All product-verification cases in this document are **Not run**; the PoC host build does not count as board or product-function testing. Build and ELF static checks are complete, but the tick-target conflict is unresolved and verification-hardware design is not released; board runtime remains Not run and product hardware freeze is not released. ThreadX startup, interrupt, tick, thread switch/sleep/wakeup, clock measurement, reset recovery, and sustained-runtime cases remain Not run. Define duration, repetitions, load, and thresholds in the specific test plan before execution. See OPEN-001 O18/O19 for software and board release gates. This document defines coverage and is not a board verification report.
