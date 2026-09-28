#include "dbgc_ch585_uid.h"

#include <string.h>

/* Prototype matches CH58x_flash.h in the supplied CH585EVT archive. */
extern void GET_UNIQUE_ID(uint8_t *Buffer);

int dbgc_ch585_uid_read(uint8_t *buffer, size_t buffer_capacity)
{
    uint32_t aligned_uid_words[2];

    if ((buffer == NULL) || (buffer_capacity < DBGC_CH585_UID_SIZE)) {
        return -1;
    }

    GET_UNIQUE_ID((uint8_t *)aligned_uid_words);
    memcpy(buffer, aligned_uid_words, DBGC_CH585_UID_SIZE);
    return 0;
}
