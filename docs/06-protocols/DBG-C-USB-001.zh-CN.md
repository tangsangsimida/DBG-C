# DBG-C USB Device 规范

**文档编号：** DBG-C-USB-001　**版本：** V0.3　**状态：** USBHS 架构草案；未冻结描述符/端点；无产品固件枚举或实板验证

## 1. 目的与边界

定义 DBG-C Probe 的 PC 有线 USB 设备方向。当前基线采用 CH585M USBHS Device，主调试接口为 CMSIS-DAP v2 Bulk；不以 CMSIS-DAP v1 HID 作为主调试通道。USBFS PB10/PB11 保留为恢复/生产评估资源，其是否接连接器、启动条件以及能否与 USBHS 并行工作均待确认。

本文件不定义 Type-C 自定义目标接口触点；该内容属于 DBG-C-IF-001。USBHS 引脚依据 MCU-001：PB12 为 U2D−，PB13 为 U2D+。该引脚分配和 USB 描述符不是同一层接口定义。

## 2. 资料依据与事实边界

| 来源 | 已核实内容 | 限制 |
|---|---|---|
| CH585/CH584 Datasheet V1.6，表 1-1（CH585M 列） | PB12/U2D− 与 PB13/U2D+ 的封装脚位；USBHS 外设能力摘要 | 不给出 DBG-C 的复合设备描述符、端点分配或 ThreadX 集成 |
| 项目 EVT：`EVT/EXAM/IAP/USBHS_IAP/src/Main.c`、`usb_desc.h` | 存在 USBHS Device/IAP 示例；示例源码定义自己的 VID/PID、端点和包长 | 示例标识符、描述符与包长属于 WCH 示例，不得复制为 DBG-C 产品值，也不证明 CMSIS-DAP + CDC |
| 项目 EVT：`EVT/EXAM/USB/USBHS/DEVICE/SimulateCDC/User/usb_desc.c/.h`、`ch585_usbhs_device.c` | 存在 USBHS CDC Device 示例，分别定义 FS/HS 描述符；CDC 使用两个接口、EP3 中断 IN 及 EP2 Bulk IN/OUT。端点初始化将 EP2 RX DMA 指向 `UART2_Tx_Buf`；EP2 OUT 中断读取 RX 长度、切换 DATA 位、设置 NAK，并清除 `CDC.DownloadPoint_Busy` | 这是 CDC 示例，不含 CMSIS-DAP；示例描述符、端点与标识符不是 DBG-C 冻结值。该 ISR 行为属于示例，未定义 DBG-C 的收包队列、数据所有权或 ISR 到 ThreadX 的同步契约 |
| 同一示例 `User/ch585_usbhs_device.h`，SHA-256 `5378baa82669a0898c3770cda070c1a65ff2b80858ab1862669b44bd16a6340e` | 声明 `USBHS_Device_Endp_Init(void)`、`USBHS_Device_Init(FunctionalState)`、`USBHS_Device_SetAddress(uint32_t)`、`USBHS_IRQHandler(void)` 和 `USBHS_Endp_DataUp(uint8_t, uint8_t *, uint16_t, uint8_t)`；头文件还定义 `DEF_UEP_NUM` 为 16，并给出 DMA/复制发送模式常量 | `USBHS_IRQHandler` 仅出现在头文件声明和 C 文件注释；C 文件实际定义 `USB2_DEVICE_IRQHandler`，归档 `EVT/EXAM/SRC/Startup/startup_CH585.S`（SHA-256 `40a6ed31faed8915e66a465ae369152cbcf57f6d39c933cd71720b8e45509d8c`）的 USB2_DEVICE 向量指向同名 handler。该命名不一致必须避免照抄为 DBG-C API。其它声明也是该样例接口，不能据此推定稳定通用 SDK API或产品端点预算 |
| 项目 EVT：`EVT/EXAM/USB/USBHS/DEVICE/CompositeKM/User/usb_desc.c/.h` | 存在 USBHS 双 HID 接口复合设备示例 | 不含 CDC 或 CMSIS-DAP，不能证明 DAP Bulk + CDC 复合设备可用 |
| CMSIS-DAP 子模块 `software/third_party/cmsis-dap`，提交 `12636590eec66fae2d1bba4518749426ad5a4595`，`Documentation/Doxygen/src/dap_firmware.md` 与 `Firmware/Config/DAP_config.h` | CMSIS-DAP v2 使用 USB Bulk；可选 CDC ACM 用于 UART；配置头文件说明 WinUSB High-Speed Bulk 常见包长为 512 字节 | 该数值是上游配置说明，不是已验证的 CH585 USBHS 驱动/描述符配置；最终端点及缓存需核实控制器实现和主机传输 |

资料审查日期：2026-09-30。项目 EVT ZIP SHA-256：`cdab364ffc24d793b300f22b3aa7ddfd97306f880d3962448da47ef9da870274`。

## 3. 设备逻辑功能

正常应用状态的逻辑功能需求：

1. CMSIS-DAP v2 调试命令通过 Bulk OUT 接收、Bulk IN 返回响应；DAP Command Core 不直接调用 USB 寄存器或 USB 驱动。
2. Target UART 通过 CDC ACM 暴露给 PC；CDC 数据路径进入 UART Bridge Service，不复用 DAP Command Core。
3. 可选 DBG-C Management USB 接口是否加入 V1 描述符，需在管理命令及 PC Tool 需求冻结后决定。
4. USB 固件升级复用 USBHS 物理连接；Bootloader 与应用可采用不同配置/描述符，但具体模式切换、身份标识、镜像协议和恢复行为待 Boot/Update 设计确定。

接口数、接口号、Alternate Setting、端点地址、传输类型和最大包长均标记为**待验证**。不得从 WCH IAP 示例、其他 DAPLink 板卡或 CH585 以外芯片推导 DBG-C 数值。

## 4. 软件数据路径

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

上述为设计分层，不代表现有产品 USBHS 代码。中断处理仅负责按正式 WCH USBHS API 完成控制器要求的收包/状态处理并通知服务上下文；DAP 命令处理不得在 ISR 中执行。ThreadX 同步原语、USBHS API 调用上下文、端点缓冲所有权和缓存一致性必须从实际 SDK 源码/库接口核实后实现。主机侧 CMSIS-DAP 服务及有界 dispatch 已有独立主机检查，但尚未接入该路径。

已审查的 EVT `SimulateCDC` 示例中，EP2 RX DMA 直接写入 `UART2_Tx_Buf`；EP2 OUT 分支在 `USB2_DEVICE_IRQHandler()` 中读取 `R16_U2EP2_RX_LEN`、切换 EP2 RX DATA 位、将 EP2 RX 响应设为 NAK，并清除 `CDC.DownloadPoint_Busy`。该分支没有把数据复制到 DBG-C 定长包队列，也没有提供 DBG-C 可复用的 ISR/ThreadX 同步实现。DBG-C 通用定长包队列明确要求调用方串行访问，因此不得直接从该示例推导 ISR 与服务线程可并发访问队列。上述仅为 EVT 源码静态证据，不代表 DBG-C 实现或 CH585M 板级验证。

该示例的 `main()` 设置系统时钟、初始化调试输出和 UART2，调用 `USBHS_Device_Init(ENABLE)`，随后在无限循环中轮询 `UART2_DataRx_Deal()` 与 `UART2_DataTx_Deal()`；没有 ThreadX 初始化或 DAP Command Core。示例 `USBHS_Device_Endp_Init()` 配置 EP2 RX/TX DMA 及 EP3 TX DMA，EP2 收发与 EP3 发送的具体缓冲和描述符属于该示例。实现 DBG-C 前仍须确定 CMSIS-DAP v2 与 CDC 共存的产品描述符/端点、数据缓冲所有权和 ISR 到线程的同步机制。

CDC ACM 采用独立收发缓冲与 UART Bridge Service。DAP 流量和 CDC 高流量同时到达时的优先级、队列容量、背压和丢弃策略须经吞吐/延迟测试后冻结，不在本草案中编造数值。

## 5. USBHS 与 USBFS 恢复关系

- USBHS：主 PC 接口，连接 PB12/PB13；目标为 USBHS Device 上的 CMSIS-DAP v2 Bulk + CDC ACM。
- USBFS：PB10/PB11 保留，当前不宣称运行中的应用 USBHS 故障后可自动切换到 USBFS。
- 待验证：两个控制器的初始化、时钟、DMA、IRQ、RAM 缓冲和端口电气是否允许同固件并行；恢复时是否通过复位/BOOT 启动另一套 USBFS 固件；官方 ISP/IAP 是否使用该恢复通道。

## 6. 描述符与操作系统策略

以下信息必须等 VID/PID 决策、实际 USBHS 栈及正式描述符实现后填写，不得猜测：

- VID、PID、bcdDevice、Manufacturer、Product、Serial 字符串策略；
- Configuration、Interface、Endpoint 数量和编号；
- CMSIS-DAP v2 WinUSB 兼容描述符或驱动安装策略；
- CDC ACM 控制/数据接口及端点；
- 管理接口是否存在；
- USB 2.0 High-Speed 枚举及 Full-Speed fallback 行为；
- Windows、Linux/udev、macOS 主机兼容性；多个 DBG-C 同连时的唯一序列号和设备选择。

验证必须使用最终固件和真实 USBHS 硬件，覆盖描述符解析、CMSIS-DAP 请求/响应、CDC 全双工、热插拔、复位恢复、多设备实例选择和长时间并发流量。

## 7. 升级模式

USB Update Transport 只传递分片并报告链路错误；统一 Update Manager 管理镜像状态、写入目标存储、完整性检查、提交、启动确认和失败恢复。USB Transport 不自行决定内部 Flash 分区，也不允许应用擦除当前唯一有效镜像。USBHS IAP 示例可用于 API/流程审查，不能据其示例分区直接确定 DBG-C Bootloader/App 边界。

## 8. 待关闭项

| 问题 | 关闭所需证据 |
|---|---|
| USBHS Device 驱动和端点能力 | EVT CDC 示例头文件声明 `USBHS_Device_Init()`、`USBHS_Device_Endp_Init()`、`USBHS_Device_SetAddress()`、`USBHS_IRQHandler()` 和 `USBHS_Endp_DataUp()`；示例 EP2 RX DMA 及 OUT ISR 操作已审查 | 当前 SDK/示例 API 与寄存器访问上下文、DBG-C 端点计划、DMA 缓冲所有权、ThreadX 同步规则、目标构建及实板枚举仍待确认 |
| CMSIS-DAP v2 + CDC ACM 复合描述符 | 具体 USB 栈/描述符实现、端点预算、主机枚举及同时传输记录 |
| USBHS 与 USBFS 恢复关系 | 官方初始化/恢复例程、资源冲突审查和验证板恢复实验 |
| VID/PID 与字符串 | 组织 VID 决策、序列号来源及生产规则 |
| USB 更新镜像及 Boot 流程 | 实际链接 map、镜像尺寸、外部 NOR 选型和断电/损坏恢复测试 |
| Host/IDE 支持矩阵 | Windows、Linux、macOS 与目标 IDE/OpenOCD/pyOCD 实测 |

相关开放项见 OPEN-001 O02、O08、O17；需求见 PRD-001；系统分层见 SYS-001；验证见 TEST-001。
