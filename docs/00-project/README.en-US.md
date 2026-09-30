# DBG-C Engineering Documents

This document set is a V0.x engineering baseline draft, not a frozen interface or proof of implementation. Review Phase 1 product scope and acceptance first, then proceed to interface and architecture freeze. Do not release a PCB based on an unfrozen interface specification.

| ID | Document | Current status |
|---|---|---|
| PRD-001 | [Product Requirements Specification](../01-requirements/DBG-C-PRD-001.en-US.md) | V0.5; active Target Power scope fixed at nominal 3.3 V output only; electrical limits remain to be defined |
| SYS-001 | [System Architecture](../02-system/DBG-C-SYS-001.en-US.md) | V0.6; records active Target Power at nominal 3.3 V only; circuit and product integration not frozen/verified |
| IF-001 | [DBG-C Interface Specification](../03-interfaces/DBG-C-IF-001.en-US.md) | V0.6; records 24-contact review input; USB-IF applicability, cable map, and electrical protection remain open; do not release a PCB from the map |
| MCU-001 | [MCU Selection and Resource Assessment](../04-hardware/DBG-C-MCU-001.en-US.md) | V0.19; PA5/PA6/PA7/PA12 Target front-end control resources added; PA5 depends on UART3 remapping |
| HW-001 | [Hardware Design Specification](../04-hardware/DBG-C-HW-001.en-US.md) | V0.5; active output scope set to 3.3 V; VTref overvoltage gating and Type-C mapping block circuit freeze; modular validation-board design only |
| FW-001 | [Firmware Architecture and PoC-1 Record](../05-firmware/DBG-C-FW-001.en-US.md) | V0.79; PB8 KEY_MODE/PB9 LED_TARGET raw GPIO passes 56 checks without electrical policy; PB6 TARGET_PWR_EN raw GPIO adapter passes 36 modeled-register host checks without defining power polarity; SPI1 configuration BSP passes 19 host checks and PA0–PA3 GPIO and raw PA3 chip-select BSP passes 104 host checks; data transfer is not implemented; PA4/A0 raw ADC adapter added to PoC build with 44 host checks passing and silicon sampling pending; image-update transaction manager added with 57 host checks; fish environment setup examples added; CH585 JTAG GPIO BSP and upstream CMSIS-DAP JTAG Sequence, IDCODE, and DP Transfer write/posted-read host model pass 1610 cumulative checks; SWD/Reset BSP migrated to MCU-001 V0.11 and related host checks pass; queued CMSIS-DAP service passes 40 host checks; fixed-slot packet queue passes 54 host checks; generic Target Reset service passes 24 callback checks; PB5 reset-sequence adapter passes 37 modeled-register checks; PB5 GPIO BSP including direction-switching emulation passes 69 modeled-register checks; ten SWD-engine host line-model cases now execute through the GPIO BSP modeled-register path (3321 assertions); two clean rebuilds at a fixed path produced matching ELF/map hashes; current ELF text/data/bss are 8916/8/5580 bytes; PoC tick target is 1000 ticks/s; PB5 integration and two clean builds at 1000 ticks/s pass; ThreadX board runtime remains unverified |
| USB-001 | [USB Device Specification](../06-protocols/DBG-C-USB-001.en-US.md) | V0.3 architecture draft; EVT CDC endpoints/API and header-versus-vector IRQ name mismatch recorded; DAP+CDC descriptors/endpoints not frozen, no product enumeration evidence |
| RF-001 | [Private 2.4G Protocol Specification](../06-protocols/DBG-C-RF-001.en-US.md) | V0.1 framework; not ready for implementation |
| TEST-001 | [Verification Specification](../07-verification/DBG-C-TEST-001.en-US.md) | V0.73; adds cable-map, VTref fault-isolation, 3.3 V power, and misconnection cases; all board tests Not run |
| RISK-001 | [Risk Register](../08-risk/DBG-C-RISK-001.en-US.md) | V0.15; adds VTref overvoltage, Type-C use, and PB5 reset-polarity risks R27–R29; all remain open |
| OPEN-001 | [Open Questions and Verification List](DBG-C-OPEN-001.en-US.md) | V0.51; active output voltage scope decided; Type-C permission, VTref fault isolation, and PB5 reset polarity still block circuit freeze |

BLE-001 has not yet been created; HW-001 V0.5 defines validation-board hardware design inputs; USB-001 is a bilingual V0.3 architecture draft.

## Source Material and Evidence Boundary

- The project-designated CH585M IC manual is the repository [CH585/CH584 Datasheet V1.6](../09-references/CH585-CH584_Datasheet_V1.6.pdf), the primary basis for chip parameters, pin multiplexing, and documented peripheral capabilities. SDK APIs, concurrency performance, and board behavior still require SDK review and measurement.
- The [CH585EVT reference package](../09-references/CH585EVT/README.md) keeps the user-supplied original archive in one place. The CH585 SWD GPIO BSP is now based on the archive GPIO implementation and MCU-001 pin allocation; USB, UART, BLE, and RF drivers still require review of their specific official sources and accompanying notices.
- USB Type-C design shall check the applicable formal revision on the USB-IF [Type-C Cable and Connector Specification page](https://www.usb.org/usb-type-cr-cable-and-connector-specification). The full specification is not stored in this repository.
- CMSIS-DAP v2 Bulk and optional CDC design reference: Arm [CMSIS-DAP USB Peripheral Configuration](https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__ConfigUSB__gr.html), CMSIS_5 documentation series (CMSIS-DAP V2.1.1). This does not prove DBG-C compatibility with any IDE/OS.
- External webpages checked on 2026-09-28. Recheck revisions before specification freeze.
