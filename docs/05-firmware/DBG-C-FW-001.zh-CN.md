# DBG-C Firmware Architecture 与 PoC-1 记录

**文档编号：** DBG-C-FW-001　**版本：** V0.8　**状态：** 实验草案；通用 FIFO 模块及 3678 项主机断言已通过；验证板设计门已通过；tick 目标冲突待决策；实板运行及产品硬件冻结未通过

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
- WCH CH585 EVT：[`归档说明`](../09-references/CH585EVT/README.md)及本节列出的归档文件与散列。
- MounRiver Linux x64 Toolchain V2.4.0：[官方下载页](https://www.mounriver.com/download)，工具链包 SHA-256 见第 3 节。

## 8. 软件模块实现状态

资源依据见 [MCU-001 第 5 节](../04-hardware/DBG-C-MCU-001.zh-CN.md)：USB CDC/UART、RF 收发均需要 SRAM 数据缓冲。官方 EVT 归档的 `EVT/EXAM/BLE/BLE_UART/APP/app_drv_fifo/app_drv_fifo.c` 与 `.h`（原始 SHA-256 分别为 `312e039857300b5142f6ae6898a592b6257170d146f13a92e02bd8808bc5ec67`、`5ce231b33f9a5897a6be2abddc1901cd181f63c93c3fa00867216ac14cc8b6cc`）及 `EVT/EXAM/BLE/BLE_UART/APP/peripheral.c` 证明 WCH 示例采用字节 FIFO 缓冲 UART 数据。该示例不证明其并发临界区适配 ThreadX；本项目未直接复制或调用该 FIFO。

| 模块 | 软件位置/资源依据 | 当前实现边界 | 状态/验证 |
|---|---|---|---|
| 通用字节 FIFO | `software/common/byte_fifo/`；对应 MCU-001 规划的 USB CDC、UART 与 RF 收发缓冲 | 调用方提供固定存储；支持任意非零容量、部分读写、不覆盖未读数据、清空与容量查询；无动态分配、无芯片寄存器/中断/ThreadX API；不保证并发安全，调用方必须串行化访问 | 已加入现有 CMake 构建并通过 CH585 交叉编译；既有构建脚本中的主机验证运行通过 3678 项断言；当前未接入 USB/UART/RF，未做实板测试；不是冻结的产品 ABI |
| CMSIS-DAP / DAP command core | MCU-001 USBFS 分配；PRD 的 CMSIS-DAP v2 目标 | 仓库没有 CMSIS-DAP 源码或已选定版本，USB/DAP 接口和 VID/PID 也未冻结 | 暂不实现；需先锁定上游源码/版本并完成 USB-001 及 O02/O08 |
| USBFS Device + CDC transport | MCU-001 USBFS、PB10/PB11；WCH EVT 含 USB 示例 | Endpoint、描述符、WCH USB Device API、WinUSB 与组织 VID 尚未确定 | 暂不实现；按 O02/O08 取证并定义 USB-001 后再做 |
| UART0 driver/bridge | MCU-001 UART0、PB4/PB7 | WCH UART API/复用初始化、波特率/流控和 ISR 到 ThreadX 的同步规则待核实 | 暂不实现；需核对 EVT 的 CH585 UART 例程并确定 CDC/UART 行为（O09） |
| SWD Engine / Target Manager | MCU-001 的 PB5/PB6 SWD GPIO、PA4 Reset GPIO | SWD 时序、方向切换、电气边界及 Target 电压/保护未形成完整可执行规范 | 暂不实现；需冻结 IF-001 并按 O11/O12 验证时序及电气要求 |
| BLE GATT/配置/OTA | 集成 BLE Radio 与 WCH BLE 示例 | 产品 Service/Characteristic、认证、MTU、配对和 OTA 镜像行为未定 | 暂不实现；需先完成 BLE-001、O06/O07/O10/O13 |
| 私有 2.4G transport | 集成 Radio，DBG-C RF Protocol 目标 | PHY/API、包格式、序列、重试、恢复和时延目标未确定 | 暂不实现；需先完成 RF-001、O01/O03/O12 |
| CH585M ThreadX port | 当前 `software/poc1-ch585-threadx/platform/ch585/` | reset、SysTick、PFIC/VTF/HPE 与上下文切换依赖芯片硬件语义 | 仅实验 PoC；启动、中断、tick、上下文切换、调度、睡眠/唤醒均未做 CH585M 实板验证，保持 O18 开放 |

字节 FIFO 只建立与硬件无关的存储边界。USB、UART、RF 调用方不得直接依赖其内部索引；在同步/并发模型和传输 API 确认前，不把该模块包装成 ISR-safe queue 或产品数据协议。FIFO 主机用例由 `software/common/byte_fifo/tests/test_dbgc_byte_fifo.c` 提供，并由既有 `software/poc1-ch585-threadx/build.sh` 使用本机 C99 编译器构建和运行；当前 3678 项断言通过。该结果不验证并发、ISR、CH585M SRAM 或板级行为。
