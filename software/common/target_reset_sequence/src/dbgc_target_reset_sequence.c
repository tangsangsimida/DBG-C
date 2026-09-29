#include "dbgc_target_reset_sequence.h"

int dbgc_target_reset_sequence_execute(const dbgc_target_reset_sequence_ops_t *operations)
{
	int assert_status;
	int hold_status = 0;
	int release_status;

	if ((operations == 0) || (operations->assert_reset == 0) || (operations->hold_reset == 0) ||
	    (operations->release_reset == 0)) {
		return -1;
	}

	assert_status = operations->assert_reset(operations->context);
	if (assert_status == 0) {
		hold_status = operations->hold_reset(operations->context);
	}
	release_status = operations->release_reset(operations->context);

	if (release_status != 0) {
		return release_status;
	}
	if (assert_status != 0) {
		return assert_status;
	}
	return hold_status;
}
