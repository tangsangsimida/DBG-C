/*
 * Host-only configuration for exercising the pinned CMSIS-DAP command core.
 * These values do not define DBG-C product capabilities or USB parameters.
 */
#ifndef DBGC_CMSIS_DAP_HOST_TEST_CONFIG_H
#define DBGC_CMSIS_DAP_HOST_TEST_CONFIG_H

#include <stdint.h>

#define CPU_CLOCK 4U
#define IO_PORT_WRITE_CYCLES 1U
#define DAP_SWD 0
#define DAP_JTAG 0
#define DAP_JTAG_DEV_CNT 0U
#define DAP_DEFAULT_PORT 0U
#define DAP_DEFAULT_SWJ_CLOCK 1U
#define DELAY_FAST_CYCLES 1U
#define DAP_PACKET_SIZE 64U
#define DAP_PACKET_COUNT 1U
#define SWO_UART 0
#define SWO_MANCHESTER 0
#define SWO_BUFFER_SIZE 0U
#define SWO_STREAM 0
#define TIMESTAMP_CLOCK 0U
#define DAP_UART 0
#define DAP_UART_USB_COM_PORT 0
#define DAP_UART_RX_BUFFER_SIZE 0U
#define DAP_UART_TX_BUFFER_SIZE 0U
#define TARGET_FIXED 0

static inline uint8_t DAP_GetVendorString(char *str)
{
    (void)str;
    return 0U;
}

static inline uint8_t DAP_GetProductString(char *str)
{
    (void)str;
    return 0U;
}

static inline uint8_t DAP_GetSerNumString(char *str)
{
    (void)str;
    return 0U;
}

static inline uint8_t DAP_GetTargetDeviceVendorString(char *str)
{
    (void)str;
    return 0U;
}

static inline uint8_t DAP_GetTargetDeviceNameString(char *str)
{
    (void)str;
    return 0U;
}

static inline uint8_t DAP_GetTargetBoardVendorString(char *str)
{
    (void)str;
    return 0U;
}

static inline uint8_t DAP_GetTargetBoardNameString(char *str)
{
    (void)str;
    return 0U;
}

static inline uint8_t DAP_GetProductFirmwareVersionString(char *str)
{
    (void)str;
    return 0U;
}

#define DAP_SETUP() ((void)0)
#define PORT_OFF() ((void)0)
#define RESET_TARGET() 0U
#define LED_CONNECTED_OUT(value) ((void)(value))
#define LED_RUNNING_OUT(value) ((void)(value))

#define __STATIC_INLINE static inline
#define __STATIC_FORCEINLINE static inline
#define __NOP() ((void)0)
#define __WEAK __attribute__((weak))

#endif
