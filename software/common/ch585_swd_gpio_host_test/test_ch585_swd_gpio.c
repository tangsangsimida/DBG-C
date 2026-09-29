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

/**
 * @brief 比较寄存器模型值并在不匹配时终止检查。
 * @param actual 实际值。
 * @param expected 期望值。
 * @param expression 被检查表达式文本。
 * @param line 调用位置行号。
 */
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

/**
 * @brief 比较函数返回值并在不匹配时终止检查。
 * @param actual 实际值。
 * @param expected 期望值。
 * @param expression 被检查表达式文本。
 * @param line 调用位置行号。
 */
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

/**
 * @brief 将 GPIOB 模拟寄存器重置为测试基准值。
 */
static void reset_registers(void)
{
	dbgc_host_R32_PB_DIR = 0xA5A5A5A5U;
	dbgc_host_R32_PB_PIN = 0U;
	dbgc_host_R32_PB_CLR = 0U;
	dbgc_host_R32_PB_PU = 0x5A5A5A5AU;
	dbgc_host_R32_PB_PD_DRV = 0x96969696U;
	dbgc_host_R32_PB_SET = 0U;
}

/**
 * @brief 检查指定 SWD GPIO 模式对应的模拟寄存器更新。
 * @param signal SWD 信号。
 * @param mask 依据 MCU-001 V0.11 的端口 B 位掩码。
 * @param mode 待检查 GPIO 模式。
 * @param clear_pd 是否清除驱动寄存器位。
 * @param set_pd 是否设置驱动寄存器位。
 * @param clear_pu 是否清除上拉寄存器位。
 * @param set_pu 是否设置上拉寄存器位。
 * @param clear_dir 是否配置输入方向。
 * @param set_dir 是否配置输出方向。
 */
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

/**
 * @brief 验证 Target SWD GPIO BSP 的 PB0/PB1 模拟寄存器映射。
 * @return 所有检查通过时返回零，否则返回非零值。
 */
int main(void)
{
	uint8_t high = 0U;
	uint32_t saved_dir;
	uint32_t saved_pu;
	uint32_t saved_pd;

	check_mode(DBGC_CH585_SWD_SIGNAL_SWDIO, 0x00000002U, DBGC_CH585_GPIO_INPUT_FLOATING, 1, 0,
		   1, 0, 1, 0);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWCLK, 0x00000001U, DBGC_CH585_GPIO_INPUT_FLOATING, 1, 0,
		   1, 0, 1, 0);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWDIO, 0x00000002U, DBGC_CH585_GPIO_INPUT_PULL_UP, 1, 0, 0,
		   1, 1, 0);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWCLK, 0x00000001U, DBGC_CH585_GPIO_INPUT_PULL_UP, 1, 0, 0,
		   1, 1, 0);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWDIO, 0x00000002U, DBGC_CH585_GPIO_INPUT_PULL_DOWN, 0, 1,
		   1, 0, 1, 0);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWCLK, 0x00000001U, DBGC_CH585_GPIO_INPUT_PULL_DOWN, 0, 1,
		   1, 0, 1, 0);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWDIO, 0x00000002U, DBGC_CH585_GPIO_OUTPUT_PP_5MA, 1, 0, 0,
		   0, 0, 1);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWCLK, 0x00000001U, DBGC_CH585_GPIO_OUTPUT_PP_5MA, 1, 0, 0,
		   0, 0, 1);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWDIO, 0x00000002U, DBGC_CH585_GPIO_OUTPUT_PP_20MA, 0, 1,
		   0, 0, 0, 1);
	check_mode(DBGC_CH585_SWD_SIGNAL_SWCLK, 0x00000001U, DBGC_CH585_GPIO_OUTPUT_PP_20MA, 0, 1,
		   0, 0, 0, 1);

	reset_registers();
	CHECK_RESULT(dbgc_ch585_swd_gpio_write(DBGC_CH585_SWD_SIGNAL_SWDIO, 1U), 0);
	CHECK_VALUE(dbgc_host_R32_PB_SET, 0x00000002U);
	CHECK_RESULT(dbgc_ch585_swd_gpio_write(DBGC_CH585_SWD_SIGNAL_SWCLK, 0U), 0);
	CHECK_VALUE(dbgc_host_R32_PB_CLR, 0x00000001U);

	dbgc_host_R32_PB_PIN = 0x00000003U;
	CHECK_RESULT(dbgc_ch585_swd_gpio_read(DBGC_CH585_SWD_SIGNAL_SWDIO, &high), 0);
	CHECK_VALUE(high, 1U);
	CHECK_RESULT(dbgc_ch585_swd_gpio_read(DBGC_CH585_SWD_SIGNAL_SWCLK, &high), 0);
	CHECK_VALUE(high, 1U);
	dbgc_host_R32_PB_PIN = 0x00000002U;
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
