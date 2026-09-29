#ifndef DBGC_PACKET_QUEUE_H
#define DBGC_PACKET_QUEUE_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    DBGC_PACKET_QUEUE_OK = 0,
    DBGC_PACKET_QUEUE_INVALID = -1,
    DBGC_PACKET_QUEUE_FULL = -2,
    DBGC_PACKET_QUEUE_TOO_LARGE = -3,
    DBGC_PACKET_QUEUE_EMPTY = -4,
    DBGC_PACKET_QUEUE_BUFFER_TOO_SMALL = -5
} dbgc_packet_queue_status_t;

/* Caller-owned fixed-slot record queue. The queue object, length array, and
 * payload storage must be separate memory regions and remain valid for the
 * queue lifetime. Data pointers passed to push/pop must not overlap those
 * regions. Access is not thread-safe; callers must serialize all operations.
 * Payload size and slot count are supplied by the caller; this module chooses
 * no USB, RF, BLE, or CMSIS-DAP packet dimensions.
 */
typedef struct {
    uint8_t *storage;
    size_t storage_size;
    size_t *lengths;
    size_t length_count;
    size_t slot_size;
    size_t slot_count;
    size_t head;
    size_t tail;
    size_t count;
} dbgc_packet_queue_t;

/* Initializes an empty queue. storage_size must hold slot_size * slot_count
 * bytes, and length_count must be at least slot_count. Returns INVALID for
 * null pointers, zero dimensions, or a multiplication/size overflow. A
 * failed initialization leaves a valid queue object empty and unusable.
 */
dbgc_packet_queue_status_t dbgc_packet_queue_initialize(
    dbgc_packet_queue_t *queue,
    uint8_t *storage,
    size_t storage_size,
    size_t *lengths,
    size_t length_count,
    size_t slot_size,
    size_t slot_count);

/* Enqueue one record. A zero-length record is valid and may use a null data
 * pointer. Failure does not change queue contents.
 */
dbgc_packet_queue_status_t dbgc_packet_queue_push(
    dbgc_packet_queue_t *queue,
    const uint8_t *data,
    size_t length);

/* Dequeue one record. On BUFFER_TOO_SMALL, *length receives the required
 * payload size and the record remains queued. On EMPTY or INVALID, *length
 * is unchanged. A zero-length record may use a null output pointer.
 */
dbgc_packet_queue_status_t dbgc_packet_queue_pop(
    dbgc_packet_queue_t *queue,
    uint8_t *data,
    size_t output_capacity,
    size_t *length);

void dbgc_packet_queue_clear(dbgc_packet_queue_t *queue);
size_t dbgc_packet_queue_count(const dbgc_packet_queue_t *queue);
size_t dbgc_packet_queue_capacity(const dbgc_packet_queue_t *queue);

#endif
