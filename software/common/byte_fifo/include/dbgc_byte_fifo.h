#ifndef DBGC_BYTE_FIFO_H
#define DBGC_BYTE_FIFO_H

#include <stddef.h>
#include <stdint.h>

/*
 * Caller-owned byte FIFO for transport payload buffering.
 *
 * Initialize before using any operation. This type is not thread-safe;
 * callers must serialize access when shared between execution contexts.
 * Writes never overwrite unread bytes and may accept fewer bytes than
 * requested when capacity is limited. Data pointers passed to read/write must
 * not overlap the caller-owned storage.
 */
typedef struct {
    uint8_t *storage;
    size_t capacity;
    size_t read_index;
    size_t write_index;
    size_t used;
} dbgc_byte_fifo_t;

/* Returns zero on success and nonzero for a null object, null storage, or
 * zero capacity. Storage remains owned by the caller for the FIFO lifetime. */
int dbgc_byte_fifo_initialize(dbgc_byte_fifo_t *fifo,
                              uint8_t *storage,
                              size_t capacity);

/* Copies up to length bytes. Returns the number of bytes accepted/read.
 * A null data pointer with nonzero length returns zero without changing state.
 */
size_t dbgc_byte_fifo_write(dbgc_byte_fifo_t *fifo,
                            const uint8_t *data,
                            size_t length);
size_t dbgc_byte_fifo_read(dbgc_byte_fifo_t *fifo,
                           uint8_t *data,
                           size_t length);

/* Reset discards buffered data but preserves the caller-owned storage. */
void dbgc_byte_fifo_clear(dbgc_byte_fifo_t *fifo);

/* Query functions return zero when fifo is null. */
size_t dbgc_byte_fifo_count(const dbgc_byte_fifo_t *fifo);
size_t dbgc_byte_fifo_capacity(const dbgc_byte_fifo_t *fifo);
size_t dbgc_byte_fifo_space(const dbgc_byte_fifo_t *fifo);

#endif
