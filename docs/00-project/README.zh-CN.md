# DBG-C 工程文档

当前文档集是 V0.x 研发基线草案，不代表接口已经冻结或实现已验证。先评审 Phase 1 产品范围与验收，再进入接口和架构冻结；不得根据未冻结的接口规范投板。

| 编号 | 文档 | 当前状态 |
|---|---|---|
| PRD-001 | [产品需求规格书](../01-requirements/DBG-C-PRD-001.zh-CN.md) | V0.1，待评审 |
| SYS-001 | [系统架构设计](../02-system/DBG-C-SYS-001.zh-CN.md) | V0.1，待评审 |
| IF-001 | [DBG-C Interface Specification](../03-interfaces/DBG-C-IF-001.zh-CN.md) | V0.1 概念草案，Pin 未冻结 |
| MCU-001 | [MCU 选型与资源评估](../04-hardware/DBG-C-MCU-001.zh-CN.md) | V0.1，仅基于本地数据手册的初步评估 |
| RF-001 | [Private 2.4G Protocol Specification](../06-protocols/DBG-C-RF-001.zh-CN.md) | V0.1 协议框架，不可据此编码 |
| TEST-001 | [Verification Specification](../07-verification/DBG-C-TEST-001.zh-CN.md) | V0.1 测试计划，无测试结果 |
| RISK-001 | [Risk Register](../08-risk/DBG-C-RISK-001.zh-CN.md) | V0.1 初始风险 |
| OPEN-001 | [未决问题与验证清单](DBG-C-OPEN-001.zh-CN.md) | V0.1 开放项 |

后续阶段文档 DBG-C-HW-001、FW-001、USB-001、BLE-001 尚未建立。

## 资料来源与证据边界

- 项目指定的 CH585M IC 手册是仓库内 [CH585/CH584 数据手册 V1.6](../09-references/CH585-CH584_Datasheet_V1.6.pdf)，作为芯片参数、引脚复用与已记载外设能力的首要依据；SDK API、并发性能和板级行为仍需 SDK 核对与实测。
- USB Type-C 设计应核对 USB-IF [Type-C Cable and Connector Specification](https://www.usb.org/usb-type-cr-cable-and-connector-specification) 的适用正式版本。规范全文尚未放入仓库。
- CMSIS-DAP v2 Bulk 和可选 CDC 设计参考 Arm [CMSIS-DAP USB Peripheral Configuration](https://arm-software.github.io/CMSIS_5/DAP/html/group__DAP__ConfigUSB__gr.html)，CMSIS_5 文档系列（CMSIS-DAP V2.1.1）。它不能证明 DBG-C 已兼容任何 IDE/OS。
- 外部网页核对日期：2026-09-28。规范冻结前应再次核对版本。
