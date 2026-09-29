#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbgc_ch585_target_reset_gpio.h"
#include "dbgc_ch585_target_reset_gpio_host_regs.h"

volatile uint32_t dbgc_host_R32_PA_DIR;
volatile uint32_t dbgc_host_R32_PA_PIN;
volatile uint32_t dbgc_host_R32_PA_CLR;
volatile uint32_t dbgc_host_R32_PA_PU;
volatile uint32_t dbgc_host_R32_PA_PD_DRV;
volatile uint32_t dbgc_host_R32_PA_SET;

static unsigned int checks;

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

static void reset_registers(void)
{
	dbgc_host_R32_PA_DIR = 0xA5A5A5A5U;
	dbgc_host_R32_PA_PIN = 0U;
	dbgc_host_R32_PA_CLR = 0U;
	dbgc_host_R32_PA_PU = 0x5A5A5A5AU;
	dbgc_host_R32_PA_PD_DRV = 0x96969696U;
	dbgc_host_R32_PA_SET = 0U;
}

static void check_mode(dbgc_ch585_gpio_mode_t mode, uint32_t expected_dir, uint32_t expected_pu,
		       uint32_t expected_pd)
{
	reset_registers();
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_configure(mode), 0);
	CHECK_VALUE(dbgc_host_R32_PA_DIR, expected_dir);
	CHECK_VALUE(dbgc_host_R32_PA_PU, expected_pu);
	CHECK_VALUE(dbgc_host_R32_PA_PD_DRV, expected_pd);
}

int main(void)
{
	uint8_t high = 0U;
	uint32_t saved_dir;
	uint32_t saved_pu;
	uint32_t saved_pd;

	check_mode(DBGC_CH585_GPIO_INPUT_FLOATING, 0xA5A5A5A5U, 0x5A5A5A4AU, 0x96969686U);
	check_mode(DBGC_CH585_GPIO_INPUT_PULL_UP, 0xA5A5A5A5U, 0x5A5A5A5AU, 0x96969686U);
	check_mode(DBGC_CH585_GPIO_INPUT_PULL_DOWN, 0xA5A5A5A5U, 0x5A5A5A4AU, 0x96969696U);
	check_mode(DBGC_CH585_GPIO_OUTPUT_PP_5MA, 0xA5A5A5B5U, 0x5A5A5A5AU, 0x96969686U);
	check_mode(DBGC_CH585_GPIO_OUTPUT_PP_20MA, 0xA5A5A5B5U, 0x5A5A5A5AU, 0x96969696U);

	reset_registers();
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_write(1U), 0);
	CHECK_VALUE(dbgc_host_R32_PA_SET, 0x00000010U);
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_write(0U), 0);
	CHECK_VALUE(dbgc_host_R32_PA_CLR, 0x00000010U);

	dbgc_host_R32_PA_PIN = 0x00000010U;
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_read(&high), 0);
	CHECK_VALUE(high, 1U);
	dbgc_host_R32_PA_PIN = 0U;
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_read(&high), 0);
	CHECK_VALUE(high, 0U);
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_read(NULL), -1);

	saved_dir = dbgc_host_R32_PA_DIR;
	saved_pu = dbgc_host_R32_PA_PU;
	saved_pd = dbgc_host_R32_PA_PD_DRV;
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_configure((dbgc_ch585_gpio_mode_t)5), -1);
	CHECK_VALUE(dbgc_host_R32_PA_DIR, saved_dir);
	CHECK_VALUE(dbgc_host_R32_PA_PU, saved_pu);
	CHECK_VALUE(dbgc_host_R32_PA_PD_DRV, saved_pd);

	printf("PASS: %u CH585 Target Reset GPIO register-model checks\n", checks);
	return EXIT_SUCCESS;
}
