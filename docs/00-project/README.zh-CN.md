# DBG-C 工程文档

当前文档集是 V0.x 研发基线草案，不代表接口已经冻结或实现已验证。先评审 Phase 1 产品范围与验收，再进入接口和架构冻结；不得根据未冻结的接口规范投板。

| 编号 | 文档 | 当前状态 |
|---|---|---|
| PRD-001 | [产品需求规格书](../01-requirements/DBG-C-PRD-001.zh-CN.md) | V0.1，待评审 |
| SYS-001 | [系统架构设计](../02-system/DBG-C-SYS-001.zh-CN.md) | V0.2，待评审 |
| IF-001 | [DBG-C Interface Specification](../03-interfaces/DBG-C-IF-001.zh-CN.md) | V0.1 概念草案，Pin 未冻结 |
| MCU-001 | [MCU 选型与资源评估](../04-hardware/DBG-C-MCU-001.zh-CN.md) | V0.11；按数据手册/EVT 更新 USBHS、SWD/JTAG、UART、SWO、VTref、外部 SPI NOR 分配；PB16–PB21 RF 复用、BOOT 入口和电气参数待验证；现有 PoC GPIO BSP 仍需迁移 |
| FW-001 | [固件架构与 PoC-1 记录](../05-firmware/DBG-C-FW-001.zh-CN.md) | V0.52；MCU-001 V0.11 引脚基线已更新；PoC 的 SWD/Reset BSP 仍是旧引脚映射，历史主机检查不验证新分配；定长包队列 54 项主机检查通过；通用 Target Reset 服务 24 项检查、PA4 序列适配器 37 项集成检查通过；SWD 引擎 9 个主机线模型用例经 GPIO BSP 模拟寄存器执行，2759 项断言；两次固定路径 clean rebuild 的 ELF/map 一致，当前 ELF text/data/bss 为 8916/8/5580 字节；PoC tick 目标 1000 tick/s；PA4 集成及 1000 tick/s 配置两次干净构建通过，ThreadX 实板运行仍未验证 |
| RF-001 | [Private 2.4G Protocol Specification](../06-protocols/DBG-C-RF-001.zh-CN.md) | V0.1 协议框架，不可据此编码 |
| TEST-001 | [Verification Specification](../07-verification/DBG-C-TEST-001.zh-CN.md) | V0.48；SWD/Reset 当前主机测试仍覆盖旧引脚分配；定长包队列 54 项主机检查通过；新增 ThreadX 实板观测用例，当前全部未执行；UID 适配器 31 项 mock 检查和目标对象编译已执行，硅片读取未执行；UART0 与桥接主机检查结果见文档 |
| RISK-001 | [Risk Register](../08-risk/DBG-C-RISK-001.zh-CN.md) | V0.9；新增 R19 跟踪 MCU-001 引脚变更后的 PoC 映射不一致；PoC tick 目标 1000 tick/s，ThreadX 移植待实板验证 |
| OPEN-001 | [未决问题与验证清单](DBG-C-OPEN-001.zh-CN.md) | V0.39；ThreadX 创建状态/tick 观测符号已加入 PoC，实板运行未执行；1000 tick/s 配置两次固定路径干净构建及 ELF/map 复核通过，验证板设计软件门通过；产品硬件冻结未放行；O11/O21 仍开放 |

后续阶段文档 DBG-C-HW-001、USB-001、BLE-001 尚未建立。

## 资料来源与证据边界

- 项目指定的 CH585M IC 手册是仓库内 [CH585/CH584 数据手册 V1.6](../09-references/CH585-CH584_Datasheet_V1.6.pdf)，作为芯片参数、引脚复用与已记载外设能力的首要依据；SDK API、并发性能和板级行为仍需 SDK 核对与实测。
- [CH585EVT 官方资料包](../09-references/CH585EVT/README.md)集中保存用户提供的原始压缩包。当前 CH585 SWD GPIO BSP 已按归档中 GPIO 实现及 MCU-001 引脚分配建立；USB、UART、BLE、RF 驱动仍须按需核对对应官方源码并保留随附声明。
- USB Type-C 设计应核对 USB-IF [Type-C Cable and Connector Specification](https://www.usb.org/usb-type-cr-cable-and-connector-specification) 的适用正式版本。规范全文尚未放入仓库。
- CMSIS-DAP v2 Bulk 和可选 CDC 设计参考 Arm [CMSIS-DAP USB Peripheral Configuration](https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__ConfigUSB__gr.html)，CMSIS_5 文档系列（CMSIS-DAP V2.1.1）。它不能证明 DBG-C 已兼容任何 IDE/OS。
- 外部网页核对日期：2026-09-28。规范冻结前应再次核对版本。
