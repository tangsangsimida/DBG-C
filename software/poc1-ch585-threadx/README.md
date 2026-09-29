# PoC-1: CH585M + Eclipse ThreadX

This is an experimental CH585M ThreadX port, not a board-verified port. It uses the official ThreadX RISC-V32/GNU context routines, WCH CH585 startup/linker inputs, an EVT-derived clock initialization, and a local low-level/SysTick adapter. Two same-priority threads increment debugger-visible counters, record their latest ThreadX tick, and sleep for one tick. The application also exposes each `tx_thread_create()` result so a board run can detect failed thread creation before interpreting the counters.

The PoC local `tx_user.h` explicitly configures `TX_TIMER_TICKS_PER_SECOND` to 1000 ticks/s. Host compilation and linking do not verify QingKe V3C hardware stack push, HPE/VTF behavior, exception return, clock frequency, tick delivery, or scheduling. Do not use this PoC as product firmware.

Build and development scripts are centralized under `software/tools/python/`. See the bilingual [Chinese tool guide](../tools/python/README.zh-CN.md) and [English tool guide](../tools/python/README.en-US.md), and `docs/05-firmware/DBG-C-FW-001.*.md` for source provenance and validation limits. The build tool resolves external programs from `PATH` or documented environment variables; it does not assume a fixed tool installation path.
