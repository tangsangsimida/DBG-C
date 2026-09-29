#include "dbgc_byte_fifo.h"

#include <stdio.h>
#include <string.h>

static unsigned int checks;
static unsigned int failures;

#define CHECK(condition)                                                                       \
	do {                                                                                   \
		++checks;                                                                      \
		if (!(condition)) {                                                            \
			++failures;                                                            \
			(void)fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, \
				      #condition);                                             \
		}                                                                              \
	} while (0)

static void test_initialize_boundaries(void)
{
	dbgc_byte_fifo_t fifo;
	uint8_t storage[3];
	const uint8_t one_byte = 0x7DU;
	uint8_t readback = 0U;

	(void)memset(&fifo, 0xA5, sizeof(fifo));
	CHECK(dbgc_byte_fifo_initialize(NULL, storage, sizeof(storage)) != 0);
	CHECK(dbgc_byte_fifo_initialize(&fifo, NULL, sizeof(storage)) != 0);
	CHECK(fifo.storage == NULL);
	CHECK(fifo.capacity == 0U);
	CHECK(dbgc_byte_fifo_initialize(&fifo, storage, 0U) != 0);
	CHECK(fifo.storage == NULL);
	CHECK(fifo.capacity == 0U);
	CHECK(dbgc_byte_fifo_initialize(&fifo, storage, 1U) == 0);
	CHECK(dbgc_byte_fifo_capacity(&fifo) == 1U);
	CHECK(dbgc_byte_fifo_write(&fifo, &one_byte, 1U) == 1U);
	CHECK(dbgc_byte_fifo_count(&fifo) == 1U);
	CHECK(dbgc_byte_fifo_write(&fifo, &one_byte, 1U) == 0U);
	CHECK(dbgc_byte_fifo_read(&fifo, &readback, 1U) == 1U);
	CHECK(readback == one_byte);
	CHECK(dbgc_byte_fifo_initialize(&fifo, storage, sizeof(storage)) == 0);
	CHECK(dbgc_byte_fifo_capacity(&fifo) == sizeof(storage));
	CHECK(dbgc_byte_fifo_initialize(NULL, NULL, 0U) != 0);
}

static void test_write_read_and_full(void)
{
	dbgc_byte_fifo_t fifo;
	uint8_t guarded_storage[5] = { 0xA5U, 0U, 0U, 0U, 0x5AU };
	const uint8_t input[] = { 0x11U, 0x22U, 0x33U, 0x44U };
	uint8_t output[4] = { 0U };

	CHECK(dbgc_byte_fifo_initialize(&fifo, &guarded_storage[1], 3U) == 0);
	CHECK(dbgc_byte_fifo_read(&fifo, output, sizeof(output)) == 0U);
	CHECK(dbgc_byte_fifo_write(&fifo, input, 2U) == 2U);
	CHECK(dbgc_byte_fifo_write(&fifo, &input[2], 2U) == 1U);
	CHECK(dbgc_byte_fifo_count(&fifo) == 3U);
	CHECK(dbgc_byte_fifo_space(&fifo) == 0U);
	CHECK(dbgc_byte_fifo_write(&fifo, &input[3], 1U) == 0U);
	CHECK(dbgc_byte_fifo_read(&fifo, output, 1U) == 1U);
	CHECK(output[0] == input[0]);
	CHECK(dbgc_byte_fifo_read(&fifo, &output[1], sizeof(output) - 1U) == 2U);
	CHECK(output[1] == input[1] && output[2] == input[2]);
	CHECK(dbgc_byte_fifo_count(&fifo) == 0U);
	CHECK(guarded_storage[0] == 0xA5U && guarded_storage[4] == 0x5AU);
}

static void test_wraparound(void)
{
	dbgc_byte_fifo_t fifo;
	uint8_t guarded_storage[5] = { 0xA5U, 0U, 0U, 0U, 0x5AU };
	const uint8_t first[] = { 1U, 2U };
	const uint8_t second[] = { 3U, 4U };
	uint8_t output[4] = { 0U };

	CHECK(dbgc_byte_fifo_initialize(&fifo, &guarded_storage[1], 3U) == 0);
	CHECK(dbgc_byte_fifo_write(&fifo, first, sizeof(first)) == sizeof(first));
	CHECK(dbgc_byte_fifo_read(&fifo, output, 1U) == 1U);
	CHECK(output[0] == 1U);
	CHECK(dbgc_byte_fifo_write(&fifo, second, sizeof(second)) == sizeof(second));
	CHECK(dbgc_byte_fifo_read(&fifo, output, sizeof(output)) == 3U);
	CHECK(output[0] == 2U && output[1] == 3U && output[2] == 4U);
	CHECK(dbgc_byte_fifo_count(&fifo) == 0U);
	CHECK(guarded_storage[0] == 0xA5U && guarded_storage[4] == 0x5AU);
}

static void test_invalid_buffers_and_zero_length(void)
{
	dbgc_byte_fifo_t fifo;
	uint8_t storage[2];
	const uint8_t input[] = { 0xA1U, 0xB2U };
	uint8_t output[2] = { 0U };

	CHECK(dbgc_byte_fifo_initialize(&fifo, storage, sizeof(storage)) == 0);
	CHECK(dbgc_byte_fifo_write(&fifo, input, sizeof(input)) == sizeof(input));
	CHECK(dbgc_byte_fifo_write(&fifo, NULL, 1U) == 0U);
	CHECK(dbgc_byte_fifo_read(&fifo, NULL, 1U) == 0U);
	CHECK(dbgc_byte_fifo_count(&fifo) == sizeof(input));
	CHECK(dbgc_byte_fifo_write(&fifo, NULL, 0U) == 0U);
	CHECK(dbgc_byte_fifo_read(&fifo, NULL, 0U) == 0U);
	CHECK(dbgc_byte_fifo_count(&fifo) == sizeof(input));
	CHECK(dbgc_byte_fifo_read(&fifo, output, sizeof(output)) == sizeof(output));
	CHECK(output[0] == input[0] && output[1] == input[1]);
}

static void test_clear_and_queries(void)
{
	dbgc_byte_fifo_t fifo;
	uint8_t storage[4];
	const uint8_t input[] = { 1U, 2U, 3U };

	CHECK(dbgc_byte_fifo_count(NULL) == 0U);
	CHECK(dbgc_byte_fifo_capacity(NULL) == 0U);
	CHECK(dbgc_byte_fifo_space(NULL) == 0U);
	dbgc_byte_fifo_clear(NULL);
	CHECK(dbgc_byte_fifo_initialize(&fifo, storage, sizeof(storage)) == 0);
	CHECK(dbgc_byte_fifo_capacity(&fifo) == sizeof(storage));
	CHECK(dbgc_byte_fifo_space(&fifo) == sizeof(storage));
	CHECK(dbgc_byte_fifo_write(&fifo, input, sizeof(input)) == sizeof(input));
	CHECK(dbgc_byte_fifo_count(&fifo) == sizeof(input));
	CHECK(dbgc_byte_fifo_space(&fifo) == 1U);
	dbgc_byte_fifo_clear(&fifo);
	CHECK(dbgc_byte_fifo_count(&fifo) == 0U);
	CHECK(dbgc_byte_fifo_capacity(&fifo) == sizeof(storage));
	CHECK(dbgc_byte_fifo_space(&fifo) == sizeof(storage));
	dbgc_byte_fifo_clear(&fifo);
	CHECK(dbgc_byte_fifo_count(&fifo) == 0U);
}

static void test_deterministic_state_sequence(void)
{
	enum {
		FIFO_CAPACITY = 7U,
		STEP_COUNT = 512U
	};
	dbgc_byte_fifo_t fifo;
	uint8_t guarded_storage[FIFO_CAPACITY + 2U] = { 0xA5U };
	uint8_t reference[FIFO_CAPACITY];
	uint8_t input[9];
	uint8_t output[9];
	size_t reference_count = 0U;
	size_t step;

	guarded_storage[FIFO_CAPACITY + 1U] = 0x5AU;
	CHECK(dbgc_byte_fifo_initialize(&fifo, &guarded_storage[1], FIFO_CAPACITY) == 0);

	for (step = 0U; step < STEP_COUNT; ++step) {
		size_t write_length = (step * 13U) % sizeof(input);
		size_t expected_write;
		size_t actual_write;
		size_t read_length = (step * 7U + 3U) % sizeof(output);
		size_t expected_read;
		size_t actual_read;
		size_t index;

		if ((step % 29U) == 0U) {
			dbgc_byte_fifo_clear(&fifo);
			reference_count = 0U;
			CHECK(dbgc_byte_fifo_count(&fifo) == 0U);
			CHECK(dbgc_byte_fifo_space(&fifo) == FIFO_CAPACITY);
		}

		for (index = 0U; index < sizeof(input); ++index) {
			input[index] = (uint8_t)(step + (index * 37U));
		}

		expected_write = FIFO_CAPACITY - reference_count;
		if (write_length < expected_write) {
			expected_write = write_length;
		}
		actual_write = dbgc_byte_fifo_write(&fifo, input, write_length);
		CHECK(actual_write == expected_write);
		for (index = 0U; index < expected_write; ++index) {
			reference[reference_count + index] = input[index];
		}
		reference_count += expected_write;

		expected_read = (read_length < reference_count) ? read_length : reference_count;
		actual_read = dbgc_byte_fifo_read(&fifo, output, read_length);
		CHECK(actual_read == expected_read);
		CHECK(memcmp(output, reference, expected_read) == 0);
		reference_count -= expected_read;
		if (reference_count != 0U) {
			(void)memmove(reference, &reference[expected_read], reference_count);
		}

		CHECK(dbgc_byte_fifo_count(&fifo) == reference_count);
		CHECK(dbgc_byte_fifo_space(&fifo) == FIFO_CAPACITY - reference_count);
		CHECK(guarded_storage[0] == 0xA5U);
		CHECK(guarded_storage[FIFO_CAPACITY + 1U] == 0x5AU);
	}
}

int main(void)
{
	test_initialize_boundaries();
	test_write_read_and_full();
	test_wraparound();
	test_invalid_buffers_and_zero_length();
	test_clear_and_queries();
	test_deterministic_state_sequence();

	if (failures != 0U) {
		(void)fprintf(stderr, "%u of %u checks failed\n", failures, checks);
		return 1;
	}

	(void)printf("PASS: %u byte FIFO checks\n", checks);
	return 0;
}
