#include "dbgc_packet_queue.h"

#include <string.h>

/**
 * @brief 检查队列对象及其运行状态是否有效。
 * @param queue 待检查的队列对象。
 * @return 状态有效时返回非零值，否则返回零。
 */
static int dbgc_packet_queue_is_valid(const dbgc_packet_queue_t *queue)
{
	return (queue != NULL) && (queue->storage != NULL) && (queue->lengths != NULL) &&
	       (queue->storage_size != 0U) && (queue->slot_size != 0U) &&
	       (queue->slot_count != 0U) && (queue->length_count >= queue->slot_count) &&
	       (queue->slot_count <= (SIZE_MAX / queue->slot_size)) &&
	       (queue->storage_size >= (queue->slot_size * queue->slot_count)) &&
	       (queue->count <= queue->slot_count) && (queue->head < queue->slot_count) &&
	       (queue->tail < queue->slot_count);
}

/**
 * @brief 将槽位索引前移一位，并在末尾回绕到零。
 * @param index 当前槽位索引。
 * @param capacity 队列槽位总数。
 * @return 前移后的槽位索引。
 */
static size_t dbgc_packet_queue_advance(size_t index, size_t capacity)
{
	++index;
	return (index == capacity) ? 0U : index;
}

/**
 * @brief 初始化空队列并检查调用方提供的存储空间。
 * @param queue 队列对象。
 * @param storage 负载存储区。
 * @param storage_size 负载存储区字节数。
 * @param lengths 记录长度数组。
 * @param length_count 长度数组元素数。
 * @param slot_size 单条记录的最大字节数。
 * @param slot_count 队列槽位数。
 * @return 操作状态。
 */
dbgc_packet_queue_status_t dbgc_packet_queue_initialize(dbgc_packet_queue_t *queue,
							uint8_t *storage, size_t storage_size,
							size_t *lengths, size_t length_count,
							size_t slot_size, size_t slot_count)
{
	if (queue == NULL) {
		return DBGC_PACKET_QUEUE_INVALID;
	}

	queue->storage = NULL;
	queue->storage_size = 0U;
	queue->lengths = NULL;
	queue->length_count = 0U;
	queue->slot_size = 0U;
	queue->slot_count = 0U;
	queue->head = 0U;
	queue->tail = 0U;
	queue->count = 0U;

	if ((storage == NULL) || (lengths == NULL) || (storage_size == 0U) ||
	    (length_count == 0U) || (slot_size == 0U) || (slot_count == 0U) ||
	    (slot_count > (SIZE_MAX / slot_size)) || (storage_size < (slot_size * slot_count)) ||
	    (length_count < slot_count)) {
		return DBGC_PACKET_QUEUE_INVALID;
	}

	queue->storage = storage;
	queue->storage_size = storage_size;
	queue->lengths = lengths;
	queue->length_count = length_count;
	queue->slot_size = slot_size;
	queue->slot_count = slot_count;
	return DBGC_PACKET_QUEUE_OK;
}

/**
 * @brief 检查记录和容量后，将一条记录复制到队列尾部。
 * @param queue 队列对象。
 * @param data 要加入队列的记录数据。
 * @param length 记录长度。
 * @return 操作状态。
 */
dbgc_packet_queue_status_t dbgc_packet_queue_push(dbgc_packet_queue_t *queue, const uint8_t *data,
						  size_t length)
{
	size_t offset;

	if (!dbgc_packet_queue_is_valid(queue) || ((data == NULL) && (length != 0U))) {
		return DBGC_PACKET_QUEUE_INVALID;
	}
	if (length > queue->slot_size) {
		return DBGC_PACKET_QUEUE_TOO_LARGE;
	}
	if (queue->count == queue->slot_count) {
		return DBGC_PACKET_QUEUE_FULL;
	}

	offset = queue->tail * queue->slot_size;
	if (length != 0U) {
		memcpy(&queue->storage[offset], data, length);
	}
	queue->lengths[queue->tail] = length;
	queue->tail = dbgc_packet_queue_advance(queue->tail, queue->slot_count);
	++queue->count;
	return DBGC_PACKET_QUEUE_OK;
}

/**
 * @brief 将队列头部记录复制到输出缓冲区并移除该记录。
 * @param queue 队列对象。
 * @param data 接收记录数据的缓冲区。
 * @param output_capacity 输出缓冲区容量。
 * @param length 接收实际或所需记录长度的指针。
 * @return 操作状态。
 */
/**
 * @brief 复制队首记录，并按参数决定是否将记录移出队列。
 * @param queue 队列对象。
 * @param data 接收记录数据的缓冲区。
 * @param output_capacity 输出缓冲区容量。
 * @param length 接收实际或所需记录长度的指针。
 * @return 操作状态。
 */
static dbgc_packet_queue_status_t dbgc_packet_queue_copy_front(const dbgc_packet_queue_t *queue,
							       uint8_t *data,
							       size_t output_capacity,
							       size_t *length)
{
	size_t payload_length;
	size_t offset;

	if (!dbgc_packet_queue_is_valid(queue) || (length == NULL)) {
		return DBGC_PACKET_QUEUE_INVALID;
	}
	if (queue->count == 0U) {
		return DBGC_PACKET_QUEUE_EMPTY;
	}

	payload_length = queue->lengths[queue->head];
	if (payload_length > queue->slot_size) {
		return DBGC_PACKET_QUEUE_INVALID;
	}
	if ((data == NULL) && (payload_length != 0U)) {
		return DBGC_PACKET_QUEUE_INVALID;
	}
	if (output_capacity < payload_length) {
		*length = payload_length;
		return DBGC_PACKET_QUEUE_BUFFER_TOO_SMALL;
	}

	offset = queue->head * queue->slot_size;
	if (payload_length != 0U) {
		memcpy(data, &queue->storage[offset], payload_length);
	}
	*length = payload_length;
	return DBGC_PACKET_QUEUE_OK;
}

/**
 * @brief 从队列头部取出一条记录。
 * @param queue 队列对象。
 * @param data 接收记录数据的缓冲区。
 * @param output_capacity 输出缓冲区容量。
 * @param length 接收实际或所需记录长度的指针。
 * @return 操作状态。
 */
dbgc_packet_queue_status_t dbgc_packet_queue_pop(dbgc_packet_queue_t *queue, uint8_t *data,
						 size_t output_capacity, size_t *length)
{
	dbgc_packet_queue_status_t status;

	status = dbgc_packet_queue_copy_front(queue, data, output_capacity, length);
	if (status == DBGC_PACKET_QUEUE_OK) {
		queue->lengths[queue->head] = 0U;
		queue->head = dbgc_packet_queue_advance(queue->head, queue->slot_count);
		--queue->count;
	}

	return status;
}

/**
 * @brief 查看队首记录但保留其在队列中的状态。
 * @param queue 队列对象。
 * @param data 接收记录数据的缓冲区。
 * @param output_capacity 输出缓冲区容量。
 * @param length 接收实际或所需记录长度的指针。
 * @return 操作状态。
 */
dbgc_packet_queue_status_t dbgc_packet_queue_peek(const dbgc_packet_queue_t *queue, uint8_t *data,
						  size_t output_capacity, size_t *length)
{
	return dbgc_packet_queue_copy_front(queue, data, output_capacity, length);
}

/**
 * @brief 清空记录并重置队列索引。
 * @param queue 队列对象。
 */
void dbgc_packet_queue_clear(dbgc_packet_queue_t *queue)
{
	if (!dbgc_packet_queue_is_valid(queue)) {
		return;
	}

	queue->head = 0U;
	queue->tail = 0U;
	queue->count = 0U;
}

/**
 * @brief 查询当前记录数。
 * @param queue 队列对象。
 * @return 有效队列中的记录数；无效队列返回零。
 */
size_t dbgc_packet_queue_count(const dbgc_packet_queue_t *queue)
{
	return dbgc_packet_queue_is_valid(queue) ? queue->count : 0U;
}

/**
 * @brief 查询队列配置的槽位总数。
 * @param queue 队列对象。
 * @return 有效队列的槽位总数；无效队列返回零。
 */
size_t dbgc_packet_queue_capacity(const dbgc_packet_queue_t *queue)
{
	return dbgc_packet_queue_is_valid(queue) ? queue->slot_count : 0U;
}

/**
 * @brief 查询每条记录的槽位字节数。
 * @param queue 队列对象。
 * @return 有效队列的单条记录容量；无效队列返回零。
 */
size_t dbgc_packet_queue_record_capacity(const dbgc_packet_queue_t *queue)
{
	return dbgc_packet_queue_is_valid(queue) ? queue->slot_size : 0U;
}
