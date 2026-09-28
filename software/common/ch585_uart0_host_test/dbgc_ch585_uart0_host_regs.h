#ifndef DBGC_CH585_UART0_HOST_REGS_H
#define DBGC_CH585_UART0_HOST_REGS_H

#include <stdint.h>

extern volatile uint32_t dbgc_host_R32_PB_DIR;
extern volatile uint32_t dbgc_host_R32_PB_PIN;
extern volatile uint32_t dbgc_host_R32_PB_SET;
extern volatile uint32_t dbgc_host_R32_PB_PU;
extern volatile uint32_t dbgc_host_R32_PB_PD_DRV;
extern volatile uint16_t dbgc_host_R16_PIN_ALTERNATE;
extern volatile uint16_t dbgc_host_R16_UART0_DL;
extern volatile uint8_t dbgc_host_R8_UART0_FCR;
extern volatile uint8_t dbgc_host_R8_UART0_LCR;
extern volatile uint8_t dbgc_host_R8_UART0_IER;
extern volatile uint8_t dbgc_host_R8_UART0_DIV;
extern volatile uint8_t dbgc_host_R8_UART0_RBR;
extern volatile uint8_t dbgc_host_R8_UART0_THR;
extern volatile uint8_t dbgc_host_R8_UART0_RFC;
extern volatile uint8_t dbgc_host_R8_UART0_TFC;

#define R32_PB_DIR dbgc_host_R32_PB_DIR
#define R32_PB_PIN dbgc_host_R32_PB_PIN
#define R32_PB_SET dbgc_host_R32_PB_SET
#define R32_PB_PU dbgc_host_R32_PB_PU
#define R32_PB_PD_DRV dbgc_host_R32_PB_PD_DRV
#define R16_PIN_ALTERNATE dbgc_host_R16_PIN_ALTERNATE
#define R16_UART0_DL dbgc_host_R16_UART0_DL
#define R8_UART0_FCR dbgc_host_R8_UART0_FCR
#define R8_UART0_LCR dbgc_host_R8_UART0_LCR
#define R8_UART0_IER dbgc_host_R8_UART0_IER
#define R8_UART0_DIV dbgc_host_R8_UART0_DIV
#define R8_UART0_RBR dbgc_host_R8_UART0_RBR
#define R8_UART0_THR dbgc_host_R8_UART0_THR
#define R8_UART0_RFC dbgc_host_R8_UART0_RFC
#define R8_UART0_TFC dbgc_host_R8_UART0_TFC

#define RB_PIN_UART0 0x10U
#define RB_FCR_FIFO_EN 0x01U
#define RB_FCR_RX_FIFO_CLR 0x02U
#define RB_FCR_TX_FIFO_CLR 0x04U
#define RB_FCR_FIFO_TRIG 0xC0U
#define RB_IER_TXD_EN 0x40U
#define UART_FIFO_SIZE 8U

#endif
