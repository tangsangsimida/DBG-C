# DBG-C Open Questions and Verification List

**Document ID:** DBG-C-OPEN-001　**Version:** V0.5　**Status:** Open items

| ID | Question | Evidence/decision required | Affected documents | Status |
|---|---|---|---|---|
| O01 | What CH585M private RF PHY/API is available, and what are BLE/private-RF coexistence limits? | Official WCH reference manual, SDK/examples, and measurements | MCU, RF, FW, TEST | To obtain |
| O02 | Does the WCH USBFS Device stack provide the endpoints and buffers required for CMSIS-DAP v2 Bulk + CDC? | CH585M USBFS Device SDK example, endpoint APIs, host OS tests | MCU, USB, SYS | To verify |
| O03 | What RF DAP round-trip latency/retry limits work with common debuggers? | CMSIS-DAP host measurements and RF prototype | RF, TEST, PRD | To verify |
| O04 | What Type-C pins/cable approach is valid and reliable for Basic/Full? | Applicable USB-IF specification, cable construction evidence, electrical review | IF, HW, TEST | To research |
| O05 | How do USB insertion, user choice, and wireless connection determine Standalone/Host/Target? | Product state-machine review, including conflicts/transitions | PRD, SYS, RF, BLE | Decision needed |
| O06 | Is pairing persistent/automatic, and what happens after unpair? | UX/security review and persistence tests | PRD, RF, BLE | Decision needed |
| O07 | What is the Device ID source, length, API, and authentication binding? | CH585M SDK/official interface and security review | MCU, RF, BLE | To verify |
| O08 | What USB VID/PID, interface/endpoints, strings, and serial policy will be used? | Formal implementation and VID ownership decision | USB, TEST | Decision needed |
| O09 | What CDC baud rates, flow control, target levels, and performance limits apply? | User requirements, electrical design, and measurements | PRD, IF, TEST | Decision needed |
| O10 | Does OTA support signatures, dual image, rollback, and power-loss recovery? | WCH Boot/SDK docs, examples, and power-cut testing | MCU, FW, BLE, RF, RISK | To verify |
| O11 | What are target UART/RESET/SWD voltage limits and protection needs? | Target compatibility scope, electrical specification, tests | IF, HW, TEST | Decision needed |
| O12 | What are performance goals for DAP latency, throughput, range, and endurance? | Prototype data and product review | PRD, RF, TEST | Decision needed |
| O13 | Is V1 BLE OTA limited to Probe self-update, or is RF OTA also needed? | Requirement and security/resource review | PRD, BLE, RF | Decision needed |
| O14 | Which PC OS, IDE, OpenOCD, and pyOCD versions are supported? | Product support policy and interoperability tests | PRD, USB, TEST | Decision needed |
| O15 | Which datasheet revision/errata/reference manual apply to the current silicon? | WCH release page and chip revision check | MCU, HW, FW | To obtain |
| O16 | What BLE wireless-download protocol, image format, target scope, and resume rules apply? | Define DBG-C Tool ↔ Probe protocol and select an acceptance target board | PRD, BLE, TEST | Decision needed |
| O17 | Can USBFS and USBHS operate concurrently? What implementation constraint would require V1 to switch from its USBFS allocation to USBHS? | SDK examples, official resource limits, comparative measurements | MCU, USB, SYS | To verify |
| O18 | Can Eclipse ThreadX RISC-V32/GNU context routines and the local CH585M low-level adapter run correctly on QingKe V3C? | Verify HPE, PFIC/VTF, startup, exception frames, SysTick, sleep/wakeup, scheduling, and sustained runtime on CH585M | MCU, FW, TEST | To verify; experimental ISR/timebase implemented, host ELF build passes |

## O18 Evidence Update

- ThreadX is pinned to `v6.5.1.202602a_rel`, commit `b91b03b9e75fa523b17127f9e0eca09dca916459`; MounRiver Linux x64 Toolchain V2.4.0 GCC 12.2.0 is installed in the current user account.
- PoC-1 now contains experimental clock initialization, a low-level unused-memory boundary, VTF SysTick registration, and a ThreadX tick ISR. The threads read `tx_time_get()` and sleep for one tick. Host cross-build passed; output is ELF32 RISC-V, text 8876, data 8, bss 5564 bytes.
- The tick uses the upstream ThreadX header default of 100 ticks/s. HPE/VTF behavior, exception frames, actual SysTick frequency, tick delivery, sleep/wakeup, scheduling, and endurance remain unverified on hardware; O18 remains open.
- `docs/05-firmware/DBG-C-FW-001.en-US.md` records EVT provenance, archive hashes, and host setup. The WCH EVT `.cproject` establishes a GCC12 configuration but not the GCC patch version; the installed toolchain was measured as GCC 12.2.0.

## Confirmed Evidence Boundary

- The repository contains `docs/09-references/CH585-CH584_Datasheet_V1.6.pdf`, the `docs/09-references/CH585EVT/CH585EVT.ZIP` archive, and the limited EVT startup/linker/header copies used by PoC-1. No separate CH585 reference manual, DBG-C schematic, board-test record, or packet capture is present.
- The EVT index is dated 2026.08 and its FreeRTOS example uses FreeRTOS-Kernel V11.3.0; the material does not declare a standalone WCH SDK semantic version. Extracted APIs/registers support the ThreadX PoC only and do not prove RF/BLE/USB resource coexistence or board behavior.
- DBG-C Interface pins, RF frame fields, USB descriptors, and role-switch rules are not frozen.
- The product direction includes BLE dongle-free target download and USBFS as the V1 USB allocation. BLE download lacks an application protocol/target scope; USBFS endpoint and SDK Device compatibility still require verification.
