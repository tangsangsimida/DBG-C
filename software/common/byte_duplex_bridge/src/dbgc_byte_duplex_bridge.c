#include "dbgc_byte_duplex_bridge.h"

dbgc_byte_stream_bridge_status_t dbgc_byte_duplex_bridge_initialize(
    dbgc_byte_duplex_bridge_t *bridge,
    dbgc_byte_fifo_t *first_to_second_fifo,
    dbgc_byte_stream_bridge_read_fn first_read,
    void *first_read_context,
    dbgc_byte_stream_bridge_write_fn second_write,
    void *second_write_context,
    dbgc_byte_fifo_t *second_to_first_fifo,
    dbgc_byte_stream_bridge_read_fn second_read,
    void *second_read_context,
    dbgc_byte_stream_bridge_write_fn first_write,
    void *first_write_context)
{
    dbgc_byte_stream_bridge_status_t status;

    if ((bridge == 0) || (first_to_second_fifo == 0) ||
        (second_to_first_fifo == 0) ||
        (first_to_second_fifo == second_to_first_fifo) ||
        (first_read == 0) || (second_write == 0) || (second_read == 0) ||
        (first_write == 0) ||
        (dbgc_byte_fifo_capacity(first_to_second_fifo) == 0U) ||
        (dbgc_byte_fifo_capacity(second_to_first_fifo) == 0U)) {
        return DBGC_BYTE_STREAM_BRIDGE_INVALID_ARGUMENT;
    }

    status = dbgc_byte_stream_bridge_initialize(
        &bridge->first_to_second, first_to_second_fifo, first_read,
        first_read_context, second_write, second_write_context);
    if (status != DBGC_BYTE_STREAM_BRIDGE_OK) {
        return status;
    }

    return dbgc_byte_stream_bridge_initialize(
        &bridge->second_to_first, second_to_first_fifo, second_read,
        second_read_context, first_write, first_write_context);
}

dbgc_byte_stream_bridge_status_t dbgc_byte_duplex_bridge_service(
    dbgc_byte_duplex_bridge_t *bridge,
    size_t first_to_second_budget,
    size_t second_to_first_budget,
    size_t *first_to_second_transferred,
    size_t *second_to_first_transferred)
{
    dbgc_byte_stream_bridge_status_t status;

    if (first_to_second_transferred != 0) {
        *first_to_second_transferred = 0U;
    }
    if (second_to_first_transferred != 0) {
        *second_to_first_transferred = 0U;
    }
    if (bridge == 0) {
        return DBGC_BYTE_STREAM_BRIDGE_INVALID_ARGUMENT;
    }

    status = dbgc_byte_stream_bridge_service(
        &bridge->first_to_second, first_to_second_budget,
        first_to_second_transferred);
    if (status != DBGC_BYTE_STREAM_BRIDGE_OK) {
        return status;
    }

    return dbgc_byte_stream_bridge_service(
        &bridge->second_to_first, second_to_first_budget,
        second_to_first_transferred);
}
