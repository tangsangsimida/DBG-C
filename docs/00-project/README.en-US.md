# DBG-C Engineering Documents

This document set is a V0.x engineering baseline draft, not a frozen interface or proof of implementation. Review Phase 1 product scope and acceptance first, then proceed to interface and architecture freeze. Do not release a PCB based on an unfrozen interface specification.

| ID | Document | Current status |
|---|---|---|
| PRD-001 | [Product Requirements Specification](../01-requirements/DBG-C-PRD-001.en-US.md) | V0.1, for review |
| SYS-001 | [System Architecture](../02-system/DBG-C-SYS-001.en-US.md) | V0.2, for review |
| IF-001 | [DBG-C Interface Specification](../03-interfaces/DBG-C-IF-001.en-US.md) | V0.1 concept draft; pins not frozen |
| MCU-001 | [MCU Selection and Resource Assessment](../04-hardware/DBG-C-MCU-001.en-US.md) | V0.6; verification-board design gate passed, tick target pending decision, board verification not run |
| FW-001 | [Firmware Architecture and PoC-1 Record](../05-firmware/DBG-C-FW-001.en-US.md) | V0.18 FIFO and eight CMSIS-DAP command-core and four upstream SWD-engine line-model host checks pass; product HAL adaptation incomplete; ThreadX board runtime unverified |
| RF-001 | [Private 2.4G Protocol Specification](../06-protocols/DBG-C-RF-001.en-US.md) | V0.1 framework; not ready for implementation |
| TEST-001 | [Verification Specification](../07-verification/DBG-C-TEST-001.en-US.md) | V0.16; FIFO and eight CMSIS-DAP command-core and four upstream SWD-engine line-model host checks pass, ThreadX board tests not run; verification-board design gate passed |
| RISK-001 | [Risk Register](../08-risk/DBG-C-RISK-001.en-US.md) | V0.5; ThreadX port awaits board verification, tick target pending decision |
| OPEN-001 | [Open Questions and Verification List](DBG-C-OPEN-001.en-US.md) | V0.20; CMSIS-DAP product HAL/link remain open; verification-board design gate passed, ThreadX board verification not run, tick target pending decision, product-hardware freeze not released |

Future phase documents DBG-C-HW-001, USB-001, and BLE-001 have not yet been created.

## Source Material and Evidence Boundary

- The project-designated CH585M IC manual is the repository [CH585/CH584 Datasheet V1.6](../09-references/CH585-CH584_Datasheet_V1.6.pdf), the primary basis for chip parameters, pin multiplexing, and documented peripheral capabilities. SDK APIs, concurrency performance, and board behavior still require SDK review and measurement.
- The [CH585EVT reference package](../09-references/CH585EVT/README.md) keeps the user-supplied original archive in one place. Extract only the official headers, sources, or libraries needed when implementing a specific driver, and retain accompanying vendor and third-party notices. No DBG-C driver code is currently available to select specific components.
- USB Type-C design shall check the applicable formal revision on the USB-IF [Type-C Cable and Connector Specification page](https://www.usb.org/usb-type-cr-cable-and-connector-specification). The full specification is not stored in this repository.
- CMSIS-DAP v2 Bulk and optional CDC design reference: Arm [CMSIS-DAP USB Peripheral Configuration](https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__ConfigUSB__gr.html), CMSIS_5 documentation series (CMSIS-DAP V2.1.1). This does not prove DBG-C compatibility with any IDE/OS.
- External webpages checked on 2026-09-28. Recheck revisions before specification freeze.
