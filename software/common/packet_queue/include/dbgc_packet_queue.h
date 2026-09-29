#ifndef DBGC_PACKET_QUEUE_H
#define DBGC_PACKET_QUEUE_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief 定义定长包队列操作结果。
 */
typedef enum {
	/** 操作成功。 */
	DBGC_PACKET_QUEUE_OK = 0,
	/** 参数或队列状态无效。 */
	DBGC_PACKET_QUEUE_INVALID = -1,
	/** 队列已满。 */
	DBGC_PACKET_QUEUE_FULL = -2,
	/** 记录长度超过单个槽位容量。 */
	DBGC_PACKET_QUEUE_TOO_LARGE = -3,
	/** 队列为空。 */
	DBGC_PACKET_QUEUE_EMPTY = -4,
	/** 输出缓冲区不足，记录仍保留在队列中。 */
	DBGC_PACKET_QUEUE_BUFFER_TOO_SMALL = -5
} dbgc_packet_queue_status_t;

/**
 * @brief 表示由调用方提供存储空间的定长记录队列。
 *
 * 队列对象、长度数组和负载存储区必须互不重叠，并在队列使用期间保持有效。
 * push/pop 使用的数据缓冲区不得与上述区域重叠。队列不提供线程安全保证，
 * 调用方必须串行执行所有操作。负载大小和槽位数量由调用方提供；本模块不
 * 规定 USB、RF、BLE 或 CMSIS-DAP 的数据包尺寸。
 */
typedef struct {
	uint8_t *storage;
	size_t storage_size;
	size_t *lengths;
	size_t length_count;
	size_t slot_size;
	size_t slot_count;
	size_t head;
	size_t tail;
	size_t count;
} dbgc_packet_queue_t;

/**
 * @brief 初始化空队列。
 *
 * storage_size 必须至少容纳 slot_size 乘以 slot_count 个字节，length_count
 * 必须不小于 slot_count。参数包含空指针、零尺寸或乘法/容量溢出时返回
 * DBGC_PACKET_QUEUE_INVALID。初始化失败后，非空的 queue 对象会被清空并保持
 * 不可用。
 *
 * @param queue 队列对象。
 * @param storage 调用方提供的负载存储区。
 * @param storage_size 负载存储区的字节数。
 * @param lengths 调用方提供的记录长度数组。
 * @param length_count 长度数组的元素数。
 * @param slot_size 每个记录槽位的最大字节数。
 * @param slot_count 队列可容纳的记录数。
 * @return 操作状态。
 */
dbgc_packet_queue_status_t dbgc_packet_queue_initialize(dbgc_packet_queue_t *queue,
							uint8_t *storage, size_t storage_size,
							size_t *lengths, size_t length_count,
							size_t slot_size, size_t slot_count);

/**
 * @brief 向队列尾部加入一条记录。
 *
 * 零长度记录有效，此时 data 可以为空指针。失败时队列内容保持不变。
 *
 * @param queue 队列对象。
 * @param data 要复制到队列中的记录数据。
 * @param length 记录长度，不能超过单个槽位容量。
 * @return 操作状态。
 */
dbgc_packet_queue_status_t dbgc_packet_queue_push(dbgc_packet_queue_t *queue, const uint8_t *data,
						  size_t length);

/**
 * @brief 从队列头部取出一条记录。
 *
 * 返回 DBGC_PACKET_QUEUE_BUFFER_TOO_SMALL 时，length 写入所需负载长度且记录
 * 保留在队列中。返回 DBGC_PACKET_QUEUE_EMPTY 或 DBGC_PACKET_QUEUE_INVALID 时，
 * length 保持不变。零长度记录可以使用空的输出指针。
 *
 * @param queue 队列对象。
 * @param data 接收记录数据的输出缓冲区。
 * @param output_capacity 输出缓冲区容量。
 * @param length 接收实际或所需记录长度的指针。
 * @return 操作状态。
 */
dbgc_packet_queue_status_t dbgc_packet_queue_pop(dbgc_packet_queue_t *queue, uint8_t *data,
						 size_t output_capacity, size_t *length);

/** @brief 清空队列中的记录，但保留其配置和存储区。 */
void dbgc_packet_queue_clear(dbgc_packet_queue_t *queue);

/** @brief 返回当前队列中的记录数；无效队列返回零。 */
size_t dbgc_packet_queue_count(const dbgc_packet_queue_t *queue);

/** @brief 返回队列可容纳的记录数；无效队列返回零。 */
size_t dbgc_packet_queue_capacity(const dbgc_packet_queue_t *queue);

#endif
