#include "dbgc_cmsis_dap_bounds.h"

#include <limits.h>

#include "cmsis_compiler.h"
#include "DAP.h"

#define DBGC_CMSIS_DAP_PACKED_LENGTH_MAX ((size_t)UINT16_MAX)

typedef struct {
    const uint8_t *request;
    size_t length;
    size_t offset;
} dbgc_dap_reader_t;

static dbgc_cmsis_dap_bounds_status_t dbgc_dap_read_byte(
    dbgc_dap_reader_t *reader,
    uint8_t *value)
{
    if (reader->offset >= reader->length) {
        return DBGC_CMSIS_DAP_BOUNDS_NEED_MORE_INPUT;
    }

    *value = reader->request[reader->offset];
    ++reader->offset;
    return DBGC_CMSIS_DAP_BOUNDS_OK;
}

dbgc_cmsis_dap_bounds_status_t dbgc_cmsis_dap_bounds_dispatch(
    const uint8_t *request,
    size_t request_length,
    uint8_t *response,
    size_t response_capacity,
    const dbgc_cmsis_dap_bounds_profile_t *profile,
    dbgc_cmsis_dap_execute_fn execute,
    dbgc_cmsis_dap_dispatch_result_t *result)
{
    dbgc_cmsis_dap_bounds_result_t bounds;
    dbgc_cmsis_dap_bounds_status_t status;
    uint32_t packed_result;
    size_t consumed;
    size_t written;

    if (result != NULL) {
        result->request_bytes = 0U;
        result->response_bytes = 0U;
    }
    if ((response == NULL) || (execute == NULL) || (result == NULL)) {
        return DBGC_CMSIS_DAP_BOUNDS_INVALID_ARGUMENT;
    }

    status = dbgc_cmsis_dap_bounds_measure(request, request_length,
                                           response_capacity, profile,
                                           &bounds);
    if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
        return status;
    }

    packed_result = execute(request, response);
    consumed = (size_t)((packed_result >> 16) & UINT32_C(0xFFFF));
    written = (size_t)(packed_result & UINT32_C(0xFFFF));
    if ((consumed != bounds.request_bytes) ||
        (consumed > request_length) ||
        (written > bounds.response_bytes_max) ||
        (written > response_capacity)) {
        return DBGC_CMSIS_DAP_BOUNDS_DISPATCH_CONTRACT;
    }

    result->request_bytes = consumed;
    result->response_bytes = written;
    return DBGC_CMSIS_DAP_BOUNDS_OK;
}

static dbgc_cmsis_dap_bounds_status_t dbgc_dap_skip(
    dbgc_dap_reader_t *reader,
    size_t count)
{
    if (count > (reader->length - reader->offset)) {
        return DBGC_CMSIS_DAP_BOUNDS_NEED_MORE_INPUT;
    }

    reader->offset += count;
    return DBGC_CMSIS_DAP_BOUNDS_OK;
}

static dbgc_cmsis_dap_bounds_status_t dbgc_dap_add_length(
    size_t *length,
    size_t addition)
{
    if (addition > (DBGC_CMSIS_DAP_PACKED_LENGTH_MAX - *length)) {
        return DBGC_CMSIS_DAP_BOUNDS_LIMIT_EXCEEDED;
    }

    *length += addition;
    return DBGC_CMSIS_DAP_BOUNDS_OK;
}

static dbgc_cmsis_dap_bounds_status_t dbgc_dap_read_u16(
    dbgc_dap_reader_t *reader,
    uint16_t *value)
{
    uint8_t low;
    uint8_t high;
    dbgc_cmsis_dap_bounds_status_t status;

    status = dbgc_dap_read_byte(reader, &low);
    if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
        return status;
    }
    status = dbgc_dap_read_byte(reader, &high);
    if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
        return status;
    }

    *value = (uint16_t)((uint16_t)low | ((uint16_t)high << 8));
    return DBGC_CMSIS_DAP_BOUNDS_OK;
}

static size_t dbgc_dap_sequence_data_bytes(uint8_t sequence_info,
                                           uint8_t bit_mask)
{
    size_t bit_count = (size_t)(sequence_info & bit_mask);

    if (bit_count == 0U) {
        bit_count = 64U;
    }

    return (bit_count + 7U) / 8U;
}

static dbgc_cmsis_dap_bounds_status_t dbgc_dap_scan_transfer(
    dbgc_dap_reader_t *reader,
    const dbgc_cmsis_dap_bounds_profile_t *profile,
    size_t *response_bytes)
{
    uint8_t transfer_count;
    uint8_t transfer_info;
    uint8_t index;
    size_t response_per_transfer;
    dbgc_cmsis_dap_bounds_status_t status;

    status = dbgc_dap_skip(reader, 1U);
    if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
        return status;
    }
    status = dbgc_dap_read_byte(reader, &transfer_count);
    if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
        return status;
    }

    for (index = 0U; index < transfer_count; ++index) {
        status = dbgc_dap_read_byte(reader, &transfer_info);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }

        if (((transfer_info & DAP_TRANSFER_RnW) == 0U) ||
            ((transfer_info & DAP_TRANSFER_MATCH_VALUE) != 0U)) {
            status = dbgc_dap_skip(reader, 4U);
            if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
                return status;
            }
        }
    }

    response_per_transfer = profile->timestamp_enabled ? 8U : 4U;
    *response_bytes = 3U + ((size_t)transfer_count * response_per_transfer);
    if (*response_bytes > DBGC_CMSIS_DAP_PACKED_LENGTH_MAX) {
        return DBGC_CMSIS_DAP_BOUNDS_LIMIT_EXCEEDED;
    }

    return DBGC_CMSIS_DAP_BOUNDS_OK;
}

static dbgc_cmsis_dap_bounds_status_t dbgc_dap_scan_transfer_block(
    dbgc_dap_reader_t *reader,
    size_t *response_bytes)
{
    uint16_t transfer_count;
    uint8_t transfer_info;
    size_t data_bytes;
    dbgc_cmsis_dap_bounds_status_t status;

    status = dbgc_dap_skip(reader, 1U);
    if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
        return status;
    }
    status = dbgc_dap_read_u16(reader, &transfer_count);
    if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
        return status;
    }
    status = dbgc_dap_read_byte(reader, &transfer_info);
    if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
        return status;
    }

    data_bytes = ((size_t)transfer_count) * 4U;
    if ((transfer_info & DAP_TRANSFER_RnW) == 0U) {
        if (data_bytes > (DBGC_CMSIS_DAP_PACKED_LENGTH_MAX - 5U)) {
            return DBGC_CMSIS_DAP_BOUNDS_LIMIT_EXCEEDED;
        }
        status = dbgc_dap_skip(reader, data_bytes);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        if (data_bytes > (DBGC_CMSIS_DAP_PACKED_LENGTH_MAX - 4U)) {
            return DBGC_CMSIS_DAP_BOUNDS_LIMIT_EXCEEDED;
        }
        *response_bytes = 4U;
    } else {
        if (data_bytes > (DBGC_CMSIS_DAP_PACKED_LENGTH_MAX - 4U)) {
            return DBGC_CMSIS_DAP_BOUNDS_LIMIT_EXCEEDED;
        }
        *response_bytes = 4U + data_bytes;
    }

    return DBGC_CMSIS_DAP_BOUNDS_OK;
}

static dbgc_cmsis_dap_bounds_status_t dbgc_dap_scan_swd_sequence(
    dbgc_dap_reader_t *reader,
    const dbgc_cmsis_dap_bounds_profile_t *profile,
    size_t *response_bytes)
{
    uint8_t sequence_count;
    uint8_t sequence_info;
    uint8_t index;
    size_t data_bytes;
    dbgc_cmsis_dap_bounds_status_t status;

    status = dbgc_dap_read_byte(reader, &sequence_count);
    if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
        return status;
    }

    *response_bytes = 2U;
    for (index = 0U; index < sequence_count; ++index) {
        status = dbgc_dap_read_byte(reader, &sequence_info);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        data_bytes = dbgc_dap_sequence_data_bytes(sequence_info,
                                                  SWD_SEQUENCE_CLK);
        if ((sequence_info & SWD_SEQUENCE_DIN) == 0U) {
            status = dbgc_dap_skip(reader, data_bytes);
            if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
                return status;
            }
        } else if (profile->swd_enabled != 0U) {
            status = dbgc_dap_add_length(response_bytes, data_bytes);
            if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
                return status;
            }
        }
    }

    return DBGC_CMSIS_DAP_BOUNDS_OK;
}

static dbgc_cmsis_dap_bounds_status_t dbgc_dap_scan_jtag_sequence(
    dbgc_dap_reader_t *reader,
    const dbgc_cmsis_dap_bounds_profile_t *profile,
    size_t *response_bytes)
{
    uint8_t sequence_count;
    uint8_t sequence_info;
    uint8_t index;
    size_t data_bytes;
    dbgc_cmsis_dap_bounds_status_t status;

    status = dbgc_dap_read_byte(reader, &sequence_count);
    if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
        return status;
    }

    *response_bytes = 2U;
    for (index = 0U; index < sequence_count; ++index) {
        status = dbgc_dap_read_byte(reader, &sequence_info);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        data_bytes = dbgc_dap_sequence_data_bytes(sequence_info,
                                                  JTAG_SEQUENCE_TCK);
        status = dbgc_dap_skip(reader, data_bytes);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        if ((profile->jtag_enabled != 0U) &&
            ((sequence_info & JTAG_SEQUENCE_TDO) != 0U)) {
            status = dbgc_dap_add_length(response_bytes, data_bytes);
            if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
                return status;
            }
        }
    }

    return DBGC_CMSIS_DAP_BOUNDS_OK;
}

static dbgc_cmsis_dap_bounds_status_t dbgc_dap_scan_command(
    dbgc_dap_reader_t *reader,
    const dbgc_cmsis_dap_bounds_profile_t *profile,
    uint8_t nested,
    size_t *response_bytes)
{
    uint8_t command;
    uint8_t value;
    uint8_t index;
    size_t data_bytes;
    size_t ignored_response;
    dbgc_cmsis_dap_bounds_status_t status;

    status = dbgc_dap_read_byte(reader, &command);
    if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
        return status;
    }

    if ((command >= ID_DAP_Vendor0) && (command <= ID_DAP_Vendor31)) {
        return DBGC_CMSIS_DAP_BOUNDS_UNSUPPORTED_COMMAND;
    }

    if ((nested != 0U) && (command == ID_DAP_ExecuteCommands)) {
        *response_bytes = 1U;
        return DBGC_CMSIS_DAP_BOUNDS_OK;
    }

    status = DBGC_CMSIS_DAP_BOUNDS_OK;
    switch (command) {
    case ID_DAP_Info:
        status = dbgc_dap_skip(reader, 1U);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        if (profile->max_info_payload_bytes == 0U) {
            return DBGC_CMSIS_DAP_BOUNDS_UNSUPPORTED_COMMAND;
        }
        if (profile->max_info_payload_bytes >
            (DBGC_CMSIS_DAP_PACKED_LENGTH_MAX - 2U)) {
            return DBGC_CMSIS_DAP_BOUNDS_LIMIT_EXCEEDED;
        }
        *response_bytes = 2U + profile->max_info_payload_bytes;
        break;
    case ID_DAP_HostStatus:
        status = dbgc_dap_skip(reader, 2U);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        *response_bytes = 2U;
        break;
    case ID_DAP_Connect:
    case ID_DAP_SWD_Configure:
        status = dbgc_dap_skip(reader, 1U);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        *response_bytes = 2U;
        break;
    case ID_DAP_Disconnect:
        *response_bytes = 2U;
        break;
    case ID_DAP_TransferConfigure:
        status = dbgc_dap_skip(reader, 5U);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        *response_bytes = 2U;
        break;
    case ID_DAP_Transfer:
        status = dbgc_dap_scan_transfer(reader, profile, response_bytes);
        break;
    case ID_DAP_TransferBlock:
        status = dbgc_dap_scan_transfer_block(reader, response_bytes);
        break;
    case ID_DAP_WriteABORT:
        status = dbgc_dap_skip(reader, 5U);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        *response_bytes = 2U;
        break;
    case ID_DAP_Delay:
        status = dbgc_dap_skip(reader, 2U);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        *response_bytes = 2U;
        break;
    case ID_DAP_ResetTarget:
        *response_bytes = 3U;
        break;
    case ID_DAP_SWJ_Pins:
        status = dbgc_dap_skip(reader, 6U);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        *response_bytes = 2U;
        break;
    case ID_DAP_SWJ_Clock:
        status = dbgc_dap_skip(reader, 4U);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        *response_bytes = 2U;
        break;
    case ID_DAP_SWJ_Sequence:
        status = dbgc_dap_read_byte(reader, &value);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        data_bytes = (value == 0U) ? 32U : (((size_t)value + 7U) / 8U);
        status = dbgc_dap_skip(reader, data_bytes);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        *response_bytes = 2U;
        break;
    case ID_DAP_SWD_Sequence:
        status = dbgc_dap_scan_swd_sequence(reader, profile, response_bytes);
        break;
    case ID_DAP_JTAG_Sequence:
        status = dbgc_dap_scan_jtag_sequence(reader, profile, response_bytes);
        break;
    case ID_DAP_JTAG_Configure:
        status = dbgc_dap_read_byte(reader, &value);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        if ((profile->jtag_enabled != 0U) &&
            (value > profile->max_jtag_devices)) {
            return DBGC_CMSIS_DAP_BOUNDS_LIMIT_EXCEEDED;
        }
        status = dbgc_dap_skip(reader, (size_t)value);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        *response_bytes = 2U;
        break;
    case ID_DAP_JTAG_IDCODE:
        status = dbgc_dap_skip(reader, 1U);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        *response_bytes = (profile->jtag_enabled != 0U) ? 6U : 2U;
        break;
    case ID_DAP_ExecuteCommands:
        if (nested != 0U) {
            *response_bytes = 1U;
            break;
        }
        status = dbgc_dap_read_byte(reader, &value);
        if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
            return status;
        }
        *response_bytes = 2U;
        for (index = 0U; index < value; ++index) {
            status = dbgc_dap_scan_command(reader, profile, 1U,
                                           &ignored_response);
            if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
                return status;
            }
            status = dbgc_dap_add_length(response_bytes, ignored_response);
            if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
                return status;
            }
        }
        break;
    default:
        if ((command >= ID_DAP_SWO_Transport) &&
            (command <= ID_DAP_SWO_ExtendedStatus)) {
            return DBGC_CMSIS_DAP_BOUNDS_UNSUPPORTED_COMMAND;
        }
        if ((command >= ID_DAP_UART_Transport) &&
            (command <= ID_DAP_UART_Status)) {
            return DBGC_CMSIS_DAP_BOUNDS_UNSUPPORTED_COMMAND;
        }
        *response_bytes = 1U;
        break;
    }

    return status;
}

dbgc_cmsis_dap_bounds_status_t dbgc_cmsis_dap_bounds_measure(
    const uint8_t *request,
    size_t request_length,
    size_t response_capacity,
    const dbgc_cmsis_dap_bounds_profile_t *profile,
    dbgc_cmsis_dap_bounds_result_t *result)
{
    dbgc_dap_reader_t reader;
    dbgc_cmsis_dap_bounds_status_t status;
    size_t response_bytes;

    if (result != NULL) {
        result->request_bytes = 0U;
        result->response_bytes_max = 0U;
    }
    if ((request == NULL) || (profile == NULL) || (result == NULL)) {
        return DBGC_CMSIS_DAP_BOUNDS_INVALID_ARGUMENT;
    }

    reader.request = request;
    reader.length = request_length;
    reader.offset = 0U;

    status = dbgc_dap_scan_command(&reader, profile, 0U, &response_bytes);
    if (status != DBGC_CMSIS_DAP_BOUNDS_OK) {
        return status;
    }
    if (reader.offset > DBGC_CMSIS_DAP_PACKED_LENGTH_MAX) {
        return DBGC_CMSIS_DAP_BOUNDS_LIMIT_EXCEEDED;
    }

    result->request_bytes = reader.offset;
    result->response_bytes_max = response_bytes;
    if (response_bytes > response_capacity) {
        return DBGC_CMSIS_DAP_BOUNDS_OUTPUT_TOO_SMALL;
    }

    return DBGC_CMSIS_DAP_BOUNDS_OK;
}
