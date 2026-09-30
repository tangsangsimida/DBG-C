# DBG-C MCU 选型与资源评估

**文档编号：** DBG-C-MCU-001　**版本：** V0.17　**状态：** CH585M V1 MCU 资源分配草案；Target 1.8 V/3.3 V域要求由 IF-001/PRD-001规定；验证板硬件输入见 HW-001 V0.3；实板验证未执行

## 1. 证据来源

`docs/09-references/CH585-CH584_Datasheet_V1.6.pdf` 是项目指定的 CH585M IC 手册；PDF 内文标题为《CH585/CH584 数据手册》V1.6，共 160 页。本文件作为当前芯片参数、引脚功能复用与寄存器能力判断的首要项目依据。SDK API、具体工程配置、射频并发性能及板级行为仍须用 SDK/实测核验。不得把 CH584/其他 CH 系列资料外推到 CH585M。

仓库保留 `docs/09-references/CH585EVT/CH585EVT.ZIP` 官方 EVT 压缩包，未整体解压。用于本次分配核对的归档文件包括 `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_gpio.h`（SHA-256 `c7f450ceaa501e4c5a912bc0429bca1540fa3c3f26e3fd55dfd908fad9b06183`）、`EVT/EXAM/IAP/USBHS_IAP/src/Main.c`（SHA-256 `5f6edcee35151480161a032c2995b495e70b9ecd352c57c4a99ee351cb40476f`）和 `EVT/PUB/CH585SCH.pdf`（SHA-256 `70391feaa719a7c5ffc1b738b927ba166bf2402692ec42f7042366839c1fb7bb`）。EVT GPIO 头文件声明 UART3 `RB_PIN_UART3` 映射 PA4/PA5 至 PB20/PB21；USBHS IAP Main 初始化 USBHS Device 控制器；官方参考原理图显示 PB22 接 BOOT 下载开关。归档索引 `EVT/CH585_List_EN.txt` 标注日期 2026.08，未声明独立 SDK 语义版本。上述示例不证明 DBG-C 固件实现或板级验证。

SPI 能力核对还使用 `EVT/EXAM/SRC/StdPeriphDriver/CH58x_spi1.c`（SHA-256 `bf8c13f5e7ead9983cb3a25d9d0c59ceadb494ca14f9883495ae5b83fdf57a9f`）及 `EVT/EXAM/SPI/src/Main.c`（SHA-256 `68e4b60472fce13db8d873a792f5e6d6dbda4d0b2a9356c197d52662b9c79831`）；来源为上述 2026.08 EVT 归档。

## 2. 资源表

| 资源 | 数据手册陈述 | 对 DBG-C 的评估/状态 |
|---|---|---|
| CPU | 青稞 RISC-V3C，RV32IMBC 与自扩展；最高 78 MHz | 具体时钟档、SDK配置待核实 |
| FlashROM | 512 KB：448 KB CodeFlash、32 KB DataFlash、24 KB BootLoader、8 KB InfoFlash | 分区、升级双镜像/回滚、可用空间待 SDK/Boot 验证 |
| SRAM | 128 KB：96 KB RAM96K、32 KB RAM32K | RF/USB 并发缓冲峰值待测 |
| USBFS | 1 组 FS USB2.0 控制器/PHY；15 endpoints；64 B packet；DMA；支持 Host/Device | PB10/PB11 保留作恢复、生产和调试通道；V1 主 USB 使用 USBHS。USBFS Device 栈与恢复流程待实现和验证 |
| USBHS | 1 组 480 Mbps USB2.0 HS 控制器/PHY；1024 B packet；DMA；支持 HS/FS Host/Device | V1 主机侧 USB Device 目标；PB12=U2D−、PB13=U2D+。EVT 有 USBHS Device 与 USBHS IAP 示例；复合设备、ThreadX 集成及板级高速信号仍待实现/验证 |
| UART | 4 组；8 级 FIFO；数据手册称通信波特率可达 9 Mbps | UART0 PB4/PB7 用于 Target UART；UART3 RX/TX 通过 EVT `RB_PIN_UART3` 从 PA4/PA5 映射到 PB20/PB21，PB20 接收 SWO。复用配置和波特率待固件验证 |
| SPI | SPI0 支持主机/从机及 DMA；SPI1 仅支持主机。两组支持 SPI Mode 0/3、8 字节 FIFO，最高频率为 Fsys/2；主机分频系数范围 2–254（数据手册 §10.1.1、§10.2） | PA0/PA1/PA2 的 SCK1/MOSI1/MISO1 复用来自手册；项目将 PA3 分配为 GPIO 片选。EVT `EVT/EXAM/SPI/src/Main.c` 中当前由 `#if 1` 屏蔽的 SPI1 分支使用 PA12 作为 GPIO 片选，并将 PA0、PA1、PA12 配为输出；该示例不能作为 PA3 电气配置依据。SPI1 寄存器配置、PA0–PA3 显式 GPIO 模式及 PA3 原始片选电平 BSP 已纳入 PoC；传输、超时恢复、Flash 型号与容量待实现/选型；不得将 SPI0 DMA 能力外推至 SPI1 |
| ADC | 12 位；14 外部+3 内部通道（概述） | PA4 的 A0 复用由手册确认，分配为 Target_VTREF_ADC；分压、输入保护、量程和阈值待电气设计与测量 |
| GPIO | 手册概述列出 40 个 GPIO，其中 2 个支持 5V 输入、32 个支持中断/唤醒输入 | 表 1-1 的 CH585M 列列出 PA0–PA15 与 PB0–PB23，共 40 个带封装脚号的 GPIO 标识，与概述计数一致；这不代表 40 个脚均可自由分配或均具备中断/唤醒能力。5VT 不代表可输出 5V |
| BLE/RF | BLE 5.4；集成 2.4 GHz RF；1/2 Mbps；手册描述 2.4G 模式及最高 8 kHz 上报率 | EVT 含 `RF_Basic`、`RF_PHY`、`RF_PHY_Hop` 示例；私有 RF 协议、RF DMA、BLE 与私有 RF 共存及性能仍待库接口审查和板测 |
| 定时器/PWM | 4 组 26 位定时器；4 路 capture；PWM 资源见手册 | SWD 时序实现适用性需 SDK/波形验证 |
| UID/安全 | AES-128 与芯片唯一 ID | EVT `ISP585.h` 声明底层 ROM 命令与 0 成功/非 0 失败；WCH `GET_UNIQUE_ID()` 说明输出 64 位、缓冲区需 4 字节对齐，但其 void 包装会忽略命令状态。DBG-C 适配层直接检查底层状态并复用官方 UID 字节构造；唯一性/稳定性承诺与密钥边界仍需安全审查，硅片读取未验证 |
| Boot/OTA | 手册称支持 ICP/ISP/IAP、OTA 无线升级 | BootLoader 协议、OTA API、回滚/签名/断电恢复均待官方资料验证 |
| 封装 | CH585M：QFN48 | 焊盘、尺寸与 Pin 表需按正式封装资料复核 |
| 时钟 | 手册引脚表给出 32 MHz HSE 晶体端 X32MO/X32MI，并列出 PA10/PA11 的 32 kHz 晶体功能 | 32 MHz 晶体网络分配到 QFN48 脚 31/32；32 kHz 是否需外接晶体由 WCH BLE/低功耗 SDK 配置确认 |
| Debug | 单/双线仿真接口；手册指出启用后 PB15/PB14 占用 | 生产启用后 PB14/PB15 专用；生产调试口和量产锁定策略待决策 |

## 3. CH585M QFN48 引脚分配矩阵

下表封装脚号按手册表 1-1（手册印刷页 5–8）的 **CH585M 列**读取。MCU 侧资源按用户提出的 V1 基线更新为原理图输入；焊盘号和表列复用依据 CH585M 数据手册表 1-1，UART3 重映射依据 EVT GPIO 头文件，BOOT 网络依据 EVT CH585M 参考原理图。此表不冻结 DBG-C Interface 的 Type-C 触点映射；目标电压、驱动、保护及各外设运行仍需设计和验证。

| QFN48 脚号 | GPIO/引脚 | V1 网络分配 | 复用冲突/约束 |
|---:|---|---|---|
| 1 | VDCID | 按手册电源电路连接 | 电容与 DC-DC 连接按手册核验 |
| 2 | VSW | 按手册电源电路连接 | DC-DC 电感/旁路连接按手册核验 |
| 3 | VDD33 / VIO33 | 电源与 I/O 电源网络 | 去耦及 USB 供电关系按参考设计核验 |
| 4 | PA7 | 未分配，保留 | 不启用 TXD2、PWM5、LED6 或 ADC A11 |
| 5 | PA8 | 官方 ISP UART 资源预留 | 手册复用为 RXD1；具体 ISP 通道/进入方式待官方下载资料确认 |
| 6 | PA9 | 官方 ISP UART 资源预留 | 手册复用为 TMR0、TXD1、ADC A13；下载功能用途待官方资料确认 |
| 7 | PB9 | LED_TARGET | GPIO/NFCI；V1 不启用 NFC；LED 电气极性和限流待定 |
| 8 | PB8 | KEY_MODE | GPIO/NFCM；V1 不启用 NFC；按键上拉、去抖与唤醒策略待定 |
| 9 | PB17 | EXT_RESET_N | GPIO；EVT 还列有 RF antenna switch 控制复用，确认天线网络不启用该 GPIO 输出功能 |
| 10 | PB16 | EXT_IRQ | GPIO；EVT 还列有 RF antenna switch 控制复用，确认天线网络不启用该 GPIO 输出功能 |
| 11 | PB15 / TCK | CH585 自身仿真调试时钟 | 启用仿真调试接口后专用于 TCK；不分配给 Target JTAG |
| 12 | PB14 / TIO | CH585 自身仿真调试数据 | 启用仿真调试接口后专用于 TIO；不分配给 Target JTAG |
| 13 | PB13 / U2D+ | PC USBHS D+ | USBHS Device 数据线；不得与 USBFS D+ 混接 |
| 14 | PB12 / U2D− | PC USBHS D− | USBHS Device 数据线；不得与 USBFS D− 混接 |
| 15 | PB11 / UD+ | USBFS D+ 恢复通道预留 | 恢复/生产接口是否装配待硬件设计确认；不与 USBHS 混接 |
| 16 | PB10 / UD− | USBFS D− 恢复通道预留 | 恢复/生产接口是否装配待硬件设计确认；不与 USBHS 混接 |
| 17 | PB7 / TXD0 | TARGET_UART_TX | UART0 TXD0；连接 Target RX |
| 18 | PB6 | TARGET_PWR_EN | GPIO 控制信号；目标供电开关、电平和默认状态待电源设计 |
| 19 | PB5 | TARGET_nRESET | GPIO 控制信号；极性、驱动级、默认状态和脉宽待 IF/电气设计 |
| 20 | PB4 / RXD0 | TARGET_UART_RX | UART0 RXD0；连接 Target TX |
| 21 | PB3 | TARGET_TDO | GPIO 输入；JTAG 模式使用，电气约束待 IF 设计 |
| 22 | PB2 | TARGET_TDI | GPIO 输出；JTAG 模式使用，电气约束待 IF 设计 |
| 23 | PB1 | TARGET_SWDIO_TMS | GPIO 双向；SWD 使用 SWDIO，JTAG 使用 TMS |
| 24 | PB0 | TARGET_SWCLK_TCK | GPIO 输出；SWD 使用 SWCLK，JTAG 使用 TCK |
| 25 | PB23 / RST | CH585 自身低有效复位 | 手册表 1-1 标为外部复位输入，内置上拉；不可接 Target_nRESET 输出 |
| 26 | PB22 | CH585_BOOT 板级控制网络 | EVT CH585M 参考原理图将 PB22 接至 BOOT 下载开关；芯片表内无专用 BOOT 复用，进入条件/时序待官方下载说明确认 |
| 27 | PB21 | 保留 | GPIO/UART3 TXD3 重映射目标；保留作扩展，不与 PB20 SWO 接收混淆 |
| 28 | PB20 / RXD3_ | TARGET_SWO | UART3 RXD3 重映射目标；EVT `RB_PIN_UART3` 将 UART3 从 PA4/PA5 映射到 PB20/PB21 |
| 29 | PB19 | LED_RF | GPIO；EVT 还列有 RF antenna switch 控制复用，需确认不启用该输出 |
| 30 | PB18 | LED_USB | GPIO；EVT 还列有 RF antenna switch 控制复用，需确认不启用该输出 |
| 31 | X32MO | 32 MHz 外部晶体网络一端 | 手册标注为 HSE 晶体端；晶体参数及负载按 WCH 参考设计核验 |
| 32 | X32MI | 32 MHz 外部晶体网络另一端 | 手册标注为 HSE 晶体端；晶体参数及负载按 WCH 参考设计核验 |
| 33 | VINTA | 按手册连接去耦电容 | 容值和走线按手册/参考设计核验 |
| 34 | ANT | RF 射频网络/天线连接 | 手册标注 RF 输入输出并建议直连天线；具体网络必须依据 WCH CH585M 射频参考设计核验，当前仓库未包含该设计 |
| 35 | VDCIA | 按手册连接去耦电容 | 与 VDCID 的连接按手册核验 |
| 36 | PA4 / A0 | TARGET_VTREF_ADC | ADC A0 复用；同时是 UART3 RX 默认引脚，启用 UART3 RX 重映射至 PB20 后避免冲突；前端量程/保护待设计 |
| 37 | PA5 | 保留 | UART3 TX 默认引脚；若使用 PB20/PB21 重映射，需确认复用配置；保留 |
| 38 | PA6 | 未分配，保留 | 不启用 RXD2、PWM4_、LED5 或 ADC A10 |
| 39 | PA0 / SCK1 | EXT_FLASH_SCK | SPI1 SCK1 复用；总线模式/时钟待驱动设计 |
| 40 | PA1 / MOSI1 | EXT_FLASH_MOSI | SPI1 MOSI1 复用 |
| 41 | PA2 / MISO1 | EXT_FLASH_MISO | SPI1 MISO1 复用 |
| 42 | PA3 | EXT_FLASH_CS | GPIO 片选；手册不列 SPI1 专用 CS 复用。EVT `EVT/EXAM/SPI/src/Main.c` 中当前由 `#if 1` 屏蔽的 SPI1 分支使用 PA12，不等同于项目 PA3 分配 |
| 43 | PA15 | 官方 ISP UART 资源预留 | 手册列 UART0 RXD0_ 重映射；是否为 ISP 通道待官方下载资料确认 |
| 44 | PA14 | 官方 ISP UART 资源预留 | 手册列 UART0 TXD0_ 重映射；是否为 ISP 通道待官方下载资料确认 |
| 45 | PA13 | 未分配，保留 | 不启用 SPI0 SCK 或 PWM5 |
| 46 | PA12 | 未分配，保留 | 不启用 SPI0 SCS 或 PWM4 |
| 47 | PA11 / X32KO | 保留给 32 kHz 时钟评估，不接外部信号 | 低频振荡器输出；是否需要 32 kHz 晶体由 WCH BLE/低功耗 SDK 配置确认 |
| 48 | PA10 / X32KI | 保留给 32 kHz 时钟评估，不接外部信号 | 低频振荡器输入；是否需要 32 kHz 晶体由 WCH BLE/低功耗 SDK 配置确认 |

注：封装脚号来自手册表 1-1 的 CH585M 列。表中逐项列出全部 40 个带封装脚号的 PA/PB GPIO 标识，以及 DBG-C 相关电源、时钟和射频脚；其余脚号按手册对应列核对，不据此推断电气用途。GPIO 中断/唤醒能力须按具体芯片资料与 SDK 核验。

## 4. 资源分配状态

| 功能 | 计划需求 | MCU 资源/引脚分配 | 状态 |
|---|---|---|---|
| PC USB Device | USBHS CMSIS-DAP v2 Bulk、CDC ACM 与管理通道 | USBHS；PB12 QFN48-14=U2D−，PB13 QFN48-13=U2D+ | EVT 有 USBHS Device/IAP 示例；复合描述符、ThreadX 集成和 HS 实板枚举待实现/验证 |
| USBFS 恢复 | 生产/恢复 USB 通道 | PB10 QFN48-16=UD−，PB11 QFN48-15=UD+ | 保留；是否接入连接器、与官方 ISP 的完整流程待核实 |
| Target SWD/JTAG | 共用 GPIO 引擎 | PB0 QFN48-24=SWCLK/TCK；PB1 QFN48-23=SWDIO/TMS；PB2 QFN48-22=TDI；PB3 QFN48-21=TDO | 引脚复用表对应关系已核；波形、速率和 DAP JTAG 能力待软件/板测 |
| Target Reset/Power | 复位、目标电源使能 | PB5 QFN48-19=TARGET_nRESET；PB6 QFN48-18=TARGET_PWR_EN | GPIO 焊盘已核；电平转换、默认态、供电保护与控制逻辑待设计 |
| Target UART/CDC | 全双工 UART 桥 | UART0；PB4 QFN48-20=RXD0，PB7 QFN48-17=TXD0 | EVT UART0 引脚和接口已核；CDC 桥接、速率和并发待实现/验证 |
| SWO | NRZ SWO 接收 | PB20 QFN48-28=RXD3_；UART3 RX 由 EVT `RB_PIN_UART3` 重映射至 PB20 | 头文件声明映射；SWO 采样、波特率和 CMSIS-DAP SWO 输出路径待实现/验证 |
| Target VTref | 目标电压检测 | PA4 QFN48-36=ADC A0 | 仅确认 ADC 复用名；分压、钳位、量程、校准和判定阈值待电气设计/测量 |
| 外部 SPI NOR | 升级缓存、回滚、日志及离线镜像暂存资源 | SPI1：PA0= SCK1、PA1=MOSI1、PA2=MISO1；PA3=GPIO CS | 引脚复用已核；Flash 型号/容量、协议、时钟与分区待选型/实现；不能据此冻结 Flash 布局 |
| 私有 RF/BLE | 私有 2.4 GHz 与 BLE 管理 | 芯片集成 Radio；ANT QFN48-34 | EVT 有 RF 示例；协议、DMA、RF 与 BLE 同时运行能力待源码/库接口分析及板测 |
| Probe 自身调试/复位 | 生产调试和恢复 | PB15 QFN48-11=TCK；PB14 QFN48-12=TIO；PB23 QFN48-25=RST | 依数据手册保留；是否持续开放由生产策略确定 |
| BOOT 控制 | 官方下载/恢复入口 | PB22 QFN48-26 接板级 BOOT 控制网络 | EVT 参考原理图确认网络；芯片复用表没有专用 BOOT 功能，启动条件待官方文档确认 |
| UI | 模式键和状态指示 | PB8=KEY_MODE；PB9=LED_TARGET；PB18=LED_USB；PB19=LED_RF | 引脚为 GPIO；PB18/PB19 兼有 RF antenna switch 控制复用，相关复用配置须关闭/核查 |
| 扩展控制 | 扩展器件中断/复位 | PB16=EXT_IRQ；PB17=EXT_RESET_N | GPIO 焊盘已核；PB16/PB17 兼有 RF antenna switch 控制复用 |

## 5. V1 外设需求与原理图预分配

下表将用户提出的引脚基线与官方资料交叉核对后作为原理图输入。焊盘号/复用依据 CH585/CH584 数据手册 V1.6 表 1-1 的 CH585M 列；GPIO 重映射和 UART3 映射依据归档 `EVT/EXAM/SRC/StdPeriphDriver/inc/CH58x_gpio.h`；PB22 BOOT 网络依据 `EVT/PUB/CH585SCH.pdf`。这不代表外设驱动、组合工作或板级电气行为已经验证，也不冻结 DBG-C Interface 的 Type-C 触点。

| 功能 | MCU 资源 | 原理图分配 | 当前证据与待办 |
|---|---|---|---|
| PC USB | USBHS Device | PB12=USBHS D−、PB13=USBHS D+ | 数据手册引脚表及 EVT `USBHS_IAP`、USBHS Device 示例可确认外设/示例存在；DAP Bulk+CDC 复合描述符、端点规划、ThreadX 集成与 HS 信号实测待完成 |
| USB 恢复 | USBFS Device 保留 | PB10=USBFS D−、PB11=USBFS D+，预留生产/恢复接入 | 数据手册确认引脚；恢复模式进入和产品是否引出待确认 |
| Target SWD/JTAG | 共用 SWD/JTAG GPIO backend | PB0=SWCLK/TCK、PB1=SWDIO/TMS、PB2=TDI、PB3=TDO | GPIO 复用有效；IO 电压、输出结构、转换器和时序待 IF-001/验证板确定 |
| Target nRESET | GPIO | PB5=TARGET_nRESET | 不与芯片自身 PB23/RST 混网；驱动级、默认态、脉宽待定 |
| Target 电源控制 | GPIO | PB6=TARGET_PWR_EN | 外部负载开关、限流、反灌、故障检测和默认状态需电源设计 |
| Target UART/CDC | UART0 + USB CDC ACM | PB4=Probe RX/Target TX；PB7=Probe TX/Target RX | EVT remap 文档与 datasheet 对应；波特率、CDC interface/endpoint 和并发待实现 |
| SWO | UART3 RX | PB20=RXD3_；启用 EVT `RB_PIN_UART3` 映射 | PA4/PA5 默认 UART3 路径与 PA4 ADC 分配冲突；使用重映射后 PA4 可保留 ADC，需用实际初始化验证 |
| VTref | ADC A0 | PA4=TARGET_VTREF_ADC | PA4 同时具有 UART3 RX 默认复用，必须启用 UART3 重映射；前端不能把任意目标电压直接送入 MCU，量程和保护待计算 |
| 外部 Flash | SPI1 + GPIO CS | PA0=SCK1、PA1=MOSI1、PA2=MISO1、PA3=CS | 手册给出 SPI1 信号复用；SPI CS 为 GPIO；型号、容量、兼容性和离线镜像需求待选型 |
| CH585 自身调试/复位 | 仿真调试接口与 RST | PB14=TIO、PB15=TCK、PB23=RST | 与 Target SWD/JTAG 独立；复位脚保持可访问 |
| BOOT | 板级下载控制 | PB22 接 BOOT 开关/电路 | EVT `CH585SCH.pdf` 可确认参考板的网络连接；不要把 PB22 描述为片内专用 BOOT 复用，启动门限和顺序待官方下载资料确认 |
| ISP UART 资源 | 保留可能的下载串口路由 | PA8、PA9、PA14、PA15 保留 | 手册确认其 UART 复用，但当前证据尚不能确认四脚都用于官方 ISP；查验下载文档/SDK后再决定占用 |
| UI | GPIO | PB8=KEY_MODE；PB9=LED_TARGET；PB18=LED_USB；PB19=LED_RF | PB8/PB9/ PB18/PB19 为 GPIO；PB18/PB19 同属 EVT RF antenna switch 控制复用范围，确认初始化不启用该功能 |
| 扩展 | GPIO | PB16=EXT_IRQ；PB17=EXT_RESET_N；PB21 保留 | EVT 将 RF antenna switch 控制输出列为 PB16–PB21；须确认 RF 配置未占用这些 GPIO 输出 |
| RF | 芯片集成 2.4 GHz/BLE Radio | ANT QFN48-34 按 WCH 射频参考设计连接 | EVT RF 示例存在；匹配网络、天线、净空及 USB 共存布局必须从官方板级资料提取；不能按通用经验定值 |

### GPIO/引脚约束

1. PB12/PB13 是 USBHS U2D−/U2D+；PB10/PB11 是 USBFS UD−/UD+，两组不得混接。
2. Target SWD/JTAG 使用 PB0–PB3，Target UART 使用 PB4/PB7，Target nRESET/PWR_EN 使用 PB5/PB6。该组合符合数据手册 CH585M 引脚表；电气兼容和时序仍待验证。
3. PB14/PB15 是 CH585 自身 TIO/TCK；PB23 的 RST 是 CH585 自身低有效外部复位；均与 Target 接口分开。
4. PB20 可通过 EVT 声明的 UART3 重映射作为 RXD3；PA4/A0 因而可用于 VTref ADC。若未应用重映射，UART3 RX 会与 PA4 资源冲突。
5. PB22 仅称为板级 BOOT 控制网络：参考原理图显示开关连至该脚，芯片引脚表未列专用 BOOT 复用。进入条件须以官方 ISP/下载说明确认。
6. PB16–PB21 的 EVT GPIO 重映射说明包含 RF 天线开关控制输出。EXT_IRQ、EXT_RESET_N、LED、SWO 等分配能否同时成立，取决于 RF 初始化是否启用此输出；检查源码/库配置并在硬件测试前关闭冲突配置。
7. Target 可能使用 1.8 V 或 3.3 V。数据手册 GPIO 复用并不能证明任意电压容忍；VTref 前端、Target 信号转换、保护及掉电隔离须单独设计。

### V1 外设范围

V1 基线包含 USBHS、USBFS 恢复预留、CMSIS-DAP v2 Bulk、SWD、JTAG、SWO、Target UART/CDC、BLE、私有 2.4 GHz、USB 自升级和无线自升级架构，以及 VTref 检测、Target 电源控制和外部 SPI NOR 资源。实现顺序及验收仍以 PRD/FW/TEST 文档为准。JTAG/SWO 和升级传输在资料分析/实现阶段，不代表已有产品代码或实测能力。

### 结论

按数据手册焊盘表和 EVT 映射资料，当前资源分配可作为原理图设计输入继续评审。以下事项仍是设计门槛：PB16–PB21 与 RF 天线开关复用的影响、BOOT 入口条件、USBHS 复合端点与 ThreadX 集成、Target 电平转换和电源保护、RF/BLE 共存，以及外部 Flash 型号与容量。上述事项不得在文档中标为已实现或实测通过。

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

ThreadX 不增加外部连接器信号；当前分配为 USBHS 主 Device、USBFS 恢复预留、UART0、PB0–PB3 Target SWD/JTAG、PB5/PB6 Target reset/power、集成 Radio。硬件侧须保留 Probe 编程/恢复路径。当前资源基线将 SysTick 指定为 ThreadX kernel tick 来源，TMR0 至 TMR3 保留；端口接入验证失败时，须记录并评审资源变更。

### ThreadX 上游依赖来源

V1 固件通过 **Git Submodule** 引用 Eclipse ThreadX 官方 GitHub 项目：[eclipse-threadx/threadx](https://github.com/eclipse-threadx/threadx)。当前 PoC 的子模块路径为 `software/third_party/threadx/`，固定标签 `v6.5.1.202602a_rel` 和 commit `b91b03b9e75fa523b17127f9e0eca09dca916459`；该检出包含 MIT `LICENSE.txt`。该固定版本目前用于主机交叉构建，不能据此宣称 CH585M 板级兼容。

ThreadX 上游该版本包含 `ports/risc-v32/gnu`，PoC 使用通用上下文保存/恢复、线程栈构建、系统返回和中断控制例程；CH585M 的 low-level 初始化与 SysTick 入口由本地代码提供。WCH FreeRTOS 样例仅作 QingKe V3C 启动/中断参考，未将 FreeRTOS 任务切换代码移入 ThreadX。此适配的 HPE/PFIC/VTF、异常帧和调度行为仍须板上验证。克隆 DBG-C 固件仓库时须递归初始化子模块，以检出主仓库记录的精确 ThreadX commit。

上游所固定版本的根目录 `LICENSE.txt` 标示 MIT License。纳入源码或二进制发布前，应在 DBG-C 第三方组件清单与发布材料中保留所需声明。该记录不是法律意见。以上上游资料查阅日期：2026-09-28。

## 7. 必须补齐的资料

匹配的 WCH SDK/例程版本、官方封装图、芯片修订/勘误、USBHS Device 与 USBFS recovery 示例和恢复说明、CH585M 射频天线参考设计、RF/BLE 共存说明、OTA/ISP/IAP 入口与接口、Target 信号电气规格及系统时钟要求。取得后更新此文并记录文件版本/哈希与审阅日期。

## 8. 验证板硬件接口边界

MCU-001 只定义 MCU 侧资源分配草案；实际外围电路、供电、电平转换、保护、恢复路径、测试点和 PCB 约束由 [HW-001](DBG-C-HW-001.zh-CN.md) 管理。V1 Target I/O域为1.8 V与3.3 V，所有Target数字信号必须使用VTref关联电平适配和掉电隔离。HW-001 V0.3允许验证板模块化原理图继续，但转换器件、Target Power、Type-C自定义映射、BOOT/恢复时序、PB16–PB21 RF复用、RF匹配及供电/时钟具体值仍需证据。此结论不等于MCU引脚已电气验证或产品硬件冻结。
