# DBG-C 硬件设计规范

**文档编号：** DBG-C-HW-001　**版本：** V0.4　**状态：** 验证板设计输入草案；V1 Target电平适配架构已冻结；转换器选型、数值电气边界及主动Target供电仍开放；不是产品板冻结依据

## 1. 范围与设计门

本文定义 CH585M 验证板及后续产品硬件的电气设计输入。MCU 封装引脚和外设分配以 [MCU-001](DBG-C-MCU-001.zh-CN.md) 为准；DBG-C 与 Target 之间的连接器触点仍由 [IF-001](../03-interfaces/DBG-C-IF-001.zh-CN.md) 管理。当前软件门只放行验证板设计。验证板可用于实测，不代表 V1 产品电路或 PCB 已冻结。

判定：**Validation Board 原理图可以继续，但当前仅可绘制已定义信号名/方向/MCU资源/电源域/安全默认态的核心、USB PHY、隔离测试入口和外围模块；电平转换器选型、Target供电、连接器触点和器件级故障参数未定的电路须保持模块化，不得当作定案电路。** PCB Layout和产品硬件冻结门均未通过。

## 2. 证据和边界

芯片参数以仓库 [CH585/CH584 数据手册 V1.6](../09-references/CH585-CH584_Datasheet_V1.6.pdf) 为项目基准；其 SHA-256 为 `68270bbf3424956f36183bb6f0fef5b8892b540026f56fab3a7c0a1976d596f3f`。官方 EVT 归档索引标记为 2026.08。已检查归档内 `EVT/PUB/CH585SCH.pdf`、USBHS IAP、GPIO/UART/SPI/ADC 驱动和 RF 示例。EVT 原理图是参考设计证据，不是 DBG-C 已评审电路；预编译 RF 库、硅片电气表现和板级结果不能从示例推定。归档 SHA-256 与使用规则见 [EVT 资料说明](../09-references/CH585EVT/README.md)。

| 结论类别 | 含义 |
|---|---|
| 手册/源码确认 | 仅确认资料明确列出的器件功能、封装引脚或软件接口 |
| 验证板设计输入 | 为获得可测结果而保留的连接、跳线或测试点，不是产品承诺 |
| 待验证 | 缺少足够的官方证据、选型数据或实板结果；不得填入经验参数 |

## 3. 原理图模块划分

仓库当前没有已提交的 EDA 工程或硬件命名规范。以下为验证板的建议页层级，最终页名须服从选定 EDA 工程规则：

| 页序 | 建议模块 | 设计边界 |
|---:|---|---|
| 01 | CH585M 核心 | QFN48、所有电源脚、去耦、参考时钟、复位 |
| 02 | 电源入口与电源树 | PC VBUS、系统电源、探针电源、Target 电源隔离 |
| 03 | 编程、调试与恢复 | TIO/TCK、RST、BOOT、USBFS/UART 可恢复入口 |
| 04 | USBHS PC 接口 | USB-C receptacle、CC、VBUS 检测、D+/D−、ESD |
| 05 | USBFS 恢复接口 | 独立可访问恢复连接，不与 USBHS 并线 |
| 06 | Target 调试与 UART | SWD/JTAG/UART/Reset 的 VTref 关联电平适配前端；测试点位于适配路径两侧且可隔离，不提供绕过适配器的 Target 工作通路 |
| 07 | VTref 与 Target 电源 | ADC 前端、电平兼容、限流/反灌保护 |
| 08 | 外部 SPI NOR | PA0–PA3、器件焊盘和总线测试点；器件待选型 |
| 09 | BLE/私有 RF | RF 供电、官方参考网络、天线和测试边界 |
| 10 | 按键、指示灯与测试点 | GPIO 负载、可观测信号、可配置跳线 |

## 4. MCU 引脚和网络基线

以下 MCU 侧网络采用 MCU-001 当前分配，不定义 Type-C 连接器 Pin，也不代表各项可同时运行已验证。

| 引脚 | MCU 侧网络/用途 | 原理图约束 |
|---|---|---|
| PB0 / PB1 | TARGET_SWCLK_TCK / TARGET_SWDIO_TMS | SWD/JTAG 共用信号；保留独立测试点和可隔离路径 |
| PB2 / PB3 | TARGET_TDI / TARGET_TDO | JTAG 信号；未用时状态待固件/电气设计确定 |
| PB4 / PB7 | TARGET_UART_RX / TARGET_UART_TX | MCU 视角方向；分别连接 Target TX / RX，经电气前端 |
| PB5 | TARGET_nRESET | 与 MCU 自身 PB23/RST 分网，驱动拓扑待定 |
| PB6 | TARGET_PWR_EN | 仅为 GPIO 控制资源，不直接驱动负载 |
| PA4 | TARGET_VTREF_ADC | ADC A0；必须经过限压、限流设计，不能直接假定兼容任意 VTref |
| PB20 | TARGET_SWO | UART3 RX 重映射资源；与 RF 天线开关复用待解冲突 |
| PB10 / PB11 | USBFS D− / D+ | 恢复通道，不与 USBHS 并接 |
| PB12 / PB13 | USBHS D− / D+ | 主机 USB Device 接口 |
| PB14 / PB15 | CH585 TIO / TCK | Probe 自身调试；不得与 Target 调试网混用 |
| PB22 / PB23 | BOOT 板级网络 / CH585 RST | BOOT 进入条件待官方文档确认；PB23 为芯片自身复位 |
| PA0–PA2 / PA3 | SPI1 SCK/MOSI/MISO / GPIO CS | SPI1 传输与恢复行为尚未实现；PA3 片选为项目分配 |
| PB8 / PB9 / PB18 / PB19 | KEY_MODE / LED_TARGET / LED_USB / LED_RF | PB18–PB19 与 RF 天线开关复用冲突需先解决 |
| PB16 / PB17 / PB21 | EXT_IRQ / EXT_RESET_N / 预留 | 与 RF 天线开关资源潜在冲突；使用可隔离焊盘 |

## 5. CH585M 核心、电源与时钟

### 5.1 电源网络

CH585M 电源脚的依据为数据手册 §1.2 表 1-1、§5.1 图 5-1、§22.2 表 22-2，以及 EVT `EVT/PUB/CH585SCH.pdf`。当前证据给出的连接/数值输入如下；DC-DC模式、VDD33/VIO33电源关系及整板元件值须在供电方案评审中确定，不得混用两种模式的数值。

| 网络/引脚 | 官方用途/建议 | 原理图决策状态 |
|---|---|---|
| VDD33 | 系统电源输入。表22-2：使用USB时3.15–3.45 V；不使用USB时1.85–3.6 V | USBHS V1按使用USB条件审查；系统稳压来源/预算待闭环 |
| VIO33 | GPIO与Flash I/O电源输入。表22-2给出的USB/非USB范围分别为3.15–3.45 V/1.85–3.6 V | 不与Target VTref混网；与VDD33关系按WCH参考电路评审 |
| VDCID | 内部数字LDO电源输入；启用DC-DC时建议4.7 µF（支持1–10 µF），禁用时建议不小于1 µF | 按最终DC-DC模式选值；器件和布局待审查 |
| VSW | DC-DC开关输出；启用时须靠近引脚串接电感至VDCID，建议10 µH（支持4.7–22 µH）；禁用时可直连VDCID | DC-DC模式及电感选型待功耗/启动评审，不混用不同模式连接 |
| VDD33去耦 | 手册引脚表：启用DC-DC建议2.2 µF或1 µF；禁用时建议不小于1 µF | 与实际电源源阻抗、USB负载和布局共同评审 |
| VINTA | 内部模拟电源节点；禁用DC-DC建议0.47 µF；启用时建议不小于0.47 µF，支持0.47–2.2 µF | 按供电模式及WCH布局要求实现 |
| VDCIA | 内部模拟LDO电源输入；建议0.1 µF并直连VDCID | 逐网核对参考电路 |
| GND/底板 | 公共地和芯片底板 | 焊盘连接按封装资料及WCH land pattern评审 |

数据手册 §5.1 说明系统电源从VDD33输入，GPIO/Flash I/O电源从VIO33输入；上电默认直通供电，DC-DC为可选节能模式。验证板必须明确选择的模式并使供电连接、去耦/电感和固件配置相符。WCH EVT拓扑可作为参考，不能替代DBG-C电源预算与PCB评审。

### 5.2 去耦与储能

每个供电脚附近按 WCH 推荐布局放置去耦；RF/USB 负载瞬态和供电完整性需结合官方参考设计。器件值、数量、封装和位置须进入原理图/BOM评审记录，不能用本文猜定。验证板应提供 MCU 供电轨、VBUS、Target 电源的测量点。

### 5.3 时钟

32 MHz 晶振接 X32MI/X32MO，按手册和 EVT 参考设计核对负载电容、ESR、驱动和布局。EVT 参考图标注 32 MHz ±10 ppm、12 pF、30 Ω 的晶体示例；该标注只描述参考图所选器件，不自动成为 DBG-C 采购规格。需取得所选晶体完整数据并核算负载。

PA10/PA11 对应 32 kHz 晶体功能。是否装配须由 CH585 BLE/RF SDK 对低速时钟源的选择、精度、功耗和 RF 行为要求决定；现有资料检查尚未形成可追溯决策，验证板宜预留选装焊盘，值与装配状态待定。

### 5.4 MCU Reset、TIO/TCK、BOOT 与恢复

PB23/RST 是 CH585 自身复位；PB14/TIO、PB15/TCK 是芯片自身调试接口。验证板应保留可接触的调试/复位点。PB22 在 WCH 参考图连至板级 BOOT 下载开关，但手册/已审查示例尚未证明进入条件、电平、复位采样顺序以及对应 ISP 介质。不得据此直接冻结按钮或上拉/下拉值。

恢复设计必须包含不依赖应用固件运行的入口。USBFS PB10/PB11、官方 UART ISP 相关 PA8/PA9/PA14/PA15 的具体组合与进入顺序待 WCH 官方下载说明确认。验证板在证据补齐前应保留可访问的恢复测试点/焊盘，USBHS 应用层升级不得作为唯一恢复路径。

## 6. USB 接口硬件

### 6.1 USBHS PC 接口

PB12=USBHS D−、PB13=USBHS D+ 已由数据手册与 EVT 示例支持，可继续绘制 USBHS PHY 原理图模块。Type-C receptacle 的 CC1/CC2、VBUS、GND、Shield、Device 下拉/供电角色、VBUS 检测、ESD、串联器件及差分布线必须依 USB-IF Type-C/USB 规范和 CH585 官方参考设计逐项核对；元件值、差分阻抗及保护型号未冻结。

CMSIS-DAP v2 Bulk、CDC ACM 描述符/端点和 VID/PID 属于固件/USB 规范，未冻结不阻止绘制物理 PHY，但不得假称枚举兼容已验证。普通 USB-C 电脑线用于 PC USB 功能；Target 自定义信号不得借用该线缆触点。

### 6.2 USBFS Recovery

PB10/PB11 是 USBFS D−/D+。USBFS 是否可与 USBHS 同固件并发仍未验证。验证板建议提供独立可接入的恢复连接/焊盘，并通过跳线或明确隔离避免两 PHY 电气并接。连接器种类、恢复固件入口和同时供电关系待设计验证。

## 7. DBG-C Target Interface 与线缆

IF-001 的 Basic/Full 仍是概念分类，没有冻结连接器触点。USB-IF Type-C Cable and Connector Specification Release 2.5（2026-04-08）是本轮检索到的正式版本；其适用范围明确限制 USB Type-C 连接器/线缆的第三方功能须为规范明确描述的功能。因此不能将 Type-C receptacle 触点直接视为可承载任意 DBG-C 信号，也不能把普通 USB2.0 被动线描述为传输 SWD/UART/Reset 的线缆。

| 触点/导体类别 | USB 规范用途 | DBG-C 验证板结论 |
|---|---|---|
| D+/D− | USB 2.0 数据 | 用于 PC USBHS/USBFS；不可直接复用为 Target 信号 |
| CC1/CC2 | Type-C attach/orientation/供电角色相关 | 不作为 Target GPIO；按 USB Type-C 规范实现 |
| VBUS/GND | 总线电源/回流 | 不得未经电源隔离就与 Target 供电并网 |
| SBU1/SBU2 | 规范定义的 Sideband 用途 | 普通 USB2.0 线缆是否具备贯通导体不能假定；不能据此冻结自定义映射 |
| SuperSpeed TX/RX 对 | USB 3.x 高速差分链路 | 被动 USB2.0 线缆不保证存在；主动线缆可能含重定时/转换，不得假定透明传递低速自定义信号 |
| Shield | 屏蔽/机壳相关 | 连接策略须由 EMC/ESD 和电源设计确定 |

连接器 Contact→Cable Conductor→Target Signal 映射目前**无可批准行**：需选定具体连接器/线缆组件，并用其正式结构资料、方向矩阵和实物导通测量证明后才能填表。正反插、CC方向检测与可能的 MUX 亦未冻结。验证板应把 Target SWD/JTAG/UART/nRESET/SWO/VTref/Target Power/GND 全部引出到独立测试点或可隔离排针，Type-C 实验不得成为其它模块的唯一测试通道。

## 8. Target 电气前端

### 8.1 电压兼容与 SWD/JTAG/UART

DBG-C V1 外部供电 Target 的 I/O 电压域已冻结为**标称 1.8 V 与 3.3 V**。CH585M 保持在 Probe 侧 I/O 电压域。所有面向 Target 的数字信号必须经过与 VTref 关联的电平适配，并在 Probe 或 Target 任一侧掉电时隔离。禁止将 CH585M GPIO 直连 Target 作为工作通路。Target 侧逻辑域跟随有效 Target VTref，不要求用户通过软件手动选择。此处冻结的是接口架构，不是两个标称电压周围未经证实的数值容差。

电平转换器选型必须满足以下准则，并在产品原理图冻结前逐项依据器件数据手册及验证板实测关闭：

- Probe 侧供电为 DBG-C Probe I/O 电压域；Target 侧供电/参考通过经审查且受保护的路径跟随 Target VTref，并支持标称 1.8 V 与 3.3 V 域。
- 任一侧电源缺失时，器件须有 Ioff 或有正式资料证明等效的掉电隔离能力；Target 侧输出可保持高阻或另一种经评审的安全态。
- 输入阈值须由选定供电条件下的器件规格保证；不得依赖 CH585M GPIO 直接识别 Target 域电平。
- SWDIO 必须显式控制方向。方向控制路径须在与 SWD bit engine 同步时满足 SWD turnaround 时序。无证据证明支持推挽 SWD 波形及 turnaround 时，不得选用自动方向器件。
- 传播延迟、输出驱动、边沿、负载和通道偏斜必须纳入最终 SWD/JTAG/SWO/UART 时序预算。目前时序预算和最高速率待确定，本文不编造数值器件门限。
- 仅面向开漏总线的器件不得承载推挽 SWD/JTAG 信号，除非具体电路经过正式评审和验证。
- 每路均须审查绝对最大额定值、掉电注入/反灌、ESD、故障电压、热插拔和输出使能默认行为。

信号方向要求：SWCLK/TCK、TDI、Probe UART TX 为 Probe 至 Target 输出；TDO、Target UART TX、SWO 为 Target 至 Probe 输入；SWD 模式下 SWDIO 双向；JTAG 模式下 TMS 为 Probe 至 Target。验证板应保留可替换器件位和测试点，不得提供绕过适配路径的工作通路。

### 8.2 Target nRESET

PB5 固件方向切换仿真不证明硬件开漏。nRESET 位于 Target 电压域，默认必须释放；调试器断电、复位、Bootloader 运行或固件异常时不得拉低 Target。输出级必须仅在 Target 域拉低并释放，由 Target 侧 VTref 建立释放电平，不得向 Target 输出固定 3.3 V 高电平；并须具备掉电隔离、VTref 无效时禁止断言。具体器件、上拉值、极性、脉宽及故障行为必须由器件规格和实板验证确定。

### 8.3 VTref 检测

VTref 表示 Target 实际 I/O 供电，不是 Probe 产生的固定参考。V1 外部供电 Target 要求覆盖标称 1.8 V 与 3.3 V 域。PA4/A0 检测必须能区分有效 Target 供电、掉电和异常状态。当前仅有原始 ADC BSP 和主机模拟寄存器检查，尚无硅片采样、分压、量程、误差或保护验证。须依据 CH585M ADC 输入/参考、注入电流、精度及所选转换器规格计算允许最小/最大值、故障范围、分压、输入阻抗、RC、钳位、校准和误差预算。仅凭标称电压不能定义有效门限。

VTref 检测与转换器参考供电属于不同电气路径/功能。受保护的 Target VTref 路径应为电平适配器 Target 侧供电/参考；不得将 PA4 分压节点作为转换器供电。分别分析负载与启动顺序。VTref 缺失、无效或异常时，硬件须禁止 Probe 至 Target 输出并进入安全隔离态。软件可以显示电压及有效状态，但不选择电气电压域。

### 8.4 Target Power

V1 必须支持 Target 使用自身外部电源、I/O 域为标称 1.8 V 或 3.3 V 时的调试能力；这与 DBG-C 主动输出 Target 电源是不同能力。DBG-C 是否必须主动供电，以及输出须支持 1.8 V、3.3 V 或两者，仍是明确的产品决策项。PB6 仅为控制 GPIO，不能代表已定义供电电路。决策完成前不得宣称主动供电能力或冻结其电源模块。若保留主动输出，须分别定义输出电压选择、电流、电源来源、负载开关/稳压、限流、短路、反向阻断、浪涌、Fault 报告及与外部供电 Target 并存的处理。验证板可预留断开且可隔离的模块和测量点。电流测量不是 V1 强制功能。

## 9. SPI NOR、RF 与 UI

### 9.1 SPI NOR

PA0–PA2 分配 SPI1，PA3 为 GPIO CS。当前传输驱动、超时恢复、存储模型未完成；外部 Flash 容量不能由 PoC 镜像大小推定。选型前需预算 APP、更新暂存、回滚、恢复、配置、元数据、校验和未来离线 Target 镜像；需要的并存副本和故障恢复语义应明确后，再选常见 3.3 V SPI NOR。器件型号/容量/封装待决。

| 数据类别 | 规划用途 | 冻结边界 |
|---|---|---|
| 运行中 Probe APP | 运行于CH585M内部CodeFlash | 以最终链接map和Boot布局核算，不假定外部NOR可替代内部启动映像 |
| USB/RF/后续BLE升级镜像 | 经统一Update Manager接收，外部NOR用于整镜像暂存；完整校验后再请求Boot/安装阶段处理 | 镜像格式、完整性/认证算法、提交原子性和Boot接口未冻结；不得直接擦写唯一运行映像 |
| Rollback/Recovery | 保留恢复映像或旧版的能力需由Boot更新机制及Flash容量共同决定 | 外部暂存不自动等于回滚；内部A/B、BackupUpgrade或外部启动均未选定 |
| 配置/配对元数据 | 持久化需求评审后选择DataFlash或外部NOR | 写入原子性、寿命、布局、唯一身份绑定未冻结 |
| 更新元数据 | 记录接收状态、长度、版本、校验结果和恢复阶段 | 字段/格式跟随正式更新协议与Boot设计冻结；本文不假定存储结构 |
| 离线Target映像 | 为后续离线烧录预留容量预算输入 | 不属于当前已冻结V1强制功能；只有容量预算确定后才选Flash |

USB、2.4 GHz与后续BLE仅是镜像传输入口，Flash写入、完整性检查、提交状态及恢复由统一更新管理逻辑处理。当前SPI1传输和NOR后端未实现，以上为容量预算/可靠性设计边界，不是可运行的分区方案。

### 9.2 BLE 与 2.4 GHz RF

BLE 与私有 RF 使用同一 RF 子系统，当前不假定同时完整工作。EVT `CH58x_gpio.h` 声明 `RB_RF_ANT_SW_EN` 会将 PB16–PB21 用作天线开关控制输出；因此 PB16/PB17 扩展、PB18/PB19 LED、PB20 SWO、PB21 保留可能冲突。公开示例未找到显式 remap 调用不能证明预编译 RF 库不会启用。验证板须让这些网通过 0R/焊桥/断开选项隔离；不得在冲突未明前同时固定接入负载。

RF 天线、匹配网络、净空、地平面和走线必须来自可追溯 WCH 官方 RF 参考资料与所选天线厂商资料。当前已审查 EVT 原理图文本不足以确认 DBG-C PCB 的完整 RF 匹配/布局条件；可留匹配网络位置，但不填未经依据的数值。投 PCB 前须有 RF 原理图/布局评审及可执行的 VNA 或实板调试方案。

### 9.3 按键与 LED

PB8/PB9/PB18/PB19 的 GPIO 主机模拟检查不确定 LED 电流、极性、复用冲突或按键默认态。验证板外围需限流并可断开；PB18/PB19 先按 RF 复用风险使用隔离焊盘。电阻值、极性与按键时序待选型/固件规范。

## 10. 保护、测试点与可配置项

USB、Target 信号、外部连接器、电源轨需分别分析 ESD/EOS、误插、短路、反接、热插拔和反向供电路径。保护等级和器件选型未冻结。不得让 PC VBUS、Target Power、Target VTref 或 MCU 电源形成未经分析的倒灌路径。

验证板测试点至少覆盖：VBUS、系统/MCU各电源轨、GND、X32MI/X32MO可测观测方案、可选32 kHz、RST、BOOT、TIO、TCK、USBHS/USBFS D+/D−、SWCLK、SWDIO、TDI、TDO、Target nRESET、UART TX/RX、SWO、VTref 前端输入与 PA4 节点、TARGET_PWR_EN、Target Power 输出、SPI1 SCK/MOSI/MISO/CS、RF 测试节点，以及测试计划需要的 ThreadX tick/运行观察 GPIO。RF 测点不得破坏阻抗或天线性能；需采纳官方测量方法。

对于尚未冻结的电气选项，优先使用串联 0R、焊桥、排针或 DNI 器件，让 SWDIO方向、Target电平、Reset输出、Target供电、PB16–PB21复用与Type-C实验路径可更改。具体值/封装必须由可测性和安全评审确定。测试点列表需与 TEST-001 实板用例逐项对照。

## 11. PCB 布局约束

投 PCB 前必须取得并遵循 WCH CH585M 封装/电源/RF 参考设计；确认 USBHS/USBFS 差分路径和回流、晶振布局、DC-DC 电流回路、RF 天线净空及地平面、Target接口保护与回流、跨电源域隔离、USB与RF/时钟/开关电源串扰。阻抗、间距、净空、层叠和天线尺寸不得在资料缺失时编造。Validation Board 可用于实验但必须保留测量和改值能力。

## 12. Validation Board 与 Product Board Gate

### 信号级默认状态基线

原理图进入逐信号电路设计前，采用以下可审查的安全默认态：主动Target供电输出关闭；所有Target数字电平适配路径默认隔离；VTref缺失/无效时由硬件禁止Probe至Target输出；仅在VTref处于冻结有效范围且方向配置完成后使能路径；仅在VTref有效且收到有效复位请求时才允许`TARGET_nRESET`拉低，默认保持释放。VTref检测输入按ADC保护方案处理，不直接承担电平转换器供电。具体电阻/逻辑极性/输出使能默认态和隔离能力仍须按所选器件数据手册核验并在原理图冻结。

| 信号组 | 逻辑方向 | 上电/复位/固件未初始化安全态 | 使能条件 |
|---|---|---|---|
| TARGET_SWCLK_TCK / TARGET_TDI | Probe to Target | 隔离/高阻 | 硬件确认VTref有效、Target操作已启动且路径已使能 |
| TARGET_SWDIO_TMS | SWD双向；JTAG为Probe至Target | 隔离/高阻 | 硬件确认VTref有效且方向已配置；SWD方向与bit engine同步 |
| TARGET_TDO / TARGET_UART_RX / TARGET_SWO | Target to Probe | Probe输入侧隔离且不加载Target信号 | VTref有效且接收路径已配置 |
| TARGET_UART_TX | Probe to Target | 隔离/高阻 | 硬件确认VTref有效且CDC/UART已启动 |
| TARGET_nRESET | Probe控制，Target电压域 | 释放，不主动拉低 | VTref有效并收到有效复位请求后，在Target域拉低再释放 |
| TARGET_VTREF_ADC | Analog Sense | 输入限流/钳位，按ADC安全范围 | 采样前完成ADC初始化 |
| TARGET_PWR_EN | 内部控制 | 默认关闭 | 明确供电请求且保护状态有效 |

### 原理图开始前

- 冻结信号名、方向、MCU资源、电源域与本节安全默认态；MCU-001 引脚/网络基线纳入原理图，PB23/RST 与 PB22/BOOT 分网。
- 明确验证板范围、供电来源和禁止连接条件；所有未确定的电气数值和器件标记 TBD/DNI。
- 建立 CH585M core、USBHS、USBFS恢复、Target电平适配接口、测试点页；测试入口不得形成绕过电平适配器的 Target 工作通路。

### 可以边画边补证据

- USB描述符/端点；USBHS 与 USBFS 并发；Type-C Target 自定义接口映射；VTref 精度；电平转换器选择；Target Power；RF复用与时钟；SPI NOR型号/容量。
- 以上项必须以模块化、可隔离、可替换方式绘制；不得因此误报冻结。

### PCB Layout 前必须完成

- WCH 电源、去耦、晶振、USB、RF 参考设计逐页核对并标来源。
- 目标电压前端与掉电/反灌分析；Target Power 电源树；恢复路径可访问性。
- PB16–PB21 RF 复用决策；天线/匹配网络来源；USB、晶振、DC-DC、RF布局约束。
- Type-C连接器若承载自定义Target信号，须先有规范许可、线缆导体证据、正反插设计和测试计划；否则不得布该自定义接口。

### 投板前必须完成

- 原理图 ERC/审查、器件封装核对、电源/保护计算、测试点清单、DNI/0R状态、恢复入口演练方案、风险与未决项评审。
- 明确所有“未验证”模块对应的安全边界和实验接线，不把高风险未知电压直接接入芯片或 Target。

### V1 产品硬件冻结前必须完成

- TEST-001 的 ThreadX启动/tick/中断/上下文/睡眠唤醒实板项通过；USB、Target接口、SWD/JTAG/UART、VTref/电源、RF和恢复相关验收有记录。
- 所有产品必需接口的电气规格、线缆/连接器兼容范围、BOM、PCB布局、EMC/ESD和异常恢复完成评审。
- OPEN-001 中产品冻结阻塞项关闭或由正式变更决策接受；RISK-001 高风险项有证据和处置结果。

## 13. 必须由实板/外部资料关闭的问题

| 事项 | 所需证据 |
|---|---|
| CH585M各电源网/晶振/去耦最终连接 | 手册原文页码、WCH参考电路核对、实际器件参数和电源测量 |
| BOOT/ISP/USBFS/UART Recovery | WCH正式进入说明、验证板入口时序和恢复演练记录 |
| USBHS/USBFS组合与USB-C Device电路 | 控制器资源/官方电路、枚举和恢复实测 |
| Type-C Target自定义触点和线材 | USB-IF适用规范审查、具体线缆BOM/结构、双方向导通矩阵、信号测试 |
| 1.8 V/3.3 V/掉电Target兼容 | CH585电气规格、尚未选定的电平转换器规格、上电顺序/反灌/波形实测 |
| VTref量程和精度 | ADC与前端误差预算、校准方式、校准源实测 |
| Target Power路径 | 电源预算、限流/短路/反向隔离分析及负载故障实测 |
| RF复用、天线和时钟 | 实际SDK/库配置、官方匹配/布局资料、RF工作测量 |
| 外部SPI NOR | 最终镜像预算、恢复/回滚语义、器件采购和驱动验证 |
| ESD/EMC/热插拔 | 冻结的测试等级、样机测试记录和故障分析 |

## 14. 参考资料

| 来源 | 版本/日期 | 用途与结论 |
|---|---|---|
| WCH, CH585/CH584 Datasheet | V1.6，仓库版本 | 芯片引脚、电气参数、电源/时钟和外设依据；使用前应核实勘误 |
| WCH CH585EVT archive | 索引 2026.08；`EVT/PUB/CH585SCH.pdf` | 官方示例原理图/软件接口参考，不等于DBG-C冻结电路 |
| USB-IF, USB Type-C Cable and Connector Specification | Release 2.5，2026-04-08 | Type-C线缆/连接器规范；本轮确认其对第三方功能有明确范围约束，需在接口冻结前审阅适用章节 |

外部来源：[USB-IF Type-C Specification Release 2.5](https://www.usb.org/document-library/usb-type-cr-cable-and-connector-specification-release-25)，核对日期 2026-09-30。本文不复制规范表格，线缆导体细节仍待按具体组件核验。
