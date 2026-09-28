#include "dbgc_ch585_swd_gpio.h"

#if defined(DBGC_CH585_SWD_GPIO_HOST_TEST)
#include "dbgc_ch585_swd_gpio_host_regs.h"
#else
#include "wch/CH585SFR.h"
#endif

#define DBGC_CH585_PB5_MASK (1UL << 5)
#define DBGC_CH585_PB6_MASK (1UL << 6)

static int dbgc_ch585_swd_gpio_mask(dbgc_ch585_swd_signal_t signal,
                                    uint32_t *mask)
{
    if (mask == 0) {
        return -1;
    }

    switch (signal) {
    case DBGC_CH585_SWD_SIGNAL_SWDIO:
        *mask = DBGC_CH585_PB5_MASK;
        return 0;
    case DBGC_CH585_SWD_SIGNAL_SWCLK:
        *mask = DBGC_CH585_PB6_MASK;
        return 0;
    default:
        return -1;
    }
}

int dbgc_ch585_swd_gpio_configure(dbgc_ch585_swd_signal_t signal,
                                  dbgc_ch585_gpio_mode_t mode)
{
    uint32_t mask;

    if (dbgc_ch585_swd_gpio_mask(signal, &mask) != 0) {
        return -1;
    }

    switch (mode) {
    case DBGC_CH585_GPIO_INPUT_FLOATING:
        R32_PB_PD_DRV &= ~mask;
        R32_PB_PU &= ~mask;
        R32_PB_DIR &= ~mask;
        return 0;
    case DBGC_CH585_GPIO_INPUT_PULL_UP:
        R32_PB_PD_DRV &= ~mask;
        R32_PB_PU |= mask;
        R32_PB_DIR &= ~mask;
        return 0;
    case DBGC_CH585_GPIO_INPUT_PULL_DOWN:
        R32_PB_PD_DRV |= mask;
        R32_PB_PU &= ~mask;
        R32_PB_DIR &= ~mask;
        return 0;
    case DBGC_CH585_GPIO_OUTPUT_PP_5MA:
        R32_PB_PD_DRV &= ~mask;
        R32_PB_DIR |= mask;
        return 0;
    case DBGC_CH585_GPIO_OUTPUT_PP_20MA:
        R32_PB_PD_DRV |= mask;
        R32_PB_DIR |= mask;
        return 0;
    default:
        return -1;
    }
}

int dbgc_ch585_swd_gpio_write(dbgc_ch585_swd_signal_t signal, uint8_t high)
{
    uint32_t mask;

    if (dbgc_ch585_swd_gpio_mask(signal, &mask) != 0) {
        return -1;
    }

    if (high != 0U) {
        R32_PB_SET = mask;
    } else {
        R32_PB_CLR = mask;
    }

    return 0;
}

int dbgc_ch585_swd_gpio_read(dbgc_ch585_swd_signal_t signal, uint8_t *high)
{
    uint32_t mask;

    if ((high == 0) || (dbgc_ch585_swd_gpio_mask(signal, &mask) != 0)) {
        return -1;
    }

    *high = ((R32_PB_PIN & mask) != 0U) ? 1U : 0U;
    return 0;
}
