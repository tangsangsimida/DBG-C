#ifndef DBGC_CMSIS_DAP_SERVICE_H
#define DBGC_CMSIS_DAP_SERVICE_H

#include "dbgc_cmsis_dap_bounds.h"
#include "dbgc_packet_queue.h"

typedef enum {
	DBGC_CMSIS_DAP_SERVICE_OK = 0,
	DBGC_CMSIS_DAP_SERVICE_INVALID_ARGUMENT,
	DBGC_CMSIS_DAP_SERVICE_REQUEST_EMPTY,
	DBGC_CMSIS_DAP_SERVICE_REQUEST_BUFFER_TOO_SMALL,
	DBGC_CMSIS_DAP_SERVICE_RESPONSE_QUEUE_FULL,
	DBGC_CMSIS_DAP_SERVICE_RESPONSE_TOO_SMALL,
	DBGC_CMSIS_DAP_SERVICE_TRAILING_INPUT,
	DBGC_CMSIS_DAP_SERVICE_BOUNDS_REJECTED,
	DBGC_CMSIS_DAP_SERVICE_EXECUTION_CONTRACT,
	DBGC_CMSIS_DAP_SERVICE_QUEUE_ERROR
} dbgc_cmsis_dap_service_status_t;

/**
 * @brief 记录一次队列化 CMSIS-DAP 命令处理的长度及预检结果。
 */
typedef struct {
	size_t request_bytes;
	size_t response_bytes;
	dbgc_cmsis_dap_bounds_status_t bounds_status;
} dbgc_cmsis_dap_service_result_t;

/**
 * @brief 从请求队列处理一条 CMSIS-DAP 命令，并将响应放入响应队列。
 *
 * 每条请求记录必须恰好包含一条完整命令；若边界预检发现记录尾部有未消费
 * 字节，函数会在执行命令前拒绝该记录并保留在请求队列中。所有队列、缓冲区
 * 和配置均由调用方提供；函数不分配内存，也不选择传输包长或队列容量。
 *
 * 请求队列和响应队列必须是不同对象；两个队列对象、长度数组、存储区以及请求和
 * 响应暂存区彼此不得重叠。调用方必须串行调用本函数及队列 API；execute 回调不得
 * 访问或修改这两个队列。响应队列无空槽、缓冲区或槽位容量不足、请求不完整或
 * 边界预检拒绝时，不调用 execute，且请求保留在队列中。如果 execute 已被调用但
 * 违反上游长度契约，函数会移除该请求以避免调用方自动重放可能已产生副作用的命令。
 *
 * @param request_queue 保存完整 DAP 命令记录的队列。
 * @param response_queue 接收 DAP 响应记录的队列。
 * @param request_buffer 调用方提供的请求暂存区。
 * @param request_capacity 请求暂存区容量。
 * @param response_buffer 调用方提供的响应暂存区。
 * @param response_capacity 响应暂存区容量。
 * @param profile 与固定 CMSIS-DAP 构建配置一致的边界配置。
 * @param execute 固定上游 CMSIS-DAP 命令执行函数。
 * @param result 接收处理长度和边界状态的结构体。
 * @return 本次处理状态。
 */
dbgc_cmsis_dap_service_status_t dbgc_cmsis_dap_service_process_one(
	dbgc_packet_queue_t *request_queue, dbgc_packet_queue_t *response_queue,
	uint8_t *request_buffer, size_t request_capacity, uint8_t *response_buffer,
	size_t response_capacity, const dbgc_cmsis_dap_bounds_profile_t *profile,
	dbgc_cmsis_dap_execute_fn execute, dbgc_cmsis_dap_service_result_t *result);

#endif
