# PoC-1: CH585M + Eclipse ThreadX

This is an experimental CH585M ThreadX port, not a board-verified port. It uses the official ThreadX RISC-V32/GNU context routines, WCH CH585 startup/linker inputs, an EVT-derived clock initialization, and a local low-level/SysTick adapter. Two same-priority threads increment debugger-visible counters, read ThreadX time, and sleep for one tick.

The local adapter configures SysTick using ThreadX's upstream default of 100 ticks/s and routes it through ThreadX timer/context routines. Host compilation and linking do not verify QingKe V3C hardware stack push, HPE/VTF behavior, exception return, clock frequency, tick delivery, or scheduling. Do not use this PoC as product firmware.

See `docs/05-firmware/DBG-C-FW-001.*.md` for exact source provenance, environment setup, build commands, and unresolved target-port work.

Run `./build.sh` to build the PoC, run the generic byte FIFO checks, exercise eight isolated CMSIS-DAP/SWD command-layer cases with a mocked SWD transaction API, including TransferConfigure retry-count behavior and AP posted reads in Transfer and TransferBlock, and compile the SWD-enabled test configuration to an ELF32 RISC-V object with WCH GCC. Test pin operations are no-ops; this is not product firmware. The script keeps build output under this PoC directory. Its default output directory is `build/`; set `DBGC_BUILD_DIR` to another relative directory under this project when a clean build is needed. None of these checks verifies CH585M runtime behavior.
