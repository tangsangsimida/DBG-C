# PoC-1: CH585M + Eclipse ThreadX

This is a build integration probe, not a verified CH585M ThreadX port. It links the official ThreadX RISC-V32/GNU context implementation and the WCH CH585 startup/linker inputs. Two same-priority threads increment debugger-visible counters and yield.

The archived WCH FreeRTOS startup's default SysTick handler loops forever. This PoC does not configure SysTick. It therefore does not demonstrate tick-driven sleeps/timeouts, and it must not be used as product firmware or as evidence that upstream RISC-V32 context switching works with QingKe V3C hardware push-stack behavior.

See `docs/05-firmware/DBG-C-FW-001.*.md` for exact source provenance, environment setup, build commands, and unresolved target-port work.
