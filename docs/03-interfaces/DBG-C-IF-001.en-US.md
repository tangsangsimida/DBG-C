# DBG-C Interface Specification

**Document ID:** DBG-C-IF-001　**Version:** V0.1　**Status:** Concept draft; do not freeze pins or release PCB from this document

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
| UART TX/RX | Target UART | Full option; direction and voltage: TBD |
| nRESET | Target hardware reset | Full option; open-drain/push-pull and default: TBD |
| SWO/VTREF/JTAG/Target Power/BOOT/Target Detect | Future functions | Outside mandatory V1; allocation belongs to a later revision |

No connector pin numbers are defined here. Actual Basic/Full mappings remain for review and freeze.

## 3. Basic and Full Concepts

- **DBG-C Basic:** Intended to carry the minimum SWD signals over a specified passive USB 2.0 Type-C data cable. The actual conductor mapping at both ends must be validated for custom-signal transmission. Standard USB 2.0 cable definitions do not imply arbitrary custom signals are available. Compatible cable types/models remain TBD.
- **DBG-C Full:** Separately evaluate a clearly specified cable assembly carrying signals such as UART and nRESET. Evaluate Passive Full-Featured Type-C Cable, Active Cable, SBU, SuperSpeed differential pairs, orientation detection, and MUX independently. Active cables may retime/convert signals and cannot be assumed to transparently pass custom low-speed signals.

Do not use vague wording such as “advanced Type-C cable.” If the selected cable is not an ordinary passive USB 2.0 cable, state its actual conductors and compatibility/certification limits.

## 4. Electrical and Mechanical Requirements (TBD)

Pin definition, direction, voltage range, IO type, default state, pulls, overvoltage/reverse/short-circuit protection, ESD level, hot plug, VBUS/CC, insertion sequence, orientation mapping, misconnection protection, impedance/crosstalk, and revision compatibility require schematic review and measurement. Do not infer interface compatibility from connector appearance or CH585M GPIO features.

## 5. Freeze Gate

Before freeze, obtain the applicable USB-IF Type-C Cable and Connector Specification, target electrical requirements, cable conductor-mapping evidence, protection review, and test records for both orientations, hot plug, and abnormal/misconnected cases. V0.1 has not met this gate.
