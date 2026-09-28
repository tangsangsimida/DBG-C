# DBG-C Firmware Architecture 与 PoC-1 记录

**文档编号：** DBG-C-FW-001　**版本：** V0.44　**状态：** 实验草案；通用 FIFO、单向/双向无调度字节流桥接、CMSIS-DAP 命令层、上游 SWD 引擎、请求/响应边界预检及有界 dispatch 主机检查已通过；CMSIS 编译器宏映射、SWD GPIO、Target Reset GPIO、UART0、UID 读取适配器、桥接适配器与 WCH ROM 命令库目标编译/链接检查通过；PB5/PB6、PA4、UART0、UID 读取适配器、单向/双向桥接主机检查分别为 57 项、33 项、94 项、31 项、15 项和 15 项；PoC 新增两线程创建状态及独立最近 tick 观测符号，固定路径两次 clean rebuild 的 ELF/map 散列一致；产品 DAP/USB 接入、GPIO/UART 电气行为及时序未验证；芯片 UID 实际读取、产品命令配置与边界契约仍待验证/定义；验证板设计门已通过；PoC-1 实板运行验证未执行；产品硬件冻结未放行

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
| ELF 构建 | `text=8916`、`data=8`、`bss=5580` 字节；ELF32 little-endian RISC-V，入口 `_start=0x0`；`.highcode_init`/`.highcode` 位于链接 RAM，代码/数据装载地址位于链接 Flash | 固定构建路径两次 clean rebuild 的 ELF SHA-256 均为 `00f0e40b217d61df475f721607a6dd88b2e9f1450b511863b2984899e97e32f8`，map SHA-256 均为 `823322fcfe0849eefa6b88e4df478ac28e6bdc094a11173faaba1e85697fdfa5`；详见 `software/poc1-ch585-threadx/build-evidence.log`。仅证明固定环境下产物可重复且符合当前 linker script，不证明芯片运行 |
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

本轮 ThreadX 可观测性变更完成后，在固定构建路径 `software/poc1-ch585-threadx/build/threadx-observability/` 执行两次 clean rebuild，产物一致：ELF SHA-256 `00f0e40b217d61df475f721607a6dd88b2e9f1450b511863b2984899e97e32f8`，map SHA-256 `823322fcfe0849eefa6b88e4df478ac28e6bdc094a11173faaba1e85697fdfa5`。不同构建路径产生不同的 ELF/map SHA-256，不作为同路径可复现性比较。ELF header、section、program header、符号地址及 map 已与 PoC 启动代码和 `ch585.ld` 静态核对；`_start` 为 ELF 入口，应用 `.highcode` 段位于链接 RAM，链接段在当前脚本声明的 Flash/RAM 范围内。构建记录含 ThreadX 精确修订 `b91b03b9e75fa523b17127f9e0eca09dca916459`、MounRiver GCC 12.2.0、GNU assembler/linker 2.38、ELF32 RISC-V、入口 `0x0`，text 8916、data 8、bss 5580 字节。没有执行烧录、板上中断/调度、时钟测量或长时间运行测试。

### 6.1 软件验证与硬件设计顺序

当前无可用硬件。按“软件静态验证 → 限定用途验证板设计 → 实板运行验证 → 产品硬件冻结”设置两道放行门：

1. **验证板设计门：已通过。** 两次干净构建产生相同 ELF/map；ELF 架构、入口、加载/运行段、符号与链接脚本已静态核对；源码审查覆盖 reset 初始化、ThreadX 上下文入口、异常栈切换、SysTick/VTF/PFIC/HPE 配置及 ISR 路径。允许开始限定用途的最小验证板设计，并应提供编程/恢复接入及复位、系统时钟、SysTick、中断活动的观测/测量点。该门不代表 CH585M 实板运行通过，也不允许冻结产品原理图或 PCB。主机交叉构建不是芯片运行验证。
2. **PoC-1 实板运行验证：未执行，不能记为通过。产品硬件冻结门：未通过。** CH585M 实板运行证据尚不存在。ThreadX 启动、线程切换/休眠/唤醒、实测 SysTick/tick 投递、栈完整性、复位/睡眠唤醒和持续运行均须实测；测试时长、重复次数、负载和通过门限须在执行前写入测试方案。源码当前为 100 tick/s，需求文字提及 1000 tick/s；该冲突不阻止验证板设计，但须在冻结实板验收频率门限前解决。

静态审查不能证明 QingKe V3C 异常压栈、VTF/HPE、中断返回或调度语义；相应运行项在 TEST-001 中保持未执行，必须通过 CH585M 实板测量。上游 ThreadX `qemu_virt` 示例使用独立的 QEMU virt 入口和链接布局，即使运行也不能替代 CH585M/WCH 路径验证。当前未安装 QEMU、Spike 或 Renode。详细软件门槛和判定见 OPEN-001 O19。

### 6.2 验证用硬件设计约束

验证板应保持 PoC 最小化，并提供可安全恢复/烧录的调试接入点，以及观测复位、系统时钟、SysTick 和中断活动所需的测量接入点。具体引脚、探测方法、负载和电气保护须依据 CH585M 数据手册、WCH EVT 和测试方案确定；本记录不指定未经核对的引脚或硬件电路。验证板不得宣称为产品原理图/PCB冻结版本。

### 6.3 尚需完成

1. 在具体 CH585M 板卡上确认编程器、下载流程、芯片修订和启动行为。
2. 先检查 `thread_a_create_status` 与 `thread_b_create_status` 均为 ThreadX `TX_SUCCESS`；观察 `thread_a_runs`、`thread_b_runs`、`thread_a_last_tick`、`thread_b_last_tick` 与 `threadx_tick_observed` 的变化；验证 `tx_thread_sleep(1U)` 的超时/唤醒、同优先级轮转及系统复位后的稳定性。这些符号只提供调试观测，不会自行判定 tick 频率或调度正确。
3. 实测 SysTick 频率、时钟稳定性、VTF/PFIC/HPE 行为和异常栈完整性；保存固件散列、板卡版本、下载日志和运行日志。
4. 先在测试方案中冻结实板测试时长、重复次数、负载与通过门限；验证中断嵌套、调度抢占和长时间运行后，才能关闭 O18 并同步 MCU、FW、TEST、OPEN 文档。

## 7. 资料来源

- Eclipse ThreadX 官方仓库与 MIT 许可：[`software/third_party/threadx/`](../../software/third_party/threadx/)，标签和提交见第 1 节。
- Arm 官方 CMSIS-DAP 仓库：[`software/third_party/cmsis-dap/`](../../software/third_party/cmsis-dap/)，固定提交 `12636590eec66fae2d1bba4518749426ad5a4595`（2026-09-29 核对）；`DAP.h` 声明的 CMSIS-DAP 固件版本为 2.1.2；仓库许可为 Apache-2.0。此引用是 CMSIS-DAP 协议固件源码，不代表集成了完整 DAPLink。
- 编译器宏映射依据：WCH EVT `core_riscv.h` 提供 GNU 编译器下的 `__ASM`/`__INLINE` 定义，需先包含含 `IRQn_Type` 的 `CH585SFR.h`；GCC 官方 [内联汇编](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html)及[函数属性](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html)文档说明 GNU 内联汇编、`always_inline` 与 weak 属性；RISC-V 官方[非特权指令规范](https://docs.riscv.org/reference/isa/unpriv/rv32.html)定义 `nop`。外部资料核对日期：2026-09-29。该依据只支持编译器基元映射，不支持 GPIO/SWD 时序或芯片运行结论。
- WCH CH585 EVT：[`归档说明`](../09-references/CH585EVT/README.md)及本节列出的归档文件与散列。
- MounRiver Linux x64 Toolchain V2.4.0：[官方下载页](https://www.mounriver.com/download)，工具链包 SHA-256 见第 3 节。

## 8. 软件模块实现状态

资源依据见 [MCU-001 第 5 节](../04-hardware/DBG-C-MCU-001.zh-CN.md)：USB CDC/UART、RF 收发均需要 SRAM 数据缓冲。官方 EVT 归档的 `EVT/EXAM/BLE/BLE_UART/APP/app_drv_fifo/app_drv_fifo.c` 与 `.h`（原始 SHA-256 分别为 `312e039857300b5142f6ae6898a592b6257170d146f13a92e02bd8808bc5ec67`、`5ce231b33f9a5897a6be2abddc1901cd181f63c93c3fa00867216ac14cc8b6cc`）及 `EVT/EXAM/BLE/BLE_UART/APP/peripheral.c` 证明 WCH 示例采用字节 FIFO 缓冲 UART 数据。该示例不证明其并发临界区适配 ThreadX；本项目未直接复制或调用该 FIFO。

| 模块 | 软件位置/资源依据 | 当前实现边界 | 状态/验证 |
|---|---|---|---|
| 通用字节 FIFO | `software/common/byte_fifo/`；对应 MCU-001 规划的 USB CDC、UART 与 RF 收发缓冲 | 调用方提供固定存储；支持任意非零容量、部分读写、不覆盖未读数据、清空与容量查询；无动态分配、无芯片寄存器/中断/ThreadX API；不保证并发安全，调用方必须串行化访问 | 已加入现有 CMake 构建并通过 CH585 交叉编译；既有构建脚本中的主机验证运行通过 3683 项断言；当前未接入 USB/UART/RF，未做实板测试；不是冻结的产品 ABI |
| 无调度字节流桥接通道 | `software/common/byte_stream_bridge`；PRD-001 CDC UART 双向数据要求、MCU-001 UART0 PB4/PB7 与 USB CDC、通用 FIFO 及轮询 UART0 原语 | 单执行上下文轮询；端点使用显式单字节非阻塞回调；固定 FIFO 加每次服务限额；FIFO 满时停止读源，写端背压或错误时保留待发字节；不含 ThreadX、ISR、USB 栈 API、产品波特率/流控或并发保证 | 15 项回调主机检查通过；静态库在 PoC 构建中编译，但应用未调用，尚未形成 USB CDC/UART 产品桥接；实板 UART/USB、电气、并发和产品流控未验证 |
| 双向无调度字节流桥接服务 | `software/common/byte_duplex_bridge/`；复用两路通用字节流桥接通道，适用于 MCU-001 的 UART0 PB4/PB7 与待实现的 USB CDC 端点之间的数据双向传输 | 调用方分别提供两路 FIFO、读写回调和每次服务预算；按固定先后顺序服务两个方向；单方向回压不阻止另一方向服务；不分配内存、不选定 USB/UART API、不含 ThreadX/ISR/并发保证，也不定义缓冲容量或调度频率 | 15 项回调模型主机检查通过；PoC CMake 构建编译静态库，但 PoC 应用未调用；尚未接入 USB CDC/UART 产品路径，不证明端点公平性、吞吐、电气或实板行为 |
| CMSIS-DAP / DAP command core | MCU-001 USBFS 分配；PRD 的 CMSIS-DAP v2 目标；Arm 官方源码固定于 `software/third_party/cmsis-dap/` 提交 `12636590eec66fae2d1bba4518749426ad5a4595` | `DAP.h` 声明 `DAP_ProcessCommand()`/`DAP_ExecuteCommand()`，固件版本宏为 2.1.2；上游依赖缺失的 `cmsis_compiler.h`，非 ArmCC 分支使用 Arm `subs` 内联汇编，不能直接用于 WCH RISC-V GCC | 8 项主机用例覆盖 Info/错误响应、TransferConfigure 重试配置、DP 读写、WAIT 重试、DAP_Transfer AP posted-read 与 `DP_RDBUFF` 响应顺序，以及 AP DAP_TransferBlock 读；WCH RISC-V GCC 将启用 SWD 命令分支的 `DAP.c` 编译为 ELF32 RISC-V 对象。测试 pin 宏为空操作、SWD 事务为模拟桩、JTAG 关闭；对象未链接到 PoC。这不是产品 HAL/配置，不验证 GPIO、电气时序、USB、线程或实板 |
| CMSIS-DAP 请求/响应边界预检与 dispatch | `software/common/cmsis_dap_bounds/`；命令 ID 和 transfer 标志取自固定 CMSIS-DAP `DAP.h` | API 接收实际请求长度、响应容量及显式 profile；dispatch 在调用上游 `DAP_ExecuteCommand()` 前执行预检，成功后核对上游报告的消费/响应长度；vendor、SWO、CMSIS-DAP UART 命令，以及未提供 Info payload 上限的 profile 失败关闭。profile 必须与最终 DAP 编译配置一致；调用方仍须核实回调实际写入不超过其声明上限，调用后的长度核对不能阻止违规回调越界写入 | 102 项边界主机检查及 5 个有界 dispatch 主机用例通过；WCH RISC-V GCC 生成 ELF32 RISC-V 对象。dispatch 尚未接入产品 USB 收包路径；产品 profile、Info 回调上限、USB 实际收包长度和产品缓冲容量未定义，因此不构成产品请求边界安全证明 |
| CMSIS-DAP SWD I/O engine | 固定上游 `Firmware/Source/SW_DP.c`；MCU-001 分配 PB5=SWDIO、PB6=SWCLK | 上游位级算法经 `DAP_config.h` 的 pin 宏调用 IO；本项目 host-test 用回调式线模型，未连接 WCH GPIO API；fast 延时为空操作 | 九项主机用例共 168 项断言，覆盖 SWD 读/写数据与奇偶校验、WAIT/FAULT ACK、10 位输入/输出序列及 DAP_SWD_Sequence 命令解析、DAP_Connect 后的 DP IDCODE 读取，以及 AP DAP_Transfer / DAP_TransferBlock posted-read 经 DP_RDBUFF 返回数据的端到端响应；WCH RISC-V GCC 将上游 `SW_DP.c` 编译成 ELF32 RISC-V 对象。对象未与产品 HAL 链接，也未在 Target 或 CH585M 板上运行；不证明 PB5/PB6 波形、频率、电平或时序 |
| CH585 SWD GPIO BSP | `software/poc1-ch585-threadx/platform/ch585/dbgc_ch585_swd_gpio.c`；MCU-001 PB5=SWDIO、PB6=SWCLK；WCH EVT `CH58x_gpio.c` 的 `GPIOB_ModeCfg()` 实现及本地 `CH585SFR.h` GPIOB 寄存器定义 | 封装 SWDIO/SWCLK 的方向/上下拉/驱动模式选择、输出置位/清零和输入读取；模式由调用方传入；不设默认电气状态，不提供临界区或时序延迟，尚未接入 CMSIS-DAP | 同一 BSP 源码以模拟寄存器运行 57 项主机检查，覆盖两根信号的五种寄存器模式、读写和无效参数；WCH RISC-V GCC 以 `-Werror` 编译为 ELF32 RISC-V 对象，未链接 PoC。主机模型不证明真实寄存器、目标电平兼容、输出电流、SWD 波形、方向切换时序或板级行为 |
| CH585 Target Reset GPIO BSP | `software/poc1-ch585-threadx/platform/ch585/dbgc_ch585_target_reset_gpio.c`；MCU-001 的 PA4 QFN48-36 分配；WCH EVT GPIOA 模式实现及本地 `CH585SFR.h` GPIOA 寄存器定义 | 封装 PA4 模式选择、原始高/低电平写入与电平读取；调用方显式选择模式；不定义有效电平转换、脉冲宽度、默认态或同步机制 | 33 项模拟寄存器主机检查覆盖五种模式、读写、无效模式和空输出指针；WCH RISC-V GCC `-Werror` 生成 ELF32 RISC-V 对象，未链接 PoC。仅验证软件寄存器操作，不验证 PA4 引脚、电气输出、目标复位效果或时序；这些仍待验证板测量 |
| USBFS Device + CDC transport | MCU-001 USBFS、PB10/PB11；EVT `CH58x_usbdev.h/.c` 及 USB Device COM 示例 | 已查到归档底层 API、EP0–EP4 缓冲结构与 CDC 示例描述符；当前示例把 CDC/厂商模式分开，未证明 DAP + CDC 组合、当前工程兼容性或 ThreadX ISR 同步 | 暂不实现；需验证 O02/O08，冻结 USB-001 并完成主机枚举/传输验证 |
| CH585 UART0 polled BSP primitive | `software/poc1-ch585-threadx/platform/ch585/dbgc_ch585_uart0.c`；MCU-001 UART0 PB4/PB7；EVT ADC `DebugInit()` 和本地 WCH-derived `CH585SFR.h` | 配置 PB4 上拉输入、PB7 先置高再设 5 mA 推挽输出、选择 UART0 默认 PB4/PB7 复用；调用者显式提供系统时钟、波特率和 FIFO trigger；可用 `dbgc_ch585_uart0_build_lcr()` 按 WCH SFR 编码组合字长、停止位和奇偶校验；`dbgc_ch585_uart0_calculate_divisor()` 按 EVT `UART0_BaudRateCfg()` 公式计算分频值且不访问寄存器；初始化不设产品默认格式；禁用 UART 中断源，保留 TXD 输出使能；非阻塞轮询读写；不含 CDC、队列、ThreadX 或并发保护 | 94 项模拟寄存器与回调主机检查及 WCH GCC ELF32 RISC-V UART0 与回调适配器目标对象编译通过；不证明真实寄存器/引脚、电气、波特率误差或数据收发。产品波特率、帧格式、流控、CDC 桥接与 ThreadX 并发仍由 O09/O02/O08 决定 |
| CH585 UART0 bridge callback adapter | software/poc1-ch585-threadx/platform/ch585/dbgc_ch585_uart0_bridge_adapter.c；MCU-001 已分配 UART0 PB4/PB7；generic byte-stream bridge callback contract 与本地 dbgc_ch585_uart0_try_read()/try_write() | 仅将现有轮询读写原语映射为单字节回调，透传空/满状态；不初始化 UART、不增加寄存器操作、不设置 baud/framing，也不含 CDC API 或同步 | 四项单字节回调映射及五项双向通道组合检查纳入 UART0 主机检查总计 94 项；WCH GCC 目标对象编译通过；未接入 CDC/产品固件，实板 UART 行为未验证 |
| CH585 UART0 duplex bridge composition | 同一适配器的 `dbgc_ch585_uart0_duplex_bridge_initialize()`；UART0 PB4/PB7 与通用双向字节流 bridge API | 将调用方 transport read/write callback 分别接到 UART0 TX/RX callback；FIFO 与 UART 配置由调用方提供/完成；无 USB API、默认 baud、任务、ISR 或同步策略 | 主机寄存器模型与 transport callback 检查传输输入到 UART0 TX、UART0 RX 到 transport writer；只验证 mock endpoint 组合，不验证 CDC 或芯片运行 |
| CH585 UID 读取适配器 | `software/poc1-ch585-threadx/platform/ch585/dbgc_ch585_uid.c`；WCH EVT `ISP585.h` 中的 `CMD_GET_ROM_INFO`、`ROM_CFG_MAC_ADDR` 与 `FLASH_EEPROM_CMD()`；`CH58x_flash.c` 的 `GET_UNIQUE_ID()` 字节构造算法 | 检查调用方容量；以两个 `uint32_t` 对齐本地缓冲区，直接调用底层 ROM 命令并检查返回状态；成功后复制 6 字节 MAC 并按 WCH 源码生成末尾 16 位和；失败时不改调用方输出；不编码产品 Device ID、不写 DataFlash、不认证 | 31 项主机 mock 检查、WCH GCC 目标编译、`ld -r` 与 EVT `libISP585.a` 链接检查通过；PoC 已包含适配器和库但应用未调用。主机和交叉构建不执行芯片 ROM 读取，UID 实值仍待 CH585M 实板验证 |
| SWD Engine / Target Manager | MCU-001 的 PB5/PB6 SWD GPIO、PA4 Reset GPIO | SWD 时序、方向切换、电气边界及 Target 电压/保护未形成完整可执行规范 | 暂不实现；需冻结 IF-001 并按 O11/O12 验证时序及电气要求 |
| BLE GATT/配置/OTA | 集成 BLE Radio 与 WCH BLE 示例 | 产品 Service/Characteristic、认证、MTU、配对和 OTA 镜像行为未定 | 暂不实现；需先完成 BLE-001、O06/O07/O10/O13 |
| 私有 2.4G transport | 集成 Radio，DBG-C RF Protocol 目标 | PHY/API、包格式、序列、重试、恢复和时延目标未确定 | 暂不实现；需先完成 RF-001、O01/O03/O12 |
| CH585M ThreadX port | 当前 `software/poc1-ch585-threadx/platform/ch585/` | reset、SysTick、PFIC/VTF/HPE 与上下文切换依赖芯片硬件语义 | 仅实验 PoC；启动、中断、tick、上下文切换、调度、睡眠/唤醒均未做 CH585M 实板验证，保持 O18 开放 |

字节 FIFO 只建立与硬件无关的存储边界。USB、UART、RF 调用方不得直接依赖其内部索引；在同步/并发模型和传输 API 确认前，不把该模块包装成 ISR-safe queue 或产品数据协议。FIFO 主机用例由 `software/common/byte_fifo/tests/test_dbgc_byte_fifo.c` 提供，并由既有 `software/poc1-ch585-threadx/build.sh` 使用本机 C99 编译器构建和运行；当前 3683 项断言通过；单向桥接 15 项、双向桥接 15 项和 UID 适配器 31 项检查通过。该结果不验证并发、ISR、CH585M SRAM 或板级行为。

CMSIS-DAP 测试位于 `software/common/cmsis_dap_host_test/`，由既有 `software/poc1-ch585-threadx/build.sh` 编译固定上游源码副本并运行。准备脚本核对 CMSIS-DAP 提交及原始源码未修改，只在构建目录副本中把 `DAP.h` 的 `__CC_ARM` 延时分支切换到专用测试宏；未定义 `__CC_ARM`，未改动第三方子模块。命令核心的 8 项检查使用空操作引脚及 `SWD_Transfer()` 模拟桩。另将原始上游 `SW_DP.c` 与同一 `DAP.c` 编译到独立测试程序，pin 宏通过回调连接到纯主机位流模型；九项测试检查读成功/奇偶校验错误、写数据及奇偶校验、WAIT/FAULT ACK、10 位 SWD 输入/输出序列及 DAP_SWD_Sequence 命令解析、DAP_Connect 到 DP IDCODE、AP DAP_Transfer 与两项 AP DAP_TransferBlock 端到端响应、方向切换和模型周期数。模型 fast 延时为空操作，周期数不是实测时钟。WCH RISC-V GCC 分别将测试配置下的 `DAP.c` 和 `SW_DP.c` 编译为 ELF32 RISC-V 对象，均未链接 PoC。以上均不验证产品 GPIO HAL、PB5/PB6 电气及时序、USBFS、ThreadX 或 CH585M 板级行为。


### 8.1 WCH USBFS 与 UART0 源码审查

本节记录对归档原始条目的静态审查，不代表这些旧版示例已移植到 DBG-C、已由当前工程编译，或已在 CH585M 实板运行。

| 官方归档条目 | 原始文件证据 | 对 DBG-C 的结论 |
|---|---|---|
| `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_usbdev.h`；SHA-256 `1b5dbf1304127b980d2f38f974e3372c5e55a747fe22319a835156deac1fee0f` | 文件头标记 V1.2、2021/11/17；声明 `USB_DeviceInit()`、`USB_DevTransProcess()`、EP1–EP4 IN/OUT 处理函数；描述 EP0 与 EP1–EP4 的 64-byte 缓冲布局 | 可确认归档中存在 WCH USBFS Device 底层接口和端点缓冲示例；不能据此确认 ThreadX ISR/任务同步或 DAP + CDC 复合设备实现 |
| `EVT/EXAM/SRC/StdPeriphDriver/CH58x_usbdev.c`；SHA-256 `dfef009a88c6c3c70670889bbab43898264967f979dc56b4c68180542b4ea540` | 文件头标记 V1.2、2021/11/17；`USB_DeviceInit()` 配置 4 个非零端点的双向通道，设置 DMA 地址、ACK/NAK 状态和 USB 中断使能 | 是直接寄存器级示例；不能当作已验证的 ThreadX-safe USB Service/HAL |
| `EVT/EXAM/USB/Device/COM/src/Main.c`；SHA-256 `4345ca31938ab30bf4c772e1ceeaccbd717c6a82c8a26012ef8708280fbcfda4` | 文件头标记 V1.0、2020/08/06；含 CDC 描述符及 CDC/厂商两套配置；CDC 配置使用 EP4 中断 IN、EP1/EP2 Bulk；运行时由 `usb_work_mode` 在 CDC 与厂商模式间选择；USB ISR 直接读写寄存器并调用 PFIC 屏蔽/恢复中断 | 证明归档有 CDC Device 示例，不证明同一配置已集成 CMSIS-DAP v2 Bulk；两个模式不是该例程中的同时复合配置。端点映射、描述符、IRQ 与 ThreadX 集成须重新设计并实测 |
| `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_uart.h`；SHA-256 `99e419e0906b4fdaba12fd1aea3cdf58109cfd8906b1508d69e5dd884c741f70` | 文件头标记 V1.2、2021/11/17；声明 UART0 波特率、触发级别、中断配置、收发接口和状态宏 | 可确认归档 API 名称；不证明 PB4/PB7 的目标工程复用初始化或 ISR 到 ThreadX 的同步安全性 |
| `EVT/EXAM/SRC/StdPeriphDriver/CH58x_uart0.c`；SHA-256 `f0b6b252dcb69530428f32cf2bb50cb585fbecd6f5cfd0eb57f194e257d86338` | 文件头标记 V1.2、2021/11/17；默认初始化把波特率设为 115200、FIFO 触发级别字段设为 4-byte、使能 TXD 中断；收发字符串函数轮询 FIFO 状态/计数 | 默认值不是 DBG-C 冻结参数。新 BSP 不调用该初始化或字符串 API；按可核对的除数计算与寄存器定义实现显式参数、关闭中断的轮询原语，不承诺产品桥接或并发安全 |
| `DebugInit()` 的 USB COM 示例与 ADC 示例；`EVT/EXAM/ADC/src/Main.c` SHA-256 `0b8d738f673acf5cf42145f45a9d57438065bfe06e71abcd47248810f753115a` | USB COM 示例将 UART0 重映射到 PA15/PA14；ADC 示例在 `ADC_SCAN_MODE_EXAM` 分支先置 PB7，再设 PB4 输入上拉、PB7 5 mA 推挽输出并调用 `UART0_DefInit()`。本地 `CH585SFR.h` 将 `RB_PIN_UART0` 定义为 0 选 PB4/PB7、1 选 PA15/PA14；EVT `GPIOPinRemap()` 通过设置/清除该位选择复用 | PB4/PB7 初始化证据已取得并用于 BSP。仅复用引脚模式和路由证据；WCH 默认波特率及 TX 中断不成为产品参数。真实 GPIO、复用及 UART 行为仍须板测 |

CH585 UART0 适配器已将轮询回调组合到通用双向桥接服务，并通过模拟 UART 寄存器和 transport callback 主机检查；该检查没有运行 USB CDC 或 CH585M 芯片。通用字节 FIFO 是已实现且可复用的硬件无关产品模块。CH585 SWD GPIO BSP 按 MCU-001 和 WCH EVT GPIOB 实现封装 PB5/PB6；CH585 Target Reset GPIO BSP 按 MCU-001 和 WCH EVT GPIOA 实现封装 PA4；UART0 BSP 仅封装分配给 PB4/PB7 的显式参数轮询原语。主机寄存器模型分别通过 SWD GPIO 57 项、Target Reset GPIO 33 项和 UART0 94 项检查；WCH GCC 为三个 BSP 及 UART0 桥接适配器生成 ELF32 RISC-V 目标对象。它们均未链接进 PoC 或 CMSIS-DAP。模拟寄存器检查不证明芯片寄存器、引脚电气、目标复位或串口实际收发。两个 GPIO BSP 均要求调用方显式选择模式；PA4 API 只传递原始高/低电平，不定义复位极性/脉宽/默认态。目标电压兼容、输出能力、保护与 SWD/RESET 时序仍按 IF-001/O11/O12 保持待决，并须在验证板测量。CMSIS-DAP 命令核心及上游 SWD 位级算法已有主机模型检查，但 host-test pin 回调仍是验证夹具。UART0 BSP 不含 UART 中断处理、队列或并发保护，不能由多个任务、ISR 或 CDC 并发调用；产品波特率、帧格式、流控和 CDC 桥接仍缺冻结参数及验证。USBFS + CDC、BLE 和私有 RF 仍缺产品接口/调度/硬件证据。O02、O08、O09、O11、O18、O20 与 O21 保持开放，所有 ThreadX/USB/UART ISR 行为仍需实板验证。

### 8.3 上游 SWD 引擎主机线模型与 WCH GPIO 证据

固定 CMSIS-DAP 提交中的 `Firmware/Source/SW_DP.c` 提供 SWJ/SWD 序列与 `SWD_Transfer()` 位级算法。测试配置通过 `PIN_SWCLK_TCK_*`、`PIN_SWDIO_*` 宏连接到测试回调；主机位流模型提供 ACK、数据和奇偶校验输入，并记录输出位、时钟上升调用及输入/输出方向调用。读取用例覆盖正确和错误奇偶校验。测试选择 fast-clock 分支，测试 `__NOP()` 为空操作；因此模型中的 46 次时钟置高调用不是频率或时序测量。

WCH EVT 官方归档 GPIO 文件 `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_gpio.h`（SHA-256 `c7f450ceaa501e4c5a912bc0429bca1540fa3c3f26e3fd55dfd908fad9b06183`）声明 `GPIOA_ModeCfg(uint32_t pin, GPIOModeTypeDef mode)` 与 `GPIOB_ModeCfg(uint32_t pin, GPIOModeTypeDef mode)`，并定义 GPIOA/GPIOB 的置位、清零与引脚读取宏。`EVT/EXAM/SRC/StdPeriphDriver/CH58x_gpio.c`（SHA-256 `c2c7c9b2518a2a56144f107776418911ae0849df523481ee7b999cbdfab01757`）给出 GPIOA/GPIOB 模式配置实现：输入浮空/上拉/下拉及输出 5mA/20mA 模式操作相应端口的 PD_DRV、PU、DIR 寄存器。本地 `CH585SFR.h` 声明 PB/PA 对应寄存器。MCU-001 将 PB5/PB6 分配给 SWDIO/SWCLK、PA4 分配给 Target_nRESET。对应 BSP 将寄存器访问限制在 CH585 层，并要求调用方显式选择模式；PA4 原语仅写入/读取原始逻辑电平，不实现复位脉冲策略。主机构建以模拟变量检查同一 BSP 源码：SWD GPIO 57 项、Target Reset GPIO 33 项；WCH GCC 为两者分别生成 ELF32 RISC-V 对象。它们均不访问真实 GPIO，也不链接 PoC/DAP，不能证明芯片引脚、电气兼容、复位效果或时序。O11 的模式、电平、保护与脉冲验收要求仍待决策和验证板测量。

### 8.2 CMSIS-DAP 上游源码固定与适配缺口

Arm 官方 CMSIS-DAP 仓库已作为 Git 子模块固定到提交 `12636590eec66fae2d1bba4518749426ad5a4595`。该提交 `Firmware/Include/DAP.h` 声明 `DAP_ProcessCommand()` 与 `DAP_ExecuteCommand()`，并通过 `DAP_FW_VER` 声明固件版本 2.1.2。此源码是 CMSIS-DAP 固件实现，不是 DAPLink 完整固件体系。上游 `Firmware/Config/DAP_config.h` 是面向示例工程的模板：其中 CPU 时钟、JTAG 能力、DAP 包尺寸及缓冲包数的默认值不得直接用于 DBG-C。

当前子模块的 `DAP.h` 包含 `cmsis_compiler.h`；该文件既不在 CMSIS-DAP 子模块，也未出现在仓库提供的 CH585EVT ZIP 条目中。该头文件的 `PIN_DELAY_SLOW()` 在非 `__CC_ARM` 分支使用 Arm `subs %0,%0,#1` 内联汇编，WCH RISC-V GCC 不能直接汇编该指令。不得伪定义 `__CC_ARM`；测试脚本只在构建目录的头文件副本中选择上游 C 循环分支。本地 `platform/ch585/cmsis_compiler.h` 基于 WCH Core 头及 GCC/RISC-V 编译器基元提供 CMSIS inline、NOP 和 weak-symbol 宏映射；构建脚本用 WCH GCC 编译该映射检查，并在测试配置下将 `DAP.c`、`SW_DP.c` 编译为 ELF32 RISC-V 对象。测试仍使用 no-op pin 宏，且这些对象未链接 PoC。此结果只验证编译器基元，不是完整 CMSIS-DAP 产品移植，不证明 GPIO、时序、目标 ACK 或 CH585M 实板运行。

#### 请求长度边界

固定上游 `DAP.h` 中 `DAP_ExecuteCommand()` 与 `DAP_ProcessCommand()` 只接收请求和响应指针；`DAP.c` 的 Transfer、TransferBlock、SWD/JTAG Sequence 根据请求字段遍历变长数据。`DAP_ExecuteCommands` 读取请求中的命令计数并据各子命令返回长度推进指针，没有可用输入长度或响应容量参数。命令 ID `0x80` 至 `0x9F` 会分派到源码标为可覆盖的 `DAP_ProcessVendorCommand()`；产品是否覆盖、接受哪些厂商命令尚未确定。`DAP_Info()` 调用字符串回调时只传入 `char *` 并接收 8 位长度，回调接口没有输出容量参数，产品字符串最大长度尚未定义。

新增 `software/common/cmsis_dap_bounds/` 纯软件边界预检及 dispatch：使用固定上游命令 ID/标志解析受支持的固定及变长标准命令，计算请求消费长度与响应最大值；dispatch 仅在预检成功后调用传入的 CMSIS-DAP 执行函数，并核对其返回的消费/响应长度。调用后的长度核对不能阻止超出 profile 的回调写入，故必须在产品配置冻结前核实每个 Info 回调的最大写入量。vendor handler、SWO、CMSIS-DAP UART 命令及 Info 上限为零时失败关闭。102 项预检主机检查及 5 个 dispatch 主机用例通过；其中截断 Transfer 请求和响应容量不足均在上游调用前被拒绝。WCH GCC ELF32 RISC-V 对象编译通过；复现命令：`DBGC_BUILD_DIR=build/cmsis-dap-bounds-dispatch software/poc1-ch585-threadx/build.sh`。dispatch 尚未接入产品 USB 收包路径，也未链接 PoC；显式 profile 是测试夹具，不代表产品设置。

仓库尚无 USB 收包路径或 DBG-C 产品 `DAP_config.h`。主机夹具中的 `DAP_PACKET_SIZE=64`、`DAP_PACKET_COUNT=1` 和空字符串回调仅供测试，不能作为产品值。产品启用命令/功能、vendor 命令策略、各 Info 回调的最大写入量、USB 实际接收长度、请求/响应容量，以及最终编译配置到 profile 的映射仍待确定。主机检查证明通用 wrapper 会先预检再调用固定上游解析器；但不证明产品 USB 调用方会使用该 wrapper，也不证明回调遵守配置上限；O21 保持开放。


### CH585 UID ROM 命令适配

WCH EVT `EVT/EXAM/SRC/StdPeriphDriver/inc/ISP585.h` 的 SHA-256 为 `244b966e79381b7ebf47c0a4942f001ee066c2cc9d32538cf7d4296984c81db8`；其中定义 `CMD_GET_ROM_INFO`、`ROM_CFG_MAC_ADDR` 并声明 `FLASH_EEPROM_CMD()`，文档规定 0 表示成功、非 0 表示失败，Buffer 位于 RAM 且 4 字节对齐。EVT `CH58x_flash.c` 的 `GET_UNIQUE_ID()` 对这条命令的长度参数传 0，接收 MAC 字节后将前三个 16 位小端字求和并写入 UID 的末两字节。WCH `libISP585.a` 的 SHA-256 为 `8ede32a24e09344e86ecd74731069e19d6befa287bd6de2dd7c757c6f1d06a9e`；归档 ADC 工程 `.cproject` 链接该库，符号表确认它定义 `FLASH_EEPROM_CMD`。

DBG-C 适配器直接调用底层命令以保留错误状态，且仅在命令成功后构造 UID 并复制给调用方。主机替身检查参数、对齐、输出构造及失败不改写调用方缓冲区；WCH RISC-V `ld -r` 检查消除 `FLASH_EEPROM_CMD` 未解析引用，PoC 全量构建通过。PoC `main` 尚未调用该适配器；这不证明 ROM 命令在 CH585M 实际执行成功或读值符合唯一性/稳定性预期。
