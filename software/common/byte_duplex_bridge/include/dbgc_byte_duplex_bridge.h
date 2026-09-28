#ifndef DBGC_BYTE_DUPLEX_BRIDGE_H
#define DBGC_BYTE_DUPLEX_BRIDGE_H

#include <stddef.h>

#include "dbgc_byte_stream_bridge.h"

typedef struct {
    dbgc_byte_stream_bridge_channel_t first_to_second;
    dbgc_byte_stream_bridge_channel_t second_to_first;
} dbgc_byte_duplex_bridge_t;

/* Initialize both directions using caller-owned FIFOs and endpoint callbacks.
 * The bridge, FIFO objects, and FIFO backing storage must be separate memory
 * objects; the two directions must use distinct FIFO objects. This module
 * does not allocate memory, select endpoints, or provide synchronization.
 */
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
    void *first_write_context);

/* Service each direction with its own caller-selected byte budget. On an
 * endpoint/FIFO error, the function returns immediately; the later direction
 * is not serviced. Each transferred output is zeroed before validation.
 */
dbgc_byte_stream_bridge_status_t dbgc_byte_duplex_bridge_service(
    dbgc_byte_duplex_bridge_t *bridge,
    size_t first_to_second_budget,
    size_t second_to_first_budget,
    size_t *first_to_second_transferred,
    size_t *second_to_first_transferred);

#endif
