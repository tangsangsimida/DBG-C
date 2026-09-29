#include "dbgc_byte_fifo.h"

int dbgc_byte_fifo_initialize(dbgc_byte_fifo_t *fifo, uint8_t *storage, size_t capacity)
{
	if (fifo == NULL) {
		return -1;
	}

	fifo->storage = NULL;
	fifo->capacity = 0U;
	fifo->read_index = 0U;
	fifo->write_index = 0U;
	fifo->used = 0U;

	if ((storage == NULL) || (capacity == 0U)) {
		return -1;
	}

	fifo->storage = storage;
	fifo->capacity = capacity;
	return 0;
}

size_t dbgc_byte_fifo_write(dbgc_byte_fifo_t *fifo, const uint8_t *data, size_t length)
{
	size_t available;
	size_t accepted;
	size_t index;

	if ((fifo == NULL) || (fifo->storage == NULL) || ((data == NULL) && (length != 0U))) {
		return 0U;
	}

	available = fifo->capacity - fifo->used;
	accepted = (length < available) ? length : available;

	for (index = 0U; index < accepted; ++index) {
		fifo->storage[fifo->write_index] = data[index];
		++fifo->write_index;
		if (fifo->write_index == fifo->capacity) {
			fifo->write_index = 0U;
		}
	}

	fifo->used += accepted;
	return accepted;
}

size_t dbgc_byte_fifo_read(dbgc_byte_fifo_t *fifo, uint8_t *data, size_t length)
{
	size_t available;
	size_t consumed;
	size_t index;

	if ((fifo == NULL) || (fifo->storage == NULL) || ((data == NULL) && (length != 0U))) {
		return 0U;
	}

	available = fifo->used;
	consumed = (length < available) ? length : available;

	for (index = 0U; index < consumed; ++index) {
		data[index] = fifo->storage[fifo->read_index];
		++fifo->read_index;
		if (fifo->read_index == fifo->capacity) {
			fifo->read_index = 0U;
		}
	}

	fifo->used -= consumed;
	return consumed;
}

void dbgc_byte_fifo_clear(dbgc_byte_fifo_t *fifo)
{
	if ((fifo == NULL) || (fifo->storage == NULL)) {
		return;
	}

	fifo->read_index = 0U;
	fifo->write_index = 0U;
	fifo->used = 0U;
}

size_t dbgc_byte_fifo_count(const dbgc_byte_fifo_t *fifo)
{
	return (fifo == NULL) ? 0U : fifo->used;
}

size_t dbgc_byte_fifo_capacity(const dbgc_byte_fifo_t *fifo)
{
	return (fifo == NULL) ? 0U : fifo->capacity;
}

size_t dbgc_byte_fifo_space(const dbgc_byte_fifo_t *fifo)
{
	if (fifo == NULL) {
		return 0U;
	}

	return fifo->capacity - fifo->used;
}
