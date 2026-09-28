#include <stdint.h>
#include <stdio.h>

#include "DAP_config.h"
#include "DAP.h"

static unsigned int checks;
static unsigned int failures;
static uint8_t input_bits[40];
static unsigned int input_bit_count;
static unsigned int input_bit_index;
static uint8_t output_bits[16];
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

int main(void)
{
    test_swd_read_request_and_data();
    test_swd_read_parity_error();

    if (failures != 0U) {
        fprintf(stderr, "%u of %u CMSIS-DAP SWD engine checks failed\n",
                failures, checks);
        return 1;
    }

    printf("CMSIS-DAP SWD engine checks passed: %u assertions, 2 cases\n",
           checks);
    return 0;
}
