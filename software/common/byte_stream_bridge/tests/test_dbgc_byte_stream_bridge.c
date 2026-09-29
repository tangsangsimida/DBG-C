#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbgc_byte_stream_bridge.h"

typedef struct {
	const uint8_t *input;
	size_t input_length;
	size_t input_offset;
	uint8_t output[16];
	size_t output_length;
	int read_result;
	int write_result;
} endpoint_t;

static unsigned int checks;

static void check(int condition, const char *name)
{
	++checks;
	if (!condition) {
		(void)fprintf(stderr, "FAIL: %s\n", name);
		exit(EXIT_FAILURE);
	}
}

static int endpoint_read(void *context, uint8_t *byte)
{
	endpoint_t *endpoint = (endpoint_t *)context;

	if (endpoint->read_result != 1) {
		return endpoint->read_result;
	}
	if (endpoint->input_offset >= endpoint->input_length) {
		return 0;
	}

	*byte = endpoint->input[endpoint->input_offset];
	++endpoint->input_offset;
	return 1;
}

static int endpoint_write(void *context, uint8_t byte)
{
	endpoint_t *endpoint = (endpoint_t *)context;

	if (endpoint->write_result != 1) {
		return endpoint->write_result;
	}
	if (endpoint->output_length >= sizeof(endpoint->output)) {
		return -1;
	}

	endpoint->output[endpoint->output_length] = byte;
	++endpoint->output_length;
	return 1;
}

static void initialize_endpoint(endpoint_t *endpoint, const uint8_t *input, size_t input_length)
{
	(void)memset(endpoint, 0, sizeof(*endpoint));
	endpoint->input = input;
	endpoint->input_length = input_length;
	endpoint->read_result = 1;
	endpoint->write_result = 1;
}

int main(void)
{
	uint8_t storage[4];
	dbgc_byte_fifo_t fifo;
	dbgc_byte_stream_bridge_channel_t channel;
	endpoint_t source;
	endpoint_t sink;
	size_t transferred;
	static const uint8_t payload[] = { 0x12U, 0x34U, 0x56U, 0x78U };

	check(dbgc_byte_fifo_initialize(&fifo, storage, sizeof(storage)) == 0,
	      "FIFO initialization succeeds");
	initialize_endpoint(&source, payload, sizeof(payload));
	initialize_endpoint(&sink, 0, 0U);
	check(dbgc_byte_stream_bridge_initialize(&channel, &fifo, endpoint_read, &source,
						 endpoint_write,
						 &sink) == DBGC_BYTE_STREAM_BRIDGE_OK,
	      "channel initialization succeeds");

	check((dbgc_byte_stream_bridge_service(&channel, 4U, &transferred) ==
	       DBGC_BYTE_STREAM_BRIDGE_OK) &&
		      (transferred == 4U),
	      "bounded service forwards available source bytes");
	check((sink.output_length == sizeof(payload)) &&
		      (memcmp(sink.output, payload, sizeof(payload)) == 0),
	      "forwarded bytes retain source order");
	check((dbgc_byte_fifo_count(&fifo) == 0U) && (channel.pending_valid == 0U),
	      "successful forwarding drains FIFO and pending byte");

	initialize_endpoint(&source, payload, sizeof(payload));
	initialize_endpoint(&sink, 0, 0U);
	sink.write_result = 0;
	check((dbgc_byte_stream_bridge_initialize(&channel, &fifo, endpoint_read, &source,
						  endpoint_write,
						  &sink) == DBGC_BYTE_STREAM_BRIDGE_OK) &&
		      (dbgc_byte_stream_bridge_service(&channel, 8U, &transferred) ==
		       DBGC_BYTE_STREAM_BRIDGE_OK) &&
		      (transferred == 0U) && (channel.pending_valid == 1U) &&
		      (dbgc_byte_fifo_count(&fifo) == 3U) &&
		      (source.input_offset == sizeof(payload)),
	      "backpressure retains pending data and buffers source bytes");
	sink.write_result = 1;
	check((dbgc_byte_stream_bridge_service(&channel, 1U, &transferred) ==
	       DBGC_BYTE_STREAM_BRIDGE_OK) &&
		      (transferred == 1U) && (sink.output[0] == payload[0]) &&
		      (channel.pending_valid == 0U),
	      "pending byte is delivered before reading another source byte");

	initialize_endpoint(&source, payload, sizeof(payload));
	initialize_endpoint(&sink, 0, 0U);
	dbgc_byte_fifo_clear(&fifo);
	sink.write_result = -1;
	check((dbgc_byte_stream_bridge_initialize(&channel, &fifo, endpoint_read, &source,
						  endpoint_write,
						  &sink) == DBGC_BYTE_STREAM_BRIDGE_OK) &&
		      (dbgc_byte_stream_bridge_service(&channel, 1U, &transferred) ==
		       DBGC_BYTE_STREAM_BRIDGE_ENDPOINT_ERROR) &&
		      (channel.pending_valid == 1U) && (transferred == 0U),
	      "sink error retains the byte for explicit recovery");

	initialize_endpoint(&source, payload, sizeof(payload));
	initialize_endpoint(&sink, 0, 0U);
	dbgc_byte_fifo_clear(&fifo);
	source.read_result = -1;
	check(dbgc_byte_stream_bridge_initialize(&channel, &fifo, endpoint_read, &source,
						 endpoint_write,
						 &sink) == DBGC_BYTE_STREAM_BRIDGE_OK,
	      "source-error channel initialization succeeds");
	check(dbgc_byte_stream_bridge_service(&channel, 1U, &transferred) ==
		      DBGC_BYTE_STREAM_BRIDGE_ENDPOINT_ERROR,
	      "source endpoint error is reported");
	check(transferred == 0U, "source error reports zero transferred bytes");
	check(dbgc_byte_fifo_count(&fifo) == 0U, "source error leaves FIFO unchanged");

	check(dbgc_byte_stream_bridge_service(&channel, 0U, &transferred) ==
		      DBGC_BYTE_STREAM_BRIDGE_OK,
	      "zero byte budget is a no-op");
	check(dbgc_byte_stream_bridge_initialize(0, &fifo, endpoint_read, &source, endpoint_write,
						 &sink) == DBGC_BYTE_STREAM_BRIDGE_INVALID_ARGUMENT,
	      "null channel rejected");
	check(dbgc_byte_stream_bridge_initialize(&channel, &fifo, 0, &source, endpoint_write,
						 &sink) == DBGC_BYTE_STREAM_BRIDGE_INVALID_ARGUMENT,
	      "null source callback rejected");

	(void)printf("Byte stream bridge checks passed: %u\n", checks);
	return EXIT_SUCCESS;
}
