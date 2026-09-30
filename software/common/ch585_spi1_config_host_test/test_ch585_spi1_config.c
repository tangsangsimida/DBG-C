#include "dbgc_ch585_spi1_config.h"
#include "dbgc_ch585_spi1_config_host_regs.h"

#include <stdio.h>

volatile uint8_t dbgc_host_R8_SPI1_CTRL_MOD;
volatile uint8_t dbgc_host_R8_SPI1_CTRL_CFG;
volatile uint8_t dbgc_host_R8_SPI1_INT_FLAG;
volatile uint8_t dbgc_host_R8_SPI1_CLOCK_DIV;

static unsigned int checks;

#define CHECK(condition)                                                               \
	do {                                                                           \
		++checks;                                                              \
		if (!(condition)) {                                                    \
			fprintf(stderr, "第 %u 项检查失败：%s\n", checks, #condition); \
			return 1;                                                      \
		}                                                                      \
	} while (0)

/**
 * @brief 验证 SPI1 配置映射、参数边界及忙状态拒绝。
 * @return 所有主机寄存器模型检查通过时返回零。
 */
int main(void)
{
	uint8_t original_mode;
	uint8_t original_config;
	uint8_t original_divider;

	dbgc_host_R8_SPI1_INT_FLAG = RB_SPI_FREE;
	dbgc_host_R8_SPI1_CTRL_MOD = 0xFFU;
	dbgc_host_R8_SPI1_CTRL_CFG = 0U;
	dbgc_host_R8_SPI1_CLOCK_DIV = 0U;

	CHECK(dbgc_ch585_spi1_configure(DBGC_CH585_SPI1_MODE_0, DBGC_CH585_SPI1_BIT_ORDER_MSB_FIRST,
					2U) == DBGC_CH585_SPI1_CONFIG_OK);
	CHECK(dbgc_host_R8_SPI1_CTRL_MOD == (RB_SPI_MOSI_OE | RB_SPI_SCK_OE));
	CHECK(dbgc_host_R8_SPI1_CTRL_CFG == (RB_SPI_AUTO_IF | RB_SPI_MST_DLY_EN));
	CHECK(dbgc_host_R8_SPI1_CLOCK_DIV == 2U);

	CHECK(dbgc_ch585_spi1_configure(DBGC_CH585_SPI1_MODE_3, DBGC_CH585_SPI1_BIT_ORDER_LSB_FIRST,
					254U) == DBGC_CH585_SPI1_CONFIG_OK);
	CHECK(dbgc_host_R8_SPI1_CTRL_MOD == (RB_SPI_MOSI_OE | RB_SPI_SCK_OE | RB_SPI_MST_SCK_MOD));
	CHECK(dbgc_host_R8_SPI1_CTRL_CFG == (RB_SPI_AUTO_IF | RB_SPI_BIT_ORDER));
	CHECK(dbgc_host_R8_SPI1_CLOCK_DIV == 254U);

	original_mode = dbgc_host_R8_SPI1_CTRL_MOD;
	original_config = dbgc_host_R8_SPI1_CTRL_CFG;
	original_divider = dbgc_host_R8_SPI1_CLOCK_DIV;
	CHECK(dbgc_ch585_spi1_configure(DBGC_CH585_SPI1_MODE_0, DBGC_CH585_SPI1_BIT_ORDER_MSB_FIRST,
					1U) == DBGC_CH585_SPI1_CONFIG_INVALID_ARGUMENT);
	CHECK(dbgc_ch585_spi1_configure(DBGC_CH585_SPI1_MODE_0, DBGC_CH585_SPI1_BIT_ORDER_MSB_FIRST,
					255U) == DBGC_CH585_SPI1_CONFIG_INVALID_ARGUMENT);
	CHECK(dbgc_ch585_spi1_configure((dbgc_ch585_spi1_mode_t)2,
					DBGC_CH585_SPI1_BIT_ORDER_MSB_FIRST,
					4U) == DBGC_CH585_SPI1_CONFIG_INVALID_ARGUMENT);
	CHECK(dbgc_ch585_spi1_configure(DBGC_CH585_SPI1_MODE_0, (dbgc_ch585_spi1_bit_order_t)2,
					4U) == DBGC_CH585_SPI1_CONFIG_INVALID_ARGUMENT);
	CHECK(dbgc_host_R8_SPI1_CTRL_MOD == original_mode);
	CHECK(dbgc_host_R8_SPI1_CTRL_CFG == original_config);
	CHECK(dbgc_host_R8_SPI1_CLOCK_DIV == original_divider);

	dbgc_host_R8_SPI1_INT_FLAG = 0U;
	CHECK(dbgc_ch585_spi1_configure(DBGC_CH585_SPI1_MODE_0, DBGC_CH585_SPI1_BIT_ORDER_MSB_FIRST,
					4U) == DBGC_CH585_SPI1_CONFIG_BUSY);
	CHECK(dbgc_host_R8_SPI1_CTRL_MOD == original_mode);
	CHECK(dbgc_host_R8_SPI1_CTRL_CFG == original_config);
	CHECK(dbgc_host_R8_SPI1_CLOCK_DIV == original_divider);

	printf("CH585 SPI1 config host checks passed: %u checks\n", checks);
	return 0;
}
