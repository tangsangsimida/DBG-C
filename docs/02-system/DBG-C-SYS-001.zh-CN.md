# DBG-C 系统架构设计

**文档编号：** DBG-C-SYS-001　**版本：** V0.1　**状态：** 待评审

## 1. 系统边界

```text
有线：PC ─USB─ DBG-C Probe ─SWD Engine─ DBG-C Interface ─ Target MCU
无线：PC ─USB/CMSIS-DAP v2─ Probe A ─ DBG-C RF Protocol/2.4G ─ Probe B ─ SWD Engine ─ Target
管理：PC Bluetooth ─ BLE ─ DBG-C Application Protocol ─ Device Management / OTA / Configuration
```

BLE 不被描述为原生 CMSIS-DAP USB 直连替代。若未来需要 BLE Debug，须另行定义 PC Bridge/Proxy。

## 2. 逻辑分层

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

传输承载进入共用命令与目标控制路径；SWD Engine 和 Target Manager 不得按 USB、BLE、RF 复制三份。HAL/BSP 隔离 CH585M 寄存器及 SDK 依赖。DAP command 在 RF 链路上的语义、批处理及副作用重放规则须在 RF 规范冻结。

PC 调试协议目标是 **CMSIS-DAP v2**。DAPLink 是可选的开源固件体系/实现来源，不是“DAPLink v2”协议版本。V1 可以复用 CMSIS-DAP/DAPLink 中可移植的协议层和算法，但 CH585M 的 USB、GPIO、时钟和无线部分必须适配 WCH SDK/HAL/BSP；是否移植完整 DAPLink 固件需单独做架构/许可/工具链审查。

## 3. 设备角色与状态

角色集合：Standalone、Host、Target。两台设备硬件/BOM/MCU相同。具体角色选择来源、切换条件、USB 插入行为、冲突仲裁和持久化规则均未冻结。

状态机草案：`Standalone`；配对管理态；`Host`/`Target` 角色建立态；无线连接态；运行态（Debug/UART/Update）；断链恢复态；错误/解绑态。此为状态集合草案，转换守卫、超时和事件待 DBG-C-RF-001/ BLE-001 定义。

## 4. 数据流

- USB DAP：PC DAP 请求 → USB Transport → DAP 层；Host 角色下可本地执行或通过 RF 代理给 Target Probe → 目标响应沿反向路径返回。
- UART：Target UART service ↔ USB CDC（有线单机/Host 侧）；无线模式的 UART 数据通过 RF 独立逻辑通道并受 QoS 调度。
- BLE 管理：BLE Transport → 应用管理命令；不得绕过身份、权限和 OTA 状态机。
- BLE 目标下载（计划）：由 DBG-C Tool 通过 BLE Application Protocol 发起；传输对象是定义好的下载/管理命令，不等同于透明 BLE CMSIS-DAP。Target 类型、下载算法/镜像格式、断点续传及 PC 兼容矩阵待 DBG-C-BLE-001 设计。
- USB 分配：V1 使用 USBFS；USBHS 不启用。USBFS 端点/缓冲与 WCH SDK Device 栈对 CMSIS-DAP v2 Bulk + CDC 的支持，需要按 SDK 示例核对。
- OTA：镜像接收、校验、安装、启动确认/失败恢复具体实现依赖 SDK/Boot 证据，当前待验证。

## 5. 硬件分层

CH585M Probe 主控、USB Device 连接、2.4 GHz 天线/RF、电源与时钟、按键/状态指示、DBG-C Interface。VTREF/Target Power/BOOT/Target Detect 属后续扩展，不进入 V1 强制实现。接口电气和引脚分配、Type-C CC/VBUS、保护和电源路径均未冻结；不得据此直接画板。

## 6. 资源冲突关注

USB FS/HS 选择、RF/BLE 共存、DMA、端点缓冲、时钟、RAM、定时器及调试下载端口资源需结合官方 SDK、参考手册与原型测量核实。现有仓库只有数据手册，尚不能确认并发组合限制。

## 7. 相关规范

需求见 DBG-C-PRD-001；物理接口见 DBG-C-IF-001；芯片资源见 DBG-C-MCU-001；RF/BLE/USB 细节分别由对应协议文档定义。本架构 V0.1 不冻结 PHY、USB 描述符、引脚或角色决策。
