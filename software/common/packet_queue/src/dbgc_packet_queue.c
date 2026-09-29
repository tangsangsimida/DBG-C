#include "dbgc_packet_queue.h"

#include <string.h>

static int dbgc_packet_queue_is_valid(const dbgc_packet_queue_t *queue)
{
    return (queue != NULL) && (queue->storage != NULL) &&
           (queue->lengths != NULL) && (queue->storage_size != 0U) &&
           (queue->slot_size != 0U) && (queue->slot_count != 0U) &&
           (queue->length_count >= queue->slot_count) &&
           (queue->slot_count <= (SIZE_MAX / queue->slot_size)) &&
           (queue->storage_size >= (queue->slot_size * queue->slot_count)) &&
           (queue->count <= queue->slot_count) && (queue->head < queue->slot_count) &&
           (queue->tail < queue->slot_count);
}

static size_t dbgc_packet_queue_advance(size_t index, size_t capacity)
{
    ++index;
    return (index == capacity) ? 0U : index;
}

dbgc_packet_queue_status_t dbgc_packet_queue_initialize(
    dbgc_packet_queue_t *queue,
    uint8_t *storage,
    size_t storage_size,
    size_t *lengths,
    size_t length_count,
    size_t slot_size,
    size_t slot_count)
{
    if (queue == NULL) {
        return DBGC_PACKET_QUEUE_INVALID;
    }

    queue->storage = NULL;
    queue->storage_size = 0U;
    queue->lengths = NULL;
    queue->length_count = 0U;
    queue->slot_size = 0U;
    queue->slot_count = 0U;
    queue->head = 0U;
    queue->tail = 0U;
    queue->count = 0U;

    if ((storage == NULL) || (lengths == NULL) || (storage_size == 0U) ||
        (length_count == 0U) || (slot_size == 0U) || (slot_count == 0U) ||
        (slot_count > (SIZE_MAX / slot_size)) ||
        (storage_size < (slot_size * slot_count)) ||
        (length_count < slot_count)) {
        return DBGC_PACKET_QUEUE_INVALID;
    }

    queue->storage = storage;
    queue->storage_size = storage_size;
    queue->lengths = lengths;
    queue->length_count = length_count;
    queue->slot_size = slot_size;
    queue->slot_count = slot_count;
    return DBGC_PACKET_QUEUE_OK;
}

dbgc_packet_queue_status_t dbgc_packet_queue_push(
    dbgc_packet_queue_t *queue,
    const uint8_t *data,
    size_t length)
{
    size_t offset;

    if (!dbgc_packet_queue_is_valid(queue) || ((data == NULL) && (length != 0U))) {
        return DBGC_PACKET_QUEUE_INVALID;
    }
    if (length > queue->slot_size) {
        return DBGC_PACKET_QUEUE_TOO_LARGE;
    }
    if (queue->count == queue->slot_count) {
        return DBGC_PACKET_QUEUE_FULL;
    }

    offset = queue->tail * queue->slot_size;
    if (length != 0U) {
        memcpy(&queue->storage[offset], data, length);
    }
    queue->lengths[queue->tail] = length;
    queue->tail = dbgc_packet_queue_advance(queue->tail, queue->slot_count);
    ++queue->count;
    return DBGC_PACKET_QUEUE_OK;
}

dbgc_packet_queue_status_t dbgc_packet_queue_pop(
    dbgc_packet_queue_t *queue,
    uint8_t *data,
    size_t output_capacity,
    size_t *length)
{
    size_t payload_length;
    size_t offset;

    if (!dbgc_packet_queue_is_valid(queue) || (length == NULL)) {
        return DBGC_PACKET_QUEUE_INVALID;
    }
    if (queue->count == 0U) {
        return DBGC_PACKET_QUEUE_EMPTY;
    }

    payload_length = queue->lengths[queue->head];
    if (payload_length > queue->slot_size) {
        return DBGC_PACKET_QUEUE_INVALID;
    }
    if ((data == NULL) && (payload_length != 0U)) {
        return DBGC_PACKET_QUEUE_INVALID;
    }
    if (output_capacity < payload_length) {
        *length = payload_length;
        return DBGC_PACKET_QUEUE_BUFFER_TOO_SMALL;
    }

    offset = queue->head * queue->slot_size;
    if (payload_length != 0U) {
        memcpy(data, &queue->storage[offset], payload_length);
    }
    *length = payload_length;
    queue->lengths[queue->head] = 0U;
    queue->head = dbgc_packet_queue_advance(queue->head, queue->slot_count);
    --queue->count;
    return DBGC_PACKET_QUEUE_OK;
}

void dbgc_packet_queue_clear(dbgc_packet_queue_t *queue)
{
    if (!dbgc_packet_queue_is_valid(queue)) {
        return;
    }

    queue->head = 0U;
    queue->tail = 0U;
    queue->count = 0U;
}

size_t dbgc_packet_queue_count(const dbgc_packet_queue_t *queue)
{
    return dbgc_packet_queue_is_valid(queue) ? queue->count : 0U;
}

size_t dbgc_packet_queue_capacity(const dbgc_packet_queue_t *queue)
{
    return dbgc_packet_queue_is_valid(queue) ? queue->slot_count : 0U;
}
