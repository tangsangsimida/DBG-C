#include <stdint.h>
#include "tx_api.h"
#include "tx_initialize.h"
#include "wch/CH585SFR.h"
#include "wch/core_riscv.h"

#define DBG_C_SYSCLK_HZ 62400000UL

extern char _end[];
extern void tx_systick_isr(void);
extern VOID *_tx_initialize_unused_memory;

void tx_ch585_low_level_setup(void)
{
    uintptr_t first_free_memory;

    first_free_memory = ((uintptr_t)_end + (sizeof(ULONG) - 1U)) &
                        ~((uintptr_t)sizeof(ULONG) - 1U);
    _tx_initialize_unused_memory = (VOID *)first_free_memory;

    SetVTFIRQ((uint32_t)tx_systick_isr, SysTick_IRQn, 1U, ENABLE);
    PFIC_SetPriority(SysTick_IRQn, 0xf0U);
    if (SysTick_Config(DBG_C_SYSCLK_HZ / TX_TIMER_TICKS_PER_SECOND) != 0U) {
        for (;;) {
        }
    }
}
