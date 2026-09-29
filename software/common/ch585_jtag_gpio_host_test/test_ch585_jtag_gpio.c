#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbgc_ch585_jtag_gpio.h"
#include "dbgc_ch585_jtag_gpio_host_regs.h"

volatile uint32_t dbgc_host_R32_PB_DIR;
volatile uint32_t dbgc_host_R32_PB_PIN;
volatile uint32_t dbgc_host_R32_PB_CLR;
volatile uint32_t dbgc_host_R32_PB_PU;
volatile uint32_t dbgc_host_R32_PB_PD_DRV;
volatile uint32_t dbgc_host_R32_PB_SET;

static unsigned int checks;

/**
 * @brief 比较寄存器值并在不一致时终止检查。
 * @param actual 实际值。
 * @param expected 期望值。
 * @param expression 被检查表达式。
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
 * @brief 比较函数返回值并在不一致时终止检查。
 * @param actual 实际值。
 * @param expected 期望值。
 * @param expression 被检查表达式。
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
 * @brief 将模拟 GPIOB 寄存器初始化为可检查的基准值。
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
 * @brief 检查指定信号和模式只修改对应 PB 位。
 * @param signal JTAG 信号。
 * @param mask 对应的 GPIOB 位掩码。
 * @param mode GPIO 模式。
 * @param mode_index 模式数组索引。
 */
static void check_mode(dbgc_ch585_jtag_signal_t signal, uint32_t mask, dbgc_ch585_gpio_mode_t mode,
		       unsigned int mode_index)
{
	uint32_t dir = 0xA5A5A5A5U;
	uint32_t pullup = 0x5A5A5A5AU;
	uint32_t pulldown = 0x96969696U;

	if ((mode_index < 3U)) {
		dir &= ~mask;
	} else {
		dir |= mask;
	}
	if (mode == DBGC_CH585_GPIO_INPUT_FLOATING) {
		pullup &= ~mask;
	}
	if (mode == DBGC_CH585_GPIO_INPUT_FLOATING || mode == DBGC_CH585_GPIO_INPUT_PULL_UP ||
	    mode == DBGC_CH585_GPIO_OUTPUT_PP_5MA) {
		pulldown &= ~mask;
	}
	if (mode == DBGC_CH585_GPIO_INPUT_PULL_UP) {
		pullup |= mask;
	}
	if (mode == DBGC_CH585_GPIO_INPUT_PULL_DOWN || mode == DBGC_CH585_GPIO_OUTPUT_PP_20MA) {
		pulldown |= mask;
	}
	if (mode == DBGC_CH585_GPIO_INPUT_PULL_DOWN) {
		pullup &= ~mask;
	}

	reset_registers();
	CHECK_RESULT(dbgc_ch585_jtag_gpio_configure(signal, mode), 0);
	CHECK_VALUE(dbgc_host_R32_PB_DIR, dir);
	CHECK_VALUE(dbgc_host_R32_PB_PU, pullup);
	CHECK_VALUE(dbgc_host_R32_PB_PD_DRV, pulldown);
}

/**
 * @brief 验证 JTAG GPIO BSP 的 PB0 至 PB3 寄存器映射。
 * @return 检查全部通过时返回零。
 */
int main(void)
{
	static const dbgc_ch585_jtag_signal_t signals[] = { DBGC_CH585_JTAG_SIGNAL_TCK,
							    DBGC_CH585_JTAG_SIGNAL_TMS,
							    DBGC_CH585_JTAG_SIGNAL_TDI,
							    DBGC_CH585_JTAG_SIGNAL_TDO };
	static const uint32_t masks[] = { 1U << 0, 1U << 1, 1U << 2, 1U << 3 };
	static const dbgc_ch585_gpio_mode_t modes[] = { DBGC_CH585_GPIO_INPUT_FLOATING,
							DBGC_CH585_GPIO_INPUT_PULL_UP,
							DBGC_CH585_GPIO_INPUT_PULL_DOWN,
							DBGC_CH585_GPIO_OUTPUT_PP_5MA,
							DBGC_CH585_GPIO_OUTPUT_PP_20MA };
	uint8_t high;
	size_t signal_index;
	size_t mode_index;

	for (signal_index = 0U; signal_index < (sizeof(signals) / sizeof(signals[0]));
	     ++signal_index) {
		for (mode_index = 0U; mode_index < (sizeof(modes) / sizeof(modes[0]));
		     ++mode_index) {
			check_mode(signals[signal_index], masks[signal_index], modes[mode_index],
				   (unsigned int)mode_index);
		}

		reset_registers();
		CHECK_RESULT(dbgc_ch585_jtag_gpio_write(signals[signal_index], 1U), 0);
		CHECK_VALUE(dbgc_host_R32_PB_SET, masks[signal_index]);
		CHECK_RESULT(dbgc_ch585_jtag_gpio_write(signals[signal_index], 0U), 0);
		CHECK_VALUE(dbgc_host_R32_PB_CLR, masks[signal_index]);
		dbgc_host_R32_PB_PIN = masks[signal_index];
		CHECK_RESULT(dbgc_ch585_jtag_gpio_read(signals[signal_index], &high), 0);
		CHECK_VALUE(high, 1U);
		dbgc_host_R32_PB_PIN = 0U;
		CHECK_RESULT(dbgc_ch585_jtag_gpio_read(signals[signal_index], &high), 0);
		CHECK_VALUE(high, 0U);
	}

	CHECK_RESULT(dbgc_ch585_jtag_gpio_configure((dbgc_ch585_jtag_signal_t)4,
						    DBGC_CH585_GPIO_INPUT_FLOATING),
		     -1);
	CHECK_RESULT(dbgc_ch585_jtag_gpio_configure(DBGC_CH585_JTAG_SIGNAL_TCK,
						    (dbgc_ch585_gpio_mode_t)5),
		     -1);
	CHECK_RESULT(dbgc_ch585_jtag_gpio_write((dbgc_ch585_jtag_signal_t)4, 1U), -1);
	CHECK_RESULT(dbgc_ch585_jtag_gpio_read(DBGC_CH585_JTAG_SIGNAL_TDO, NULL), -1);
	printf("PASS: %u CH585 JTAG GPIO register-model checks\n", checks);
	return EXIT_SUCCESS;
}
