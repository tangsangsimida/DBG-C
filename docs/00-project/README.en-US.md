# DBG-C Engineering Documents

This document set is a V0.x engineering baseline draft, not a frozen interface or proof of implementation. Review Phase 1 product scope and acceptance first, then proceed to interface and architecture freeze. Do not release a PCB based on an unfrozen interface specification.

| ID | Document | Current status |
|---|---|---|
| PRD-001 | [Product Requirements Specification](../01-requirements/DBG-C-PRD-001.en-US.md) | V0.1, for review |
| SYS-001 | [System Architecture](../02-system/DBG-C-SYS-001.en-US.md) | V0.2, for review |
| IF-001 | [DBG-C Interface Specification](../03-interfaces/DBG-C-IF-001.en-US.md) | V0.1 concept draft; pins not frozen |
| MCU-001 | [MCU Selection and Resource Assessment](../04-hardware/DBG-C-MCU-001.en-US.md) | V0.8; EVT confirms UID ROM command and 8-byte construction; silicon read and Device ID security semantics remain open |
| FW-001 | [Firmware Architecture and PoC-1 Record](../05-firmware/DBG-C-FW-001.en-US.md) | V0.46; nine SWD-engine host line-model cases now execute through the GPIO BSP modeled-register path (2759 assertions); two clean rebuilds at a fixed path produced matching ELF/map hashes; current ELF text/data/bss are 8916/8/5580 bytes; ThreadX board runtime remains unverified |
| RF-001 | [Private 2.4G Protocol Specification](../06-protocols/DBG-C-RF-001.en-US.md) | V0.1 framework; not ready for implementation |
| TEST-001 | [Verification Specification](../07-verification/DBG-C-TEST-001.en-US.md) | V0.42; adds ThreadX board-observation cases, all Not run; UID adapter 31 mock checks and target-object compile recorded; silicon read not run; UART0 and bridge host checks are detailed in the document |
| RISK-001 | [Risk Register](../08-risk/DBG-C-RISK-001.en-US.md) | V0.6; ThreadX port awaits board verification, tick target pending decision |
| OPEN-001 | [Open Questions and Verification List](DBG-C-OPEN-001.en-US.md) | V0.37; PoC exposes ThreadX creation status/tick observations, but board runtime has not run; verification-board design released, product-hardware freeze not released; O11/O21 remain open |

Future phase documents DBG-C-HW-001, USB-001, and BLE-001 have not yet been created.

## Source Material and Evidence Boundary

- The project-designated CH585M IC manual is the repository [CH585/CH584 Datasheet V1.6](../09-references/CH585-CH584_Datasheet_V1.6.pdf), the primary basis for chip parameters, pin multiplexing, and documented peripheral capabilities. SDK APIs, concurrency performance, and board behavior still require SDK review and measurement.
- The [CH585EVT reference package](../09-references/CH585EVT/README.md) keeps the user-supplied original archive in one place. The CH585 SWD GPIO BSP is now based on the archive GPIO implementation and MCU-001 pin allocation; USB, UART, BLE, and RF drivers still require review of their specific official sources and accompanying notices.
- USB Type-C design shall check the applicable formal revision on the USB-IF [Type-C Cable and Connector Specification page](https://www.usb.org/usb-type-cr-cable-and-connector-specification). The full specification is not stored in this repository.
- CMSIS-DAP v2 Bulk and optional CDC design reference: Arm [CMSIS-DAP USB Peripheral Configuration](https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__ConfigUSB__gr.html), CMSIS_5 documentation series (CMSIS-DAP V2.1.1). This does not prove DBG-C compatibility with any IDE/OS.
- External webpages checked on 2026-09-28. Recheck revisions before specification freeze.
