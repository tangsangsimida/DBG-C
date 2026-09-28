# DBG-C System Architecture

**Document ID:** DBG-C-SYS-001　**Version:** V0.1　**Status:** For review

## 1. System Boundary

```text
Wired:   PC ─USB─ DBG-C Probe ─SWD Engine─ DBG-C Interface ─ Target MCU
Wireless: PC ─USB/CMSIS-DAP v2─ Probe A ─ DBG-C RF Protocol/2.4G ─ Probe B ─ SWD Engine ─ Target
Manage:  PC Bluetooth ─ BLE ─ DBG-C Application Protocol ─ Device Management / OTA / Configuration
```

BLE is not described as a direct replacement for native USB CMSIS-DAP. BLE debug in the future would require a separately defined PC Bridge/Proxy.

## 2. Logical Layers

```text
Application / Role & Pair Manager / OTA
    USB / BLE / RF Transport
    Command & DAP Layer (CMSIS-DAP v2 endpoint on USB host-facing role)
    Service (UART Bridge, Reset, Update, Device Management)
    Target Manager
    SWD Engine
    HAL
    BSP / CH585M SDK
```

Transports feed shared command and target-control paths. USB, BLE, and RF shall not have separate SWD Engine and Target Manager implementations. HAL/BSP isolate CH585M registers and SDK dependencies. DAP command semantics over RF, batching, and replay safety must be frozen in the RF specification.

The PC debug protocol target is **CMSIS-DAP v2**. DAPLink is an optional open-source firmware system/implementation source, not a “DAPLink v2” protocol version. V1 may reuse portable protocol and algorithm layers from CMSIS-DAP/DAPLink, while CH585M USB, GPIO, clocks, and wireless functions must be adapted to the WCH SDK/HAL/BSP. Porting the full DAPLink firmware requires separate architecture, license, and toolchain review.

## 3. Device Roles and States

Role set: Standalone, Host, Target. Both units have identical hardware/BOM/MCU. Role-selection source, transition conditions, USB insertion behavior, conflict arbitration, and persistence are not frozen.

Draft state set: `Standalone`; pairing-management; `Host`/`Target` role establishment; wireless connected; operating (Debug/UART/Update); link recovery; error/unpair. Transition guards, timeouts, and events remain for DBG-C-RF-001 and DBG-C-BLE-001.

## 4. Data Flows

- **USB DAP:** PC DAP request → USB Transport → DAP layer. In Host role, it may execute locally or be proxied over RF to the Target Probe; the target response returns on the reverse path.
- **UART:** Target UART service ↔ USB CDC (standalone wired/Host side). In wireless mode, UART data uses a separate logical RF channel with QoS scheduling.
- **BLE management:** BLE Transport → application management commands; it must not bypass identity, authorization, or OTA state machines.
- **BLE target download (planned):** DBG-C Tool initiates this over BLE Application Protocol. It carries defined download/management commands and is not transparent BLE CMSIS-DAP. Target types, algorithms/image format, resume behavior, and PC compatibility remain for DBG-C-BLE-001.
- **USB allocation:** V1 uses USBFS; USBHS is not enabled. Check USBFS endpoint/buffer requirements and WCH SDK Device support for CMSIS-DAP v2 Bulk + CDC against SDK examples.
- **OTA:** Image reception, validation, installation, boot confirmation, and failure recovery depend on SDK/Boot evidence and remain unverified.

## 5. Hardware Layers

CH585M Probe controller, USB Device connection, 2.4 GHz antenna/RF, power and clocks, buttons/status indicators, and DBG-C Interface. VTREF/Target Power/BOOT/Target Detect are future extensions, not mandatory V1 functions. Interface electrical design/pinout, Type-C CC/VBUS, protection, and power path are not frozen; do not start PCB design from this draft.

## 6. Resource-Conflict Topics

USB FS/HS choice, RF/BLE coexistence, DMA, endpoint buffers, clocks, RAM, timers, and debug/programming port resources require official SDK/reference-manual review and prototype measurements. The repository currently has only a datasheet, so simultaneous-operation constraints are not established.

## 7. Related Specifications

Requirements: DBG-C-PRD-001; physical interface: DBG-C-IF-001; MCU resources: DBG-C-MCU-001; RF/BLE/USB details belong in their respective specifications. This V0.1 does not freeze PHY, USB descriptors, pins, or role decisions.
