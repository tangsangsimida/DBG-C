#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbgc_byte_duplex_bridge.h"

typedef struct {
    const uint8_t *input;
    size_t input_length;
    size_t input_offset;
    uint8_t output[16];
    size_t output_length;
    int read_result;
    int write_result;
} endpoint_t;

static unsigned int checks;

static void check(int condition, const char *name)
{
    ++checks;
    if (!condition) {
        (void)fprintf(stderr, "FAIL: %s\n", name);
        exit(EXIT_FAILURE);
    }
}

static int endpoint_read(void *context, uint8_t *byte)
{
    endpoint_t *endpoint = (endpoint_t *)context;

    if (endpoint->read_result != 1) {
        return endpoint->read_result;
    }
    if (endpoint->input_offset >= endpoint->input_length) {
        return 0;
    }

    *byte = endpoint->input[endpoint->input_offset];
    ++endpoint->input_offset;
    return 1;
}

static int endpoint_write(void *context, uint8_t byte)
{
    endpoint_t *endpoint = (endpoint_t *)context;

    if (endpoint->write_result != 1) {
        return endpoint->write_result;
    }
    if (endpoint->output_length >= sizeof(endpoint->output)) {
        return -1;
    }

    endpoint->output[endpoint->output_length] = byte;
    ++endpoint->output_length;
    return 1;
}

static void initialize_endpoint(endpoint_t *endpoint,
                                const uint8_t *input,
                                size_t input_length)
{
    (void)memset(endpoint, 0, sizeof(*endpoint));
    endpoint->input = input;
    endpoint->input_length = input_length;
    endpoint->read_result = 1;
    endpoint->write_result = 1;
}

int main(void)
{
    static const uint8_t first_payload[] = { 0x10U, 0x20U, 0x30U, 0x40U };
    static const uint8_t second_payload[] = { 0xA0U, 0xB0U, 0xC0U };
    uint8_t first_storage[4];
    uint8_t second_storage[4];
    dbgc_byte_fifo_t first_fifo;
    dbgc_byte_fifo_t second_fifo;
    dbgc_byte_duplex_bridge_t bridge;
    endpoint_t first;
    endpoint_t second;
    size_t first_transferred;
    size_t second_transferred;

    check(dbgc_byte_fifo_initialize(&first_fifo, first_storage,
                                    sizeof(first_storage)) == 0,
          "first FIFO initialization succeeds");
    check(dbgc_byte_fifo_initialize(&second_fifo, second_storage,
                                    sizeof(second_storage)) == 0,
          "second FIFO initialization succeeds");
    initialize_endpoint(&first, first_payload, sizeof(first_payload));
    initialize_endpoint(&second, second_payload, sizeof(second_payload));
    check(dbgc_byte_duplex_bridge_initialize(
              &bridge, &first_fifo, endpoint_read, &first,
              endpoint_write, &second, &second_fifo, endpoint_read, &second,
              endpoint_write, &first) == DBGC_BYTE_STREAM_BRIDGE_OK,
          "both directions initialize with caller-owned endpoints");

    check((dbgc_byte_duplex_bridge_service(
               &bridge, 2U, 1U, &first_transferred, &second_transferred) ==
           DBGC_BYTE_STREAM_BRIDGE_OK) &&
              (first_transferred == 2U) && (second_transferred == 1U),
          "each direction respects its independent service budget");
    check((second.output_length == 2U) &&
              (memcmp(second.output, first_payload, 2U) == 0) &&
              (first.output_length == 1U) &&
              (first.output[0] == second_payload[0]),
          "both directions forward the correct endpoint bytes");

    second.write_result = 0;
    check((dbgc_byte_duplex_bridge_service(
           &bridge, 2U, 2U, &first_transferred, &second_transferred) ==
           DBGC_BYTE_STREAM_BRIDGE_OK) &&
              (first_transferred == 0U) && (second_transferred == 2U),
          "backpressure in one direction does not block the reverse direction");
    check((second.output_length == 2U) &&
              (memcmp(second.output, first_payload, 2U) == 0) &&
              (first.output_length == sizeof(second_payload)) &&
              (memcmp(first.output, second_payload, sizeof(second_payload)) ==
               0),
          "backpressured direction retains data without reordering");

    second.write_result = 1;
    check((dbgc_byte_duplex_bridge_service(
               &bridge, 2U, 2U, &first_transferred, &second_transferred) ==
           DBGC_BYTE_STREAM_BRIDGE_OK) &&
              (first_transferred == 2U) && (second_transferred == 0U),
          "blocked direction resumes after endpoint becomes ready");
    check((second.output_length == sizeof(first_payload)) &&
              (memcmp(second.output, first_payload, sizeof(first_payload)) ==
               0),
          "blocked direction preserves byte order across backpressure");

    check((dbgc_byte_duplex_bridge_service(
               &bridge, 0U, 0U, &first_transferred, &second_transferred) ==
           DBGC_BYTE_STREAM_BRIDGE_OK) &&
              (first_transferred == 0U) && (second_transferred == 0U),
          "zero budgets leave both directions idle");

    first.read_result = -1;
    check(dbgc_byte_duplex_bridge_service(
              &bridge, 1U, 1U, &first_transferred, &second_transferred) ==
              DBGC_BYTE_STREAM_BRIDGE_ENDPOINT_ERROR,
          "first-direction endpoint error is returned");
    check((first_transferred == 0U) && (second_transferred == 0U),
          "endpoint error reports zero transfer counts");
    check(second.input_offset == sizeof(second_payload),
          "first-direction error prevents servicing the second direction");

    check(dbgc_byte_duplex_bridge_initialize(
              0, &first_fifo, endpoint_read, &first, endpoint_write, &second,
              &second_fifo, endpoint_read, &second, endpoint_write, &first) ==
              DBGC_BYTE_STREAM_BRIDGE_INVALID_ARGUMENT,
          "null bridge is rejected");
    check(dbgc_byte_duplex_bridge_initialize(
              &bridge, &first_fifo, endpoint_read, &first, endpoint_write,
              &second, &first_fifo, endpoint_read, &second, endpoint_write,
              &first) == DBGC_BYTE_STREAM_BRIDGE_INVALID_ARGUMENT,
          "shared FIFO object is rejected");

    (void)printf("Byte duplex bridge checks passed: %u\n", checks);
    return EXIT_SUCCESS;
}
