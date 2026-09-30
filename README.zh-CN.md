# DBG-C 调试器项目

DBG-C 是基于 CH585M 的调试器项目。本仓库维护产品需求、系统架构、接口、MCU 资源评估、固件 PoC 和验证记录。项目文档中的方案与资源分配仍按各文件状态管理；CH585M ThreadX 实板运行尚未验证，不能将主机检查或交叉构建描述为实板验证。

## 目录

- [`docs/`](docs/README.md)：项目文档总索引及研发资料。
- [`software/`](software/)：软件源码、构建配置与构建输出根目录。
- [`hardware/`](hardware/)：硬件文件根目录，供原理图、符号库等资料使用。

## 关键文档

- [项目索引](docs/00-project/README.zh-CN.md) · [未决问题](docs/00-project/DBG-C-OPEN-001.zh-CN.md)
- [产品需求](docs/01-requirements/DBG-C-PRD-001.zh-CN.md) · [系统架构](docs/02-system/DBG-C-SYS-001.zh-CN.md)
- [MCU 选型与资源评估](docs/04-hardware/DBG-C-MCU-001.zh-CN.md)：CH585M QFN48 引脚分配及外设资源状态。
- [固件架构与 PoC-1](docs/05-firmware/DBG-C-FW-001.zh-CN.md) · [验证规范](docs/07-verification/DBG-C-TEST-001.zh-CN.md)
- [Python 开发工具](software/tools/python/README.zh-CN.md)

## 贡献与版本管理

受控文档使用 `.zh-CN.md` 和 `.en-US.md` 成对维护。修改一侧时同步检查另一侧、相关链接、版本和状态。软件代码遵循 [项目代码规范](docs/05-firmware/DBG-C-CODE-001.zh-CN.md)。文档和代码按 Linux 内核补丁流程进行小步、独立审查；提交信息使用 `<子系统>: <简明主题>`，Git 历史作为变更记录。参考 Linux 内核[开发流程](https://docs.kernel.org/process/howto.html)和[补丁提交指南](https://docs.kernel.org/process/submitting-patches.html)。
