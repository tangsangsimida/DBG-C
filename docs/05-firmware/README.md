# 固件文档 | Firmware Documents

- [DBG-C-FW-001 固件架构与 PoC-1 记录（中文）](DBG-C-FW-001.zh-CN.md)
- [DBG-C-FW-001 Firmware Architecture and PoC-1 Record (English)](DBG-C-FW-001.en-US.md)

DBG-C-FW-001 V0.23 记录了 ThreadX CH585M 实验性 low-level 与 SysTick 接入、WCH EVT 来源、MounRiver GCC 12 环境配置、可复现主机 ELF 构建与静态审查；通用 FIFO 主机 3683 项断言及 CMSIS-DAP 命令核心 8 项及上游 SWD 引擎集成线模型 9 项检查（读写数据、序列、DAP 命令及 SWD 序列/AP posted-read 集成、WAIT/FAULT ACK）（含 TransferConfigure 重试数及 Transfer/TransferBlock 的 AP posted-read）通过，另以测试专用配置生成启用 SWD 分支的 ELF32 RISC-V 对象。产品 GPIO HAL、电气时序及 USB 集成仍未验证；WCH USBFS/UART0 源码审查和移植边界亦有记录。验证板设计门已通过；CH585M 板上的时钟、中断入口、tick 与线程调度仍未验证，产品硬件冻结未放行。

DBG-C-FW-001 V0.23 records the experimental ThreadX CH585M low-level and SysTick integration, WCH EVT provenance, MounRiver GCC 12 setup, reproducible host ELF builds, and static review. The generic FIFO has 3683 passing host assertions; eight CMSIS-DAP command-core cases and nine CMSIS-DAP-to-SWD-engine integration line-model cases pass, covering reads/writes, sequences, and WAIT/FAULT ACKs, including TransferConfigure retry-count behavior and AP posted-read handling in Transfer and TransferBlock, and a test-only ELF32 RISC-V object was built with the SWD branch enabled. Product GPIO HAL, electrical timing, and USB integration remain unverified. The document also records WCH USBFS/UART0 source review and porting boundaries. The verification-board design gate has passed; clock, interrupt entry, tick, and thread scheduling on CH585M remain unverified, and product-hardware freeze is not released.
