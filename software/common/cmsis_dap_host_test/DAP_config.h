/*
 * Test-only configuration for the pinned CMSIS-DAP command core and SWD
 * engine. Command-core tests use no-op pins and a mocked SWD transaction;
 * engine tests route callbacks through the CH585 SWD GPIO BSP against modeled
 * registers and a scripted line model. Neither path is product I/O.
 */
#ifndef DBGC_CMSIS_DAP_HOST_TEST_CONFIG_H
#define DBGC_CMSIS_DAP_HOST_TEST_CONFIG_H

#include <stdint.h>

#define CPU_CLOCK 4U
#define IO_PORT_WRITE_CYCLES 1U
#if defined(DBGC_CMSIS_DAP_JTAG_ENGINE_TEST)
#define DAP_SWD 0
#define DAP_JTAG 1
#else
#define DAP_SWD 1
#define DAP_JTAG 0
#endif
#if defined(DBGC_CMSIS_DAP_JTAG_ENGINE_TEST)
#define DAP_JTAG_DEV_CNT 1U
#else
#define DAP_JTAG_DEV_CNT 0U
#endif
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
#define PORT_SWD_SETUP() ((void)0)
#define PORT_JTAG_SETUP() ((void)0)
#define RESET_TARGET() 0U
#define LED_CONNECTED_OUT(value) ((void)(value))
#define LED_RUNNING_OUT(value) ((void)(value))
#define PIN_TDI_IN() 0U
#define PIN_nTRST_OUT(value) ((void)(value))
#define PIN_nTRST_IN() 0U
#define PIN_nRESET_OUT(value) ((void)(value))
#define PIN_nRESET_IN() 0U
#define TIMESTAMP_GET() 0U
#if defined(DBGC_CMSIS_DAP_JTAG_ENGINE_TEST)
void dbgc_test_jtag_tck_set(void);
void dbgc_test_jtag_tck_clear(void);
void dbgc_test_jtag_tms_write(uint32_t bit);
void dbgc_test_jtag_tdi_write(uint32_t bit);
uint32_t dbgc_test_jtag_tdo_read(void);

#define PIN_TDI_OUT(value) dbgc_test_jtag_tdi_write(value)
#define PIN_TDO_IN() dbgc_test_jtag_tdo_read()
#define PIN_SWCLK_TCK_SET() dbgc_test_jtag_tck_set()
#define PIN_SWCLK_TCK_CLR() dbgc_test_jtag_tck_clear()
#define PIN_SWDIO_TMS_SET() dbgc_test_jtag_tms_write(1U)
#define PIN_SWDIO_TMS_CLR() dbgc_test_jtag_tms_write(0U)
#define PIN_SWCLK_TCK_IN() 0U
#define PIN_SWDIO_TMS_IN() 0U
#else
#define PIN_TDI_OUT(value) ((void)(value))
#define PIN_TDO_IN() 0U

#if defined(DBGC_CMSIS_DAP_SWD_ENGINE_TEST)
void dbgc_test_swclk_set(void);
void dbgc_test_swclk_clear(void);
void dbgc_test_swdio_output(uint32_t bit);
uint32_t dbgc_test_swdio_input(void);
void dbgc_test_swdio_output_enable(void);
void dbgc_test_swdio_output_disable(void);

#define PIN_SWCLK_TCK_SET() dbgc_test_swclk_set()
#define PIN_SWCLK_TCK_CLR() dbgc_test_swclk_clear()
#define PIN_SWCLK_TCK_IN() 0U
#define PIN_SWDIO_TMS_SET() dbgc_test_swdio_output(1U)
#define PIN_SWDIO_TMS_CLR() dbgc_test_swdio_output(0U)
#define PIN_SWDIO_TMS_IN() dbgc_test_swdio_input()
#define PIN_SWDIO_OUT(bit) dbgc_test_swdio_output(bit)
#define PIN_SWDIO_IN() dbgc_test_swdio_input()
#define PIN_SWDIO_OUT_ENABLE() dbgc_test_swdio_output_enable()
#define PIN_SWDIO_OUT_DISABLE() dbgc_test_swdio_output_disable()
#else
#define PIN_SWCLK_TCK_SET() ((void)0)
#define PIN_SWCLK_TCK_CLR() ((void)0)
#define PIN_SWCLK_TCK_IN() 0U
#define PIN_SWDIO_TMS_SET() ((void)0)
#define PIN_SWDIO_TMS_CLR() ((void)0)
#define PIN_SWDIO_TMS_IN() 0U
#define PIN_SWDIO_OUT(bit) ((void)(bit))
#define PIN_SWDIO_IN() 0U
#define PIN_SWDIO_OUT_ENABLE() ((void)0)
#define PIN_SWDIO_OUT_DISABLE() ((void)0)
#endif
#endif
#endif
