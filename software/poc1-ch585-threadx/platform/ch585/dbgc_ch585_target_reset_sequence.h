#ifndef DBGC_CH585_TARGET_RESET_SEQUENCE_H
#define DBGC_CH585_TARGET_RESET_SEQUENCE_H

#include <stdint.h>

typedef int (*dbgc_ch585_target_reset_hold_fn)(void *context);

typedef struct {
    uint8_t asserted_high;
    dbgc_ch585_target_reset_hold_fn hold;
    void *context;
} dbgc_ch585_target_reset_sequence_config_t;

/* PA4 must already be configured as a suitable output by the caller. The
 * caller supplies raw assertion level and hold policy; this adapter defines
 * neither reset polarity, pulse width, electrical mode, nor scheduler use.
 */
int dbgc_ch585_target_reset_sequence_execute(
    const dbgc_ch585_target_reset_sequence_config_t *configuration);

#endif
