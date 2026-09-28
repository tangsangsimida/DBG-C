#ifndef DBGC_CH585_TARGET_RESET_SEQUENCE_H
#define DBGC_CH585_TARGET_RESET_SEQUENCE_H

#include <stdint.h>

typedef int (*dbgc_ch585_target_reset_hold_fn)(void *context);

typedef struct {
    uint8_t asserted_high;
    dbgc_ch585_target_reset_hold_fn hold;
    void *context;
} dbgc_ch585_target_reset_sequence_config_t;

/* Before calling, the caller must place PA4 in its approved, released output
 * state. The caller supplies raw assertion level and hold policy; this adapter
 * defines neither reset polarity, pulse width, electrical mode, nor scheduler
 * use. Safe GPIO initialization and output-transition behavior require
 * electrical review and board verification.
 */
int dbgc_ch585_target_reset_sequence_execute(
    const dbgc_ch585_target_reset_sequence_config_t *configuration);

#endif
