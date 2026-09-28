#ifndef DBGC_CH585_SWD_GPIO_H
#define DBGC_CH585_SWD_GPIO_H

#include <stdint.h>

typedef enum {
    DBGC_CH585_SWD_SIGNAL_SWDIO = 0,
    DBGC_CH585_SWD_SIGNAL_SWCLK = 1
} dbgc_ch585_swd_signal_t;

/* Values describe the modes present in the WCH EVT GPIO driver. The caller
 * must select the electrical mode after reviewing the target interface. */
typedef enum {
    DBGC_CH585_GPIO_INPUT_FLOATING = 0,
    DBGC_CH585_GPIO_INPUT_PULL_UP,
    DBGC_CH585_GPIO_INPUT_PULL_DOWN,
    DBGC_CH585_GPIO_OUTPUT_PP_5MA,
    DBGC_CH585_GPIO_OUTPUT_PP_20MA
} dbgc_ch585_gpio_mode_t;

/* Return zero on success and -1 for an unsupported signal/mode or null output.
 * These functions do not provide synchronization. Callers must serialize
 * mode changes with any other code that configures GPIOB. */
int dbgc_ch585_swd_gpio_configure(dbgc_ch585_swd_signal_t signal,
                                  dbgc_ch585_gpio_mode_t mode);
int dbgc_ch585_swd_gpio_write(dbgc_ch585_swd_signal_t signal, uint8_t high);
int dbgc_ch585_swd_gpio_read(dbgc_ch585_swd_signal_t signal, uint8_t *high);

#endif
