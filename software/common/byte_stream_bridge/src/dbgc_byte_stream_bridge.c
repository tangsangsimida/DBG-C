#include "dbgc_byte_stream_bridge.h"

dbgc_byte_stream_bridge_status_t dbgc_byte_stream_bridge_initialize(
    dbgc_byte_stream_bridge_channel_t *channel,
    dbgc_byte_fifo_t *fifo,
    dbgc_byte_stream_bridge_read_fn read,
    void *read_context,
    dbgc_byte_stream_bridge_write_fn write,
    void *write_context)
{
    if ((channel == 0) || (fifo == 0) || (read == 0) || (write == 0) ||
        (dbgc_byte_fifo_capacity(fifo) == 0U)) {
        return DBGC_BYTE_STREAM_BRIDGE_INVALID_ARGUMENT;
    }

    channel->fifo = fifo;
    channel->read = read;
    channel->write = write;
    channel->read_context = read_context;
    channel->write_context = write_context;
    channel->pending_byte = 0U;
    channel->pending_valid = 0U;
    return DBGC_BYTE_STREAM_BRIDGE_OK;
}

dbgc_byte_stream_bridge_status_t dbgc_byte_stream_bridge_service(
    dbgc_byte_stream_bridge_channel_t *channel,
    size_t byte_budget,
    size_t *transferred)
{
    size_t moved = 0U;
    size_t received = 0U;
    uint8_t byte;
    uint8_t write_blocked = 0U;
    int result;

    if (transferred != 0) {
        *transferred = 0U;
    }
    if ((channel == 0) || (channel->fifo == 0) || (channel->read == 0) ||
        (channel->write == 0)) {
        return DBGC_BYTE_STREAM_BRIDGE_INVALID_ARGUMENT;
    }

    while (moved < byte_budget) {
        if (channel->pending_valid != 0U) {
            result = channel->write(channel->write_context,
                                    channel->pending_byte);
            if (result == 0) {
                write_blocked = 1U;
                break;
            }
            if (result != 1) {
                if (transferred != 0) {
                    *transferred = moved;
                }
                return DBGC_BYTE_STREAM_BRIDGE_ENDPOINT_ERROR;
            }

            channel->pending_valid = 0U;
            ++moved;
            continue;
        }

        if (dbgc_byte_fifo_read(channel->fifo, &byte, 1U) == 1U) {
            channel->pending_byte = byte;
            channel->pending_valid = 1U;
            continue;
        }

        break;
    }

    while (received < byte_budget &&
           dbgc_byte_fifo_space(channel->fifo) != 0U) {
        result = channel->read(channel->read_context, &byte);
        if (result == 0) {
            break;
        }
        if (result != 1) {
            if (transferred != 0) {
                *transferred = moved;
            }
            return DBGC_BYTE_STREAM_BRIDGE_ENDPOINT_ERROR;
        }

        if (dbgc_byte_fifo_write(channel->fifo, &byte, 1U) != 1U) {
            if (transferred != 0) {
                *transferred = moved;
            }
            return DBGC_BYTE_STREAM_BRIDGE_FIFO_ERROR;
        }
        ++received;
    }

    while ((write_blocked == 0U) && (moved < byte_budget)) {
        if (channel->pending_valid == 0U) {
            if (dbgc_byte_fifo_read(channel->fifo, &byte, 1U) != 1U) {
                break;
            }
            channel->pending_byte = byte;
            channel->pending_valid = 1U;
        }

        result = channel->write(channel->write_context,
                                channel->pending_byte);
        if (result == 0) {
            break;
        }
        if (result != 1) {
            if (transferred != 0) {
                *transferred = moved;
            }
            return DBGC_BYTE_STREAM_BRIDGE_ENDPOINT_ERROR;
        }

        channel->pending_valid = 0U;
        ++moved;
    }

    if (transferred != 0) {
        *transferred = moved;
    }
    return DBGC_BYTE_STREAM_BRIDGE_OK;
}
