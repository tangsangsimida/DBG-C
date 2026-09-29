#ifndef DBGC_CH585_TARGET_RESET_GPIO_H
#define DBGC_CH585_TARGET_RESET_GPIO_H

#include <stdint.h>

#include "dbgc_ch585_gpio_mode.h"

/* PB5 / QFN48 pin 19 is allocated to Target_nRESET by MCU-001.
 * The caller selects its electrical mode and raw high/low level. This API
 * does not define reset pulse width, output topology, or default state. */
/* Callers must serialize mode changes with any other code configuring GPIOA. */
int dbgc_ch585_target_reset_gpio_configure(dbgc_ch585_gpio_mode_t mode);
int dbgc_ch585_target_reset_gpio_write(uint8_t high);
int dbgc_ch585_target_reset_gpio_read(uint8_t *high);

#endif
