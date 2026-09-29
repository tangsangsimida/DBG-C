#ifndef DBGC_CH585_UART0_BRIDGE_ADAPTER_H
#define DBGC_CH585_UART0_BRIDGE_ADAPTER_H

#include <stdint.h>

#include "dbgc_byte_duplex_bridge.h"

/* Callback adapters for the generic byte-stream bridge. The context argument
 * is unused; UART0 must be configured separately before polling these calls.
 */
int dbgc_ch585_uart0_bridge_read(void *context, uint8_t *byte);
int dbgc_ch585_uart0_bridge_write(void *context, uint8_t byte);

/* Compose UART0 with a caller-selected stream transport. The caller owns both
 * FIFOs and their storage and must initialize UART0 before servicing the
 * bridge. This function selects no UART settings and provides no scheduling
 * or synchronization.
 */
dbgc_byte_stream_bridge_status_t dbgc_ch585_uart0_duplex_bridge_initialize(
	dbgc_byte_duplex_bridge_t *bridge, dbgc_byte_fifo_t *transport_to_uart_fifo,
	dbgc_byte_stream_bridge_read_fn transport_read, void *transport_read_context,
	dbgc_byte_fifo_t *uart_to_transport_fifo, dbgc_byte_stream_bridge_write_fn transport_write,
	void *transport_write_context);

#endif
