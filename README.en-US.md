# DBG-C Debugger Project

DBG-C is a debugger project based on the CH585M. This repository maintains product requirements, system architecture, interfaces, MCU resource assessment, firmware PoCs, and verification records. Follow the status recorded in each document: the CH585M ThreadX PoC has not been verified on hardware, and host checks or cross-builds must not be described as board verification.

## Repository Layout

- [`docs/`](docs/README.en-US.md): engineering document index and project references.
- [`software/`](software/): root for software source, build configuration, and build outputs.
- [`hardware/`](hardware/): root for hardware files such as schematics and symbol libraries.

## Key Documents

- [Project index](docs/00-project/README.en-US.md) · [Open questions](docs/00-project/DBG-C-OPEN-001.en-US.md)
- [Product requirements](docs/01-requirements/DBG-C-PRD-001.en-US.md) · [System architecture](docs/02-system/DBG-C-SYS-001.en-US.md)
- [MCU selection and resource assessment](docs/04-hardware/DBG-C-MCU-001.en-US.md): CH585M QFN48 pin allocation and peripheral-resource status.
- [Firmware architecture and PoC-1](docs/05-firmware/DBG-C-FW-001.en-US.md) · [Verification specification](docs/07-verification/DBG-C-TEST-001.en-US.md)
- [Python developer tools](software/tools/python/README.en-US.md)

## Contributions and Version Management

Controlled documents are maintained as `.zh-CN.md` and `.en-US.md` pairs. When changing one language, cross-check its counterpart, related links, version, and status. Follow the [project coding standard](docs/05-firmware/DBG-C-CODE-001.en-US.md) for software. Manage document and code changes as small, independent patches under the Linux kernel development model. Use `<subsystem>: <concise subject>` commit subjects and Git history as the change log. See the Linux kernel [development process](https://docs.kernel.org/process/howto.html) and [patch submission guide](https://docs.kernel.org/process/submitting-patches.html).
