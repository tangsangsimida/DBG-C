#ifndef DBGC_CH585_SPI1_CONFIG_H
#define DBGC_CH585_SPI1_CONFIG_H

#include <stdint.h>

/**
 * @brief 定义 CH585 SPI1 支持的时钟模式。
 */
typedef enum {
	DBGC_CH585_SPI1_MODE_0 = 0,
	DBGC_CH585_SPI1_MODE_3 = 1
} dbgc_ch585_spi1_mode_t;

/**
 * @brief 定义 SPI1 的数据位序。
 */
typedef enum {
	DBGC_CH585_SPI1_BIT_ORDER_MSB_FIRST = 0,
	DBGC_CH585_SPI1_BIT_ORDER_LSB_FIRST = 1
} dbgc_ch585_spi1_bit_order_t;

/**
 * @brief 定义 SPI1 配置结果。
 */
typedef enum {
	DBGC_CH585_SPI1_CONFIG_OK = 0,
	DBGC_CH585_SPI1_CONFIG_INVALID_ARGUMENT,
	DBGC_CH585_SPI1_CONFIG_BUSY
} dbgc_ch585_spi1_config_status_t;

/**
 * @brief 配置 CH585 SPI1 主机模式、时钟模式、位序与分频系数。
 *
 * 分频系数依据 CH585/CH584 数据手册限定为 2 至 254。函数不配置 PA0/PA1/PA2 GPIO，
 * 不控制外部片选，不执行传输，也不选择 Flash 参数。调用方须串行调用并保证 SPI1 空闲。
 * @param mode SPI 时钟模式 0 或 3。
 * @param bit_order 数据位序。
 * @param clock_divider 主机时钟分频系数。
 * @return 配置结果。
 */
dbgc_ch585_spi1_config_status_t dbgc_ch585_spi1_configure(dbgc_ch585_spi1_mode_t mode,
							  dbgc_ch585_spi1_bit_order_t bit_order,
							  uint8_t clock_divider);

#endif
