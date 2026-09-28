# DBG-C Product Requirements Specification

**Document ID:** DBG-C-PRD-001　**Version:** V0.1 (Draft baseline)　**Status:** For review

## 1. Product Positioning

DBG-C Platform is an embedded MCU debug and programming platform with wired, private 2.4 GHz, and BLE capabilities. DBG-C Probe is the device; DBG-C Interface is the physical connection from a Probe to a target; DBG-C RF Protocol is the wireless protocol between two Probes; DBG-C Tool is the PC configuration, management, and update software. These terms are distinct.

The goal is to replace separate SWD/JTAG/UART jumper-wire connections with DBG-C Interface. V1 focuses on SWD and excludes JTAG. One DBG-C product provides wired debug, paired 2.4 GHz debug, and BLE management/download modes; these are not three separate products.

## 2. Problem and Goals

Traditional debug setups use multiple wires that are easy to misconnect and inconvenient to move. V1 shall provide wired debug, a two-device wireless debug link, target UART, hardware reset, and BLE device management. A DBG-C shall work as a standalone wired probe; when paired, one unit connects to the PC and the other to the target. Both units use identical hardware and differ only by runtime role.

## 3. Use Cases and Workflows

1. **Wired debug:** The PC connects to one Probe over USB and uses CMSIS-DAP v2. The target connects through DBG-C Interface for SWDIO, SWCLK, nRESET, and UART.
2. **Wireless debug:** Two Probes complete an explicitly defined pairing and role-establishment procedure. The PC-side Probe exposes CMSIS-DAP v2 over USB; the target-side Probe connects to the target. Debug commands, responses, and UART data travel over DBG-C RF Protocol.
3. **BLE management and download:** DBG-C Tool uses PC Bluetooth to discover devices, read information, configure, pair/unpair, inspect status, reset the target, perform planned target downloads, and update Probe firmware. The application download protocol, supported target MCUs, and PC OS/adapter compatibility remain to be defined. BLE does not expose native CMSIS-DAP directly to Keil/IAR/OpenOCD/pyOCD.
4. **Wireless failure recovery:** On RF loss, unfinished operations shall fail or stop with a visible result and state shall be recoverable. Commands with side effects must not be silently repeated. Recovery policy and timing remain to be frozen in the protocol phase.

## 4. V1 Scope

- Single-chip CH585M; USB Device; CMSIS-DAP v2; SWD Engine; SWDIO, SWCLK, target nRESET.
- CDC UART and DBG-C Interface (physical pin mapping is not frozen).
- Standalone wired debug; private 2.4 GHz pairing between two identical Probes; Host/Target role mechanism; download, online debug, and UART channel.
- BLE discovery, configuration, pairing management, status, target reset, planned wireless download, and Probe firmware update. The BLE download application protocol and supported target scope remain open.
- Unique device identity; pairing, unpairing, re-pairing; recovery after wireless disconnection.
- A Probe shall remain usable as a standalone wired debugger after leaving a wireless pair.

## 5. Non-Goals

V1 acceptance excludes SWO, JTAG, Target Power/current sensing, offline programming, multiple targets, mobile apps, flash-file caching, automatic target-MCU identification, advanced trace, and USBHS performance optimization. The architecture may reserve for them, but V1 acceptance shall not depend on them.

## 6. System Elements and Security Requirements

The system includes PC/DBG-C Tool, DBG-C Probe, USB, BLE, private 2.4 GHz, DBG-C Interface, and Target MCU. Device identity, pairing authorization, and OTA integrity require threat analysis. Encryption/authentication algorithms, key lifecycle, and debug authorization policy remain for review; the presence of AES in the MCU does not by itself establish system security.

## 7. Requirements and Acceptance Criteria

| ID | Requirement | Acceptance criteria |
|---|---|---|
| PRD-001 | USB CMSIS-DAP v2 and SWD | Connect, identify, read/write, erase, program/verify, reset, breakpoint, single-step, register, and memory operations pass on the approved host/tool matrix; matrix and versions remain open |
| PRD-002 | CDC target UART | RX/TX/full-duplex and concurrent debug pass USB CDC host tests; baud rates, flow control, and throughput thresholds require review |
| PRD-003 | Target reset | Pulse, polarity, electrical compatibility, and connected/disconnected behavior pass interface review and oscilloscope verification; values remain open |
| PRD-004 | Two-Probe wireless link | Pairing, roles, download, online debug, UART, disconnect detection, and recovery pass RF and verification specifications |
| PRD-005 | BLE management/download | Discovery, information, configuration, pair/unpair, status, target reset, Probe OTA, and reconnect pass BLE specification; target-download commands/data flow require an application protocol and at least one target acceptance case |
| PRD-006 | Unique identity and pairing management | Devices are distinguishable; pairing, unpairing, re-pairing, and power-cycle behavior conform to frozen persistence rules |
| PRD-007 | Standalone fallback | Independent USB debug regression passes after either paired unit leaves the wireless system |
| PRD-008 | Recovery and visible errors | RF/USB/Target faults produce distinguishable states and results, with no silent command repetition or false success |

## 8. Performance Metrics

No latency, throughput, startup time, power, range, or packet-loss thresholds have been approved or measured. Do not invent values. Establish quantitative metrics and measurement conditions from USB/RF/SWD prototypes and add them to acceptance criteria.

## 9. Open Requirements

Whether USB connection affects role, role-switch rules, pairing persistence, automatic/manual pairing, multi-device USB behavior, PC OS/IDE matrix, CDC parameters, target voltage range, wireless recovery deadline, OTA rollback/authorization, BLE target-download protocol/scope, and cable capabilities remain for product review. See `../00-project/DBG-C-OPEN-001.en-US.md`.

## 10. Product Statement (Input, Not Verification Evidence)

**DBG-C is an embedded MCU wired and wireless debug/programming platform with private 2.4 GHz and BLE capabilities. One unit works as a wired probe; two identical units form a symmetric 2.4 GHz link. The PC wired interface targets CMSIS-DAP v2, while BLE management/download uses an application protocol carried by DBG-C Tool.** “One wired, two wireless; one Type-C cable for a unified MCU debug interface” is product vision copy and does not mean Type-C pin mapping or cable compatibility has been verified.

VTREF, Target Power, BOOT, and Target Detect are future extensions and not mandatory V1 acceptance. “BLE download without a dongle” is a product goal whose protocol and compatibility matrix remain open.
