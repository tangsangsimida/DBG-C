# DBG-C MCU 选型与资源评估

**文档编号：** DBG-C-MCU-001　**版本：** V0.1　**状态：** 初步数据手册摘录；资源分配未冻结

## 1. 证据来源

`docs/09-references/CH585-CH584_Datasheet_V1.6.pdf` 是项目指定的 CH585M IC 手册；PDF 内文标题为《CH585/CH584 数据手册》V1.6，共 160 页。本文件作为当前芯片参数、引脚功能复用与寄存器能力判断的首要项目依据。SDK API、具体工程配置、射频并发性能及板级行为仍须用 SDK/实测核验。不得把 CH584/其他 CH 系列资料外推到 CH585M。

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
| GPIO | 40 个；其中 2 个支持 5V 输入，32 个支持中断/唤醒输入 | 不代表可输出 5V；Pin mux 与电压约束须逐脚审核 |
| BLE/RF | BLE 5.4；集成 2.4 GHz RF；1/2 Mbps；描述有 2.4G 模式及最高 8 kHz 上报率 | 手册概述中的 2.4G 模式语义、私有协议模式/PHY/API、RF DMA 能力与 API、BLE 共存限制及性能均待 SDK/参考手册核验。当前本地手册摘录未证实 RF DMA；不得写成已确认事实 |
| 定时器/PWM | 4 组 26 位定时器；4 路 capture；PWM 资源见手册 | SWD 时序实现适用性需 SDK/波形验证 |
| UID/安全 | AES-128 与芯片唯一 ID | UID 读取接口、长度、不可变性和密钥存储边界待 SDK/安全审查 |
| Boot/OTA | 手册称支持 ICP/ISP/IAP、OTA 无线升级 | BootLoader 协议、OTA API、回滚/签名/断电恢复均待官方资料验证 |
| 封装 | CH585M：QFN48 | 焊盘、尺寸与 Pin 表需按正式封装资料复核 |
| 时钟 | 片上 PLL、16 MHz 与 32 kHz 时钟；具体外部晶体要求未在本次摘录确认 | USB、BLE/RF 时钟精度与器件要求待参考手册/SDK |
| Debug | 单/双线仿真接口；手册指出启用后 PB15/PB14 占用 | 生产调试口、复用冲突和量产锁定策略待决策 |

## 3. Pin/外设初步冲突表

手册引脚复用表显示若干 UART/SPI/TMR/ADC/仿真调试复用功能，且 PB15/PB14 可被仿真调试占用。系统尚未提供已冻结的 DBG-C Interface Pin 映射，因此此处不指定 SWD/Reset/LED/Button 的 GPIO 编号。需要对 QFN48 全引脚建立矩阵并依据 WCH SDK 初始化/复用宏核对。

## 4. 资源分配状态

| 功能 | 计划需求 | MCU 资源/引脚分配 | 状态 |
|---|---|---|---|
| USB Device | CMSIS-DAP v2 + CDC | USBFS；PB10/PB11；USBHS 暂不启用 | SDK/描述符待验证 |
| SWD Engine | SWDIO/SWCLK 时序 | 两个通用 GPIO；Timer 不分配 | GPIO 时序与速率待验证 |
| Target UART | CDC 桥接 | UART0；PB4/PB7 | SDK/并发待验证 |
| RF/BLE | 私有 2.4G、BLE 管理共存 | 集成 Radio 共享资源 | SDK 共存待验证 |
| ADC/LED/Button | 状态及扩展 | ADC 不分配；LED/按键各预留 GPIO | 按键产品决策待定 |
| DBG-C Interface | Basic/Full 信号 | 无 Pin 映射，禁止冻结 | 待验证 |
| Debug/Boot/Reserve | 生产与恢复 | 引脚/存储分区未分配 | 待验证 |

## 5. V1 外设需求与原理图预分配

目标是给原理图设计提供明确的 MCU 资源基线。此分配是 **V1 原理图输入基线**，不是芯片性能通过验证，也不冻结 DBG-C Interface 的 Type-C Pin 映射。

| 功能 | 分配的 MCU 资源 | 原理图分配 | 说明与验证边界 |
|---|---|---|---|
| PC USB Device | USBFS 控制器/PHY + USB DMA | PB10=UD−、PB11=UD+；上行 USB-C 连接器 | V1 使用 USBFS；CMSIS-DAP v2 Bulk + CDC。端点描述符和 SDK USB Device 栈在固件阶段核验；USBHS 暂不启用 |
| Target UART/CDC | UART0 + USB CDC 虚拟串口 | UART0 PB4=RXD0、PB7=TXD0；到 DBG-C Interface 的 TX/RX 连接器 Pin 待接口冻结 | UART0 的 Modem 信号不需要。CDC 通过 USBFS 枚举并桥接 UART0 |
| Target SWD | 2 个通用 GPIO，由共享 SWD Engine 控制 | 预留 SWDIO、SWCLK 两根 MCU 网络；实际 GPIO 编号及连接器 Pin 待 Pin Matrix/接口冻结 | 采用 GPIO 驱动的 SWD。首版不占 SPI；时序和可达 SWD 频率须固件示波器验证 |
| Target Reset | 1 个通用 GPIO 输出 | 预留 TARGET_nRESET MCU 网络；脚号待 Pin Matrix | 不连接 CH585M 自身 RST。开漏/推挽、串阻/保护和默认态由接口电气设计确定 |
| BLE + 私有 2.4G | 芯片集成 Radio/Baseband、ANT 脚 | CH585M ANT 按 WCH RF 参考设计连匹配/天线网络 | 共享无线子系统；是否可同时运行待 SDK 核验。不分配外部 RF SPI |
| 状态/配对交互 | GPIO | 预留 LED 1 路、按键 1 路；脚号待 Pin Matrix | 按键是否量产保留待产品交互评审；LED 极性/电流待器件选型 |
| USB/连接状态检测 | GPIO（仅当选定的电路需要） | 预留 VBUS/连接检测网络位置，暂不定 Pin | 不推断 USBFS 自动提供 VBUS 检测；按 SDK 和供电/连接器方案决定 |
| 唯一身份/配对设置 | 芯片 UID + DataFlash | 固件读取 UID；DataFlash 尚未分配，等待配对持久化需求冻结后决定用途 | UID API、长度、写入策略及寿命需 SDK 核验；存储布局/配对持久化规则待定 |
| OTA 与固件 | CodeFlash + BootLoader | 按官方烧录/升级参考设计接入调试/恢复所需电路 | 448 KB CodeFlash 不预设双镜像；升级签名、回滚、断电安全尚待证实 |
| SWD 时序/系统时基 | Timer（按需）+ GPIO | 暂不选定 Timer 实例 | 先实现/测量 GPIO SWD；若精度或 CPU 占用不达目标，再根据 SDK 分配 Timer |
| DMA | USBFS DMA；Radio DMA 待确认 | 不做固定 DMA 通道连接 | 数据手册列明 USBFS DMA；DMA 通道数及 USB/RF 仲裁需 SDK/参考手册确认 |

### GPIO/引脚保留约束

1. USBFS 使用 PB10/PB11；UART0 使用 PB4/PB7。手册将其分别定义为 UD−/UD+ 和 RXD0/TXD0；正式落板前仍需用 CH585M SDK 初始化配置确认。
2. SWDIO、SWCLK、TARGET_nRESET、LED、按键从剩余普通 GPIO 中分配；当前不指定 Pin 编号，因为 Type-C 目标侧 Pin Map 尚未冻结。
3. PB14/PB15 在仿真 Debug 启用时分别被 TIO/TCK 占用，V1 可分配 GPIO 池先排除它们；同时保留 Probe 自身编程/恢复接口。
4. PB12/PB13 标记为 USBHS U2D−/U2D+，V1 不启用 USBHS；不要与 USBFS 的 PB10/PB11 混接。
5. CH585M 自身外部复位输入在数据手册复用表中标为 PB22/RST 功能；它不是 Target_nRESET 输出，预分配时明确分网。

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

项目已确定 V1 固件运行 ThreadX。Eclipse ThreadX 上游发布了 RISC-V32 端口；这不证明 CH585M 青稞 RISC-V3C 与 WCH 启动/中断框架可直接使用该端口。ThreadX 版本、编译器及 CH585M 移植状态须通过真实 SDK 工程编译和板上测试确认。

| 资源 | ThreadX 需求 | DBG-C 分配 | 当前证据边界 |
|---|---|---|---|
| Kernel tick | 周期性时基和中断 | 指定芯片内置 32 位 SysTick 作为 ThreadX kernel tick 来源；不占用 TMR0 至 TMR3 | CH585M 手册确认 SysTick 计数器及其中断存在。ThreadX 端口接入、时钟源选择、重装值及优先级须在 WCH SDK 工程中验证；若端口不能接入 SysTick，需先提交资源变更再改用通用定时器 |
| Context switch | CPU 上下文保存/恢复、调度入口 | 接入 WCH 启动与中断框架；定义 CMSIS-DAP、SWD、RF、USB 的线程/ISR 边界 | 上游 RISC-V32 端口与青稞中断/异常框架的兼容性待验证 |
| Interrupt | 外设 ISR 到 ThreadX 调度接口 | 在固件架构中分别定义 USB、Radio/BLE、Timer ISR 的 ThreadX API 使用边界 | 中断嵌套、优先级和 SDK ISR 约束需读取 SDK |
| SRAM | ThreadX 内核对象、系统栈、线程栈及应用缓冲 | 在 128 KB SRAM 链接布局中统一预算 USB DAP/CDC、RF 重组、BLE 栈、SWD 工作区 | 各项字节数由选定 ThreadX/SDK、线程数、栈水位和链接映射测量后分配；不填猜测值 |
| CPU | 调度、中断和协议处理时间 | 以 USB/RF 并发负载验证最坏响应时间 | 手册最高 78 MHz 不能单独证明符合性能目标 |
| Compiler/Port | 与 CH585M 工具链、ABI、启动代码兼容的 ThreadX 端口 | 锁定 ThreadX 和 WCH 工具链版本后集成 | 版本、编译器及移植补丁未确定 |

ThreadX 不增加外部连接器信号，不改变已分配的 USBFS、UART0、SWD GPIO、Target Reset GPIO 和集成 Radio。硬件侧须保留 Probe 编程/恢复路径。当前资源基线将 SysTick 指定为 ThreadX kernel tick 来源，TMR0 至 TMR3 保留；端口接入验证失败时，须记录并评审资源变更。

### ThreadX 上游依赖来源

V1 固件将通过 **Git Submodule** 引用 Eclipse ThreadX 官方 GitHub 项目：[eclipse-threadx/threadx](https://github.com/eclipse-threadx/threadx)。主仓库应记录子模块 URL 和精确 commit；子模块目录路径在固件仓库布局冻结时登记。项目 README 将 `master` 描述为包含最新代码的开发分支，并明确说明它不等同于最新 GA 发布版；因此集成基线须选定并验证后记录正式发布标签及对应 commit，不能仅记录 `master`。具体版本尚未确定，见 OPEN-001 的 O18。

上游仓库列出 `risc-v32` 架构，并提供 [GNU RISC-V32 端口目录](https://github.com/eclipse-threadx/threadx/tree/master/ports/risc-v32/gnu)，但这不证明该端口可直接用于 CH585M 的青稞 RISC-V3C、WCH 工具链或中断框架。DBG-C 将以该仓库为 ThreadX 内核及端口来源；是否直接采用该端口、修改范围和 CH585M 移植方式须在实际构建与板上验证后记录。克隆 DBG-C 固件仓库时须初始化并递归更新子模块，确保工作区检出主仓库记录的 ThreadX commit。

上游仓库根目录 [LICENSE.txt](https://github.com/eclipse-threadx/threadx/blob/master/LICENSE.txt) 标示 MIT License，其中要求在软件副本或实质部分中保留版权声明和许可声明。纳入源码或二进制发布前，应按实际采用版本检查其许可证文件及源码头部声明，并在 DBG-C 第三方组件清单与发布材料中保留所需声明。该记录不是法律意见。以上上游资料查阅日期：2026-09-28。

## 7. 必须补齐的资料

项目指定的 CH585M IC 手册、匹配的 WCH SDK/例程版本、官方封装图、芯片修订/勘误、USBFS Device 栈例程、RF/BLE 共存说明、OTA/ISP/IAP 示例及接口、电气时钟要求。取得后更新此文并记录文件版本/哈希与审阅日期。
