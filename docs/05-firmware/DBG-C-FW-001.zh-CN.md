# DBG-C Firmware Architecture 与 PoC-1 记录

**文档编号：** DBG-C-FW-001　**版本：** V0.2　**状态：** 实验草案；主机交叉构建通过，CH585M 板级运行未验证

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
| ThreadX tick | PoC 使用上游 `tx_api.h` 默认 `TX_TIMER_TICKS_PER_SECOND`，其定义为 100 tick/s；SysTick ISR 调用 `_tx_thread_context_save`、`_tx_timer_interrupt`，清除 EVT 头文件定义的 SysTick `SR` | ISR 汇编能构建；HPE/VTF、异常返回和调度语义尚未验证 |
| PoC 线程 | 两个同优先级线程递增独立计数器，记录 `tx_time_get()` 并执行 `tx_thread_sleep(1U)` | 仅是待板测的可观测量设计 |
| ELF 构建 | `text=8876`、`data=8`、`bss=5564` 字节；ELF32 RISC-V，入口地址 `0x0` | 当前主机的一次交叉构建，日志见 `software/poc1-ch585-threadx/build-evidence.log`；不等于烧录或运行证据 |
| CH585M 板级运行 | 未执行 | 尚无已确认连接的 CH585M 板卡与下载/调试路径 |

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

需要 Git、CMake 3.20 或更新版本、GNU Make 和 MounRiver Linux x64 RISC-V Embedded GCC 12。当前验证环境记录为 Git 2.43.0、CMake 3.28.3、GNU Make 4.3、GCC 12.2.0、GNU assembler/linker 2.38。Ninja 非必需。

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

原始资料保存在 [`CH585EVT 归档说明`](../09-references/CH585EVT/README.md)所指的官方 EVT 压缩包内。下列散列针对 ZIP 中原始条目；仓库中的 WCH 头文件仅规范化换行符和行尾空白，其他代码保持原样。时钟初始化文件是依据所列 WCH 源文件摘取函数实现，版权与使用声明保留。

| PoC 文件 | WCH EVT 原始条目 | 原始 SHA-256 |
|---|---|---|
| `platform/ch585/startup/startup_CH585.S` | `EVT/EXAM/FreeRTOS/Startup/Startup_CH585_FreeRTOS.S` | `48248d39086a92c9a5c5d57bf5dce4c64cf3ce83d6de0ef14c14e1d6a9bdef49` |
| `platform/ch585/ld/ch585.ld` | `EVT/EXAM/FreeRTOS/Ld/Link.ld` | `a5e7454144693e4b1af34cc5f04c7a7c9277664102285f08f731fd9f93551598` |
| `platform/ch585/wch/CH585SFR.h` | `EVT/EXAM/SRC/StdPeriphDriver/inc/CH585SFR.h` | `b1ee193c9037813758d72f47bfecb02046aee0edcd0129e0f8e73bd806788de3` |
| `platform/ch585/wch/CH58x_clk.h` | `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_clk.h` | `7e5de3bbd5edb11c1d6fff2bff1b33de5725777dc7128f29dee6c7624db207da` |
| `platform/ch585/wch/CH58x_sys.h` | `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_sys.h` | `1ce59c4b44c30d59dea204be0ae907b6dc66938f0bfdb3ce3567bb9c46eb1a51` |
| `platform/ch585/wch/core_riscv.h` | `EVT/EXAM/SRC/RVMSIS/core_riscv.h` | `fd0099eb7fce96c1ae1c0c5c58fd5da0d8f6b6b167e2c3fb335d5910b0a5503e` |
| `platform/ch585/highcode_init.c` clock sequence | `EVT/EXAM/SRC/StdPeriphDriver/CH58x_sys.c` | `954a8b489c8b333d482ed65e0665a1260c5f7976398ef4878f4f928251f62772` |

工程背景证据：`EVT/CH585_List_EN.txt` SHA-256 `112318f05aa8764f591dcea84333fba3a0e978e234c9c2d4f9086dc915c27fc9`；FreeRTOS `readme.txt` SHA-256 `ae231c158445d7d68ef7b0e534100c24afcccac903bf2475f5647c9903d1d353`；`.project` SHA-256 `e98d5f026b4f06d2cb6a46933c5aa85425ccdeb5a4941fbb8a40a7a0b94c8b41`；`.cproject` SHA-256 `158d9e4ff583f3bff39be61a5b19270f976ecc1d10dd5ce17170281c0d1bab81`。FreeRTOS 样例中的 `FreeRTOSConfig.h` tick 为 500 Hz；本 PoC 不复用该值。ThreadX 使用其上游头文件定义的默认 100 tick/s。

## 5. 链接、时钟和中断边界

WCH 示例链接脚本定义 FLASH 起始 `0x00000000`、长度 448 KiB，RAM 起始 `0x20000000`、长度 128 KiB。它们是所引用 EVT 样例的链接区域配置；板上芯片修订、内存映射适用性及实际运行占用仍待确认。

WCH 示例的 `highcode_init()` 初始化 HSI PLL 到 62.4 MHz，并配置相关安全访问与 Flash 时钟字段。PoC 把该函数体摘取到 `highcode_init.c`，不能据此认定板上系统时钟已经达到 62.4 MHz。

当前 SysTick ISR 汇编入口位于 WCH startup 使用的 highcode 区，执行流程为：建立 ThreadX 32-word 通用端口中断帧、根据 WCH FreeRTOS 端口方式清除 HPE 控制位、调用 ThreadX 上下文保存与计时器中断函数、清除由 WCH `core_riscv.h` 定义的 SysTick 状态寄存器，再跳转到 ThreadX 上下文恢复入口。具体 QingKe 异常入口是否已自动压栈、VTF 的入口/返回规则、HPE 对状态保存的影响、栈帧是否与 ThreadX 上游实现匹配，以及调度返回是否正确，均须用实板验证；主机链接不能证明这些行为。

## 6. 构建结果、验收和剩余工作

本轮主机交叉构建通过，证据记录在 `software/poc1-ch585-threadx/build-evidence.log`：ThreadX 精确修订 `b91b03b9e75fa523b17127f9e0eca09dca916459`，ELF 为 ELF32 RISC-V，入口 `0x0`，text 8876、data 8、bss 5564 字节。没有执行烧录、板上中断/调度、时钟测量或长时间运行测试。

完成 PoC-1 仍需：

1. 在具体 CH585M 板卡上确认编程器、下载流程、芯片修订和启动行为。
2. 观察 `thread_a_runs`、`thread_b_runs`、`threadx_tick_observed` 的持续变化；验证 `tx_thread_sleep(1U)` 的超时/唤醒、同优先级轮转及系统复位后的稳定性。
3. 实测 SysTick 频率、时钟稳定性、VTF/PFIC/HPE 行为和异常栈完整性；保存固件散列、板卡版本、下载日志和运行日志。
4. 验证中断嵌套、调度抢占和长时间运行后，才能将端口状态改为板级运行通过，并同步 MCU、FW、TEST、OPEN 文档。

## 7. 资料来源

- Eclipse ThreadX 官方仓库与 MIT 许可：[`software/third_party/threadx/`](../../software/third_party/threadx/)，标签和提交见第 1 节。
- WCH CH585 EVT：[`归档说明`](../09-references/CH585EVT/README.md)及本节列出的归档文件与散列。
- MounRiver Linux x64 Toolchain V2.4.0：[官方下载页](https://www.mounriver.com/download)，工具链包 SHA-256 见第 3 节。
