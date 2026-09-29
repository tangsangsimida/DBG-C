#include "dbgc_cmsis_dap_bounds.h"

#include <stdio.h>
#include <string.h>

#include "DAP.h"

static unsigned int check_count;

static int check_status(const char *name, dbgc_cmsis_dap_bounds_status_t actual,
			dbgc_cmsis_dap_bounds_status_t expected)
{
	++check_count;
	if (actual != expected) {
		(void)fprintf(stderr, "%s: status %u, expected %u\n", name, (unsigned int)actual,
			      (unsigned int)expected);
		return 1;
	}
	return 0;
}

static int check_size(const char *name, size_t actual, size_t expected)
{
	++check_count;
	if (actual != expected) {
		(void)fprintf(stderr, "%s: size %zu, expected %zu\n", name, actual, expected);
		return 1;
	}
	return 0;
}

static int run_case(const char *name, const uint8_t *request, size_t request_length,
		    size_t response_capacity, const dbgc_cmsis_dap_bounds_profile_t *profile,
		    dbgc_cmsis_dap_bounds_status_t expected_status, size_t expected_request_bytes,
		    size_t expected_response_bytes)
{
	dbgc_cmsis_dap_bounds_result_t result;
	dbgc_cmsis_dap_bounds_status_t status;
	int failures = 0;

	result.request_bytes = 0U;
	result.response_bytes_max = 0U;
	status = dbgc_cmsis_dap_bounds_measure(request, request_length, response_capacity, profile,
					       &result);
	failures += check_status(name, status, expected_status);
	failures += check_size(name, result.request_bytes, expected_request_bytes);
	failures += check_size(name, result.response_bytes_max, expected_response_bytes);
	return failures;
}

int main(void)
{
	static const dbgc_cmsis_dap_bounds_profile_t profile = { 1U, 0U, 0U, 0U, 64U };
	static const uint8_t connect_request[] = { ID_DAP_Connect, DAP_PORT_SWD };
	static const uint8_t host_status_request[] = { ID_DAP_HostStatus, DAP_DEBUGGER_CONNECTED,
						       1U };
	static const uint8_t disconnect_request[] = { ID_DAP_Disconnect };
	static const uint8_t transfer_configure_request[] = {
		ID_DAP_TransferConfigure, 0U, 1U, 0U, 2U, 0U
	};
	static const uint8_t write_abort_request[] = { ID_DAP_WriteABORT, 0U, 1U, 2U, 3U, 4U };
	static const uint8_t delay_request[] = { ID_DAP_Delay, 1U, 0U };
	static const uint8_t reset_target_request[] = { ID_DAP_ResetTarget };
	static const uint8_t swj_pins_request[] = { ID_DAP_SWJ_Pins, 0U, 0U, 0U, 0U, 0U, 0U };
	static uint8_t swj_sequence_256_request[34] = { ID_DAP_SWJ_Sequence, 0U };
	static const uint8_t swj_sequence_truncated_request[] = { ID_DAP_SWJ_Sequence, 0U };
	static const uint8_t swj_clock_request[] = { ID_DAP_SWJ_Clock, 1U, 0U, 0U, 0U };
	static const uint8_t swd_sequence_request[] = {
		ID_DAP_SWD_Sequence, 2U, 0x80U, 0x02U, 0xA5U, 0x5AU
	};
	static const uint8_t transfer_request[] = {
		ID_DAP_Transfer, 0U, 2U, DAP_TRANSFER_RnW, 0U, 0x11U, 0x22U, 0x33U, 0x44U
	};
	static const uint8_t transfer_match_request[] = {
		ID_DAP_Transfer, 0U, 1U, DAP_TRANSFER_RnW | DAP_TRANSFER_MATCH_VALUE, 0U, 0U, 0U, 0U
	};
	static const uint8_t transfer_block_read_request[] = { ID_DAP_TransferBlock, 0U, 2U, 0U,
							       DAP_TRANSFER_RnW };
	static const uint8_t transfer_block_write_request[] = {
		ID_DAP_TransferBlock, 0U, 2U, 0U, 0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U
	};
	static const uint8_t execute_commands_request[] = {
		ID_DAP_ExecuteCommands, 4U, ID_DAP_Connect,	    DAP_PORT_SWD, ID_DAP_HostStatus,
		DAP_DEBUGGER_CONNECTED, 1U, ID_DAP_ExecuteCommands, 0x50U,	  0xEEU
	};
	static const uint8_t vendor_request[] = { ID_DAP_Vendor0 };
	static const uint8_t info_request[] = { ID_DAP_Info, DAP_ID_VENDOR };
	static const uint8_t unsupported_swo_request[] = { ID_DAP_SWO_Transport, 0U };
	static const uint8_t unsupported_uart_request[] = { ID_DAP_UART_Configure };
	static const uint8_t truncated_execute_request[] = { ID_DAP_ExecuteCommands, 1U };
	static const uint8_t transfer_block_oversize_request[] = { ID_DAP_TransferBlock, 0U, 0xFFU,
								   0x3FU, DAP_TRANSFER_RnW };
	static const uint8_t transfer_block_oversize_write_request[] = { ID_DAP_TransferBlock, 0U,
									 0xFFU, 0x3FU, 0U };
	static const uint8_t jtag_sequence_request[] = { ID_DAP_JTAG_Sequence, 1U,
							 JTAG_SEQUENCE_TMS | JTAG_SEQUENCE_TDO | 3U,
							 0x05U };
	static const uint8_t jtag_configure_request[] = { ID_DAP_JTAG_Configure, 2U, 5U, 7U };
	static const uint8_t jtag_idcode_request[] = { ID_DAP_JTAG_IDCODE, 0U };
	static const uint8_t unknown_request[] = { 0x50U };
	dbgc_cmsis_dap_bounds_profile_t timestamp_profile;
	dbgc_cmsis_dap_bounds_profile_t unbounded_info_profile;
	dbgc_cmsis_dap_bounds_profile_t jtag_profile;
	dbgc_cmsis_dap_bounds_result_t result;
	dbgc_cmsis_dap_bounds_status_t status;
	int failures = 0;

	failures += run_case("connect", connect_request, sizeof(connect_request), 2U, &profile,
			     DBGC_CMSIS_DAP_BOUNDS_OK, 2U, 2U);
	failures += run_case("host status", host_status_request, sizeof(host_status_request), 2U,
			     &profile, DBGC_CMSIS_DAP_BOUNDS_OK, 3U, 2U);
	failures += run_case("disconnect", disconnect_request, sizeof(disconnect_request), 2U,
			     &profile, DBGC_CMSIS_DAP_BOUNDS_OK, 1U, 2U);
	failures += run_case("transfer configure", transfer_configure_request,
			     sizeof(transfer_configure_request), 2U, &profile,
			     DBGC_CMSIS_DAP_BOUNDS_OK, 6U, 2U);
	failures += run_case("write abort", write_abort_request, sizeof(write_abort_request), 2U,
			     &profile, DBGC_CMSIS_DAP_BOUNDS_OK, 6U, 2U);
	failures += run_case("delay", delay_request, sizeof(delay_request), 2U, &profile,
			     DBGC_CMSIS_DAP_BOUNDS_OK, 3U, 2U);
	failures += run_case("target reset response", reset_target_request,
			     sizeof(reset_target_request), 3U, &profile, DBGC_CMSIS_DAP_BOUNDS_OK,
			     1U, 3U);
	failures += run_case("fixed command response capacity", connect_request,
			     sizeof(connect_request), 1U, &profile,
			     DBGC_CMSIS_DAP_BOUNDS_OUTPUT_TOO_SMALL, 2U, 2U);
	failures += run_case("truncated fixed command", swj_pins_request,
			     sizeof(swj_pins_request) - 1U, sizeof(swj_pins_request), &profile,
			     DBGC_CMSIS_DAP_BOUNDS_NEED_MORE_INPUT, 0U, 0U);
	failures += run_case("SWJ sequence zero means 256 bits", swj_sequence_truncated_request,
			     sizeof(swj_sequence_truncated_request), 2U, &profile,
			     DBGC_CMSIS_DAP_BOUNDS_NEED_MORE_INPUT, 0U, 0U);
	{
		failures +=
			run_case("SWJ sequence 256 bits payload", swj_sequence_256_request,
				 sizeof(swj_sequence_256_request), 2U, &profile,
				 DBGC_CMSIS_DAP_BOUNDS_OK, sizeof(swj_sequence_256_request), 2U);
	}
	failures += run_case("SWJ clock", swj_clock_request, sizeof(swj_clock_request), 2U,
			     &profile, DBGC_CMSIS_DAP_BOUNDS_OK, 5U, 2U);
	failures += run_case("SWD sequence mixed input and output", swd_sequence_request,
			     sizeof(swd_sequence_request), 10U, &profile, DBGC_CMSIS_DAP_BOUNDS_OK,
			     5U, 10U);
	failures += run_case("transfer read and write payloads", transfer_request,
			     sizeof(transfer_request), 11U, &profile, DBGC_CMSIS_DAP_BOUNDS_OK, 9U,
			     11U);
	timestamp_profile = profile;
	timestamp_profile.timestamp_enabled = 1U;
	failures += run_case("timestamp transfer upper bound", transfer_request,
			     sizeof(transfer_request), 19U, &timestamp_profile,
			     DBGC_CMSIS_DAP_BOUNDS_OK, 9U, 19U);
	failures += run_case("transfer match value payload", transfer_match_request,
			     sizeof(transfer_match_request), 7U, &profile, DBGC_CMSIS_DAP_BOUNDS_OK,
			     8U, 7U);
	failures += run_case("transfer block read", transfer_block_read_request,
			     sizeof(transfer_block_read_request), 12U, &profile,
			     DBGC_CMSIS_DAP_BOUNDS_OK, 5U, 12U);
	failures += run_case("transfer block write", transfer_block_write_request,
			     sizeof(transfer_block_write_request), 4U, &profile,
			     DBGC_CMSIS_DAP_BOUNDS_OK, 13U, 4U);
	failures += run_case("execute commands and nested execute command",
			     execute_commands_request, sizeof(execute_commands_request), 8U,
			     &profile, DBGC_CMSIS_DAP_BOUNDS_OK, 9U, 8U);
	failures += run_case("vendor command fails closed", vendor_request, sizeof(vendor_request),
			     8U, &profile, DBGC_CMSIS_DAP_BOUNDS_UNSUPPORTED_COMMAND, 0U, 0U);
	failures += run_case("Info callback bound", info_request, sizeof(info_request), 66U,
			     &profile, DBGC_CMSIS_DAP_BOUNDS_OK, 2U, 66U);
	unbounded_info_profile = profile;
	unbounded_info_profile.max_info_payload_bytes = 0U;
	failures += run_case("Info without callback bound fails closed", info_request,
			     sizeof(info_request), 66U, &unbounded_info_profile,
			     DBGC_CMSIS_DAP_BOUNDS_UNSUPPORTED_COMMAND, 0U, 0U);
	failures += run_case("SWO command fails closed", unsupported_swo_request,
			     sizeof(unsupported_swo_request), 8U, &profile,
			     DBGC_CMSIS_DAP_BOUNDS_UNSUPPORTED_COMMAND, 0U, 0U);
	failures += run_case("CMSIS-DAP UART command fails closed", unsupported_uart_request,
			     sizeof(unsupported_uart_request), 8U, &profile,
			     DBGC_CMSIS_DAP_BOUNDS_UNSUPPORTED_COMMAND, 0U, 0U);
	failures += run_case("execute command child truncation", truncated_execute_request,
			     sizeof(truncated_execute_request), 8U, &profile,
			     DBGC_CMSIS_DAP_BOUNDS_NEED_MORE_INPUT, 0U, 0U);
	failures += run_case("TransferBlock packed response limit", transfer_block_oversize_request,
			     sizeof(transfer_block_oversize_request), 65535U, &profile,
			     DBGC_CMSIS_DAP_BOUNDS_LIMIT_EXCEEDED, 0U, 0U);
	failures += run_case("TransferBlock packed request limit",
			     transfer_block_oversize_write_request,
			     sizeof(transfer_block_oversize_write_request), 65535U, &profile,
			     DBGC_CMSIS_DAP_BOUNDS_LIMIT_EXCEEDED, 0U, 0U);

	jtag_profile = profile;
	jtag_profile.jtag_enabled = 1U;
	jtag_profile.max_jtag_devices = 2U;
	failures += run_case("JTAG sequence data and TDO output", jtag_sequence_request,
			     sizeof(jtag_sequence_request), 3U, &jtag_profile,
			     DBGC_CMSIS_DAP_BOUNDS_OK, 4U, 3U);
	failures += run_case("JTAG sequence disabled response", jtag_sequence_request,
			     sizeof(jtag_sequence_request), 2U, &profile, DBGC_CMSIS_DAP_BOUNDS_OK,
			     4U, 2U);
	failures += run_case("JTAG configure bounded device count", jtag_configure_request,
			     sizeof(jtag_configure_request), 2U, &jtag_profile,
			     DBGC_CMSIS_DAP_BOUNDS_OK, 4U, 2U);
	jtag_profile.max_jtag_devices = 1U;
	failures += run_case("JTAG configure limit", jtag_configure_request,
			     sizeof(jtag_configure_request), 2U, &jtag_profile,
			     DBGC_CMSIS_DAP_BOUNDS_LIMIT_EXCEEDED, 0U, 0U);
	failures += run_case("JTAG IDCODE maximum response", jtag_idcode_request,
			     sizeof(jtag_idcode_request), 6U, &jtag_profile,
			     DBGC_CMSIS_DAP_BOUNDS_OK, 2U, 6U);
	failures +=
		run_case("unknown command maps to bounded invalid response", unknown_request,
			 sizeof(unknown_request), 1U, &profile, DBGC_CMSIS_DAP_BOUNDS_OK, 1U, 1U);

	result.request_bytes = 123U;
	result.response_bytes_max = 456U;
	status = dbgc_cmsis_dap_bounds_measure(NULL, 0U, 0U, &profile, &result);
	failures += check_status("null request", status, DBGC_CMSIS_DAP_BOUNDS_INVALID_ARGUMENT);
	failures += check_size("null request clears request result", result.request_bytes, 0U);
	failures +=
		check_size("null request clears response result", result.response_bytes_max, 0U);
	if (failures != 0) {
		return 1;
	}
	(void)printf("CMSIS-DAP bounds checks passed: %u checks\n", check_count);
	return 0;
}
