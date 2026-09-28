/* Host-only compiler primitives; this is not the CH585M implementation. */
#ifndef DBGC_CMSIS_COMPILER_HOST_TEST_H
#define DBGC_CMSIS_COMPILER_HOST_TEST_H

#define __STATIC_INLINE static inline
#define __STATIC_FORCEINLINE static inline
#define __NOP() ((void)0)
#define __WEAK __attribute__((weak))

#endif
