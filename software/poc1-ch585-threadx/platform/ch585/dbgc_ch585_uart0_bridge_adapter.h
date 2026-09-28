#ifndef DBGC_CH585_UART0_BRIDGE_ADAPTER_H
#define DBGC_CH585_UART0_BRIDGE_ADAPTER_H

#include <stdint.h>

/* Callback adapters for the generic byte-stream bridge. The context argument
 * is unused; UART0 must be configured separately before polling these calls.
 */
int dbgc_ch585_uart0_bridge_read(void *context, uint8_t *byte);
int dbgc_ch585_uart0_bridge_write(void *context, uint8_t byte);

#endif
