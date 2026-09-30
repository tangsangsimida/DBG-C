# DBG-C Interface Specification

**Document ID:** DBG-C-IF-001　**Version:** V0.4　**Status:** Concept draft; Target voltage-domain requirement frozen; contact mapping not frozen

## 1. Definition

DBG-C Interface is the physical interface between a DBG-C Probe and a target. A Type-C-shaped connector does not automatically imply USB data, compliant Type-C electrical behavior, or that arbitrary custom signals pass through every USB-C cable. Any custom signal assignment requires specification review and electrical/cable validation.

## 2. Conceptual Signal Requirements (Not Pin Assignments)

| Logical signal | Purpose | Direction/electrical/default |
|---|---|---|
| SWDIO | Bidirectional SWD data | Target voltage, turnaround, and protection: TBD |
| SWCLK | SWD clock | Probe to Target; level/timing: TBD |
| GND | Reference ground | Connection and return path: TBD |
| VBUS | Connection detection/power-related | Purpose, connection, isolation, and back-feed protection: TBD |
| CC1/CC2 | Type-C accessory/orientation detection | Study per USB-IF Type-C specification; do not reuse as arbitrary target signals |
| UART TX/RX | V1 Target UART | Logical direction is in Section 4.3; connector contacts/electrical implementation TBD |
| nRESET | V1 Target hardware reset | In Target voltage domain; safe default released; output stage/contact TBD |
| JTAG/SWO/VTref/Target Power | V1 Target-related capabilities | Logical direction is in Section 4.3; Target Power is internal control, not a Target data line; contacts/circuit TBD |
| BOOT/Target Detect | Later extension | Not part of the current frozen V1 Target signal mapping |

No connector pin numbers are defined here. Actual Basic/Full contact mappings remain for review and freeze; the V1 logical capability requirements above do not define connector contacts.

## 3. Basic and Full Concepts

- **DBG-C Basic:** Intended to carry the minimum SWD signals over a specified passive USB 2.0 Type-C data cable. The actual conductor mapping at both ends must be validated for custom-signal transmission. Standard USB 2.0 cable definitions do not imply arbitrary custom signals are available. Compatible cable types/models remain TBD.
- **DBG-C Full:** Separately evaluate a clearly specified cable assembly carrying signals such as UART and nRESET. Evaluate Passive Full-Featured Type-C Cable, Active Cable, SBU, SuperSpeed differential pairs, orientation detection, and MUX independently. Active cables may retime/convert signals and cannot be assumed to transparently pass custom low-speed signals.

Do not use vague wording such as “advanced Type-C cable.” If the selected cable is not an ordinary passive USB 2.0 cable, state its actual conductors and compatibility/certification limits.

## 4. Electrical and Mechanical Requirements

This section defines DBG-C Interface hardware constraints. The Pin Mapping section freezes actual contacts; until then, electrical safety, state control, protection, mechanical, and verification requirements remain applicable.

### 4.1 Interface Classes

| Interface | Definition | Constraint |
|---|---|---|
| PC USB Interface | Standard USB Device connection between Probe and PC | Follow formal USB Type-C and USB 2.0 specifications. Do not repurpose CC, VBUS, or USB data contacts for private DBG-C signals |
| DBG-C Target Interface | Debug connection between Probe and Target | Custom physical interface. A Type-C-shaped connector does not make it a standard USB Type-C interface. Define contacts, dedicated cable, electrical rules, and compatibility independently |

The Target Interface must tolerate misconnection to an ordinary PC Host, Charger, or other Type-C device without permanent damage. CC1/CC2 shall not carry custom debug data. VBUS may only provide an expressly defined power/detection function, never GPIO or debug data.

### 4.2 Signal Electrical Parameters

Define every Target signal separately; no unexplained blank may remain before formal schematic freeze.

| Parameter | Required definition |
|---|---|
| Signal Name / Connector Contact | Frozen project signal and connector contact |
| Direction / Voltage Domain | Probe to Target, Target to Probe, Bidirectional; reference domain |
| Valid Voltage / Absolute Fault Range | Normal operating and fault withstand ranges |
| Input Threshold / Output Level | Guaranteed input high/low thresholds and output levels |
| IO Type / Power-off / Default State | IO type and behavior when either side is off, at power-up/reset/Bootloader/pre-init |
| Pull / Drive / Series Damping | Pulls and reference rail, drive, series damping and rationale |
| Protection / Hot-plug | ESD, overvoltage, reverse current, short, insertion/removal behavior |
| Verification | Test method and pass criteria |

### 4.3 Target Signal Directions

| Signal | Direction |
|---|---|
| TARGET_SWCLK_TCK | Probe to Target |
| TARGET_SWDIO_TMS | Bidirectional |
| TARGET_TDI | Probe to Target |
| TARGET_TDO | Target to Probe |
| TARGET_UART_TX | Probe to Target |
| TARGET_UART_RX | Target to Probe |
| TARGET_nRESET | Probe controlled, Target voltage domain |
| TARGET_SWO | Target to Probe |
| TARGET_VTREF_ADC | Target to Probe, Analog Sense |
| TARGET_PWR_EN | Internal Probe control, not Target data |

DBG-C V1 shall support both 1.8 V and 3.3 V Target I/O domains. Every Target-facing digital signal shall use level adaptation associated with Target VTref and provide isolation when either Probe or Target is unpowered. Do not rely on direct electrical compatibility between CH585M 3.3 V GPIO and the Target. Freeze translator parts, direction control, propagation-delay/speed boundaries, and powered-off behavior in HW-001 from component data and measurements.

### 4.4 Default State and Hot Plug

During Probe power-up/reset, Bootloader, and before GPIO initialization, the interface shall be safe. Target Power stays off absent explicit control. Outputs shall not inject back-power current before Target supply is known. HW-001 shall specify and board-verify safe defaults for SWDIO, SWCLK, JTAG, UART, SWO, and nRESET. nRESET stays released unless reset is requested; Probe reset/update/failure/power loss shall not hold Target reset. Hot-plug sequence: disconnected → detect Target supply → establish safe IO → enable debug. On removal or VTref loss, stop driving and return to safe state. Timing is measured on the Validation Board.

### 4.5 VTref and Voltage Domains

HW-001 shall define a VTref detection range covering 1.8 V and 3.3 V, ADC protection, divider, filter, clamp, power-off isolation, calibration, and valid threshold. Do not exceed CH585 ADC limits. VTref shall be associated with level adaptation on all Target digital signals. Assess the ADC sensing path and translator reference-supply path separately for loading, startup order, and powered-off behavior.

### 4.6 Reverse Power, Shorts, and Misconnection

Verify Probe/Target OFF/OFF, ON/OFF, OFF/ON, and ON/ON states for SWDIO, SWCLK, JTAG, UART, SWO, nRESET, VTref, and Target Power. Do not rely on MCU ESD diodes as product current limiting. Test signal-to-GND/supply shorts, adjacent-contact shorts, VBUS misconnection, simultaneous external and Probe Target power, and connection to PC Host/Charger/ordinary USB Device. Define fault voltage/duration and recovery criteria; no permanent damage is allowed.

### 4.7 ESD

Externally accessible connectors require ESD protection. Use the project-approved IEC 61000-4-2 revision; discharge levels and performance criteria await the final EMC environment. Do not guess values. Check clamp, capacitance, leakage, and dynamics against signal integrity. Do not select USB or RF protectors by static withstand voltage alone.

### 4.8 VBUS and CC

PC USB VBUS/CC follow the selected Type-C Device power/data role. Target Interface VBUS, if used, has one defined purpose. Never conflate PC VBUS, Target VTref, and Target Power; define isolation and reverse blocking between domains. PC CC1/CC2 are not GPIO or Target data. Target CC use for dedicated-cable orientation detection requires a frozen cable design; do not treat CC as custom data meanwhile.

### 4.9 Reversible Orientation and Misconnection

Both plug orientations must work by naturally symmetric mapping or by detecting orientation and switching a MUX before enabling any Target output. Never probe an unknown mapping by transmitting. Test both orientations. Do not rely on silkscreen alone to prevent Target-to-USB misconnection; if electrical tolerance is not proven, use dedicated cable construction, keying, another connector, or another explicit prevention measure.

### 4.10 Signal Integrity and Crosstalk

Design PC USB D+/D− impedance, matching, return, vias, and ESD per adopted USB rules and PCB stack-up. Do not apply USB differential impedance to SWD/JTAG/UART/SWO. Derive their limits from frequency, edge rate, trace, connector, cable, Target input, and measured waveforms. Reserve adjustable series damping on the validation board. Test product waveforms at longest cable and highest debug rate. Analyze adjacent-signal coupling and return paths; determine crosstalk criteria after frequency, cable, and mapping freeze.

### 4.11 Mechanical Requirements and Version Compatibility

PC USB uses project-approved Type-C compliance requirements. Target Interface has a separate mechanical BOM and cable definition. Freeze manufacturer, part number, plug/receptacle role, cable construction/length, mating cycles, temperature, shell grounding, PCB retention, both orientations, and cable identification. V1 contact functions must not change silently; incompatible changes require a new interface version, adapter, cable identification, or misconnection protection. Old/new direct connection must not cause damage. Do not use vague BOM descriptions such as “ordinary” or “advanced” Type-C cable.

### 4.12 Validation Board Verification and Freeze Rules

Before Product Hardware Freeze, test both orientations, powered/unpowered insertion, live removal, Probe-only/Target-only/both-powered, Target short, adjacent-contact short, Target voltage limits, maximum SWD rate, UART, nRESET waveform, VTref, Target Power, ESD, ordinary USB misconnection, and dedicated-cable compatibility. Without board records, status remains unverified.

Before schematic entry, freeze signal names, directions, MCU resources, power domains, and safe defaults. Before PCB layout, additionally freeze connector part, Pin Mapping, Target voltage, level translation, protection, Target Power, USB electrical design, dedicated cable, and RF interface. Before Product Hardware Freeze, complete validation-board measurements for hot plug, orientation, misconnection, ESD, reverse power, shorts, signal integrity, and Target compatibility. Unmeasured items are not product-verified.

## 5. Freeze Gate

Before freeze, obtain the applicable USB-IF Type-C Cable and Connector Specification, Target electrical requirements, cable construction/mapping evidence, protection review, and test records for both orientations, hot plug, and abnormal/misconnected cases. The 1.8 V/3.3 V domains and VTref-associated level-adaptation requirement are frozen; component design and board verification remain incomplete. V0.4 has not met the interface freeze gate.

## 6. USB-IF Reference Review

This review checked USB-IF *USB Type-C Cable and Connector Specification* Release 2.5, dated 2026-04-08, on 2026-09-30. USB-IF states that third-party functionality on Type-C cables and connectors is limited to functionality expressly described by the specification. Therefore, DBG-C does not treat Type-C contacts as generic custom-signal conductors or an ordinary passive USB 2.0 cable as an SWD/UART/Reset cable.

| Contact/conductor class | Specification boundary | Current DBG-C decision |
|---|---|---|
| D+/D− | USB 2.0 data | Reserve for PC USB; do not map Target debug signals |
| CC1/CC2 | Type-C attach/orientation/power-role related | Implement per specification; do not reuse for Target signals |
| VBUS/GND | Bus power and return | Target power isolation strategy TBD |
| SBU1/SBU2 | Defined sideband signals | Conductor continuity in ordinary USB 2.0 cables is unconfirmed; assign no custom signal |
| SuperSpeed TX/RX | USB 3.x high-speed data lanes | Not guaranteed in passive USB 2.0 cables; Active Cable is not assumed to pass custom low-speed signals transparently |
| Shield | Shield/chassis related | Bonding decision follows EMC/ESD review |

The current Contact → Cable Conductor → Target Signal mapping table is empty and cannot be frozen. Select a specific connector and cable assembly, obtain formal cable-construction information, and complete both-orientation continuity and signal tests. USB-IF source: [Release 2.5 document page](https://www.usb.org/document-library/usb-type-cr-cable-and-connector-specification-release-25).

The validation board shall expose Target signals on separate test points/isolatable headers. Whether to retain the Basic/Full categories and any custom Type-C Target interface remain under review. Do not make it the sole Target connection before the mapping is approved.
