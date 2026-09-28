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
| `05-firmware/` | 固件架构与实现计划（后续阶段） | Firmware architecture and implementation plan (later phase) |
| `06-protocols/` | RF、BLE、USB 协议（USB/BLE 文档后续阶段） | RF, BLE, and USB protocols (USB/BLE docs in later phase) |
| `07-verification/` | 验证规范与结果 | Verification specifications and results |
| `08-risk/` | 风险登记册 | Risk register |
| `09-references/` | 数据手册、规范和来源资料 | Datasheets, standards, and source materials |

## 文档入口 | Document Index

- [中文项目索引](00-project/README.zh-CN.md) · [English Project Index](00-project/README.en-US.md)
- [中文未决问题](00-project/DBG-C-OPEN-001.zh-CN.md) · [Open Questions (English)](00-project/DBG-C-OPEN-001.en-US.md)
- [产品需求 PRD-001 中文](01-requirements/DBG-C-PRD-001.zh-CN.md) · [English](01-requirements/DBG-C-PRD-001.en-US.md)
- [系统架构 SYS-001 中文](02-system/DBG-C-SYS-001.zh-CN.md) · [English](02-system/DBG-C-SYS-001.en-US.md)
- [DBG-C Interface IF-001 中文](03-interfaces/DBG-C-IF-001.zh-CN.md) · [English](03-interfaces/DBG-C-IF-001.en-US.md)
- [MCU 评估 MCU-001 中文](04-hardware/DBG-C-MCU-001.zh-CN.md) · [English](04-hardware/DBG-C-MCU-001.en-US.md)
- [RF 协议 RF-001 中文](06-protocols/DBG-C-RF-001.zh-CN.md) · [English](06-protocols/DBG-C-RF-001.en-US.md)
- [验证规范 TEST-001 中文](07-verification/DBG-C-TEST-001.zh-CN.md) · [English](07-verification/DBG-C-TEST-001.en-US.md)
- [风险登记册 RISK-001 中文](08-risk/DBG-C-RISK-001.zh-CN.md) · [English](08-risk/DBG-C-RISK-001.en-US.md)

每次修改任一语言版本时，必须同步检查另一版本、相关文档和版本历史。不得只改一侧造成要求或状态不一致。
