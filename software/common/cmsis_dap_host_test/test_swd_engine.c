#include <stdint.h>
#include <stdio.h>

#include "DAP_config.h"
#include "DAP.h"

static unsigned int checks;
static unsigned int failures;
static uint8_t input_bits[40];
static unsigned int input_bit_count;
static unsigned int input_bit_index;
static uint8_t output_bits[64];
static unsigned int output_bit_count;
static unsigned int clock_rising_count;
static unsigned int output_enable_count;
static unsigned int output_disable_count;

#define CHECK(condition) do { \
    ++checks; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", \
                __FILE__, __LINE__, #condition); \
        ++failures; \
    } \
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
}

void dbgc_test_swclk_clear(void)
{
}

void dbgc_test_swdio_output(uint32_t bit)
{
    if (output_bit_count < (sizeof(output_bits) / sizeof(output_bits[0]))) {
        output_bits[output_bit_count++] = (uint8_t)(bit & 1U);
    } else {
        ++failures;
    }
}

uint32_t dbgc_test_swdio_input(void)
{
    if (input_bit_index >= input_bit_count) {
        ++failures;
        return 0U;
    }

    return input_bits[input_bit_index++];
}

void dbgc_test_swdio_output_enable(void)
{
    ++output_enable_count;
}

void dbgc_test_swdio_output_disable(void)
{
    ++output_disable_count;
}

static void reset_line_model(void)
{
    input_bit_count = 0U;
    input_bit_index = 0U;
    output_bit_count = 0U;
    clock_rising_count = 0U;
    output_enable_count = 0U;
    output_disable_count = 0U;
}

static void append_read_response(uint32_t ack,
                                 uint32_t value,
                                 uint32_t parity)
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
    static const uint8_t expected_request_bits[] = {
        1U, 0U, 1U, 0U, 0U, 1U, 0U, 1U
    };
    const uint32_t expected_value = 0x01234567U;
    uint32_t value = 0U;
    uint8_t status;
    unsigned int bit;

    DAP_Setup();
    DAP_Data.fast_clock = 1U;
    reset_line_model();
    append_read_response(DAP_TRANSFER_OK,
                         expected_value,
                         word_parity(expected_value));

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
    append_read_response(DAP_TRANSFER_OK,
                         expected_value,
                         word_parity(expected_value) ^ 1U);

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
    static const uint8_t expected_request_bits[] = {
        1U, 0U, 0U, 0U, 0U, 0U, 0U, 1U
    };
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
    const uint32_t acknowledgements[] = {
        DAP_TRANSFER_WAIT,
        DAP_TRANSFER_FAULT
    };
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
    static const uint8_t sequence_output[] = {0xA5U, 0x03U};
    static const uint8_t expected_output_bits[] = {
        1U, 0U, 1U, 0U, 0U, 1U, 0U, 1U, 1U, 1U
    };
    static const uint8_t sequence_input_bits[] = {
        1U, 0U, 1U, 1U, 0U, 0U, 1U, 0U, 1U, 1U
    };
    uint8_t captured[2] = {0U, 0U};
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
    const uint8_t connect_request[] = {
        ID_DAP_Connect,
        DAP_PORT_SWD
    };
    const uint8_t transfer_request[] = {
        ID_DAP_Transfer,
        0U,
        1U,
        DAP_TRANSFER_RnW | DP_IDCODE
    };
    uint8_t response[DAP_PACKET_SIZE] = {0U};
    uint32_t result;

    DAP_Setup();
    DAP_Data.fast_clock = 1U;
    result = DAP_ExecuteCommand(connect_request, response);
    CHECK((result & 0xFFFFU) == 2U);
    CHECK(response[0] == ID_DAP_Connect);
    CHECK(response[1] == DAP_PORT_SWD);

    reset_line_model();
    append_read_response(DAP_TRANSFER_OK,
                         expected_value,
                         word_parity(expected_value));
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

int main(void)
{
    test_swd_read_request_and_data();
    test_swd_read_parity_error();
    test_swd_write_data_and_parity();
    test_swd_wait_and_fault_acknowledgements();
    test_swd_sequence_output_and_input();
    test_dap_transfer_through_swd_engine();

    if (failures != 0U) {
        fprintf(stderr, "%u of %u CMSIS-DAP SWD engine checks failed\n",
                failures, checks);
        return 1;
    }

    printf("CMSIS-DAP SWD engine checks passed: %u assertions, 6 cases\n",
           checks);
    return 0;
}
