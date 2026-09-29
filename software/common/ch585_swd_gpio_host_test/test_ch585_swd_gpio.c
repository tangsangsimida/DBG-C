#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbgc_ch585_swd_gpio.h"
#include "dbgc_ch585_swd_gpio_host_regs.h"

volatile uint32_t dbgc_host_R32_PB_DIR;
volatile uint32_t dbgc_host_R32_PB_PIN;
volatile uint32_t dbgc_host_R32_PB_CLR;
volatile uint32_t dbgc_host_R32_PB_PU;
volatile uint32_t dbgc_host_R32_PB_PD_DRV;
volatile uint32_t dbgc_host_R32_PB_SET;

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
	dbgc_host_R32_PB_DIR = 0xA5A5A5A5U;
	dbgc_host_R32_PB_PIN = 0U;
	dbgc_host_R32_PB_CLR = 0U;
	dbgc_host_R32_PB_PU = 0x5A5A5A5AU;
	dbgc_host_R32_PB_PD_DRV = 0x96969696U;
	dbgc_host_R32_PB_SET = 0U;
}

static void check_mode(dbgc_ch585_swd_signal_t signal, uint32_t mask, dbgc_ch585_gpio_mode_t mode,
		       int clear_pd, int set_pd, int clear_pu, int set_pu, int clear_dir,
		       int set_dir)
{
	uint32_t expected_dir = 0xA5A5A5A5U;
	uint32_t expected_pu = 0x5A5A5A5AU;
	uint32_t expected_pd = 0x96969696U;

	if (clear_pd != 0) {
		expected_pd &= ~mask;
	}
	if (set_pd != 0) {
		expected_pd |= mask;
	}
	if (clear_pu != 0) {
		expected_pu &= ~mask;
	}
	if (set_pu != 0) {
		expected_pu |= mask;
	}
	if (clear_dir != 0) {
		expected_dir &= ~mask;
	}
	if (set_dir != 0) {
		expected_dir |= mask;
	}

	reset_registers();
	CHECK_RESULT(dbgc_ch585_swd_gpio_configure(signal, mode), 0);
	CHECK_VALUE(dbgc_host_R32_PB_DIR, expected_dir);
	CHECK_VALUE(dbgc_host_R32_PB_PU, expected_pu);
	CHECK_VALUE(dbgc_host_R32_PB_PD_DRV, expected_pd);
}

int main(void)
{
	uint8_t high = 0U;
	uint32_t saved_dir;
	uint32_t saved_pu;
	uint32_t saved_pd;

	check_mode(DBGC_CH585_SWD_SIGNAL_SWDIO, 0x00000020U, DBGC_CH585_GPIO_INPUT_FLOATING, 1, 0,
		   1, 0, 1, 0);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWCLK, 0x00000040U, DBGC_CH585_GPIO_INPUT_FLOATING, 1, 0,
		   1, 0, 1, 0);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWDIO, 0x00000020U, DBGC_CH585_GPIO_INPUT_PULL_UP, 1, 0, 0,
		   1, 1, 0);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWCLK, 0x00000040U, DBGC_CH585_GPIO_INPUT_PULL_UP, 1, 0, 0,
		   1, 1, 0);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWDIO, 0x00000020U, DBGC_CH585_GPIO_INPUT_PULL_DOWN, 0, 1,
		   1, 0, 1, 0);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWCLK, 0x00000040U, DBGC_CH585_GPIO_INPUT_PULL_DOWN, 0, 1,
		   1, 0, 1, 0);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWDIO, 0x00000020U, DBGC_CH585_GPIO_OUTPUT_PP_5MA, 1, 0, 0,
		   0, 0, 1);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWCLK, 0x00000040U, DBGC_CH585_GPIO_OUTPUT_PP_5MA, 1, 0, 0,
		   0, 0, 1);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWDIO, 0x00000020U, DBGC_CH585_GPIO_OUTPUT_PP_20MA, 0, 1,
		   0, 0, 0, 1);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWCLK, 0x00000040U, DBGC_CH585_GPIO_OUTPUT_PP_20MA, 0, 1,
		   0, 0, 0, 1);

	reset_registers();
	CHECK_RESULT(dbgc_ch585_swd_gpio_write(DBGC_CH585_SWD_SIGNAL_SWDIO, 1U), 0);
	CHECK_VALUE(dbgc_host_R32_PB_SET, 0x00000020U);
	CHECK_RESULT(dbgc_ch585_swd_gpio_write(DBGC_CH585_SWD_SIGNAL_SWCLK, 0U), 0);
	CHECK_VALUE(dbgc_host_R32_PB_CLR, 0x00000040U);

	dbgc_host_R32_PB_PIN = 0x00000060U;
	CHECK_RESULT(dbgc_ch585_swd_gpio_read(DBGC_CH585_SWD_SIGNAL_SWDIO, &high), 0);
	CHECK_VALUE(high, 1U);
	CHECK_RESULT(dbgc_ch585_swd_gpio_read(DBGC_CH585_SWD_SIGNAL_SWCLK, &high), 0);
	CHECK_VALUE(high, 1U);
	dbgc_host_R32_PB_PIN = 0x00000020U;
	CHECK_RESULT(dbgc_ch585_swd_gpio_read(DBGC_CH585_SWD_SIGNAL_SWCLK, &high), 0);
	CHECK_VALUE(high, 0U);

	saved_dir = dbgc_host_R32_PB_DIR;
	saved_pu = dbgc_host_R32_PB_PU;
	saved_pd = dbgc_host_R32_PB_PD_DRV;
	CHECK_RESULT(dbgc_ch585_swd_gpio_configure((dbgc_ch585_swd_signal_t)2,
						   DBGC_CH585_GPIO_INPUT_FLOATING),
		     -1);
	CHECK_RESULT(dbgc_ch585_swd_gpio_configure(DBGC_CH585_SWD_SIGNAL_SWDIO,
						   (dbgc_ch585_gpio_mode_t)5),
		     -1);
	CHECK_RESULT(dbgc_ch585_swd_gpio_write((dbgc_ch585_swd_signal_t)2, 1U), -1);
	CHECK_RESULT(dbgc_ch585_swd_gpio_read(DBGC_CH585_SWD_SIGNAL_SWCLK, NULL), -1);
	CHECK_VALUE(dbgc_host_R32_PB_DIR, saved_dir);
	CHECK_VALUE(dbgc_host_R32_PB_PU, saved_pu);
	CHECK_VALUE(dbgc_host_R32_PB_PD_DRV, saved_pd);

	printf("PASS: %u CH585 SWD GPIO register-model checks\n", checks);
	return EXIT_SUCCESS;
}
