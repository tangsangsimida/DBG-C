# DBG-C Firmware Architecture 与 PoC-1 记录

**文档编号：** DBG-C-FW-001　**版本：** V0.1　**状态：** 实验草案；构建已验证，CH585M 上的运行、时基和抢占未验证

## 1. 范围与状态

本阶段只验证 CH585M 工程能否引用 Eclipse ThreadX 正式源码，并用当前 MounRiver RISC-V 工具链完成固件 ELF 构建。PoC 源码位于 [`software/poc1-ch585-threadx/`](../../software/poc1-ch585-threadx/)，上游仓库作为 Git 子模块位于 [`software/third_party/threadx/`](../../software/third_party/threadx/)。它不是 DBG-C V1 固件，也未实现 USB、CMSIS-DAP、SWD、CDC、BLE、私有 2.4G 或 OTA。

| 项目 | 已知状态 | 证据边界 |
|---|---|---|
| ThreadX | 正式标签 `v6.5.1.202602a_rel`，提交 `b91b03b9e75fa523b17127f9e0eca09dca916459` | 子模块检出记录；仓库含 MIT `LICENSE.txt` |
| ThreadX 端口 | 构建使用上游 `ports/risc-v32/gnu` 通用端口 | 可编译不代表适配 QingKe V3C 的异常入口、硬件压栈或上下文切换 |
| WCH 芯片资料 | 使用仓库归档 `docs/09-references/CH585EVT/CH585EVT.ZIP` 中 FreeRTOS 示例的启动汇编和链接脚本 | 只复制实际使用文件；文件保留 WCH 原有版权/使用声明 |
| 工具链 | MounRiver Studio Linux x64 Toolchain V2.4.0 内的 RISC-V Embedded GCC 12；GCC 12.2.0，GNU assembler/linker 2.38 | 编译器前缀实测为 `riscv-wch-elf-`；不要把 EVT `.cproject` 中的 `riscv-none-embed-` 当成本 PoC 的实际前缀 |
| 编译 | `dbgc_poc1.elf` 成功链接；text 7124、data 8、bss 5560 字节 | 当前主机、当前工具链的一次构建记录；不是目标板运行证据 |
| ThreadX tick | 未接入 | WCH 启动文件的默认 `SysTick_Handler` 是无限循环；当前没有设置 SysTick，也未接入 `_tx_timer_interrupt` |
| CH585M 板上调度 | 未执行 | 当前没有确认连接的 CH585M 开发板；本机 `lsusb` 未显示可确认的 CH585 调试/下载设备 |

PoC 的两个同优先级线程分别递增 `thread_a_runs` 和 `thread_b_runs`，再调用 `tx_thread_relinquish()`。这两个 `volatile` 计数器便于调试器观察，但在当前板级中断和启动适配未完成前，不得据此断言它们已在 CH585M 上运行。

## 2. 软件分层与当前目录

```text
Application: PoC main and two test threads
ThreadX common kernel: official submodule common/
CPU port: official RISC-V32/GNU assembly sources
CH585 platform: WCH startup and linker inputs plus local adapter area
Build: CMake and MounRiver WCH RISC-V GCC 12
```

当前构建通过 ThreadX 官方 CMake 自定义端口入口引用上游通用 RISC-V32/GNU 文件；没有复制或修改 ThreadX 上游源码。`platform/ch585/threadx_port/CMakeLists.txt` 只是源码选择清单，不等于完成 CH585 端口。WCH 启动汇编仍来自 EVT FreeRTOS 工程，其中 `highcode_init()` 当前是本地空实现，SysTick 向量保持 WCH 文件提供的默认停机处理。

## 3. 主机环境配置

### 3.1 依赖

需要 Git、CMake 3.20 或更新版本、GNU Make，以及 MounRiver 的 Linux x64 RISC-V Embedded GCC 12 工具链。当前构建机已确认：Git 2.43.0、CMake 3.28.3、GNU Make 4.3、GCC 12.2.0、GNU assembler/linker 2.38。Ninja 不是必需项。

### 3.2 安装 MounRiver 工具链

从 [MounRiver 官方下载页](https://www.mounriver.com/download)取得文件 `MRS_Toolchain_Linux_X64_V240.tar.xz`。下载页为动态页面；下载文件名和校验值如下，下载后必须先核对 SHA-256：

```text
1fae593d27e24466f17c2df0fd00f746143f587fe33e912a78e35142fef82a6d
```

以下命令选择性提取需要的编译器目录，不安装整个 MounRiver IDE：

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

工具链压缩包约 411 MB。构建配置默认使用此用户级安装路径；若使用其他路径，在 CMake 配置时通过 `DBGC_WCH_TOOLCHAIN_ROOT` 指定实际 `bin` 目录。

### 3.3 获取源码与构建

```bash
git clone --recurse-submodules <DBG-C 仓库地址>
cd DBG-C
git submodule update --init --recursive
software/poc1-ch585-threadx/build.sh
```

如果工具链装在其他目录：

```bash
cmake -S software/poc1-ch585-threadx \
  -B software/poc1-ch585-threadx/build \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/software/poc1-ch585-threadx/cmake/wch-riscv32.cmake" \
  -DDBGC_WCH_TOOLCHAIN_ROOT="/实际路径/RISC-V Embedded GCC12/bin" \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build software/poc1-ch585-threadx/build --parallel
```

Build 输出保留在 `software/poc1-ch585-threadx/build/`，由该 PoC 子目录自己的 `.gitignore` 排除；根目录 `.gitignore` 不需要修改。

## 4. 芯片启动与资源边界

当前复制的 WCH 文件来源：

| 仓库内文件 | EVT 原始路径 | 原始归档 SHA-256 |
|---|---|---|
| `software/poc1-ch585-threadx/platform/ch585/startup/startup_CH585.S` | `EVT/EXAM/FreeRTOS/Startup/Startup_CH585_FreeRTOS.S` | `48248d39086a92c9a5c5d57bf5dce4c64cf3ce83d6de0ef14c14e1d6a9bdef49` |
| `software/poc1-ch585-threadx/platform/ch585/ld/ch585.ld` | `EVT/EXAM/FreeRTOS/Ld/Link.ld` | `a5e7454144693e4b1af34cc5f04c7a7c9277664102285f08f731fd9f93551598` |

仓库副本仅规范化行结束符和纯空白；表中 SHA-256 对应 ZIP 内原始文件字节。

链接脚本按该 WCH 示例定义 FLASH 从 `0x00000000` 起、长度 448 KiB，RAM 从 `0x20000000` 起、长度 128 KiB。它们是所引用 EVT 样例的链接区域参数，不构成对具体 DBG-C PCB、封装/芯片批次或全部可用 RAM 的新验证。

WCH EVT FreeRTOS 启动配置 PFIC/VTF、`mtvec` 和 QingKe 的 CSR；其 FreeRTOS 移植还使用软件中断切换任务，并在上下文保存时调整 HPE。ThreadX 上游通用端口假定标准 RISC-V M-mode 异常入口和软件栈帧。两者的中断入口与上下文模型存在尚未验证的集成边界，不能仅凭成功链接就复用。当前 `highcode_init()` 空实现也没有设置系统时钟；不得将 SDK 默认/样例主频视为本 PoC 实际运行频率。

## 5. 验收记录和后续工作

已完成的仅为主机交叉编译验收：CMake 配置成功，ThreadX 公共源码与 RISC-V 汇编全部编译，ELF 链接成功，输出大小为 text 7124、data 8、bss 5560 字节。构建命令为 `software/poc1-ch585-threadx/build.sh`。未执行烧录或硬件测试。

继续做 PoC-1 之前需要：

1. 基于 CH585 官方参考手册和 EVT 源码完成 ThreadX 的启动入口、系统栈/未用内存边界、异常/中断栈帧、HPE 设置与上下文保存/恢复适配；保留第三方版权声明。
2. 将 SysTick 配置成 ThreadX 系统时钟，定义不会与 ThreadX 调度冲突的 ISR 入口，处理 tick 清除与抢占路径；所有具体寄存器/API 先从 EVT/参考手册核准。
3. 移除 WCH startup 的弱默认 SysTick 死循环覆盖，设置经过验证的时钟，并让链接脚本为 ISR/系统栈及两个线程栈留出明确边界。
4. 在 CH585M 板上观测两个线程计数器、ThreadX tick、线程睡眠超时/唤醒、抢占和连续运行；记录板卡版本、晶振/时钟配置、下载方式、固件散列、测试日志和复位结果。
5. 成功完成板级验证后再把端口状态提升为“PoC-1 运行验证通过”，并更新 MCU、FW、TEST 和 OPEN 文档。

## 6. 来源

- Eclipse ThreadX 官方仓库及 MIT 许可：[`software/third_party/threadx/`](../../software/third_party/threadx/)，标签 `v6.5.1.202602a_rel`，提交 `b91b03b9e75fa523b17127f9e0eca09dca916459`。
- WCH 官方 EVT 压缩包：[`CH585EVT 归档说明`](../09-references/CH585EVT/README.md)；只据其中上述 FreeRTOS 启动和链接文件记录具体移植证据。
- MounRiver 官方下载页：[MounRiver Studio Downloads](https://www.mounriver.com/download)，Linux x64 Toolchain V2.4.0；本机下载包 SHA-256 如上。
