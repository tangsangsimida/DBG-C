#ifndef DBGC_TARGET_RESET_SEQUENCE_H
#define DBGC_TARGET_RESET_SEQUENCE_H

/* Hardware-independent ordering for a reset operation. The callbacks own
 * signal polarity, electrical mode, and the reset-hold timing policy. */
typedef int (*dbgc_target_reset_sequence_action_fn)(void *context);

typedef struct {
    void *context;
    dbgc_target_reset_sequence_action_fn assert_reset;
    dbgc_target_reset_sequence_action_fn hold_reset;
    dbgc_target_reset_sequence_action_fn release_reset;
} dbgc_target_reset_sequence_ops_t;

/* Return zero on success. All callbacks must return zero on success and a
 * nonzero error on failure. The hold callback runs only after a successful
 * assertion. Release is attempted after every assertion attempt; a release
 * error takes precedence, otherwise the first earlier error is returned.
 * Invalid operation tables return -1 without invoking callbacks.
 */
int dbgc_target_reset_sequence_execute(
    const dbgc_target_reset_sequence_ops_t *operations);

#endif
