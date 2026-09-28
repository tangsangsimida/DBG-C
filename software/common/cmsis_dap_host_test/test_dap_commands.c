#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "DAP_config.h"
#include "DAP.h"
#include "dbgc_cmsis_dap_bounds.h"

static unsigned int failures;
static unsigned int mock_swd_calls;
static unsigned int dispatch_callback_calls;
static unsigned int mock_wait_responses;
static uint32_t mock_read_value;
static uint32_t mock_read_sequence[4];
static unsigned int mock_read_sequence_length;
static unsigned int mock_read_sequence_index;
static uint32_t mock_last_request;
static uint32_t mock_last_write_value;
static uint8_t mock_final_response;

static const dbgc_cmsis_dap_bounds_profile_t dispatch_profile = {
    1U, 0U, 0U, 0U, 64U
};

static uint32_t count_dispatch_callback(const uint8_t *request,
                                        uint8_t *response)
{
    (void)request;
    (void)response;
    dispatch_callback_calls++;
    return 0U;
}

void dbgc_test_swclk_set(void)
{
}

void dbgc_test_swclk_clear(void)
{
}

void dbgc_test_swdio_output(uint32_t bit)
{
    (void)bit;
}

uint32_t dbgc_test_swdio_input(void)
{
    return 0U;
}

void dbgc_test_swdio_output_enable(void)
{
}

void dbgc_test_swdio_output_disable(void)
{
}

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        failures++; \
    } \
} while (0)

uint8_t SWD_Transfer(uint32_t request, uint32_t *data)
{
    mock_swd_calls++;
    mock_last_request = request;

    if (data != NULL) {
        if ((request & DAP_TRANSFER_RnW) != 0U) {
            if (mock_read_sequence_index < mock_read_sequence_length) {
                *data = mock_read_sequence[mock_read_sequence_index++];
            } else {
                *data = mock_read_value;
            }
        } else {
            mock_last_write_value = *data;
        }
    }

    if (mock_wait_responses != 0U) {
        mock_wait_responses--;
        return DAP_TRANSFER_WAIT;
    }

    return mock_final_response;
}

void SWJ_Sequence(uint32_t count, const uint8_t *data)
{
    (void)count;
    (void)data;
}

void SWD_Sequence(uint32_t info, const uint8_t *swdo, uint8_t *swdi)
{
    (void)info;
    (void)swdo;
    (void)swdi;
}

static void connect_swd(void)
{
    static const uint8_t request[] = { ID_DAP_Connect, DAP_PORT_SWD };
    uint8_t response[8] = {0};
    uint32_t result;

    DAP_Setup();
    result = DAP_ExecuteCommand(request, response);
    CHECK((result & 0xffffU) == 2U);
    CHECK((result >> 16) == sizeof(request));
    CHECK(response[0] == ID_DAP_Connect);
    CHECK(response[1] == DAP_PORT_SWD);
}

static void test_firmware_version_info(void)
{
    static const uint8_t request[] = { ID_DAP_Info, DAP_ID_DAP_FW_VER };
    static const char expected[] = "2.1.2";
    uint8_t response[64] = {0};
    uint32_t result = DAP_ExecuteCommand(request, response);

    CHECK((result & 0xffffU) == (2U + sizeof(expected)));
    CHECK((result >> 16) == sizeof(request));
    CHECK(response[0] == ID_DAP_Info);
    CHECK(response[1] == sizeof(expected));
    CHECK(memcmp(&response[2], expected, sizeof(expected)) == 0);
}

static void test_unknown_info_identifier(void)
{
    static const uint8_t request[] = { ID_DAP_Info, 0xA0U };
    uint8_t response[8] = {0};
    uint32_t result = DAP_ExecuteCommand(request, response);

    CHECK((result & 0xffffU) == 2U);
    CHECK((result >> 16) == sizeof(request));
    CHECK(response[0] == ID_DAP_Info);
    CHECK(response[1] == 0U);
}

static void test_unknown_command(void)
{
    static const uint8_t request[] = { 0x30U };
    uint8_t response[8] = {0};
    uint32_t result = DAP_ExecuteCommand(request, response);

    CHECK((result & 0xffffU) == 1U);
    CHECK((result >> 16) == sizeof(request));
    CHECK(response[0] == ID_DAP_Invalid);
}

static void test_swd_read_and_wait_retry(void)
{
    static const uint8_t request[] = {
        ID_DAP_Transfer, 0U, 1U, DAP_TRANSFER_RnW
    };
    uint8_t response[16] = {0};
    uint32_t result;

    connect_swd();
    mock_swd_calls = 0U;
    mock_wait_responses = 2U;
    mock_read_value = 0x1234abcdu;
    mock_final_response = DAP_TRANSFER_OK;
    result = DAP_ExecuteCommand(request, response);

    CHECK((result & 0xffffU) == 7U);
    CHECK((result >> 16) == sizeof(request));
    CHECK(response[0] == ID_DAP_Transfer);
    CHECK(response[1] == 1U);
    CHECK(response[2] == DAP_TRANSFER_OK);
    CHECK(response[3] == 0xcdu);
    CHECK(response[4] == 0xabu);
    CHECK(response[5] == 0x34u);
    CHECK(response[6] == 0x12u);
    CHECK(mock_swd_calls == 3U);
    CHECK(mock_last_request == DAP_TRANSFER_RnW);
}

static void test_swd_write_and_completion_check(void)
{
    static const uint8_t request[] = {
        ID_DAP_Transfer, 0U, 1U, 0U, 0x78u, 0x56u, 0x34u, 0x12u
    };
    uint8_t response[8] = {0};
    uint32_t result;

    connect_swd();
    mock_swd_calls = 0U;
    mock_wait_responses = 0U;
    mock_final_response = DAP_TRANSFER_OK;
    result = DAP_ExecuteCommand(request, response);

    CHECK((result & 0xffffU) == 3U);
    CHECK((result >> 16) == sizeof(request));
    CHECK(response[0] == ID_DAP_Transfer);
    CHECK(response[1] == 1U);
    CHECK(response[2] == DAP_TRANSFER_OK);
    CHECK(mock_swd_calls == 2U);
    CHECK(mock_last_write_value == 0x12345678u);
    CHECK(mock_last_request == (DP_RDBUFF | DAP_TRANSFER_RnW));
}

static void test_swd_posted_ap_reads(void)
{
    static const uint8_t request[] = {
        ID_DAP_Transfer, 0U, 2U,
        DAP_TRANSFER_APnDP | DAP_TRANSFER_RnW,
        DAP_TRANSFER_APnDP | DAP_TRANSFER_RnW
    };
    uint8_t response[16] = {0};
    uint32_t result;

    connect_swd();
    mock_swd_calls = 0U;
    mock_wait_responses = 0U;
    mock_read_sequence[0] = 0x11223344u;
    mock_read_sequence[1] = 0xaabbccddu;
    mock_read_sequence_length = 2U;
    mock_read_sequence_index = 0U;
    mock_final_response = DAP_TRANSFER_OK;
    result = DAP_ExecuteCommand(request, response);

    CHECK((result & 0xffffU) == 11U);
    CHECK((result >> 16) == sizeof(request));
    CHECK(response[0] == ID_DAP_Transfer);
    CHECK(response[1] == 2U);
    CHECK(response[2] == DAP_TRANSFER_OK);
    CHECK(response[3] == 0x44u);
    CHECK(response[4] == 0x33u);
    CHECK(response[5] == 0x22u);
    CHECK(response[6] == 0x11u);
    CHECK(response[7] == 0xddu);
    CHECK(response[8] == 0xccu);
    CHECK(response[9] == 0xbbu);
    CHECK(response[10] == 0xaau);
    CHECK(mock_swd_calls == 3U);
    CHECK(mock_read_sequence_index == 2U);
    CHECK(mock_last_request == (DP_RDBUFF | DAP_TRANSFER_RnW));
}

static void test_transfer_configure_retry_count(void)
{
    static const uint8_t configure_request[] = {
        ID_DAP_TransferConfigure, 0U, 1U, 0U, 0U, 0U
    };
    static const uint8_t transfer_request[] = {
        ID_DAP_Transfer, 0U, 1U, DAP_TRANSFER_RnW
    };
    uint8_t response[16] = {0};
    uint32_t result;

    connect_swd();
    result = DAP_ExecuteCommand(configure_request, response);

    CHECK((result & 0xffffU) == 2U);
    CHECK((result >> 16) == sizeof(configure_request));
    CHECK(response[0] == ID_DAP_TransferConfigure);
    CHECK(response[1] == DAP_OK);

    mock_swd_calls = 0U;
    mock_wait_responses = 2U;
    mock_final_response = DAP_TRANSFER_OK;
    result = DAP_ExecuteCommand(transfer_request, response);

    CHECK((result & 0xffffU) == 3U);
    CHECK((result >> 16) == sizeof(transfer_request));
    CHECK(response[0] == ID_DAP_Transfer);
    CHECK(response[1] == 0U);
    CHECK(response[2] == DAP_TRANSFER_WAIT);
    CHECK(mock_swd_calls == 2U);
}

static void test_swd_transfer_block_posted_ap_reads(void)
{
    static const uint8_t request[] = {
        ID_DAP_TransferBlock, 0U, 2U, 0U,
        DAP_TRANSFER_APnDP | DAP_TRANSFER_RnW
    };
    uint8_t response[16] = {0};
    uint32_t result;

    connect_swd();
    mock_swd_calls = 0U;
    mock_wait_responses = 0U;
    mock_read_sequence[0] = 0x01234567u;
    mock_read_sequence[1] = 0x89abcdefu;
    mock_read_sequence_length = 2U;
    mock_read_sequence_index = 0U;
    mock_final_response = DAP_TRANSFER_OK;
    result = DAP_ExecuteCommand(request, response);

    CHECK((result & 0xffffU) == 12U);
    CHECK(response[0] == ID_DAP_TransferBlock);
    CHECK(response[1] == 2U);
    CHECK(response[2] == 0U);
    CHECK(response[3] == DAP_TRANSFER_OK);
    CHECK(response[4] == 0x67u);
    CHECK(response[5] == 0x45u);
    CHECK(response[6] == 0x23u);
    CHECK(response[7] == 0x01u);
    CHECK(response[8] == 0xefu);
    CHECK(response[9] == 0xcdu);
    CHECK(response[10] == 0xabu);
    CHECK(response[11] == 0x89u);
    CHECK(mock_swd_calls == 3U);
    CHECK(mock_read_sequence_index == 2U);
    CHECK(mock_last_request == (DP_RDBUFF | DAP_TRANSFER_RnW));
}

static void test_bounded_dispatch_rejects_before_upstream(void)
{
    static const uint8_t truncated_write[] = {
        ID_DAP_Transfer, 0U, 1U, 0U
    };
    static const uint8_t connect_request[] = {
        ID_DAP_Connect, DAP_PORT_SWD
    };
    uint8_t response[8];
    dbgc_cmsis_dap_dispatch_result_t result = { 99U, 99U };
    dbgc_cmsis_dap_bounds_status_t status;

    memset(response, 0xA5, sizeof(response));
    mock_swd_calls = 0U;
    status = dbgc_cmsis_dap_bounds_dispatch(
        truncated_write, sizeof(truncated_write), response, sizeof(response),
        &dispatch_profile, DAP_ExecuteCommand, &result);

    CHECK(status == DBGC_CMSIS_DAP_BOUNDS_NEED_MORE_INPUT);
    CHECK(mock_swd_calls == 0U);
    CHECK(response[0] == 0xA5U);
    CHECK(result.request_bytes == 0U);
    CHECK(result.response_bytes == 0U);

    dispatch_callback_calls = 0U;
    status = dbgc_cmsis_dap_bounds_dispatch(
        connect_request, sizeof(connect_request), response, 1U,
        &dispatch_profile, count_dispatch_callback, &result);
    CHECK(status == DBGC_CMSIS_DAP_BOUNDS_OUTPUT_TOO_SMALL);
    CHECK(dispatch_callback_calls == 0U);
}

static void test_bounded_dispatch_runs_upstream_after_preflight(void)
{
    static const uint8_t request[] = { ID_DAP_Connect, DAP_PORT_SWD };
    uint8_t response[2] = { 0U, 0U };
    dbgc_cmsis_dap_dispatch_result_t result = { 0U, 0U };
    dbgc_cmsis_dap_bounds_status_t status;

    DAP_Setup();
    status = dbgc_cmsis_dap_bounds_dispatch(
        request, sizeof(request), response, sizeof(response),
        &dispatch_profile, DAP_ExecuteCommand, &result);

    CHECK(status == DBGC_CMSIS_DAP_BOUNDS_OK);
    CHECK(result.request_bytes == sizeof(request));
    CHECK(result.response_bytes == sizeof(response));
    CHECK(response[0] == ID_DAP_Connect);
    CHECK(response[1] == DAP_PORT_SWD);
}

static void test_bounded_dispatch_detects_length_contract_mismatch(void)
{
    static const uint8_t request[] = { ID_DAP_Connect, DAP_PORT_SWD };
    uint8_t response[2] = { 0U, 0U };
    dbgc_cmsis_dap_dispatch_result_t result = { 99U, 99U };
    dbgc_cmsis_dap_bounds_status_t status;

    status = dbgc_cmsis_dap_bounds_dispatch(
        request, sizeof(request), response, sizeof(response),
        &dispatch_profile, count_dispatch_callback, &result);

    CHECK(status == DBGC_CMSIS_DAP_BOUNDS_DISPATCH_CONTRACT);
    CHECK(dispatch_callback_calls == 1U);
    CHECK(result.request_bytes == 0U);
    CHECK(result.response_bytes == 0U);
}

static void test_bounded_dispatch_execute_commands(void)
{
    static const uint8_t request[] = {
        ID_DAP_ExecuteCommands, 2U,
        ID_DAP_Connect, DAP_PORT_SWD,
        ID_DAP_Transfer, 0U, 1U, DAP_TRANSFER_RnW
    };
    uint8_t response[16] = {0U};
    dbgc_cmsis_dap_dispatch_result_t result = { 0U, 0U };
    dbgc_cmsis_dap_bounds_status_t status;

    DAP_Setup();
    mock_swd_calls = 0U;
    mock_read_value = 0x89ABCDEFu;
    mock_final_response = DAP_TRANSFER_OK;
    status = dbgc_cmsis_dap_bounds_dispatch(
        request, sizeof(request), response, sizeof(response),
        &dispatch_profile, DAP_ExecuteCommand, &result);

    CHECK(status == DBGC_CMSIS_DAP_BOUNDS_OK);
    CHECK(result.request_bytes == sizeof(request));
    CHECK(result.response_bytes == 11U);
    CHECK(response[0] == ID_DAP_ExecuteCommands);
    CHECK(response[1] == 2U);
    CHECK(response[2] == ID_DAP_Connect);
    CHECK(response[4] == ID_DAP_Transfer);
    CHECK(mock_swd_calls == 1U);
}

static void test_bounded_dispatch_transfer_error_consumes_full_request(void)
{
    static const uint8_t request[] = {
        ID_DAP_Transfer, 0U, 2U,
        DAP_TRANSFER_RnW, DAP_TRANSFER_RnW
    };
    uint8_t response[16] = {0U};
    dbgc_cmsis_dap_dispatch_result_t result = { 0U, 0U };
    dbgc_cmsis_dap_bounds_status_t status;

    connect_swd();
    mock_swd_calls = 0U;
    mock_final_response = DAP_TRANSFER_FAULT;
    status = dbgc_cmsis_dap_bounds_dispatch(
        request, sizeof(request), response, sizeof(response),
        &dispatch_profile, DAP_ExecuteCommand, &result);

    CHECK(status == DBGC_CMSIS_DAP_BOUNDS_OK);
    CHECK(result.request_bytes == sizeof(request));
    CHECK(result.response_bytes == 3U);
    CHECK(response[0] == ID_DAP_Transfer);
    CHECK(response[1] == 0U);
    CHECK(response[2] == DAP_TRANSFER_FAULT);
    CHECK(mock_swd_calls == 1U);
}

int main(void)
{
    test_firmware_version_info();
    test_unknown_info_identifier();
    test_unknown_command();
    test_swd_read_and_wait_retry();
    test_swd_write_and_completion_check();
    test_swd_posted_ap_reads();
    test_transfer_configure_retry_count();
    test_swd_transfer_block_posted_ap_reads();
    test_bounded_dispatch_rejects_before_upstream();
    test_bounded_dispatch_runs_upstream_after_preflight();
    test_bounded_dispatch_detects_length_contract_mismatch();
    test_bounded_dispatch_execute_commands();
    test_bounded_dispatch_transfer_error_consumes_full_request();

    if (failures != 0U) {
        fprintf(stderr, "%u CMSIS-DAP host checks failed\n", failures);
        return 1;
    }

    puts("CMSIS-DAP host checks passed: 8 command-core and 5 bounded-dispatch cases");
    return 0;
}
