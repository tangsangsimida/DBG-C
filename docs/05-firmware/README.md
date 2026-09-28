# 固件文档 | Firmware Documents

- [DBG-C-FW-001 固件架构与 PoC-1 记录（中文）](DBG-C-FW-001.zh-CN.md)
- [DBG-C-FW-001 Firmware Architecture and PoC-1 Record (English)](DBG-C-FW-001.en-US.md)

DBG-C-FW-001 V0.7 记录了 ThreadX CH585M 实验性 low-level 与 SysTick 接入、WCH EVT 来源、MounRiver GCC 12 环境配置、可复现主机 ELF 构建与静态审查；FIFO 主机 57 项断言通过。验证板设计门已通过；CH585M 板上的时钟、中断入口、tick 与线程调度仍未验证，产品硬件冻结未放行。

DBG-C-FW-001 V0.7 records the experimental ThreadX CH585M low-level and SysTick integration, WCH EVT provenance, MounRiver GCC 12 setup, reproducible host ELF builds, static review, and 57 passing FIFO host assertions. The verification-board design gate has passed; clock, interrupt entry, tick, and thread scheduling on CH585M remain unverified, and product-hardware freeze is not released.
