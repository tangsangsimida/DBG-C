# DBG-C 未决问题与验证清单

**文档编号：** DBG-C-OPEN-001　**版本：** V0.33　**状态：** 开放项

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
| O11 | 目标 UART/RESET/SWD 最大允许电压、保护和 SWD GPIO 工作模式是什么？ | 冻结 Target 兼容范围、电气规范、SWDIO 上下拉/输出驱动/空闲态/方向切换要求与实测方案 | IF, HW, FW, TEST | 待决策；PB5/PB6 模式选择与 PA4 原始电平 GPIO BSP 已完成主机模型和目标对象检查；产品电气模式、复位有效电平映射、默认态、保护和脉宽仍未定义，须由电气规范评审并在验证板测量 |
| O12 | 性能目标：DAP 延迟、吞吐、射程、稳定运行时长？ | 原型数据与产品评审 | PRD, RF, TEST | 待决策 |
| O13 | V1 BLE OTA 仅升级 Probe 自身；是否需要 RF OTA？ | 需求确认及资源/安全评审 | PRD, BLE, RF | 待决策 |
| O14 | PC OS、IDE/OpenOCD/pyOCD 支持矩阵？ | 产品支持策略与逐项互操作测试 | PRD, USB, TEST | 待决策 |
| O15 | 当前数据手册适用修订、勘误与参考手册版本？ | WCH 官方发布页和芯片版本核对 | MCU, HW, FW | 待获取 |
| O16 | BLE 无线下载的应用协议、镜像格式、Target 范围和断点续传规则？ | 定义 DBG-C Tool ↔ Probe 协议并选定 Target 验收板 | PRD, BLE, TEST | 待决策 |
| O17 | USBFS/USBHS 是否可同时运行？V1 已分配 USBFS，什么实现限制会要求改用 USBHS？ | SDK 实例、官方资源限制与对照实测 | MCU, USB, SYS | 待验证 |
| O18 | Eclipse ThreadX RISC-V32/GNU 线程上下文例程及本地 CH585M 低层适配能否正确运行于 QingKe V3C？ | 在 CH585M 板上验证 HPE、PFIC/VTF、启动入口、异常栈帧、SysTick、睡眠唤醒、调度和持续运行 | MCU, FW, TEST | 待实板验证；软件门已放行验证板设计，但当前无可用板卡，芯片运行未执行 |
| O19 | 进入硬件设计前，PoC-1 的“软件验证通过”门槛是什么？ | 采用 O19 放行标准：两次可复现构建、ELF/链接资源检查、startup/ThreadX 上下文与中断路径静态审查；仅放行验证硬件设计，产品冻结须实板验证 | MCU, FW, SYS, TEST | 软件放行门已通过，仅允许设计验证用硬件；tick 目标冲突仍待确认；实板运行与产品硬件冻结未通过 |
| O20 | Arm CMSIS-DAP 固件核心能否由 CH585M 的 WCH RISC-V GCC 编译，需做哪些有依据的编译器/指令集适配？ | 对固定上游提交审查编译器头、内联汇编及端口依赖；完成有依据的适配后验证目标编译和主机命令层，禁止引入 Arm ISA 汇编 | FW, MCU, TEST | 本地 CMSIS 编译器宏适配及编译检查通过；8 项命令层主机检查、9 项 SWD 线模型用例通过；WCH GCC 将测试配置的 `DAP.c`、`SW_DP.c` 编译为 ELF32 RISC-V 对象。产品 HAL、时序校准、USB 接入和固件链接未完成，O20 仍开放 |
| O21 | 如何在调用 CMSIS-DAP 上游命令处理前，验证实际输入长度及响应容量？ | 明确产品启用命令/功能、厂商命令覆盖策略、字符串回调最大写入量、USB 实际收包长度及请求/响应缓冲容量；把有界 dispatch 接入真实产品 USB 收包路径并覆盖产品命令配置 | USB, FW, TEST | 通用预检与有界 dispatch 已实现：102 项预检检查、3 个上游 dispatch 主机用例及 WCH GCC ELF32 RISC-V 对象编译通过；截断输入和容量不足会在上游调用前被拒绝，vendor/SWO/CMSIS-DAP UART 失败关闭。产品命令与 feature profile、Info 最大写入、USB 收包长度/响应容量及 USB 调用接入尚未实现，O21 保持开放；测试夹具不是产品配置 |

## O18 更新证据

- 通用字节 FIFO 的 FIFO-01 至 FIFO-07 主机用例已通过，既有构建入口报告 3683 项断言通过；该结果只覆盖纯 C FIFO 行为，不关闭 O18，也不证明 ThreadX/ISR/芯片运行正确。
- ThreadX 已锁定为 `v6.5.1.202602a_rel`，提交 `b91b03b9e75fa523b17127f9e0eca09dca916459`；MounRiver Linux x64 Toolchain V2.4.0 的 GCC 12.2.0 已安装到当前用户目录。
- PoC-1 有实验性时钟初始化、low-level 内存边界、VTF SysTick 注册和 ThreadX tick ISR；线程读取 `tx_time_get()` 并睡眠一个 tick。主机交叉构建通过，输出为 ELF32 RISC-V，text 8876、data 8、bss 5564 字节。
- Tick 采用 ThreadX 上游头文件默认值 100 tick/s。HPE/VTF 行为、异常栈帧、SysTick 实际频率、tick 投递、睡眠唤醒、调度和长期稳定性都未在板上验证；因此 O18 仍未关闭。
- 用户确认当前没有可用板卡。按两道放行门处理：两次 clean rebuild 的 ELF/map SHA-256 相同，ELF 与链接脚本及启动符号完成静态核对，源码审查已覆盖启动、ThreadX 上下文及中断路径，因此软件门通过，允许进入限定用途的验证硬件设计。此许可不代表 ThreadX 已在 CH585M 上运行验证，也不允许冻结产品原理图或 PCB。源码当前为 100 tick/s，既有需求描述为 1000 tick/s，冲突仍待确认；在定义实板 tick 频率验收门限前必须解决。中断、SysTick、调度、睡眠/唤醒仍未运行验证。
- ThreadX 上游 `qemu_virt` 示例使用 QEMU virt 地址/入口/链接布局，与 PoC 的 WCH CH585 启动和 PFIC/VTF 代码不同；当前环境无 QEMU、Spike、Renode，模拟器运行证据不存在。
- 当前 PoC 实际使用 `TX_TIMER_TICKS_PER_SECOND=100`；在源码声明的 62.4 MHz 下，比较值计算为 623999。用户描述中出现的 1000 tick/s 尚未在源码中实现，须确认后才能修改配置。
- `docs/05-firmware/DBG-C-FW-001.zh-CN.md` 记录 EVT 来源、原始归档散列和主机环境。WCH EVT `.cproject` 只确定 GCC12 选项，未确定 GCC 补丁版本；当前安装工具链实测 GCC 12.2.0。

## O20 更新证据

- 新增 `software/poc1-ch585-threadx/platform/ch585/cmsis_compiler.h`，用 WCH EVT `CH585SFR.h`、`core_riscv.h` 的精确 GNU 宏定义及 GCC/RISC-V 指令基元映射 CMSIS inline、NOP 与 weak-symbol 宏。GCC 12.2.0 成功编译检查对象，ELF 符号表显示检查函数为 `WEAK`。
- WCH GCC 使用该头文件将测试配置下的固定上游 `DAP.c` 与 `SW_DP.c` 编译为 ELF32 RISC-V 对象；测试 pin 仍为空操作宏，SWD 事务模型仍仅用于主机检查。两个对象没有链接进 PoC。
- 上游 `DAP.h` 的 Arm `subs` 延时分支依然需要构建目录副本选择 C 循环；目前没有 SWD GPIO HAL、PB5/PB6 电气/时序校准、DAP USB/ThreadX 集成或产品固件链接。因此这是编译器基元和目标对象编译证据，不关闭 O20，也不代表 CH585M 实板通过。

- 固定 CMSIS-DAP 提交 `12636590eec66fae2d1bba4518749426ad5a4595` 的 8 项主机用例通过，覆盖固件版本 Info、未知 Info 标识符、未实现命令、DP 读的 WAIT 重试、DP 写完成检查、DAP_Transfer 与 DAP_TransferBlock 的 AP 读结果/`DP_RDBUFF` 顺序，以及 TransferConfigure 重试数对后续 DP 读的影响。
- 测试启用 SWD 命令处理分支，但 SWD pin 宏为空操作，`SWD_Transfer()` 由确定性测试桩实现；不调用 GPIO、USB、ThreadX 或 CH585M API。构建目录中的头文件副本只调整延时分支选择条件，没有定义 `__CC_ARM` 或改动第三方子模块。
- WCH RISC-V GCC 将同一 SWD-enabled 测试配置的 `DAP.c` 编译为 ELF32 RISC-V 对象；该对象未链接至 PoC。
- 这些结果不证明 CMSIS-DAP 产品集成、实际 SWD 物理传输、产品配置或 USB 接口。O20 保持开放。

- 本轮新增上游 `SW_DP.c` 位级引擎 host line-model 九项检查共 168 项断言，覆盖读/写、WAIT/FAULT、10 位 SWD 输入/输出序列、DAP_Connect + DP IDCODE 读取，以及 AP DAP_Transfer / 两项 AP DAP_TransferBlock posted-read 经 DP_RDBUFF 返回数据的集成路径和 DAP_SWD_Sequence 混合输入输出序列命令；fast 延时为空操作，计数不代表频率/时序。WCH RISC-V GCC 将 `SW_DP.c` 编译为 ELF32 RISC-V 对象，未链接产品固件。
- WCH EVT GPIO 头文件/实现中静态核对到 `GPIOB_ModeCfg`、`GPIOB_SetBits`、`GPIOB_ResetBits`、`GPIOB_ReadPortPin`。这些 API 尚未集成编译或连接到 SWD HAL；PB5/PB6 电气、时序和保护仍待 IF-001 与验证板测量。
- O20 保持开放；上述 host 模型与对象编译不证明实际 SWD、GPIO、ThreadX、USB 或 CH585M 运行。

## O19 放行标准与判定

## 验证板设计放行门

进入限定用途的**验证硬件设计**前，必须满足以下可复核条件：

1. 固定源码/子模块修订和工具链版本；执行至少两次干净构建，均生成 ELF 和 map，二者 SHA-256 完全一致。对尚未决策的功能配置作显式记录，不将其伪装成已冻结要求。
2. 检查 ELF 架构、端序、入口、启动符号、段装载地址/运行地址；与实际 startup 和 linker script 对照，所有静态段都落在脚本声明的 Flash/RAM 范围内。记录 text/data/bss 和已静态分配栈/缓冲占用。本 PoC 静态声明两个 1024-byte 应用线程栈、2048-byte ISR 栈和 512-byte ThreadX timer stack；ELF 的 data 为 8 byte、bss 为 5564 byte。该记录不包含实板栈水位结论。
3. 静态审查 reset/时钟初始化、ThreadX 上下文保存/恢复、异常栈帧和栈切换、MIE/HPE/PFIC/VTF 配置、SysTick 装载值/状态清除及 ISR 到调度器的调用顺序。不得存在已发现且未处置的源码、符号或链接矛盾。
4. 区分静态可判定项和芯片运行项。无法由源码/ELF证明的硬件异常语义、实际频率、中断投递、tick、调度、睡眠唤醒，列为实板测试项，不得记通过。
5. 验证板设计输入须覆盖编程/恢复、复位、系统时钟、SysTick、中断活动的观察/测量路径，并为调试和测量保留接入点；具体引脚和电路要从正式资料确认。

**当前判定：验证板设计门已通过；PoC-1 的 CH585M 运行验证未通过，状态为未执行；产品硬件冻结门未通过。** 已有两次一致的干净构建、ELF/链接资源检查和源码静态审查，因此可开展限定用途的最小验证板设计。验证板必须具备编程/恢复路径，并能观测复位、系统时钟、SysTick 和中断活动；设计细节仍须依据正式芯片资料和测试方案。不得将其称为冻结的产品原理图/PCB。主机交叉构建只证明目标 ELF 可构建与静态检查，不证明 CH585M 中断、SysTick 投递、线程切换或调度正确。当前 tick 配置与需求文字冲突不会阻止验证板设计，但必须在冻结实板 tick 频率验收门限前解决。

产品硬件冻结前，必须有 CH585M 实板记录证明 ThreadX 启动、线程运行/切换/睡眠/唤醒、实测 SysTick 频率与 tick 投递、中断进出与上下文恢复/栈完整性、复位/睡眠唤醒及持续运行达到预先冻结的时长、重复次数、负载和通过门限。实板未测项保持未执行；板测之前不得关闭 O18。当前 100 tick/s 源码配置与 1000 tick/s 需求描述的冲突仍待决策，须在实板验收方案冻结目标频率前解决。

## 当前确认边界

- 仓库包含 `docs/09-references/CH585-CH584_Datasheet_V1.6.pdf`、`docs/09-references/CH585EVT/CH585EVT.ZIP` 归档及 PoC-1 使用的少量 EVT 启动、链接和头文件副本；没有独立 CH585 参考手册、DBG-C 原理图、板级测试记录或抓包。
- EVT 索引日期为 2026.08，FreeRTOS 示例使用 FreeRTOS-Kernel V11.3.0；资料没有声明独立 WCH SDK 语义版本。已摘录 API/寄存器仅用于 ThreadX PoC，不代表 RF/BLE/USB 资源并发可行或已通过板级验证。
- DBG-C Interface 的 Pin 映射、RF 帧字段、USB 描述符、角色切换规则均未冻结。
- 用户补充明确了 BLE 无 Dongle 目标下载和 USBFS 优先的产品方向；前者仍缺应用协议与目标范围，后者已经确定为 V1 USB 分配；端点和 SDK Device 栈兼容性仍需验证。
