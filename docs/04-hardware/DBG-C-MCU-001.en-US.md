# DBG-C MCU Selection and Resource Assessment

**Document ID:** DBG-C-MCU-001　**Version:** V0.1　**Status:** Preliminary datasheet extract; resource allocation not frozen

## 1. Evidence Source

`docs/09-references/CH585-CH584_Datasheet_V1.6.pdf` is the project-designated CH585M IC manual. Its internal title is *CH585/CH584 Datasheet*, V1.6, 160 pages. Use it as the primary project source for chip parameters, pin multiplexing, and documented peripheral capabilities. SDK APIs, concrete project configuration, RF concurrency performance, and board behavior still require SDK review/measurement. Do not infer CH585M behavior from CH584 or other CH series.

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
| USB Device | CMSIS-DAP v2 + CDC | USBFS; PB10/PB11; USBHS unused for now | SDK/descriptors TBD |
| SWD Engine | SWDIO/SWCLK timing | Reserve two general GPIOs; Timer as needed | GPIO timing/rate TBD |
| Target UART | CDC bridge | UART0; PB4/PB7 | SDK/concurrency TBD |
| RF/BLE | Private 2.4 GHz with BLE management | Integrated shared Radio resource | SDK coexistence TBD |
| ADC/LED/Button | Status/extensions | No ADC; reserve one GPIO each for LED/button | Button product decision TBD |
| DBG-C Interface | Basic/Full signals | No pin map; must not freeze | TBD |
| Debug/Boot/Reserve | Production and recovery | Pins/storage partitions unassigned | TBD |

## 5. V1 Peripheral Requirements and Schematic Pre-allocation

This is a **V1 schematic-input baseline**. It is not proof of MCU performance and does not freeze the Type-C pin mapping of DBG-C Interface.

| Function | MCU resource assignment | Schematic assignment | Notes and verification boundary |
|---|---|---|---|
| PC USB Device | USBFS controller/PHY + USB DMA | PB10=UD−, PB11=UD+; upstream USB-C connector | Use USBFS in V1 for CMSIS-DAP v2 Bulk + CDC. Verify descriptors and SDK USB Device stack during firmware work; leave USBHS disabled |
| Target UART/CDC | UART0 + USB CDC virtual COM port | UART0 PB4=RXD0, PB7=TXD0; connector pins to DBG-C Interface wait for interface freeze | UART0 modem signals are not needed. CDC enumerates over USBFS and bridges to UART0 |
| Target SWD | Two general GPIOs controlled by shared SWD Engine | Reserve MCU nets for SWDIO and SWCLK; GPIO numbers and connector pins wait for pin matrix/interface freeze | GPIO-driven SWD; do not consume SPI in V1. Verify timing and achievable SWD frequency with firmware/oscilloscope |
| Target Reset | One general GPIO output | Reserve a TARGET_nRESET MCU net; pin number waits for pin matrix | Do not connect to CH585M's own RST. Open-drain/push-pull, series/protection, and default state belong to electrical design |
| BLE + private 2.4 GHz | Integrated Radio/Baseband and ANT pin | Connect CH585M ANT through matching/antenna network per WCH RF reference design | Shared wireless subsystem; simultaneous use requires SDK confirmation. No external RF SPI assigned |
| Status/pairing interaction | GPIO | Reserve one LED and one button GPIO; pin numbers wait for pin matrix | Confirm production button need in UX review; LED polarity/current wait for component selection |
| USB/connection detection | GPIO only if selected circuit requires it | Reserve a VBUS/connection-detect net location; pin TBD | Do not assume USBFS automatically provides VBUS sensing; decide from SDK and power/connector design |
| Unique identity/pair settings | Chip UID + DataFlash | Firmware reads UID; DataFlash is unassigned until pairing-persistence requirements are frozen | UID API/length, write policy/endurance require SDK review; storage layout and pair persistence remain open |
| OTA and firmware | CodeFlash + BootLoader | Include programming/recovery circuitry per official programming/update design | Do not assume dual-image fits in 448 KB CodeFlash; signature, rollback, and power-fail safety are unverified |
| SWD timing/system time base | Timer if needed + GPIO | No Timer instance selected yet | First implement/measure GPIO SWD. If precision or CPU use misses targets, assign a Timer based on SDK support |
| DMA | USBFS DMA; Radio DMA TBD | No fixed DMA channel wiring | Datasheet lists USBFS DMA. DMA channel count and USB/RF arbitration require SDK/reference-manual confirmation |

### GPIO/Pin Reservation Constraints

1. Use PB10/PB11 for USBFS and PB4/PB7 for UART0. The manual defines them as UD−/UD+ and RXD0/TXD0 respectively; check CH585M SDK initialization before layout release.
2. Allocate SWDIO, SWCLK, TARGET_nRESET, LED, and button from remaining GPIOs. Do not assign pin numbers yet because the target-side Type-C pin map is not frozen.
3. PB14/PB15 are used as TIO/TCK when emulation debug is enabled. Exclude them from the allocatable GPIO pool and preserve Probe programming/recovery access.
4. PB12/PB13 are labeled USBHS U2D−/U2D+; V1 leaves USBHS unused. Do not confuse them with USBFS PB10/PB11.
5. The CH585M's own external reset input is shown as PB22/RST in the datasheet mux table. It is not the Target_nRESET output; keep the nets distinct.

### Peripherals Not Enabled in V1

SPI, I2C, ADC, NFC, TouchKey, LCD/LED Matrix, USBHS, SWO/JTAG, Target Power/VTREF are not allocated. Target-voltage sensing is a future function; do not consume required GPIOs for it now. Re-run pin-matrix and conflict review if scope changes.

### Conclusion

Peripheral counts support moving this design into a **schematic draft**: USBFS, UART0, two SWD GPIOs, one Target Reset GPIO, integrated wireless Radio, status GPIOs, and storage/Boot. This means required peripheral resources exist; it does not prove that CH585M meets target performance while USB, BLE, private 2.4 GHz, CDC, and SWD operate together. SDK review and board verification must close performance and radio coexistence.

### Memory Budgeting Approach

| Memory | Datasheet capacity/partition | Current plan |
|---|---|---|
| CodeFlash | 448 KB user application area | Hold for DBG-C application and portable CMSIS-DAP code. Partition only after measuring SDK and DAP build; do not assume dual-image OTA |
| BootLoader | 24 KB system boot area | Preserve for chip/official update use; verify whether product secure boot/rollback is supported |
| DataFlash | 32 KB user nonvolatile data | Unassigned until configuration and pairing-persistence requirements are frozen; endurance/atomic update/API TBD |
| InfoFlash | 8 KB system configuration | Do not use as ordinary product storage; follow official definition |
| SRAM | 128 KB total | Include USB DAP buffers, USB CDC rings, RF RX/TX/reassembly, BLE stack, SWD workspace, and task stacks in linker-map/peak budget. Byte counts require SDK integration/measurement; do not invent fixed percentages |

## 6. Additional Resource Requirements for ThreadX

The project has selected ThreadX for V1 firmware. Eclipse ThreadX upstream has published a RISC-V32 port; this does not prove that CH585M QingKe RISC-V3C works directly with that port or the WCH startup/interrupt framework. Confirm the ThreadX version, compiler, and CH585M integration by building with the actual SDK and testing on the board.

| Resource | ThreadX requirement | DBG-C allocation | Evidence boundary |
|---|---|---|---|
| Kernel tick | Periodic time base and interrupt | Designate the built-in 32-bit SysTick as the ThreadX kernel tick source; do not consume TMR0 through TMR3 | The CH585M manual confirms the SysTick counter and interrupt. ThreadX port integration, clock source, reload value, and priority must be verified in the WCH SDK project. If the port cannot use SysTick, review a resource change before using a general-purpose timer |
| Context switch | CPU context save/restore and scheduler entry | Integrate with WCH startup/interrupt framework; define thread/ISR boundaries for CMSIS-DAP, SWD, RF, and USB | Compatibility between upstream RISC-V32 port and QingKe interrupt/exception framework is unverified |
| Interrupts | Peripheral ISR to ThreadX scheduling interface | Define ThreadX API boundaries for USB, Radio/BLE, and Timer ISRs in firmware architecture | Interrupt nesting, priorities, and SDK ISR constraints require SDK review |
| SRAM | ThreadX objects, system stack, thread stacks, application buffers | Budget USB DAP/CDC, RF reassembly, BLE stack, and SWD workspace together in the 128 KB SRAM linker layout | Allocate byte counts after measuring selected ThreadX/SDK, thread count, stack watermarks, and linker map; do not invent values |
| CPU | Scheduling, interrupt, and protocol processing | Verify worst-case response under concurrent USB/RF load | Datasheet maximum of 78 MHz alone does not prove performance targets |
| Compiler/Port | ThreadX port compatible with CH585M toolchain, ABI, and startup | Integrate after locking ThreadX and WCH toolchain versions | Version, compiler, and porting patches are not selected |

ThreadX adds no external connector signal and does not change the assigned USBFS, UART0, SWD GPIO, Target Reset GPIO, and integrated Radio. Preserve Probe programming/recovery access. The current resource baseline designates SysTick for the ThreadX kernel tick and reserves TMR0 through TMR3; if port integration fails, document and review a resource change.

### ThreadX Upstream Dependency Source

V1 firmware will reference the official Eclipse ThreadX GitHub project through a **Git Submodule**: [eclipse-threadx/threadx](https://github.com/eclipse-threadx/threadx). The DBG-C superproject must record the submodule URL and exact commit; its directory path will be recorded when the firmware repository layout is frozen. The upstream README describes `master` as the development branch containing the newest code and explicitly says it does not represent the latest GA release. The integration baseline must therefore select and verify a formal release, then record its tag and corresponding commit; recording only `master` is insufficient. The exact version is unresolved; see O18 in OPEN-001.

The upstream repository lists the `risc-v32` architecture and provides a [GNU RISC-V32 port directory](https://github.com/eclipse-threadx/threadx/tree/master/ports/risc-v32/gnu), but this does not establish direct compatibility with CH585M QingKe RISC-V3C, the WCH toolchain, or its interrupt framework. DBG-C will use this repository as the source for the ThreadX kernel and port. Whether to adopt that port directly, the required modifications, and the CH585M integration method must be recorded after an actual build and on-board verification. DBG-C firmware checkouts must initialize and recursively update submodules so the working tree checks out the ThreadX commit recorded by the superproject.

The upstream root [LICENSE.txt](https://github.com/eclipse-threadx/threadx/blob/master/LICENSE.txt) identifies the MIT License, which requires preserving the copyright and permission notices in copies or substantial portions of the software. Before source or binary distribution, inspect the license file and source headers in the exact version used, and retain the required notices in the DBG-C third-party component inventory and release materials. This record is not legal advice. Upstream materials checked 2026-09-28.

## 7. Required Materials

Obtain the official WCH CH585M reference manual, matching SDK/example version, package drawing, silicon revision/errata, USBFS Device example, RF/BLE coexistence guidance, OTA/ISP/IAP examples and interfaces, and clock/electrical requirements. Record file version/hash and review date when updating this document.
