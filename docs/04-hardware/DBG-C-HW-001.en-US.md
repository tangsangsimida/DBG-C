# DBG-C Hardware Design Specification

**Document ID:** DBG-C-HW-001　**Version:** V0.5　**Status:** Validation-board design-input draft; active Target output scope fixed at 3.3 V only; contact mapping, translation/protection circuits, and part numbers remain unfrozen; not a product-board freeze basis

## 1. Scope and Design Gate

This document defines electrical design inputs for the CH585M validation board and future product hardware. MCU package pins and peripheral allocation are controlled by [MCU-001](DBG-C-MCU-001.en-US.md); the connector contacts between DBG-C and a Target remain controlled by [IF-001](../03-interfaces/DBG-C-IF-001.en-US.md). The current software gate permits validation-board design only. A validation board is for measurement and does not mean the V1 product circuit or PCB is frozen.

Disposition: **Validation-board schematic work may continue only for the core, USB PHY, isolated test access, and peripheral modules whose signal names/directions/MCU resources/power domains/safe defaults are defined. Circuits with unresolved translator selection, Target power, connector contacts, or component-level fault parameters must remain modular and must not be treated as finalized circuits.** PCB layout and product-hardware gates have not passed.

## 2. Evidence and Limits

The repository [CH585/CH584 Datasheet V1.6](../09-references/CH585-CH584_Datasheet_V1.6.pdf) is the project baseline; its SHA-256 is `68270bbf3424956f36183bb6f0fef5b8892b540026f56fab3a7c0a1976d596f3f`. The official EVT archive index is dated 2026.08. Reviewed archive files include `EVT/PUB/CH585SCH.pdf`, USBHS IAP, GPIO/UART/SPI/ADC drivers, and RF examples. The EVT schematic is reference evidence, not an approved DBG-C circuit. Prebuilt RF-library behavior, silicon electrical behavior, and board results cannot be inferred from an example. The archive hash and handling rules are in the [EVT reference note](../09-references/CH585EVT/README.md).

| Evidence class | Meaning |
|---|---|
| Manual/source confirmed | Confirms only functions, package pins, or software interfaces explicitly stated in the source |
| Validation-board input | A connection, jumper, or test point retained to obtain measurements; not a product commitment |
| Pending verification | Official evidence, selection data, or board results are insufficient; do not insert experience-based values |

## 3. Schematic Module Hierarchy

No EDA project or hardware naming convention is currently committed. The following is a suggested validation-board sheet hierarchy; final names shall follow the selected EDA project rules.

| Sheet | Suggested module | Design boundary |
|---:|---|---|
| 01 | CH585M core | QFN48, all supply pins, decoupling, reference clocks, reset |
| 02 | Power entry and tree | PC VBUS, system supply, probe supply, Target power isolation |
| 03 | Programming, debug, and recovery | TIO/TCK, RST, BOOT, recoverable USBFS/UART access |
| 04 | USBHS PC interface | USB-C receptacle, CC, VBUS detection, D+/D−, ESD |
| 05 | USBFS recovery interface | Independently accessible recovery connection; no USBHS wiring tie |
| 06 | Target debug and UART | VTref-associated level-adaptation front ends for SWD/JTAG/UART/Reset; isolatable test points on both sides of the adaptation path, with no Target operating path that bypasses adaptation |
| 07 | VTref and Target power | ADC front end, level compatibility, current/back-feed protection |
| 08 | External SPI NOR | PA0–PA3, device footprint and bus test points; part TBD |
| 09 | BLE/private RF | RF supply, official reference network, antenna and test boundary |
| 10 | Key, LEDs, and test points | GPIO loads, observable signals, configurable jumpers |

## 4. MCU Pin and Net Baseline

The MCU-side nets below follow the current MCU-001 allocation. They do not define Type-C connector contacts and do not prove simultaneous operation.

| Pin | MCU-side net/use | Schematic constraint |
|---|---|---|
| PB0 / PB1 | TARGET_SWCLK_TCK / TARGET_SWDIO_TMS | Shared SWD/JTAG signals; retain separate test points and an isolatable path |
| PB2 / PB3 | TARGET_TDI / TARGET_TDO | JTAG signals; unused state requires firmware/electrical definition |
| PB4 / PB7 | TARGET_UART_RX / TARGET_UART_TX | MCU perspective; connect to Target TX / RX through an electrical front end |
| PB5 | TARGET_RESET_ASSERT (reset-stage control input) | Separate from MCU PB23/RST; external interface net TARGET_nRESET must pull low/release in the Target voltage domain |
| PB6 | TARGET_PWR_EN | GPIO control resource only; do not drive a load directly |
| PA4 | TARGET_VTREF_ADC | ADC A0; voltage/current limiting required; no assumed arbitrary VTref compatibility |
| PB20 | TARGET_SWO | UART3 RX remap resource; resolve RF antenna-switch mux conflict |
| PB10 / PB11 | USBFS D− / D+ | Recovery channel; do not connect to USBHS |
| PB12 / PB13 | USBHS D− / D+ | Primary PC USB Device interface |
| PB14 / PB15 | CH585 TIO / TCK | Probe self-debug; keep separate from Target debug nets |
| PB22 / PB23 | BOOT board net / CH585 RST | BOOT entry conditions TBD; PB23 is MCU reset |
| PA0–PA2 / PA3 | SPI1 SCK/MOSI/MISO / GPIO CS | SPI1 transfer/recovery not implemented; PA3 CS is project allocation |
| PB8 / PB9 / PB18 / PB19 | KEY_MODE / LED_TARGET / LED_USB / LED_RF | PB18–PB19 conflict with RF antenna-switch mux remains unresolved |
| PB16 / PB17 / PB21 | EXT_IRQ / EXT_RESET_N / reserve | Potential RF antenna-switch conflict; use isolatable pads |

## 5. CH585M Core, Power, and Clocks

### 5.1 Supply Nets

The CH585M supply-pin evidence is in datasheet §1.2 Table 1-1, §5.1 Figure 5-1, §22.2 Table 22-2, and EVT `EVT/PUB/CH585SCH.pdf`. The current evidence gives these connection/value inputs; the DC-DC mode, VDD33/VIO33 supply relationship, and board-level component values require a power-design decision. Do not mix values for different modes.

| Net/pin | Official function/recommendation | Schematic decision status |
|---|---|---|
| VDD33 | System supply input. Table 22-2: 3.15–3.45 V when USB is used; 1.85–3.6 V otherwise | USBHS V1 design uses USB condition for review; system regulator/source budget remains open |
| VIO33 | GPIO and Flash I/O supply. Table 22-2 gives 3.15–3.45 V with USB and 1.85–3.6 V without USB | Do not merge with Target VTref; review relationship to VDD33 against WCH reference circuit |
| VDCID | Internal digital LDO supply input; with DC-DC, recommended 4.7 µF (supports 1–10 µF); without DC-DC, at least 1 µF recommended | Select by final DC-DC mode; part/layout review pending |
| VSW | DC-DC switch output; when enabled, place an inductor close to the pin in series to VDCID, recommended 10 µH (supports 4.7–22 µH); when disabled, it may connect directly to VDCID | DC-DC mode and inductor require power/startup review; do not mix mode-specific wiring |
| VDD33 decoupling | Datasheet pin table: with DC-DC, 2.2 µF or 1 µF recommended; without DC-DC, at least 1 µF recommended | Review with source impedance, USB load, and layout |
| VINTA | Internal analog supply node; 0.47 µF recommended without DC-DC; with DC-DC at least 0.47 µF, supports 0.47–2.2 µF | Implement for selected mode and WCH layout requirements |
| VDCIA | Internal analog LDO supply input; 0.1 µF recommended and connect directly to VDCID | Cross-check each net against reference circuit |
| GND/exposed pad | Common ground and device substrate | Review pad connection against package data and WCH land pattern |

Datasheet §5.1 states that system supply enters through VDD33 and GPIO/Flash I/O supply through VIO33. Direct feed is the power-up default; DC-DC is an optional efficiency mode. The validation board shall select one mode and keep supply wiring, decoupling/inductor, and firmware configuration consistent. WCH EVT topology is a reference, not a substitute for DBG-C power budgeting and PCB review.

### 5.2 Decoupling and Bulk Storage

Place decoupling near each supply pin per WCH recommendations. RF/USB load transients and supply integrity require review against the official reference design. Values, count, packages, and placement must be recorded in schematic/BOM review; this document does not guess them. Provide measurement access to MCU rails, VBUS, and Target supply.

### 5.3 Clocks

Connect the 32 MHz crystal to X32MI/X32MO and check load capacitance, ESR, drive, and layout against the datasheet and EVT reference. The EVT schematic annotates a 32 MHz ±10 ppm, 12 pF, 30 Ω crystal example. That annotation describes the example part only and is not automatically a DBG-C purchasing specification. Obtain the selected crystal's full data and calculate the load.

PA10/PA11 provide 32 kHz crystal functions. Assembly depends on the CH585 BLE/RF SDK requirements for low-speed clock source, accuracy, power, and RF behavior. The current review has not produced a traceable decision; reserve optional pads on the validation board, with values and population TBD.

### 5.4 MCU Reset, TIO/TCK, BOOT, and Recovery

PB23/RST is CH585 reset; PB14/TIO and PB15/TCK are the chip's own debug interface. Keep accessible debug/reset points on the validation board. PB22 connects to a board-level BOOT download switch in the WCH reference schematic, but the reviewed manual/examples do not establish entry level, reset-sampling order, or the corresponding ISP medium. Do not freeze a button or pull value from this alone.

Recovery must not depend on application firmware running. The exact combination and entry sequence for USBFS PB10/PB11 and official UART ISP-related PA8/PA9/PA14/PA15 require WCH's official download instructions. Until obtained, retain accessible recovery pads/test points. Application-level USB update must not be the only recovery path.

## 6. USB Hardware Interfaces

### 6.1 USBHS PC Interface

PB12=USBHS D− and PB13=USBHS D+ are supported by the datasheet and EVT examples, so schematic work on the USBHS PHY module may continue. Type-C receptacle CC1/CC2, VBUS, GND, Shield, Device pull-down/power role, VBUS detection, ESD, series components, and differential routing must each be checked against USB-IF Type-C/USB specifications and WCH's official reference design. Component values, differential impedance, and protection part are not frozen.

CMSIS-DAP v2 Bulk, CDC ACM descriptors/endpoints, and VID/PID belong to firmware/USB specifications. Their lack of freeze does not block drawing the physical PHY, but enumeration compatibility must not be claimed. Ordinary USB-C PC cables are for the PC USB function; Target custom signals must not use those cable contacts.

### 6.2 USBFS Recovery

PB10/PB11 are USBFS D−/D+. Concurrent USBFS and USBHS operation in one firmware remains unverified. Provide an independently accessible recovery connector/pads and use a jumper or explicit isolation so the two PHYs are not electrically tied together. Connector choice, recovery firmware entry, and simultaneous power relationship require design verification.

## 7. DBG-C Target Interface and Cables

IF-001 Basic/Full remain conceptual and define no connector contacts. The USB-IF USB Type-C Cable and Connector Specification Release 2.5 (2026-04-08) is the formal revision found in this review; its scope explicitly limits third-party functionality of Type-C connectors/cables to functionality described by the specification. Therefore, Type-C receptacle contacts cannot be treated as arbitrary DBG-C signal conductors, and an ordinary passive USB 2.0 cable cannot be described as carrying SWD/UART/Reset.

| Contact/conductor class | USB specification use | DBG-C validation-board conclusion |
|---|---|---|
| D+/D− | USB 2.0 data | Use for PC USBHS/USBFS; do not repurpose as Target signals |
| CC1/CC2 | Type-C attach/orientation/power-role related | Do not use as Target GPIO; implement per USB Type-C specification |
| VBUS/GND | Bus power/return | Do not connect to Target power without analyzed isolation |
| SBU1/SBU2 | Defined sideband use | Conductor continuity in an ordinary USB 2.0 cable cannot be assumed; do not freeze custom mapping |
| SuperSpeed TX/RX pairs | USB 3.x high-speed differential links | Not guaranteed in passive USB 2.0 cables; active cables may retime/convert and cannot be assumed transparent for custom low-speed signals |
| Shield | Shield/chassis related | Bonding strategy requires EMC/ESD and power review |

There are currently **no approved rows** for Connector Contact→Cable Conductor→Target Signal mapping. Select a specific connector/cable assembly and prove it with formal construction data, orientation matrix, and continuity measurements before populating that mapping. Receptacle orientation, CC orientation detection, and any MUX are also unfrozen. Bring all Target SWD/JTAG/UART/nRESET/SWO/VTref/Target Power/GND signals to separate test points or isolatable headers so Type-C experiments cannot block other tests.

## 8. Target Electrical Front Ends

### 8.1 Voltage Compatibility, SWD/JTAG, and UART

The DBG-C V1 externally powered Target I/O domains are frozen at **nominal 1.8 V and 3.3 V**. CH585M remains in the Probe-side I/O domain. Every Target-facing digital signal shall pass through VTref-associated level adaptation and be isolated when either Probe or Target is unpowered. A direct CH585M GPIO-to-Target operating path is prohibited. The Target-side logic domain follows valid Target VTref; no user software voltage selector is required. This freezes the interface architecture, not the numeric tolerance around either nominal voltage.

The following are mandatory translator-selection criteria; close each against the selected component data sheet and validation-board measurements before product schematic freeze:

- Probe-side supply is the DBG-C Probe I/O domain; Target-side supply/reference follows Target VTref through a reviewed and protected path and supports the frozen nominal 1.8 V and 3.3 V domains.
- With either supply absent, the device provides Ioff or a documented functionally equivalent partial-power-down isolation behavior. Target-side outputs can be held high impedance or in another reviewed safe state.
- Input thresholds must be specified for the selected supply conditions. Do not rely on CH585M GPIO directly recognizing Target-domain levels.
- SWDIO requires explicit direction control. The translator and control path must meet SWD turnaround timing when direction changes are synchronized with the SWD bit engine. Do not select an auto-direction device without evidence that it supports the push-pull SWD waveform and turnaround.
- Propagation delay, output drive, edge rate, loading, and channel skew must fit the eventual SWD/JTAG/SWO/UART timing budgets. These budgets and maximum rates remain to be established; no numeric component limit is asserted here.
- A component intended only for open-drain buses must not carry push-pull SWD/JTAG signals unless the exact circuit is formally reviewed and validated.
- Review absolute maximum ratings, powered-off injection/back-feed, ESD, fault voltage, hot-plug, and output-enable default behavior for every channel.

Signal direction requirements: SWCLK/TCK, TDI, and Probe UART TX are Probe-to-Target outputs; TDO, Target UART TX, and SWO are Target-to-Probe inputs; SWDIO is bidirectional in SWD; TMS is Probe-to-Target in JTAG. Keep replaceable footprints and test points on the validation board, with no operating bypass around the adaptation path.

### 8.2 Target nRESET

Firmware direction-switch emulation does not prove a hardware open-drain output. nRESET is in the Target voltage domain and must default to released; Probe power-off/reset, Bootloader, or firmware failure shall not assert it. The output stage shall pull low and release, with the released level established from Target-side VTref; it shall not drive a fixed 3.3 V high level into Target. Provide powered-off isolation and inhibit assertion when VTref is invalid. Part, pull value, polarity, pulse width, and fault behavior require component review and board verification.

### 8.3 VTref Detection

VTref is the Target's actual I/O supply, not a Probe-generated reference. The V1 external-Target requirement covers nominal 1.8 V and 3.3 V domains. PA4/A0 detection must distinguish valid Target supply, power-off, and abnormal states. PA4/A0 currently has a raw ADC BSP and modeled-register host checks only; silicon sampling, divider, range, error, and protection are unverified. Calculate the allowed minimum/maximum, fault range, divider resistance, input impedance, RC, clamp, calibration, and error budget from CH585 ADC input/reference, injection-current, accuracy, and selected translator specifications. No nominal voltage alone defines a valid threshold.

VTref sensing and the translator reference supply are separate electrical paths/functions. The protected Target VTref feed shall supply/reference the Target side of the level adapters; do not use the PA4 divider node as translator supply. Analyze loading and startup order separately. Invalid, absent, or abnormal VTref shall cause hardware inhibition of Probe-to-Target outputs and safe isolation. Software may report voltage and validity but does not select the electrical voltage domain.

### 8.4 Target Power

Support for an externally powered Target at nominal 1.8 V or 3.3 V is mandatory in V1. The user has decided that DBG-C active Target Power **sources nominal 3.3 V only, not 1.8 V**; a 1.8 V Target must be externally powered, with DBG-C following valid VTref. This product-scope decision does not freeze the circuit. PB6 is only a control GPIO. Validation-board review inputs include a controlled load switch, current limiting, external-supply conflict protection, fault reporting, and default-off behavior. Output accuracy/tolerance, continuous current, inrush, and thermal conditions must be frozen after review against the power tree, component data sheets, and Target load.

The part evaluation input submitted on 2026-09-30 is TPS22950CDDCR, RILIM 2.21 kΩ, CIN/COUT 1 µF each, and a typical current-limit setting near 500 mA. TI documentation associates 2.21 kΩ with a typical setting near 500 mA; it is not a precision guaranteed threshold. Orderable package, reverse current with an external supply, fault pin, thermal/inrush behavior, output capacitance, and upstream USB budget have not been reviewed item by item. These part/value selections are circuit-review input only, not a frozen BOM or a 500 mA product guarantee.

## 9. SPI NOR, RF, and UI

### 9.1 SPI NOR

PA0–PA2 are allocated to SPI1 and PA3 to GPIO CS. The transfer driver, timeout recovery, and storage model are unfinished; Flash capacity must not be inferred from PoC image size. Before selection, budget APP, update staging, rollback, recovery, configuration, metadata, integrity data, and future offline Target images. Define concurrent-copy and failure-recovery semantics, then select a commonly available 3.3 V SPI NOR. Part, capacity, and package are TBD.

| Data class | Planned use | Freeze boundary |
|---|---|---|
| Running Probe APP | Runs from CH585M internal CodeFlash | Budget against final link map and Boot layout; do not assume external NOR replaces the internal boot image |
| USB/RF/future BLE update image | Received by the unified Update Manager; external NOR stages the complete image before verification and Boot/install processing | Image format, integrity/authentication algorithm, atomic commit, and Boot interface are unfrozen; do not erase the only running image directly |
| Rollback/Recovery | Keeping a recovery image or previous version depends on Boot update mechanism and Flash capacity | External staging does not itself provide rollback; internal A/B, BackupUpgrade, and external boot are not selected |
| Configuration/pairing metadata | Select DataFlash or external NOR after persistence requirements are reviewed | Atomic writes, endurance, layout, and identity binding are not frozen |
| Update metadata | Records receive state, length, version, verification result, and recovery phase | Field/format follow the formal update protocol and Boot design; this document assumes no storage structure |
| Offline Target images | Capacity-budget input for later offline programming | Not a frozen mandatory V1 feature; select Flash only after capacity is budgeted |

USB, 2.4 GHz, and future BLE are image-transport entry points only; Flash writes, integrity checks, commit state, and recovery belong to the unified update logic. SPI1 transfer and the NOR backend are not implemented. This is a capacity/reliability design boundary, not an executable partition layout.

### 9.2 BLE and 2.4 GHz RF

BLE and private RF use the same RF subsystem; full simultaneous operation is not assumed. EVT `CH58x_gpio.h` declares `RB_RF_ANT_SW_EN` as using PB16–PB21 for antenna-switch outputs. Thus PB16/PB17 expansion, PB18/PB19 LEDs, PB20 SWO, and PB21 reserve may conflict. Failure to find an explicit remap call in public examples does not prove a prebuilt RF library will not enable it. Isolate these nets on the validation board using 0R links/solder bridges/disconnect options; do not permanently connect conflicting loads before resolution.

RF antenna, matching, clearance, ground plane, and routing must come from traceable WCH RF references and the selected antenna vendor. The reviewed EVT schematic text is insufficient to establish the complete matching/layout requirements for a DBG-C PCB. Matching-network footprints may be reserved, but do not enter unsupported values. Before PCB release, review the RF schematic/layout and establish a VNA or board-tuning plan.

### 9.3 Key and LEDs

PB8/PB9/PB18/PB19 GPIO host-model checks do not establish LED current, polarity, mux conflicts, or key default state. Make validation-board loads current-limited and disconnectable; isolate PB18/PB19 while RF conflict remains open. Values, polarity, and key timing are TBD by part selection and firmware specification.

## 10. Protection, Test Points, and Configuration Options

Analyze ESD/EOS, misconnection, short circuit, reverse polarity, hot plug, and reverse current separately for USB, Target signals, external connectors, and power rails. Protection level/part are unfrozen. No unreviewed back-feed path may connect PC VBUS, Target Power, Target VTref, or MCU supplies.

Validation-board test points shall cover at least: VBUS, system/MCU rails, GND, a measurement method for X32MI/X32MO, optional 32 kHz, RST, BOOT, TIO, TCK, USBHS/USBFS D+/D−, SWCLK, SWDIO, TDI, TDO, Target nRESET, UART TX/RX, SWO, VTref front-end input and PA4 node, TARGET_PWR_EN, Target Power output, SPI1 SCK/MOSI/MISO/CS, RF measurement nodes, and ThreadX tick/runtime observation GPIO if required by the test plan. RF probing must not compromise impedance or antenna behavior and must follow an official method.

Use series 0R links, solder bridges, headers, or DNI parts for unfrozen electrical options so SWDIO direction, Target levels, reset output, Target power, PB16–PB21 mux, and Type-C experiments can change. Values/packages require design-for-test and safety review. Cross-check the test-point list with TEST-001 board cases.

## 11. PCB Layout Constraints

Before PCB layout, obtain and follow WCH CH585M package, power, and RF references. Review USBHS/USBFS differential paths and return, crystal layout, DC-DC current loops, RF antenna clearance and ground, Target interface protection/return, power-domain isolation, and coupling among USB, RF, clocks, and switching supplies. Do not invent impedance, spacing, clearance, stack-up, or antenna dimensions when evidence is missing. The validation board shall retain measurement and rework capability.

## 12. Validation Board and Product Board Gates

### Per-signal default-state baseline

Before drawing individual electrical circuits, use these reviewable safe defaults: active Target power output off; every Target digital level-adaptation path isolated by default; invalid/absent VTref must hardware-inhibit Probe-to-Target outputs; enable paths only after VTref is within a frozen valid range and direction is configured; `TARGET_nRESET` remains released unless a valid reset is requested and VTref is valid. Protect the VTref ADC input according to its limits; it is not the digital translator supply. Verify resistor values, logic polarity, output-enable defaults, and isolation against selected part data sheets before schematic freeze.

| Signal group | Logical direction | Safe state at power-up/reset/pre-init | Enable condition |
|---|---|---|---|
| TARGET_SWCLK_TCK / TARGET_TDI | Probe to Target | Isolated/high impedance | Hardware confirms VTref valid, Target operation started, and path enabled |
| TARGET_SWDIO_TMS | Bidirectional in SWD; Probe to Target in JTAG | Isolated/high impedance | Hardware confirms VTref valid and direction configured; SWD direction synchronized with bit engine |
| TARGET_TDO / TARGET_UART_RX / TARGET_SWO | Target to Probe | Probe input isolated and does not load Target signal | VTref valid and receive path configured |
| TARGET_UART_TX | Probe to Target | Isolated/high impedance | Hardware confirms VTref valid and CDC/UART started |
| TARGET_nRESET | Probe-controlled, Target domain | Released; do not assert | VTref valid and valid reset request; pull low then release in Target domain |
| TARGET_VTREF_ADC | Analog Sense | Current-limited/clamped within ADC-safe range | ADC initialized before sampling |
| TARGET_PWR_EN | Internal control | Off by default | Explicit power request and protection valid |

### Before schematic entry

- Freeze signal names/directions/MCU resources/power domains and safe defaults; import MCU-001 pin/net baseline; keep PB23/RST separate from PB22/BOOT.
- Define validation-board scope, power sources, and connection prohibitions; mark unresolved electrical values/parts TBD/DNI.
- Establish CH585M core, USBHS, USBFS recovery, direct Target access, and test-point sheets.

### Evidence that may be completed during schematic work

- USB descriptors/endpoints; USBHS/USBFS concurrency; custom Type-C Target mapping; VTref accuracy; translator selection; Target power; RF mux/clocks; SPI NOR part/capacity.
- Draw these as modular, isolatable, replaceable blocks; do not report them frozen.

### Required before PCB layout

- Cross-check each WCH power, decoupling, crystal, USB, and RF reference and cite its source.
- Review Target voltage front end and powered-off/back-feed cases; Target power tree; recovery accessibility.
- Decide PB16–PB21 RF mux strategy; source antenna/matching network; set USB/crystal/DC-DC/RF layout constraints.
- If Type-C carries custom Target signals, first establish specification permission, cable-conductor evidence, orientation handling, and tests. Otherwise, do not route the custom interface.

### Required before fabrication

- Schematic ERC/review, footprint check, power/protection calculations, test-point list, DNI/0R states, recovery-entry exercise plan, and open-risk review.
- Document safe experimental connection boundaries for every unverified block; never connect an unknown voltage directly to the chip or Target.

### Required before V1 product hardware freeze

- Pass and record TEST-001 board cases for ThreadX startup/tick/interrupt/context/sleep-wake; USB, Target interface, SWD/JTAG/UART, VTref/power, RF, and recovery acceptance.
- Review electrical specifications, cable/connector compatibility, BOM, PCB, EMC/ESD, and fault recovery for all required product interfaces.
- Close product-freeze blockers in OPEN-001 or disposition them through a formal change decision; document evidence and treatment for high RISK-001 items.

## 13. Items Requiring Board Tests or Additional Official Evidence

| Item | Required evidence |
|---|---|
| Final CH585M supply/crystal/decoupling wiring | Datasheet page citation, WCH circuit review, selected part parameters, rail measurements |
| BOOT/ISP/USBFS/UART recovery | WCH formal entry instructions, validation-board timing and recovery exercise |
| USBHS/USBFS combination and USB-C Device circuit | Controller/resource and official-circuit review, enumeration/recovery tests |
| Type-C custom Target contacts and cable | USB-IF applicable-clause review, specific cable BOM/construction, both-orientation continuity matrix, signal tests |
| 1.8 V/3.3 V/unpowered Target compatibility | CH585 electrical limits, data for not-yet-selected level translators, power-sequence/back-feed/waveform measurements |
| VTref range and accuracy | ADC/front-end error budget, calibration method, calibrated-source measurements |
| Target Power path | Power budget, current-limit/short/reverse isolation review and load-fault tests |
| RF mux, antenna, and clocks | Actual SDK/library configuration, official matching/layout references, RF measurements |
| External SPI NOR | Final image budget, recovery/rollback semantics, part procurement and driver validation |
| ESD/EMC/hot plug | Frozen test levels, sample results, and fault analysis |

## 14. References

| Source | Revision/date | Use and conclusion |
|---|---|---|
| WCH, CH585/CH584 Datasheet | V1.6, repository copy | Chip pin, electrical, power/clock, and peripheral basis; check errata before use |
| WCH CH585EVT archive | Index 2026.08; `EVT/PUB/CH585SCH.pdf` | Official example circuit/software reference, not a frozen DBG-C circuit |
| Texas Instruments, SN74AXC1T45 Datasheet | Rev. E, 2023-12 | Six-pin package, DIR, dual-supply range, and powered-off isolation boundaries; [vendor datasheet](https://www.ti.com/lit/ds/symlink/sn74axc1t45.pdf) |
| Texas Instruments, SN74AXC4T245 Datasheet | Rev. B, 2024-04 | Dual DIR/OE controls, supply range, and high-impedance conditions; [vendor datasheet](https://www.ti.com/lit/ds/symlink/sn74axc4t245.pdf) |
| Texas Instruments, TPS22950 Datasheet | Rev. B, 2023-02 | Target load switch and current-limit resistor example; typical values are not guaranteed limits; [vendor datasheet](https://www.ti.com/lit/ds/symlink/tps22950.pdf) |
| USB-IF, USB Type-C Cable and Connector Specification | Release 2.5, 2026-04-08 | Type-C cable/connector specification; applicability clauses must be reviewed before interface freeze |

External source: [USB-IF Type-C Specification Release 2.5](https://www.usb.org/document-library/usb-type-cr-cable-and-connector-specification-release-25), checked 2026-09-30. This document does not reproduce specification tables; conductor details remain subject to verification against a specific assembly.

## 15. Validation-Board Front-End Review Inputs and Blocking Items, 2026-09-30

The parts and nets below are specific review inputs, not an approved BOM. Vendor data sheets establish device boundaries but do not replace system-topology review, package/pin verification, timing budgets, power-fault analysis, or board measurements.

| Function | Submitted evaluation part | Vendor-documented boundary | Current disposition |
|---|---|---|---|
| Bidirectional SWDIO translation | SN74AXC1T45DBVR | VCCA/VCCB 0.65–3.6 V; DIR control; six-pin package has no separate OE; ports become high-Z below 100 mV supply and support Ioff | Not approved. With VTref directly on VCCB, below-100-mV isolation does not cover 0.1–0.65 V or above-3.6-V faults. Hardware gating must also synchronize with SWD direction changes |
| Fixed-direction signals | SN74AXC4T245PWR | Dual supplies 0.65–3.6 V; two DIR/OE groups; controls referenced to VCCA; high OE gives high-Z; Ioff supported | Verify channel grouping and package pins against Rev. B; shared OE does not protect against VCCB overvoltage |
| UART/JTAG mux | TMUX1574PWR | Four-channel 2:1 analog switch | Supply, control thresholds, power-off state, and placement relative to translators require full data-sheet review; connection diagram not approved |
| Target nRESET | TXU0101DBVR | Single-channel fixed-direction translator with OE | Proposed A=GND, B=Target reset, OE-controlled pull-low/high-Z topology requires exact package-pin, OE-polarity, VCCB, and Target pull-up review; not approved |
| VTref ADC front end | 10 kΩ/10 kΩ divider, 10 nF | Ideal calculation: 1.8 V→0.9 V, 3.3 V→1.65 V, 5 V→2.5 V | PA4 ADC range, error, acquisition time, divider loading, fault current/clamp, and calibration remain open; calculations are not measurements |
| Target Power | TPS22950CDDCR, RILIM 2.21 kΩ, 1 µF input/output | TI associates 2.21 kΩ with a typical current-limit setting near 500 mA | 3.3 V budget, guaranteed limits, fault/short/reverse-current behavior, output capacitance, and load tests incomplete; not frozen |
| Target ESD | TPD4E05U06DQAR ×2 | ESD protection device | Not a 5 V DC overvoltage clamp; channel assignment, return path, and Target Power/VTref fault capability unapproved |
| PC USB ESD | TPD2EUSB30A | USB data-line protection device | Connector, package, routing, and USBHS electrical design require separate review |

Submitted connector evaluation parts are Korean Hroparts `TYPE-C-31-M-05` (distributor number C845581, 24 contacts) for Target and `TYPE-C-31-M-14` (distributor number C223907, 16 contacts) for PC. Neither may be frozen in schematic/PCB libraries until a controlled manufacturer data sheet is obtained and the symbol, package, pad numbering, shield contacts, and mechanical dimensions are checked. Distributor identifiers, stock, and price are not durable controlled design evidence.

Submitted control-net default biases are review inputs: 100 kΩ pull-down on PA5 `TARGET_SWDIO_DIR`; 100 kΩ pull-up to 3.3 V on PA6 `TARGET_IF_OE_N`; 100 kΩ pull-down on PA7 `TARGET_MODE_SEL`; 100 kΩ pull-down on PB5 `TARGET_RESET_ASSERT`; and 100 kΩ pull-down on PB6 `TARGET_PWR_EN`. The pull-up supply/value for PA12 `TARGET_PWR_FLT_N` is undefined. Verify these values and logic defaults against MCU reset/high-impedance behavior, translator/load-switch control thresholds, leakage, startup sequencing, and VTref gating before BOM freeze. Do not copy them directly into a frozen BOM. AXC1T45 has no OE, so `TARGET_IF_OE_N` alone cannot disable the entire interface.

**PB5 reset-polarity conflict:** The current PoC `dbgc_ch585_target_reset_gpio_set_nreset()` drives PB5 low for CMSIS-DAP reset-bit zero and changes it to input for release. The submitted TXU0101 proposal instead uses PB5 high to pull the Target reset low and PB5 low to make the output high-impedance. These polarities conflict. `TARGET_RESET_ASSERT` in MCU-001 is the proposed hardware-control semantic; it does not mean current firmware already uses that polarity. Before integrating the proposed circuit, revise the PB5 HAL mapping and its host cases. This turn did not modify or run software tests, and board behavior remains unverified.

**Blocking item: the VTref circuit cannot be frozen as submitted.** The proposal directly connects Target VTref to VCCB of SN74AXC1T45/SN74AXC4T245 while treating 5 V as a fault to detect. Both AXC devices have a 3.6 V supply maximum; the 10 kΩ/10 kΩ divider only affects the ADC input and does not limit VCCB. Firmware disabling outputs after reading 2.5 V cannot prevent overvoltage from reaching VCCB first; AXC1T45 also has no OE pin. Design and substantiate VTref over/undervoltage gating, Target-side supply isolation, and a safe SWDIO path from vendor specifications before freezing. Do not guess thresholds, delays, or fault current. Also verify I/O behavior for VCCB between 0.1 V and 0.65 V; the below-100-mV high-Z statement cannot be extended to the full invalid-voltage interval.

**Target contact/power risk:** The submitted map joins four VBUS contacts as `DBG_TARGET_PWR` and sources 3.3 V, using Type-C-defined VBUS contacts for custom Target power. Neither `NOT USB` markings nor current limiting establish acceptability. Wait for the USB-IF applicability decision in IF-001; otherwise use a dedicated connector/cable. Verify every power and signal fault path when misconnected to a standard Host/Charger; ESD parts are not reverse-current blockers.

**Series-damping input:** The proposal places 33 Ω in each of six digital signals and reserves 0/22/33/47 Ω options for the validation board. This is an SI tuning input. Driver impedance, cable, Target input, and maximum-rate evidence are absent; configurable pads may be reserved, but the assembly value follows waveform/timing review.

**PC connector input:** TYPE-C-31-M-14, C223907, its 16 contacts, and USBHS/CC/VBUS/GND pin relationships remain subject to the controlled vendor drawing. Do not freeze symbol/footprint from a distributor page alone. The 90 Ω D+/D− target, TPD2EUSB30A, and omission of a common-mode choke require review against USB rules, CH585 USBHS evidence, PCB stack-up, and measurement plan.

Until overvoltage gating, Target power, Type-C permission/map, pin-by-pin component review, and cable matrix are closed, validation-board work may continue on block boundaries and independent test access; do not freeze the Target-interface schematic, BOM, or PCB routing. Target test access must still pass through level adaptation and must not provide a direct CH585M GPIO bypass.
