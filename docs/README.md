# DBG-C 工程文档

[English version](README.en-US.md)

本目录按研发领域组织项目文档。受控文档采用 `.zh-CN.md` 和 `.en-US.md` 两种独立语言文件；修改任一语言时，同步检查对应版本、编号、版本状态和相关链接。各文档当前状态以文件内说明为准，草案或测试计划不代表设计冻结或验证通过。

## 目录结构

| 目录 | 内容 |
|---|---|
| `00-project/` | 项目索引、未决问题 |
| `01-requirements/` | 产品需求 |
| `02-system/` | 系统架构 |
| `03-interfaces/` | DBG-C Interface 规范 |
| `04-hardware/` | MCU 与硬件设计资料 |
| `05-firmware/` | 固件架构、实现计划与 ThreadX PoC |
| `06-protocols/` | RF、BLE、USB 协议 |
| `07-verification/` | 验证规范与结果 |
| `08-risk/` | 风险登记册 |
| `09-references/` | 数据手册、规范和来源资料；`CH585EVT/` 集中存放 CH585 EVT 资料包 |

## 仓库根目录

| 目录 | 用途 |
|---|---|
| `software/` | 所有软件代码和软件构建的根目录；软件构建配置与构建根目录均放在此处 |
| `hardware/` | 硬件文件根目录，包括原理图、符号库等 |

## 文档入口

- [项目索引](00-project/README.zh-CN.md) · [未决问题](00-project/DBG-C-OPEN-001.zh-CN.md)
- [产品需求 PRD-001](01-requirements/DBG-C-PRD-001.zh-CN.md) · [系统架构 SYS-001](02-system/DBG-C-SYS-001.zh-CN.md)
- [DBG-C Interface IF-001](03-interfaces/DBG-C-IF-001.zh-CN.md) · [MCU 选型与资源评估 MCU-001](04-hardware/DBG-C-MCU-001.zh-CN.md)
- [代码规范 CODE-001](05-firmware/DBG-C-CODE-001.zh-CN.md) · [固件架构与 PoC-1 FW-001](05-firmware/DBG-C-FW-001.zh-CN.md)
- [USB Device 规范 USB-001](06-protocols/DBG-C-USB-001.zh-CN.md) · [RF 协议 RF-001](06-protocols/DBG-C-RF-001.zh-CN.md)
- [验证规范 TEST-001](07-verification/DBG-C-TEST-001.zh-CN.md) · [风险登记册 RISK-001](08-risk/DBG-C-RISK-001.zh-CN.md)
- [CH585EVT 官方资料包目录](09-references/CH585EVT/README.md)

## 术语基线

PC 调试协议称为 **CMSIS-DAP v2**。DBG-C 固件按 **DAPLink 固件体系**组织并采用 **CMSIS-DAP v2** 协议；DAPLink 不是“DAPLink v2”协议，是否移植完整上游固件需另行核实。V1 有线主接口基线为 USBHS 上的 CMSIS-DAP v2 Bulk + CDC；USBFS 保留作恢复评估，底层移植范围和描述符仍待评审。
