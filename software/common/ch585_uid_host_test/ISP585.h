#ifndef DBGC_CH585_UID_HOST_ISP585_H
#define DBGC_CH585_UID_HOST_ISP585_H

#include <stdint.h>

#define CMD_GET_ROM_INFO 0x06
#define ROM_CFG_MAC_ADDR 0x7F018

uint32_t FLASH_EEPROM_CMD(uint8_t cmd, uint32_t StartAddr,
                          void *Buffer, uint32_t Length);

#endif
