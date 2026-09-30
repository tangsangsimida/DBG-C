# DBG-C Interface Specification

**文档编号：** DBG-C-IF-001　**版本：** V0.5　**状态：** 概念草案；V1 Target电气架构要求已冻结；触点映射和器件参数未冻结

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
| UART TX/RX | V1 Target UART | 逻辑方向见第 4.3 节；连接器触点和电气实现待冻结 |
| nRESET | V1 Target 硬件复位 | 位于Target电压域；安全默认释放，输出级/触点待HW设计 |
| JTAG/SWO/VTref/Target Power | V1 Target相关能力 | 逻辑方向见第4.3节；Target Power为内部控制，不是Target数据线；实际触点/电路待冻结 |
| BOOT/Target Detect | 后续扩展能力 | 不属于当前冻结的 V1 Target信号映射 |

本表没有连接器 Pin 编号。Basic/Full 的实际触点映射均待评审冻结；上述 V1 逻辑能力要求不等于连接器触点已定义。

## 3. Basic 与 Full 概念

- **DBG-C Basic：** 目标是在指定被动 USB2.0 Type-C 数据线中传递最小 SWD 所需信号。必须先验证该线缆在连接器两端实际导线映射是否满足自定义信号传递；标准 USB2.0 线缆定义不能被推定为可传递任意自定义引脚信号。因此兼容线缆种类/型号仍待验证。
- **DBG-C Full：** 另行评估包含 UART、nRESET 等信号的明确线缆组件。需分别评估 Passive Full-Featured Type-C Cable、Active Cable、SBU、SuperSpeed 差分对、方向检测及 MUX。Active Cable 可能重定时/转换并不透明传递自定义低速信号；不可作为兼容前提。

不得使用“高级 Type-C 线”作为线缆要求。若最终线材不是普通被动 USB2.0 线，必须明确标识其实际导线和认证/兼容边界。

## 4. 电气与机械要求

本节定义 DBG-C Interface 的硬件设计约束。具体触点分配由本规范 Pin Mapping 章节冻结；触点分配冻结前，本节安全、状态控制、保护、机械和验证要求仍然有效。

### 4.1 接口分类

| 接口 | 定义 | 约束 |
|---|---|---|
| PC USB Interface | Probe 与 PC 连接的标准 USB Device 接口 | 按正式 USB Type-C 和 USB 2.0 规范设计；不得把 CC、VBUS、USB 数据触点改作 DBG-C 私有调试信号 |
| DBG-C Target Interface | Probe 与 Target 之间的调试接口 | 自定义物理接口；即使使用 Type-C 外形也不得声明为标准 USB Type-C 接口；触点、专用线缆、电气规则和兼容范围由本规范独立定义 |

Target Interface 必须考虑误插普通 PC Host、USB Charger 或其它 Type-C 设备，且不得造成永久损坏。CC1/CC2 不得用于 SWD/JTAG/UART/RESET 等自定义数据线路。VBUS 仅承担本规范明确规定的供电或检测功能，不作为 GPIO/调试数据。

### 4.2 信号电气参数

每个 Target 信号必须单独定义下表字段；正式原理图冻结前不得留下无法解释的空项。

| 参数 | 必须定义 |
|---|---|
| Signal Name / Connector Contact | 冻结信号名称及连接器触点 |
| Direction / Voltage Domain | Probe to Target、Target to Probe、Bidirectional；参考电压域 |
| Valid Voltage / Absolute Fault Range | 工作电压范围及异常承受范围 |
| Input Threshold / Output Level | 输入高低阈值、输出高低电平保证范围 |
| IO Type / Power-off State / Default State | IO 类型；Probe/Target 掉电状态；上电、复位、Bootloader和固件未初始化状态 |
| Pull / Drive / Series Damping | 上下拉及其电源、驱动要求、串联阻尼与依据 |
| Protection / Hot-plug | ESD、过压、反灌、短路、插拔过程行为 |
| Verification | 测试方法和通过条件 |

### 4.3 Target 信号方向

| 信号 | 逻辑方向 |
|---|---|
| TARGET_SWCLK_TCK | Probe to Target |
| TARGET_SWDIO_TMS | Bidirectional |
| TARGET_TDI | Probe to Target |
| TARGET_TDO | Target to Probe |
| TARGET_UART_TX | Probe to Target |
| TARGET_UART_RX | Target to Probe |
| TARGET_nRESET | Probe 控制，处于 Target 电压域；以 Target VTref 为参考的拉低/释放输出 |
| TARGET_SWO | Target to Probe |
| TARGET_VTREF_ADC | Target to Probe，Analog Sense |
| TARGET_PWR_EN | Probe 内部控制，不是 Target 数据接口 |

DBG-C V1 必须支持标称 1.8 V 与 3.3 V 两种 Target I/O 电压域。CH585M Probe 侧 I/O 保持在 Probe 电压域。所有面向 Target 的数字信号必须经过与 VTref 关联的电平适配，并在 Probe 或 Target 任一侧掉电时提供隔离。禁止将 CH585M GPIO 直连 Target 作为工作通路。这些属于已冻结的 V1 需求，但不代表未经确认的具体电压容差已冻结。

| 信号 | 方向 | 已冻结的电气要求 |
|---|---|---|
| TARGET_SWCLK_TCK | Probe 至 Target | 输出电平适配到 Target VTref 电压域；VTref 无效时禁止输出或隔离 |
| TARGET_SWDIO_TMS | SWD 双向；JTAG TMS 为 Probe 至 Target | 使用显式方向控制。SWD 模式下方向切换须与 SWD bit engine 及 turnaround 同步；JTAG TMS 模式固定为 Probe 至 Target。不得假定自动方向转换器件适用 |
| TARGET_TDI | Probe 至 Target | 输出电平适配到 Target VTref 电压域；VTref 无效时禁止输出或隔离 |
| TARGET_TDO | Target 至 Probe | 从 Target VTref 域适配到 Probe 域；任一侧掉电时隔离 |
| TARGET_UART_TX | Probe 至 Target | 输出电平适配到 Target VTref 电压域；VTref 无效时禁止输出或隔离 |
| TARGET_UART_RX | Target 至 Probe | 从 Target VTref 域适配到 Probe 域；任一侧掉电时隔离 |
| TARGET_SWO | Target 至 Probe | 从 Target VTref 域适配到 Probe 域；任一侧掉电时隔离 |
| TARGET_nRESET | Probe 控制，Target 电压域 | 在 Target 域实现拉低和释放；不得以固定 3.3 V 高电平驱动 Target |
| TARGET_VTREF_ADC | Target 至 Probe，模拟检测 | 使用独立且受保护的 ADC 检测路径；ADC 分压节点不得作为电平转换器供电 |

VTref 表示 Target 实际 I/O 供电电压，不是 Probe 产生的固定参考。Target 侧电平转换器供电/参考须经审查后跟随 Target VTref。ADC 检测路径与电平转换器参考路径是不同网络/功能，须分别分析负载、保护、启动顺序和掉电行为。1.8 V/3.3 V 域不要求用户通过软件手动选择；接口电平随有效 Target VTref 工作。软件可报告测量值及有效状态。

VTref 无效或 Target 未供电时，所有 Probe 至 Target 输出（`TARGET_SWCLK_TCK`、`TARGET_SWDIO_TMS`、`TARGET_TDI`、`TARGET_UART_TX`、`TARGET_nRESET`）必须保持经硬件评审的高阻或等效隔离安全态。任一侧掉电时，所有 Target 数字路径须防止向另一侧形成不可接受的反向供电。有效 VTref 范围、故障范围、误差预算和检测门限，应在电平转换器及 ADC 前端选型和审查后冻结。

### 4.4 上电默认状态

Probe 上电、复位、Bootloader运行和 GPIO 初始化前，Target Interface 必须处于安全态。未经单独批准的主动Target供电输出必须保持关闭。VTref缺失/无效时，硬件必须禁止Probe至Target数字输出。Target供电状态未确认前，输出不得注入可能反向供电的电流。nRESET 必须在 Target 电压域实现拉低/释放（开漏或经评审的等效方式），释放电平以 Target VTref 为参考，不得固定驱动为3.3 V高电平。仅在VTref有效且收到复位请求时允许拉低；Probe复位、升级、异常重启或掉电不得使 Target 长时间保持复位。

### 4.5 VTref 与电压域

`TARGET_VTREF_ADC` 检测 Target 电压。HW-001 须基于所选器件规格定义覆盖标称 1.8 V 与 3.3 V Target 域的有效最小/最大值、故障范围、ADC保护、分压、滤波、钳位、掉电隔离、校准、误差预算和有效门限。Target 电压不得超过 MCU ADC 允许范围。VTref 必须关联所有 Target 数字电平适配路径的 Target 侧参考；ADC 检测路径与电平转换器参考供电路径须分别评估。VTref 无效时，硬件必须禁止 Target 输出。

### 4.6 热插拔

允许的插拔状态须经设计定义。插入后按顺序执行：未连接 → 检测 Target 电源 → 设置安全 IO 状态 → 允许调试。拔出或 VTref 失效后，停止驱动 Target 并恢复安全态。检测滤波及软件响应时间待 Validation Board 实测后冻结。

### 4.7 反向供电

至少验证下列 Probe/Target 供电组合，并把 SWDIO、SWCLK、JTAG、UART、SWO、nRESET、VTref、Target Power 纳入测试：

| Probe | Target | 要求 |
|---|---|---|
| OFF | OFF | 不出现异常状态 |
| ON | OFF | 数据线不形成不可接受的 Target 反向供电 |
| OFF | ON | 数据线不形成不可接受的 Probe 反向供电 |
| ON | ON | 正常通信 |

不得把依赖 MCU 内部 ESD 二极管限流的方案直接视为产品保护。

### 4.8 短路与误连接

外部接口至少覆盖 Signal-to-GND、Signal-to-Target-Supply、相邻触点短路、VBUS误接、Target自供电与Probe Target Power并存，以及Target Interface误接PC Host/Charger/普通USB Device。故障解除后须按定义恢复且无永久硬件损坏。允许故障电压、持续时间和通过条件按最终产品电压范围冻结。

### 4.9 ESD

外部连接器须有 ESD 防护。系统级验证采用项目批准版本 IEC 61000-4-2；接触/空气放电等级和性能判据待最终 EMC 环境冻结，不得猜数值。保护器件钳位电压、寄生电容、漏电和动态特性须满足信号电气及完整性要求。USB D+/D− 与 RF 保护器件不能只按静态耐压选择。

### 4.10 VBUS

PC USB Interface 的 VBUS 按 USB Type-C Device 电源角色设计。Target Interface 若使用 VBUS，须定义唯一功能。不得把同一网络同时称为 PC USB VBUS、Target VTref 和任意 Target Power；电源域间必须定义方向控制和反向电流阻断。

### 4.11 CC1/CC2

PC USB Interface 的 CC1/CC2 按最终 USB Type-C Device 电源/数据角色配置，不得作普通 GPIO 或 Target 信号。Target Interface 是否利用 CC 做专用线缆方向识别须结合线缆结构评审；方案冻结前不得将 CC 当作自定义数据资源。

### 4.12 正反插

可翻转连接器须保证两个方向有明确行为：信号天然对称，或在任何 Target 输出使能前检测方向并通过 MUX 重映射。不得通过向未知触点发信号来探测方向。正反插均须执行完整功能和异常状态测试。

### 4.13 防误插

不得只靠丝印防止 Target Interface 与普通 USB-C 接口互插。须验证接入 PC Host、Charger、普通 Device 均不会永久损坏；若不能满足，须采用专用线缆结构、机械防呆、不同连接器或其它明确措施。

### 4.14 信号完整性

PC USB D+/D− 的阻抗、长度匹配、回流、过孔和 ESD 布局按采用的 USB 2.0/Type-C 规范及 PCB Stackup 设计。SWD/JTAG/UART/SWO 不套用 USB 差分阻抗；其约束根据最高频率、驱动边沿、PCB走线、连接器、线长、Target输入和实测波形确定。Validation Board 为高速 Target 输出保留可调整串联阻尼位置。产品波形至少覆盖最长线缆和最高目标调试速率。

### 4.15 串扰

触点与线缆分配须考虑相邻信号耦合。未经回流分析，不得让时钟信号与高阻输入、模拟 VTref 或敏感 RF 长距离平行。串扰门限待最高频率、线缆和 Pin Mapping 冻结后仿真或实测确定。

### 4.16 机械要求

PC USB Interface 使用项目批准的 USB Type-C Connector/Cable 规范和符合性要求。Target Interface 建立独立机械 BOM/线缆定义，至少冻结制造商、连接器料号、Plug/Receptacle角色、线缆结构与长度范围、插拔寿命、温度范围、屏蔽壳接地、PCB固定、双方向行为和专用线缆识别。不得用“普通 Type-C 线”或“高级 Type-C 线”作为可验收BOM定义。

### 4.17 接口版本兼容

V1 Pin Mapping 冻结后不得静默改变触点功能。未来改变可能电气冲突的定义须通过接口新版本、适配器、线缆识别或防误接措施管理；新旧设备互插不得造成硬件损坏。

### 4.18 Validation Board 验证

进入 Product Hardware Freeze 前，Validation Board 至少验证：正反插、上/掉电插入、带电拔出、Probe单独供电、Target单独供电、双方供电、目标电源短路、相邻触点短路、目标电压上下限、SWD最高速率、UART通信、nRESET波形、VTref、Target Power、ESD、普通USB误插和专用线缆兼容性。没有实板记录的项目保持未验证。

### 4.19 冻结规则

原理图开始前冻结信号名称、方向、MCU资源、电源域和安全默认态。PCB Layout前进一步冻结连接器型号、Pin Mapping、Target电压范围、电平转换、保护、Target Power、USB电气、专用线缆和RF接口。Product Hardware Freeze前完成热插拔、正反插、误插、ESD、反向供电、短路、信号完整性和目标兼容性的Validation Board实测。无实测内容不得标为产品验证通过。

## 5. 冻结门槛

冻结前需取得 USB-IF Type-C Cable and Connector Specification 对应版本、目标端电气需求、线缆结构/映射证据、保护方案评审及正反插/热插拔/异常误连验证记录。标称电压域及VTref跟随电平适配架构已冻结，当前为 V0.5；器件数值边界、触点映射与板级验证尚未完成。

## 6. USB-IF 资料审查记录

本轮核对 USB-IF《USB Type-C Cable and Connector Specification》Release 2.5，发布日期 2026-04-08，核对日期 2026-09-30。USB-IF 文档说明 Type-C 线缆和连接器上的第三方功能仅限规范明确描述的功能。故本项目不把 Type-C 触点等同于通用自定义信号触点，也不把普通 USB 2.0 被动线视为 SWD/UART/Reset 线缆。

| Contact/导体类别 | 本规范中的功能边界 | DBG-C 当前决定 |
|---|---|---|
| D+/D− | USB 2.0 数据 | 预留给 PC USB，不映射 Target 调试信号 |
| CC1/CC2 | Type-C 连接/方向/供电角色相关 | 依规范实现，不复用为 Target 信号 |
| VBUS/GND | 总线供电和回流 | 与 Target 电源隔离策略待定 |
| SBU1/SBU2 | 规范定义的边带信号 | 普通 USB2 线缆的导体连续性未确认，不分配自定义信号 |
| SuperSpeed TX/RX | USB 3.x 高速数据通道 | USB2 被动线不保证存在；Active Cable 不保证透明传递自定义低速信号 |
| Shield | 屏蔽/机壳相关 | EMC/ESD评审后决定连接方式 |

当前 Contact → Cable Conductor → Target Signal 映射表为空，不能冻结。需选择具体连接器与线缆组件，取得正式线缆结构信息并完成正反插导通及信号验证。USB-IF 来源：[Release 2.5 文档页](https://www.usb.org/document-library/usb-type-cr-cable-and-connector-specification-release-25)。

验证板应在强制电平适配路径两侧使用独立测试点/可隔离排针暴露 Target 信号；任何测试配置均不得形成绕过适配器的 CH585M 至 Target 工作通路。Basic/Full 分类是否保留以及任何 Type-C 自定义目标接口均待评审；在映射获批前不得把它作为唯一 Target 连接方式。
