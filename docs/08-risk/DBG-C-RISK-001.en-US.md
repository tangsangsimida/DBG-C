# DBG-C Risk Register

**Document ID:** DBG-C-RISK-001　**Version:** V0.1　**Status:** Initial risk list

Ratings use qualitative High/Medium/Low and await an agreed project scoring method.

| ID | Risk / trigger | Consequence | Rating | Mitigation | Verification | Status |
|---|---|---|---|---|---|---|
| R01 | Custom signals connect to Type-C contacts not wired by a standard cable | No operation or damage from misconnection | High | Do not publish an unfrozen pinout; specify and validate cable conductor mapping | Cable mapping, both orientations, misconnection tests | Open |
| R02 | VBUS/target power path lacks isolation or direction control | Back-feed, overcurrent, damage | High | Define power boundary, protection, and detection | Direction, short, hot-plug bench tests | Open |
| R03 | Target I/O voltage exceeds Probe capability | IO damage or communication failure | High | Freeze voltage range and level-compatibility design | Voltage extremes and fault injection | Open |
| R04 | Active Cable retimer/MUX does not pass custom signals | Compatibility failure | High | Specify cable assembly and distinguish Passive/Active | Multiple brands, orientations, lengths | Open |
| R05 | Type-C orientation causes incorrect signal mapping | Debug failure or damage | High | Design orientation detection or symmetric mapping and verify | Both plug orientations | Open |
| R06 | USB endpoint/descriptors conflict with PC stacks | Enumeration/feature failure | Medium | Derive descriptors from approved USB design and test real implementations | Windows/Linux/macOS enumeration | Open |
| R07 | USBFS/HS, DMA, RAM resources conflict with SDK limits | Data loss or failed concurrency | Medium | SDK review, resource budget, prototype stress | Dual-transport stress and memory-watermark logs | Open |
| R08 | RF latency/loss exceeds debugger tolerance | Debug timeout or poor experience | High | QoS, flow control, bounded retries, real-host measurements | Latency distribution, interference, packet-loss tests | Open |
| R09 | Retry duplicates a side-effecting DAP command | Target state corruption | High | Session sequence deduplication; define idempotent/non-replayable commands | Lost-ACK/duplicate-frame injection | Open |
| R10 | BLE/private 2.4 GHz coexistence scheduling is unclear | Loss, disconnect, reduced throughput | High | Verify SDK radio coexistence features/limits | RF stress during BLE activity | Open |
| R11 | Pair/role conflict or device misidentification | Wrong target connection | High | Define authorization, identity display, role arbitration, unpair | Multi-device pairing fault tests | Open |
| R12 | OTA power loss, corrupt image, or unknown rollback | Probe bricking | High | Verify Boot/OTA atomicity, signature, and recovery mode | Staged power-cut/corrupt-image tests | Open |
| R13 | Incompatible firmware versions | Wireless session unavailable | Medium | Protocol negotiation and compatibility matrix | Cross-version combinations | Open |
| R14 | RF antenna/matching/layout deviates from reference design | Link performance failure | High | Obtain official RF/layout guidance and review | Matching, sensitivity, radiated tests | Open |
| R15 | Insufficient ESD/EMC/power integrity | Reset, damage, or regulatory failure | High | Protection and layout review during design | ESD/EMC/power-transient tests | Open |
| R16 | SWD timing/target voltage tolerance is unverified | Instability or damage on some targets | High | Define target compatibility and SWD frequency range | Oscilloscope measurements and multi-target regression | Open |
