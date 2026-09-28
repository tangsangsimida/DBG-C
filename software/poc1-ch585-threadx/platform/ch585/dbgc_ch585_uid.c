#include "dbgc_ch585_uid.h"

#include <string.h>

#ifndef DBGC_CH585_UID_HOST_TEST
#include "CH585SFR.h"
#include "core_riscv.h"
#endif
#include "ISP585.h"

int dbgc_ch585_uid_read(uint8_t *buffer, size_t buffer_capacity)
{
    uint32_t aligned_uid_words[2];
    uint8_t *uid_bytes = (uint8_t *)aligned_uid_words;
    uint16_t checksum;
    uint32_t status;

    if ((buffer == NULL) || (buffer_capacity < DBGC_CH585_UID_SIZE)) {
        return -1;
    }

    status = FLASH_EEPROM_CMD(CMD_GET_ROM_INFO, ROM_CFG_MAC_ADDR,
                              uid_bytes, 0U);
    if (status != 0U) {
        return -2;
    }

    checksum = (uint16_t)(((uint16_t)uid_bytes[0] |
                           ((uint16_t)uid_bytes[1] << 8)) +
                          ((uint16_t)uid_bytes[2] |
                           ((uint16_t)uid_bytes[3] << 8)) +
                          ((uint16_t)uid_bytes[4] |
                           ((uint16_t)uid_bytes[5] << 8)));
    uid_bytes[6] = (uint8_t)(checksum & 0xFFU);
    uid_bytes[7] = (uint8_t)(checksum >> 8);
    memcpy(buffer, uid_bytes, DBGC_CH585_UID_SIZE);
    return 0;
}
