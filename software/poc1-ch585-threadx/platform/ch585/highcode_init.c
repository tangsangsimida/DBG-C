#include <stdint.h>

/*
 * Temporary no-op required by the archived WCH startup sequence.
 * Clock and QingKe interrupt-controller initialization are not yet integrated.
 */
__attribute__((section(".highcode_init"), noinline))
void highcode_init(void)
{
}

static uint32_t isr_stack[512];
uint32_t *xISRStackTop = &isr_stack[512];
