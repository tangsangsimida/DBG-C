#ifndef DBGC_CH585_TARGET_RESET_GPIO_HOST_REGS_H
#define DBGC_CH585_TARGET_RESET_GPIO_HOST_REGS_H

#include <stdint.h>

extern volatile uint32_t dbgc_host_R32_PA_DIR;
extern volatile uint32_t dbgc_host_R32_PA_PIN;
extern volatile uint32_t dbgc_host_R32_PA_CLR;
extern volatile uint32_t dbgc_host_R32_PA_PU;
extern volatile uint32_t dbgc_host_R32_PA_PD_DRV;
extern volatile uint32_t dbgc_host_R32_PA_SET;

#define R32_PA_DIR dbgc_host_R32_PA_DIR
#define R32_PA_PIN dbgc_host_R32_PA_PIN
#define R32_PA_CLR dbgc_host_R32_PA_CLR
#define R32_PA_PU dbgc_host_R32_PA_PU
#define R32_PA_PD_DRV dbgc_host_R32_PA_PD_DRV
#define R32_PA_SET dbgc_host_R32_PA_SET

#endif
