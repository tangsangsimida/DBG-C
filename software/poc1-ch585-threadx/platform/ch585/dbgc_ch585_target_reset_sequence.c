#include "dbgc_ch585_target_reset_sequence.h"

#include "dbgc_ch585_target_reset_gpio.h"
#include "dbgc_target_reset_sequence.h"

typedef struct {
	const dbgc_ch585_target_reset_sequence_config_t *configuration;
} dbgc_ch585_target_reset_context_t;

static int dbgc_ch585_target_reset_assert(void *context)
{
	const dbgc_ch585_target_reset_context_t *reset_context = context;

	return dbgc_ch585_target_reset_gpio_write(reset_context->configuration->asserted_high);
}

static int dbgc_ch585_target_reset_hold(void *context)
{
	const dbgc_ch585_target_reset_context_t *reset_context = context;

	return reset_context->configuration->hold(reset_context->configuration->context);
}

static int dbgc_ch585_target_reset_release(void *context)
{
	const dbgc_ch585_target_reset_context_t *reset_context = context;
	uint8_t released_high = (reset_context->configuration->asserted_high == 0U) ? 1U : 0U;

	return dbgc_ch585_target_reset_gpio_write(released_high);
}

int dbgc_ch585_target_reset_sequence_execute(
	const dbgc_ch585_target_reset_sequence_config_t *configuration)
{
	dbgc_ch585_target_reset_context_t context;
	dbgc_target_reset_sequence_ops_t operations;

	if ((configuration == 0) || (configuration->asserted_high > 1U) ||
	    (configuration->hold == 0)) {
		return -1;
	}

	context.configuration = configuration;
	operations.context = &context;
	operations.assert_reset = dbgc_ch585_target_reset_assert;
	operations.hold_reset = dbgc_ch585_target_reset_hold;
	operations.release_reset = dbgc_ch585_target_reset_release;

	return dbgc_target_reset_sequence_execute(&operations);
}
