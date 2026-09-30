# DBG-C Engineering Documents

[简体中文](README.md)

This directory organizes project documents by engineering domain. Controlled documents use separate `.zh-CN.md` and `.en-US.md` files. When changing either language, check its counterpart, document ID, version/status, and related links. Refer to each file for current status; a draft or test plan does not mean a design is frozen or verification has passed.

## Directory Structure

| Directory | Contents |
|---|---|
| `00-project/` | Project index and open questions |
| `01-requirements/` | Product requirements |
| `02-system/` | System architecture |
| `03-interfaces/` | DBG-C Interface specifications |
| `04-hardware/` | MCU and hardware design documents |
| `05-firmware/` | Firmware architecture, implementation plans, and ThreadX PoC |
| `06-protocols/` | RF, BLE, and USB protocols |
| `07-verification/` | Verification specifications and results |
| `08-risk/` | Risk register |
| `09-references/` | Datasheets, specifications, and references; `CH585EVT/` holds the CH585 EVT archive |

## Repository Roots

| Directory | Purpose |
|---|---|
| `software/` | Root for all software code and builds, including software build configuration and build roots |
| `hardware/` | Root for hardware files, including schematics and symbol libraries |

## Document Index

- [Project index](00-project/README.en-US.md) · [Open questions](00-project/DBG-C-OPEN-001.en-US.md)
- [Product requirements PRD-001](01-requirements/DBG-C-PRD-001.en-US.md) · [System architecture SYS-001](02-system/DBG-C-SYS-001.en-US.md)
- [DBG-C Interface IF-001](03-interfaces/DBG-C-IF-001.en-US.md) · [MCU selection and resource assessment MCU-001](04-hardware/DBG-C-MCU-001.en-US.md)
- [Coding standard CODE-001](05-firmware/DBG-C-CODE-001.en-US.md) · [Firmware architecture and PoC-1 FW-001](05-firmware/DBG-C-FW-001.en-US.md)
- [USB Device specification USB-001](06-protocols/DBG-C-USB-001.en-US.md) · [RF protocol RF-001](06-protocols/DBG-C-RF-001.en-US.md)
- [Verification specification TEST-001](07-verification/DBG-C-TEST-001.en-US.md) · [Risk register RISK-001](08-risk/DBG-C-RISK-001.en-US.md)
- [CH585EVT official package index](09-references/CH585EVT/README.md)

## Terminology Baseline

Call the PC debug protocol **CMSIS-DAP v2**. DBG-C firmware follows the **DAPLink firmware system** and uses the **CMSIS-DAP v2** protocol. DAPLink is not a “DAPLink v2” protocol; a complete upstream firmware port requires separate review. The V1 wired-interface baseline is CMSIS-DAP v2 Bulk + CDC over USBHS. USBFS is reserved for recovery evaluation, and its porting scope and descriptors remain under review.
