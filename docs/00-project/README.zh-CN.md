# DBG-C 工程文档

当前文档集是 V0.x 研发基线草案，不代表接口已经冻结或实现已验证。先评审 Phase 1 产品范围与验收，再进入接口和架构冻结；不得根据未冻结的接口规范投板。

| 编号 | 文档 | 当前状态 |
|---|---|---|
| PRD-001 | [产品需求规格书](../01-requirements/DBG-C-PRD-001.zh-CN.md) | V0.2，待评审 |
| SYS-001 | [系统架构设计](../02-system/DBG-C-SYS-001.zh-CN.md) | V0.3，待评审 |
| IF-001 | [DBG-C Interface Specification](../03-interfaces/DBG-C-IF-001.zh-CN.md) | V0.1 概念草案，Pin 未冻结 |
| MCU-001 | [MCU 选型与资源评估](../04-hardware/DBG-C-MCU-001.zh-CN.md) | V0.11；按数据手册/EVT 更新 USBHS、SWD/JTAG、UART、SWO、VTref、外部 SPI NOR 分配；PB16–PB21 RF 复用、BOOT 入口和电气参数待验证；PoC SWD/Reset GPIO 软件映射已迁移至 PB1/PB0 与 PB5，主机模型与交叉构建通过；硅片和电气行为待验证 |
| FW-001 | [固件架构与 PoC-1 记录](../05-firmware/DBG-C-FW-001.zh-CN.md) | V0.60；fish 环境配置示例已补充；CH585 JTAG GPIO BSP 与上游 CMSIS-DAP JTAG Sequence、IDCODE、DP Transfer 写入/posted-read 主机模型累计 1610 项检查通过；MCU-001 V0.11 的 SWD/Reset BSP 已迁移并通过相关主机检查；队列化 CMSIS-DAP 服务 40 项主机检查通过；定长包队列 54 项主机检查通过；通用 Target Reset 服务 24 项检查、PB5 序列适配器 37 项集成检查通过；SWD 引擎 10 个主机线模型用例经 GPIO BSP 模拟寄存器执行，3321 项断言；两次固定路径 clean rebuild 的 ELF/map 一致，当前 ELF text/data/bss 为 8916/8/5580 字节；PoC tick 目标 1000 tick/s；PB5 集成及 1000 tick/s 配置两次干净构建通过，ThreadX 实板运行仍未验证 |
| USB-001 | [USB Device 规范](../06-protocols/DBG-C-USB-001.zh-CN.md) | V0.1 架构草案；描述符和端点未冻结，无产品枚举验证 |
| RF-001 | [Private 2.4G Protocol Specification](../06-protocols/DBG-C-RF-001.zh-CN.md) | V0.1 协议框架，不可据此编码 |
| TEST-001 | [Verification Specification](../07-verification/DBG-C-TEST-001.zh-CN.md) | V0.56；JTAG GPIO PB0–PB3 与上游 CMSIS-DAP JTAG Sequence、IDCODE、DP Transfer 经 GPIO BSP 的累计 1610 项主机寄存器模型检查通过；目标访问与实板仍未验证；SWD/Reset 主机检查现覆盖 PB1/PB0 与 PB5 软件映射，板测未执行；定长包队列 54 项主机检查通过；新增 ThreadX 实板观测用例，当前全部未执行；UID 适配器 31 项 mock 检查和目标对象编译已执行，硅片读取未执行；UART0 与桥接主机检查结果见文档 |
| RISK-001 | [Risk Register](../08-risk/DBG-C-RISK-001.zh-CN.md) | V0.10；R19 跟踪 PB16–PB21 RF 天线开关复用及 PB22 BOOT 条件风险；O22 追踪 EVT RF 库对相关 GPIO 的实际占用；PoC tick 目标 1000 tick/s，ThreadX 移植待实板验证 |
| OPEN-001 | [未决问题与验证清单](DBG-C-OPEN-001.zh-CN.md) | V0.40；ThreadX 创建状态/tick 观测符号已加入 PoC，实板运行未执行；1000 tick/s 配置两次固定路径干净构建及 ELF/map 复核通过，验证板设计软件门通过；产品硬件冻结未放行；O11/O21/O22 仍开放 |

后续阶段文档 DBG-C-HW-001、BLE-001 尚未建立；USB-001 已建立 V0.1 双语架构草案。

## 资料来源与证据边界

- 项目指定的 CH585M IC 手册是仓库内 [CH585/CH584 数据手册 V1.6](../09-references/CH585-CH584_Datasheet_V1.6.pdf)，作为芯片参数、引脚复用与已记载外设能力的首要依据；SDK API、并发性能和板级行为仍需 SDK 核对与实测。
- [CH585EVT 官方资料包](../09-references/CH585EVT/README.md)集中保存用户提供的原始压缩包。当前 CH585 SWD GPIO BSP 已按归档中 GPIO 实现及 MCU-001 引脚分配建立；USB、UART、BLE、RF 驱动仍须按需核对对应官方源码并保留随附声明。
- USB Type-C 设计应核对 USB-IF [Type-C Cable and Connector Specification](https://www.usb.org/usb-type-cr-cable-and-connector-specification) 的适用正式版本。规范全文尚未放入仓库。
- CMSIS-DAP v2 Bulk 和可选 CDC 设计参考 Arm [CMSIS-DAP USB Peripheral Configuration](https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__ConfigUSB__gr.html)，CMSIS_5 文档系列（CMSIS-DAP V2.1.1）。它不能证明 DBG-C 已兼容任何 IDE/OS。
- 外部网页核对日期：2026-09-28。规范冻结前应再次核对版本。
