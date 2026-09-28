#ifndef DBGC_CH585_UART0_H
#define DBGC_CH585_UART0_H

#include <stdint.h>

/* Values match the documented RB_FCR_FIFO_TRIG field in CH585SFR.h. */
typedef enum {
    DBGC_CH585_UART0_FIFO_TRIGGER_1_BYTE = 0x00,
    DBGC_CH585_UART0_FIFO_TRIGGER_2_BYTES = 0x40,
    DBGC_CH585_UART0_FIFO_TRIGGER_4_BYTES = 0x80,
    DBGC_CH585_UART0_FIFO_TRIGGER_7_BYTES = 0xC0
} dbgc_ch585_uart0_fifo_trigger_t;

/* Encodings follow the documented RB_LCR_WORD_SZ/RB_LCR_STOP_BIT/
 * RB_LCR_PAR_EN/RB_LCR_PAR_MOD fields in CH585SFR.h. These are choices, not
 * DBG-C product defaults.
 */
typedef enum {
    DBGC_CH585_UART0_WORD_LENGTH_5 = 0x00,
    DBGC_CH585_UART0_WORD_LENGTH_6 = 0x01,
    DBGC_CH585_UART0_WORD_LENGTH_7 = 0x02,
    DBGC_CH585_UART0_WORD_LENGTH_8 = 0x03
} dbgc_ch585_uart0_word_length_t;

typedef enum {
    DBGC_CH585_UART0_STOP_BITS_1 = 0x00,
    DBGC_CH585_UART0_STOP_BITS_2 = 0x04
} dbgc_ch585_uart0_stop_bits_t;

typedef enum {
    DBGC_CH585_UART0_PARITY_NONE = 0x00,
    DBGC_CH585_UART0_PARITY_ODD = 0x08,
    DBGC_CH585_UART0_PARITY_EVEN = 0x18,
    DBGC_CH585_UART0_PARITY_MARK = 0x28,
    DBGC_CH585_UART0_PARITY_SPACE = 0x38
} dbgc_ch585_uart0_parity_t;

/* Compose an LCR byte from explicit format choices. Returns zero on success,
 * or -1 for an invalid choice or null output pointer. No default is selected.
 */
int dbgc_ch585_uart0_build_lcr(dbgc_ch585_uart0_word_length_t word_length,
                               dbgc_ch585_uart0_stop_bits_t stop_bits,
                               dbgc_ch585_uart0_parity_t parity,
                               uint8_t *line_control);

/* Initialize UART0 on the MCU-001 PB4/PB7 allocation.
 * line_control is the caller-selected documented UART0 LCR value, with bit 7
 * clear. sys_clock_hz, baudrate, framing, and FIFO trigger are explicit inputs;
 * this function does not choose product defaults. UART interrupt sources are
 * disabled; the WCH-defined UART TXD output-enable bit is set.
 * Returns zero on success, -1 for invalid arguments. No synchronization is
 * provided; serialize calls with code that changes GPIOB or pin remapping.
 */
int dbgc_ch585_uart0_init(uint32_t sys_clock_hz, uint32_t baudrate,
                          uint8_t line_control,
                          dbgc_ch585_uart0_fifo_trigger_t fifo_trigger);

/* Nonblocking polling operations: return 1 when a byte moved, 0 when the
 * corresponding FIFO cannot move a byte now, and -1 for a null read pointer.
 * These are not ISR-safe queues and do not provide synchronization.
 */
int dbgc_ch585_uart0_try_write(uint8_t byte);
int dbgc_ch585_uart0_try_read(uint8_t *byte);

#endif
