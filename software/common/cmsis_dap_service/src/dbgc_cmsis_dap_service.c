#include "dbgc_cmsis_dap_service.h"

/**
 * @brief 比较两个容量并返回较小值。
 * @param left 第一个容量。
 * @param right 第二个容量。
 * @return 两个容量中的较小值。
 */
static size_t dbgc_cmsis_dap_service_min(size_t left, size_t right)
{
	return (left < right) ? left : right;
}

/**
 * @brief 处理单条已排队命令，并保持队列容量约束。
 * @param request_queue 请求队列。
 * @param response_queue 响应队列。
 * @param request_buffer 请求暂存区。
 * @param request_capacity 请求暂存区容量。
 * @param response_buffer 响应暂存区。
 * @param response_capacity 响应暂存区容量。
 * @param profile 命令边界配置。
 * @param execute 上游命令执行回调。
 * @param result 处理结果输出。
 * @return 服务状态。
 */
dbgc_cmsis_dap_service_status_t dbgc_cmsis_dap_service_process_one(
	dbgc_packet_queue_t *request_queue, dbgc_packet_queue_t *response_queue,
	uint8_t *request_buffer, size_t request_capacity, uint8_t *response_buffer,
	size_t response_capacity, const dbgc_cmsis_dap_bounds_profile_t *profile,
	dbgc_cmsis_dap_execute_fn execute, dbgc_cmsis_dap_service_result_t *result)
{
	dbgc_cmsis_dap_bounds_result_t bounds;
	dbgc_cmsis_dap_dispatch_result_t dispatch;
	dbgc_cmsis_dap_bounds_status_t bounds_status;
	dbgc_packet_queue_status_t queue_status;
	dbgc_cmsis_dap_service_status_t service_status;
	size_t request_length;
	size_t response_slot_capacity;
	size_t effective_response_capacity;

	if (result != NULL) {
		result->request_bytes = 0U;
		result->response_bytes = 0U;
		result->bounds_status = DBGC_CMSIS_DAP_BOUNDS_OK;
	}
	if ((request_queue == NULL) || (response_queue == NULL) ||
	    (request_queue == response_queue) || (request_buffer == NULL) ||
	    (response_buffer == NULL) || (request_capacity == 0U) || (response_capacity == 0U) ||
	    (profile == NULL) || (execute == NULL) || (result == NULL)) {
		return DBGC_CMSIS_DAP_SERVICE_INVALID_ARGUMENT;
	}

	if ((dbgc_packet_queue_capacity(request_queue) == 0U) ||
	    (dbgc_packet_queue_capacity(response_queue) == 0U)) {
		return DBGC_CMSIS_DAP_SERVICE_QUEUE_ERROR;
	}

	queue_status = dbgc_packet_queue_peek(request_queue, request_buffer, request_capacity,
					      &request_length);
	if (queue_status == DBGC_PACKET_QUEUE_EMPTY) {
		return DBGC_CMSIS_DAP_SERVICE_REQUEST_EMPTY;
	}
	if (queue_status == DBGC_PACKET_QUEUE_BUFFER_TOO_SMALL) {
		return DBGC_CMSIS_DAP_SERVICE_REQUEST_BUFFER_TOO_SMALL;
	}
	if (queue_status != DBGC_PACKET_QUEUE_OK) {
		return DBGC_CMSIS_DAP_SERVICE_QUEUE_ERROR;
	}

	if (dbgc_packet_queue_count(response_queue) >= dbgc_packet_queue_capacity(response_queue)) {
		return DBGC_CMSIS_DAP_SERVICE_RESPONSE_QUEUE_FULL;
	}

	response_slot_capacity = dbgc_packet_queue_record_capacity(response_queue);
	if (response_slot_capacity == 0U) {
		return DBGC_CMSIS_DAP_SERVICE_QUEUE_ERROR;
	}
	effective_response_capacity =
		dbgc_cmsis_dap_service_min(response_capacity, response_slot_capacity);

	bounds_status = dbgc_cmsis_dap_bounds_measure(
		request_buffer, request_length, effective_response_capacity, profile, &bounds);
	result->bounds_status = bounds_status;
	if (bounds_status == DBGC_CMSIS_DAP_BOUNDS_OUTPUT_TOO_SMALL) {
		return DBGC_CMSIS_DAP_SERVICE_RESPONSE_TOO_SMALL;
	}
	if (bounds_status != DBGC_CMSIS_DAP_BOUNDS_OK) {
		return DBGC_CMSIS_DAP_SERVICE_BOUNDS_REJECTED;
	}
	if (bounds.request_bytes != request_length) {
		return DBGC_CMSIS_DAP_SERVICE_TRAILING_INPUT;
	}

	service_status = DBGC_CMSIS_DAP_SERVICE_OK;
	bounds_status = dbgc_cmsis_dap_bounds_dispatch(request_buffer, request_length,
						       response_buffer, effective_response_capacity,
						       profile, execute, &dispatch);
	result->bounds_status = bounds_status;
	if (bounds_status == DBGC_CMSIS_DAP_BOUNDS_DISPATCH_CONTRACT) {
		queue_status = dbgc_packet_queue_pop(request_queue, request_buffer,
						     request_capacity, &request_length);
		if (queue_status != DBGC_PACKET_QUEUE_OK) {
			return DBGC_CMSIS_DAP_SERVICE_QUEUE_ERROR;
		}
		return DBGC_CMSIS_DAP_SERVICE_EXECUTION_CONTRACT;
	}
	if (bounds_status != DBGC_CMSIS_DAP_BOUNDS_OK) {
		return DBGC_CMSIS_DAP_SERVICE_BOUNDS_REJECTED;
	}

	result->request_bytes = dispatch.request_bytes;
	result->response_bytes = dispatch.response_bytes;
	queue_status =
		dbgc_packet_queue_push(response_queue, response_buffer, dispatch.response_bytes);
	if (queue_status != DBGC_PACKET_QUEUE_OK) {
		return DBGC_CMSIS_DAP_SERVICE_QUEUE_ERROR;
	}
	queue_status = dbgc_packet_queue_pop(request_queue, request_buffer, request_capacity,
					     &request_length);
	if (queue_status != DBGC_PACKET_QUEUE_OK) {
		return DBGC_CMSIS_DAP_SERVICE_QUEUE_ERROR;
	}

	return service_status;
}
