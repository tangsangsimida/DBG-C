#include "dbgc_ch585_uart0.h"

#if defined(DBGC_CH585_UART0_HOST_TEST)
#include "dbgc_ch585_uart0_host_regs.h"
#else
#include "wch/CH585SFR.h"
#endif

#define DBGC_CH585_PB4_MASK (1UL << 4)
#define DBGC_CH585_PB7_MASK (1UL << 7)
#define DBGC_CH585_UART0_LCR_DEFINED_MASK 0x7FU

int dbgc_ch585_uart0_init(uint32_t sys_clock_hz, uint32_t baudrate,
                          uint8_t line_control,
                          dbgc_ch585_uart0_fifo_trigger_t fifo_trigger)
{
    uint64_t scaled_divisor;
    uint64_t divisor;
    uint64_t denominator;

    if ((sys_clock_hz == 0U) || (baudrate == 0U) ||
        ((line_control & (uint8_t)~DBGC_CH585_UART0_LCR_DEFINED_MASK) != 0U)) {
        return -1;
    }
    switch (fifo_trigger) {
    case DBGC_CH585_UART0_FIFO_TRIGGER_1_BYTE:
    case DBGC_CH585_UART0_FIFO_TRIGGER_2_BYTES:
    case DBGC_CH585_UART0_FIFO_TRIGGER_4_BYTES:
    case DBGC_CH585_UART0_FIFO_TRIGGER_7_BYTES:
        break;
    default:
        return -1;
    }

    denominator = (uint64_t)8U * (uint64_t)baudrate;
    scaled_divisor = (10U * (uint64_t)sys_clock_hz) / denominator;
    divisor = (scaled_divisor + 5U) / 10U;
    if ((divisor == 0U) || (divisor > UINT16_MAX)) {
        return -1;
    }

    /* EVT ADC DebugInit configures PB4/PB7 this way for UART0. */
    R32_PB_SET = DBGC_CH585_PB7_MASK;
    R32_PB_PD_DRV &= ~(DBGC_CH585_PB4_MASK | DBGC_CH585_PB7_MASK);
    R32_PB_PU |= DBGC_CH585_PB4_MASK;
    R32_PB_DIR &= ~DBGC_CH585_PB4_MASK;
    R32_PB_DIR |= DBGC_CH585_PB7_MASK;
    R16_PIN_ALTERNATE &= (uint16_t)~RB_PIN_UART0;

    R16_UART0_DL = (uint16_t)divisor;
    R8_UART0_FCR = (uint8_t)fifo_trigger | RB_FCR_TX_FIFO_CLR |
                   RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN;
    R8_UART0_LCR = line_control;
    R8_UART0_IER = RB_IER_TXD_EN;
    R8_UART0_DIV = 1U;
    return 0;
}

int dbgc_ch585_uart0_try_write(uint8_t byte)
{
    if (R8_UART0_TFC >= UART_FIFO_SIZE) {
        return 0;
    }

    R8_UART0_THR = byte;
    return 1;
}

int dbgc_ch585_uart0_try_read(uint8_t *byte)
{
    if (byte == 0) {
        return -1;
    }
    if (R8_UART0_RFC == 0U) {
        return 0;
    }

    *byte = R8_UART0_RBR;
    return 1;
}
