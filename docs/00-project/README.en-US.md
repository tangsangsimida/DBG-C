# DBG-C Engineering Documents

This document set is a V0.x engineering baseline draft, not a frozen interface or proof of implementation. Review Phase 1 product scope and acceptance first, then proceed to interface and architecture freeze. Do not release a PCB based on an unfrozen interface specification.

| ID | Document | Current status |
|---|---|---|
| PRD-001 | [Product Requirements Specification](../01-requirements/DBG-C-PRD-001.en-US.md) | V0.1, for review |
| SYS-001 | [System Architecture](../02-system/DBG-C-SYS-001.en-US.md) | V0.1, for review |
| IF-001 | [DBG-C Interface Specification](../03-interfaces/DBG-C-IF-001.en-US.md) | V0.1 concept draft; pins not frozen |
| MCU-001 | [MCU Selection and Resource Assessment](../04-hardware/DBG-C-MCU-001.en-US.md) | V0.1 preliminary datasheet-only assessment |
| RF-001 | [Private 2.4G Protocol Specification](../06-protocols/DBG-C-RF-001.en-US.md) | V0.1 framework; not ready for implementation |
| TEST-001 | [Verification Specification](../07-verification/DBG-C-TEST-001.en-US.md) | V0.1 test plan; no results |
| RISK-001 | [Risk Register](../08-risk/DBG-C-RISK-001.en-US.md) | V0.1 initial risks |
| OPEN-001 | [Open Questions and Verification List](DBG-C-OPEN-001.en-US.md) | V0.1 open |

Future phase documents DBG-C-HW-001, FW-001, USB-001, and BLE-001 have not yet been created.

## Source Material and Evidence Boundary

- CH585M parameter extracts are based on the repository [CH585/CH584 Datasheet V1.6](../09-references/CH585-CH584_Datasheet_V1.6.pdf). It is not a reference manual or SDK; implementation and concurrency still require official materials and testing.
- USB Type-C design shall check the applicable formal revision on the USB-IF [Type-C Cable and Connector Specification page](https://www.usb.org/usb-type-cr-cable-and-connector-specification). The full specification is not stored in this repository.
- CMSIS-DAP v2 Bulk and optional CDC design reference: Arm [CMSIS-DAP USB Peripheral Configuration](https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__ConfigUSB__gr.html), CMSIS_5 documentation series (CMSIS-DAP V2.1.1). This does not prove DBG-C compatibility with any IDE/OS.
- External webpages checked on 2026-09-28. Recheck revisions before specification freeze.
