#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbgc_ch585_target_reset_gpio.h"
#include "dbgc_ch585_target_reset_gpio_host_regs.h"
#include "dbgc_ch585_target_reset_sequence.h"

volatile uint32_t dbgc_host_R32_PB_DIR;
volatile uint32_t dbgc_host_R32_PB_PIN;
volatile uint32_t dbgc_host_R32_PB_CLR;
volatile uint32_t dbgc_host_R32_PB_PU;
volatile uint32_t dbgc_host_R32_PB_PD_DRV;
volatile uint32_t dbgc_host_R32_PB_SET;

static unsigned int checks;

typedef struct {
	uint8_t asserted_high;
	int hold_result;
	unsigned int hold_calls;
} reset_fixture_t;

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
 * @brief 验证复位保持回调在断言电平写入后执行。
 * @param context 测试夹具。
 * @return 夹具配置的回调结果。
 */
static int hold_reset(void *context)
{
	reset_fixture_t *fixture = context;

	++fixture->hold_calls;
	if (fixture->asserted_high != 0U) {
		CHECK_VALUE(dbgc_host_R32_PB_SET, 0x20U);
	} else {
		CHECK_VALUE(dbgc_host_R32_PB_CLR, 0x20U);
	}
	return fixture->hold_result;
}

/**
 * @brief 使用模拟 GPIOB 寄存器检查一个复位序列参数组合。
 * @param asserted_high 测试所用的原始断言电平。
 * @param hold_result 保持回调返回值。
 */
static void run_reset_case(uint8_t asserted_high, int hold_result)
{
	reset_fixture_t fixture = { asserted_high, hold_result, 0U };
	dbgc_ch585_target_reset_sequence_config_t configuration;
	int expected_result = hold_result;

	dbgc_host_R32_PB_DIR = 0U;
	dbgc_host_R32_PB_PIN = 0U;
	dbgc_host_R32_PB_CLR = 0U;
	dbgc_host_R32_PB_PU = 0U;
	dbgc_host_R32_PB_PD_DRV = 0U;
	dbgc_host_R32_PB_SET = 0U;

	/* This is a host-test fixture mode, not a product electrical decision. */
	CHECK_RESULT(dbgc_ch585_target_reset_gpio_configure(DBGC_CH585_GPIO_OUTPUT_PP_5MA), 0);
	CHECK_VALUE(dbgc_host_R32_PB_DIR & 0x20U, 0x20U);

	configuration.asserted_high = asserted_high;
	configuration.hold = hold_reset;
	configuration.context = &fixture;

	CHECK_RESULT(dbgc_ch585_target_reset_sequence_execute(&configuration), expected_result);
	CHECK_VALUE(fixture.hold_calls, 1U);
	CHECK_VALUE(dbgc_host_R32_PB_SET, 0x20U);
	CHECK_VALUE(dbgc_host_R32_PB_CLR, 0x20U);
}

/**
 * @brief 验证 PB5 Target nRESET GPIO 与通用复位序列适配。
 * @return 所有检查通过时返回零，否则返回非零值。
 */
int main(void)
{
	dbgc_ch585_target_reset_sequence_config_t invalid = { 0U, 0, 0 };

	dbgc_host_R32_PB_DIR = 0xA1U;
	dbgc_host_R32_PB_PIN = 0xB2U;
	dbgc_host_R32_PB_CLR = 0xC3U;
	dbgc_host_R32_PB_PU = 0xD4U;
	dbgc_host_R32_PB_PD_DRV = 0xE5U;
	dbgc_host_R32_PB_SET = 0xF6U;
	CHECK_RESULT(dbgc_ch585_target_reset_sequence_execute(0), -1);
	CHECK_RESULT(dbgc_ch585_target_reset_sequence_execute(&invalid), -1);
	invalid.asserted_high = 2U;
	invalid.hold = hold_reset;
	CHECK_RESULT(dbgc_ch585_target_reset_sequence_execute(&invalid), -1);
	CHECK_VALUE(dbgc_host_R32_PB_DIR, 0xA1U);
	CHECK_VALUE(dbgc_host_R32_PB_PIN, 0xB2U);
	CHECK_VALUE(dbgc_host_R32_PB_CLR, 0xC3U);
	CHECK_VALUE(dbgc_host_R32_PB_PU, 0xD4U);
	CHECK_VALUE(dbgc_host_R32_PB_PD_DRV, 0xE5U);
	CHECK_VALUE(dbgc_host_R32_PB_SET, 0xF6U);

	run_reset_case(1U, 0);
	run_reset_case(0U, 0);
	run_reset_case(1U, -7);
	run_reset_case(0U, -9);

	printf("PASS: %u CH585 Target Reset sequence integration checks\n", checks);
	return EXIT_SUCCESS;
}
