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
