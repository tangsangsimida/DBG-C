#ifndef DBGC_CH585_UID_H
#define DBGC_CH585_UID_H

#include <stddef.h>
#include <stdint.h>

#define DBGC_CH585_UID_SIZE 8U

/* Returns 0 on success, -1 for an invalid destination, or -2 on ROM command failure. */
int dbgc_ch585_uid_read(uint8_t *buffer, size_t buffer_capacity);

#endif
