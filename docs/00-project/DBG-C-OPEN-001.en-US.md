# DBG-C Open Questions and Verification List

**Document ID:** DBG-C-OPEN-001　**Version:** V0.28　**Status:** Open items

| ID | Question | Evidence/decision required | Affected documents | Status |
|---|---|---|---|---|
| O01 | What CH585M private RF PHY/API is available, and what are BLE/private-RF coexistence limits? | Official WCH reference manual, SDK/examples, and measurements | MCU, RF, FW, TEST | To obtain |
| O02 | Does the WCH USBFS Device stack provide the endpoints and buffers required for CMSIS-DAP v2 Bulk + CDC? | CH585M USBFS Device SDK example, endpoint APIs, host OS tests | MCU, USB, SYS | To verify |
| O03 | What RF DAP round-trip latency/retry limits work with common debuggers? | CMSIS-DAP host measurements and RF prototype | RF, TEST, PRD | To verify |
| O04 | What Type-C pins/cable approach is valid and reliable for Basic/Full? | Applicable USB-IF specification, cable construction evidence, electrical review | IF, HW, TEST | To research |
| O05 | How do USB insertion, user choice, and wireless connection determine Standalone/Host/Target? | Product state-machine review, including conflicts/transitions | PRD, SYS, RF, BLE | Decision needed |
| O06 | Is pairing persistent/automatic, and what happens after unpair? | UX/security review and persistence tests | PRD, RF, BLE | Decision needed |
| O07 | What is the Device ID source, length, API, and authentication binding? | CH585M SDK/official interface and security review | MCU, RF, BLE | To verify |
| O08 | What USB VID/PID, interface/endpoints, strings, and serial policy will be used? | Formal implementation and VID ownership decision | USB, TEST | Decision needed |
| O09 | What CDC baud rates, flow control, target levels, and performance limits apply? | User requirements, electrical design, and measurements | PRD, IF, TEST | Decision needed |
| O10 | Does OTA support signatures, dual image, rollback, and power-loss recovery? | WCH Boot/SDK docs, examples, and power-cut testing | MCU, FW, BLE, RF, RISK | To verify |
| O11 | What are target UART/RESET/SWD voltage limits, protection needs, and SWD GPIO operating modes? | Freeze Target compatibility, electrical limits, SWDIO pulls/output drive/idle state/direction-switch requirements, and measurements | IF, HW, FW, TEST | Decision needed; BSP exposes explicit mode selection, but product electrical mode is undefined and must not be guessed during integration |
| O12 | What are performance goals for DAP latency, throughput, range, and endurance? | Prototype data and product review | PRD, RF, TEST | Decision needed |
| O13 | Is V1 BLE OTA limited to Probe self-update, or is RF OTA also needed? | Requirement and security/resource review | PRD, BLE, RF | Decision needed |
| O14 | Which PC OS, IDE, OpenOCD, and pyOCD versions are supported? | Product support policy and interoperability tests | PRD, USB, TEST | Decision needed |
| O15 | Which datasheet revision/errata/reference manual apply to the current silicon? | WCH release page and chip revision check | MCU, HW, FW | To obtain |
| O16 | What BLE wireless-download protocol, image format, target scope, and resume rules apply? | Define DBG-C Tool ↔ Probe protocol and select an acceptance target board | PRD, BLE, TEST | Decision needed |
| O17 | Can USBFS and USBHS operate concurrently? What implementation constraint would require V1 to switch from its USBFS allocation to USBHS? | SDK examples, official resource limits, comparative measurements | MCU, USB, SYS | To verify |
| O18 | Can Eclipse ThreadX RISC-V32/GNU context routines and the local CH585M low-level adapter run correctly on QingKe V3C? | Verify HPE, PFIC/VTF, startup, exception frames, SysTick, sleep/wakeup, scheduling, and sustained runtime on CH585M | MCU, FW, TEST | Board verification pending; software gate releases verification-board design, but no board is currently available and chip runtime has not been tested |
| O19 | What is the PoC-1 software-verification gate before hardware design? | Apply the O19 criteria: two reproducible builds, ELF/link resource checks, and static review of startup/ThreadX context/interrupt paths; release verification-board design only, with product freeze gated by board tests | MCU, FW, SYS, TEST | Software gate passed for verification-hardware design only; tick-target conflict remains open; board runtime and product-hardware freeze have not passed |
| O20 | Can the Arm CMSIS-DAP firmware core compile with the CH585M WCH RISC-V GCC, and what compiler/ISA adaptation is evidence-based? | Review compiler headers, inline assembly, and port dependencies in the pinned upstream commit; verify target compilation and host command-layer behavior without Arm ISA assembly | FW, MCU, TEST | Local CMSIS compiler-macro adaptation and compile checks pass; eight command-core host checks and nine SWD line-model cases pass; WCH GCC compiles test-configured `DAP.c` and `SW_DP.c` into ELF32 RISC-V objects. Product HAL, timing calibration, USB integration, and firmware linking are incomplete, so O20 remains open |
| O21 | How are actual input length and response capacity validated before calling the upstream CMSIS-DAP command processor? | Fix the USB receive-length/buffer contract, DAP_PACKET_SIZE, and product DAP configuration; then add bounded parsing/response budgeting for all enabled commands and DAP_ExecuteCommands, with truncated, overflow, and response-capacity host cases | USB, FW, TEST | To define; upstream `DAP_ExecuteCommand()`/`DAP_ProcessCommand()` have no length parameters, and some commands walk variable-length data based on request fields. No USB receive implementation or frozen DAP_PACKET_SIZE exists; no bounds adapter is implemented. Existing host command checks are not malformed/truncated-packet security tests |

## O18 Evidence Update

- Host cases FIFO-01 through FIFO-07 for the generic byte FIFO pass; the existing build entry reports 3683 assertions passed. This covers only pure C FIFO behavior, does not close O18, and does not prove ThreadX/ISR/chip runtime behavior.
- ThreadX is pinned to `v6.5.1.202602a_rel`, commit `b91b03b9e75fa523b17127f9e0eca09dca916459`; MounRiver Linux x64 Toolchain V2.4.0 GCC 12.2.0 is installed in the current user account.
- PoC-1 now contains experimental clock initialization, a low-level unused-memory boundary, VTF SysTick registration, and a ThreadX tick ISR. The threads read `tx_time_get()` and sleep for one tick. Host cross-build passed; output is ELF32 RISC-V, text 8876, data 8, bss 5564 bytes.
- The tick uses the upstream ThreadX header default of 100 ticks/s. HPE/VTF behavior, exception frames, actual SysTick frequency, tick delivery, sleep/wakeup, scheduling, and endurance remain unverified on hardware; O18 remains open.
- The user confirms no hardware is currently available. Apply two release gates: two clean rebuilds produced identical ELF/map SHA-256 values, the ELF was checked against the linker script and startup symbols, and source review covered startup, ThreadX context, and interrupt paths. The software gate is therefore passed for limited-purpose verification-hardware design. This does not claim ThreadX runtime verification on CH585M and does not release product schematic or PCB freeze. Source is currently configured for 100 ticks/s while existing requirement text says 1000 ticks/s; this conflict remains open and must be resolved before the board-test tick-frequency acceptance threshold is frozen. Interrupt, SysTick, scheduling, and sleep/wakeup have not been runtime-verified.
- The upstream ThreadX `qemu_virt` example uses QEMU virt addresses, entry, and linker layout, unlike the PoC WCH CH585 startup and PFIC/VTF code. QEMU, Spike, and Renode are unavailable in the current environment; no simulator evidence exists.
- The current PoC source sets `TX_TIMER_TICKS_PER_SECOND=100`; with the source-declared 62.4 MHz, the compare value calculates to 623999. The user's mention of 1000 ticks/s is not implemented in source and must be confirmed before changing the setting.
- `docs/05-firmware/DBG-C-FW-001.en-US.md` records EVT provenance, archive hashes, and host setup. The WCH EVT `.cproject` establishes a GCC12 configuration but not the GCC patch version; the installed toolchain was measured as GCC 12.2.0.

## O20 Evidence Update

- Added `software/poc1-ch585-threadx/platform/ch585/cmsis_compiler.h`, mapping CMSIS inline, NOP, and weak-symbol macros using the exact GNU definitions in WCH EVT `CH585SFR.h`/`core_riscv.h` and GCC/RISC-V compiler primitives. GCC 12.2.0 compiled the check object, and its ELF symbol table reports the check function as `WEAK`.
- WCH GCC compiled the pinned upstream `DAP.c` and `SW_DP.c` under the test configuration with this header into ELF32 RISC-V objects. Pin macros remain no-ops, and the SWD transaction model remains host-test-only. Neither object is linked into PoC.
- Upstream `DAP.h` still requires a build-directory copy to select the C-loop delay branch instead of its Arm `subs` assembly. No SWD GPIO HAL, PB5/PB6 electrical/timing calibration, DAP USB/ThreadX integration, or product firmware link exists. This is compiler-primitive and target-object compile evidence; it does not close O20 or establish CH585M board runtime.

- Four upstream `SW_DP.c` host line-model cases now cover read success/parity failure, write data/parity, and WAIT/FAULT ACK paths. The model uses callback pins and a no-op fast delay; it provides no electrical/timing or hardware evidence.
- WCH EVT GPIO API spellings are statically confirmed, but no product GPIO HAL has been integrated or built. PB5/PB6 voltage compatibility, protection, drive mode, and timing remain open.
- Eight host checks pass for pinned CMSIS-DAP commit `12636590eec66fae2d1bba4518749426ad5a4595`: firmware-version Info, unknown Info identifier, unsupported command, DP read with WAIT retries, DP write completion, ordered AP read results and final `DP_RDBUFF` in Transfer and TransferBlock, and TransferConfigure retry-count behavior on a following DP read.
- SWD command processing is enabled, but pin macros are no-ops and `SWD_Transfer()` is a deterministic mock. The test calls no GPIO, USB, ThreadX, or CH585M API. A build-directory header copy changes only the delay-branch selector; it does not define `__CC_ARM` or modify the third-party submodule.
- WCH RISC-V GCC compiles the SWD-enabled test configuration to an ELF32 RISC-V object; the object is not linked into PoC.
- These results do not prove CMSIS-DAP product integration, physical SWD transfers, product configuration, or USB integration. O20 remains open.

- Nine host cases with 168 assertions cover upstream `SW_DP.c` read/write behavior, WAIT/FAULT ACKs, 10-bit SWD input/output sequences, the DAP_Connect plus DP IDCODE path, AP posted-read data returned through DP_RDBUFF in DAP_Transfer, the two-item AP DAP_TransferBlock read sequence, and mixed output/input DAP_SWD_Sequence handling. Fast delay is a no-op, so counts are not frequency/timing evidence. WCH RISC-V GCC compiles `SW_DP.c` to an ELF32 RISC-V object that is not linked into product firmware.
- WCH EVT GPIO header/implementation statically confirm `GPIOB_ModeCfg`, `GPIOB_SetBits`, `GPIOB_ResetBits`, and `GPIOB_ReadPortPin`. These APIs are not integrated into an SWD HAL; PB5/PB6 electrical, timing, and protection requirements remain for IF-001 and verification-board measurements.
- O20 remains open; host models and object compilation do not prove physical SWD, GPIO, ThreadX, USB, or CH585M runtime behavior.

## O19 Release Criteria and Verdict

## Verification-board design release gate

Before limited-purpose **verification-hardware design**, all of these reviewable conditions are required:

1. Source/submodule revisions and toolchain versions are fixed. At least two clean builds produce ELF and map files with identical SHA-256 values. Any undecided configuration is explicitly recorded and is not represented as a frozen requirement.
2. Check ELF architecture, endianness, entry, startup symbols, load/run addresses, and compare them with the actual startup and linker script. Every static section must fit within the Flash/RAM ranges declared by that script. Record text/data/bss and statically allocated stacks/buffers. This PoC declares two 1024-byte application thread stacks, a 2048-byte ISR stack, and a 512-byte ThreadX timer stack; ELF data is 8 bytes and bss is 5564 bytes. This does not establish stack watermarks on hardware.
3. Review reset/clock initialization, ThreadX context save/restore, exception frame and stack switching, MIE/HPE/PFIC/VTF setup, SysTick reload/status clear, and ISR-to-scheduler call order. No discovered and unresolved source, symbol, or link inconsistency may remain.
4. Separate statically decidable items from chip-runtime items. Hardware exception semantics, measured frequency, interrupt delivery, tick, scheduling, and sleep/wakeup that cannot be proven from source/ELF remain board-test items and must not be marked Pass.
5. Verification-board design inputs must provide programming/recovery, reset, system-clock, SysTick, and interrupt-activity observation/measurement paths, with debug and measurement access points. Confirm exact pins and circuitry from authoritative project sources.

**Current verdict: the verification-board design gate has passed; product-hardware freeze has not passed.** The two reproducible builds, ELF/link resource checks, and source static review are complete, so a minimal board for verification may be designed. It must provide programming/recovery access and measurement access for reset, system clock, SysTick, and interrupt activity. Design details must be grounded in official chip material and the test plan. This is not a release to freeze the product schematic or PCB.

Before product-hardware freeze, CH585M board records must show ThreadX startup, thread run/switch/sleep/wakeup, measured SysTick frequency and tick delivery, interrupt entry/exit and context/stack integrity, reset/sleep-wakeup, and sustained operation meeting duration, repetition, load, and pass thresholds frozen in advance. Keep untested items Not run; O18 cannot close before board testing. The current 100 ticks/s source setting conflicts with existing 1000 ticks/s requirement text; resolve that decision before freezing the board-test frequency acceptance threshold.

## Confirmed Evidence Boundary

- The repository contains `docs/09-references/CH585-CH584_Datasheet_V1.6.pdf`, the `docs/09-references/CH585EVT/CH585EVT.ZIP` archive, and the limited EVT startup/linker/header copies used by PoC-1. No separate CH585 reference manual, DBG-C schematic, board-test record, or packet capture is present.
- The EVT index is dated 2026.08 and its FreeRTOS example uses FreeRTOS-Kernel V11.3.0; the material does not declare a standalone WCH SDK semantic version. Extracted APIs/registers support the ThreadX PoC only and do not prove RF/BLE/USB resource coexistence or board behavior.
- DBG-C Interface pins, RF frame fields, USB descriptors, and role-switch rules are not frozen.
- The product direction includes BLE dongle-free target download and USBFS as the V1 USB allocation. BLE download lacks an application protocol/target scope; USBFS endpoint and SDK Device compatibility still require verification.
