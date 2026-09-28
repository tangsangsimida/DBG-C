# DBG-C Verification Specification

**文档编号：** DBG-C-TEST-001　**版本：** V0.36　**状态：** 测试计划草案；FIFO、单向/双向字节流桥接、CMSIS-DAP 命令层、有界 dispatch、SWD 引擎主机模型、边界预检器、CMSIS 编译器映射及 CH585 SWD GPIO、Target Reset GPIO 与 UART0 模型/目标对象检查已执行；产品 UART/命令边界与 ThreadX 实板测试未执行

## 1. 通过规则

每例记录 DUT 硬件/固件版本、Host OS/工具版本、Target 板/芯片、接线/线缆、环境、步骤、期望、实测、日志/抓包及 Pass/Fail/Blocked。未执行写“未执行”，无证据不得标 Pass。定量门限待相关规范冻结。

## 2. USB

验证枚举、描述符、CMSIS-DAP v2 传输、CDC Control/Data、热插拔/重连、多个 DBG-C 同连与实例区分；覆盖 Windows/WinUSB/CDC、Linux/udev、macOS。VID/PID、端点、包长和描述符待 USB 规范/实现冻结。

## 3. SWD 与 Target

覆盖 Connect、ID 读取、内存读写、擦除、Program/Verify、Reset、断点、单步、寄存器和内存访问。至少 STM32、GD32 各选定具体型号和板卡；型号待测试夹具冻结。检查 SWD 频率范围、信号质量、目标掉电/插拔与错误返回。

## 4. UART

验证 RX、TX、全双工、多波特率、长时间高流量、溢出/流控策略，以及无线 Debug/UART 并发；具体波特率和吞吐限值待评审。

## 5. 2.4G

配对/解绑/重配、角色建立、重连；屏蔽/干扰/距离变化、丢包/重复/乱序/断电；测时延分布、吞吐、重试及错误率；下载、在线 Debug、UART 并发与连续运行。协议冻结后按帧字段、CRC/认证及版本兼容做边界/畸形帧测试。

## 6. BLE

Discovery、设备信息、Pair/Unpair、配置、状态、OTA、中断恢复、兼容性和权限测试。覆盖指定 PC 蓝牙适配器/OS 矩阵，矩阵待冻结。

## 7. 压力与故障注入

连续烧录/Debug、RF 长时间运行、USB 插拔、Target 插拔、RF 断连、Target 掉电、DBG-C 重启；检查无静默伪成功、无危险重复命令、能恢复到定义状态。时长、次数、门限待冻结。

## 8. 通用字节 FIFO 主机测试计划

测试对象为 `software/common/byte_fifo/`，按其公开头文件定义的函数契约测试；只验证纯 C 数据行为，不覆盖并发、ISR、USB/UART/RF 或 MCU 实际 SRAM。

| 用例 | 检查内容 | 通过条件 | 状态 |
|---|---|---|---|
| FIFO-01 初始化边界 | Null FIFO、Null 存储、零容量、容量 1 初始化与单字节满载/拒写/读回、非 2 的幂容量 | 无效输入返回非零且对象保持不可用；有效容量初始化成功；容量 1 满载后拒绝额外写入并正确读回 | 通过 |
| FIFO-02 写入与满载 | 空 FIFO 写入、恰好写满、超容量写入、继续写入 | 返回接受字节数；容量受限时允许部分写；不得覆盖已有未读字节 | 通过 |
| FIFO-03 读取与空载 | 空 FIFO 读取、少量读取、读取至空 | 返回实际读取数，字节顺序与写入一致；空 FIFO 不改变状态 | 通过 |
| FIFO-04 回绕 | 使用非 2 的幂容量，安排读写操作使读写索引均回绕 | 回绕后数据顺序和计数正确，存储区两侧保护字节保持不变 | 通过 |
| FIFO-05 无效缓冲参数 | 长度非零而读写数据指针为空 | 返回 0，FIFO 状态不变 | 通过 |
| FIFO-06 清空与查询 | 清空非空/已空 FIFO；查询容量、占用和剩余空间；零长度读写 | 查询与实际状态一致；清空丢弃占用但保留容量；零长度操作不改变状态 | 通过 |
| FIFO-07 确定性状态序列 | 7 字节容量下执行 512 轮确定性写入、读取、查询与周期性清空 | 每步读写量、顺序、占用/剩余空间均与参考队列一致；存储区保护字节不变 | 通过 |

本 FIFO 不具备并发安全保证，因此不定义多线程/ISR 并发通过项。七组主机用例已接入既有 `software/poc1-ch585-threadx/build.sh`；运行结果为 `PASS: 3683 byte FIFO checks`，命令与工具链信息记录在 `software/poc1-ch585-threadx/build-evidence.log`。测试只证明当前主机上的纯 C FIFO 数据行为，不证明 ThreadX 并发、ISR 安全、CH585M SRAM 或实板行为。

## 8.1 无调度字节流桥接主机检查

测试对象为 `software/common/byte_stream_bridge/`。它复用通用字节 FIFO，以显式单字节回调代表源端和写端；每次服务分别受最大转发字节数限制。测试不使用 USB、UART 寄存器、中断或 ThreadX。

| 用例 | 检查内容 | 通过条件 | 状态 |
|---|---|---|---|
| BYTE-BRIDGE-HOST-01 | 测试源端到写端的顺序、服务限额、写端背压时的待发字节和 FIFO 缓存、源端/写端错误、零限额及无效初始化参数 | 15 项回调模型检查通过；未接受的字节仍保留在通道或 FIFO | 通过（仅主机回调模型） |

该通道是可由调用方轮询的单执行上下文原语；它不提供并发保护，也未接入 CDC/UART 产品路径。目标静态库随现有 PoC 构建编译，但 PoC 应用未调用该通道。主机检查不证明 USB CDC 或 UART 实际行为、持续背压时的产品吞吐及 ThreadX/ISR 集成。

复现命令（仓库根目录执行）：`DBGC_BUILD_DIR=build/byte-stream-bridge software/poc1-ch585-threadx/build.sh`。

### 8.2 双向无调度字节流桥接主机检查

测试对象为 `software/common/byte_duplex_bridge/`，将两路既有单向字节流通道组合起来。端点均为测试回调；两个方向使用独立 FIFO 和调用方给定的服务预算。服务按固定顺序处理两个方向，单路回压不阻止另一方向继续；模块不提供调度、并发保护或 USB/UART 驱动。

| 用例 | 检查内容 | 通过条件 | 状态 |
|---|---|---|---|
| BYTE-DUPLEX-HOST-01 | 检查双向初始化、两个独立预算、两路数据顺序、单向写回压与恢复、零预算、首方向端点错误快速返回、空对象和共用 FIFO 拒绝 | 15 项回调模型检查通过 | 通过（仅主机回调模型） |

复现命令（仓库根目录执行）：`DBGC_BUILD_DIR=build/duplex-bridge software/poc1-ch585-threadx/build.sh`。该命令也交叉构建 ThreadX PoC，但 PoC 应用不调用双向桥接服务；构建和主机模型均不证明 CH585M 中断、调度、USB CDC、UART、电气或实板行为。


## 9. CMSIS-DAP 命令核心主机检查

测试对象为固定上游提交的 `Firmware/Source/DAP.c`，通过 `DAP_ExecuteCommand()` 调用。配置位于 `software/common/cmsis_dap_host_test/`，不是 DBG-C 产品配置；SWD 命令处理开启，JTAG 关闭。SWD pin 操作是空操作，底层 `SWD_Transfer()` 由测试桩模拟。测试构建只在 `software/poc1-ch585-threadx/build/host-tests/` 复制并修改 `DAP.h` 延时分支选择条件，没有定义 `__CC_ARM`，也不改动第三方子模块。

| 用例 | 请求/检查 | 通过条件 | 状态 |
|---|---|---|---|
| DAP-HOST-01 | `ID_DAP_Info` 查询 `DAP_ID_DAP_FW_VER` | 响应命令、长度和固件版本字符串与固定上游实现一致 | 通过 |
| DAP-HOST-02 | `ID_DAP_Info` 查询未定义的信息标识符 `0xA0` | 响应长度为 2，信息长度为 0 | 通过 |
| DAP-HOST-03 | 发送未实现命令 `0x30` | 返回 `ID_DAP_Invalid`，响应长度为 1 | 通过 |
| DAP-HOST-04 | 执行单次 DP 读，模拟桩先返回两次 WAIT 再返回 OK 和数据 | 上游命令层重试两次，返回计数、状态及小端数据正确 | 通过 |
| DAP-HOST-05 | 执行 DP 写并检查写完成读回请求 | 写入值传给事务桩，随后发出 `DP_RDBUFF | DAP_TRANSFER_RnW`，响应计数与状态正确 | 通过 |
| DAP-HOST-06 | 连续执行两次 AP 读，事务桩为 posted read 与 RDBUFF 返回不同数据 | 两个数据按请求顺序返回；调用序列包含两次 AP 读和末尾 `DP_RDBUFF | DAP_TRANSFER_RnW` | 通过 |
| DAP-HOST-07 | 先配置 `retry_count=1`，再模拟连续两次 WAIT 的 DP 读 | Configure 响应为 OK；后续读只执行一次重试，返回 WAIT，事务桩调用两次 | 通过 |
| DAP-HOST-08 | 执行含两个读操作的 AP `ID_DAP_TransferBlock` | 返回两个按请求顺序排列的小端数据；事务序列以 `DP_RDBUFF | DAP_TRANSFER_RnW` 收尾 | 通过 |

这八项验证固定上游命令层，包括 TransferConfigure 的 retry_count 生效、TransferBlock 的 AP posted-read 处理、由桩模拟物理事务边界的 DP/AP 读写及 WAIT 重试；它们不验证 GPIO、SWD 电气时序、CMSIS-DAP v2 USB 传输或 CH585M 行为。有界 dispatch 主机用例另见第 10.2 节。构建脚本使用 WCH RISC-V GCC 将启用 SWD 命令分支的测试配置编译为 ELF32 RISC-V 对象；引脚宏为空操作，事务函数为测试桩，该对象未链接到 PoC，也未在 CH585M 上运行。它们不能替代 TEST-001 第 2–7 节的产品级用例。

## 9.1 CMSIS-DAP 上游 SWD 引擎主机线模型

测试对象为固定上游 `Firmware/Source/SW_DP.c` 和配套 `DAP.c`。测试 pin 宏通过回调连接到纯主机位流模型；模型提供 ACK、数据和奇偶校验输入，并记录输出位、时钟置高调用及方向切换。模型 fast 延时为空操作，调用计数不是时钟频率或时序测量。

| 用例 | 检查内容 | 通过条件 | 状态 |
|---|---|---|---|
| SWD-ENG-01 | SWD 读请求、ACK=OK、32 位数据与正确奇偶校验 | 请求位序、返回数据、方向切换、最终时钟线状态和模型调用计数符合夹具预期 | 通过 |
| SWD-ENG-02 | SWD 读返回错误奇偶校验 | 引擎报告 `DAP_TRANSFER_ERROR`，捕获数据、方向切换和模型调用计数符合夹具预期 | 通过 |
| SWD-ENG-03 | SWD 写入 `0x89ABCDEF` | 请求位序、32 位低位优先数据、数据奇偶校验、方向切换及模型调用计数符合夹具预期 | 通过 |
| SWD-ENG-04 | SWD 读返回 WAIT 与 FAULT ACK | 两种 ACK 均原样返回；模型输入消耗、方向切换、空闲线状态和模型调用计数符合夹具预期 | 通过 |
| SWD-ENG-05 | 10 位 SWD 输出序列及输入序列 | 输出按最低位先行；输入按上游算法打包至两个字节；位数和模型时钟调用数符合预期 | 通过 |
| SWD-ENG-06 | 经 `DAP_Connect` 建立 SWD 端口后，发送单次 DP IDCODE `DAP_Transfer` 读 | 上游命令层与位级引擎端到端模型返回长度、命令、传输状态及小端数据正确 | 通过 |
| SWD-ENG-07 | 经 `DAP_Connect` 建立 SWD 端口后，发送单次 AP `DAP_Transfer` 读，分别模拟 AP posted 数据与后续 RDBUFF 数据 | 两次 SWD 事务均被模型消费；DAP 响应返回 RDBUFF 阶段的数据，长度、状态和小端序正确 | 通过 |
| SWD-ENG-08 | 经 `DAP_Connect` 建立 SWD 端口后，执行两项 AP `DAP_TransferBlock` 读 | 模型消费初始 posted read、后续 AP 读和末项 RDBUFF 三个 SWD 事务；DAP 按小端序返回两项数据及正确计数/状态 | 通过 |
| SWD-ENG-09 | 执行包含 10 位 SWD 输出及 10 位输入序列的 `DAP_SWD_Sequence` 命令 | 返回命令长度与成功状态；输出位序、输入字节打包、模型时钟次数及方向切换调用符合预期 | 通过 |

另由 WCH RISC-V GCC 将测试配置下的上游 `SW_DP.c` 编译为 ELF32 RISC-V 对象。九项主机用例共 168 项断言；它们与目标对象编译均不使用 WCH GPIO、USB、ThreadX 或 CH585M；对象未链接至产品固件。它们验证固定上游位级算法在模拟线模型中的逻辑，不验证 PB5/PB6 波形、电平、SWD 频率、建立/保持时间或 Target 电气行为。

## 10. 当前执行状态

当前仓库包含 CH585M 数据手册、CH585EVT 压缩包及 ThreadX PoC-1。PoC 已有主机交叉构建日志，详见 FW-001；用户确认目前没有可用硬件，故无 CH585M 下载/调试及运行证据。DBG-C 产品固件、线缆样品或抓包也未提供。本文列出的产品级验证用例均未执行；FIFO 七组主机用例、八项 CMSIS-DAP 命令核心检查、五项 CMSIS-DAP 边界预检/dispatch 主机用例、九项 CMSIS-DAP 命令到 SWD 引擎集成线模型检查、两项 CMSIS 编译器宏目标对象检查、SWD GPIO BSP 模拟寄存器 57 项、Target Reset GPIO BSP 模拟寄存器 33 项、UART0 BSP/回调适配器模拟寄存器 89 项及单向字节流桥接 15 项、双向字节流桥接 15 项主机检查及四项 BSP/适配器目标对象编译检查已通过，但均不计为板级或产品功能测试，PoC 交叉构建也不计为实板运行证据。构建、ELF/链接检查和源码静态审查已完成，验证板设计门已通过，可设计限定用途的验证板。该门要求可复现的干净构建、ELF/链接/资源检查，以及对启动、上下文和中断路径的静态审查；仅允许设计验证板，不放行产品原理图或 PCB 冻结。实板运行仍未执行，产品硬件冻结未放行。ThreadX 启动、中断进入/退出、SysTick 频率与 tick 投递、线程切换/睡眠/唤醒、栈完整性、复位恢复和持续运行项目均保持未执行，须在具体测试方案中预先定义时长、重复次数、负载和通过门限。软件放行与实板门见 OPEN-001 O18/O19。本文件定义覆盖面，不构成实板验证报告。

### 10.1 CMSIS 与 CH585 GPIO 主机/目标检查

| 用例 | 检查内容 | 通过条件 | 状态 |
|---|---|---|---|
| CMSIS-COMP-01 | WCH RISC-V GCC 12.2.0 编译 `platform/ch585/cmsis_compiler.h` 检查对象，并查看 ELF 符号表和反汇编 | 对象为 ELF32 RISC-V；弱符号检查函数标记为 `WEAK`；反汇编包含 RISC-V `nop` | 通过 |
| CMSIS-COMP-02 | 用 WCH RISC-V GCC 和该编译器头编译测试配置下的固定上游 `DAP.c`、`SW_DP.c` | 两个独立对象均成功生成为 ELF32 RISC-V；无 Arm ISA 汇编进入对象编译路径 | 通过 |
| CH585-GPIO-HOST-01 | 以模拟寄存器编译并运行 `dbgc_ch585_swd_gpio.c`，检查 SWDIO/SWCLK 的所有模式、读写和错误参数 | 模拟寄存器位变化符合 WCH EVT GPIOB 模式实现；57 项检查通过 | 通过（仅主机寄存器模型） |
| CH585-GPIO-OBJ-01 | 用 WCH RISC-V GCC 编译 `platform/ch585/dbgc_ch585_swd_gpio.c` 并检查对象 ELF 头 | `-Werror` 编译成功，目标对象为 ELF32 RISC-V | 通过（仅目标对象编译） |
| CH585-RESET-HOST-01 | 以模拟寄存器编译并运行 Target Reset GPIO BSP，检查五种模式、原始电平读写及错误参数 | PA4 模拟寄存器位变化符合 WCH EVT GPIOA 实现；33 项检查通过 | 通过（仅主机寄存器模型） |
| CH585-RESET-OBJ-01 | 用 WCH RISC-V GCC 编译 `platform/ch585/dbgc_ch585_target_reset_gpio.c` 并检查对象 ELF 头 | `-Werror` 编译成功，目标对象为 ELF32 RISC-V | 通过（仅目标对象编译） |

SWD GPIO 与 Target Reset GPIO 主机检查分别以普通变量模拟 GPIOB/GPIOA 寄存器，验证 BSP 源码的位操作；不能证明 CH585M 实际寄存器、引脚、电气、目标复位效果或时序。Target Reset GPIO API 传递原始高/低电平，不实现复位极性映射或脉冲策略。目标对象检查不运行对象；CMSIS-DAP 与 GPIO/UART BSP 对象均未链接到 PoC，也不证明产品 DAP 配置、USB、ThreadX 或 CH585M 实板行为。为编译 CMSIS-DAP，构建脚本只在构建目录的 `DAP.h` 副本中选择上游 C 循环延时分支；该 C 循环的 SWD 时序尚未在实板校准。

复现命令（仓库根目录执行）：`DBGC_BUILD_DIR=build/uart0-bridge-adapter software/poc1-ch585-threadx/build.sh`。该现有构建入口运行 SWD GPIO、Target Reset GPIO 和 UART0 主机寄存器模型，并将三个 BSP 与 UART0 桥接适配器编译为 RISC-V 目标对象；这些对象均未链接到 PoC。

### CH585-UART0 主机寄存器模型与目标对象检查

| 用例 | 检查内容 | 通过条件 | 状态 |
|---|---|---|---|
| CH585-UART0-HOST-01 | 以模拟寄存器编译并运行 UART0 BSP；检查显式时钟/波特率除数、PB4/PB7 模式和复用、FIFO 四种触发值、LCR 40 种合法字段组合及无效输入、WCH EVT 除数纯函数 8 项检查、IER/DIV 初始化、UART0 桥接回调映射、非阻塞收发及无效参数 | 89 项检查符合 WCH EVT GPIO/UART 源码和本地 SFR 定义；错误初始化参数在寄存器写入前拒绝 | 通过（仅主机寄存器模型） |
| CH585-UART0-OBJ-01 | 用 WCH RISC-V GCC 编译 `platform/ch585/dbgc_ch585_uart0.c` 并检查对象 ELF 头 | `-Werror` 编译成功，目标对象为 ELF32 RISC-V | 通过（仅目标对象编译） |
| CH585-UART0-ADAPTER-OBJ-01 | 用 WCH RISC-V GCC 编译 UART0 桥接回调适配器并检查对象 ELF 头 | -Werror 编译成功，目标对象为 ELF32 RISC-V；对象不执行，也未链接进产品固件 | 通过（仅目标对象编译） |

寄存器模型不模拟 UART 硬件 FIFO 的读出副作用，因此仅检查寄存器选择/写入、接收数据读取路径和满/空分支；不证明 UART 中断、真实 FIFO、波特率误差、PB4/PB7 电气、串口数据传输或 ThreadX/CDC 并发。产品 UART 参数和全链路测试仍按第 4 节保持未执行。

### 10.2 CMSIS-DAP 请求长度安全检查

| 检查项 | 所需覆盖 | 状态 |
|---|---|---|
| DAP-GUARD-01 | 固定上游产品启用命令和厂商路径的输入边界、截断、整数范围及 USB packet 上限 | 产品级未执行；产品配置/USB 长度契约未冻结，vendor 命令失败关闭 |
| DAP-GUARD-02 | 实际产品回调/命令配置下的最坏响应写入和响应缓冲边界 | 产品级未执行；产品 profile、回调上限和缓冲容量未定义 |
| DAP-BOUNDS-HOST-01 | 硬件无关预检器：固定/变长命令、Transfer/TransferBlock、SWD/JTAG Sequence、ExecuteCommands、截断、容量不足、packed length 上限及失败关闭路径 | 通过；102 项检查，使用显式测试 profile，不代表产品配置 |
| DAP-DISPATCH-HOST-01 | 使用 pinned 上游 `DAP_ExecuteCommand()` 验证有界 dispatch：拒绝截断/容量不足、有效 Connect、检查长度契约不符、执行多命令包、SWD Transfer 遇 FAULT 后仍消费完整请求 | 通过；5 个主机用例，使用测试 profile，不代表 USB 产品接入 |
| DAP-BOUNDS-TARGET-OBJ-01 | WCH RISC-V GCC 编译边界预检器并检查对象架构 | 通过；ELF32 RISC-V 对象，未链接至 PoC/DAP |

复现命令（仓库根目录）：`DBGC_BUILD_DIR=build/cmsis-dap-bounds-dispatch software/poc1-ch585-threadx/build.sh`。该现有构建入口通过 102 项边界预检、5 个有界 dispatch 主机用例，并将边界模块编译为 ELF32 RISC-V 对象；主机 profile 由测试显式提供。dispatch 主机用例调用固定上游 `DAP_ExecuteCommand()`，但尚未接入产品 USB 收包路径。上游 vendor、SWO、CMSIS-DAP UART 命令被拒绝；Info 检查依赖调用方提供真实最大写入字节数。该结果不证明产品调用路径、真实 USB 包长、回调行为或 CH585M 运行。O21 仍开放，产品级用例保持未执行。
