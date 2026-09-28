# 固件文档 | Firmware Documents

- [DBG-C-FW-001 固件架构与 PoC-1 记录（中文）](DBG-C-FW-001.zh-CN.md)
- [DBG-C-FW-001 Firmware Architecture and PoC-1 Record (English)](DBG-C-FW-001.en-US.md)

DBG-C-FW-001 V0.12 记录了 ThreadX CH585M 实验性 low-level 与 SysTick 接入、WCH EVT 来源、MounRiver GCC 12 环境配置、可复现主机 ELF 构建与静态审查；通用 FIFO 主机 3683 项断言及 CMSIS-DAP 命令核心 3 项响应检查通过，另以测试专用配置生成 ELF32 RISC-V 对象。CMSIS-DAP 的产品 RISC-V 适配仍未完成；WCH USBFS/UART0 源码审查和移植边界亦有记录。验证板设计门已通过；CH585M 板上的时钟、中断入口、tick 与线程调度仍未验证，产品硬件冻结未放行。

DBG-C-FW-001 V0.12 records the experimental ThreadX CH585M low-level and SysTick integration, WCH EVT provenance, MounRiver GCC 12 setup, reproducible host ELF builds, and static review. The generic FIFO has 3683 passing host assertions; three CMSIS-DAP command responses pass host checks, and a test-only ELF32 RISC-V object was built. Product RISC-V adaptation remains incomplete. The document also records WCH USBFS/UART0 source review and porting boundaries. The verification-board design gate has passed; clock, interrupt entry, tick, and thread scheduling on CH585M remain unverified, and product-hardware freeze is not released.
