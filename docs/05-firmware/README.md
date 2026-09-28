# 固件文档 | Firmware Documents

- [DBG-C-FW-001 固件架构与 PoC-1 记录（中文）](DBG-C-FW-001.zh-CN.md)
- [DBG-C-FW-001 Firmware Architecture and PoC-1 Record (English)](DBG-C-FW-001.en-US.md)

DBG-C-FW-001 V0.26 记录了 ThreadX CH585M 实验性 low-level 与 SysTick 接入、WCH EVT 来源、MounRiver GCC 12 环境配置、可复现主机 ELF 构建与静态审查；通用 FIFO 主机 3683 项断言、CMSIS-DAP 命令核心 8 项、SWD 引擎线模型 9 项检查通过，并新增基于 WCH EVT GPIOB 模式配置语义与 MCU-001 PB5/PB6 分配的 SWD GPIO BSP。该 BSP 仅通过 ELF32 RISC-V 目标对象编译，未链接 PoC 或 CMSIS-DAP，也未实板运行；电气模式、SWD 时序、USB/UART 集成仍未验证。ThreadX 的时钟、中断入口、tick 与线程调度仍须 CH585M 实板验证；产品硬件冻结未放行。

DBG-C-FW-001 V0.26 records the experimental ThreadX CH585M low-level and SysTick integration, WCH EVT provenance, MounRiver GCC 12 setup, reproducible host ELF builds, and static review. The generic FIFO has 3683 passing host assertions; eight CMSIS-DAP command-core cases and nine SWD-engine line-model cases pass. A CH585 SWD GPIO BSP based on WCH EVT GPIOB mode semantics and the MCU-001 PB5/PB6 allocation has also been added. It only passes ELF32 RISC-V target-object compilation; it is not linked into PoC or CMSIS-DAP and has not run on hardware. Electrical modes, SWD timing, and USB/UART integration remain unverified. ThreadX clock, interrupt entry, tick, and thread scheduling still require CH585M board testing; product-hardware freeze is not released.
