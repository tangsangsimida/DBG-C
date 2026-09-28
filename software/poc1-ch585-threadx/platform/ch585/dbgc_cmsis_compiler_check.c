#include <stdint.h>

#include "cmsis_compiler.h"

__STATIC_FORCEINLINE uint32_t dbgc_cmsis_compiler_inline_check(uint32_t value)
{
    __NOP();
    return value;
}

__WEAK uint32_t dbgc_cmsis_compiler_weak_check(uint32_t value)
{
    return dbgc_cmsis_compiler_inline_check(value);
}
