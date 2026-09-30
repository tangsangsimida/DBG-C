# DBG-C USB Device Specification

**Document ID:** DBG-C-USB-001　**Version:** V0.3　**Status:** USBHS architecture draft; descriptors/endpoints not frozen; no product enumeration or board validation

## 1. Purpose and Boundary

This document defines the PC-facing USB direction for DBG-C Probe. The current baseline uses CH585M USBHS Device, with CMSIS-DAP v2 Bulk as the primary debug interface; CMSIS-DAP v1 HID is not the primary channel. USBFS PB10/PB11 is reserved for recovery/production evaluation. Connector use, entry conditions, and concurrent operation with USBHS are unconfirmed.

This document does not define custom target-interface contacts on Type-C; see DBG-C-IF-001. MCU-001 assigns PB12 to U2D− and PB13 to U2D+. Pin assignment and USB descriptors are separate interface layers.

## 2. Evidence and Limits

| Source | Verified content | Limit |
|---|---|---|
| CH585/CH584 Datasheet V1.6, Table 1-1, CH585M column | Package pins for PB12/U2D− and PB13/U2D+; USBHS capability summary | Does not define DBG-C composite descriptors, endpoint assignment, or ThreadX integration |
| Project EVT: `EVT/EXAM/IAP/USBHS_IAP/src/Main.c`, `usb_desc.h` | USBHS Device/IAP example exists; its source defines its own VID/PID, endpoints, and packet sizes | Example identifiers/descriptors/sizes are WCH sample values. Do not copy them as DBG-C product values; they do not prove CMSIS-DAP + CDC |
| Project EVT: `EVT/EXAM/USB/USBHS/DEVICE/SimulateCDC/User/usb_desc.c/.h`, `ch585_usbhs_device.c` | A USBHS CDC Device example exists with FS/HS descriptors; CDC uses two interfaces, EP3 interrupt IN, and EP2 Bulk IN/OUT. Endpoint initialization directs EP2 RX DMA to `UART2_Tx_Buf`; the EP2 OUT interrupt reads the RX length, toggles DATA, sets NAK, and clears `CDC.DownloadPoint_Busy` | This is a CDC example without CMSIS-DAP. Its descriptors, endpoints, and identifiers are not frozen DBG-C values. Its ISR behavior does not define DBG-C receive queues, data ownership, or an ISR-to-ThreadX synchronization contract |
| Same example, `User/ch585_usbhs_device.h`, SHA-256 `5378baa82669a0898c3770cda070c1a65ff2b80858ab1862669b44bd16a6340e` | Declares `USBHS_Device_Endp_Init(void)`, `USBHS_Device_Init(FunctionalState)`, `USBHS_Device_SetAddress(uint32_t)`, `USBHS_IRQHandler(void)`, and `USBHS_Endp_DataUp(uint8_t, uint8_t *, uint16_t, uint8_t)`; also defines `DEF_UEP_NUM` as 16 and DMA/copy send-mode constants | `USBHS_IRQHandler` appears only in the header declaration and C source comment; the C file defines `USB2_DEVICE_IRQHandler`, and the archive vector table in `EVT/EXAM/SRC/Startup/startup_CH585.S` (SHA-256 `40a6ed31faed8915e66a465ae369152cbcf57f6d39c933cd71720b8e45509d8c`) points USB2_DEVICE to that same handler name. Do not copy this naming mismatch into the DBG-C API. Other declarations are sample interfaces, not proof of a stable generic SDK API or product endpoint budget |
| Project EVT: `EVT/EXAM/USB/USBHS/DEVICE/CompositeKM/User/usb_desc.c/.h` | A USBHS composite device example with two HID interfaces exists | It has neither CDC nor CMSIS-DAP and does not prove DAP Bulk + CDC composite operation |
| CMSIS-DAP submodule `software/third_party/cmsis-dap`, commit `12636590eec66fae2d1bba4518749426ad5a4595`, `Documentation/Doxygen/src/dap_firmware.md` and `Firmware/Config/DAP_config.h` | CMSIS-DAP v2 uses USB Bulk; optional CDC ACM carries UART; the config header states 512 bytes as a typical High-Speed WinUSB Bulk packet size | This is upstream configuration guidance, not a verified CH585 USBHS driver/descriptor setting; final endpoints and buffers require controller and host-transfer validation |

Review date: 2026-09-30. Project EVT ZIP SHA-256: `cdab364ffc24d793b300f22b3aa7ddfd97306f880d3962448da47ef9da870274`.

## 3. Logical Device Functions

Normal application mode requires these logical functions:

1. CMSIS-DAP v2 commands arrive over Bulk OUT and responses return over Bulk IN. DAP Command Core does not call USB registers or driver APIs directly.
2. Target UART is exposed to the PC through CDC ACM and routed to UART Bridge Service, separate from DAP Command Core.
3. Whether an optional DBG-C Management USB interface belongs in V1 depends on frozen management-command and PC Tool requirements.
4. USB firmware update reuses the USBHS physical connection. Application and Bootloader may use different configurations/descriptors, but mode transition, identity, image protocol, and recovery behavior require the Boot/Update design.

Interface count, interface numbers, alternate settings, endpoint addresses, transfer types, and maximum packet sizes are **To verify**. Do not infer DBG-C values from WCH IAP examples, other DAPLink boards, or non-CH585 devices.

## 4. Software Data Path

```text
USBHS Device ISR / driver
        ↓
USBHS receive record queue
        ↓
USB Transport service
        ↓
CMSIS-DAP request length/capacity guard
        ↓
DAP Command Core
        ↓
SWD/JTAG backend and Target Manager
        ↓
response record queue
        ↓
USBHS Bulk IN
```

This is a layer design, not existing product USBHS code. Interrupt handling only performs the receive/status work required by the final WCH USBHS API and notifies a service context; DAP command processing must not run in the ISR. Verify ThreadX synchronization primitives, USBHS API context restrictions, endpoint-buffer ownership, and cache rules from actual SDK sources/library interfaces before implementation. The host CMSIS-DAP service and bounded dispatch have separate host checks but are not connected to this path.

In the reviewed EVT `SimulateCDC` example, EP2 RX DMA writes directly to `UART2_Tx_Buf`. The EP2 OUT branch in `USB2_DEVICE_IRQHandler()` reads `R16_U2EP2_RX_LEN`, toggles the EP2 RX DATA bit, sets the EP2 RX response to NAK, and clears `CDC.DownloadPoint_Busy`. This branch does not copy data into the DBG-C fixed-record packet queue and does not provide a reusable DBG-C ISR/ThreadX synchronization implementation. The DBG-C generic fixed-record queue explicitly requires serialized caller access, so the EVT example does not establish safe concurrent queue access by an ISR and service thread. These are static EVT source findings only, not DBG-C implementation or CH585M board validation.

The example `main()` sets the system clock, initializes debug output and UART2, calls `USBHS_Device_Init(ENABLE)`, then polls `UART2_DataRx_Deal()` and `UART2_DataTx_Deal()` in an infinite loop; it does not initialize ThreadX or a DAP Command Core. `USBHS_Device_Endp_Init()` configures EP2 RX/TX DMA and EP3 TX DMA. These endpoint buffers and descriptors are specific to the example. DBG-C implementation still requires a product descriptor/endpoint plan for CMSIS-DAP v2 plus CDC, buffer ownership, and ISR-to-thread synchronization.

CDC ACM uses separate RX/TX buffering and UART Bridge Service. Priorities, queue capacity, backpressure, and drop policy when DAP and high-rate CDC traffic arrive together must be frozen after throughput/latency testing; this draft does not invent values.

## 5. USBHS and USBFS Recovery Relationship

- USBHS: primary PC interface on PB12/PB13; target is CMSIS-DAP v2 Bulk + CDC ACM on USBHS Device.
- USBFS: PB10/PB11 reserved; there is no claim that a running application can automatically fall back to USBFS after USBHS failure.
- To verify: whether controller initialization, clocks, DMA, IRQ, RAM buffers, and port electrical design permit both controllers in one firmware; whether recovery uses reset/BOOT to launch a separate USBFS image; and whether official ISP/IAP uses this path.

## 6. Descriptors and Host OS Policy

Do not fill the following until VID/PID decisions, actual USBHS stack, and descriptor implementation exist:

- VID, PID, bcdDevice, Manufacturer, Product, and Serial policy;
- Configuration, Interface, and Endpoint counts/numbering;
- CMSIS-DAP v2 WinUSB compatible descriptors or driver-installation strategy;
- CDC ACM control/data interfaces and endpoints;
- whether a management interface exists;
- USB 2.0 High-Speed enumeration and Full-Speed fallback behavior;
- Windows, Linux/udev, and macOS compatibility; unique serial and device selection with multiple probes attached.

Validate against final firmware and real USBHS hardware: descriptor parsing, CMSIS-DAP request/response, CDC full duplex, hotplug, reset recovery, multi-device selection, and long concurrent transfers.

## 7. Update Mode

USB Update Transport only transfers fragments and reports link errors. A shared Update Manager owns image state, storage writes, integrity checks, commit, boot confirmation, and failure recovery. USB Transport does not select internal Flash partitions and the running application must not erase its only valid image. The USBHS IAP example is evidence for API/flow review, not a basis for copying sample Bootloader/App boundaries into DBG-C.

## 8. Open Items

| Item | Evidence required to close |
|---|---|
| USBHS Device driver and endpoint capability | EVT CDC header declares `USBHS_Device_Init()`, `USBHS_Device_Endp_Init()`, `USBHS_Device_SetAddress()`, `USBHS_IRQHandler()`, and `USBHS_Endp_DataUp()`; the sample EP2 RX DMA and OUT ISR operations have been reviewed | Current SDK/sample API and register-access context, DBG-C endpoint plan, DMA-buffer ownership, ThreadX synchronization, target build, and board enumeration remain to be confirmed |
| CMSIS-DAP v2 + CDC ACM composite descriptors | Actual USB stack/descriptor implementation, endpoint budget, host enumeration, and simultaneous-transfer records |
| USBHS and USBFS recovery relationship | Official init/recovery example, resource-conflict review, and verification-board recovery experiment |
| VID/PID and strings | Organization VID decision, serial-number source, and manufacturing rules |
| USB update image and Boot flow | Actual link map, image sizes, external NOR selection, and power-cut/corruption recovery tests |
| Host/IDE support matrix | Tests on Windows, Linux, macOS and target IDE/OpenOCD/pyOCD |

See OPEN-001 O02/O08/O17, PRD-001, SYS-001, and TEST-001.
