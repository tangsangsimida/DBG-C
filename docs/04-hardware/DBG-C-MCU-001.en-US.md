# DBG-C MCU Selection and Resource Assessment

**Document ID:** DBG-C-MCU-001　**Version:** V0.5　**Status:** CH585M V1 schematic resource allocation draft; software gate awaits tick-target confirmation, verification-hardware design not released

## 1. Evidence Source

`docs/09-references/CH585-CH584_Datasheet_V1.6.pdf` is the project-designated CH585M IC manual. Its internal title is *CH585/CH584 Datasheet*, V1.6, 160 pages. Use it as the primary project source for chip parameters, pin multiplexing, and documented peripheral capabilities. SDK APIs, concrete project configuration, RF concurrency performance, and board behavior still require SDK review/measurement. Do not infer CH585M behavior from CH584 or other CH series.

The repository also keeps the official EVT archive at `docs/09-references/CH585EVT/CH585EVT.ZIP` and copies only the startup, linker, and WCH headers used by the ThreadX PoC into `software/poc1-ch585-threadx/platform/ch585/`. The archive index `EVT/CH585_List_EN.txt` is dated 2026.08; the material does not declare a separate WCH SDK semantic version. EVT provenance/hashes, the pinned ThreadX version, and MounRiver toolchain version are recorded in DBG-C-FW-001. The PoC host cross-build does not prove board runtime or product-level peripheral concurrency.

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
| GPIO | The overview states 40 GPIOs, two with 5 V input tolerance and 32 with interrupt/wake capability | Table 1-1 CH585M column lists PA0–PA15 and PB0–PB23, 40 GPIO identifiers with package pad numbers, matching the overview count. This does not mean all 40 are freely allocatable or interrupt/wake capable. 5VT does not imply 5 V output |
| BLE/RF | BLE 5.4; integrated 2.4 GHz RF; 1/2 Mbps; mentions 2.4G mode up to 8 kHz report rate | Meaning of 2.4G mode, private PHY/API, RF DMA capability/API, BLE coexistence, and performance require SDK/reference-manual confirmation. Local datasheet extract does not confirm RF DMA |
| Timers/PWM | Four 26-bit timers; four capture channels; PWM resources in datasheet | Applicability to SWD timing requires SDK and waveform validation |
| UID/security | AES-128 and unique chip ID | UID API, length, immutability, and key-storage boundaries TBD from SDK/security review |
| Boot/OTA | Datasheet states ICP/ISP/IAP and OTA wireless update support | Boot protocol, OTA APIs, rollback/signature/power-fail recovery require official evidence |
| Package | CH585M: QFN48 | Verify land pattern, dimensions, and pin table against package documentation |
| Clocks | Pin table identifies 32 MHz HSE crystal pins X32MO/X32MI and 32 kHz crystal functions on PA10/PA11 | Allocate the 32 MHz crystal network to QFN48 pads 31/32; confirm whether a 32 kHz crystal is required from WCH BLE/low-power SDK configuration |
| Debug | Single/dual-wire emulation debug; datasheet says PB15/PB14 are used when enabled | Production debug access, mux conflict, and production lock policy TBD |

## 3. CH585M QFN48 Pin Allocation Matrix

Package pin numbers below are read from the **CH585M column** of datasheet Table 1-1 (printed pages 5–8). MCU-side resources are allocated as schematic inputs; this does not freeze the Type-C contact mapping on DBG-C Interface. Verify pin-mux initialization with the WCH SDK. Target voltage, pulls, drive topology, and protection remain subject to the interface electrical specification.

| QFN48 pad | GPIO/pin | V1 net allocation | Mux conflict/constraint |
|---:|---|---|---|
| 1 | VDCID | Connect per datasheet power circuit | Capacitor and DC-DC connection per datasheet |
| 2 | VSW | Connect per datasheet power circuit | DC-DC inductor/bypass per datasheet |
| 3 | VDD33 / VIO33 | Supply and I/O supply net | Verify decoupling and USB supply relationship against reference design |
| 4 | PA7 | Unallocated reserve | Do not enable TXD2, PWM5, LED6, or ADC A11 |
| 5 | PA8 | Unallocated reserve | Do not enable RXD1, LED7, or ADC A12 |
| 6 | PA9 | Optional pairing/function button GPIO reserve | Button population requires PRD confirmation; do not enable TMR0, TXD1, or ADC A13 mux |
| 7 | PB9 | Unallocated reserve | GPIO/NFCI; NFC disabled in V1 |
| 8 | PB8 | Unallocated reserve | GPIO/NFCM; NFC disabled in V1 |
| 9 | PB17 | Unallocated reserve | GPIO/NFC+; NFC disabled in V1 |
| 10 | PB16 | Unallocated reserve | GPIO/NFC−; NFC disabled in V1 |
| 11 | PB15 / TCK | Probe emulation-debug clock | Dedicated to TCK when emulation debug is enabled; do not allocate to Target SWD |
| 12 | PB14 / TIO | Probe emulation-debug data | Dedicated to TIO when emulation debug is enabled; do not allocate to Target SWD |
| 13 | PB13 / U2D+ | Reserve; USBHS disabled in V1 | Do not connect to USBFS D+ |
| 14 | PB12 / U2D− | Reserve; USBHS disabled in V1 | Do not connect to USBFS D− |
| 15 | PB11 / UD+ | USBFS D+ | Do not use as ordinary GPIO |
| 16 | PB10 / UD− | USBFS D− | Do not use as ordinary GPIO |
| 17 | PB7 / TXD0 | Probe UART TX, connected toward Target RX | UART0 TXD0; MODEM signals unused |
| 18 | PB6 | Target SWCLK | GPIO-driven; do not enable RTS/PWM8 mux |
| 19 | PB5 | Target SWDIO | Bidirectional GPIO; do not enable UART0 DTR mux |
| 20 | PB4 / RXD0 | Probe UART RX, connected from Target TX | UART0 RXD0 |
| 21 | PB3 | Unallocated reserve | Do not enable DCD or PWM9_ |
| 22 | PB2 | Unallocated reserve | Do not enable PWM8_ |
| 23 | PB1 | Unallocated reserve | Do not enable DSR or PWM7_ |
| 24 | PB0 | Unallocated reserve | Do not enable CTS or PWM6 |
| 25 | PB23 / RST | Probe active-low chip reset input | Alternate functions also include TMR0_, TXD2_, and PWM11; reserve per datasheet and do not connect as Target_nRESET output |
| 26 | PB22 | Unallocated reserve | Do not enable TMR3 or RXD2_; this is not the chip RST pin |
| 27 | PB21 | Unallocated reserve | Do not enable SCL_ or TXD3_ |
| 28 | PB20 | Unallocated reserve | Do not enable SDA_ or RXD3_ |
| 29 | PB19 | Unallocated reserve | General GPIO; unallocated in V1 |
| 30 | PB18 | Unallocated reserve | General GPIO; unallocated in V1 |
| 31 | X32MO | One side of 32 MHz crystal network | HSE crystal pin per datasheet; crystal parameters/load per WCH reference design |
| 32 | X32MI | Other side of 32 MHz crystal network | HSE crystal pin per datasheet; crystal parameters/load per WCH reference design |
| 33 | VINTA | Decoupling capacitor per datasheet | Value and routing per datasheet/reference design |
| 34 | ANT | RF network/antenna connection | The datasheet describes an RF input/output and recommends direct antenna connection. Verify the actual network against the WCH CH585M RF reference design, which is not present in this repository |
| 35 | VDCIA | Decoupling capacitor per datasheet | VDCIA/VDCID connection per datasheet |
| 36 | PA4 | Target_nRESET control output | GPIO; do not enable UART3 RXD3, LEDC, or ADC A0 mux; output stage/default state pending electrical design |
| 37 | PA5 | Optional status LED GPIO reserve | LED population requires PRD/hardware review; do not enable UART3 TXD3, LED4, or ADC A1 mux |
| 38 | PA6 | Unallocated reserve | Do not enable RXD2, PWM4_, LED5, or ADC A10 |
| 39 | PA0 | Unallocated reserve | Do not enable SCK1, LED0, or ADC A9 |
| 40 | PA1 | Unallocated reserve | Do not enable MOSI1, LED1, or ADC A8 |
| 41 | PA2 | Unallocated reserve | Do not enable TMR3_, MISO1, RI, LED2, or ADC A7 |
| 42 | PA3 | Unallocated reserve | Do not enable LED3 or ADC A6 |
| 43 | PA15 | Unallocated reserve | Do not enable SPI0 MISO or UART0 RXD0 remap |
| 44 | PA14 | Unallocated reserve | Do not enable SPI0 MOSI or UART0 TXD0 remap |
| 45 | PA13 | Unallocated reserve | Do not enable SPI0 SCK or PWM5 |
| 46 | PA12 | Unallocated reserve | Do not enable SPI0 SCS or PWM4 |
| 47 | PA11 / X32KO | Reserve for 32 kHz clock review; no external signal | Low-frequency oscillator output; WCH BLE/low-power SDK configuration must confirm whether a 32 kHz crystal is needed |
| 48 | PA10 / X32KI | Reserve for 32 kHz clock review; no external signal | Low-frequency oscillator input; WCH BLE/low-power SDK configuration must confirm whether a 32 kHz crystal is needed |

Note: pad numbers come from the CH585M column of datasheet Table 1-1 (printed pages 5–8). The table lists all 40 PA/PB GPIO identifiers with package pad numbers, as well as DBG-C-related power, clock, and RF pins. Verify interrupt/wake capability per pin against applicable chip data and the SDK.

## 4. Resource Allocation Status

| Function | Planned need | MCU resource/pin allocation | Status |
|---|---|---|---|
| USB Device | CMSIS-DAP v2 + CDC | USBFS; PB10 QFN48-16=UD−, PB11 QFN48-15=UD+; USBHS disabled | SDK/descriptors TBD |
| SWD Engine | SWDIO/SWCLK timing | PB5 QFN48-19=SWDIO; PB6 QFN48-18=SWCLK; GPIO bit-bang | GPIO timing/rate TBD |
| Target Reset | Hardware reset | PA4 QFN48-36=Target_nRESET | Output stage, default state, and voltage pending electrical design |
| Target UART | CDC bridge | UART0; PB4 QFN48-20=Probe RX/Target TX, PB7 QFN48-17=Probe TX/Target RX | SDK/concurrency TBD |
| RF/BLE | Private 2.4 GHz with BLE management | Integrated shared Radio resource | SDK coexistence TBD |
| LED/Button | Status/pairing GPIO reserve | PA5 QFN48-37=LED reserve; PA9 QFN48-6=optional button reserve | PRD/hardware review decides population; polarity, current limit, pulls, and interrupt config TBD |
| HSE clock | 32 MHz external crystal | X32MO QFN48-31; X32MI QFN48-32 | Crystal parameters/layout per WCH reference design |
| LSE clock | Optional 32 kHz crystal | Reserve PA10 QFN48-48 and PA11 QFN48-47 | Need for external crystal TBD from WCH BLE/low-power SDK configuration |
| RF | BLE/private 2.4 GHz | ANT QFN48-34 to RF network/antenna | Datasheet recommends direct antenna connection; verify the final network against the WCH CH585M RF reference design, not yet obtained |
| DBG-C Interface | Basic/Full signals | MCU-side SWD/UART/Reset nets assigned; Type-C contact map not frozen | IF-001 must freeze connector mapping and electrical parameters |
| Probe Debug/Reset | Production/recovery | PB15 QFN48-11=TCK; PB14 QFN48-12=TIO; PB23 QFN48-25=chip RST | Preserve Probe programming/reset function |

## 5. V1 Peripheral Requirements and Schematic Pre-allocation

This is a **V1 schematic-input baseline**. It is not proof of MCU performance and does not freeze the Type-C pin mapping of DBG-C Interface.

| Function | MCU resource assignment | Schematic assignment | Notes and verification boundary |
|---|---|---|---|
| PC USB Device | USBFS controller/PHY + USB DMA | PB10 QFN48-16=UD−, PB11 QFN48-15=UD+; upstream USB-C | Use USBFS in V1 for CMSIS-DAP v2 Bulk + CDC. Verify descriptors and SDK Device stack; leave USBHS disabled |
| Target UART/CDC | UART0 + USB CDC virtual COM port | PB4 QFN48-20=Probe RX/Target TX, PB7 QFN48-17=Probe TX/Target RX; connector pins wait for IF-001 freeze | UART0 MODEM signals unused. CDC enumerates over USBFS and bridges to UART0 |
| Target SWD | Two GPIOs controlled by shared SWD Engine | PB5 QFN48-19=SWDIO, PB6 QFN48-18=SWCLK; connector pins wait for IF-001 freeze | GPIO-driven SWD; no SPI allocation. Verify timing and achievable SWD frequency with firmware waveform measurement |
| Target Reset | One GPIO control output | PA4 QFN48-36=Target_nRESET; connector pin waits for IF-001 freeze | Keep separate from CH585M PB23/RST at QFN48 pad 25; output stage/default state/target voltage pending electrical design |
| BLE + private 2.4 GHz | Integrated Radio/Baseband and ANT | ANT QFN48-34 to RF network/antenna | Datasheet recommends direct antenna connection; verify the final network against the WCH CH585M RF reference design, not yet obtained. Coexistence requires SDK confirmation; no external RF SPI assigned |
| Status/pairing interaction | GPIO reserves | PA5 QFN48-37=LED reserve; PA9 QFN48-6=optional button reserve | PRD/hardware review decides population; LED polarity/current limit and button pulls/debounce/wake policy TBD |
| HSE clock | External 32 MHz crystal network | X32MO QFN48-31; X32MI QFN48-32 | Datasheet marks external 32 MHz HSE crystal; verify component parameters against WCH reference design |
| LSE clock | Optional 32 kHz crystal network | Reserve PA10 QFN48-48 and PA11 QFN48-47 | Need for external crystal TBD from WCH BLE/low-power SDK configuration |
| USB/connection detection | GPIO only if selected circuit requires it | Reserve a VBUS/connection-detect net location; pin TBD | Do not assume USBFS automatically provides VBUS sensing; decide from SDK and power/connector design |
| Unique identity/pair settings | Chip UID + DataFlash | Firmware reads UID; DataFlash is unassigned until pairing-persistence requirements are frozen | UID API/length, write policy/endurance require SDK review; storage layout and pair persistence remain open |
| OTA and firmware | CodeFlash + BootLoader | Include programming/recovery circuitry per official programming/update design | Do not assume dual-image fits in 448 KB CodeFlash; signature, rollback, and power-fail safety are unverified |
| SWD timing/system time base | Timer if needed + GPIO | No Timer instance selected yet | First implement/measure GPIO SWD. If precision or CPU use misses targets, assign a Timer based on SDK support |
| DMA | USBFS DMA; Radio DMA TBD | No fixed DMA channel wiring | Datasheet lists USBFS DMA. DMA channel count and USB/RF arbitration require SDK/reference-manual confirmation |

### GPIO/Pin Reservation Constraints

1. MCU-side assignments: PB10 QFN48-16=USBFS UD−, PB11 QFN48-15=USBFS UD+, PB4 QFN48-20=Probe RX/Target TX, and PB7 QFN48-17=Probe TX/Target RX. Verify WCH SDK mux initialization before layout release.
2. MCU-side assignments: PB5 QFN48-19=SWDIO, PB6 QFN48-18=SWCLK, and PA4 QFN48-36=Target_nRESET. Reserve PA5 QFN48-37 for LED and PA9 QFN48-6 for an optional button. DBG-C Interface Type-C contact mapping remains unfrozen.
3. CH585M QFN48 pads 11/12 are PB15/TCK and PB14/TIO. Preserve them for Probe emulation debug; do not allocate them to Target SWD.
4. PB12 QFN48-14/PB13 QFN48-13 are USBHS U2D−/U2D+; V1 leaves USBHS unused. Do not confuse them with USBFS PB10/PB11.
5. The CH585M's own active-low reset input is the RST alternate function on PB23, QFN48 pad 25. PB22 QFN48 pad 26 lists TMR3/RXD2_ mux functions. Keep both separate from Target_nRESET on PA4.
6. PB5/PB6 also have UART0 DTR/RTS and PWM8 alternate functions, unused in V1. PA4/PA5/PA9 UART/LED/ADC alternate functions are also unused by this allocation.
7. The selected pins are not marked 5VT in the datasheet. Do not directly connect target signals until DBG-C Interface voltage and protection design is complete.

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

The project has selected ThreadX for V1 firmware. PoC-1 pins Eclipse ThreadX `v6.5.1.202602a_rel` (commit `b91b03b9e75fa523b17127f9e0eca09dca916459`), uses its `ports/risc-v32/gnu` context routines, and adds experimental CH585M low-level, SysTick, WCH startup, and clock adaptation. A host cross-build passed with MounRiver Linux x64 Toolchain V2.4.0 GCC 12.2.0; the WCH EVT `.cproject` selects a GCC12 configuration but does not record its patch version. The user confirms no hardware is currently available and requires software verification before hardware design, followed by verification on real hardware. A successful host build does not establish compatibility of QingKe RISC-V3C hardware stack push, VTF/HPE, exception return, or scheduling. Two clean rebuilds produced identical ELF/map files and static review is complete; the OPEN-001 O19 build and ELF static checks passed, but source configuration at 100 ticks/s conflicts with the stated 1000 ticks/s requirement, so verification-hardware design is not released. Board validation has not run, and product hardware freeze still requires board runtime evidence under O18/O19.

| Resource | ThreadX requirement | DBG-C allocation | Evidence boundary |
|---|---|---|---|
| Kernel tick | Periodic time base and interrupt | Designate the built-in 32-bit SysTick as the ThreadX kernel tick source; do not consume TMR0 through TMR3 | The PoC configures the EVT-defined SysTick API/IRQ at ThreadX upstream default 100 ticks/s; host build passes, but actual frequency, VTF/HPE, tick delivery, and wakeup are untested on board |
| Context switch | CPU context save/restore and scheduler entry | Experimental integration with WCH startup/interrupt framework; product thread/ISR boundaries remain to be defined | Uses upstream RISC-V32 context routines plus a local tick ISR; hardware exception frame, VTF/HPE, and restore path still require CH585M board verification |
| Interrupts | Peripheral ISR to ThreadX scheduling interface | Define ThreadX API boundaries for USB, Radio/BLE, and Timer ISRs in firmware architecture | Interrupt nesting, priorities, and SDK ISR constraints require SDK review |
| SRAM | ThreadX objects, system stack, thread stacks, application buffers | Budget USB DAP/CDC, RF reassembly, BLE stack, and SWD workspace together in the 128 KB SRAM linker layout | Allocate byte counts after measuring selected ThreadX/SDK, thread count, stack watermarks, and linker map; do not invent values |
| CPU | Scheduling, interrupt, and protocol processing | Verify worst-case response under concurrent USB/RF load | Datasheet maximum of 78 MHz alone does not prove performance targets |
| Compiler/Port | ThreadX port compatible with CH585M toolchain, ABI, and startup | ThreadX `v6.5.1.202602a_rel`, commit `b91b03b9e75fa523b17127f9e0eca09dca916459`; MounRiver GCC 12.2.0; WCH EVT GCC12 configuration | Host cross-build passes; EVT compiler patch version is not specified, board port compatibility is unverified |

ThreadX adds no external connector signal and does not change the assigned USBFS, UART0, SWD GPIO, Target Reset GPIO, and integrated Radio. Preserve Probe programming/recovery access. The current resource baseline designates SysTick for the ThreadX kernel tick and reserves TMR0 through TMR3; if port integration fails, document and review a resource change.

### ThreadX Upstream Dependency Source

V1 firmware references the official Eclipse ThreadX GitHub project through a **Git Submodule**: [eclipse-threadx/threadx](https://github.com/eclipse-threadx/threadx). The current PoC submodule is at `software/third_party/threadx/`, pinned to tag `v6.5.1.202602a_rel` and commit `b91b03b9e75fa523b17127f9e0eca09dca916459`; that checkout includes `LICENSE.txt` under MIT. This pin is used for host cross-build and does not establish CH585M board compatibility.

This upstream version contains `ports/risc-v32/gnu`; the PoC uses its generic context save/restore, thread-stack build, system-return, and interrupt-control routines. Local code provides CH585M low-level initialization and the SysTick entry. The WCH FreeRTOS sample is used only as a QingKe V3C startup/interrupt reference; its FreeRTOS task-switching code was not moved into ThreadX. HPE/PFIC/VTF, exception-frame, and scheduling behavior remain for board verification. Initialize submodules recursively when cloning DBG-C firmware so the checkout uses the exact ThreadX commit recorded by the superproject.

The pinned upstream version's root `LICENSE.txt` identifies the MIT License. Retain the required notices in the DBG-C third-party component inventory and release materials. This record is not legal advice. Upstream materials checked 2026-09-28.

## 7. Required Materials

Obtain the matching WCH SDK/example version, official package drawing, silicon revision/errata, USBFS Device example, CH585M RF antenna reference design, RF/BLE coexistence guidance, OTA/ISP/IAP examples and interfaces, and clock/electrical requirements. Record file version/hash and review date when updating this document.
