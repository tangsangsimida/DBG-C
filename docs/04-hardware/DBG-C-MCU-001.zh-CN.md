# DBG-C MCU 选型与资源评估

**文档编号：** DBG-C-MCU-001　**版本：** V0.10　**状态：** CH585M V1 原理图资源分配草案；PoC tick 目标 1000 tick/s；1000 tick/s 配置已完成两次固定路径干净构建复核；实板验证未执行

## 1. 证据来源

`docs/09-references/CH585-CH584_Datasheet_V1.6.pdf` 是项目指定的 CH585M IC 手册；PDF 内文标题为《CH585/CH584 数据手册》V1.6，共 160 页。本文件作为当前芯片参数、引脚功能复用与寄存器能力判断的首要项目依据。SDK API、具体工程配置、射频并发性能及板级行为仍须用 SDK/实测核验。不得把 CH584/其他 CH 系列资料外推到 CH585M。

仓库还保留了 `docs/09-references/CH585EVT/CH585EVT.ZIP` 官方 EVT 压缩包，并仅将 ThreadX PoC 实际使用的启动、链接文件及 WCH 头文件副本放入 `software/poc1-ch585-threadx/platform/ch585/`。归档索引 `EVT/CH585_List_EN.txt` 标注日期 2026.08；资料未声明独立 WCH SDK 语义版本。PoC 的 EVT 来源、散列、ThreadX 固定版本和 MounRiver 工具链版本见 DBG-C-FW-001。PoC 主机交叉构建不证明板上运行或产品级外设并发。

## 2. 资源表

| 资源 | 数据手册陈述 | 对 DBG-C 的评估/状态 |
|---|---|---|
| CPU | 青稞 RISC-V3C，RV32IMBC 与自扩展；最高 78 MHz | 具体时钟档、SDK配置待核实 |
| FlashROM | 512 KB：448 KB CodeFlash、32 KB DataFlash、24 KB BootLoader、8 KB InfoFlash | 分区、升级双镜像/回滚、可用空间待 SDK/Boot 验证 |
| SRAM | 128 KB：96 KB RAM96K、32 KB RAM32K | RF/USB 并发缓冲峰值待测 |
| USBFS | 1 组 FS USB2.0 控制器/PHY；15 endpoints；64 B packet；DMA；支持 Host/Device | Endpoint 分配与 SDK API 待查 |
| USBHS | 1 组 480 Mbps USB2.0 HS 控制器/PHY；1024 B packet；DMA；支持 HS/FS Host/Device | V1 暂不作性能优化目标；实际 USB Device 栈和 FS/HS 选择待查 |
| UART | 4 组；8 级 FIFO；数据手册称通信波特率可达 9 Mbps | 可用引脚/时钟精度及目标电平待核实 |
| SPI | 2 组，Master/Slave，DMA | RF 是否需外部收发器目前没有证据；内部 RF 路径待 SDK |
| ADC | 12 位；14 外部+3 内部通道（概述） | V1 非强制，具体可用通道需封装/复用核对 |
| GPIO | 手册概述列出 40 个 GPIO，其中 2 个支持 5V 输入、32 个支持中断/唤醒输入 | 表 1-1 的 CH585M 列列出 PA0–PA15 与 PB0–PB23，共 40 个带封装脚号的 GPIO 标识，与概述计数一致；这不代表 40 个脚均可自由分配或均具备中断/唤醒能力。5VT 不代表可输出 5V |
| BLE/RF | BLE 5.4；集成 2.4 GHz RF；1/2 Mbps；描述有 2.4G 模式及最高 8 kHz 上报率 | 手册概述中的 2.4G 模式语义、私有协议模式/PHY/API、RF DMA 能力与 API、BLE 共存限制及性能均待 SDK/参考手册核验。当前本地手册摘录未证实 RF DMA；不得写成已确认事实 |
| 定时器/PWM | 4 组 26 位定时器；4 路 capture；PWM 资源见手册 | SWD 时序实现适用性需 SDK/波形验证 |
| UID/安全 | AES-128 与芯片唯一 ID | EVT `ISP585.h` 声明底层 ROM 命令与 0 成功/非 0 失败；WCH `GET_UNIQUE_ID()` 说明输出 64 位、缓冲区需 4 字节对齐，但其 void 包装会忽略命令状态。DBG-C 适配层直接检查底层状态并复用官方 UID 字节构造；唯一性/稳定性承诺与密钥边界仍需安全审查，硅片读取未验证 |
| Boot/OTA | 手册称支持 ICP/ISP/IAP、OTA 无线升级 | BootLoader 协议、OTA API、回滚/签名/断电恢复均待官方资料验证 |
| 封装 | CH585M：QFN48 | 焊盘、尺寸与 Pin 表需按正式封装资料复核 |
| 时钟 | 手册引脚表给出 32 MHz HSE 晶体端 X32MO/X32MI，并列出 PA10/PA11 的 32 kHz 晶体功能 | 32 MHz 晶体网络分配到 QFN48 脚 31/32；32 kHz 是否需外接晶体由 WCH BLE/低功耗 SDK 配置确认 |
| Debug | 单/双线仿真接口；手册指出启用后 PB15/PB14 占用 | 生产调试口、复用冲突和量产锁定策略待决策 |

## 3. CH585M QFN48 引脚分配矩阵

下表封装脚号按手册表 1-1（手册印刷页 5–8）的 **CH585M 列**读取。MCU 侧资源已分配为原理图输入，但不冻结 DBG-C Interface 连接器上的 Type-C 触点映射。引脚复用初始化须用 WCH SDK 核验；目标电压、上拉/下拉、驱动方式和保护电路须由接口电气规范确定。

| QFN48 脚号 | GPIO/引脚 | V1 网络分配 | 复用冲突/约束 |
|---:|---|---|---|
| 1 | VDCID | 按手册电源电路连接 | 电容与 DC-DC 连接按手册核验 |
| 2 | VSW | 按手册电源电路连接 | DC-DC 电感/旁路连接按手册核验 |
| 3 | VDD33 / VIO33 | 电源与 I/O 电源网络 | 去耦及 USB 供电关系按参考设计核验 |
| 4 | PA7 | 未分配，保留 | 不启用 TXD2、PWM5、LED6 或 ADC A11 |
| 5 | PA8 | 未分配，保留 | 不启用 RXD1、LED7 或 ADC A12 |
| 6 | PA9 | 可选配对/功能按键 GPIO 预留 | 是否装配按键待 PRD 确认；不启用 TMR0、TXD1、ADC A13 复用 |
| 7 | PB9 | 未分配，保留 | GPIO/NFCI；V1 不启用 NFC |
| 8 | PB8 | 未分配，保留 | GPIO/NFCM；V1 不启用 NFC |
| 9 | PB17 | 未分配，保留 | GPIO/NFC+；V1 不启用 NFC |
| 10 | PB16 | 未分配，保留 | GPIO/NFC−；V1 不启用 NFC |
| 11 | PB15 / TCK | Probe 芯片仿真调试时钟 | 启用仿真调试接口时专用于 TCK；不得分配给 Target SWD |
| 12 | PB14 / TIO | Probe 芯片仿真调试数据 | 启用仿真调试接口时专用于 TIO；不得分配给 Target SWD |
| 13 | PB13 / U2D+ | 保留，USBHS V1 不启用 | 不与 USBFS D+ 混接 |
| 14 | PB12 / U2D− | 保留，USBHS V1 不启用 | 不与 USBFS D− 混接 |
| 15 | PB11 / UD+ | USBFS D+ | 不用作普通 GPIO |
| 16 | PB10 / UD− | USBFS D− | 不用作普通 GPIO |
| 17 | PB7 / TXD0 | Probe UART TX，连接方向为 Target RX | UART0 TXD0；MODEM 信号不使用 |
| 18 | PB6 | Target SWCLK | GPIO 驱动；不启用 RTS/PWM8 复用 |
| 19 | PB5 | Target SWDIO | GPIO 双向驱动；不启用 UART0 DTR 复用 |
| 20 | PB4 / RXD0 | Probe UART RX，连接方向为 Target TX | UART0 RXD0 |
| 21 | PB3 | 未分配，保留 | 不启用 DCD 或 PWM9_ |
| 22 | PB2 | 未分配，保留 | 不启用 PWM8_ |
| 23 | PB1 | 未分配，保留 | 不启用 DSR 或 PWM7_ |
| 24 | PB0 | 未分配，保留 | 不启用 CTS 或 PWM6 |
| 25 | PB23 / RST | Probe 芯片自身低有效复位输入 | 复用功能还包括 TMR0_、TXD2_、PWM11；按手册保留，不能连接为 Target_nRESET 输出 |
| 26 | PB22 | 未分配，保留 | 不启用 TMR3 或 RXD2_；不是芯片 RST 引脚 |
| 27 | PB21 | 未分配，保留 | 不启用 SCL_ 或 TXD3_ |
| 28 | PB20 | 未分配，保留 | 不启用 SDA_ 或 RXD3_ |
| 29 | PB19 | 未分配，保留 | 通用 GPIO，V1 不分配 |
| 30 | PB18 | 未分配，保留 | 通用 GPIO，V1 不分配 |
| 31 | X32MO | 32 MHz 外部晶体网络一端 | 手册标注为 HSE 晶体端；晶体参数及负载按 WCH 参考设计核验 |
| 32 | X32MI | 32 MHz 外部晶体网络另一端 | 手册标注为 HSE 晶体端；晶体参数及负载按 WCH 参考设计核验 |
| 33 | VINTA | 按手册连接去耦电容 | 容值和走线按手册/参考设计核验 |
| 34 | ANT | RF 射频网络/天线连接 | 手册标注 RF 输入输出并建议直连天线；具体网络必须依据 WCH CH585M 射频参考设计核验，当前仓库未包含该设计 |
| 35 | VDCIA | 按手册连接去耦电容 | 与 VDCID 的连接按手册核验 |
| 36 | PA4 | Target_nRESET 控制输出 | GPIO；不启用 UART3 RXD3、LEDC 或 ADC A0；输出级/默认态待电气设计 |
| 37 | PA5 | 可选状态 LED GPIO 预留 | 是否装配 LED 待 PRD/硬件评审确认；不启用 UART3 TXD3、LED4 或 ADC A1 |
| 38 | PA6 | 未分配，保留 | 不启用 RXD2、PWM4_、LED5 或 ADC A10 |
| 39 | PA0 | 未分配，保留 | 不启用 SCK1、LED0 或 ADC A9 |
| 40 | PA1 | 未分配，保留 | 不启用 MOSI1、LED1 或 ADC A8 |
| 41 | PA2 | 未分配，保留 | 不启用 TMR3_、MISO1、RI、LED2 或 ADC A7 |
| 42 | PA3 | 未分配，保留 | 不启用 LED3 或 ADC A6 |
| 43 | PA15 | 未分配，保留 | 不启用 SPI0 MISO 或 UART0 RXD0 重映射 |
| 44 | PA14 | 未分配，保留 | 不启用 SPI0 MOSI 或 UART0 TXD0 重映射 |
| 45 | PA13 | 未分配，保留 | 不启用 SPI0 SCK 或 PWM5 |
| 46 | PA12 | 未分配，保留 | 不启用 SPI0 SCS 或 PWM4 |
| 47 | PA11 / X32KO | 保留给 32 kHz 时钟评估，不接外部信号 | 低频振荡器输出；是否需要 32 kHz 晶体由 WCH BLE/低功耗 SDK 配置确认 |
| 48 | PA10 / X32KI | 保留给 32 kHz 时钟评估，不接外部信号 | 低频振荡器输入；是否需要 32 kHz 晶体由 WCH BLE/低功耗 SDK 配置确认 |

注：封装脚号来自手册表 1-1 的 CH585M 列。表中逐项列出全部 40 个带封装脚号的 PA/PB GPIO 标识，以及 DBG-C 相关电源、时钟和射频脚；其余脚号按手册对应列核对，不据此推断电气用途。GPIO 中断/唤醒能力须按具体芯片资料与 SDK 核验。

## 4. 资源分配状态

| 功能 | 计划需求 | MCU 资源/引脚分配 | 状态 |
|---|---|---|---|
| USB Device | CMSIS-DAP v2 + CDC | USBFS；PB10 QFN48-16=UD−，PB11 QFN48-15=UD+；USBHS 暂不启用 | SDK/描述符待验证 |
| SWD Engine | SWDIO/SWCLK 时序 | PB5 QFN48-19=SWDIO；PB6 QFN48-18=SWCLK；GPIO bit-bang | GPIO 时序与速率待验证 |
| Target Reset | 硬件复位 | PA4 QFN48-36=Target_nRESET | 输出级、默认态和电平待电气设计 |
| Target UART | CDC 桥接 | UART0；PB4 QFN48-20=Probe RX/Target TX，PB7 QFN48-17=Probe TX/Target RX | SDK/并发待验证 |
| RF/BLE | 私有 2.4G、BLE 管理共存 | 集成 Radio 共享资源 | SDK 共存待验证 |
| LED/Button | 状态及配对交互 GPIO 预留 | PA5 QFN48-37=LED 预留；PA9 QFN48-6=可选按键预留 | 是否装配由 PRD/硬件评审确认；电气极性、限流、上拉与中断配置待定 |
| HSE 时钟 | 32 MHz 外部晶体 | X32MO QFN48-31；X32MI QFN48-32 | 晶体参数与布局按 WCH 参考设计核验 |
| LSE 时钟 | 可选 32 kHz 晶体 | PA10 QFN48-48、PA11 QFN48-47 保留 | 是否需要外部晶体由 WCH BLE/低功耗 SDK 配置确认 |
| RF | BLE/私有 2.4G | ANT QFN48-34 接 RF 射频网络/天线 | 手册建议直连天线；最终网络按 WCH CH585M 射频参考设计核验，该资料尚未取得 |
| DBG-C Interface | Basic/Full 信号 | MCU 侧 SWD/UART/Reset 已分配；Type-C 触点映射未冻结 | IF-001 冻结连接器映射和电气参数 |
| Probe Debug/Reset | 生产与恢复 | PB15 QFN48-11=TCK；PB14 QFN48-12=TIO；PB23 QFN48-25=芯片 RST | 保留芯片自身编程/复位功能 |

## 5. V1 外设需求与原理图预分配

目标是给原理图设计提供明确的 MCU 资源基线。此分配是 **V1 原理图输入基线**，不是芯片性能通过验证，也不冻结 DBG-C Interface 的 Type-C Pin 映射。

| 功能 | 分配的 MCU 资源 | 原理图分配 | 说明与验证边界 |
|---|---|---|---|
| PC USB Device | USBFS 控制器/PHY + USB DMA | PB10 QFN48-16=UD−，PB11 QFN48-15=UD+；上行 USB-C | V1 使用 USBFS；CMSIS-DAP v2 Bulk + CDC。USBHS 暂不启用；端点描述符和 SDK Device 栈待固件核验 |
| Target UART/CDC | UART0 + USB CDC 虚拟串口 | PB4 QFN48-20=Probe RX/Target TX，PB7 QFN48-17=Probe TX/Target RX；连接器侧 Pin 待 IF-001 冻结 | UART0 MODEM 信号不使用；CDC 经 USBFS 枚举并桥接 UART0 |
| Target SWD | 共享 SWD Engine 控制的两个 GPIO | PB5 QFN48-19=SWDIO，PB6 QFN48-18=SWCLK；连接器侧 Pin 待 IF-001 冻结 | GPIO 驱动 SWD；不占 SPI。SWD 时序和频率待固件波形验证 |
| Target Reset | 一个 GPIO 控制输出 | PA4 QFN48-36=Target_nRESET；连接器侧 Pin 待 IF-001 冻结 | 不连接 CH585M 自身 PB23/RST（QFN48 脚 25）；输出级、默认态和目标电压兼容性待电气设计 |
| BLE + 私有 2.4G | 芯片集成 Radio/Baseband 与 ANT | ANT QFN48-34 接 RF 射频网络/天线 | 手册建议直连天线；最终网络按 WCH CH585M 射频参考设计核验，该资料尚未取得；共存须 SDK 确认 |
| 状态/配对交互 | GPIO 预留 | PA5 QFN48-37=LED 预留；PA9 QFN48-6=可选按键预留 | 是否装配待 PRD/硬件评审；LED 极性/限流、按键上拉/去抖/唤醒策略待定 |
| HSE 时钟 | 外部 32 MHz 晶体网络 | X32MO QFN48-31，X32MI QFN48-32 | 手册标注 HSE 外接 32 MHz 晶体；具体器件参数按 WCH 参考设计核验 |
| LSE 时钟 | 可选 32 kHz 晶体网络 | PA10 QFN48-48、PA11 QFN48-47 保留 | 是否需要外部晶体由 WCH BLE/低功耗 SDK 配置确认 |
| USB/连接状态检测 | GPIO（仅当选定的电路需要） | 预留 VBUS/连接检测网络位置，暂不定 Pin | 不推断 USBFS 自动提供 VBUS 检测；按 SDK 和供电/连接器方案决定 |
| 唯一身份/配对设置 | 芯片 UID + DataFlash | 适配层经 `FLASH_EEPROM_CMD(CMD_GET_ROM_INFO, ROM_CFG_MAC_ADDR, ..., 0)` 读取 6 字节 MAC 并按 WCH `GET_UNIQUE_ID()` 源码算法生成 8 字节 UID；DataFlash 暂不分配 | 适配层 31 项主机 mock、WCH RISC-V 目标编译及与 `libISP585.a` 的可重定位链接检查通过；不代表硅片读取通过。产品 Device ID 编码、认证绑定和配对存储规则待定 |
| OTA 与固件 | CodeFlash + BootLoader | 按官方烧录/升级参考设计接入调试/恢复所需电路 | 448 KB CodeFlash 不预设双镜像；升级签名、回滚、断电安全尚待证实 |
| SWD 时序/系统时基 | Timer（按需）+ GPIO | 暂不选定 Timer 实例 | 先实现/测量 GPIO SWD；若精度或 CPU 占用不达目标，再根据 SDK 分配 Timer |
| DMA | USBFS DMA；Radio DMA 待确认 | 不做固定 DMA 通道连接 | 数据手册列明 USBFS DMA；DMA 通道数及 USB/RF 仲裁需 SDK/参考手册确认 |

### GPIO/引脚保留约束

1. MCU 侧分配为 PB10 QFN48-16=USBFS UD−、PB11 QFN48-15=USBFS UD+、PB4 QFN48-20=Probe RX/Target TX、PB7 QFN48-17=Probe TX/Target RX；布局发布前核对 WCH SDK 复用初始化。
2. MCU 侧分配为 PB5 QFN48-19=SWDIO、PB6 QFN48-18=SWCLK、PA4 QFN48-36=Target_nRESET；PA5 QFN48-37 预留 LED，PA9 QFN48-6 预留可选按键。DBG-C Interface 的 Type-C 触点映射仍未冻结。
3. CH585M QFN48 脚 11/12 为 PB15/TCK、PB14/TIO。保留给 Probe 仿真调试，不得分配给 Target SWD。
4. PB12 QFN48-14/PB13 QFN48-13 为 USBHS U2D−/U2D+；V1 不启用 USBHS，不要与 USBFS PB10/PB11 混接。
5. CH585M 自身低有效复位输入是 PB23 的 RST 复用功能，QFN48 脚 25；PB22 QFN48 脚 26 的手册复用功能为 TMR3/RXD2_。Target_nRESET 使用 PA4，三者必须分网。
6. PB5/PB6 同时列有 UART0 DTR/RTS 及 PWM8 复用功能，V1 不启用这些功能；PA4/PA5/PA9 的 UART/LED/ADC 复用功能也不启用。
7. 本分配引脚未在手册中标为 5VT。未完成 DBG-C Interface 电平和保护设计前，不得把目标板信号直接接入这些 GPIO。

### V1 不启用的外设

SPI、I2C、ADC、NFC、TouchKey、LCD/LED Matrix、USBHS、SWO/JTAG、Target Power/VTREF 暂不分配。Target 电压检测属于后续功能；不得为了预留而消耗当前必需 GPIO。后续若增项，重新做 Pin Matrix 与资源冲突评审。

### 结论

外设资源计数支持将该方案推进到 **原理图草案**：USBFS、UART0、2 个 SWD GPIO、1 个 Target Reset GPIO、无线 Radio、状态 GPIO及存储/Boot。此结论只表示所需外设资源存在；不能据此宣称 CH585M 已经证明能以目标性能同时运行 USB、BLE、私有 2.4G、CDC 与 SWD。实际性能及无线共存由 SDK 审查和板级验证闭环。

### 存储资源预算方式

| 存储 | 数据手册容量/分区 | 当前规划 |
|---|---|---|
| CodeFlash | 448 KB 用户应用区 | 承载 DBG-C 应用和可移植 CMSIS-DAP 代码；先取得 SDK、DAP 实际构建体积再做分区，不假设双镜像 OTA |
| BootLoader | 24 KB 系统引导程序区 | 保留芯片/官方升级所需用途；能否承载产品安全启动/回滚需验证 |
| DataFlash | 32 KB 用户非易失数据 | 当前不分配；配对持久化与配置存储需求冻结后再分配。磨损均衡/原子更新和 API 待确认 |
| InfoFlash | 8 KB 系统配置信息 | 不当作普通产品数据区使用，按官方定义处理 |
| SRAM | 128 KB 总量 | USB DAP 缓冲、USB CDC 环形队列、RF 收发/重组、BLE 栈、SWD 工作区、任务栈均须纳入链接映射/峰值预算；各自字节数待 SDK 集成后测量，不臆造固定比例 |

## 6. ThreadX 对资源分配的增量要求

项目已确定 V1 固件运行 ThreadX。PoC-1 固定 Eclipse ThreadX `v6.5.1.202602a_rel`（commit `b91b03b9e75fa523b17127f9e0eca09dca916459`），使用其 `ports/risc-v32/gnu` 上下文例程，并加入实验性 CH585M low-level、SysTick 和 WCH 启动/时钟适配。MounRiver Linux x64 Toolchain V2.4.0 的 GCC 12.2.0 主机交叉构建通过；WCH EVT `.cproject` 选择 GCC12 配置但未记载补丁版本。用户确认目前没有可用板卡，且要求软件验证通过后才能设计硬件，再以实物验证。主机编译通过不证明 QingKe RISC-V3C 的硬件压栈、VTF/HPE、异常返回或调度兼容；此前 100 tick/s 配置的两次 clean rebuild ELF/map 一致并完成静态核对。当前 1000 tick/s 配置已在固定路径下完成两次干净构建，ELF/map 散列一致并完成链接资源复核；按 OPEN-001 O19，验证板设计软件门复核通过。这不代表实板运行通过，也不允许冻结产品硬件。PoC 已按决策设置 1000 tick/s；实际频率和 tick 投递仍须实板验证，验收容差在测试方案中冻结。板上验证尚未执行，产品硬件冻结还须实板运行通过，见 O18/O19。

| 资源 | ThreadX 需求 | DBG-C 分配 | 当前证据边界 |
|---|---|---|---|
| Kernel tick | 周期性时基和中断 | 指定芯片内置 32 位 SysTick 作为 ThreadX kernel tick 来源；不占用 TMR0 至 TMR3 | PoC 已使用 EVT 定义的 SysTick API/IRQ 并显式配置为 1000 tick/s；已完成主机交叉构建，实际频率、VTF/HPE、tick 投递和唤醒未板测 |
| Context switch | CPU 上下文保存/恢复、调度入口 | 实验性接入 WCH 启动与中断框架；产品线程/ISR 边界仍待定义 | 使用上游 RISC-V32 上下文例程并添加本地 tick ISR；硬件异常栈帧、VTF/HPE 和恢复路径仍待 CH585M 实板验证 |
| Interrupt | 外设 ISR 到 ThreadX 调度接口 | 在固件架构中分别定义 USB、Radio/BLE、Timer ISR 的 ThreadX API 使用边界 | 中断嵌套、优先级和 SDK ISR 约束需读取 SDK |
| SRAM | ThreadX 内核对象、系统栈、线程栈及应用缓冲 | 在 128 KB SRAM 链接布局中统一预算 USB DAP/CDC、RF 重组、BLE 栈、SWD 工作区 | 各项字节数由选定 ThreadX/SDK、线程数、栈水位和链接映射测量后分配；不填猜测值 |
| CPU | 调度、中断和协议处理时间 | 以 USB/RF 并发负载验证最坏响应时间 | 手册最高 78 MHz 不能单独证明符合性能目标 |
| Compiler/Port | 与 CH585M 工具链、ABI、启动代码兼容的 ThreadX 端口 | ThreadX `v6.5.1.202602a_rel`，commit `b91b03b9e75fa523b17127f9e0eca09dca916459`；MounRiver GCC 12.2.0；WCH EVT GCC12 配置 | 主机交叉构建通过；EVT 工具链补丁版本未标注，板级端口兼容性未验证 |

ThreadX 不增加外部连接器信号，不改变已分配的 USBFS、UART0、SWD GPIO、Target Reset GPIO 和集成 Radio。硬件侧须保留 Probe 编程/恢复路径。当前资源基线将 SysTick 指定为 ThreadX kernel tick 来源，TMR0 至 TMR3 保留；端口接入验证失败时，须记录并评审资源变更。

### ThreadX 上游依赖来源

V1 固件通过 **Git Submodule** 引用 Eclipse ThreadX 官方 GitHub 项目：[eclipse-threadx/threadx](https://github.com/eclipse-threadx/threadx)。当前 PoC 的子模块路径为 `software/third_party/threadx/`，固定标签 `v6.5.1.202602a_rel` 和 commit `b91b03b9e75fa523b17127f9e0eca09dca916459`；该检出包含 MIT `LICENSE.txt`。该固定版本目前用于主机交叉构建，不能据此宣称 CH585M 板级兼容。

ThreadX 上游该版本包含 `ports/risc-v32/gnu`，PoC 使用通用上下文保存/恢复、线程栈构建、系统返回和中断控制例程；CH585M 的 low-level 初始化与 SysTick 入口由本地代码提供。WCH FreeRTOS 样例仅作 QingKe V3C 启动/中断参考，未将 FreeRTOS 任务切换代码移入 ThreadX。此适配的 HPE/PFIC/VTF、异常帧和调度行为仍须板上验证。克隆 DBG-C 固件仓库时须递归初始化子模块，以检出主仓库记录的精确 ThreadX commit。

上游所固定版本的根目录 `LICENSE.txt` 标示 MIT License。纳入源码或二进制发布前，应在 DBG-C 第三方组件清单与发布材料中保留所需声明。该记录不是法律意见。以上上游资料查阅日期：2026-09-28。

## 7. 必须补齐的资料

匹配的 WCH SDK/例程版本、官方封装图、芯片修订/勘误、USBFS Device 栈例程、CH585M 射频天线参考设计、RF/BLE 共存说明、OTA/ISP/IAP 示例及接口、电气时钟要求。取得后更新此文并记录文件版本/哈希与审阅日期。
