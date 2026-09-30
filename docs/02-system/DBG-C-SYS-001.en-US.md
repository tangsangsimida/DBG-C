# DBG-C System Architecture

**Document ID:** DBG-C-SYS-001　**Version:** V0.5　**Status:** Architecture draft; V1 externally powered Target voltage domains and VTref-following level-adaptation architecture frozen; product integration and board verification not run

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
    USBHS / BLE / RF Transport
    DAPLink firmware components / CMSIS-DAP v2 DAP Command Core
    Service (UART Bridge, Reset, Update, Device Management)
    Target Manager
    SWD Engine
    HAL
    BSP / CH585M SDK
```

Transports feed shared command and target-control paths. USB, BLE, and RF shall not have separate SWD Engine and Target Manager implementations. HAL/BSP isolate CH585M registers and SDK dependencies. DAP command semantics over RF, batching, and replay safety must be frozen in the RF specification.

Firmware follows the DAPLink firmware system; the PC debug target is **CMSIS-DAP v2 over USB Bulk**; CMSIS-DAP v1 HID is not the primary debug interface. DAPLink is a firmware system/open-source implementation source, not a “DAPLink v2” protocol. The DAP Command Core is decoupled from USBHS, BLE, and RF Transports; wireless payloads enter the same core, while RF handles transport reliability and fragmentation. Upstream CMSIS-DAP currently has separate host-model and target-object checks but is not connected to a product USBHS receive path. CH585M USB, GPIO, clocks, radio, and ThreadX adaptation must be isolated through WCH SDK/HAL/BSP. The exact scope, licensing, and build integration for a complete DAPLink port are not settled.

## 3. Device Roles and States

Role set: Standalone, Host, Target. Both units have identical hardware/BOM/MCU. Role-selection source, transition conditions, USB insertion behavior, conflict arbitration, and persistence are not frozen.

Draft state set: `Standalone`; pairing-management; `Host`/`Target` role establishment; wireless connected; operating (Debug/UART/Update); link recovery; error/unpair. Transition guards, timeouts, and events remain for DBG-C-RF-001 and DBG-C-BLE-001.

## 4. Data Flows

- **USB DAP:** PC DAP request → USB Transport → DAP layer. In Host role, it may execute locally or be proxied over RF to the Target Probe; the target response returns on the reverse path.
- **UART:** Target UART service ↔ USB CDC (standalone wired/Host side). In wireless mode, UART data uses a separate logical RF channel with QoS scheduling.
- **BLE management:** BLE Transport → application management commands; it must not bypass identity, authorization, or OTA state machines.
- **BLE target download (planned):** DBG-C Tool initiates this over BLE Application Protocol. It carries defined download/management commands and is not transparent BLE CMSIS-DAP. Target types, algorithms/image format, resume behavior, and PC compatibility remain for DBG-C-BLE-001.
- **USB allocation:** The PC connection uses USBHS PB12/PB13, targeting CMSIS-DAP v2 Bulk on USBHS Device and UART CDC ACM; whether management is a separate interface is for the USB specification. USBFS PB10/PB11 is reserved for recovery/production evaluation. Do not assume simultaneous USBHS/USBFS operation. EVT has USBHS CDC, dual-HID composite, and IAP examples, but no evidence for a DAP+CDC combination; these examples do not prove DBG-C integration, ThreadX synchronization, endpoint allocation, or host compatibility.
- **OTA:** Image reception, validation, installation, boot confirmation, and failure recovery depend on SDK/Boot evidence and remain unverified.

## 5. Hardware Layers

CH585M Probe controller, USBHS Device connected to the PC Host, USBFS recovery resources, 2.4 GHz antenna/RF, power and clocks, buttons/status indicators, DBG-C Interface, Target SWD/JTAG/SWO/UART, VTref ADC, target power control, and external SPI NOR. V1 supports externally powered Targets with nominal 1.8 V and 3.3 V I/O domains. Every Target digital signal uses VTref-following level adaptation, with hardware output inhibit for invalid VTref and isolation when either side is unpowered. Active power sourcing from DBG-C is a separate unresolved product decision. Pin baseline is in MCU-001; electrical implementation is in HW-001. This is not an electrical schematic. Translator parts and numeric limits, active Target power, VBUS isolation, Type-C CC/VBUS, protection, BOOT conditions, RF antenna-switch muxing, and external Flash selection remain unfrozen; do not freeze product hardware from this draft.

## 6. Resource-Conflict Topics

USB FS/HS choice, RF/BLE coexistence, DMA, endpoint buffers, clocks, RAM, timers, and debug/programming port resources require official SDK, reference-manual, and prototype review. The repository contains the CH585EVT archive and only the EVT startup/linker/header copies used by PoC-1; the archive index is dated 2026.08 but does not give a separate WCH SDK semantic version. The FreeRTOS example is not a concurrent-resource specification and does not establish DBG-C product coexistence. A separate reference manual and prototype measurements are still missing.

## 7. Related Specifications

Requirements: DBG-C-PRD-001; physical interface: DBG-C-IF-001; MCU resources: DBG-C-MCU-001; RF/BLE/USB details belong in their respective specifications. This architecture does not freeze PHY, USB descriptors, endpoints, roles, OTA partitions, or electrical design.
