# DBG-C Firmware Architecture 与 PoC-1 记录

**文档编号：** DBG-C-FW-001　**版本：** V0.22　**状态：** 实验草案；通用 FIFO、CMSIS-DAP 命令层和上游 SWD 引擎主机检查已通过；产品 HAL 适配未完成；验证板设计门已通过；实板运行及产品硬件冻结未通过

## 1. 范围与状态

本阶段验证 Eclipse ThreadX 能否在 CH585M 工程中完成主机交叉构建，并实现实验性的启动、时钟、ThreadX 时基和两个线程睡眠/唤醒路径。PoC 位于 [`software/poc1-ch585-threadx/`](../../software/poc1-ch585-threadx/)，ThreadX 位于 [`software/third_party/threadx/`](../../software/third_party/threadx/)。这不是 DBG-C V1 固件，不包含 USB、CMSIS-DAP、SWD、CDC、BLE、私有 2.4G 或 OTA。

| 项目 | 当前记录 | 证据边界 |
|---|---|---|
| ThreadX | 标签 `v6.5.1.202602a_rel`，提交 `b91b03b9e75fa523b17127f9e0eca09dca916459`；上游仓库含 MIT `LICENSE.txt` | 子模块精确检出 |
| WCH EVT | 归档索引标注 `2026.08`；FreeRTOS 示例说明其使用 FreeRTOS-Kernel V11.3.0 与 CH585/CH584 QingKe V3C | 这是 WCH 示例信息，不表示本 PoC 使用 FreeRTOS |
| EVT 构建配置 | `.cproject` 选择 RISC-V Compiler 的 GCC12 配置，架构选项为 RV32I 加 M/C，ABI 为 ilp32，配置的程序前缀为 `riscv-none-embed-` | `.cproject` 未给出 GCC 补丁版本；前缀与当前 MounRiver 工具链不同 |
| 当前工具链 | MounRiver Studio Linux x64 Toolchain V2.4.0；GCC 12.2.0，GNU assembler/linker 2.38；实际程序前缀 `riscv-wch-elf-` | 本机命令实测，非 EVT 工程工具链版本结论 |
| 系统时钟初始化 | `highcode_init()` 移植 WCH `CH58x_sys.c` 中的复位时钟配置序列，选择 HSI PLL 62.4 MHz | 源码对应关系已核对；未在板上测量频率或时钟稳定性 |
| ThreadX low-level | 设置 `_tx_initialize_unused_memory` 到链接符号 `_end` 对齐后的位置；配置 WCH VTF SysTick 入口、PFIC 优先级和 SysTick | 主机编译/链接通过；内存边界及中断行为未板测 |
| ThreadX tick | PoC 使用上游 `tx_api.h` 默认 `TX_TIMER_TICKS_PER_SECOND`，其定义为 100 tick/s；以源码声明的 62.4 MHz 计算，`SysTick_Config` 输入为 624000，比较值为 623999 | 这是软件配置计算，不是实测时钟/tick；用户文字提及的 1000 tick/s 与当前源码不符，需确认需求；HPE/VTF、异常返回和调度语义尚未验证 |
| PoC 线程 | 两个同优先级线程递增独立计数器，记录 `tx_time_get()` 并执行 `tx_thread_sleep(1U)` | 仅是待板测的可观测量设计 |
| ELF 构建 | `text=8876`、`data=8`、`bss=5564` 字节；ELF32 little-endian RISC-V，入口 `_start=0x0`；`.highcode_init`/`.highcode` 位于链接 RAM，代码/数据装载地址位于链接 Flash | 两次 clean rebuild 的 ELF SHA-256 均为 `d06802e345843562221f86dfb29934442e7d15d25d55de3327ca9528c961101f`，map SHA-256 均为 `fec0082f30bbc3022aa696f7ec13f9037dc03aa8717e19bff854645d5964f936`；详见 `software/poc1-ch585-threadx/build-evidence.log`。仅证明固定环境下产物可重复且符合当前 linker script，不证明芯片运行 |
| CH585M 板级运行 | 未执行，待验证 | 用户确认目前没有可用板卡；暂无下载/调试和运行证据 |

## 2. 软件层和目录

```text
PoC application: src/main.c
ThreadX common kernel: software/third_party/threadx/common
ThreadX generic RISC-V32 context routines: upstream ports/risc-v32/gnu
CH585M adapter and startup: platform/ch585
Build: CMake and MounRiver WCH RISC-V GCC 12
```

PoC 沿用 ThreadX 上游 RISC-V32/GNU 的线程上下文保存/恢复、系统返回和中断控制例程，并以本地 `tx_initialize_low_level.S`、`tx_initialize_low_level.c`、`tx_systick.S` 接入芯片初始化与 SysTick。`highcode_init.c` 从 WCH EVT 的 `CH58x_sys.c` 提取对应时钟初始化序列；WCH startup 和 linker 文件来自 EVT FreeRTOS 示例。所有这些目前构成实验性端口，不代表其与 QingKe V3C 硬件压栈、VTF 或 `mret` 语义兼容。

## 3. Linux 主机环境配置

### 3.1 工具要求

需要 Git、CMake 3.20 或更新版本、GNU Make、MounRiver Linux x64 RISC-V Embedded GCC 12，以及本机 C99 编译器（默认命令为 `cc`；可通过环境变量 `CC` 指定）。当前验证环境记录为 Git 2.43.0、CMake 3.28.3、GNU Make 4.3、GCC 12.2.0、GNU assembler/linker 2.38 和 Ubuntu cc 13.3.0。Ninja 非必需。

### 3.2 安装 MounRiver 工具链

从 [MounRiver 官方下载页](https://www.mounriver.com/download)取得 `MRS_Toolchain_Linux_X64_V240.tar.xz`。下载页面是动态页面；先核对归档 SHA-256：

```text
1fae593d27e24466f17c2df0fd00f746143f587fe33e912a78e35142fef82a6d
```

以下命令选择性提取编译器目录，不安装整个 IDE：

```bash
archive="/path/to/MRS_Toolchain_Linux_X64_V240.tar.xz"
printf '%s  %s\n' \
  '1fae593d27e24466f17c2df0fd00f746143f587fe33e912a78e35142fef82a6d' \
  "$archive" | sha256sum -c -
mkdir -p "$HOME/.cache/DBG-C/MRS-2.4.0"
tar -xJf "$archive" -C "$HOME/.cache/DBG-C/MRS-2.4.0" \
  'Toolchain/RISC-V Embedded GCC12'
mkdir -p "$HOME/.local/share/DBG-C/toolchains/MRS-2.4.0"
cp -a "$HOME/.cache/DBG-C/MRS-2.4.0/Toolchain/RISC-V Embedded GCC12" \
  "$HOME/.local/share/DBG-C/toolchains/MRS-2.4.0/"
"$HOME/.local/share/DBG-C/toolchains/MRS-2.4.0/RISC-V Embedded GCC12/bin/riscv-wch-elf-gcc" --version
```

构建脚本默认使用上述用户级目录。工具链放在其他位置时，设置环境变量 `DBGC_WCH_TOOLCHAIN_ROOT` 为实际 `bin` 目录。

### 3.3 获取与构建

```bash
git clone --recurse-submodules <DBG-C 仓库地址>
cd DBG-C
git submodule update --init --recursive
software/poc1-ch585-threadx/build.sh
```

自定义工具链与构建目录：

```bash
DBGC_WCH_TOOLCHAIN_ROOT="/实际路径/RISC-V Embedded GCC12/bin" \
DBGC_BUILD_DIR="build-local" \
software/poc1-ch585-threadx/build.sh
```

构建目录必须位于 PoC 子目录中，默认目录为 `build/`；`software/poc1-ch585-threadx/.gitignore` 仅排除 `/build/`。脚本把工具版本、仓库与 ThreadX 修订、构建输出、ELF 大小和 ELF 头摘要写入 `build-evidence.log`。根目录 `.gitignore` 无需为构建产物修改。

## 4. WCH 来源与文件清单

原始资料保存在 [`CH585EVT 归档说明`](../09-references/CH585EVT/README.md)所指的官方 EVT 压缩包内。下列散列针对 ZIP 中原始条目；仓库中的 WCH 头文件仅规范化换行符和行尾空白，启动汇编与链接脚本还规范化了缩进空白及末尾空行；差异核对确认均为纯空白变化。时钟初始化文件是依据所列 WCH 源文件摘取函数实现，版权与使用声明保留。

| PoC 文件 | WCH EVT 原始条目 | 原始 SHA-256 |
|---|---|---|
| `platform/ch585/startup/startup_CH585.S` | `EVT/EXAM/FreeRTOS/Startup/Startup_CH585_FreeRTOS.S` | `48248d39086a92c9a5c5d57bf5dce4c64cf3ce83d6de0ef14c14e1d6a9bdef49` |
| `platform/ch585/ld/ch585.ld` | `EVT/EXAM/FreeRTOS/Ld/Link.ld` | `a5e7454144693e4b1af34cc5f04c7a7c9277664102285f08f731fd9f93551598` |
| `platform/ch585/wch/CH585SFR.h` | `EVT/EXAM/SRC/StdPeriphDriver/inc/CH585SFR.h` | `b1ee193c9037813758d72f47bfecb02046aee0edcd0129e0f8e73bd806788de3` |
| `platform/ch585/wch/CH58x_clk.h` | `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_clk.h` | `7e5de3bbd5edb11c1d6fff2bff1b33de5725777dc7128f29dee6c7624db207da` |
| `platform/ch585/wch/CH58x_sys.h` | `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_sys.h` | `1ce59c4b44c30d59dea204be0ae907b6dc66938f0bfdb3ce3567bb9c46eb1a51` |
| `platform/ch585/wch/core_riscv.h` | `EVT/EXAM/SRC/RVMSIS/core_riscv.h` | `fd0099eb7fce96c1ae1c0c5c58fd5da0d8f6b6b167e2c3fb335d5910b0a5503e` |
| `platform/ch585/highcode_init.c` clock sequence | `EVT/EXAM/SRC/StdPeriphDriver/CH58x_sys.c` | `954a8b489c8b333d482ed65e0665a1260c5f7976398ef4878f4f928251f62772` |

版本与工程格式证据：`EVT/CH585_List_EN.txt`（SHA-256 `112318f05aa8764f591dcea84333fba3a0e978e234c9c2d4f9086dc915c27fc9`）标注日期 `2026.08`；该索引和下述 FreeRTOS 工程元数据没有声明独立的 WCH SDK 语义版本，因此不得把日期称为 SDK 版本。示例 `EVT/EXAM/FreeRTOS/readme.txt`（SHA-256 `ae231c158445d7d68ef7b0e534100c24afcccac903bf2475f5647c9903d1d353`）声明它移植 FreeRTOS-Kernel V11.3.0 到 CH585/CH584 QingKe V3C，并说明硬件压栈、中断栈与 SysTick 快速中断入口的约束；这只是移植参考，不证明 ThreadX 兼容。`EVT/EXAM/FreeRTOS/.project`（SHA-256 `e98d5f026b4f06d2cb6a46933c5aa85425ccdeb5a4941fbb8a40a7a0b94c8b41`）是 Eclipse CDT 工程描述，项目名为 `FreeRTOS`；`.cproject`（SHA-256 `158d9e4ff583f3bff39be61a5b19270f976ecc1d10dd5ce17170281c0d1bab81`）是 CDT managed-build 配置，选择 GCC12 RISC-V 编译器选项，RV32I 加 M/C，ilp32，配置前缀 `riscv-none-embed-`，但未标明 GCC 补丁版本。FreeRTOS 样例 `EVT/EXAM/FreeRTOS/FreeRTOS/FreeRTOSConfig.h` 的 tick 为 500 Hz；本 PoC 不复用该值。ThreadX 使用上游头文件定义的默认 100 tick/s。

## 5. 链接、时钟和中断边界

WCH 示例链接脚本定义 FLASH 起始 `0x00000000`、长度 448 KiB，RAM 起始 `0x20000000`、长度 128 KiB。它们是所引用 EVT 样例的链接区域配置；板上芯片修订、内存映射适用性及实际运行占用仍待确认。

WCH 示例的 `highcode_init()` 初始化 HSI PLL 到 62.4 MHz，并配置相关安全访问与 Flash 时钟字段。PoC 把该函数体摘取到 `highcode_init.c`，不能据此认定板上系统时钟已经达到 62.4 MHz。

当前 SysTick ISR 汇编入口位于 WCH startup 使用的 highcode 区，执行流程为：建立 ThreadX 32-word 通用端口中断帧、根据 WCH FreeRTOS 端口方式清除 HPE 控制位、调用 ThreadX 上下文保存与计时器中断函数、清除由 WCH `core_riscv.h` 定义的 SysTick 状态寄存器，再跳转到 ThreadX 上下文恢复入口。具体 QingKe 异常入口是否已自动压栈、VTF 的入口/返回规则、HPE 对状态保存的影响、栈帧是否与 ThreadX 上游实现匹配，以及调度返回是否正确，均须用实板验证；主机链接不能证明这些行为。

## 6. 构建结果、验收和剩余工作

本轮完成两次 clean rebuild，ELF 与 map 散列一致。ELF header、section、program header、符号地址及 map 已与 PoC 启动代码和 `ch585.ld` 静态核对；`_start` 为 ELF 入口，应用 `.highcode` 段位于链接 RAM，链接段在当前脚本声明的 Flash/RAM 范围内。构建记录含 ThreadX 精确修订 `b91b03b9e75fa523b17127f9e0eca09dca916459`、MounRiver GCC 12.2.0、GNU assembler/linker 2.38、ELF32 RISC-V、入口 `0x0`，text 8876、data 8、bss 5564 字节。没有执行烧录、板上中断/调度、时钟测量或长时间运行测试。

### 6.1 软件验证与硬件设计顺序

当前无可用硬件。按“软件静态验证 → 限定用途验证板设计 → 实板运行验证 → 产品硬件冻结”设置两道放行门：

1. **验证板设计门：已通过。** 两次干净构建产生相同 ELF/map；ELF 架构、入口、加载/运行段、符号与链接脚本已静态核对；源码审查覆盖 reset 初始化、ThreadX 上下文入口、异常栈切换、SysTick/VTF/PFIC/HPE 配置及 ISR 路径。允许开始限定用途的最小验证板设计，并应提供编程/恢复接入及复位、系统时钟、SysTick、中断活动的观测/测量点。该门不代表 CH585M 实板运行通过，也不允许冻结产品原理图或 PCB。
2. **产品硬件冻结门：未通过。** CH585M 实板运行证据尚不存在。ThreadX 启动、线程切换/休眠/唤醒、实测 SysTick/tick 投递、栈完整性、复位/睡眠唤醒和持续运行均须实测；测试时长、重复次数、负载和通过门限须在执行前写入测试方案。源码当前为 100 tick/s，需求文字提及 1000 tick/s；须在冻结实板验收频率门限前解决该冲突。

静态审查不能证明 QingKe V3C 异常压栈、VTF/HPE、中断返回或调度语义；相应运行项在 TEST-001 中保持未执行，必须通过 CH585M 实板测量。上游 ThreadX `qemu_virt` 示例使用独立的 QEMU virt 入口和链接布局，即使运行也不能替代 CH585M/WCH 路径验证。当前未安装 QEMU、Spike 或 Renode。详细软件门槛和判定见 OPEN-001 O19。

### 6.2 验证用硬件设计约束

验证板应保持 PoC 最小化，并提供可安全恢复/烧录的调试接入点，以及观测复位、系统时钟、SysTick 和中断活动所需的测量接入点。具体引脚、探测方法、负载和电气保护须依据 CH585M 数据手册、WCH EVT 和测试方案确定；本记录不指定未经核对的引脚或硬件电路。验证板不得宣称为产品原理图/PCB冻结版本。

### 6.3 尚需完成

1. 在具体 CH585M 板卡上确认编程器、下载流程、芯片修订和启动行为。
2. 观察 `thread_a_runs`、`thread_b_runs`、`threadx_tick_observed` 的持续变化；验证 `tx_thread_sleep(1U)` 的超时/唤醒、同优先级轮转及系统复位后的稳定性。
3. 实测 SysTick 频率、时钟稳定性、VTF/PFIC/HPE 行为和异常栈完整性；保存固件散列、板卡版本、下载日志和运行日志。
4. 先在测试方案中冻结实板测试时长、重复次数、负载与通过门限；验证中断嵌套、调度抢占和长时间运行后，才能关闭 O18 并同步 MCU、FW、TEST、OPEN 文档。

## 7. 资料来源

- Eclipse ThreadX 官方仓库与 MIT 许可：[`software/third_party/threadx/`](../../software/third_party/threadx/)，标签和提交见第 1 节。
- Arm 官方 CMSIS-DAP 仓库：[`software/third_party/cmsis-dap/`](../../software/third_party/cmsis-dap/)，固定提交 `12636590eec66fae2d1bba4518749426ad5a4595`（2026-09-29 核对）；`DAP.h` 声明的 CMSIS-DAP 固件版本为 2.1.2；仓库许可为 Apache-2.0。此引用是 CMSIS-DAP 协议固件源码，不代表集成了完整 DAPLink。
- WCH CH585 EVT：[`归档说明`](../09-references/CH585EVT/README.md)及本节列出的归档文件与散列。
- MounRiver Linux x64 Toolchain V2.4.0：[官方下载页](https://www.mounriver.com/download)，工具链包 SHA-256 见第 3 节。

## 8. 软件模块实现状态

资源依据见 [MCU-001 第 5 节](../04-hardware/DBG-C-MCU-001.zh-CN.md)：USB CDC/UART、RF 收发均需要 SRAM 数据缓冲。官方 EVT 归档的 `EVT/EXAM/BLE/BLE_UART/APP/app_drv_fifo/app_drv_fifo.c` 与 `.h`（原始 SHA-256 分别为 `312e039857300b5142f6ae6898a592b6257170d146f13a92e02bd8808bc5ec67`、`5ce231b33f9a5897a6be2abddc1901cd181f63c93c3fa00867216ac14cc8b6cc`）及 `EVT/EXAM/BLE/BLE_UART/APP/peripheral.c` 证明 WCH 示例采用字节 FIFO 缓冲 UART 数据。该示例不证明其并发临界区适配 ThreadX；本项目未直接复制或调用该 FIFO。

| 模块 | 软件位置/资源依据 | 当前实现边界 | 状态/验证 |
|---|---|---|---|
| 通用字节 FIFO | `software/common/byte_fifo/`；对应 MCU-001 规划的 USB CDC、UART 与 RF 收发缓冲 | 调用方提供固定存储；支持任意非零容量、部分读写、不覆盖未读数据、清空与容量查询；无动态分配、无芯片寄存器/中断/ThreadX API；不保证并发安全，调用方必须串行化访问 | 已加入现有 CMake 构建并通过 CH585 交叉编译；既有构建脚本中的主机验证运行通过 3683 项断言；当前未接入 USB/UART/RF，未做实板测试；不是冻结的产品 ABI |
| CMSIS-DAP / DAP command core | MCU-001 USBFS 分配；PRD 的 CMSIS-DAP v2 目标；Arm 官方源码固定于 `software/third_party/cmsis-dap/` 提交 `12636590eec66fae2d1bba4518749426ad5a4595` | `DAP.h` 声明 `DAP_ProcessCommand()`/`DAP_ExecuteCommand()`，固件版本宏为 2.1.2；上游依赖缺失的 `cmsis_compiler.h`，非 ArmCC 分支使用 Arm `subs` 内联汇编，不能直接用于 WCH RISC-V GCC | 8 项主机用例覆盖 Info/错误响应、TransferConfigure 重试配置、DP 读写、WAIT 重试、DAP_Transfer AP posted-read 与 `DP_RDBUFF` 响应顺序，以及 AP DAP_TransferBlock 读；WCH RISC-V GCC 将启用 SWD 命令分支的 `DAP.c` 编译为 ELF32 RISC-V 对象。测试 pin 宏为空操作、SWD 事务为模拟桩、JTAG 关闭；对象未链接到 PoC。这不是产品 HAL/配置，不验证 GPIO、电气时序、USB、线程或实板 |
| CMSIS-DAP SWD I/O engine | 固定上游 `Firmware/Source/SW_DP.c`；MCU-001 分配 PB5=SWDIO、PB6=SWCLK | 上游位级算法经 `DAP_config.h` 的 pin 宏调用 IO；本项目 host-test 用回调式线模型，未连接 WCH GPIO API；fast 延时为空操作 | 八项主机用例共 148 项断言，覆盖 SWD 读/写数据与奇偶校验、WAIT/FAULT ACK、10 位输入/输出序列、DAP_Connect 后的 DP IDCODE 读取，以及 AP DAP_Transfer / DAP_TransferBlock posted-read 经 DP_RDBUFF 返回数据的端到端响应；WCH RISC-V GCC 将上游 `SW_DP.c` 编译成 ELF32 RISC-V 对象。对象未与产品 HAL 链接，也未在 Target 或 CH585M 板上运行；不证明 PB5/PB6 波形、频率、电平或时序 |
| USBFS Device + CDC transport | MCU-001 USBFS、PB10/PB11；EVT `CH58x_usbdev.h/.c` 及 USB Device COM 示例 | 已查到归档底层 API、EP0–EP4 缓冲结构与 CDC 示例描述符；当前示例把 CDC/厂商模式分开，未证明 DAP + CDC 组合、当前工程兼容性或 ThreadX ISR 同步 | 暂不实现；需验证 O02/O08，冻结 USB-001 并完成主机枚举/传输验证 |
| UART0 driver/bridge | MCU-001 UART0、PB4/PB7；EVT `CH58x_uart.h`、`CH58x_uart0.c` | 已查到归档 API 名称和轮询实现；COM 示例把 UART0 重映射至 PA15/PA14，与 MCU-001 的 PB4/PB7 分配不同；实际 PB4/PB7 初始化、CDC 行为、波特率/流控和 ThreadX 同步仍待确认 | 暂不实现；取得 PB4/PB7 的当前 SDK 初始化证据，并定义 O09 参数与 ISR/任务并发行为 |
| SWD Engine / Target Manager | MCU-001 的 PB5/PB6 SWD GPIO、PA4 Reset GPIO | SWD 时序、方向切换、电气边界及 Target 电压/保护未形成完整可执行规范 | 暂不实现；需冻结 IF-001 并按 O11/O12 验证时序及电气要求 |
| BLE GATT/配置/OTA | 集成 BLE Radio 与 WCH BLE 示例 | 产品 Service/Characteristic、认证、MTU、配对和 OTA 镜像行为未定 | 暂不实现；需先完成 BLE-001、O06/O07/O10/O13 |
| 私有 2.4G transport | 集成 Radio，DBG-C RF Protocol 目标 | PHY/API、包格式、序列、重试、恢复和时延目标未确定 | 暂不实现；需先完成 RF-001、O01/O03/O12 |
| CH585M ThreadX port | 当前 `software/poc1-ch585-threadx/platform/ch585/` | reset、SysTick、PFIC/VTF/HPE 与上下文切换依赖芯片硬件语义 | 仅实验 PoC；启动、中断、tick、上下文切换、调度、睡眠/唤醒均未做 CH585M 实板验证，保持 O18 开放 |

字节 FIFO 只建立与硬件无关的存储边界。USB、UART、RF 调用方不得直接依赖其内部索引；在同步/并发模型和传输 API 确认前，不把该模块包装成 ISR-safe queue 或产品数据协议。FIFO 主机用例由 `software/common/byte_fifo/tests/test_dbgc_byte_fifo.c` 提供，并由既有 `software/poc1-ch585-threadx/build.sh` 使用本机 C99 编译器构建和运行；当前 3683 项断言通过。该结果不验证并发、ISR、CH585M SRAM 或板级行为。

CMSIS-DAP 测试位于 `software/common/cmsis_dap_host_test/`，由既有 `software/poc1-ch585-threadx/build.sh` 编译固定上游源码副本并运行。准备脚本核对 CMSIS-DAP 提交及原始源码未修改，只在构建目录副本中把 `DAP.h` 的 `__CC_ARM` 延时分支切换到专用测试宏；未定义 `__CC_ARM`，未改动第三方子模块。命令核心的 8 项检查使用空操作引脚及 `SWD_Transfer()` 模拟桩。另将原始上游 `SW_DP.c` 与同一 `DAP.c` 编译到独立测试程序，pin 宏通过回调连接到纯主机位流模型；八项测试检查读成功/奇偶校验错误、写数据及奇偶校验、WAIT/FAULT ACK、10 位 SWD 输入/输出序列、DAP_Connect 到 DP IDCODE、AP DAP_Transfer 与两项 AP DAP_TransferBlock 端到端响应、方向切换和模型周期数。模型 fast 延时为空操作，周期数不是实测时钟。WCH RISC-V GCC 分别将测试配置下的 `DAP.c` 和 `SW_DP.c` 编译为 ELF32 RISC-V 对象，均未链接 PoC。以上均不验证产品 GPIO HAL、PB5/PB6 电气及时序、USBFS、ThreadX 或 CH585M 板级行为。


### 8.1 WCH USBFS 与 UART0 源码审查

本节记录对归档原始条目的静态审查，不代表这些旧版示例已移植到 DBG-C、已由当前工程编译，或已在 CH585M 实板运行。

| 官方归档条目 | 原始文件证据 | 对 DBG-C 的结论 |
|---|---|---|
| `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_usbdev.h`；SHA-256 `1b5dbf1304127b980d2f38f974e3372c5e55a747fe22319a835156deac1fee0f` | 文件头标记 V1.2、2021/11/17；声明 `USB_DeviceInit()`、`USB_DevTransProcess()`、EP1–EP4 IN/OUT 处理函数；描述 EP0 与 EP1–EP4 的 64-byte 缓冲布局 | 可确认归档中存在 WCH USBFS Device 底层接口和端点缓冲示例；不能据此确认 ThreadX ISR/任务同步或 DAP + CDC 复合设备实现 |
| `EVT/EXAM/SRC/StdPeriphDriver/CH58x_usbdev.c`；SHA-256 `dfef009a88c6c3c70670889bbab43898264967f979dc56b4c68180542b4ea540` | 文件头标记 V1.2、2021/11/17；`USB_DeviceInit()` 配置 4 个非零端点的双向通道，设置 DMA 地址、ACK/NAK 状态和 USB 中断使能 | 是直接寄存器级示例；不能当作已验证的 ThreadX-safe USB Service/HAL |
| `EVT/EXAM/USB/Device/COM/src/Main.c`；SHA-256 `4345ca31938ab30bf4c772e1ceeaccbd717c6a82c8a26012ef8708280fbcfda4` | 文件头标记 V1.0、2020/08/06；含 CDC 描述符及 CDC/厂商两套配置；CDC 配置使用 EP4 中断 IN、EP1/EP2 Bulk；运行时由 `usb_work_mode` 在 CDC 与厂商模式间选择；USB ISR 直接读写寄存器并调用 PFIC 屏蔽/恢复中断 | 证明归档有 CDC Device 示例，不证明同一配置已集成 CMSIS-DAP v2 Bulk；两个模式不是该例程中的同时复合配置。端点映射、描述符、IRQ 与 ThreadX 集成须重新设计并实测 |
| `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_uart.h`；SHA-256 `99e419e0906b4fdaba12fd1aea3cdf58109cfd8906b1508d69e5dd884c741f70` | 文件头标记 V1.2、2021/11/17；声明 UART0 波特率、触发级别、中断配置、收发接口和状态宏 | 可确认归档 API 名称；不证明 PB4/PB7 的目标工程复用初始化或 ISR 到 ThreadX 的同步安全性 |
| `EVT/EXAM/SRC/StdPeriphDriver/CH58x_uart0.c`；SHA-256 `f0b6b252dcb69530428f32cf2bb50cb585fbecd6f5cfd0eb57f194e257d86338` | 文件头标记 V1.2、2021/11/17；默认初始化把波特率设为 115200、FIFO 触发级别字段设为 4-byte、使能 TXD 中断；收发字符串函数轮询 FIFO 状态/计数 | 这些是示例默认值，不是 DBG-C 冻结参数。轮询 API 不满足尚未定义的 UART/ThreadX 并发服务契约，不能直接作为产品桥接驱动 |
| `EVT/EXAM/USB/Device/COM/src/Main.c` 中 `DebugInit()` | 调用 `GPIOPinRemap(ENABLE, RB_PIN_UART0)`，然后配置 PA15/PA14 并初始化 UART0 | 该 CDC 示例将 UART0 重映射到 PA15/PA14；DBG-C MCU-001 分配 PB4/PB7。不得复制此示例的引脚初始化到产品固件。PB4/PB7 的最终 SDK 初始化证据仍需取得 |

通用字节 FIFO 是当前已实现且可复用的硬件无关产品模块。CMSIS-DAP 命令核心及上游 SWD 位级算法已增加主机检查；SWD host-test 的 pin 回调/线模型是验证夹具，不是产品 HAL 或实际 GPIO 实现。EVT GPIO API 已静态核对，但未集成编译到固件：PB5/PB6 输入输出模式、输出能力、目标电压兼容、外部保护及所需 SWD 时序仍须按 IF-001/O11/O12 确认，并在验证板上测量。USBFS + CDC、UART0 驱动/桥接、BLE 和私有 RF 仍缺产品接口/调度/硬件证据。O02、O08、O09 与 O18 保持开放，所有 ThreadX/USB/UART ISR 行为仍需实板验证。

### 8.3 上游 SWD 引擎主机线模型与 WCH GPIO 证据

固定 CMSIS-DAP 提交中的 `Firmware/Source/SW_DP.c` 提供 SWJ/SWD 序列与 `SWD_Transfer()` 位级算法。测试配置通过 `PIN_SWCLK_TCK_*`、`PIN_SWDIO_*` 宏连接到测试回调；主机位流模型提供 ACK、数据和奇偶校验输入，并记录输出位、时钟上升调用及输入/输出方向调用。读取用例覆盖正确和错误奇偶校验。测试选择 fast-clock 分支，测试 `__NOP()` 为空操作；因此模型中的 46 次时钟置高调用不是频率或时序测量。

WCH EVT 官方归档 GPIO 文件：`EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_gpio.h`（SHA-256 `c7f450ceaa501e4c5a912bc0429bca1540fa3c3f26e3fd55dfd908fad9b06183`）声明 `GPIOB_ModeCfg(uint32_t pin, GPIOModeTypeDef mode)`，并定义 `GPIOB_SetBits`、`GPIOB_ResetBits`、`GPIOB_ReadPortPin`；`EVT/EXAM/SRC/StdPeriphDriver/CH58x_gpio.c`（SHA-256 `c2c7c9b2518a2a56144f107776418911ae0849df523481ee7b999cbdfab01757`）给出 GPIOB 模式配置实现。MCU-001 将 PB5/PB6 分配给 SWDIO/SWCLK。资料支持确认 SDK 符号与端口 API，但尚未证明已编入本工程，也不能决定产品驱动模式、目标电平/保护或 GPIO 切换时序。取得 IF-001 电气约束和目标兼容范围后，才可实现 WCH GPIO BSP 并在验证板上测量波形。

### 8.2 CMSIS-DAP 上游源码固定与适配缺口

Arm 官方 CMSIS-DAP 仓库已作为 Git 子模块固定到提交 `12636590eec66fae2d1bba4518749426ad5a4595`。该提交 `Firmware/Include/DAP.h` 声明 `DAP_ProcessCommand()` 与 `DAP_ExecuteCommand()`，并通过 `DAP_FW_VER` 声明固件版本 2.1.2。此源码是 CMSIS-DAP 固件实现，不是 DAPLink 完整固件体系。上游 `Firmware/Config/DAP_config.h` 是面向示例工程的模板：其中 CPU 时钟、JTAG 能力、DAP 包尺寸及缓冲包数的默认值不得直接用于 DBG-C。

当前子模块的 `DAP.h` 包含 `cmsis_compiler.h`；该文件既不在 CMSIS-DAP 子模块，也未出现在仓库提供的 CH585EVT ZIP 条目中。该头文件的 `PIN_DELAY_SLOW()` 在非 `__CC_ARM` 分支使用 Arm `subs %0,%0,#1` 内联汇编，WCH RISC-V GCC 不能直接汇编该指令。不得伪定义 `__CC_ARM`；测试脚本只在构建目录的头文件副本中选择上游 C 循环分支，并以 WCH GCC 验证 DAP.c 的 RISC-V 对象编译。CMSIS-DAP 产品配置仍需正式的编译器支持头和 GPIO/时钟 HAL。当前测试使用空操作 pin 宏和模拟 `SWD_Transfer()`，不证明 PB5/PB6 波形、电气时序或目标 ACK 行为。产品接口及实板验证未完成，不把该测试对象接入 PoC 产品固件。
