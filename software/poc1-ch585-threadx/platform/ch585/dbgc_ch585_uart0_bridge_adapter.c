#include "dbgc_ch585_uart0_bridge_adapter.h"

#include "dbgc_ch585_uart0.h"

int dbgc_ch585_uart0_bridge_read(void *context, uint8_t *byte)
{
    (void)context;
    return dbgc_ch585_uart0_try_read(byte);
}

int dbgc_ch585_uart0_bridge_write(void *context, uint8_t byte)
{
    (void)context;
    return dbgc_ch585_uart0_try_write(byte);
}

dbgc_byte_stream_bridge_status_t dbgc_ch585_uart0_duplex_bridge_initialize(
    dbgc_byte_duplex_bridge_t *bridge,
    dbgc_byte_fifo_t *transport_to_uart_fifo,
    dbgc_byte_stream_bridge_read_fn transport_read,
    void *transport_read_context,
    dbgc_byte_fifo_t *uart_to_transport_fifo,
    dbgc_byte_stream_bridge_write_fn transport_write,
    void *transport_write_context)
{
    return dbgc_byte_duplex_bridge_initialize(
        bridge, transport_to_uart_fifo, transport_read, transport_read_context,
        dbgc_ch585_uart0_bridge_write, 0, uart_to_transport_fifo,
        dbgc_ch585_uart0_bridge_read, 0, transport_write,
        transport_write_context);
}
