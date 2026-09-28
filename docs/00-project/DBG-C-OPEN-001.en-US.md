# DBG-C Open Questions and Verification List

**Document ID:** DBG-C-OPEN-001　**Version:** V0.3　**Status:** Open items

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
| O18 | Can the upstream Eclipse ThreadX RISC-V32/GNU port be correctly adapted to CH585M QingKe V3C? | Implement and verify HPE, PFIC/VTF, startup, exception frames, SysTick, and ThreadX preemption; run PoC-1 on CH585M | MCU, FW, TEST | To verify; host ELF build passes |

## O18 Evidence Update

- ThreadX is pinned to `v6.5.1.202602a_rel`, commit `b91b03b9e75fa523b17127f9e0eca09dca916459`; MounRiver Linux x64 Toolchain V2.4.0 GCC 12.2.0 is installed in the current user account.
- The PoC-1 ELF cross-build passed on the host, but SysTick is not configured; the WCH startup default SysTick handler is a halt loop, and HPE/exception-frame/context-switch adaptation is unverified.
- No CH585M board was identified as connected, so O18 remains open.

## Confirmed Evidence Boundary

- The repository currently contains only `docs/09-references/CH585-CH584_Datasheet_V1.6.pdf`; no SDK, reference manual, source, schematic, test record, or packet capture was found.
- The MCU assessment is therefore a datasheet extract, not evidence of SDK APIs, concurrency, or board validation.
- DBG-C Interface pins, RF frame fields, USB descriptors, and role-switch rules are not frozen.
- The product direction includes BLE dongle-free target download and USBFS as the V1 USB allocation. BLE download lacks an application protocol/target scope; USBFS endpoint and SDK Device compatibility still require verification.
