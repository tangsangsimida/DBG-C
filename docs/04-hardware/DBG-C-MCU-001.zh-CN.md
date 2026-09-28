# DBG-C MCU 选型与资源评估

**文档编号：** DBG-C-MCU-001　**版本：** V0.1　**状态：** 初步数据手册摘录；资源分配未冻结

## 1. 证据来源

仓库文件 `docs/09-references/CH585-CH584_Datasheet_V1.6.pdf`，标题《CH585/CH584 数据手册》，V1.6，160 页。下述芯片能力来自该本地文件；实际硅版本/勘误、SDK 实现和并发能力未核实。必须补充 WCH 官方参考手册与 SDK 后复核。不得把 CH584/其他 CH 系列资料外推到 CH585M。

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

手册引脚复用表显示若干 UART/SPI/TMR/ADC/仿真调试复用候选，且 PB15/PB14 可被仿真调试占用。由于系统未给出已批准的 USB/RF/DBG-C 接口 Pin 分配，此处不指定任何最终 GPIO。需要对 QFN48 全引脚建立矩阵并依据 WCH SDK 初始化/复用宏验证。

## 4. 资源分配状态

| 功能 | 计划需求 | MCU 资源/引脚分配 | 状态 |
|---|---|---|---|
| USB Device | CMSIS-DAP v2 + CDC | USBFS/USBHS 待择；Endpoint 未分配 | 待验证 |
| SWD Engine | SWDIO/SWCLK 时序 | GPIO/定时器/DMA 未分配 | 待验证 |
| Target UART | CDC 桥接 | UART 与引脚未分配 | 待验证 |
| RF/BLE | 私有 2.4G、BLE 管理共存 | SDK 模式、Radio 调度未确认 | 待验证 |
| ADC/LED/Button | 状态及扩展 | 通道/引脚未分配 | 待验证 |
| DBG-C Interface | Basic/Full 信号 | 无 Pin 映射，禁止冻结 | 待验证 |
| Debug/Boot/Reserve | 生产与恢复 | 引脚/存储分区未分配 | 待验证 |

## 5. 必须补齐的资料

WCH 官方 CH585M 参考手册、与芯片匹配的 SDK/例程版本、官方封装图、芯片修订/勘误、USB FS/HS Device 栈例程、RF/BLE 共存说明、OTA/ISP/IAP 示例及接口、电气时钟要求。取得后更新此文并记录文件版本/哈希与审阅日期。
