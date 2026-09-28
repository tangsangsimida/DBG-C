/*
 * Compiler primitives used by the pinned CMSIS-DAP sources on CH585M.
 *
 * This header only maps compiler and ISA operations. It does not implement
 * target GPIO, SWD timing, interrupt handling, or a CMSIS-DAP product port.
 */
#ifndef DBGC_CH585_CMSIS_COMPILER_H
#define DBGC_CH585_CMSIS_COMPILER_H

#include "CH585SFR.h"
#include "core_riscv.h"

#ifndef __STATIC_INLINE
#define __STATIC_INLINE static __INLINE
#endif
#ifndef __STATIC_FORCEINLINE
#define __STATIC_FORCEINLINE static __INLINE __attribute__((always_inline))
#endif
#ifndef __NOP
#define __NOP() __ASM volatile("nop")
#endif
#ifndef __WEAK
#define __WEAK __attribute__((weak))
#endif

#endif
