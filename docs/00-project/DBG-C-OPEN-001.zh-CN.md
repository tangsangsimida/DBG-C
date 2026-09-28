# DBG-C 未决问题与验证清单

**文档编号：** DBG-C-OPEN-001　**版本：** V0.12　**状态：** 开放项

| ID | 问题 | 需要的证据/决策 | 影响文档 | 状态 |
|---|---|---|---|---|
| O01 | CH585M 可用私有 RF PHY/API、BLE 与私有 RF 并发限制是什么？ | WCH 官方参考手册、SDK/例程版本及实测 | MCU, RF, FW, TEST | 待获取 |
| O02 | WCH SDK 的 USBFS Device 栈能否提供 CMSIS-DAP v2 Bulk + CDC 所需端点与缓冲？ | CH585M USBFS Device SDK 示例、端点 API、Host OS 实测 | MCU, USB, SYS | 待验证 |
| O03 | RF DAP 往返延迟和重试边界如何满足常见调试器？ | CMSIS-DAP Host 实测及 RF 原型 | RF, TEST, PRD | 待验证 |
| O04 | DBG-C Basic/Full 在 Type-C 上的合法/可靠引脚及线缆方案？ | USB-IF 最新 Type-C 规范、线缆结构证据、电气评审 | IF, HW, TEST | 待研究 |
| O05 | USB 插入、用户选择、无线连接如何决定 Standalone/Host/Target？ | 产品状态机评审，含冲突/切换规则 | PRD, SYS, RF, BLE | 待决策 |
| O06 | Pairing 是否持久化、是否自动配对、解绑后行为？ | 安全/使用流程评审与持久化测试 | PRD, RF, BLE | 待决策 |
| O07 | Device ID 的实际来源、长度、读取 API 和认证绑定？ | CH585M SDK/官方接口及安全评审 | MCU, RF, BLE | 待验证 |
| O08 | USB VID/PID、接口/端点布局、字符串、Serial 策略？ | 正式实现及组织 VID 决策 | USB, TEST | 待决策 |
| O09 | CDC UART 波特率、流控、目标电平和性能门限？ | 用户需求、电气设计及测量 | PRD, IF, TEST | 待决策 |
| O10 | OTA 是否支持签名、双镜像、回滚及断电恢复？ | WCH Boot/SDK 文档、示例和断电测试 | MCU, FW, BLE, RF, RISK | 待验证 |
| O11 | 目标 UART/RESET/SWD 最大允许电压和保护？ | Target 兼容范围、电气规范与测试 | IF, HW, TEST | 待决策 |
| O12 | 性能目标：DAP 延迟、吞吐、射程、稳定运行时长？ | 原型数据与产品评审 | PRD, RF, TEST | 待决策 |
| O13 | V1 BLE OTA 仅升级 Probe 自身；是否需要 RF OTA？ | 需求确认及资源/安全评审 | PRD, BLE, RF | 待决策 |
| O14 | PC OS、IDE/OpenOCD/pyOCD 支持矩阵？ | 产品支持策略与逐项互操作测试 | PRD, USB, TEST | 待决策 |
| O15 | 当前数据手册适用修订、勘误与参考手册版本？ | WCH 官方发布页和芯片版本核对 | MCU, HW, FW | 待获取 |
| O16 | BLE 无线下载的应用协议、镜像格式、Target 范围和断点续传规则？ | 定义 DBG-C Tool ↔ Probe 协议并选定 Target 验收板 | PRD, BLE, TEST | 待决策 |
| O17 | USBFS/USBHS 是否可同时运行？V1 已分配 USBFS，什么实现限制会要求改用 USBHS？ | SDK 实例、官方资源限制与对照实测 | MCU, USB, SYS | 待验证 |
| O18 | Eclipse ThreadX RISC-V32/GNU 线程上下文例程及本地 CH585M 低层适配能否正确运行于 QingKe V3C？ | 在 CH585M 板上验证 HPE、PFIC/VTF、启动入口、异常栈帧、SysTick、睡眠唤醒、调度和持续运行 | MCU, FW, TEST | 待实板验证；软件门已放行验证板设计，但当前无可用板卡，芯片运行未执行 |
| O19 | 进入硬件设计前，PoC-1 的“软件验证通过”门槛是什么？ | 采用 O19 放行标准：两次可复现构建、ELF/链接资源检查、startup/ThreadX 上下文与中断路径静态审查；仅放行验证硬件设计，产品冻结须实板验证 | MCU, FW, SYS, TEST | 软件放行门已通过，仅允许设计验证用硬件；tick 目标冲突仍待确认；实板运行与产品硬件冻结未通过 |

## O18 更新证据

- 通用字节 FIFO 的 FIFO-01 至 FIFO-07 主机用例已通过，既有构建入口报告 3683 项断言通过；该结果只覆盖纯 C FIFO 行为，不关闭 O18，也不证明 ThreadX/ISR/芯片运行正确。
- ThreadX 已锁定为 `v6.5.1.202602a_rel`，提交 `b91b03b9e75fa523b17127f9e0eca09dca916459`；MounRiver Linux x64 Toolchain V2.4.0 的 GCC 12.2.0 已安装到当前用户目录。
- PoC-1 有实验性时钟初始化、low-level 内存边界、VTF SysTick 注册和 ThreadX tick ISR；线程读取 `tx_time_get()` 并睡眠一个 tick。主机交叉构建通过，输出为 ELF32 RISC-V，text 8876、data 8、bss 5564 字节。
- Tick 采用 ThreadX 上游头文件默认值 100 tick/s。HPE/VTF 行为、异常栈帧、SysTick 实际频率、tick 投递、睡眠唤醒、调度和长期稳定性都未在板上验证；因此 O18 仍未关闭。
- 用户确认当前没有可用板卡。按两道放行门处理：两次 clean rebuild 的 ELF/map SHA-256 相同，ELF 与链接脚本及启动符号完成静态核对，源码审查已覆盖启动、ThreadX 上下文及中断路径，因此软件门通过，允许进入限定用途的验证硬件设计。此许可不代表 ThreadX 已在 CH585M 上运行验证，也不允许冻结产品原理图或 PCB。源码当前为 100 tick/s，既有需求描述为 1000 tick/s，冲突仍待确认；在定义实板 tick 频率验收门限前必须解决。中断、SysTick、调度、睡眠/唤醒仍未运行验证。
- ThreadX 上游 `qemu_virt` 示例使用 QEMU virt 地址/入口/链接布局，与 PoC 的 WCH CH585 启动和 PFIC/VTF 代码不同；当前环境无 QEMU、Spike、Renode，模拟器运行证据不存在。
- 当前 PoC 实际使用 `TX_TIMER_TICKS_PER_SECOND=100`；在源码声明的 62.4 MHz 下，比较值计算为 623999。用户描述中出现的 1000 tick/s 尚未在源码中实现，须确认后才能修改配置。
- `docs/05-firmware/DBG-C-FW-001.zh-CN.md` 记录 EVT 来源、原始归档散列和主机环境。WCH EVT `.cproject` 只确定 GCC12 选项，未确定 GCC 补丁版本；当前安装工具链实测 GCC 12.2.0。

## O19 放行标准与判定

## 验证板设计放行门

进入限定用途的**验证硬件设计**前，必须满足以下可复核条件：

1. 固定源码/子模块修订和工具链版本；执行至少两次干净构建，均生成 ELF 和 map，二者 SHA-256 完全一致。对尚未决策的功能配置作显式记录，不将其伪装成已冻结要求。
2. 检查 ELF 架构、端序、入口、启动符号、段装载地址/运行地址；与实际 startup 和 linker script 对照，所有静态段都落在脚本声明的 Flash/RAM 范围内。记录 text/data/bss 和已静态分配栈/缓冲占用。本 PoC 静态声明两个 1024-byte 应用线程栈、2048-byte ISR 栈和 512-byte ThreadX timer stack；ELF 的 data 为 8 byte、bss 为 5564 byte。该记录不包含实板栈水位结论。
3. 静态审查 reset/时钟初始化、ThreadX 上下文保存/恢复、异常栈帧和栈切换、MIE/HPE/PFIC/VTF 配置、SysTick 装载值/状态清除及 ISR 到调度器的调用顺序。不得存在已发现且未处置的源码、符号或链接矛盾。
4. 区分静态可判定项和芯片运行项。无法由源码/ELF证明的硬件异常语义、实际频率、中断投递、tick、调度、睡眠唤醒，列为实板测试项，不得记通过。
5. 验证板设计输入须覆盖编程/恢复、复位、系统时钟、SysTick、中断活动的观察/测量路径，并为调试和测量保留接入点；具体引脚和电路要从正式资料确认。

**当前判定：验证板设计门已通过，产品硬件冻结门未通过。** 已有两次一致的干净构建、ELF/链接资源检查和源码静态审查，因此可开展限定用途的最小验证板设计。验证板必须具备编程/恢复路径，并能观测复位、系统时钟、SysTick 和中断活动；设计细节仍须依据正式芯片资料和测试方案。不得将其称为冻结的产品原理图/PCB。

产品硬件冻结前，必须有 CH585M 实板记录证明 ThreadX 启动、线程运行/切换/睡眠/唤醒、实测 SysTick 频率与 tick 投递、中断进出与上下文恢复/栈完整性、复位/睡眠唤醒及持续运行达到预先冻结的时长、重复次数、负载和通过门限。实板未测项保持未执行；板测之前不得关闭 O18。当前 100 tick/s 源码配置与 1000 tick/s 需求描述的冲突仍待决策，须在实板验收方案冻结目标频率前解决。

## 当前确认边界

- 仓库包含 `docs/09-references/CH585-CH584_Datasheet_V1.6.pdf`、`docs/09-references/CH585EVT/CH585EVT.ZIP` 归档及 PoC-1 使用的少量 EVT 启动、链接和头文件副本；没有独立 CH585 参考手册、DBG-C 原理图、板级测试记录或抓包。
- EVT 索引日期为 2026.08，FreeRTOS 示例使用 FreeRTOS-Kernel V11.3.0；资料没有声明独立 WCH SDK 语义版本。已摘录 API/寄存器仅用于 ThreadX PoC，不代表 RF/BLE/USB 资源并发可行或已通过板级验证。
- DBG-C Interface 的 Pin 映射、RF 帧字段、USB 描述符、角色切换规则均未冻结。
- 用户补充明确了 BLE 无 Dongle 目标下载和 USBFS 优先的产品方向；前者仍缺应用协议与目标范围，后者已经确定为 V1 USB 分配；端点和 SDK Device 栈兼容性仍需验证。
