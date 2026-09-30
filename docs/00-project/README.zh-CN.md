# DBG-C 工程文档

当前文档集是 V0.x 研发基线草案，不代表接口已经冻结或实现已验证。先评审 Phase 1 产品范围与验收，再进入接口和架构冻结；不得根据未冻结的接口规范投板。

| 编号 | 文档 | 当前状态 |
|---|---|---|
| PRD-001 | [产品需求规格书](../01-requirements/DBG-C-PRD-001.zh-CN.md) | V0.5；主动Target Power范围冻结为标称3.3 V输出，不输出1.8 V；电气限值仍待定义 |
| SYS-001 | [系统架构设计](../02-system/DBG-C-SYS-001.zh-CN.md) | V0.6；记录主动Target Power仅标称3.3 V；电路和产品集成未冻结/验证 |
| IF-001 | [DBG-C Interface Specification](../03-interfaces/DBG-C-IF-001.zh-CN.md) | V0.6；记录24触点评审输入；USB-IF适用性、电缆映射和电气保护未关闭，触点不得释放PCB |
| MCU-001 | [MCU 选型与资源评估](../04-hardware/DBG-C-MCU-001.zh-CN.md) | V0.19；加入PA5/PA6/PA7/PA12 Target前端控制资源；PA5依赖UART3重映射 |
| HW-001 | [硬件设计规范](../04-hardware/DBG-C-HW-001.zh-CN.md) | V0.5；主动输出范围定为3.3 V；VTref过压门控与Type-C映射构成阻断项；仅可继续模块化验证板设计 |
| FW-001 | [固件架构与 PoC-1 记录](../05-firmware/DBG-C-FW-001.zh-CN.md) | V0.79；PB8 KEY_MODE/PB9 LED_TARGET 原始 GPIO 56 项检查通过，按键与 LED 电气策略未定义；PB6 TARGET_PWR_EN 原始 GPIO 适配器 36 项主机寄存器检查通过且不定义电源极性；SPI1 配置 BSP 19 项及 PA0–PA3 GPIO 与 PA3 原始片选 BSP 104 项主机检查通过，数据传输未实现；PA4/A0 原始 ADC 采样适配器纳入 PoC 构建，44 项主机检查通过、硅片采样待验证；镜像更新事务管理器新增 57 项主机检查；fish 环境配置示例已补充；CH585 JTAG GPIO BSP 与上游 CMSIS-DAP JTAG Sequence、IDCODE、DP Transfer 写入/posted-read 主机模型累计 1610 项检查通过；MCU-001 V0.11 的 SWD/Reset BSP 已迁移并通过相关主机检查；队列化 CMSIS-DAP 服务 40 项主机检查通过；定长包队列 54 项主机检查通过；通用 Target Reset 服务 24 项检查、PB5 序列适配器 37 项集成检查及 PB5 GPIO 方向切换原语累计 69 项检查通过；SWD 引擎 10 个主机线模型用例经 GPIO BSP 模拟寄存器执行，3321 项断言；两次固定路径 clean rebuild 的 ELF/map 一致，当前 ELF text/data/bss 为 8916/8/5580 字节；PoC tick 目标 1000 tick/s；PB5 集成及 1000 tick/s 配置两次干净构建通过，ThreadX 实板运行仍未验证 |
| USB-001 | [USB Device 规范](../06-protocols/DBG-C-USB-001.zh-CN.md) | V0.3 架构草案；已记录 EVT CDC 端点/API 及头文件 IRQ 名与实际向量函数名不一致；DAP+CDC 描述符和端点未冻结，无产品枚举验证 |
| RF-001 | [Private 2.4G Protocol Specification](../06-protocols/DBG-C-RF-001.zh-CN.md) | V0.1 协议框架，不可据此编码 |
| TEST-001 | [Verification Specification](../07-verification/DBG-C-TEST-001.zh-CN.md) | V0.73；新增线缆映射、VTref故障隔离、3.3 V供电及误插测试；全部板级测试未执行 |
| RISK-001 | [Risk Register](../08-risk/DBG-C-RISK-001.zh-CN.md) | V0.15；新增VTref过压、Type-C用途和PB5复位极性风险R27–R29；均未关闭 |
| OPEN-001 | [未决问题与验证清单](DBG-C-OPEN-001.zh-CN.md) | V0.51；主动输出电压范围已决策；Type-C许可、VTref故障隔离和PB5复位极性仍阻断电路冻结 |

BLE-001 尚未建立；HW-001 V0.5 已建立验证板硬件设计输入；USB-001 为 V0.3 双语架构草案。

## 资料来源与证据边界

- 项目指定的 CH585M IC 手册是仓库内 [CH585/CH584 数据手册 V1.6](../09-references/CH585-CH584_Datasheet_V1.6.pdf)，作为芯片参数、引脚复用与已记载外设能力的首要依据；SDK API、并发性能和板级行为仍需 SDK 核对与实测。
- [CH585EVT 官方资料包](../09-references/CH585EVT/README.md)集中保存用户提供的原始压缩包。当前 CH585 SWD GPIO BSP 已按归档中 GPIO 实现及 MCU-001 引脚分配建立；USB、UART、BLE、RF 驱动仍须按需核对对应官方源码并保留随附声明。
- USB Type-C 设计应核对 USB-IF [Type-C Cable and Connector Specification](https://www.usb.org/usb-type-cr-cable-and-connector-specification) 的适用正式版本。规范全文尚未放入仓库。
- CMSIS-DAP v2 Bulk 和可选 CDC 设计参考 Arm [CMSIS-DAP USB Peripheral Configuration](https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__ConfigUSB__gr.html)，CMSIS_5 文档系列（CMSIS-DAP V2.1.1）。它不能证明 DBG-C 已兼容任何 IDE/OS。
- 外部网页核对日期：2026-09-28。规范冻结前应再次核对版本。
