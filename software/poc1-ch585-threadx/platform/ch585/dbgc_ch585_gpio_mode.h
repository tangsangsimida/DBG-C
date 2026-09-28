#ifndef DBGC_CH585_GPIO_MODE_H
#define DBGC_CH585_GPIO_MODE_H

/* Logical names for the five modes implemented by the WCH EVT GPIO driver.
 * A caller must select a mode only after reviewing the target electrical
 * interface. These values are not WCH SDK enum values. */
typedef enum {
    DBGC_CH585_GPIO_INPUT_FLOATING = 0,
    DBGC_CH585_GPIO_INPUT_PULL_UP,
    DBGC_CH585_GPIO_INPUT_PULL_DOWN,
    DBGC_CH585_GPIO_OUTPUT_PP_5MA,
    DBGC_CH585_GPIO_OUTPUT_PP_20MA
} dbgc_ch585_gpio_mode_t;

#endif
