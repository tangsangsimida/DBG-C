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

static uint8_t input_bits[128];
static uint8_t tdi_bits[64];
static uint8_t output_bits[128];
static uint8_t check_tdi_bits;
static unsigned int input_count;
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
	uint8_t output_level = (uint8_t)(bit & 1U);
	uint32_t expected_mask = (output_level != 0U) ? (1UL << 2) : 0U;
	uint32_t actual_mask;

	if (check_tdi_bits != 0U) {
		CHECK_VALUE(output_level, tdi_bits[output_index]);
	}
	dbgc_host_R32_PB_SET = 0U;
	dbgc_host_R32_PB_CLR = 0U;
	CHECK_VALUE(dbgc_ch585_jtag_gpio_write(DBGC_CH585_JTAG_SIGNAL_TDI, output_level), 0U);
	actual_mask = dbgc_host_R32_PB_SET;
	CHECK_VALUE(actual_mask, expected_mask);
	CHECK_VALUE(dbgc_host_R32_PB_CLR, output_level == 0U ? (1UL << 2) : 0U);
	output_bits[output_index] = output_level;
	++output_index;
}

/**
 * @brief 从脚本化输入设置 PB3 并通过 CH585 GPIO BSP 读取 TDO。
 * @return 当前脚本化 TDO 位。
 */
uint32_t dbgc_test_jtag_tdo_read(void)
{
	uint8_t bit;

	if (input_index >= input_count) {
		fprintf(stderr, "unexpected TDO read at bit %u\n", input_index);
		exit(EXIT_FAILURE);
	}
	bit = input_bits[input_index];

	if (bit != 0U) {
		dbgc_host_R32_PB_PIN |= 1UL << 3;
	} else {
		dbgc_host_R32_PB_PIN &= ~(1UL << 3);
	}
	CHECK_VALUE(dbgc_ch585_jtag_gpio_read(DBGC_CH585_JTAG_SIGNAL_TDO, &bit), 0U);
	CHECK_VALUE(bit, input_bits[input_index]);
	++input_index;
	return bit;
}

/**
 * @brief 将一个 32 位数按最低有效位优先装入脚本化 TDO 输入队列。
 * @param value 输入数据。
 */
static void set_tdo_word_lsb_first(uint32_t value)
{
	unsigned int bit;

	input_count = 32U;
	input_index = 0U;
	for (bit = 0U; bit < input_count; ++bit) {
		input_bits[bit] = (uint8_t)((value >> bit) & 1U);
	}
}

/**
 * @brief 将一个脚本化逻辑位追加到 TDO 输入队列。
 * @param bit 待追加的逻辑位。
 */
static void append_tdo_bit(uint32_t bit)
{
	if (input_count >= (sizeof(input_bits) / sizeof(input_bits[0]))) {
		fprintf(stderr, "TDO input model capacity exceeded\n");
		exit(EXIT_FAILURE);
	}
	input_bits[input_count++] = (uint8_t)(bit & 1U);
}

/**
 * @brief 按上游 JTAG 位采样顺序追加 DAP_TRANSFER_OK 确认位。
 */
static void append_tdo_ack_ok(void)
{
	append_tdo_bit(0U);
	append_tdo_bit(1U);
	append_tdo_bit(0U);
}

/**
 * @brief 将一个 32 位数按最低有效位优先追加到 TDO 输入队列。
 * @param value 输入数据。
 */
static void append_tdo_word_lsb_first(uint32_t value)
{
	unsigned int bit;

	for (bit = 0U; bit < 32U; ++bit) {
		append_tdo_bit(value >> bit);
	}
}

/**
 * @brief 通过上游 JTAG Transfer 命令验证 DP 写入与 AP posted-read/RDBUFF 路径。
 */
static void test_jtag_transfer_commands(void)
{
	static const uint8_t write_request[] = { ID_DAP_Transfer, 0U,	 1U,	0U,
						 0xEFU,		  0xCDU, 0xABU, 0x89U };
	static const uint8_t read_request[] = { ID_DAP_Transfer, 0U, 1U, DAP_TRANSFER_RnW };
	const uint32_t write_value = 0x89ABCDEFU;
	const uint32_t posted_value = 0x13579BDFU;
	const uint32_t read_value = 0x2468ACE0U;
	uint8_t response[12] = { 0U };
	uint32_t result;
	unsigned int bit;

	input_count = 0U;
	input_index = 0U;
	append_tdo_ack_ok();
	append_tdo_ack_ok();
	append_tdo_word_lsb_first(0U);
	output_index = 0U;
	check_tdi_bits = 0U;
	result = DAP_ExecuteCommand(write_request, response);
	CHECK_VALUE(result, 0x00080003U);
	CHECK_VALUE(response[0], ID_DAP_Transfer);
	CHECK_VALUE(response[1], 1U);
	CHECK_VALUE(response[2], DAP_TRANSFER_OK);
	CHECK_VALUE(input_index, 38U);
	CHECK_VALUE(output_index, 46U);
	for (bit = 0U; bit < 32U; ++bit) {
		CHECK_VALUE(output_bits[9U + bit], (write_value >> bit) & 1U);
	}

	input_count = 0U;
	input_index = 0U;
	append_tdo_ack_ok();
	append_tdo_word_lsb_first(posted_value);
	append_tdo_ack_ok();
	append_tdo_word_lsb_first(read_value);
	result = DAP_ExecuteCommand(read_request, response);
	CHECK_VALUE(result, 0x00040007U);
	CHECK_VALUE(response[0], ID_DAP_Transfer);
	CHECK_VALUE(response[1], 1U);
	CHECK_VALUE(response[2], DAP_TRANSFER_OK);
	CHECK_VALUE(response[3], (uint8_t)(read_value >> 0));
	CHECK_VALUE(response[4], (uint8_t)(read_value >> 8));
	CHECK_VALUE(response[5], (uint8_t)(read_value >> 16));
	CHECK_VALUE(response[6], (uint8_t)(read_value >> 24));
	CHECK_VALUE(input_index, 70U);
}

/**
 * @brief 通过 CMSIS-DAP 命令层读取脚本化 JTAG IDCODE。
 */
static void test_jtag_idcode_command(void)
{
	static const uint8_t connect_request[] = { ID_DAP_Connect, DAP_PORT_JTAG };
	static const uint8_t configure_request[] = { ID_DAP_JTAG_Configure, 1U, 4U };
	static const uint8_t idcode_request[] = { ID_DAP_JTAG_IDCODE, 0U };
	const uint32_t expected_idcode = 0x2BA01477U;
	uint8_t response[8] = { 0U };
	uint32_t result;

	input_count = 0U;
	input_index = 0U;
	output_index = 0U;
	check_tdi_bits = 0U;
	set_tdo_word_lsb_first(expected_idcode);
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
	result = DAP_ExecuteCommand(connect_request, response);
	CHECK_VALUE(result, 0x00020002U);
	CHECK_VALUE(response[0], ID_DAP_Connect);
	CHECK_VALUE(response[1], DAP_PORT_JTAG);

	result = DAP_ExecuteCommand(configure_request, response);
	CHECK_VALUE(result, 0x00030002U);
	CHECK_VALUE(response[0], ID_DAP_JTAG_Configure);
	CHECK_VALUE(response[1], DAP_OK);

	result = DAP_ExecuteCommand(idcode_request, response);
	CHECK_VALUE(result, 0x00020006U);
	CHECK_VALUE(response[0], ID_DAP_JTAG_IDCODE);
	CHECK_VALUE(response[1], DAP_OK);
	CHECK_VALUE(response[2], (uint8_t)(expected_idcode >> 0));
	CHECK_VALUE(response[3], (uint8_t)(expected_idcode >> 8));
	CHECK_VALUE(response[4], (uint8_t)(expected_idcode >> 16));
	CHECK_VALUE(response[5], (uint8_t)(expected_idcode >> 24));
	CHECK_VALUE(input_index, 32U);
	test_jtag_transfer_commands();
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

	input_count = 8U;
	for (input_index = 0U; input_index < input_count; ++input_index) {
		input_bits[input_index] = (uint8_t)((0x96U >> input_index) & 1U);
		tdi_bits[input_index] = (uint8_t)((0xA5U >> input_index) & 1U);
	}
	input_index = 0U;
	output_index = 0U;
	check_tdi_bits = 1U;
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
	CHECK_VALUE(response[2], 0x96U);
	CHECK_VALUE(output_index, 8U);
	CHECK_VALUE(input_index, 8U);
	CHECK_VALUE(tck_rising_count, 8U);
	CHECK_VALUE(tck_falling_count, 8U);
	CHECK_VALUE(dbgc_host_R32_PB_DIR & 0x0FU, 0x07U);
	test_jtag_idcode_command();
	printf("PASS: %u CMSIS-DAP JTAG host-model checks\n", checks);
	return EXIT_SUCCESS;
}
