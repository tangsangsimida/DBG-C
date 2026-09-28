# DBG-C Private 2.4G Protocol Specification

**Document ID:** DBG-C-RF-001　**Version:** V0.1　**Status:** Design framework; do not implement unfrozen fields

## 1. Goal and Boundary

The protocol carries Debug, UART, control, and future update data between two identical DBG-C Probes. Directly forwarding USB Bulk packets is not a protocol definition. The PC still connects to the Host Probe using USB CMSIS-DAP v2; BLE management is a separate protocol.

## 2. Physical Layer and Pairing

The IC manual does not specify private-RF PHY, channel, hopping, address, power, rate, or WCH RF SDK API. Read the SDK/official protocol materials and define each before implementation. The datasheet confirms integrated BLE/2.4 GHz RF and mentions “2.4G mode”; this does not establish that an arbitrary private PHY is available.

Define Device ID, Peer ID, and Pair ID semantics and uniqueness. Width/encoding are not frozen in V0.1. Define user authorization, pairing window, authentication, unpair, persistence, re-pair, replay protection, and key management.

## 3. Logical Frame Fields (TBD)

Frames need to express protocol version, role/session, command or event type, sequence number, fragmentation, length, data, and integrity/authentication information. Field names, widths, byte order, alignment, maximum frame length, CRC parameters, authentication tag, and RF address format are all TBD and must be frozen in a V0.2+ field table. Do not guess them for implementation.

## 4. Command Classes

Command/Response/Event coverage includes pairing/unpairing/role negotiation, session setup, link status, keep-alive, DAP debug data, UART data, errors, and update control. Whether target-MCU firmware OTA uses BLE/RF/USB and is in V1 requires a product decision. DBG-C self-update is in scope, but its transport remains open.

## 5. Reliability and Real-Time Behavior

Define ACK, timeout, retry limits/backoff, duplicate-detection window, fragmentation/reassembly, reordering, link-loss detection, buffer limits, flow control, and recovery. Repeated DAP requests may have side effects; the receiver must deduplicate by session/sequence. Do not automatically replay unsafe commands before the rule is frozen.

QoS shall distinguish at least control, debug request/response, UART, and update data. Scheduling, reserved bandwidth, and fairness require measurement of round-trip latency, sustained UART load, and packet loss. Measure CMSIS-DAP latency sensitivity to RF retries before committing to a target.

## 6. Errors and Compatibility

Define protocol version negotiation, unknown command, invalid length, CRC/authentication failure, timeout, exhausted retries, pairing mismatch, target power loss, and recovery events. Major/minor compatibility policy remains open. After recovery, do not report stale target state from the previous session as current.

## 7. Work Before Freeze

Obtain the CH585M SDK and RF examples; verify available PHY/coexistence constraints; create a capture format and test tool; measure real CMSIS-DAP host round-trip latency with an RF prototype. Then freeze the field table, transitions, timeout values, frame vectors, and compatibility rules.
