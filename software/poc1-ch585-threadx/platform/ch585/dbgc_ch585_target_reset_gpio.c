#include "dbgc_ch585_target_reset_gpio.h"

#if defined(DBGC_CH585_TARGET_RESET_GPIO_HOST_TEST)
#include "dbgc_ch585_target_reset_gpio_host_regs.h"
#else
#include "wch/CH585SFR.h"
#endif

#define DBGC_CH585_PA4_MASK (1UL << 4)

int dbgc_ch585_target_reset_gpio_configure(dbgc_ch585_gpio_mode_t mode)
{
    switch (mode) {
    case DBGC_CH585_GPIO_INPUT_FLOATING:
        R32_PA_PD_DRV &= ~DBGC_CH585_PA4_MASK;
        R32_PA_PU &= ~DBGC_CH585_PA4_MASK;
        R32_PA_DIR &= ~DBGC_CH585_PA4_MASK;
        return 0;
    case DBGC_CH585_GPIO_INPUT_PULL_UP:
        R32_PA_PD_DRV &= ~DBGC_CH585_PA4_MASK;
        R32_PA_PU |= DBGC_CH585_PA4_MASK;
        R32_PA_DIR &= ~DBGC_CH585_PA4_MASK;
        return 0;
    case DBGC_CH585_GPIO_INPUT_PULL_DOWN:
        R32_PA_PD_DRV |= DBGC_CH585_PA4_MASK;
        R32_PA_PU &= ~DBGC_CH585_PA4_MASK;
        R32_PA_DIR &= ~DBGC_CH585_PA4_MASK;
        return 0;
    case DBGC_CH585_GPIO_OUTPUT_PP_5MA:
        R32_PA_PD_DRV &= ~DBGC_CH585_PA4_MASK;
        R32_PA_DIR |= DBGC_CH585_PA4_MASK;
        return 0;
    case DBGC_CH585_GPIO_OUTPUT_PP_20MA:
        R32_PA_PD_DRV |= DBGC_CH585_PA4_MASK;
        R32_PA_DIR |= DBGC_CH585_PA4_MASK;
        return 0;
    default:
        return -1;
    }
}

int dbgc_ch585_target_reset_gpio_write(uint8_t high)
{
    if (high != 0U) {
        R32_PA_SET = DBGC_CH585_PA4_MASK;
    } else {
        R32_PA_CLR = DBGC_CH585_PA4_MASK;
    }

    return 0;
}

int dbgc_ch585_target_reset_gpio_read(uint8_t *high)
{
    if (high == 0) {
        return -1;
    }

    *high = ((R32_PA_PIN & DBGC_CH585_PA4_MASK) != 0U) ? 1U : 0U;
    return 0;
}
