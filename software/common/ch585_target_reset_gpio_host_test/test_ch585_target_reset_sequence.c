#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbgc_ch585_target_reset_gpio.h"
#include "dbgc_ch585_target_reset_gpio_host_regs.h"
#include "dbgc_ch585_target_reset_sequence.h"

volatile uint32_t dbgc_host_R32_PA_DIR;
volatile uint32_t dbgc_host_R32_PA_PIN;
volatile uint32_t dbgc_host_R32_PA_CLR;
volatile uint32_t dbgc_host_R32_PA_PU;
volatile uint32_t dbgc_host_R32_PA_PD_DRV;
volatile uint32_t dbgc_host_R32_PA_SET;

static unsigned int checks;

typedef struct {
	uint8_t asserted_high;
	int hold_result;
	unsigned int hold_calls;
} reset_fixture_t;

static void check_value(uint32_t actual, uint32_t expected, const char *expression,
			unsigned int line)
{
	++checks;
	if (actual != expected) {
		fprintf(stderr, "line %u: %s: got 0x%08lx, expected 0x%08lx\n", line, expression,
			(unsigned long)actual, (unsigned long)expected);
		exit(EXIT_FAILURE);
	}
}

static void check_result(int actual, int expected, const char *expression, unsigned int line)
{
	++checks;
	if (actual != expected) {
		fprintf(stderr, "line %u: %s: got %d, expected %d\n", line, expression, actual,
			expected);
		exit(EXIT_FAILURE);
	}
}

#define CHECK_VALUE(actual, expected) check_value((actual), (expected), #actual, __LINE__)
#define CHECK_RESULT(actual, expected) check_result((actual), (expected), #actual, __LINE__)

static int hold_reset(void *context)
{
	reset_fixture_t *fixture = context;

	++fixture->hold_calls;
	if (fixture->asserted_high != 0U) {
		CHECK_VALUE(dbgc_host_R32_PA_SET, 0x10U);
	} else {
		CHECK_VALUE(dbgc_host_R32_PA_CLR, 0x10U);
	}
	return fixture->hold_result;
}

static void run_reset_case(uint8_t asserted_high, int hold_result)
{
	reset_fixture_t fixture = { asserted_high, hold_result, 0U };
	dbgc_ch585_target_reset_sequence_config_t configuration;
	int expected_result = hold_result;

	dbgc_host_R32_PA_DIR = 0U;
	dbgc_host_R32_PA_PIN = 0U;
	dbgc_host_R32_PA_CLR = 0U;
	dbgc_host_R32_PA_PU = 0U;
	dbgc_host_R32_PA_PD_DRV = 0U;
	dbgc_host_R32_PA_SET = 0U;

	/* This is a host-test fixture mode, not a product electrical decision. */
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_configure(DBGC_CH585_GPIO_OUTPUT_PP_5MA), 0);
	CHECK_VALUE(dbgc_host_R32_PA_DIR & 0x10U, 0x10U);

	configuration.asserted_high = asserted_high;
	configuration.hold = hold_reset;
	configuration.context = &fixture;

	CHECK_RESULT(dbgc_ch585_target_reset_sequence_execute(&configuration), expected_result);
	CHECK_VALUE(fixture.hold_calls, 1U);
	CHECK_VALUE(dbgc_host_R32_PA_SET, 0x10U);
	CHECK_VALUE(dbgc_host_R32_PA_CLR, 0x10U);
}

int main(void)
{
	dbgc_ch585_target_reset_sequence_config_t invalid = { 0U, 0, 0 };

	dbgc_host_R32_PA_DIR = 0xA1U;
	dbgc_host_R32_PA_PIN = 0xB2U;
	dbgc_host_R32_PA_CLR = 0xC3U;
	dbgc_host_R32_PA_PU = 0xD4U;
	dbgc_host_R32_PA_PD_DRV = 0xE5U;
	dbgc_host_R32_PA_SET = 0xF6U;
	CHECK_RESULT(dbgc_ch585_target_reset_sequence_execute(0), -1);
	CHECK_RESULT(dbgc_ch585_target_reset_sequence_execute(&invalid), -1);
	invalid.asserted_high = 2U;
	invalid.hold = hold_reset;
	CHECK_RESULT(dbgc_ch585_target_reset_sequence_execute(&invalid), -1);
	CHECK_VALUE(dbgc_host_R32_PA_DIR, 0xA1U);
	CHECK_VALUE(dbgc_host_R32_PA_PIN, 0xB2U);
	CHECK_VALUE(dbgc_host_R32_PA_CLR, 0xC3U);
	CHECK_VALUE(dbgc_host_R32_PA_PU, 0xD4U);
	CHECK_VALUE(dbgc_host_R32_PA_PD_DRV, 0xE5U);
	CHECK_VALUE(dbgc_host_R32_PA_SET, 0xF6U);

	run_reset_case(1U, 0);
	run_reset_case(0U, 0);
	run_reset_case(1U, -7);
	run_reset_case(0U, -9);

	printf("PASS: %u CH585 Target Reset sequence integration checks\n", checks);
	return EXIT_SUCCESS;
}
