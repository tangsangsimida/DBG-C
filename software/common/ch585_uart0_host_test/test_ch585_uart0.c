#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbgc_ch585_uart0.h"
#include "dbgc_ch585_uart0_host_regs.h"

volatile uint32_t dbgc_host_R32_PB_DIR;
volatile uint32_t dbgc_host_R32_PB_PIN;
volatile uint32_t dbgc_host_R32_PB_SET;
volatile uint32_t dbgc_host_R32_PB_PU;
volatile uint32_t dbgc_host_R32_PB_PD_DRV;
volatile uint16_t dbgc_host_R16_PIN_ALTERNATE;
volatile uint16_t dbgc_host_R16_UART0_DL;
volatile uint8_t dbgc_host_R8_UART0_FCR;
volatile uint8_t dbgc_host_R8_UART0_LCR;
volatile uint8_t dbgc_host_R8_UART0_IER;
volatile uint8_t dbgc_host_R8_UART0_DIV;
volatile uint8_t dbgc_host_R8_UART0_RBR;
volatile uint8_t dbgc_host_R8_UART0_THR;
volatile uint8_t dbgc_host_R8_UART0_RFC;
volatile uint8_t dbgc_host_R8_UART0_TFC;

static unsigned int checks;

static void check(int condition, const char *name)
{
    ++checks;
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        exit(EXIT_FAILURE);
    }
}

static void reset_regs(void)
{
    dbgc_host_R32_PB_DIR = UINT32_MAX;
    dbgc_host_R32_PB_PIN = 0U;
    dbgc_host_R32_PB_SET = 0U;
    dbgc_host_R32_PB_PU = 0U;
    dbgc_host_R32_PB_PD_DRV = UINT32_MAX;
    dbgc_host_R16_PIN_ALTERNATE = UINT16_MAX;
    dbgc_host_R16_UART0_DL = 0U;
    dbgc_host_R8_UART0_FCR = 0U;
    dbgc_host_R8_UART0_LCR = 0U;
    dbgc_host_R8_UART0_IER = UINT8_MAX;
    dbgc_host_R8_UART0_DIV = 0U;
    dbgc_host_R8_UART0_RBR = 0U;
    dbgc_host_R8_UART0_THR = 0U;
    dbgc_host_R8_UART0_RFC = 0U;
    dbgc_host_R8_UART0_TFC = 0U;
}

int main(void)
{
    const uint32_t pb4 = 1UL << 4;
    const uint32_t pb7 = 1UL << 7;
    const dbgc_ch585_uart0_fifo_trigger_t triggers[] = {
        DBGC_CH585_UART0_FIFO_TRIGGER_1_BYTE,
        DBGC_CH585_UART0_FIFO_TRIGGER_2_BYTES,
        DBGC_CH585_UART0_FIFO_TRIGGER_4_BYTES,
        DBGC_CH585_UART0_FIFO_TRIGGER_7_BYTES
    };
    size_t trigger_index;
    uint8_t byte = 0U;

    reset_regs();
    check(dbgc_ch585_uart0_init(62400000U, 115200U, 3U,
              DBGC_CH585_UART0_FIFO_TRIGGER_1_BYTE) == 0,
          "init accepts explicit clock, baud, frame, and trigger");
    check((dbgc_host_R32_PB_SET == pb7), "PB7 output latch set high first");
    check((dbgc_host_R32_PB_DIR & pb4) == 0U, "PB4 configured input");
    check((dbgc_host_R32_PB_DIR & pb7) != 0U, "PB7 configured output");
    check((dbgc_host_R32_PB_PU & pb4) != 0U, "PB4 pull-up enabled");
    check((dbgc_host_R32_PB_PD_DRV & (pb4 | pb7)) == 0U,
          "PB4 and PB7 pulldown or high-drive bits cleared");
    check((dbgc_host_R16_PIN_ALTERNATE & RB_PIN_UART0) == 0U,
          "UART0 routed to PB4 and PB7");
    check(dbgc_host_R16_UART0_DL == 68U, "WCH baud divisor calculation");
    check(dbgc_host_R8_UART0_FCR == 0x07U,
          "FIFO enabled and RX/TX FIFOs cleared");
    check(dbgc_host_R8_UART0_LCR == 3U, "explicit line control applied");
    check(dbgc_host_R8_UART0_IER == RB_IER_TXD_EN,
          "TXD output enabled with UART interrupt sources disabled");
    check(dbgc_host_R8_UART0_DIV == 1U, "pre-divisor set per WCH default init");

    for (trigger_index = 0U;
         trigger_index < sizeof(triggers) / sizeof(triggers[0]);
         ++trigger_index) {
        reset_regs();
        check((dbgc_ch585_uart0_init(62400000U, 115200U, 3U,
                   triggers[trigger_index]) == 0) &&
                  ((dbgc_host_R8_UART0_FCR & RB_FCR_FIFO_TRIG) ==
                   (uint8_t)triggers[trigger_index]),
              "each documented FIFO trigger value is applied");
    }

    check(dbgc_ch585_uart0_try_write(0xA5U) == 1,
          "write succeeds when TX FIFO is not full");
    check(dbgc_host_R8_UART0_THR == 0xA5U, "write reaches THR");
    dbgc_host_R8_UART0_TFC = 8U;
    check(dbgc_ch585_uart0_try_write(0x5AU) == 0,
          "write reports full TX FIFO");
    check(dbgc_host_R8_UART0_THR == 0xA5U,
          "full TX FIFO does not modify THR");

    check(dbgc_ch585_uart0_try_read(&byte) == 0,
          "read reports empty RX FIFO");
    dbgc_host_R8_UART0_RFC = 1U;
    dbgc_host_R8_UART0_RBR = 0x3CU;
    check(dbgc_ch585_uart0_try_read(&byte) == 1,
          "read succeeds with RX FIFO data");
    check(byte == 0x3CU, "read returns receiver buffer byte");
    check(dbgc_ch585_uart0_try_read(0) == -1, "null read pointer rejected");

    reset_regs();
    check(dbgc_ch585_uart0_init(0U, 115200U, 3U,
              DBGC_CH585_UART0_FIFO_TRIGGER_1_BYTE) == -1,
          "zero system clock rejected");
    check(dbgc_ch585_uart0_init(62400000U, 0U, 3U,
              DBGC_CH585_UART0_FIFO_TRIGGER_1_BYTE) == -1,
          "zero baud rate rejected");
    check(dbgc_ch585_uart0_init(1U, UINT32_MAX, 3U,
              DBGC_CH585_UART0_FIFO_TRIGGER_1_BYTE) == -1,
          "zero divisor rejected");
    check(dbgc_ch585_uart0_init(UINT32_MAX, 1U, 3U,
              DBGC_CH585_UART0_FIFO_TRIGGER_1_BYTE) == -1,
          "divisor beyond register width rejected");
    check(dbgc_ch585_uart0_init(62400000U, 115200U, 0x80U,
              DBGC_CH585_UART0_FIFO_TRIGGER_1_BYTE) == -1,
          "reserved line-control bit rejected");
    check(dbgc_ch585_uart0_init(62400000U, 115200U, 3U,
              (dbgc_ch585_uart0_fifo_trigger_t)0x20) == -1,
          "unsupported FIFO trigger field rejected");
    check(dbgc_ch585_uart0_init(62400000U, 115200U, 3U,
              (dbgc_ch585_uart0_fifo_trigger_t)0x100) == -1,
          "FIFO trigger value beyond its field width rejected");
    check(dbgc_ch585_uart0_init(62400000U, 115200U, 3U,
              (dbgc_ch585_uart0_fifo_trigger_t)-1) == -1,
          "negative FIFO trigger rejected");
    check(dbgc_host_R32_PB_SET == 0U && dbgc_host_R8_UART0_IER == UINT8_MAX,
          "invalid init leaves hardware registers untouched");

    printf("CH585 UART0 host register checks passed: %u\n", checks);
    return EXIT_SUCCESS;
}
