# DBG-C MCU Selection and Resource Assessment

**Document ID:** DBG-C-MCU-001　**Version:** V0.1　**Status:** Preliminary datasheet extract; resource allocation not frozen

## 1. Evidence Source

Repository file `docs/09-references/CH585-CH584_Datasheet_V1.6.pdf`, titled *CH585/CH584 Datasheet*, V1.6, 160 pages. The capabilities below are from this local file; silicon revision/errata, SDK implementation, and concurrency have not been checked. Obtain the WCH reference manual and matching SDK before review. Do not infer CH585M behavior from CH584 or other CH series.

## 2. Resource Table

| Resource | Datasheet statement | DBG-C assessment/status |
|---|---|---|
| CPU | QingKe RISC-V3C, RV32IMBC plus extensions; up to 78 MHz | Clock modes and SDK configuration TBD |
| FlashROM | 512 KB: 448 KB CodeFlash, 32 KB DataFlash, 24 KB BootLoader, 8 KB InfoFlash | Partitions, dual-image/rollback update, usable space TBD from SDK/Boot verification |
| SRAM | 128 KB: 96 KB RAM96K, 32 KB RAM32K | Peak buffers under concurrent RF/USB require measurement |
| USBFS | One FS USB 2.0 controller/PHY; 15 endpoints; 64-byte packets; DMA; Host/Device | Endpoint allocation and SDK APIs TBD |
| USBHS | One 480 Mbps USB 2.0 HS controller/PHY; 1024-byte packets; DMA; HS/FS Host/Device | V1 does not target performance optimization; USB Device stack and FS/HS selection TBD |
| UART | Four instances; 8-level FIFO; datasheet states up to 9 Mbps | Pins, clock accuracy, and target levels TBD |
| SPI | Two instances, Master/Slave, DMA | No evidence yet whether RF needs an external transceiver; internal RF path TBD from SDK |
| ADC | 12-bit; 14 external + 3 internal channels (overview) | Not mandatory in V1; package/mux channel check required |
| GPIO | 40 GPIO; two support 5 V input; 32 support interrupt/wake input | Does not imply 5 V output. Review every pin mux and voltage constraint |
| BLE/RF | BLE 5.4; integrated 2.4 GHz RF; 1/2 Mbps; mentions 2.4G mode up to 8 kHz report rate | Meaning of 2.4G mode, private PHY/API, RF DMA capability/API, BLE coexistence, and performance require SDK/reference-manual confirmation. Local datasheet extract does not confirm RF DMA |
| Timers/PWM | Four 26-bit timers; four capture channels; PWM resources in datasheet | Applicability to SWD timing requires SDK and waveform validation |
| UID/security | AES-128 and unique chip ID | UID API, length, immutability, and key-storage boundaries TBD from SDK/security review |
| Boot/OTA | Datasheet states ICP/ISP/IAP and OTA wireless update support | Boot protocol, OTA APIs, rollback/signature/power-fail recovery require official evidence |
| Package | CH585M: QFN48 | Verify land pattern, dimensions, and pin table against package documentation |
| Clocks | On-chip PLL, 16 MHz and 32 kHz clocks; external crystal requirements not confirmed in this extract | USB/BLE/RF accuracy and component requirements TBD from reference manual/SDK |
| Debug | Single/dual-wire emulation debug; datasheet says PB15/PB14 are used when enabled | Production debug access, mux conflict, and production lock policy TBD |

## 3. Preliminary Pin/Peripheral Conflicts

The datasheet mux table shows UART/SPI/TMR/ADC/emulation-debug alternatives; PB15/PB14 may be occupied by emulation debug. Since no approved USB/RF/DBG-C Interface pin plan exists, no final GPIO is assigned here. Build a QFN48 matrix and verify it against WCH SDK initialization/mux definitions.

## 4. Resource Allocation Status

| Function | Planned need | MCU resource/pin allocation | Status |
|---|---|---|---|
| USB Device | CMSIS-DAP v2 + CDC | USBFS/USBHS undecided; endpoints unassigned | TBD |
| SWD Engine | SWDIO/SWCLK timing | GPIO/timer/DMA unassigned | TBD |
| Target UART | CDC bridge | UART and pins unassigned | TBD |
| RF/BLE | Private 2.4 GHz with BLE management coexistence | SDK mode and radio scheduling unconfirmed | TBD |
| ADC/LED/Button | Status and extensions | Channels/pins unassigned | TBD |
| DBG-C Interface | Basic/Full signals | No pin map; must not freeze | TBD |
| Debug/Boot/Reserve | Production and recovery | Pins/storage partitions unassigned | TBD |

## 5. Required Materials

Obtain the official WCH CH585M reference manual, matching SDK/example version, package drawing, silicon revision/errata, USB FS/HS Device examples, RF/BLE coexistence guidance, OTA/ISP/IAP examples and interfaces, and clock/electrical requirements. Record file version/hash and review date when updating this document.
