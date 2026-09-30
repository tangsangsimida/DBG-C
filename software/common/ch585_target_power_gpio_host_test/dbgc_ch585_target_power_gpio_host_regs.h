#ifndef DBGC_CH585_TARGET_POWER_GPIO_HOST_REGS_H
#define DBGC_CH585_TARGET_POWER_GPIO_HOST_REGS_H

#include <stdint.h>

extern volatile uint32_t dbgc_host_R32_PB_DIR;
extern volatile uint32_t dbgc_host_R32_PB_PIN;
extern volatile uint32_t dbgc_host_R32_PB_CLR;
extern volatile uint32_t dbgc_host_R32_PB_PU;
extern volatile uint32_t dbgc_host_R32_PB_PD_DRV;
extern volatile uint32_t dbgc_host_R32_PB_SET;

#define R32_PB_DIR dbgc_host_R32_PB_DIR
#define R32_PB_PIN dbgc_host_R32_PB_PIN
#define R32_PB_CLR dbgc_host_R32_PB_CLR
#define R32_PB_PU dbgc_host_R32_PB_PU
#define R32_PB_PD_DRV dbgc_host_R32_PB_PD_DRV
#define R32_PB_SET dbgc_host_R32_PB_SET

#endif
