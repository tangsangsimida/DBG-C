#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbgc_ch585_uart0.h"
#include "dbgc_ch585_uart0_bridge_adapter.h"
#include "dbgc_ch585_uart0_host_regs.h"

typedef struct {
	const uint8_t *input;
	size_t input_length;
	size_t input_offset;
	uint8_t output[4];
	size_t output_length;
} endpoint_t;

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

static int endpoint_read(void *context, uint8_t *byte)
{
	endpoint_t *endpoint = (endpoint_t *)context;

	if (endpoint->input_offset >= endpoint->input_length) {
		return 0;
	}

	*byte = endpoint->input[endpoint->input_offset];
	++endpoint->input_offset;
	return 1;
}

static int endpoint_write(void *context, uint8_t byte)
{
	endpoint_t *endpoint = (endpoint_t *)context;

	if (endpoint->output_length >= sizeof(endpoint->output)) {
		return -1;
	}

	endpoint->output[endpoint->output_length] = byte;
	++endpoint->output_length;
	return 1;
}

int main(void)
{
	const uint32_t pb4 = 1UL << 4;
	const uint32_t pb7 = 1UL << 7;
	const dbgc_ch585_uart0_fifo_trigger_t triggers[] = {
		DBGC_CH585_UART0_FIFO_TRIGGER_1_BYTE, DBGC_CH585_UART0_FIFO_TRIGGER_2_BYTES,
		DBGC_CH585_UART0_FIFO_TRIGGER_4_BYTES, DBGC_CH585_UART0_FIFO_TRIGGER_7_BYTES
	};
	const dbgc_ch585_uart0_word_length_t word_lengths[] = { DBGC_CH585_UART0_WORD_LENGTH_5,
								DBGC_CH585_UART0_WORD_LENGTH_6,
								DBGC_CH585_UART0_WORD_LENGTH_7,
								DBGC_CH585_UART0_WORD_LENGTH_8 };
	const dbgc_ch585_uart0_stop_bits_t stop_options[] = { DBGC_CH585_UART0_STOP_BITS_1,
							      DBGC_CH585_UART0_STOP_BITS_2 };
	const dbgc_ch585_uart0_parity_t parity_options[] = { DBGC_CH585_UART0_PARITY_NONE,
							     DBGC_CH585_UART0_PARITY_ODD,
							     DBGC_CH585_UART0_PARITY_EVEN,
							     DBGC_CH585_UART0_PARITY_MARK,
							     DBGC_CH585_UART0_PARITY_SPACE };
	size_t trigger_index;
	size_t word_index;
	size_t stop_index;
	size_t parity_index;
	uint8_t byte = 0U;
	uint8_t line_control = 0U;
	uint16_t divisor = 0U;
	const uint8_t transport_input[] = { 0x51U, 0x52U };
	uint8_t transport_to_uart_storage[4];
	uint8_t uart_to_transport_storage[4];
	dbgc_byte_fifo_t transport_to_uart_fifo;
	dbgc_byte_fifo_t uart_to_transport_fifo;
	dbgc_byte_duplex_bridge_t duplex_bridge;
	endpoint_t transport_source = { transport_input, sizeof(transport_input), 0U, { 0U }, 0U };
	endpoint_t transport_sink = { 0, 0U, 0U, { 0U }, 0U };
	size_t transport_to_uart_transferred;
	size_t uart_to_transport_transferred;

	reset_regs();
	for (word_index = 0U; word_index < sizeof(word_lengths) / sizeof(word_lengths[0]);
	     ++word_index) {
		for (stop_index = 0U; stop_index < sizeof(stop_options) / sizeof(stop_options[0]);
		     ++stop_index) {
			for (parity_index = 0U;
			     parity_index < sizeof(parity_options) / sizeof(parity_options[0]);
			     ++parity_index) {
				const uint8_t expected = (uint8_t)word_lengths[word_index] |
							 (uint8_t)stop_options[stop_index] |
							 (uint8_t)parity_options[parity_index];
				check((dbgc_ch585_uart0_build_lcr(
					       word_lengths[word_index], stop_options[stop_index],
					       parity_options[parity_index], &line_control) == 0) &&
					      (line_control == expected),
				      "all documented LCR field combinations compose exactly");
			}
		}
	}
	check(dbgc_ch585_uart0_build_lcr((dbgc_ch585_uart0_word_length_t)4,
					 DBGC_CH585_UART0_STOP_BITS_1, DBGC_CH585_UART0_PARITY_NONE,
					 &line_control) == -1,
	      "invalid word length rejected");
	check(dbgc_ch585_uart0_build_lcr(DBGC_CH585_UART0_WORD_LENGTH_8,
					 (dbgc_ch585_uart0_stop_bits_t)8,
					 DBGC_CH585_UART0_PARITY_NONE, &line_control) == -1,
	      "invalid stop-bit choice rejected");
	check(dbgc_ch585_uart0_build_lcr(DBGC_CH585_UART0_WORD_LENGTH_8,
					 DBGC_CH585_UART0_STOP_BITS_1,
					 (dbgc_ch585_uart0_parity_t)0x40, &line_control) == -1,
	      "invalid parity choice rejected");
	check(dbgc_ch585_uart0_build_lcr(DBGC_CH585_UART0_WORD_LENGTH_8,
					 DBGC_CH585_UART0_STOP_BITS_1, DBGC_CH585_UART0_PARITY_NONE,
					 0) == -1,
	      "null LCR output pointer rejected");

	check((dbgc_ch585_uart0_calculate_divisor(62400000U, 115200U, &divisor) == 0) &&
		      (divisor == 68U),
	      "WCH UART0 divisor formula rounds 62.4 MHz and 115200 baud");
	check((dbgc_ch585_uart0_calculate_divisor(32000000U, 9600U, &divisor) == 0) &&
		      (divisor == 417U),
	      "WCH UART0 divisor formula rounds 32 MHz and 9600 baud");
	check((dbgc_ch585_uart0_calculate_divisor(32000000U, 4000000U, &divisor) == 0) &&
		      (divisor == 1U),
	      "minimum nonzero UART0 divisor accepted");
	check(dbgc_ch585_uart0_calculate_divisor(0U, 115200U, &divisor) == -1,
	      "zero UART0 clock rejected");
	check(dbgc_ch585_uart0_calculate_divisor(62400000U, 0U, &divisor) == -1,
	      "zero UART0 baud rejected");
	check(dbgc_ch585_uart0_calculate_divisor(62400000U, 115200U, 0) == -1,
	      "null UART0 divisor output rejected");
	check(dbgc_ch585_uart0_calculate_divisor(1U, UINT32_MAX, &divisor) == -1,
	      "zero UART0 divisor rejected");
	check(dbgc_ch585_uart0_calculate_divisor(UINT32_MAX, 1U, &divisor) == -1,
	      "UART0 divisor wider than data register rejected");

	check(dbgc_ch585_uart0_init(62400000U, 115200U, 3U, DBGC_CH585_UART0_FIFO_TRIGGER_1_BYTE) ==
		      0,
	      "init accepts explicit clock, baud, frame, and trigger");
	check((dbgc_host_R32_PB_SET == pb7), "PB7 output latch set high first");
	check((dbgc_host_R32_PB_DIR & pb4) == 0U, "PB4 configured input");
	check((dbgc_host_R32_PB_DIR & pb7) != 0U, "PB7 configured output");
	check((dbgc_host_R32_PB_PU & pb4) != 0U, "PB4 pull-up enabled");
	check((dbgc_host_R32_PB_PD_DRV & (pb4 | pb7)) == 0U,
	      "PB4 and PB7 pulldown or high-drive bits cleared");
	check((dbgc_host_R16_PIN_ALTERNATE & RB_PIN_UART0) == 0U, "UART0 routed to PB4 and PB7");
	check(dbgc_host_R16_UART0_DL == 68U, "WCH baud divisor calculation");
	check(dbgc_host_R8_UART0_FCR == 0x07U, "FIFO enabled and RX/TX FIFOs cleared");
	check(dbgc_host_R8_UART0_LCR == 3U, "explicit line control applied");
	check(dbgc_host_R8_UART0_IER == RB_IER_TXD_EN,
	      "TXD output enabled with UART interrupt sources disabled");
	check(dbgc_host_R8_UART0_DIV == 1U, "pre-divisor set per WCH default init");

	for (trigger_index = 0U; trigger_index < sizeof(triggers) / sizeof(triggers[0]);
	     ++trigger_index) {
		reset_regs();
		check((dbgc_ch585_uart0_init(62400000U, 115200U, 3U, triggers[trigger_index]) ==
		       0) && ((dbgc_host_R8_UART0_FCR & RB_FCR_FIFO_TRIG) ==
			      (uint8_t)triggers[trigger_index]),
		      "each documented FIFO trigger value is applied");
	}

	check(dbgc_ch585_uart0_try_write(0xA5U) == 1, "write succeeds when TX FIFO is not full");
	check(dbgc_host_R8_UART0_THR == 0xA5U, "write reaches THR");
	dbgc_host_R8_UART0_TFC = 8U;
	check(dbgc_ch585_uart0_try_write(0x5AU) == 0, "write reports full TX FIFO");
	check(dbgc_host_R8_UART0_THR == 0xA5U, "full TX FIFO does not modify THR");

	check(dbgc_ch585_uart0_try_read(&byte) == 0, "read reports empty RX FIFO");
	dbgc_host_R8_UART0_RFC = 1U;
	dbgc_host_R8_UART0_RBR = 0x3CU;
	check(dbgc_ch585_uart0_try_read(&byte) == 1, "read succeeds with RX FIFO data");
	check(byte == 0x3CU, "read returns receiver buffer byte");
	dbgc_host_R8_UART0_RFC = 0U;
	check(dbgc_ch585_uart0_bridge_read(&line_control, &byte) == 0,
	      "bridge source callback reports empty UART RX FIFO");
	dbgc_host_R8_UART0_RFC = 1U;
	dbgc_host_R8_UART0_RBR = 0x69U;
	check((dbgc_ch585_uart0_bridge_read(&line_control, &byte) == 1) && (byte == 0x69U),
	      "bridge source callback forwards one UART RX byte");
	check(dbgc_ch585_uart0_try_read(0) == -1, "null read pointer rejected");
	dbgc_host_R8_UART0_TFC = 0U;
	check((dbgc_ch585_uart0_bridge_write(&line_control, 0x96U) == 1) &&
		      (dbgc_host_R8_UART0_THR == 0x96U),
	      "bridge sink callback forwards one byte to UART TX");
	dbgc_host_R8_UART0_TFC = 8U;
	check(dbgc_ch585_uart0_bridge_write(&line_control, 0xA6U) == 0,
	      "bridge sink callback preserves UART TX backpressure");

	reset_regs();
	check(dbgc_ch585_uart0_init(62400000U, 115200U, 3U, DBGC_CH585_UART0_FIFO_TRIGGER_1_BYTE) ==
		      0,
	      "UART0 initializes before duplex bridge composition");
	check((dbgc_byte_fifo_initialize(&transport_to_uart_fifo, transport_to_uart_storage,
					 sizeof(transport_to_uart_storage)) == 0) &&
		      (dbgc_byte_fifo_initialize(&uart_to_transport_fifo, uart_to_transport_storage,
						 sizeof(uart_to_transport_storage)) == 0),
	      "duplex bridge FIFOs initialize with caller-owned storage");
	check(dbgc_ch585_uart0_duplex_bridge_initialize(
		      &duplex_bridge, &transport_to_uart_fifo, endpoint_read, &transport_source,
		      &uart_to_transport_fifo, endpoint_write,
		      &transport_sink) == DBGC_BYTE_STREAM_BRIDGE_OK,
	      "UART0 adapter composes transport callbacks with both directions");
	check((dbgc_byte_duplex_bridge_service(
		       &duplex_bridge, 1U, 1U, &transport_to_uart_transferred,
		       &uart_to_transport_transferred) == DBGC_BYTE_STREAM_BRIDGE_OK) &&
		      (transport_to_uart_transferred == 1U) &&
		      (uart_to_transport_transferred == 0U) && (dbgc_host_R8_UART0_THR == 0x51U),
	      "transport input reaches the allocated UART0 TX path");
	dbgc_host_R8_UART0_RFC = 1U;
	dbgc_host_R8_UART0_RBR = 0xA7U;
	check((dbgc_byte_duplex_bridge_service(
		       &duplex_bridge, 1U, 1U, &transport_to_uart_transferred,
		       &uart_to_transport_transferred) == DBGC_BYTE_STREAM_BRIDGE_OK) &&
		      (transport_to_uart_transferred == 1U) &&
		      (uart_to_transport_transferred == 1U) && (dbgc_host_R8_UART0_THR == 0x52U) &&
		      (transport_sink.output_length == 1U) && (transport_sink.output[0] == 0xA7U),
	      "UART0 RX byte reaches the caller transport write callback");

	reset_regs();
	check(dbgc_ch585_uart0_init(0U, 115200U, 3U, DBGC_CH585_UART0_FIFO_TRIGGER_1_BYTE) == -1,
	      "zero system clock rejected");
	check(dbgc_ch585_uart0_init(62400000U, 0U, 3U, DBGC_CH585_UART0_FIFO_TRIGGER_1_BYTE) == -1,
	      "zero baud rate rejected");
	check(dbgc_ch585_uart0_init(1U, UINT32_MAX, 3U, DBGC_CH585_UART0_FIFO_TRIGGER_1_BYTE) == -1,
	      "zero divisor rejected");
	check(dbgc_ch585_uart0_init(UINT32_MAX, 1U, 3U, DBGC_CH585_UART0_FIFO_TRIGGER_1_BYTE) == -1,
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
	check(dbgc_ch585_uart0_init(62400000U, 115200U, 3U, (dbgc_ch585_uart0_fifo_trigger_t)-1) ==
		      -1,
	      "negative FIFO trigger rejected");
	check(dbgc_host_R32_PB_SET == 0U && dbgc_host_R8_UART0_IER == UINT8_MAX,
	      "invalid init leaves hardware registers untouched");

	printf("CH585 UART0 host register checks passed: %u\n", checks);
	return EXIT_SUCCESS;
}
