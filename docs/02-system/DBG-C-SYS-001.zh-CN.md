# DBG-C 系统架构设计

**文档编号：** DBG-C-SYS-001　**版本：** V0.5　**状态：** 架构草案；V1外部供电Target电压域及VTref跟随电平适配架构已冻结；产品集成及实板验证未执行

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
    USBHS / BLE / RF Transport
    DAPLink 固件组件 / CMSIS-DAP v2 DAP Command Core
    Service (UART Bridge, Reset, Update, Device Management)
    Target Manager
    SWD Engine
    HAL
    BSP / CH585M SDK
```

传输承载进入共用命令与目标控制路径；SWD Engine 和 Target Manager 不得按 USB、BLE、RF 复制三份。HAL/BSP 隔离 CH585M 寄存器及 SDK 依赖。DAP command 在 RF 链路上的语义、批处理及副作用重放规则须在 RF 规范冻结。

固件按 DAPLink 体系组织；PC 调试目标为 **CMSIS-DAP v2 + USB Bulk**，不实现 CMSIS-DAP v1 HID 作为主调试通道。DAPLink 是固件体系/开源实现来源，不是“DAPLink v2”协议。DAP Command Core 与 USBHS、BLE、RF Transport 解耦；无线载荷进入同一 DAP Command Core，RF 负责传输可靠性和分片。当前上游 CMSIS-DAP 代码已有独立主机模型/目标对象检查，但尚未接入产品 USBHS 收包路径。CH585M 的 USB、GPIO、时钟、无线和 ThreadX 适配须经 WCH SDK/HAL/BSP 隔离；完整移植 DAPLink 的模块范围、许可与构建集成尚未定案。

## 3. 设备角色与状态

角色集合：Standalone、Host、Target。两台设备硬件/BOM/MCU相同。具体角色选择来源、切换条件、USB 插入行为、冲突仲裁和持久化规则均未冻结。

状态机草案：`Standalone`；配对管理态；`Host`/`Target` 角色建立态；无线连接态；运行态（Debug/UART/Update）；断链恢复态；错误/解绑态。此为状态集合草案，转换守卫、超时和事件待 DBG-C-RF-001/ BLE-001 定义。

## 4. 数据流

- USB DAP：PC DAP 请求 → USB Transport → DAP 层；Host 角色下可本地执行或通过 RF 代理给 Target Probe → 目标响应沿反向路径返回。
- UART：Target UART service ↔ USB CDC（有线单机/Host 侧）；无线模式的 UART 数据通过 RF 独立逻辑通道并受 QoS 调度。
- BLE 管理：BLE Transport → 应用管理命令；不得绕过身份、权限和 OTA 状态机。
- BLE 目标下载（计划）：由 DBG-C Tool 通过 BLE Application Protocol 发起；传输对象是定义好的下载/管理命令，不等同于透明 BLE CMSIS-DAP。Target 类型、下载算法/镜像格式、断点续传及 PC 兼容矩阵待 DBG-C-BLE-001 设计。
- USB 分配：PC 主机连接使用 USBHS PB12/PB13，目标为 USBHS Device 上的 CMSIS-DAP v2 Bulk，并提供 UART CDC ACM；管理接口是否独立暴露待 USB 规范定义。USBFS PB10/PB11 保留为恢复/生产通道评估，不假设其与 USBHS 可同时运行。EVT 有 USBHS Device 与 USBHS IAP 示例，但不证明 DBG-C 复合设备、ThreadX 同步、端点分配或主机兼容性。
- OTA：镜像接收、校验、安装、启动确认/失败恢复具体实现依赖 SDK/Boot 证据，当前待验证。

## 5. 硬件分层

CH585M Probe 主控、USBHS Device 接入 PC Host、USBFS 恢复资源、2.4 GHz 天线/RF、电源与时钟、按键/状态指示、DBG-C Interface、Target SWD/JTAG/SWO/UART、VTref ADC、Target 电源控制及外部 SPI NOR。V1支持外部供电、标称 I/O 域为1.8 V与3.3 V的Target。所有Target数字信号须使用VTref跟随电平适配；VTref无效时由硬件禁止输出，任一侧掉电时隔离。DBG-C主动输出Target电源属于单独的待决策产品能力。引脚基线见 MCU-001；电气实现见 HW-001。本文件不是电气原理图。转换器件及数值边界、主动Target供电、VBUS隔离、Type-C CC/VBUS、保护、BOOT条件、RF天线开关复用和外部Flash型号仍未冻结，不得据此冻结产品硬件。

## 6. 资源冲突关注

USB FS/HS 选择、RF/BLE 共存、DMA、端点缓冲、时钟、RAM、定时器及调试下载端口资源需结合官方 SDK、参考手册与原型测量核实。仓库已有 CH585EVT 压缩包和 PoC-1 实际使用的少量 EVT 启动/链接/头文件副本；归档索引日期为 2026.08，但没有单独标注 WCH SDK 语义版本。FreeRTOS 示例不是并发资源规范，也不能确认 DBG-C 产品功能的并发组合限制；独立参考手册和原型测量仍缺。

## 7. 相关规范

需求见 DBG-C-PRD-001；物理接口见 DBG-C-IF-001；芯片资源见 DBG-C-MCU-001；RF/BLE/USB 细节分别由对应协议文档定义。本架构不冻结 PHY、USB 描述符、端点、角色、OTA 分区或电气设计。
