# DBG-C 工程文档 | Engineering Documents

本目录按研发领域组织，避免规范散落在文档根目录。正式文档采用成对语言文件：`.zh-CN.md` 为简体中文，`.en-US.md` 为英文；编号、版本和技术内容应保持一致。当前均为 V0.x 草案，具体冻结状态见各文件。

This directory is organized by engineering domain. Each controlled document has paired language files: `.zh-CN.md` for Simplified Chinese and `.en-US.md` for English. IDs, versions, and technical content must remain aligned. Current documents are V0.x drafts; see each file for its status.

## 目录结构 | Directory Structure

| Directory | 内容 | Contents |
|---|---|---|
| `00-project/` | 项目索引、未决问题 | Project index and open questions |
| `01-requirements/` | 产品需求 | Product requirements |
| `02-system/` | 系统架构 | System architecture |
| `03-interfaces/` | DBG-C Interface 规范 | DBG-C Interface specifications |
| `04-hardware/` | MCU 与硬件设计资料 | MCU and hardware design |
| `05-firmware/` | 固件架构、实现计划与当前 ThreadX PoC | Firmware architecture, implementation plan, and current ThreadX PoC |
| `06-protocols/` | RF、BLE、USB 协议（USB/BLE 文档后续阶段） | RF, BLE, and USB protocols (USB/BLE docs in later phase) |
| `07-verification/` | 验证规范与结果 | Verification specifications and results |
| `08-risk/` | 风险登记册 | Risk register |
| `09-references/` | 数据手册、规范和来源资料；`CH585EVT/` 集中存放 CH585 EVT 资料包 | Datasheets, standards, and source materials; `CH585EVT/` holds the CH585 EVT package |

## 仓库根目录 | Repository Roots

| Directory | 用途 | Purpose |
|---|---|---|
| `software/` | 所有软件代码和软件构建的根目录；软件构建配置与构建根目录均放在此处 | Root for all software code and software builds; keep software build configuration and build roots here |
| `hardware/` | 硬件文件根目录，包括原理图、符号库等 | Root for hardware files, including schematics and symbol libraries |

## 文档入口 | Document Index

- [中文项目索引](00-project/README.zh-CN.md) · [English Project Index](00-project/README.en-US.md)
- [中文未决问题](00-project/DBG-C-OPEN-001.zh-CN.md) · [Open Questions (English)](00-project/DBG-C-OPEN-001.en-US.md)
- [产品需求 PRD-001 中文](01-requirements/DBG-C-PRD-001.zh-CN.md) · [English](01-requirements/DBG-C-PRD-001.en-US.md)
- [系统架构 SYS-001 中文](02-system/DBG-C-SYS-001.zh-CN.md) · [English](02-system/DBG-C-SYS-001.en-US.md)
- [DBG-C Interface IF-001 中文](03-interfaces/DBG-C-IF-001.zh-CN.md) · [English](03-interfaces/DBG-C-IF-001.en-US.md)
- [MCU 评估 MCU-001 中文](04-hardware/DBG-C-MCU-001.zh-CN.md) · [English](04-hardware/DBG-C-MCU-001.en-US.md)
- [代码规范 CODE-001 中文](05-firmware/DBG-C-CODE-001.zh-CN.md) · [Coding Standard](05-firmware/DBG-C-CODE-001.en-US.md)
- [RF 协议 RF-001 中文](06-protocols/DBG-C-RF-001.zh-CN.md) · [English](06-protocols/DBG-C-RF-001.en-US.md)
- [验证规范 TEST-001 中文](07-verification/DBG-C-TEST-001.zh-CN.md) · [English](07-verification/DBG-C-TEST-001.en-US.md)
- [风险登记册 RISK-001 中文](08-risk/DBG-C-RISK-001.zh-CN.md) · [English](08-risk/DBG-C-RISK-001.en-US.md)
- [CH585EVT 官方资料包目录](09-references/CH585EVT/README.md) · [Package index](09-references/CH585EVT/README.md)

每次修改任一语言版本时，必须同步检查另一版本、相关文档和版本历史。不得只改一侧造成要求或状态不一致。

术语基线：PC 调试协议写作 **CMSIS-DAP v2**；**DAPLink** 指可选的开源固件体系/实现来源，不写作“DAPLink v2”协议。V1 的 USB 目标是 CMSIS-DAP v2 Bulk + CDC，底层移植策略后续评审。

Terminology baseline: call the PC debug protocol **CMSIS-DAP v2**. **DAPLink** is an optional open-source firmware system/source, not a “DAPLink v2” protocol. The V1 USB target is CMSIS-DAP v2 Bulk + CDC; the porting strategy remains for later review.
