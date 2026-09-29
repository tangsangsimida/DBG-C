# DBG-C MCU Selection and Resource Assessment

**Document ID:** DBG-C-MCU-001　**Version:** V0.11　**Status:** CH585M V1 schematic resource allocation draft; PoC tick target 1000 ticks/s; two fixed-path clean-build reviews for 1000 ticks/s complete; board verification not run

## 1. Evidence Source

`docs/09-references/CH585-CH584_Datasheet_V1.6.pdf` is the project-designated CH585M IC manual. Its internal title is *CH585/CH584 Datasheet*, V1.6, 160 pages. Use it as the primary project source for chip parameters, pin multiplexing, and documented peripheral capabilities. SDK APIs, concrete project configuration, RF concurrency performance, and board behavior still require SDK review/measurement. Do not infer CH585M behavior from CH584 or other CH series.

The repository retains the official EVT archive `docs/09-references/CH585EVT/CH585EVT.ZIP` without extracting it wholesale. Files checked for this allocation include `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_gpio.h` (SHA-256 `c7f450ceaa501e4c5a912bc0429bca1540fa3c3f26e3fd55dfd908fad9b06183`), `EVT/EXAM/IAP/USBHS_IAP/src/Main.c` (SHA-256 `5f6edcee35151480161a032c2995b495e70b9ecd352c57c4a99ee351cb40476f`), and `EVT/PUB/CH585SCH.pdf` (SHA-256 `70391feaa719a7c5ffc1b738b927ba166bf2402692ec42f7042366839c1fb7bb`). The EVT GPIO header declares UART3 `RB_PIN_UART3` routing from PA4/PA5 to PB20/PB21; the USBHS IAP Main initializes the USBHS Device controller; the official reference schematic shows PB22 connected to a BOOT download switch. The archive index `EVT/CH585_List_EN.txt` is dated 2026.08 and declares no separate SDK semantic version. These examples do not prove DBG-C implementation or board validation.

## 2. Resource Table

| Resource | Datasheet statement | DBG-C assessment/status |
|---|---|---|
| CPU | QingKe RISC-V3C, RV32IMBC plus extensions; up to 78 MHz | Clock modes and SDK configuration TBD |
| FlashROM | 512 KB: 448 KB CodeFlash, 32 KB DataFlash, 24 KB BootLoader, 8 KB InfoFlash | Partitions, dual-image/rollback update, usable space TBD from SDK/Boot verification |
| SRAM | 128 KB: 96 KB RAM96K, 32 KB RAM32K | Peak buffers under concurrent RF/USB require measurement |
| USBFS | One FS USB 2.0 controller/PHY; 15 endpoints; 64-byte packets; DMA; Host/Device | Reserve PB10/PB11 for recovery, production, and debug access; USBHS is the V1 primary USB. USBFS Device stack and recovery flow remain to be implemented and verified |
| USBHS | One 480 Mbps USB 2.0 HS controller/PHY; 1024-byte packets; DMA; HS/FS Host/Device | V1 primary USB Device target; PB12=U2D− and PB13=U2D+. EVT includes USBHS Device and USBHS IAP examples; composite-device and ThreadX integration plus board-level HS signal behavior remain to be implemented/verified |
| UART | Four instances; 8-level FIFO; datasheet states up to 9 Mbps | Pins, clock accuracy, and target levels TBD |
| SPI | Two instances, Master/Slave, DMA | No evidence yet whether RF needs an external transceiver; internal RF path TBD from SDK |
| ADC | 12-bit; 14 external + 3 internal channels (overview) | Not mandatory in V1; package/mux channel check required |
| GPIO | The overview states 40 GPIOs, two with 5 V input tolerance and 32 with interrupt/wake capability | Table 1-1 CH585M column lists PA0–PA15 and PB0–PB23, 40 GPIO identifiers with package pad numbers, matching the overview count. This does not mean all 40 are freely allocatable or interrupt/wake capable. 5VT does not imply 5 V output |
| BLE/RF | BLE 5.4; integrated 2.4 GHz RF; 1/2 Mbps; mentions 2.4G mode up to 8 kHz report rate | Meaning of 2.4G mode, private PHY/API, RF DMA capability/API, BLE coexistence, and performance require SDK/reference-manual confirmation. Local datasheet extract does not confirm RF DMA |
| Timers/PWM | Four 26-bit timers; four capture channels; PWM resources in datasheet | Applicability to SWD timing requires SDK and waveform validation |
| UID/security | AES-128 and unique chip ID | EVT `ISP585.h` declares the low-level ROM command and its zero-success/nonzero-failure result; WCH `GET_UNIQUE_ID()` documents a 64-bit output and 4-byte buffer alignment, but its `void` wrapper discards command status. The DBG-C adapter checks the low-level status and follows the official UID-byte construction; uniqueness/stability claims and key boundaries still require security review, and silicon reads remain unverified |
| Boot/OTA | Datasheet states ICP/ISP/IAP and OTA wireless update support | Boot protocol, OTA APIs, rollback/signature/power-fail recovery require official evidence |
| Package | CH585M: QFN48 | Verify land pattern, dimensions, and pin table against package documentation |
| Clocks | Pin table identifies 32 MHz HSE crystal pins X32MO/X32MI and 32 kHz crystal functions on PA10/PA11 | Allocate the 32 MHz crystal network to QFN48 pads 31/32; confirm whether a 32 kHz crystal is required from WCH BLE/low-power SDK configuration |
| Debug | Single/dual-wire emulation debug; datasheet says PB15/PB14 are used when enabled | Production debug access, mux conflict, and production lock policy TBD |

## 3. CH585M QFN48 Pin Allocation Matrix

Package pin numbers below are read from the **CH585M column** of datasheet Table 1-1 (printed pages 5–8). MCU-side resources are updated as schematic inputs against the proposed V1 baseline. Pad numbers/muxes follow CH585M datasheet Table 1-1; UART3 remapping follows the EVT GPIO header; the PB22 BOOT net follows the EVT CH585M reference schematic. This does not freeze the DBG-C Interface Type-C contact map; target voltage, drive, protection, and peripheral behavior still require design and verification.

| QFN48 pad | GPIO/pin | V1 net allocation | Mux conflict/constraint |
|---:|---|---|---|
| 1 | VDCID | Connect per datasheet power circuit | Capacitor and DC-DC connection per datasheet |
| 2 | VSW | Connect per datasheet power circuit | DC-DC inductor/bypass per datasheet |
| 3 | VDD33 / VIO33 | Supply and I/O supply net | Verify decoupling and USB supply relationship against reference design |
| 4 | PA7 | Unallocated reserve | Do not enable TXD2, PWM5, LED6, or ADC A11 |
| 5 | PA8 | Reserve for official ISP UART resources | Datasheet mux is RXD1; exact ISP channel/entry method requires official download documentation |
| 6 | PA9 | Reserve for official ISP UART resources | Datasheet muxes are TMR0, TXD1, ADC A13; download use requires official documentation |
| 7 | PB9 | Unallocated reserve | GPIO/NFCI; NFC disabled in V1 |
| 8 | PB8 | Unallocated reserve | GPIO/NFCM; NFC disabled in V1 |
| 9 | PB17 | EXT_RESET_N | GPIO; EVT also lists RF antenna-switch control mux; verify that this GPIO output is not used by RF configuration |
| 10 | PB16 | EXT_IRQ | GPIO; EVT also lists RF antenna-switch control mux; verify that this GPIO output is not used by RF configuration |
| 11 | PB15 / TCK | CH585 emulation-debug clock | Dedicated to TCK when emulation debug is enabled; not Target JTAG |
| 12 | PB14 / TIO | CH585 emulation-debug data | Dedicated to TIO when emulation debug is enabled; not Target JTAG |
| 13 | PB13 / U2D+ | PC USBHS D+ | USBHS Device data line; do not connect to USBFS D+ |
| 14 | PB12 / U2D− | PC USBHS D− | USBHS Device data line; do not connect to USBFS D− |
| 15 | PB11 / UD+ | Reserve USBFS D+ recovery path | Recovery/production connector population TBD; keep separate from USBHS |
| 16 | PB10 / UD− | Reserve USBFS D− recovery path | Recovery/production connector population TBD; keep separate from USBHS |
| 17 | PB7 / TXD0 | TARGET_UART_TX | UART0 TXD0; connects to Target RX |
| 18 | PB6 | TARGET_PWR_EN | GPIO control; target power switch, voltage, and default state require power design |
| 19 | PB5 | TARGET_nRESET | GPIO control; polarity, driver, default state, and pulse width require IF/electrical design |
| 20 | PB4 / RXD0 | TARGET_UART_RX | UART0 RXD0; connects to Target TX |
| 21 | PB3 | TARGET_TDO | GPIO input used in JTAG mode; electrical constraints require IF design |
| 22 | PB2 | TARGET_TDI | GPIO output used in JTAG mode; electrical constraints require IF design |
| 23 | PB1 | TARGET_SWDIO_TMS | Bidirectional GPIO; SWDIO in SWD mode and TMS in JTAG mode |
| 24 | PB0 | TARGET_SWCLK_TCK | GPIO output; SWCLK in SWD mode and TCK in JTAG mode |
| 25 | PB23 / RST | CH585 active-low reset | Datasheet Table 1-1 identifies external reset input with internal pull-up; do not connect to Target_nRESET output |
| 26 | PB22 | CH585_BOOT board-control net | EVT CH585M reference schematic connects PB22 to a BOOT download switch; the chip mux table has no dedicated BOOT function. Entry condition/timing requires official download documentation |
| 27 | PB21 | Reserve | UART3 TXD3 remap target; reserve and distinguish from PB20 SWO receive |
| 28 | PB20 / RXD3_ | TARGET_SWO | UART3 RXD3 remap target; EVT `RB_PIN_UART3` remaps UART3 from PA4/PA5 to PB20/PB21 |
| 29 | PB19 | LED_RF | GPIO; EVT also lists RF antenna-switch control mux; verify that this output is not enabled |
| 30 | PB18 | LED_USB | GPIO; EVT also lists RF antenna-switch control mux; verify that this output is not enabled |
| 31 | X32MO | One side of 32 MHz crystal network | HSE crystal pin per datasheet; crystal parameters/load per WCH reference design |
| 32 | X32MI | Other side of 32 MHz crystal network | HSE crystal pin per datasheet; crystal parameters/load per WCH reference design |
| 33 | VINTA | Decoupling capacitor per datasheet | Value and routing per datasheet/reference design |
| 34 | ANT | RF network/antenna connection | The datasheet describes an RF input/output and recommends direct antenna connection. Verify the actual network against the WCH CH585M RF reference design, which is not present in this repository |
| 35 | VDCIA | Decoupling capacitor per datasheet | VDCIA/VDCID connection per datasheet |
| 36 | PA4 / A0 | TARGET_VTREF_ADC | ADC A0 mux; also default UART3 RX pin. Enable UART3 RX remap to PB20 to avoid conflict; front-end range/protection TBD |
| 37 | PA5 | Reserve | Default UART3 TX pin; verify mux setup when using PB20/PB21 remap; reserve |
| 38 | PA6 | Unallocated reserve | Do not enable RXD2, PWM4_, LED5, or ADC A10 |
| 39 | PA0 / SCK1 | EXT_FLASH_SCK | SPI1 SCK1 mux; bus mode/clock TBD in driver design |
| 40 | PA1 / MOSI1 | EXT_FLASH_MOSI | SPI1 MOSI1 mux |
| 41 | PA2 / MISO1 | EXT_FLASH_MISO | SPI1 MISO1 mux |
| 42 | PA3 | EXT_FLASH_CS | GPIO chip select; datasheet does not list a dedicated SPI1 CS mux |
| 43 | PA15 | Reserve for official ISP UART resources | Datasheet lists UART0 RXD0_ remap; confirm whether this is an ISP channel from official download documentation |
| 44 | PA14 | Reserve for official ISP UART resources | Datasheet lists UART0 TXD0_ remap; confirm whether this is an ISP channel from official download documentation |
| 45 | PA13 | Unallocated reserve | Do not enable SPI0 SCK or PWM5 |
| 46 | PA12 | Unallocated reserve | Do not enable SPI0 SCS or PWM4 |
| 47 | PA11 / X32KO | Reserve for 32 kHz clock review; no external signal | Low-frequency oscillator output; WCH BLE/low-power SDK configuration must confirm whether a 32 kHz crystal is needed |
| 48 | PA10 / X32KI | Reserve for 32 kHz clock review; no external signal | Low-frequency oscillator input; WCH BLE/low-power SDK configuration must confirm whether a 32 kHz crystal is needed |

Note: pad numbers come from the CH585M column of datasheet Table 1-1 (printed pages 5–8). The table lists all 40 PA/PB GPIO identifiers with package pad numbers, as well as DBG-C-related power, clock, and RF pins. Verify interrupt/wake capability per pin against applicable chip data and the SDK.

## 4. Resource Allocation Status

| Function | Planned need | MCU resource/pin allocation | Status |
|---|---|---|---|
| PC USB Device | USBHS CMSIS-DAP v2 Bulk, CDC ACM, and management channel | USBHS; PB12 QFN48-14=U2D−, PB13 QFN48-13=U2D+ | EVT has USBHS Device/IAP examples; composite descriptors, ThreadX integration, and HS board enumeration remain to be implemented/verified |
| USBFS recovery | Production/recovery USB path | PB10 QFN48-16=UD−, PB11 QFN48-15=UD+ | Reserved; connector and complete official ISP recovery flow TBD |
| Target SWD/JTAG | Shared GPIO engine | PB0 QFN48-24=SWCLK/TCK; PB1 QFN48-23=SWDIO/TMS; PB2 QFN48-22=TDI; PB3 QFN48-21=TDO | Mux assignments checked; waveform, speed, and DAP JTAG support require software/board testing |
| Target reset/power | Reset and target power enable | PB5 QFN48-19=TARGET_nRESET; PB6 QFN48-18=TARGET_PWR_EN | GPIO pads verified; level shifting, default state, power protection, and control logic TBD |
| Target UART/CDC | Full-duplex UART bridge | UART0; PB4 QFN48-20=RXD0, PB7 QFN48-17=TXD0 | EVT UART0 pins and interface verified; CDC bridge, baud rate, and concurrency TBD |
| SWO | NRZ SWO receive | PB20 QFN48-28=RXD3_; UART3 RX remapped to PB20 by EVT `RB_PIN_UART3` | Header declares remap; SWO sampling, baud rate, and CMSIS-DAP output path TBD |
| Target VTref | Target voltage sensing | PA4 QFN48-36=ADC A0 | ADC mux name confirmed; divider, clamp, range, calibration, and thresholds TBD |
| External SPI NOR | Update cache, rollback, logs, and offline image candidate storage | SPI1: PA0=SCK1, PA1=MOSI1, PA2=MISO1; PA3=GPIO CS | Pin muxes checked; part/capacity/protocol/clock/partition TBD; this does not freeze Flash layout |
| Private RF/BLE | Private 2.4 GHz and BLE management | Integrated Radio; ANT QFN48-34 | EVT RF examples exist; protocol, DMA, concurrency, and performance require source/library review and board testing |
| Probe debug/reset | Production debug and recovery | PB15 QFN48-11=TCK; PB14 QFN48-12=TIO; PB23 QFN48-25=RST | Preserve per datasheet; production access policy TBD |
| BOOT control | Official download/recovery entry | PB22 QFN48-26 board-level BOOT net | Reference schematic confirms net; chip mux table has no dedicated BOOT function; entry conditions require official documentation |
| UI | Mode key and status indicators | PB8=KEY_MODE; PB9=LED_TARGET; PB18=LED_USB; PB19=LED_RF | GPIO pads; PB18/PB19 also fall in EVT RF antenna-switch control mux range; verify that remap is disabled |
| Expansion control | Expansion interrupt/reset | PB16=EXT_IRQ; PB17=EXT_RESET_N | GPIO pads; also in EVT RF antenna-switch control mux range |

## 5. V1 Peripheral Requirements and Schematic Pre-allocation

The table cross-checks the proposed V1 pin baseline against official materials. Pad/mux assignments follow the CH585M column of CH585/CH584 Datasheet V1.6 Table 1-1; GPIO remapping and UART3 routing follow `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_gpio.h`; the PB22 BOOT net follows `EVT/PUB/CH585SCH.pdf`. This does not prove drivers, combined operation, or board electrical behavior, and does not freeze DBG-C Interface Type-C contacts.

| Function | MCU resource | Schematic assignment | Evidence and remaining work |
|---|---|---|---|
| PC USB | USBHS Device | PB12=USBHS D−, PB13=USBHS D+ | Datasheet pin table and EVT `USBHS_IAP`/USBHS Device examples confirm peripheral/example presence; DAP Bulk+CDC composite descriptors, endpoint plan, ThreadX integration, and HS signal measurement remain |
| USB recovery | Reserve USBFS Device | PB10=USBFS D−, PB11=USBFS D+; reserve production/recovery access | Datasheet pins confirmed; recovery entry and connector decision TBD |
| Target SWD/JTAG | Shared SWD/JTAG GPIO backend | PB0=SWCLK/TCK, PB1=SWDIO/TMS, PB2=TDI, PB3=TDO | GPIO muxes valid; IO voltage, output topology, translators, and timing TBD in IF/validation board |
| Target nRESET | GPIO | PB5=TARGET_nRESET | Separate from chip PB23/RST; driver, default state, pulse width TBD |
| Target power control | GPIO | PB6=TARGET_PWR_EN | External load switch, current limit, reverse current, fault handling, and default state require power design |
| Target UART/CDC | UART0 + USB CDC ACM | PB4=Probe RX/Target TX; PB7=Probe TX/Target RX | EVT remap documentation and datasheet pin mapping agree; baud, CDC interfaces/endpoints, and concurrency TBD |
| SWO | UART3 RX | PB20=RXD3_; apply EVT `RB_PIN_UART3` remap | Default PA4/PA5 UART3 route conflicts with PA4 ADC allocation; apply remap and verify actual initialization |
| VTref | ADC A0 | PA4=TARGET_VTREF_ADC | PA4 also has default UART3 RX mux; remap UART3 to PB20. Do not connect arbitrary target voltage directly to the MCU; range/protection require calculation |
| External Flash | SPI1 + GPIO CS | PA0=SCK1, PA1=MOSI1, PA2=MISO1, PA3=CS | Datasheet lists SPI1 signal muxes; CS is GPIO. Part, capacity, compatibility, and offline-image need TBD |
| CH585 debug/reset | Emulation debug and RST | PB14=TIO; PB15=TCK; PB23=RST | Separate from Target SWD/JTAG; keep reset accessible |
| BOOT | Board-level download control | PB22 connected to BOOT switch/circuit | EVT `CH585SCH.pdf` confirms reference-board net; do not describe PB22 as dedicated chip BOOT mux. Entry conditions/sequence require official download documentation |
| ISP UART resources | Reserve possible download UART routes | Reserve PA8, PA9, PA14, PA15 | Datasheet confirms UART muxes, but current evidence does not establish that all four are used by official ISP; verify official download docs/SDK before consuming |
| UI | GPIO | PB8=KEY_MODE; PB9=LED_TARGET; PB18=LED_USB; PB19=LED_RF | GPIO pads; PB18/PB19 are in EVT RF antenna-switch output mux range; verify initialization does not enable it |
| Expansion | GPIO | PB16=EXT_IRQ; PB17=EXT_RESET_N; reserve PB21 | EVT describes RF antenna-switch outputs on PB16–PB21; confirm RF setup does not use these GPIO outputs |
| RF | Integrated 2.4 GHz/BLE Radio | ANT QFN48-34 routed per WCH RF reference design | EVT RF examples exist; matching network, antenna, keepout, and USB coexistence layout must come from official board material; do not guess values |

### GPIO/Pin Constraints

1. PB12/PB13 are USBHS U2D−/U2D+; PB10/PB11 are USBFS UD−/UD+. Keep the pairs separate.
2. Target SWD/JTAG use PB0–PB3; Target UART uses PB4/PB7; Target reset/power enable use PB5/PB6. This matches the CH585M datasheet pin table; electrical compatibility and timing remain unverified.
3. PB14/PB15 are CH585 TIO/TCK; PB23 RST is the chip's own active-low external reset. Keep both separate from Target interface nets.
4. PB20 can be UART3 RXD3 through the EVT UART3 remap; this allows PA4/A0 to remain available for VTref ADC. Without remapping, UART3 RX conflicts with PA4 allocation.
5. Describe PB22 only as a board-level BOOT control net: the reference schematic shows a switch connected to it, but the chip pin table lists no dedicated BOOT mux. Confirm entry requirements from official ISP/download documentation.
6. EVT GPIO remap documentation includes RF antenna-switch outputs on PB16–PB21. Whether EXT_IRQ, EXT_RESET_N, LEDs, and SWO coexist depends on whether RF initialization enables this output; inspect source/library configuration and disable conflicting routing before board testing.
7. Targets may use 1.8 V or 3.3 V. GPIO mux data does not prove tolerance at arbitrary voltage; separately design VTref input, target signal translation, protection, and power-off isolation.

### V1 Peripheral Scope

The V1 baseline includes USBHS, a reserved USBFS recovery path, CMSIS-DAP v2 Bulk, SWD, JTAG, SWO, Target UART/CDC, BLE, private 2.4 GHz, USB and wireless self-update architecture, VTref sensing, target power control, and external SPI NOR resources. Implementation order and acceptance remain governed by PRD/FW/TEST documents. JTAG/SWO and update transports are design/implementation work, not existing or measured product capability.

### Conclusion

The current allocation can proceed as a schematic-design input based on the datasheet pin table and EVT mux evidence. Remaining design gates include PB16–PB21 RF antenna-switch interaction, BOOT entry conditions, USBHS composite endpoints and ThreadX integration, target-level translation and power protection, RF/BLE coexistence, and external Flash selection/capacity. Do not mark these items implemented or measured as passing.

### Memory Budgeting Approach

| Memory | Datasheet capacity/partition | Current plan |
|---|---|---|
| CodeFlash | 448 KB user application area | Hold for DBG-C application and portable CMSIS-DAP code. Partition only after measuring SDK and DAP build; do not assume dual-image OTA |
| BootLoader | 24 KB system boot area | Preserve for chip/official update use; verify whether product secure boot/rollback is supported |
| DataFlash | 32 KB user nonvolatile data | Unassigned until configuration and pairing-persistence requirements are frozen; endurance/atomic update/API TBD |
| InfoFlash | 8 KB system configuration | Do not use as ordinary product storage; follow official definition |
| SRAM | 128 KB total | Include USB DAP buffers, USB CDC rings, RF RX/TX/reassembly, BLE stack, SWD workspace, and task stacks in linker-map/peak budget. Byte counts require SDK integration/measurement; do not invent fixed percentages |

## 6. Additional Resource Requirements for ThreadX

The project has selected ThreadX for V1 firmware. PoC-1 pins Eclipse ThreadX `v6.5.1.202602a_rel` (commit `b91b03b9e75fa523b17127f9e0eca09dca916459`), uses its `ports/risc-v32/gnu` context routines, and adds experimental CH585M low-level, SysTick, WCH startup, and clock adaptation. A host cross-build passed with MounRiver Linux x64 Toolchain V2.4.0 GCC 12.2.0; the WCH EVT `.cproject` selects a GCC12 configuration but does not record its patch version. The user confirms no hardware is currently available and requires software verification before hardware design, followed by verification on real hardware. A successful host build does not establish compatibility of QingKe RISC-V3C hardware stack push, VTF/HPE, exception return, or scheduling. The two matching clean builds, ELF/map files, and static review were for the earlier 100 ticks/s configuration. The current 1000 ticks/s source has completed two clean builds at a fixed path with matching ELF/map hashes and ELF/link resource review. Under OPEN-001 O19, the verification-board software gate is re-reviewed and passed. This does not establish board runtime success or release product-hardware freeze. The PoC uses the decided 1000 ticks/s target; actual frequency and tick delivery remain for board verification, with tolerance to be frozen in the test plan. Board validation has not run, and product hardware freeze still requires board runtime evidence under O18/O19.

| Resource | ThreadX requirement | DBG-C allocation | Evidence boundary |
|---|---|---|---|
| Kernel tick | Periodic time base and interrupt | Designate the built-in 32-bit SysTick as the ThreadX kernel tick source; do not consume TMR0 through TMR3 | The PoC configures the EVT-defined SysTick API/IRQ explicitly at 1000 ticks/s; actual frequency, VTF/HPE, tick delivery, and wakeup are untested on board |
| Context switch | CPU context save/restore and scheduler entry | Experimental integration with WCH startup/interrupt framework; product thread/ISR boundaries remain to be defined | Uses upstream RISC-V32 context routines plus a local tick ISR; hardware exception frame, VTF/HPE, and restore path still require CH585M board verification |
| Interrupts | Peripheral ISR to ThreadX scheduling interface | Define ThreadX API boundaries for USB, Radio/BLE, and Timer ISRs in firmware architecture | Interrupt nesting, priorities, and SDK ISR constraints require SDK review |
| SRAM | ThreadX objects, system stack, thread stacks, application buffers | Budget USB DAP/CDC, RF reassembly, BLE stack, and SWD workspace together in the 128 KB SRAM linker layout | Allocate byte counts after measuring selected ThreadX/SDK, thread count, stack watermarks, and linker map; do not invent values |
| CPU | Scheduling, interrupt, and protocol processing | Verify worst-case response under concurrent USB/RF load | Datasheet maximum of 78 MHz alone does not prove performance targets |
| Compiler/Port | ThreadX port compatible with CH585M toolchain, ABI, and startup | ThreadX `v6.5.1.202602a_rel`, commit `b91b03b9e75fa523b17127f9e0eca09dca916459`; MounRiver GCC 12.2.0; WCH EVT GCC12 configuration | Host cross-build passes; EVT compiler patch version is not specified, board port compatibility is unverified |

ThreadX adds no external connector signal; current allocation is USBHS primary Device, reserved USBFS recovery, UART0, PB0–PB3 Target SWD/JTAG, PB5/PB6 Target reset/power, and integrated Radio. Preserve Probe programming/recovery access. The current resource baseline designates SysTick for the ThreadX kernel tick and reserves TMR0 through TMR3; if port integration fails, document and review a resource change.

### ThreadX Upstream Dependency Source

V1 firmware references the official Eclipse ThreadX GitHub project through a **Git Submodule**: [eclipse-threadx/threadx](https://github.com/eclipse-threadx/threadx). The current PoC submodule is at `software/third_party/threadx/`, pinned to tag `v6.5.1.202602a_rel` and commit `b91b03b9e75fa523b17127f9e0eca09dca916459`; that checkout includes `LICENSE.txt` under MIT. This pin is used for host cross-build and does not establish CH585M board compatibility.

This upstream version contains `ports/risc-v32/gnu`; the PoC uses its generic context save/restore, thread-stack build, system-return, and interrupt-control routines. Local code provides CH585M low-level initialization and the SysTick entry. The WCH FreeRTOS sample is used only as a QingKe V3C startup/interrupt reference; its FreeRTOS task-switching code was not moved into ThreadX. HPE/PFIC/VTF, exception-frame, and scheduling behavior remain for board verification. Initialize submodules recursively when cloning DBG-C firmware so the checkout uses the exact ThreadX commit recorded by the superproject.

The pinned upstream version's root `LICENSE.txt` identifies the MIT License. Retain the required notices in the DBG-C third-party component inventory and release materials. This record is not legal advice. Upstream materials checked 2026-09-28.

## 7. Required Materials

Obtain the matching WCH SDK/example version, official package drawing, silicon revision/errata, USBHS Device and USBFS recovery examples/entry instructions, CH585M RF antenna reference design, RF/BLE coexistence guidance, OTA/ISP/IAP entry/interfaces, Target signal electrical requirements, and clock requirements. Record file version/hash and review date when updating this document.
