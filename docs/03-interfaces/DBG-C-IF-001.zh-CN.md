# DBG-C Interface Specification

**文档编号：** DBG-C-IF-001　**版本：** V0.1　**状态：** 概念草案；禁止据此冻结 Pin 或投板

## 1. 定义

DBG-C Interface 指 DBG-C Probe 与 Target 之间的物理接口。Type-C 连接器外形不自动意味着 USB 数据、Type-C 兼容电气行为或自定义信号可在任意 USB-C 线缆中传递。任何自定义信号分配须经规范审阅、电气验证和线缆兼容性验证。

## 2. 概念信号需求（非 Pin 定义）

| 逻辑信号 | 用途 | 方向/电气/默认态 |
|---|---|---|
| SWDIO | SWD 双向数据 | 待验证目标电压、方向切换与保护 |
| SWCLK | SWD 时钟 | Probe 至 Target；电平与时序待验证 |
| GND | 参考地 | 连接策略与回流路径待硬件评审 |
| VBUS | 连接检测/电源相关 | 用途、是否连接、是否隔离和倒灌保护均待决策 |
| CC1/CC2 | Type-C 附件/方向检测 | 按 USB-IF Type-C 规范研究；不得直接当作任意目标信号 |
| UART TX/RX | Target UART | Full 选项；方向及电压待验证 |
| nRESET | Target 硬件复位 | Full 选项；开漏/推挽及默认态待验证 |
| SWO/VTREF/JTAG/Target Power/BOOT/Target Detect | 后续能力 | 不属于 V1 强制范围，分配待后续版本 |

本表没有连接器 Pin 编号。Basic/Full 的实际映射均待评审冻结。

## 3. Basic 与 Full 概念

- **DBG-C Basic：** 目标是在指定被动 USB2.0 Type-C 数据线中传递最小 SWD 所需信号。必须先验证该线缆在连接器两端实际导线映射是否满足自定义信号传递；标准 USB2.0 线缆定义不能被推定为可传递任意自定义引脚信号。因此兼容线缆种类/型号仍待验证。
- **DBG-C Full：** 另行评估包含 UART、nRESET 等信号的明确线缆组件。需分别评估 Passive Full-Featured Type-C Cable、Active Cable、SBU、SuperSpeed 差分对、方向检测及 MUX。Active Cable 可能重定时/转换并不透明传递自定义低速信号；不可作为兼容前提。

不得使用“高级 Type-C 线”作为线缆要求。若最终线材不是普通被动 USB2.0 线，必须明确标识其实际导线和认证/兼容边界。

## 4. 电气与机械要求（待定）

Pin 定义、信号方向、电压范围、IO 类型、默认态、上拉/下拉、过压/反接/短路保护、ESD 等级、热插拔、VBUS/CC、插拔顺序、正反插映射、防误插、阻抗/串扰与版本兼容均待原理图和实测验证。不得从外观或 CH585M GPIO 能力推导接口兼容性。

## 5. 冻结门槛

冻结前需取得 USB-IF Type-C Cable and Connector Specification 对应版本、目标端电气需求、线缆拆解/映射证据、保护方案评审及正反插/热插拔/异常误连验证记录。当前为 V0.1，未满足冻结门槛。
