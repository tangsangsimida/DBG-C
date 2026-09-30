#ifndef DBGC_CH585_VTREF_ADC_HOST_REGS_H
#define DBGC_CH585_VTREF_ADC_HOST_REGS_H

#include <stdint.h>

extern volatile uint32_t dbgc_host_R32_PA_DIR;
extern volatile uint32_t dbgc_host_R32_PA_PU;
extern volatile uint32_t dbgc_host_R32_PA_PD_DRV;
extern volatile uint32_t dbgc_host_R32_PIN_IN_DIS;
extern volatile uint8_t dbgc_host_R8_TKEY_CFG;
extern volatile uint8_t dbgc_host_R8_ADC_CHANNEL;
extern volatile uint8_t dbgc_host_R8_ADC_CFG;
extern volatile uint8_t dbgc_host_R8_ADC_CONVERT;
extern volatile uint16_t dbgc_host_R16_ADC_DATA;
extern volatile uint16_t dbgc_host_R16_CLK_SYS_CFG;

#define R32_PA_DIR dbgc_host_R32_PA_DIR
#define R32_PA_PU dbgc_host_R32_PA_PU
#define R32_PA_PD_DRV dbgc_host_R32_PA_PD_DRV
#define R32_PIN_IN_DIS dbgc_host_R32_PIN_IN_DIS
#define R8_TKEY_CFG dbgc_host_R8_TKEY_CFG
#define R8_ADC_CHANNEL dbgc_host_R8_ADC_CHANNEL
#define R8_ADC_CFG dbgc_host_R8_ADC_CFG
#define R8_ADC_CONVERT dbgc_host_R8_ADC_CONVERT
#define R16_ADC_DATA dbgc_host_R16_ADC_DATA
#define R16_CLK_SYS_CFG dbgc_host_R16_CLK_SYS_CFG

#define RB_TKEY_PWR_ON 0x01U
#define RB_ADC_BUF_EN 0x02U
#define RB_ADC_POWER_ON 0x01U
#define RB_ADC_PGA_GAIN2 0x02U
#define RB_ADC_START 0x01U
#define RB_ADC_EOC_X 0x80U
#define RB_ADC_DATA 0x0FFFU

void dbgc_ch585_vtref_adc_host_start_conversion(void);
uint8_t dbgc_ch585_vtref_adc_host_read_conversion_status(void);
uint16_t dbgc_ch585_vtref_adc_host_read_data(void);

#endif
