#include <stdint.h>
#include <stdio.h>

#include "DAP_config.h"
#include "DAP.h"
#include "dbgc_ch585_swd_gpio.h"
#include "dbgc_ch585_swd_gpio_host_regs.h"

static unsigned int checks;
static unsigned int failures;
static uint8_t input_bits[120];
static unsigned int input_bit_count;
static unsigned int input_bit_index;
static uint8_t output_bits[64];
static unsigned int output_bit_count;
static unsigned int clock_rising_count;
static unsigned int output_enable_count;
static unsigned int output_disable_count;

volatile uint32_t dbgc_host_R32_PB_DIR;
volatile uint32_t dbgc_host_R32_PB_PIN;
volatile uint32_t dbgc_host_R32_PB_CLR;
volatile uint32_t dbgc_host_R32_PB_PU;
volatile uint32_t dbgc_host_R32_PB_PD_DRV;
volatile uint32_t dbgc_host_R32_PB_SET;

#define CHECK(condition)                                                                     \
	do {                                                                                 \
		++checks;                                                                    \
		if (!(condition)) {                                                          \
			fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
			++failures;                                                          \
		}                                                                            \
	} while (0)

static void append_input_bit(uint32_t bit)
{
	if (input_bit_count < (sizeof(input_bits) / sizeof(input_bits[0]))) {
		input_bits[input_bit_count++] = (uint8_t)(bit & 1U);
	} else {
		++failures;
	}
}

static void append_input_word(uint32_t value)
{
	unsigned int bit;

	for (bit = 0U; bit < 32U; ++bit) {
		append_input_bit(value >> bit);
	}
}

static uint32_t word_parity(uint32_t value)
{
	uint32_t parity = 0U;

	while (value != 0U) {
		parity ^= value & 1U;
		value >>= 1;
	}

	return parity;
}

void dbgc_test_swclk_set(void)
{
	++clock_rising_count;
	CHECK(dbgc_ch585_swd_gpio_write(DBGC_CH585_SWD_SIGNAL_SWCLK, 1U) == 0);
	CHECK(dbgc_host_R32_PB_SET == (1UL << 6));
}

void dbgc_test_swclk_clear(void)
{
	CHECK(dbgc_ch585_swd_gpio_write(DBGC_CH585_SWD_SIGNAL_SWCLK, 0U) == 0);
	CHECK(dbgc_host_R32_PB_CLR == (1UL << 6));
}

void dbgc_test_swdio_output(uint32_t bit)
{
	const uint32_t mask = 1UL << 5;

	CHECK(dbgc_ch585_swd_gpio_write(DBGC_CH585_SWD_SIGNAL_SWDIO, (uint8_t)(bit & 1U)) == 0);
	CHECK(((bit & 1U) != 0U ? dbgc_host_R32_PB_SET : dbgc_host_R32_PB_CLR) == mask);
	if ((bit & 1U) != 0U) {
		dbgc_host_R32_PB_PIN |= mask;
	} else {
		dbgc_host_R32_PB_PIN &= ~mask;
	}
	if (output_bit_count < (sizeof(output_bits) / sizeof(output_bits[0]))) {
		output_bits[output_bit_count++] = (uint8_t)(bit & 1U);
	} else {
		++failures;
	}
}

uint32_t dbgc_test_swdio_input(void)
{
	uint8_t bit;

	if (input_bit_index >= input_bit_count) {
		++failures;
		return 0U;
	}

	bit = input_bits[input_bit_index++];
	if (bit != 0U) {
		dbgc_host_R32_PB_PIN |= 1UL << 5;
	} else {
		dbgc_host_R32_PB_PIN &= ~(1UL << 5);
	}
	CHECK(dbgc_ch585_swd_gpio_read(DBGC_CH585_SWD_SIGNAL_SWDIO, &bit) == 0);

	return bit;
}

void dbgc_test_swdio_output_enable(void)
{
	++output_enable_count;
	/* This mode is a host-test fixture, not a product electrical choice. */
	CHECK(dbgc_ch585_swd_gpio_configure(DBGC_CH585_SWD_SIGNAL_SWDIO,
					    DBGC_CH585_GPIO_OUTPUT_PP_5MA) == 0);
	CHECK((dbgc_host_R32_PB_DIR & (1UL << 5)) != 0U);
}

void dbgc_test_swdio_output_disable(void)
{
	++output_disable_count;
	/* This mode is a host-test fixture, not a product electrical choice. */
	CHECK(dbgc_ch585_swd_gpio_configure(DBGC_CH585_SWD_SIGNAL_SWDIO,
					    DBGC_CH585_GPIO_INPUT_FLOATING) == 0);
	CHECK((dbgc_host_R32_PB_DIR & (1UL << 5)) == 0U);
}

static void reset_line_model(void)
{
	dbgc_host_R32_PB_DIR = 0U;
	dbgc_host_R32_PB_PIN = 0U;
	dbgc_host_R32_PB_CLR = 0U;
	dbgc_host_R32_PB_PU = 0U;
	dbgc_host_R32_PB_PD_DRV = 0U;
	dbgc_host_R32_PB_SET = 0U;
	input_bit_count = 0U;
	input_bit_index = 0U;
	output_bit_count = 0U;
	clock_rising_count = 0U;
	output_enable_count = 0U;
	output_disable_count = 0U;
}

static void append_read_response(uint32_t ack, uint32_t value, uint32_t parity)
{
	unsigned int bit;

	for (bit = 0U; bit < 3U; ++bit) {
		append_input_bit(ack >> bit);
	}
	append_input_word(value);
	append_input_bit(parity);
}

static void append_ack(uint32_t ack)
{
	unsigned int bit;

	for (bit = 0U; bit < 3U; ++bit) {
		append_input_bit(ack >> bit);
	}
}

static void test_swd_read_request_and_data(void)
{
	static const uint8_t expected_request_bits[] = { 1U, 0U, 1U, 0U, 0U, 1U, 0U, 1U };
	const uint32_t expected_value = 0x01234567U;
	uint32_t value = 0U;
	uint8_t status;
	unsigned int bit;

	DAP_Setup();
	DAP_Data.fast_clock = 1U;
	reset_line_model();
	append_read_response(DAP_TRANSFER_OK, expected_value, word_parity(expected_value));

	status = SWD_Transfer(DAP_TRANSFER_RnW | DP_IDCODE, &value);

	CHECK(status == DAP_TRANSFER_OK);
	CHECK(value == expected_value);
	CHECK(input_bit_index == input_bit_count);
	CHECK(output_bit_count == 9U);
	for (bit = 0U; bit < sizeof(expected_request_bits); ++bit) {
		CHECK(output_bits[bit] == expected_request_bits[bit]);
	}
	CHECK(output_bits[8] == 1U);
	CHECK(clock_rising_count == 46U);
	CHECK(output_disable_count == 1U);
	CHECK(output_enable_count == 1U);
}

static void test_swd_read_parity_error(void)
{
	const uint32_t expected_value = 0x89ABCDEFU;
	uint32_t value = 0U;
	uint8_t status;

	DAP_Setup();
	DAP_Data.fast_clock = 1U;
	reset_line_model();
	append_read_response(DAP_TRANSFER_OK, expected_value, word_parity(expected_value) ^ 1U);

	status = SWD_Transfer(DAP_TRANSFER_RnW | DP_IDCODE, &value);

	CHECK(status == DAP_TRANSFER_ERROR);
	CHECK(value == expected_value);
	CHECK(input_bit_index == input_bit_count);
	CHECK(clock_rising_count == 46U);
	CHECK(output_disable_count == 1U);
	CHECK(output_enable_count == 1U);
}

static void test_swd_write_data_and_parity(void)
{
	static const uint8_t expected_request_bits[] = { 1U, 0U, 0U, 0U, 0U, 0U, 0U, 1U };
	const uint32_t expected_value = 0x89ABCDEFU;
	uint32_t value = expected_value;
	uint8_t status;
	unsigned int bit;

	DAP_Setup();
	DAP_Data.fast_clock = 1U;
	reset_line_model();
	append_ack(DAP_TRANSFER_OK);

	status = SWD_Transfer(DP_ABORT, &value);

	CHECK(status == DAP_TRANSFER_OK);
	CHECK(input_bit_index == input_bit_count);
	CHECK(output_bit_count == 42U);
	for (bit = 0U; bit < sizeof(expected_request_bits); ++bit) {
		CHECK(output_bits[bit] == expected_request_bits[bit]);
	}
	for (bit = 0U; bit < 32U; ++bit) {
		CHECK(output_bits[8U + bit] == ((expected_value >> bit) & 1U));
	}
	CHECK(output_bits[40] == word_parity(expected_value));
	CHECK(clock_rising_count == 46U);
	CHECK(output_disable_count == 1U);
	CHECK(output_enable_count == 1U);
}

static void test_swd_wait_and_fault_acknowledgements(void)
{
	const uint32_t acknowledgements[] = { DAP_TRANSFER_WAIT, DAP_TRANSFER_FAULT };
	unsigned int index;

	for (index = 0U; index < sizeof(acknowledgements) / sizeof(acknowledgements[0]); ++index) {
		uint32_t value = 0U;
		uint8_t status;

		DAP_Setup();
		DAP_Data.fast_clock = 1U;
		reset_line_model();
		append_ack(acknowledgements[index]);

		status = SWD_Transfer(DAP_TRANSFER_RnW | DP_IDCODE, &value);

		CHECK(status == acknowledgements[index]);
		CHECK(input_bit_index == input_bit_count);
		CHECK(output_bit_count == 9U);
		CHECK(output_bits[8] == 1U);
		CHECK(clock_rising_count == 13U);
		CHECK(output_disable_count == 1U);
		CHECK(output_enable_count == 1U);
	}
}

static void test_swd_sequence_output_and_input(void)
{
	static const uint8_t sequence_output[] = { 0xA5U, 0x03U };
	static const uint8_t expected_output_bits[] = { 1U, 0U, 1U, 0U, 0U, 1U, 0U, 1U, 1U, 1U };
	static const uint8_t sequence_input_bits[] = { 1U, 0U, 1U, 1U, 0U, 0U, 1U, 0U, 1U, 1U };
	uint8_t captured[2] = { 0U, 0U };
	unsigned int bit;

	DAP_Setup();
	DAP_Data.fast_clock = 1U;
	reset_line_model();

	SWD_Sequence(10U, sequence_output, NULL);

	CHECK(output_bit_count == sizeof(expected_output_bits));
	for (bit = 0U; bit < sizeof(expected_output_bits); ++bit) {
		CHECK(output_bits[bit] == expected_output_bits[bit]);
	}
	CHECK(clock_rising_count == 10U);

	reset_line_model();
	for (bit = 0U; bit < sizeof(sequence_input_bits); ++bit) {
		append_input_bit(sequence_input_bits[bit]);
	}

	SWD_Sequence(SWD_SEQUENCE_DIN | 10U, NULL, captured);

	CHECK(captured[0] == 0x4DU);
	CHECK(captured[1] == 0x03U);
	CHECK(input_bit_index == input_bit_count);
	CHECK(clock_rising_count == 10U);
}

static void test_dap_transfer_through_swd_engine(void)
{
	const uint32_t expected_value = 0x2BA01477U;
	const uint8_t connect_request[] = { ID_DAP_Connect, DAP_PORT_SWD };
	const uint8_t transfer_request[] = { ID_DAP_Transfer, 0U, 1U,
					     DAP_TRANSFER_RnW | DP_IDCODE };
	uint8_t response[DAP_PACKET_SIZE] = { 0U };
	uint32_t result;

	DAP_Setup();
	DAP_Data.fast_clock = 1U;
	result = DAP_ExecuteCommand(connect_request, response);
	CHECK((result & 0xFFFFU) == 2U);
	CHECK(response[0] == ID_DAP_Connect);
	CHECK(response[1] == DAP_PORT_SWD);

	reset_line_model();
	append_read_response(DAP_TRANSFER_OK, expected_value, word_parity(expected_value));
	result = DAP_ExecuteCommand(transfer_request, response);

	CHECK(result == ((4U << 16) | 7U));
	CHECK(response[0] == ID_DAP_Transfer);
	CHECK(response[1] == 1U);
	CHECK(response[2] == DAP_TRANSFER_OK);
	CHECK(response[3] == (uint8_t)expected_value);
	CHECK(response[4] == (uint8_t)(expected_value >> 8));
	CHECK(response[5] == (uint8_t)(expected_value >> 16));
	CHECK(response[6] == (uint8_t)(expected_value >> 24));
	CHECK(input_bit_index == input_bit_count);
}

static void test_dap_ap_read_through_swd_engine(void)
{
	const uint32_t posted_value = 0xA5A55A5AU;
	const uint32_t expected_value = 0x1234ABCDU;
	const uint8_t connect_request[] = { ID_DAP_Connect, DAP_PORT_SWD };
	const uint8_t transfer_request[] = { ID_DAP_Transfer, 0U, 1U,
					     DAP_TRANSFER_APnDP | DAP_TRANSFER_RnW };
	uint8_t response[DAP_PACKET_SIZE] = { 0U };
	uint32_t result;

	DAP_Setup();
	result = DAP_ExecuteCommand(connect_request, response);
	CHECK((result & 0xFFFFU) == 2U);
	CHECK(response[1] == DAP_PORT_SWD);

	reset_line_model();
	append_read_response(DAP_TRANSFER_OK, posted_value, word_parity(posted_value));
	append_read_response(DAP_TRANSFER_OK, expected_value, word_parity(expected_value));
	result = DAP_ExecuteCommand(transfer_request, response);

	CHECK(result == ((4U << 16) | 7U));
	CHECK(response[0] == ID_DAP_Transfer);
	CHECK(response[1] == 1U);
	CHECK(response[2] == DAP_TRANSFER_OK);
	CHECK(response[3] == (uint8_t)expected_value);
	CHECK(response[4] == (uint8_t)(expected_value >> 8));
	CHECK(response[5] == (uint8_t)(expected_value >> 16));
	CHECK(response[6] == (uint8_t)(expected_value >> 24));
	CHECK(input_bit_index == input_bit_count);
	CHECK(input_bit_count == 72U);
	CHECK(output_bit_count == 18U);
	CHECK(clock_rising_count == 92U);
	CHECK(output_disable_count == 2U);
	CHECK(output_enable_count == 2U);
}

static void test_dap_transfer_block_ap_reads_through_swd_engine(void)
{
	const uint32_t posted_value = 0xDEADBEEFU;
	const uint32_t first_value = 0x13579BDFU;
	const uint32_t second_value = 0x2468ACE0U;
	const uint8_t connect_request[] = { ID_DAP_Connect, DAP_PORT_SWD };
	const uint8_t transfer_request[] = { ID_DAP_TransferBlock, 0U, 2U, 0U,
					     DAP_TRANSFER_APnDP | DAP_TRANSFER_RnW };
	uint8_t response[DAP_PACKET_SIZE] = { 0U };
	uint32_t result;

	DAP_Setup();
	result = DAP_ExecuteCommand(connect_request, response);
	CHECK((result & 0xFFFFU) == 2U);
	CHECK(response[1] == DAP_PORT_SWD);

	reset_line_model();
	append_read_response(DAP_TRANSFER_OK, posted_value, word_parity(posted_value));
	append_read_response(DAP_TRANSFER_OK, first_value, word_parity(first_value));
	append_read_response(DAP_TRANSFER_OK, second_value, word_parity(second_value));
	result = DAP_ExecuteCommand(transfer_request, response);

	CHECK(result == ((5U << 16) | 12U));
	CHECK(response[0] == ID_DAP_TransferBlock);
	CHECK(response[1] == 2U);
	CHECK(response[2] == 0U);
	CHECK(response[3] == DAP_TRANSFER_OK);
	CHECK(response[4] == (uint8_t)first_value);
	CHECK(response[5] == (uint8_t)(first_value >> 8));
	CHECK(response[6] == (uint8_t)(first_value >> 16));
	CHECK(response[7] == (uint8_t)(first_value >> 24));
	CHECK(response[8] == (uint8_t)second_value);
	CHECK(response[9] == (uint8_t)(second_value >> 8));
	CHECK(response[10] == (uint8_t)(second_value >> 16));
	CHECK(response[11] == (uint8_t)(second_value >> 24));
	CHECK(input_bit_index == input_bit_count);
	CHECK(input_bit_count == 108U);
	CHECK(output_bit_count == 27U);
	CHECK(clock_rising_count == 138U);
	CHECK(output_disable_count == 3U);
	CHECK(output_enable_count == 3U);
}

static void test_dap_swd_sequence_command(void)
{
	static const uint8_t sequence_output_bits[] = { 1U, 0U, 1U, 0U, 0U, 1U, 0U, 1U, 1U, 1U };
	static const uint8_t sequence_input_bits[] = { 1U, 0U, 1U, 1U, 0U, 0U, 1U, 0U, 1U, 1U };
	const uint8_t request[] = { ID_DAP_SWD_Sequence,   2U, 10U, 0xA5U, 0x03U,
				    SWD_SEQUENCE_DIN | 10U };
	uint8_t response[DAP_PACKET_SIZE] = { 0U };
	uint32_t result;
	unsigned int bit;

	DAP_Setup();
	DAP_Data.fast_clock = 1U;
	reset_line_model();
	for (bit = 0U; bit < sizeof(sequence_input_bits); ++bit) {
		append_input_bit(sequence_input_bits[bit]);
	}

	result = DAP_ExecuteCommand(request, response);

	CHECK(result == ((6U << 16) | 4U));
	CHECK(response[0] == ID_DAP_SWD_Sequence);
	CHECK(response[1] == DAP_OK);
	CHECK(response[2] == 0x4DU);
	CHECK(response[3] == 0x03U);
	CHECK(output_bit_count == sizeof(sequence_output_bits));
	for (bit = 0U; bit < sizeof(sequence_output_bits); ++bit) {
		CHECK(output_bits[bit] == sequence_output_bits[bit]);
	}
	CHECK(input_bit_index == input_bit_count);
	CHECK(clock_rising_count == 20U);
	CHECK(output_disable_count == 1U);
	CHECK(output_enable_count == 2U);
}

int main(void)
{
	test_swd_read_request_and_data();
	test_swd_read_parity_error();
	test_swd_write_data_and_parity();
	test_swd_wait_and_fault_acknowledgements();
	test_swd_sequence_output_and_input();
	test_dap_transfer_through_swd_engine();
	test_dap_ap_read_through_swd_engine();
	test_dap_transfer_block_ap_reads_through_swd_engine();
	test_dap_swd_sequence_command();

	if (failures != 0U) {
		fprintf(stderr, "%u of %u CMSIS-DAP SWD engine checks failed\n", failures, checks);
		return 1;
	}

	printf("CMSIS-DAP SWD engine checks passed: %u assertions, 9 cases\n", checks);
	return 0;
}
