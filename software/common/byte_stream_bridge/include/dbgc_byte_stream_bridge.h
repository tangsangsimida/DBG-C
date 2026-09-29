#ifndef DBGC_BYTE_STREAM_BRIDGE_H
#define DBGC_BYTE_STREAM_BRIDGE_H

#include <stddef.h>
#include <stdint.h>

#include "dbgc_byte_fifo.h"

typedef enum {
	DBGC_BYTE_STREAM_BRIDGE_OK = 0,
	DBGC_BYTE_STREAM_BRIDGE_INVALID_ARGUMENT,
	DBGC_BYTE_STREAM_BRIDGE_ENDPOINT_ERROR,
	DBGC_BYTE_STREAM_BRIDGE_FIFO_ERROR
} dbgc_byte_stream_bridge_status_t;

/* Endpoint callbacks return 1 when one byte moves, 0 when not ready, and
 * -1 on endpoint error. The bridge calls them only from service().
 */
typedef int (*dbgc_byte_stream_bridge_read_fn)(void *context, uint8_t *byte);
typedef int (*dbgc_byte_stream_bridge_write_fn)(void *context, uint8_t byte);

/* One serialized, nonblocking byte-stream direction. The caller owns the FIFO
 * and must not access it or this channel concurrently with service().
 */
typedef struct {
	dbgc_byte_fifo_t *fifo;
	dbgc_byte_stream_bridge_read_fn read;
	dbgc_byte_stream_bridge_write_fn write;
	void *read_context;
	void *write_context;
	uint8_t pending_byte;
	uint8_t pending_valid;
} dbgc_byte_stream_bridge_channel_t;

/* Initialize once for a channel lifetime around an already initialized FIFO.
 * Initialization does not clear the FIFO or take ownership of its storage;
 * reinitialization resets and discards any pending byte held by the channel.
 */
dbgc_byte_stream_bridge_status_t
dbgc_byte_stream_bridge_initialize(dbgc_byte_stream_bridge_channel_t *channel,
				   dbgc_byte_fifo_t *fifo, dbgc_byte_stream_bridge_read_fn read,
				   void *read_context, dbgc_byte_stream_bridge_write_fn write,
				   void *write_context);

/* Move up to byte_budget bytes to the write endpoint. Data is first read from
 * the FIFO; when empty, the read endpoint is polled and bytes are buffered in
 * the FIFO before delivery. A byte removed from the FIFO is retained in the
 * channel if the write endpoint applies backpressure or reports an error.
 * transferred is optional and reports bytes accepted by the write endpoint.
 */
dbgc_byte_stream_bridge_status_t
dbgc_byte_stream_bridge_service(dbgc_byte_stream_bridge_channel_t *channel, size_t byte_budget,
				size_t *transferred);

#endif
