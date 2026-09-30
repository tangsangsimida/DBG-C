# DBG-C 产品需求规格书

**文档编号：** DBG-C-PRD-001　**版本：** V0.3（需求基线草案）　**状态：** Target电压域需求已补充，其他需求待评审

## 1. 产品定位

DBG-C Platform 是统一有线、私有 2.4G 与 BLE 管理能力的 MCU 调试下载平台。DBG-C Probe 是设备；DBG-C Interface 是 Probe 到目标板的物理连接；DBG-C RF Protocol 是两台 Probe 之间的无线协议；DBG-C Tool 是 PC 配置、管理与升级软件。这些术语不得互换。

目标是以 DBG-C Interface 取代分立的 SWD/JTAG/UART 杜邦线连接。V1 有线主机侧采用 USBHS 上的 CMSIS-DAP v2 Bulk；调试器固件按 DAPLink 体系组织并复用适用的开源协议/算法组件，CMSIS-DAP 是调试协议，USB Bulk 是有线传输。首阶段优先实现 SWD；JTAG、SWO、Target VTref 检测、Target 电源控制、USB 自升级、私有 2.4 GHz 自升级和外部 SPI NOR 纳入当前产品方向，但实现先后及验收依赖 FW/TEST 评审。产品形态是同一个 DBG-C Probe 通过不同连接模式提供有线调试、成对 2.4G 调试及 BLE 管理/后续 OTA 能力，而不是三个独立产品。

## 2. 问题与目标

传统调试连接需要多根线，容易接错且不便于移动。V1 应提供统一有线调试、双设备无线调试链路、目标 UART、硬件复位以及 BLE 设备管理能力。DBG-C 应可单机有线工作；成对后由一台连接 PC、另一台连接 Target，两台硬件相同，仅运行角色不同。

## 3. 使用场景与工作流

1. **有线调试：** PC 通过 USB 连接单台 Probe；PC 使用 CMSIS-DAP v2，目标侧通过 DBG-C Interface 提供 SWDIO、SWCLK、nRESET 和 UART。
2. **无线调试：** 两台 Probe 完成明确的配对及角色建立；PC 侧 Probe 通过 USB 暴露 CMSIS-DAP v2，目标侧 Probe 连接 Target；调试命令、响应及 UART 数据经 DBG-C RF Protocol 传输。
3. **BLE 管理与下载：** DBG-C Tool 经 PC Bluetooth 发现设备、读取信息、配置、配对/解绑、查看状态、目标复位，并执行计划中的目标无线下载及 Probe 自身固件升级。应用层下载协议、目标 MCU 支持范围及 PC OS/适配器兼容性待定义；BLE 不直接暴露原生 CMSIS-DAP 给 Keil/IAR/OpenOCD/pyOCD。
4. **无线故障恢复：** RF 断开时停止或失败返回未完成操作，状态可恢复；不得静默重复可能产生副作用的命令。恢复策略和时限待协议阶段冻结。

## 4. V1 范围

- CH585M 单芯片；USBHS Device；DAPLink 固件体系方向、CMSIS-DAP v2 Bulk；SWD Engine；SWDIO、SWCLK、目标 nRESET。
- Target JTAG、SWO 接收、UART0 至 CDC ACM、VTref ADC 检测、Target 电源控制资源及 USB 自升级架构；具体电气实现、升级分区和验收门槛待评审。
- 外部 SPI NOR 资源用于无线固件镜像暂存、回滚/配置及后续离线镜像评估；型号、容量和分区不得在实测链接映射、镜像大小和需求冻结前确定。
- CDC UART；DBG-C Interface（实际 Pin 映射未冻结）。
- 单机有线调试；两台同硬件 Probe 的私有 2.4G 配对、Host/Target 角色机制、下载、在线 Debug、UART 通道及 Probe 自身无线升级。
- BLE 发现、配置、配对管理、状态与目标复位。BLE OTA 和 BLE Target 下载属于后续阶段，协议与适用 Target 范围待冻结。
- 唯一设备身份；配对、解绑、重新配对；无线断开恢复。
- 设备退出无线配对后仍可单机有线使用。

## 5. 非目标

V1 不包含 Target 电流检测、离线烧录验收、多 Target、手机 App、自动识别 MCU 和高级 Trace。外部 Flash 镜像存储仅按第 4 节所述升级暂存与后续评估使用；离线烧录仍是后续功能。USBHS 是当前有线主接口基线，不属于可选性能优化。BLE 与私有 2.4 GHz 是否能并发工作尚未验证，不作为同时在线的验收承诺。

## 6. 系统组成和安全需求

组成包括 PC/DBG-C Tool、DBG-C Probe、USB、BLE、私有 2.4G、DBG-C Interface、Target MCU。设备身份、配对授权和 OTA 完整性须纳入威胁分析；加密/认证算法、密钥生命周期、调试授权策略尚待评审，不能仅凭芯片 AES 能力视为已实现安全。

## 7. 需求与验收标准

| ID | 需求 | 验收条件 |
|---|---|---|
| PRD-001 | USB CMSIS-DAP v2 与 SWD | 在正式支持主机/工具矩阵中完成连接、识别、读写、擦除、下载校验、复位、断点、单步、寄存器与内存操作；矩阵及版本待冻结 |
| PRD-002 | CDC Target UART | RX/TX/全双工及与调试并发通过 USB CDC 主机验证；波特率、流控、吞吐门限待性能评审冻结 |
| PRD-003 | Target Reset | 复位脉冲、极性、电气兼容性、连接和断开状态通过接口规范及示波器验证；数值待验证 |
| PRD-004 | 双 Probe 无线链路 | 配对、角色、下载、在线 Debug、UART、断连检测与恢复按 RF 规范及测试规范通过 |
| PRD-005 | BLE 管理/下载 | Discovery、信息、配置、配对/解绑、状态、目标复位和重连按 BLE 规范通过；BLE OTA 与 Target 下载属于后续阶段，不纳入当前强制验收 |
| PRD-006 | 唯一身份与配对管理 | 两设备身份可区分；配对、解绑、重新配对及掉电后行为符合冻结的数据持久化规则 |
| PRD-007 | 单机回退 | 任一配对设备脱离无线链路后，独立 USB 调试能力通过回归 |
| PRD-008 | 恢复与错误可见性 | RF/USB/Target 故障均有可区分状态与错误结果；不出现无报告的命令重复或伪成功 |
| PRD-009 | USBHS 复合设备 | 最终 USBHS 固件在批准的 Windows/Linux/macOS 矩阵枚举 CMSIS-DAP v2 Bulk 与 CDC ACM；描述符、端点和包长以冻结的 USB-001 实现为准 |
| PRD-010 | Target JTAG | 在至少一个冻结的 Cortex-M Target 上通过 CMSIS-DAP JTAG 连接、DP/AP 访问、下载校验、复位和在线调试；速率与兼容范围待评审 |
| PRD-011 | SWO | 接收 Target SWO 并通过已定义的主机流接口读取；波特率、数据格式、缓冲与丢失指标待 USB/FW/TEST 冻结 |
| PRD-012 | VTref 检测 | 在冻结的 Target 电压输入范围内报告电压/有效状态；分压、保护、校准、误差和阈值通过电气评审及测量 |
| PRD-013 | Target 电源控制 | 通过外部电源开关控制 Target 供电；验证默认关闭/安全状态、使能/关闭、短路/过流和反向供电处理；具体电路与门限待冻结 |
| PRD-014 | USB 自升级 | PC 通过 USBHS 更新 Probe 固件；完整镜像校验、提交、复位启动、损坏镜像恢复和断电恢复通过测试 |
| PRD-015 | 私有 2.4 GHz 自升级 | 成对设备按冻结的 RF 更新协议可靠传送 Probe 镜像；验证丢包/重传、断链恢复、完整性校验、提交和恢复 |
| PRD-016 | 外部 SPI NOR | 选定器件后验证镜像暂存读写、擦除边界、完整性校验和断电保留；是否承载回滚/离线镜像须单独冻结 |
| PRD-017 | Target I/O 电压域与电平适配 | V1 支持 1.8 V 与 3.3 V Target I/O；所有 Target 数字信号经与 Target VTref 关联的电平适配，Probe 或 Target 任一侧掉电时隔离；禁止依赖 CH585M 3.3 V GPIO 直连兼容。器件、时序、保护和误差通过 HW 评审及验证板实测 |


## 8. 性能指标

现阶段没有经测量或经批准的延迟、吞吐、启动时间、功耗、射程、掉包率门限。不得编造数值；须在 RF/USB/SWD 原型测试后建立量化指标和测量条件，并纳入验收表。

## 9. 未决需求

设备插 USB 是否影响角色、角色切换规则、配对是否持久、自动/手动配对、并发 USB 多设备规则、PC OS/IDE 矩阵、CDC 参数、Target 有效电压门限和故障范围、无线恢复时限、OTA 回滚与授权、BLE 目标下载协议/Target 范围、接口电缆能力，均待需求评审决议。Target I/O 电压域为 1.8 V 与 3.3 V 已冻结；见 [DBG-C-OPEN-001](../00-project/DBG-C-OPEN-001.zh-CN.md)。

## 10. 产品表述（需求输入，非验证结论）

产品方向概括为：**统一有线、私有 2.4G 与 BLE 无线能力的嵌入式 MCU 调试下载平台。单机可作为有线调试器，两台同型 DBG-C 组成对称式 2.4G 链路；PC 有线侧目标为 CMSIS-DAP v2，BLE 由 DBG-C Tool 承载应用管理/下载协议。**“一台有线，两台无线；一根 Type-C，统一 MCU 调试接口”是产品愿景文案，不能解释为 Type-C Pin/线缆兼容性已验证。

VTref、Target Power、SWO、JTAG、USB 自升级、私有 2.4 GHz 自升级和外部 SPI NOR 已纳入当前产品方向；各项具体验收门槛仍须在 FW/TEST 中冻结。BOOT 控制和 Target Detect 不在当前已核实的 V1 引脚基线中。BLE OTA 属后续实现阶段；BLE 无 Dongle 下载仍需独立应用协议与兼容矩阵，不代表 Keil/IAR/OpenOCD/pyOCD 可直接使用系统 BLE。
