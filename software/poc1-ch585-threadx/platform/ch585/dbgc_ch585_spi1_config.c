#include "dbgc_ch585_spi1_config.h"

#if defined(DBGC_CH585_SPI1_CONFIG_HOST_TEST)
#include "dbgc_ch585_spi1_config_host_regs.h"
#else
#include "wch/CH585SFR.h"
#endif

/**
 * @brief 检查 SPI1 时钟模式是否受 CH585 数据手册支持。
 * @param mode 待检查的时钟模式。
 * @return 有效时返回非零值。
 */
static int dbgc_ch585_spi1_mode_is_valid(dbgc_ch585_spi1_mode_t mode)
{
	return (mode == DBGC_CH585_SPI1_MODE_0) || (mode == DBGC_CH585_SPI1_MODE_3);
}

/**
 * @brief 检查数据位序枚举值。
 * @param bit_order 待检查的数据位序。
 * @return 有效时返回非零值。
 */
static int dbgc_ch585_spi1_bit_order_is_valid(dbgc_ch585_spi1_bit_order_t bit_order)
{
	return (bit_order == DBGC_CH585_SPI1_BIT_ORDER_MSB_FIRST) ||
	       (bit_order == DBGC_CH585_SPI1_BIT_ORDER_LSB_FIRST);
}

/**
 * @brief 配置 CH585 SPI1 的主机模式寄存器。
 * @param mode SPI 时钟模式。
 * @param bit_order 数据位序。
 * @param clock_divider 主机时钟分频系数。
 * @return 配置结果。
 */
dbgc_ch585_spi1_config_status_t dbgc_ch585_spi1_configure(dbgc_ch585_spi1_mode_t mode,
							  dbgc_ch585_spi1_bit_order_t bit_order,
							  uint8_t clock_divider)
{
	if (!dbgc_ch585_spi1_mode_is_valid(mode) ||
	    !dbgc_ch585_spi1_bit_order_is_valid(bit_order) || (clock_divider < 2U) ||
	    (clock_divider > 254U)) {
		return DBGC_CH585_SPI1_CONFIG_INVALID_ARGUMENT;
	}
	if ((R8_SPI1_INT_FLAG & RB_SPI_FREE) == 0U) {
		return DBGC_CH585_SPI1_CONFIG_BUSY;
	}

	R8_SPI1_CTRL_MOD = RB_SPI_ALL_CLEAR;
	R8_SPI1_CTRL_MOD = RB_SPI_MOSI_OE | RB_SPI_SCK_OE;
	R8_SPI1_CTRL_CFG |= RB_SPI_AUTO_IF;
	if (mode == DBGC_CH585_SPI1_MODE_3) {
		R8_SPI1_CTRL_MOD |= RB_SPI_MST_SCK_MOD;
	} else {
		R8_SPI1_CTRL_MOD &= (uint8_t)~RB_SPI_MST_SCK_MOD;
	}
	if (bit_order == DBGC_CH585_SPI1_BIT_ORDER_LSB_FIRST) {
		R8_SPI1_CTRL_CFG |= RB_SPI_BIT_ORDER;
	} else {
		R8_SPI1_CTRL_CFG &= (uint8_t)~RB_SPI_BIT_ORDER;
	}
	R8_SPI1_CLOCK_DIV = clock_divider;
	if (clock_divider == 2U) {
		R8_SPI1_CTRL_CFG |= RB_SPI_MST_DLY_EN;
	} else {
		R8_SPI1_CTRL_CFG &= (uint8_t)~RB_SPI_MST_DLY_EN;
	}

	return DBGC_CH585_SPI1_CONFIG_OK;
}
