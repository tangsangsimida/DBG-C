#include "dbgc_cmsis_dap_service.h"

#include <stdio.h>
#include <string.h>

#include "DAP.h"

static unsigned int checks;
static unsigned int execute_calls;
static uint32_t execute_contract;

#define CHECK(condition)                                                               \
	do {                                                                           \
		++checks;                                                              \
		if (!(condition)) {                                                    \
			fprintf(stderr, "第 %u 项检查失败：%s\n", checks, #condition); \
			return 1;                                                      \
		}                                                                      \
	} while (0)

/**
 * @brief 为队列化 DAP 服务测试模拟固定 CMSIS-DAP 上游执行函数。
 * @param request DAP 请求首地址。
 * @param response 接收模拟响应的缓冲区。
 * @return 由测试配置的上游消费/响应长度打包值。
 */
static uint32_t dbgc_test_execute(const uint8_t *request, uint8_t *response)
{
	++execute_calls;
	response[0] = request[0];
	response[1] = DAP_OK;
	return execute_contract;
}

/**
 * @brief 验证 CMSIS-DAP 队列服务的成功、背压、边界拒绝和执行契约处理。
 * @return 全部检查通过时返回零，否则返回非零值。
 */
int main(void)
{
	dbgc_packet_queue_t request_queue;
	dbgc_packet_queue_t response_queue;
	uint8_t request_storage[8];
	uint8_t response_storage[4];
	size_t request_lengths[2];
	size_t response_lengths[2];
	uint8_t request_buffer[4];
	uint8_t response_buffer[4];
	uint8_t output[4];
	const uint8_t disconnect[] = { ID_DAP_Disconnect };
	const uint8_t vendor[] = { ID_DAP_Vendor0 };
	const uint8_t trailing[] = { ID_DAP_Disconnect, 0xA5U };
	dbgc_cmsis_dap_bounds_profile_t profile = { 1U, 0U, 0U, 0U, 0U };
	dbgc_cmsis_dap_service_result_t result;
	size_t output_length;

	CHECK(dbgc_packet_queue_initialize(&request_queue, request_storage, sizeof(request_storage),
					   request_lengths, 2U, 4U, 2U) == DBGC_PACKET_QUEUE_OK);
	CHECK(dbgc_packet_queue_initialize(&response_queue, response_storage,
					   sizeof(response_storage), response_lengths, 2U, 2U,
					   2U) == DBGC_PACKET_QUEUE_OK);
	CHECK(dbgc_cmsis_dap_service_process_one(
		      &request_queue, &response_queue, request_buffer, sizeof(request_buffer),
		      response_buffer, sizeof(response_buffer), &profile, dbgc_test_execute,
		      &result) == DBGC_CMSIS_DAP_SERVICE_REQUEST_EMPTY);
	CHECK(execute_calls == 0U);

	CHECK(dbgc_packet_queue_push(&request_queue, disconnect, sizeof(disconnect)) ==
	      DBGC_PACKET_QUEUE_OK);
	execute_contract = (UINT32_C(1) << 16) | UINT32_C(2);
	CHECK(dbgc_cmsis_dap_service_process_one(
		      &request_queue, &response_queue, request_buffer, sizeof(request_buffer),
		      response_buffer, sizeof(response_buffer), &profile, dbgc_test_execute,
		      &result) == DBGC_CMSIS_DAP_SERVICE_OK);
	CHECK(execute_calls == 1U);
	CHECK(result.request_bytes == 1U);
	CHECK(result.response_bytes == 2U);
	CHECK(dbgc_packet_queue_count(&request_queue) == 0U);
	CHECK(dbgc_packet_queue_count(&response_queue) == 1U);
	CHECK(dbgc_packet_queue_pop(&response_queue, output, sizeof(output), &output_length) ==
	      DBGC_PACKET_QUEUE_OK);
	CHECK(output_length == 2U);
	CHECK(output[0] == ID_DAP_Disconnect);
	CHECK(output[1] == DAP_OK);

	CHECK(dbgc_packet_queue_push(&response_queue, output, 1U) == DBGC_PACKET_QUEUE_OK);
	CHECK(dbgc_packet_queue_push(&response_queue, output, 1U) == DBGC_PACKET_QUEUE_OK);
	CHECK(dbgc_packet_queue_push(&request_queue, disconnect, sizeof(disconnect)) ==
	      DBGC_PACKET_QUEUE_OK);
	CHECK(dbgc_cmsis_dap_service_process_one(
		      &request_queue, &response_queue, request_buffer, sizeof(request_buffer),
		      response_buffer, sizeof(response_buffer), &profile, dbgc_test_execute,
		      &result) == DBGC_CMSIS_DAP_SERVICE_RESPONSE_QUEUE_FULL);
	CHECK(execute_calls == 1U);
	CHECK(dbgc_packet_queue_count(&request_queue) == 1U);
	dbgc_packet_queue_clear(&request_queue);
	dbgc_packet_queue_clear(&response_queue);

	CHECK(dbgc_packet_queue_push(&request_queue, disconnect, sizeof(disconnect)) ==
	      DBGC_PACKET_QUEUE_OK);
	CHECK(dbgc_cmsis_dap_service_process_one(&request_queue, &response_queue, request_buffer,
						 sizeof(request_buffer), response_buffer, 1U,
						 &profile, dbgc_test_execute, &result) ==
	      DBGC_CMSIS_DAP_SERVICE_RESPONSE_TOO_SMALL);
	CHECK(execute_calls == 1U);
	CHECK(dbgc_packet_queue_count(&request_queue) == 1U);
	dbgc_packet_queue_clear(&request_queue);

	CHECK(dbgc_packet_queue_push(&request_queue, trailing, sizeof(trailing)) ==
	      DBGC_PACKET_QUEUE_OK);
	CHECK(dbgc_cmsis_dap_service_process_one(
		      &request_queue, &response_queue, request_buffer, sizeof(request_buffer),
		      response_buffer, sizeof(response_buffer), &profile, dbgc_test_execute,
		      &result) == DBGC_CMSIS_DAP_SERVICE_TRAILING_INPUT);
	CHECK(execute_calls == 1U);
	CHECK(dbgc_packet_queue_count(&request_queue) == 1U);
	dbgc_packet_queue_clear(&request_queue);

	CHECK(dbgc_packet_queue_push(&request_queue, vendor, sizeof(vendor)) ==
	      DBGC_PACKET_QUEUE_OK);
	CHECK(dbgc_cmsis_dap_service_process_one(
		      &request_queue, &response_queue, request_buffer, sizeof(request_buffer),
		      response_buffer, sizeof(response_buffer), &profile, dbgc_test_execute,
		      &result) == DBGC_CMSIS_DAP_SERVICE_BOUNDS_REJECTED);
	CHECK(result.bounds_status == DBGC_CMSIS_DAP_BOUNDS_UNSUPPORTED_COMMAND);
	CHECK(execute_calls == 1U);
	CHECK(dbgc_packet_queue_count(&request_queue) == 1U);
	dbgc_packet_queue_clear(&request_queue);

	CHECK(dbgc_packet_queue_push(&request_queue, disconnect, sizeof(disconnect)) ==
	      DBGC_PACKET_QUEUE_OK);
	execute_contract = (UINT32_C(2) << 16) | UINT32_C(2);
	CHECK(dbgc_cmsis_dap_service_process_one(
		      &request_queue, &response_queue, request_buffer, sizeof(request_buffer),
		      response_buffer, sizeof(response_buffer), &profile, dbgc_test_execute,
		      &result) == DBGC_CMSIS_DAP_SERVICE_EXECUTION_CONTRACT);
	CHECK(execute_calls == 2U);
	CHECK(dbgc_packet_queue_count(&request_queue) == 0U);
	CHECK(dbgc_packet_queue_count(&response_queue) == 0U);

	CHECK(dbgc_cmsis_dap_service_process_one(
		      &request_queue, &response_queue, request_buffer, sizeof(request_buffer),
		      response_buffer, sizeof(response_buffer), &profile, dbgc_test_execute,
		      NULL) == DBGC_CMSIS_DAP_SERVICE_INVALID_ARGUMENT);

	printf("CMSIS-DAP queued service: %u 项检查通过\n", checks);
	return 0;
}
