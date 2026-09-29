#include "dbgc_packet_queue.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static unsigned int checks;

#define CHECK(condition)                                                               \
	do {                                                                           \
		++checks;                                                              \
		if (!(condition)) {                                                    \
			fprintf(stderr, "第 %u 项检查失败：%s\n", checks, #condition); \
			return 1;                                                      \
		}                                                                      \
	} while (0)

/**
 * @brief 检查定长包队列的参数边界、顺序、容量和清空行为。
 * @return 所有检查通过时返回零，否则返回非零值。
 */
int main(void)
{
	dbgc_packet_queue_t queue;
	uint8_t storage[12];
	size_t lengths[3];
	uint8_t output[6];
	const uint8_t first[] = { 0x11U, 0x12U, 0x13U };
	const uint8_t second[] = { 0x21U, 0x22U };
	const uint8_t third[] = { 0x31U, 0x32U, 0x33U, 0x34U };
	size_t output_length;

	memset(&queue, 0xA5, sizeof(queue));
	CHECK(dbgc_packet_queue_initialize(NULL, storage, sizeof(storage), lengths, 3U, 4U, 3U) ==
	      DBGC_PACKET_QUEUE_INVALID);
	CHECK(dbgc_packet_queue_initialize(&queue, NULL, sizeof(storage), lengths, 3U, 4U, 3U) ==
	      DBGC_PACKET_QUEUE_INVALID);
	CHECK(dbgc_packet_queue_capacity(&queue) == 0U);
	CHECK(dbgc_packet_queue_initialize(&queue, storage, sizeof(storage), NULL, 3U, 4U, 3U) ==
	      DBGC_PACKET_QUEUE_INVALID);
	CHECK(dbgc_packet_queue_initialize(&queue, storage, sizeof(storage), lengths, 2U, 4U, 3U) ==
	      DBGC_PACKET_QUEUE_INVALID);
	CHECK(dbgc_packet_queue_initialize(&queue, storage, 11U, lengths, 3U, 4U, 3U) ==
	      DBGC_PACKET_QUEUE_INVALID);
	CHECK(dbgc_packet_queue_initialize(&queue, storage, sizeof(storage), lengths, 3U, 0U, 3U) ==
	      DBGC_PACKET_QUEUE_INVALID);
	CHECK(dbgc_packet_queue_initialize(&queue, storage, sizeof(storage), lengths, 3U, 4U, 0U) ==
	      DBGC_PACKET_QUEUE_INVALID);
	CHECK(dbgc_packet_queue_initialize(&queue, storage, SIZE_MAX, lengths, 3U, SIZE_MAX, 2U) ==
	      DBGC_PACKET_QUEUE_INVALID);

	CHECK(dbgc_packet_queue_initialize(&queue, storage, sizeof(storage), lengths, 3U, 4U, 3U) ==
	      DBGC_PACKET_QUEUE_OK);
	CHECK(dbgc_packet_queue_count(&queue) == 0U);
	CHECK(dbgc_packet_queue_capacity(&queue) == 3U);
	output_length = 77U;
	CHECK(dbgc_packet_queue_pop(&queue, output, sizeof(output), &output_length) ==
	      DBGC_PACKET_QUEUE_EMPTY);
	CHECK(output_length == 77U);
	CHECK(dbgc_packet_queue_push(&queue, NULL, 1U) == DBGC_PACKET_QUEUE_INVALID);
	CHECK(dbgc_packet_queue_count(&queue) == 0U);
	CHECK(dbgc_packet_queue_push(&queue, first, sizeof(first)) == DBGC_PACKET_QUEUE_OK);
	CHECK(dbgc_packet_queue_push(&queue, second, sizeof(second)) == DBGC_PACKET_QUEUE_OK);
	CHECK(dbgc_packet_queue_push(&queue, third, sizeof(third)) == DBGC_PACKET_QUEUE_OK);
	CHECK(dbgc_packet_queue_push(&queue, third, 5U) == DBGC_PACKET_QUEUE_TOO_LARGE);
	CHECK(dbgc_packet_queue_push(&queue, first, sizeof(first)) == DBGC_PACKET_QUEUE_FULL);
	CHECK(dbgc_packet_queue_count(&queue) == 3U);

	output_length = 0U;
	CHECK(dbgc_packet_queue_pop(&queue, output, 2U, &output_length) ==
	      DBGC_PACKET_QUEUE_BUFFER_TOO_SMALL);
	CHECK(output_length == sizeof(first));
	CHECK(dbgc_packet_queue_count(&queue) == 3U);
	CHECK(dbgc_packet_queue_pop(&queue, NULL, sizeof(output), &output_length) ==
	      DBGC_PACKET_QUEUE_INVALID);
	CHECK(dbgc_packet_queue_count(&queue) == 3U);
	CHECK(dbgc_packet_queue_pop(&queue, output, sizeof(output), NULL) ==
	      DBGC_PACKET_QUEUE_INVALID);
	CHECK(dbgc_packet_queue_pop(&queue, output, sizeof(output), &output_length) ==
	      DBGC_PACKET_QUEUE_OK);
	CHECK(output_length == sizeof(first));
	CHECK(memcmp(output, first, sizeof(first)) == 0);
	CHECK(dbgc_packet_queue_count(&queue) == 2U);

	CHECK(dbgc_packet_queue_push(&queue, NULL, 0U) == DBGC_PACKET_QUEUE_OK);
	CHECK(dbgc_packet_queue_pop(&queue, output, sizeof(output), &output_length) ==
	      DBGC_PACKET_QUEUE_OK);
	CHECK(output_length == sizeof(second));
	CHECK(memcmp(output, second, sizeof(second)) == 0);
	CHECK(dbgc_packet_queue_pop(&queue, output, sizeof(output), &output_length) ==
	      DBGC_PACKET_QUEUE_OK);
	CHECK(output_length == sizeof(third));
	CHECK(memcmp(output, third, sizeof(third)) == 0);
	CHECK(dbgc_packet_queue_pop(&queue, NULL, 0U, &output_length) == DBGC_PACKET_QUEUE_OK);
	CHECK(output_length == 0U);
	CHECK(dbgc_packet_queue_count(&queue) == 0U);

	CHECK(dbgc_packet_queue_push(&queue, first, sizeof(first)) == DBGC_PACKET_QUEUE_OK);
	CHECK(dbgc_packet_queue_push(&queue, third, sizeof(third)) == DBGC_PACKET_QUEUE_OK);
	CHECK(dbgc_packet_queue_pop(&queue, output, sizeof(output), &output_length) ==
	      DBGC_PACKET_QUEUE_OK);
	CHECK(output_length == sizeof(first));
	CHECK(memcmp(output, first, sizeof(first)) == 0);
	CHECK(dbgc_packet_queue_pop(&queue, output, sizeof(output), &output_length) ==
	      DBGC_PACKET_QUEUE_OK);
	CHECK(output_length == sizeof(third));
	CHECK(memcmp(output, third, sizeof(third)) == 0);

	CHECK(dbgc_packet_queue_push(&queue, second, sizeof(second)) == DBGC_PACKET_QUEUE_OK);
	dbgc_packet_queue_clear(&queue);
	CHECK(dbgc_packet_queue_count(&queue) == 0U);
	CHECK(dbgc_packet_queue_capacity(&queue) == 3U);
	CHECK(dbgc_packet_queue_pop(&queue, output, sizeof(output), &output_length) ==
	      DBGC_PACKET_QUEUE_EMPTY);

	printf("定长包队列检查通过：%u\n", checks);
	return 0;
}
