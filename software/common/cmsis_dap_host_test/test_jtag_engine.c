#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "DAP_config.h"
#include "DAP.h"
#include "dbgc_ch585_jtag_gpio.h"
#include "dbgc_ch585_jtag_gpio_host_regs.h"

volatile uint32_t dbgc_host_R32_PB_DIR;
volatile uint32_t dbgc_host_R32_PB_PIN;
volatile uint32_t dbgc_host_R32_PB_CLR;
volatile uint32_t dbgc_host_R32_PB_PU;
volatile uint32_t dbgc_host_R32_PB_PD_DRV;
volatile uint32_t dbgc_host_R32_PB_SET;

static uint8_t input_data;
static uint8_t tdi_data;
static uint8_t output_data;
static unsigned int input_index;
static unsigned int output_index;
static unsigned int tck_rising_count;
static unsigned int tck_falling_count;
static unsigned int checks;

/**
 * @brief 比较实际值和预期值，并在不一致时结束测试。
 * @param actual 实际值。
 * @param expected 预期值。
 * @param expression 被检查表达式。
 * @param line 检查所在行。
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

#define CHECK_VALUE(actual, expected) check_value((actual), (expected), #actual, __LINE__)

/**
 * @brief 将 JTAG TCK 上升沿写入 CH585 GPIO BSP 并检查 PB0 映射。
 */
void dbgc_test_jtag_tck_set(void)
{
	++tck_rising_count;
	CHECK_VALUE(dbgc_ch585_jtag_gpio_write(DBGC_CH585_JTAG_SIGNAL_TCK, 1U), 0U);
	CHECK_VALUE(dbgc_host_R32_PB_SET, 1UL << 0);
}

/**
 * @brief 将 JTAG TCK 下降沿写入 CH585 GPIO BSP 并检查 PB0 映射。
 */
void dbgc_test_jtag_tck_clear(void)
{
	++tck_falling_count;
	CHECK_VALUE(dbgc_ch585_jtag_gpio_write(DBGC_CH585_JTAG_SIGNAL_TCK, 0U), 0U);
	CHECK_VALUE(dbgc_host_R32_PB_CLR, 1UL << 0);
}

/**
 * @brief 写入 JTAG TMS 并检查 PB1 映射。
 * @param bit 待写入的原始逻辑位。
 */
void dbgc_test_jtag_tms_write(uint32_t bit)
{
	CHECK_VALUE(dbgc_ch585_jtag_gpio_write(DBGC_CH585_JTAG_SIGNAL_TMS, (uint8_t)bit), 0U);
	CHECK_VALUE(bit != 0U ? dbgc_host_R32_PB_SET : dbgc_host_R32_PB_CLR, 1UL << 1);
}

/**
 * @brief 写入 JTAG TDI 并检查 PB2 映射及逐位数据。
 * @param bit 待写入的原始逻辑位。
 */
void dbgc_test_jtag_tdi_write(uint32_t bit)
{
	uint32_t expected = (uint32_t)((tdi_data >> output_index) & 1U);

	CHECK_VALUE(bit & 1U, expected);
	CHECK_VALUE(dbgc_ch585_jtag_gpio_write(DBGC_CH585_JTAG_SIGNAL_TDI, (uint8_t)bit), 0U);
	CHECK_VALUE(bit != 0U ? dbgc_host_R32_PB_SET : dbgc_host_R32_PB_CLR, 1UL << 2);
	output_data |= (uint8_t)((bit & 1U) << output_index);
	++output_index;
}

/**
 * @brief 从脚本化输入设置 PB3 并通过 CH585 GPIO BSP 读取 TDO。
 * @return 当前脚本化 TDO 位。
 */
uint32_t dbgc_test_jtag_tdo_read(void)
{
	uint8_t bit = (uint8_t)((input_data >> input_index) & 1U);

	if (bit != 0U) {
		dbgc_host_R32_PB_PIN |= 1UL << 3;
	} else {
		dbgc_host_R32_PB_PIN &= ~(1UL << 3);
	}
	CHECK_VALUE(dbgc_ch585_jtag_gpio_read(DBGC_CH585_JTAG_SIGNAL_TDO, &bit), 0U);
	CHECK_VALUE(bit, (input_data >> input_index) & 1U);
	++input_index;
	return bit;
}

/**
 * @brief 验证上游 CMSIS-DAP JTAG Sequence 经 CH585 GPIO BSP 的主机模型路径。
 * @return 所有检查通过时返回 EXIT_SUCCESS。
 */
int main(void)
{
	const uint8_t request[] = { ID_DAP_JTAG_Sequence, 1U,
				    JTAG_SEQUENCE_TDO | JTAG_SEQUENCE_TMS | 8U, 0xA5U };
	uint8_t response[4] = { 0U };
	uint32_t result;

	input_data = 0x96U;
	tdi_data = 0xA5U;
	CHECK_VALUE(dbgc_ch585_jtag_gpio_configure(DBGC_CH585_JTAG_SIGNAL_TCK,
						   DBGC_CH585_GPIO_OUTPUT_PP_5MA),
		    0U);
	CHECK_VALUE(dbgc_ch585_jtag_gpio_configure(DBGC_CH585_JTAG_SIGNAL_TMS,
						   DBGC_CH585_GPIO_OUTPUT_PP_5MA),
		    0U);
	CHECK_VALUE(dbgc_ch585_jtag_gpio_configure(DBGC_CH585_JTAG_SIGNAL_TDI,
						   DBGC_CH585_GPIO_OUTPUT_PP_5MA),
		    0U);
	CHECK_VALUE(dbgc_ch585_jtag_gpio_configure(DBGC_CH585_JTAG_SIGNAL_TDO,
						   DBGC_CH585_GPIO_INPUT_FLOATING),
		    0U);

	DAP_Setup();
	result = DAP_ExecuteCommand(request, response);
	CHECK_VALUE(result, 0x00040003U);
	CHECK_VALUE(response[0], ID_DAP_JTAG_Sequence);
	CHECK_VALUE(response[1], DAP_OK);
	CHECK_VALUE(response[2], input_data);
	CHECK_VALUE(output_data, 0xA5U);
	CHECK_VALUE(input_index, 8U);
	CHECK_VALUE(output_index, 8U);
	CHECK_VALUE(tck_rising_count, 8U);
	CHECK_VALUE(tck_falling_count, 8U);
	CHECK_VALUE(dbgc_host_R32_PB_DIR & 0x0FU, 0x07U);
	printf("PASS: %u CMSIS-DAP JTAG GPIO host-model checks\n", checks);
	return EXIT_SUCCESS;
}
