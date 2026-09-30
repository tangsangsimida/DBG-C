#ifndef DBGC_CH585_SPI1_CONFIG_HOST_REGS_H
#define DBGC_CH585_SPI1_CONFIG_HOST_REGS_H

#include <stdint.h>

extern volatile uint8_t dbgc_host_R8_SPI1_CTRL_MOD;
extern volatile uint8_t dbgc_host_R8_SPI1_CTRL_CFG;
extern volatile uint8_t dbgc_host_R8_SPI1_INT_FLAG;
extern volatile uint8_t dbgc_host_R8_SPI1_CLOCK_DIV;

#define R8_SPI1_CTRL_MOD dbgc_host_R8_SPI1_CTRL_MOD
#define R8_SPI1_CTRL_CFG dbgc_host_R8_SPI1_CTRL_CFG
#define R8_SPI1_INT_FLAG dbgc_host_R8_SPI1_INT_FLAG
#define R8_SPI1_CLOCK_DIV dbgc_host_R8_SPI1_CLOCK_DIV

#define RB_SPI_ALL_CLEAR 0x02U
#define RB_SPI_MST_SCK_MOD 0x08U
#define RB_SPI_SCK_OE 0x20U
#define RB_SPI_MOSI_OE 0x40U
#define RB_SPI_AUTO_IF 0x10U
#define RB_SPI_BIT_ORDER 0x20U
#define RB_SPI_MST_DLY_EN 0x40U
#define RB_SPI_FREE 0x40U

#endif
