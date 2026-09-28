# DBG-C 未决问题与验证清单

**文档编号：** DBG-C-OPEN-001　**版本：** V0.3　**状态：** 开放项

| ID | 问题 | 需要的证据/决策 | 影响文档 | 状态 |
|---|---|---|---|---|
| O01 | CH585M 可用私有 RF PHY/API、BLE 与私有 RF 并发限制是什么？ | WCH 官方参考手册、SDK/例程版本及实测 | MCU, RF, FW, TEST | 待获取 |
| O02 | WCH SDK 的 USBFS Device 栈能否提供 CMSIS-DAP v2 Bulk + CDC 所需端点与缓冲？ | CH585M USBFS Device SDK 示例、端点 API、Host OS 实测 | MCU, USB, SYS | 待验证 |
| O03 | RF DAP 往返延迟和重试边界如何满足常见调试器？ | CMSIS-DAP Host 实测及 RF 原型 | RF, TEST, PRD | 待验证 |
| O04 | DBG-C Basic/Full 在 Type-C 上的合法/可靠引脚及线缆方案？ | USB-IF 最新 Type-C 规范、线缆结构证据、电气评审 | IF, HW, TEST | 待研究 |
| O05 | USB 插入、用户选择、无线连接如何决定 Standalone/Host/Target？ | 产品状态机评审，含冲突/切换规则 | PRD, SYS, RF, BLE | 待决策 |
| O06 | Pairing 是否持久化、是否自动配对、解绑后行为？ | 安全/使用流程评审与持久化测试 | PRD, RF, BLE | 待决策 |
| O07 | Device ID 的实际来源、长度、读取 API 和认证绑定？ | CH585M SDK/官方接口及安全评审 | MCU, RF, BLE | 待验证 |
| O08 | USB VID/PID、接口/端点布局、字符串、Serial 策略？ | 正式实现及组织 VID 决策 | USB, TEST | 待决策 |
| O09 | CDC UART 波特率、流控、目标电平和性能门限？ | 用户需求、电气设计及测量 | PRD, IF, TEST | 待决策 |
| O10 | OTA 是否支持签名、双镜像、回滚及断电恢复？ | WCH Boot/SDK 文档、示例和断电测试 | MCU, FW, BLE, RF, RISK | 待验证 |
| O11 | 目标 UART/RESET/SWD 最大允许电压和保护？ | Target 兼容范围、电气规范与测试 | IF, HW, TEST | 待决策 |
| O12 | 性能目标：DAP 延迟、吞吐、射程、稳定运行时长？ | 原型数据与产品评审 | PRD, RF, TEST | 待决策 |
| O13 | V1 BLE OTA 仅升级 Probe 自身；是否需要 RF OTA？ | 需求确认及资源/安全评审 | PRD, BLE, RF | 待决策 |
| O14 | PC OS、IDE/OpenOCD/pyOCD 支持矩阵？ | 产品支持策略与逐项互操作测试 | PRD, USB, TEST | 待决策 |
| O15 | 当前数据手册适用修订、勘误与参考手册版本？ | WCH 官方发布页和芯片版本核对 | MCU, HW, FW | 待获取 |
| O16 | BLE 无线下载的应用协议、镜像格式、Target 范围和断点续传规则？ | 定义 DBG-C Tool ↔ Probe 协议并选定 Target 验收板 | PRD, BLE, TEST | 待决策 |
| O17 | USBFS/USBHS 是否可同时运行？V1 已分配 USBFS，什么实现限制会要求改用 USBHS？ | SDK 实例、官方资源限制与对照实测 | MCU, USB, SYS | 待验证 |
| O18 | 上游 Eclipse ThreadX RISC-V32/GNU 端口能否正确适配 CH585M QingKe V3C？ | 实现并核验 HPE、PFIC/VTF、启动入口、异常栈帧、SysTick 与 ThreadX 抢占；在 CH585M 板上运行 PoC-1 | MCU, FW, TEST | 待验证；主机 ELF 构建通过 |

## O18 更新证据

- ThreadX 已锁定为 `v6.5.1.202602a_rel`，提交 `b91b03b9e75fa523b17127f9e0eca09dca916459`；MounRiver Linux x64 Toolchain V2.4.0 的 GCC 12.2.0 已安装到当前用户目录。
- PoC-1 ELF 主机交叉编译成功，但未配置 SysTick；WCH startup 默认 SysTick handler 为停机循环，且 HPE/异常栈帧/上下文切换适配未验证。
- 没有可确认的 CH585M 板卡连接，因此 O18 仍未关闭。

## 当前确认边界

- 仓库当前仅发现 `docs/09-references/CH585-CH584_Datasheet_V1.6.pdf`；未发现 SDK、参考手册、源代码、原理图、测试记录或抓包。
- 因此芯片评估是数据手册层面的摘录，不代表 SDK API、并发可行性或板级验证。
- DBG-C Interface 的 Pin 映射、RF 帧字段、USB 描述符、角色切换规则均未冻结。
- 用户补充明确了 BLE 无 Dongle 目标下载和 USBFS 优先的产品方向；前者仍缺应用协议与目标范围，后者已经确定为 V1 USB 分配；端点和 SDK Device 栈兼容性仍需验证。
