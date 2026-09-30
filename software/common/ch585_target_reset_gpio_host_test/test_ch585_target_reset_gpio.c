#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbgc_ch585_target_reset_gpio.h"
#include "dbgc_ch585_target_reset_gpio_host_regs.h"

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
 * @brief 检查 Target nRESET GPIO 模式对应的模拟寄存器更新。
 * @param mode 待检查 GPIO 模式。
 * @param expected_dir 期望方向寄存器值。
 * @param expected_pu 期望上拉寄存器值。
 * @param expected_pd 期望驱动寄存器值。
 */
static void check_mode(dbgc_ch585_gpio_mode_t mode, uint32_t expected_dir, uint32_t expected_pu,
		       uint32_t expected_pd)
{
	reset_registers();
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_configure(mode), 0);
	CHECK_VALUE(dbgc_host_R32_PB_DIR, expected_dir);
	CHECK_VALUE(dbgc_host_R32_PB_PU, expected_pu);
	CHECK_VALUE(dbgc_host_R32_PB_PD_DRV, expected_pd);
}

/**
 * @brief 检查 CMSIS-DAP nRESET 位值对应的释放或开漏模拟状态。
 * @param bit CMSIS-DAP nRESET 位值。
 * @param output_mode 断言时使用的推挽输出模式。
 * @param released_mode 释放时使用的输入模式。
 * @param expected_dir 期望方向寄存器值。
 * @param expected_pu 期望上拉寄存器值。
 * @param expected_pd 期望下拉/驱动寄存器值。
 * @param expected_clr 期望 GPIO 清零寄存器写入值。
 */
static void check_nreset(uint8_t bit, dbgc_ch585_gpio_mode_t output_mode,
			 dbgc_ch585_gpio_mode_t released_mode, uint32_t expected_dir,
			 uint32_t expected_pu, uint32_t expected_pd, uint32_t expected_clr)
{
	reset_registers();
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_set_nreset(bit, output_mode, released_mode), 0);
	CHECK_VALUE(dbgc_host_R32_PB_DIR, expected_dir);
	CHECK_VALUE(dbgc_host_R32_PB_PU, expected_pu);
	CHECK_VALUE(dbgc_host_R32_PB_PD_DRV, expected_pd);
	CHECK_VALUE(dbgc_host_R32_PB_CLR, expected_clr);
	CHECK_VALUE(dbgc_host_R32_PB_SET, 0U);
}

/**
 * @brief 验证 PB5 Target nRESET GPIO 与通用复位序列适配。
 * @return 所有检查通过时返回零，否则返回非零值。
 */
int main(void)
{
	uint8_t high = 0U;
	uint32_t saved_dir;
	uint32_t saved_pu;
	uint32_t saved_pd;

	check_mode(DBGC_CH585_GPIO_INPUT_FLOATING, 0xA5A5A585U, 0x5A5A5A5AU, 0x96969696U);
	check_mode(DBGC_CH585_GPIO_INPUT_PULL_UP, 0xA5A5A585U, 0x5A5A5A7AU, 0x96969696U);
	check_mode(DBGC_CH585_GPIO_INPUT_PULL_DOWN, 0xA5A5A585U, 0x5A5A5A5AU, 0x969696B6U);
	check_mode(DBGC_CH585_GPIO_OUTPUT_PP_5MA, 0xA5A5A5A5U, 0x5A5A5A5AU, 0x96969696U);
	check_mode(DBGC_CH585_GPIO_OUTPUT_PP_20MA, 0xA5A5A5A5U, 0x5A5A5A5AU, 0x969696B6U);

	reset_registers();
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_write(1U), 0);
	CHECK_VALUE(dbgc_host_R32_PB_SET, 0x00000020U);
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_write(0U), 0);
	CHECK_VALUE(dbgc_host_R32_PB_CLR, 0x00000020U);

	dbgc_host_R32_PB_PIN = 0x00000020U;
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_read(&high), 0);
	CHECK_VALUE(high, 1U);
	dbgc_host_R32_PB_PIN = 0U;
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_read(&high), 0);
	CHECK_VALUE(high, 0U);
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_read(NULL), -1);

	saved_dir = dbgc_host_R32_PB_DIR;
	saved_pu = dbgc_host_R32_PB_PU;
	saved_pd = dbgc_host_R32_PB_PD_DRV;
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_configure((dbgc_ch585_gpio_mode_t)5), -1);
	CHECK_VALUE(dbgc_host_R32_PB_DIR, saved_dir);
	CHECK_VALUE(dbgc_host_R32_PB_PU, saved_pu);
	CHECK_VALUE(dbgc_host_R32_PB_PD_DRV, saved_pd);

	check_nreset(0U, DBGC_CH585_GPIO_OUTPUT_PP_5MA, DBGC_CH585_GPIO_INPUT_FLOATING, 0xA5A5A5A5U,
		     0x5A5A5A5AU, 0x96969696U, 0x00000020U);
	check_nreset(0U, DBGC_CH585_GPIO_OUTPUT_PP_20MA, DBGC_CH585_GPIO_INPUT_FLOATING,
		     0xA5A5A5A5U, 0x5A5A5A5AU, 0x969696B6U, 0x00000020U);
	check_nreset(1U, DBGC_CH585_GPIO_OUTPUT_PP_5MA, DBGC_CH585_GPIO_INPUT_FLOATING, 0xA5A5A585U,
		     0x5A5A5A5AU, 0x96969696U, 0U);
	check_nreset(1U, DBGC_CH585_GPIO_OUTPUT_PP_5MA, DBGC_CH585_GPIO_INPUT_PULL_UP, 0xA5A5A585U,
		     0x5A5A5A7AU, 0x96969696U, 0U);
	reset_registers();
	saved_dir = dbgc_host_R32_PB_DIR;
	saved_pu = dbgc_host_R32_PB_PU;
	saved_pd = dbgc_host_R32_PB_PD_DRV;
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_set_nreset(2U, DBGC_CH585_GPIO_OUTPUT_PP_5MA,
							     DBGC_CH585_GPIO_INPUT_FLOATING),
		     -1);
	CHECK_VALUE(dbgc_host_R32_PB_DIR, saved_dir);
	CHECK_VALUE(dbgc_host_R32_PB_PU, saved_pu);
	CHECK_VALUE(dbgc_host_R32_PB_PD_DRV, saved_pd);
	CHECK_VALUE(dbgc_host_R32_PB_CLR, 0U);
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_set_nreset(0U, DBGC_CH585_GPIO_INPUT_FLOATING,
							     DBGC_CH585_GPIO_INPUT_FLOATING),
		     -1);
	CHECK_VALUE(dbgc_host_R32_PB_DIR, saved_dir);
	CHECK_VALUE(dbgc_host_R32_PB_CLR, 0U);
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_set_nreset(1U, DBGC_CH585_GPIO_OUTPUT_PP_5MA,
							     DBGC_CH585_GPIO_INPUT_PULL_DOWN),
		     -1);
	CHECK_VALUE(dbgc_host_R32_PB_PU, saved_pu);
	CHECK_VALUE(dbgc_host_R32_PB_PD_DRV, saved_pd);
	CHECK_VALUE(dbgc_host_R32_PB_CLR, 0U);

	printf("PASS: %u CH585 Target Reset GPIO register-model checks\n", checks);
	return EXIT_SUCCESS;
}
