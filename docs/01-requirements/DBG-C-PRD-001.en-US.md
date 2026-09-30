# DBG-C Product Requirements Specification

**Document ID:** DBG-C-PRD-001　**Version:** V0.5 (Draft baseline)　**Status:** V1 Target voltage domains, level adaptation, and active output voltage scope frozen; electrical limits and circuit remain to be defined

## 1. Product Positioning

DBG-C Platform is an embedded MCU debug and programming platform with wired, private 2.4 GHz, and BLE capabilities. DBG-C Probe is the device; DBG-C Interface is the physical connection from a Probe to a target; DBG-C RF Protocol is the wireless protocol between two Probes; DBG-C Tool is the PC configuration, management, and update software. These terms are distinct.

The goal is to replace separate SWD/JTAG/UART jumper-wire connections with DBG-C Interface. The V1 host-facing wired interface uses CMSIS-DAP v2 Bulk over USBHS. Firmware follows the DAPLink ecosystem direction and reuses applicable open protocol/algorithm components; CMSIS-DAP is the debug protocol and USB Bulk is the wired transport. SWD is the first implementation priority. JTAG, SWO, Target VTref sensing, target power control, USB self-update, private 2.4 GHz self-update, and external SPI NOR are in the current product direction, with sequencing and acceptance to be reviewed in FW/TEST. One DBG-C product provides wired debug, paired 2.4 GHz debug, and BLE management/future OTA modes; these are not three separate products.

## 2. Problem and Goals

Traditional debug setups use multiple wires that are easy to misconnect and inconvenient to move. V1 shall provide wired debug, a two-device wireless debug link, target UART, hardware reset, and BLE device management. A DBG-C shall work as a standalone wired probe; when paired, one unit connects to the PC and the other to the target. Both units use identical hardware and differ only by runtime role.

## 3. Use Cases and Workflows

1. **Wired debug:** The PC connects to one Probe over USB and uses CMSIS-DAP v2. The target connects through DBG-C Interface for SWDIO, SWCLK, nRESET, and UART.
2. **Wireless debug:** Two Probes complete an explicitly defined pairing and role-establishment procedure. The PC-side Probe exposes CMSIS-DAP v2 over USB; the target-side Probe connects to the target. Debug commands, responses, and UART data travel over DBG-C RF Protocol.
3. **BLE management and download:** DBG-C Tool uses PC Bluetooth to discover devices, read information, configure, pair/unpair, inspect status, reset the target, perform planned target downloads, and update Probe firmware. The application download protocol, supported target MCUs, and PC OS/adapter compatibility remain to be defined. BLE does not expose native CMSIS-DAP directly to Keil/IAR/OpenOCD/pyOCD.
4. **Wireless failure recovery:** On RF loss, unfinished operations shall fail or stop with a visible result and state shall be recoverable. Commands with side effects must not be silently repeated. Recovery policy and timing remain to be frozen in the protocol phase.

## 4. V1 Scope

- Single-chip CH585M; USBHS Device; DAPLink firmware ecosystem direction, CMSIS-DAP v2 Bulk; SWD Engine; SWDIO, SWCLK, target nRESET.
- Target JTAG, SWO receive, UART0-to-CDC ACM, VTref ADC sensing, target power control resources, and USB self-update architecture; electrical implementation, update partitioning, and acceptance criteria require review.
- External SPI NOR resources for wireless image staging, rollback/configuration, and future offline-image evaluation; do not set part, capacity, or partition before reviewing measured link maps, image sizes, and frozen requirements.
- CDC UART and DBG-C Interface (physical pin mapping is not frozen).
- Standalone wired debug; private 2.4 GHz pairing between two identical Probes; Host/Target role mechanism; download, online debug, UART channel, and Probe self-update over RF.
- BLE discovery, configuration, pairing management, status, and target reset. BLE OTA and BLE Target download are later phases; their protocols and supported Target scope remain open.
- Unique device identity; pairing, unpairing, re-pairing; recovery after wireless disconnection.
- A Probe shall remain usable as a standalone wired debugger after leaving a wireless pair.

## 5. Non-Goals

V1 excludes target current sensing, offline-programming acceptance, multiple targets, mobile apps, automatic target-MCU identification, and advanced trace. External Flash image storage is limited to update staging and future evaluation; offline programming remains a later feature. USBHS is the current wired-interface baseline, not an optional performance optimization. Concurrent BLE and private 2.4 GHz operation is unverified and is not an acceptance commitment.

## 6. System Elements and Security Requirements

The system includes PC/DBG-C Tool, DBG-C Probe, USB, BLE, private 2.4 GHz, DBG-C Interface, and Target MCU. Device identity, pairing authorization, and OTA integrity require threat analysis. Encryption/authentication algorithms, key lifecycle, and debug authorization policy remain for review; the presence of AES in the MCU does not by itself establish system security.

## 7. Requirements and Acceptance Criteria

| ID | Requirement | Acceptance criteria |
|---|---|---|
| PRD-001 | USB CMSIS-DAP v2 and SWD | Connect, identify, read/write, erase, program/verify, reset, breakpoint, single-step, register, and memory operations pass on the approved host/tool matrix; matrix and versions remain open |
| PRD-002 | CDC target UART | RX/TX/full-duplex and concurrent debug pass USB CDC host tests; baud rates, flow control, and throughput thresholds require review |
| PRD-003 | Target reset | Pulse, polarity, electrical compatibility, and connected/disconnected behavior pass interface review and oscilloscope verification; values remain open |
| PRD-004 | Two-Probe wireless link | Pairing, roles, download, online debug, UART, disconnect detection, and recovery pass RF and verification specifications |
| PRD-005 | BLE management/download | Discovery, information, configuration, pair/unpair, status, target reset, and reconnect pass BLE specification; BLE OTA and Target download are later phases and excluded from current mandatory acceptance |
| PRD-006 | Unique identity and pairing management | Devices are distinguishable; pairing, unpairing, re-pairing, and power-cycle behavior conform to frozen persistence rules |
| PRD-007 | Standalone fallback | Independent USB debug regression passes after either paired unit leaves the wireless system |
| PRD-008 | Recovery and visible errors | RF/USB/Target faults produce distinguishable states and results, with no silent command repetition or false success |
| PRD-009 | USBHS composite device | Final USBHS firmware enumerates CMSIS-DAP v2 Bulk and CDC ACM on the approved Windows/Linux/macOS matrix; descriptors, endpoints, and packet sizes follow the frozen USB-001 implementation |
| PRD-010 | Target JTAG | On at least one frozen Cortex-M target, pass CMSIS-DAP JTAG connect, DP/AP access, program/verify, reset, and online debug; speed and compatibility scope remain for review |
| PRD-011 | SWO | Receive Target SWO and expose it through a defined host stream interface; baud, format, buffering, and loss metrics require USB/FW/TEST freeze |
| PRD-012 | VTref sensing | Report voltage/validity over a frozen Target voltage range; divider, protection, calibration, accuracy, and thresholds pass electrical review and measurement |
| PRD-013 | Target power output control | V1 must support externally powered nominal 1.8 V and 3.3 V Targets. DBG-C active Target Power is decided to provide nominal 3.3 V only; active 1.8 V output is not supported. A 1.8 V Target is powered externally. Output accuracy, allowed current, current limit/short circuit, reverse current, thermal/inrush, and concurrent external-supply behavior must still be defined from selected parts and validation-board tests before schematic freeze; do not present a typical current-limit resistor setting as a guaranteed output rating |
| PRD-014 | USB self-update | Update Probe firmware from the PC over USBHS; pass complete-image validation, commit, reboot, corrupt-image recovery, and power-loss recovery |
| PRD-015 | Private 2.4 GHz self-update | Paired devices transfer Probe images reliably under the frozen RF update protocol; verify loss/retry, link recovery, integrity, commit, and recovery |
| PRD-016 | External SPI NOR | After part selection, verify image staging reads/writes, erase boundaries, integrity checks, and power-loss retention; rollback/offline-image use requires a separate decision |
| PRD-017 | Target I/O voltage domains and level adaptation | Frozen V1 requirement: support nominal 1.8 V and 3.3 V externally powered Target I/O domains. Every Target digital signal passes through VTref-associated level adaptation and is isolated when either Probe or Target is unpowered. The Target-side logic domain follows valid Target VTref without a user software voltage selector. Invalid VTref inhibits Probe-to-Target outputs in hardware. Prohibit direct CH585M GPIO-to-Target operating paths. SWDIO uses explicit direction control synchronized with SWD turnaround; JTAG TMS direction is Probe-to-Target. Freeze actual voltage tolerances, translator parts, timing, protection, and measurable thresholds from component data and validation-board tests |


## 8. Performance Metrics

No latency, throughput, startup time, power, range, or packet-loss thresholds have been approved or measured. Do not invent values. Establish quantitative metrics and measurement conditions from USB/RF/SWD prototypes and add them to acceptance criteria.

## 9. Open Requirements

Whether USB connection affects role, role-switch rules, pairing persistence, automatic/manual pairing, multi-device USB behavior, PC OS/IDE matrix, CDC parameters, allowed minimum/maximum and fault ranges around nominal Target domains, active Target Power current/tolerance/protection, wireless recovery deadline, OTA rollback/authorization, BLE target-download protocol/scope, and cable capabilities remain for product review. V1 active Target output scope is set to nominal 3.3 V; externally powered nominal 1.8 V and 3.3 V Target domains and VTref-following adaptation requirements are frozen; see `../00-project/DBG-C-OPEN-001.en-US.md`.

## 10. Product Statement (Input, Not Verification Evidence)

**DBG-C is an embedded MCU wired and wireless debug/programming platform with private 2.4 GHz and BLE capabilities. One unit works as a wired probe; two identical units form a symmetric 2.4 GHz link. The PC wired interface targets CMSIS-DAP v2, while BLE management/download uses an application protocol carried by DBG-C Tool.** “One wired, two wireless; one Type-C cable for a unified MCU debug interface” is product vision copy and does not mean Type-C pin mapping or cable compatibility has been verified.

VTref, Target Power, SWO, JTAG, USB self-update, private 2.4 GHz self-update, and external SPI NOR are in the current product direction; detailed acceptance criteria must still be frozen in FW/TEST. BOOT control and Target Detect are not in the currently verified V1 pin baseline. BLE OTA is a later implementation stage. BLE download without a dongle requires a separate application protocol and compatibility matrix and does not mean Keil/IAR/OpenOCD/pyOCD can directly use system BLE.
