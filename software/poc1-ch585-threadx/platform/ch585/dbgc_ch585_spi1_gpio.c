#include "dbgc_ch585_spi1_gpio.h"

#if defined(DBGC_CH585_SPI1_GPIO_HOST_TEST)
#include "dbgc_ch585_spi1_gpio_host_regs.h"
#else
#include "wch/CH585SFR.h"
#endif

#define DBGC_CH585_PA0_MASK (1UL << 0)
#define DBGC_CH585_PA1_MASK (1UL << 1)
#define DBGC_CH585_PA2_MASK (1UL << 2)
#define DBGC_CH585_PA3_MASK (1UL << 3)

/**
 * @brief 将 SPI1 信号映射到数据手册定义的 GPIOA 位掩码。
 * @param signal SPI1 信号。
 * @param mask 接收位掩码的输出指针。
 * @return 参数有效时返回零，否则返回非零值。
 */
static int dbgc_ch585_spi1_gpio_mask(dbgc_ch585_spi1_signal_t signal, uint32_t *mask)
{
	if (mask == 0) {
		return -1;
	}

	switch (signal) {
	case DBGC_CH585_SPI1_SIGNAL_SCK:
		*mask = DBGC_CH585_PA0_MASK;
		return 0;
	case DBGC_CH585_SPI1_SIGNAL_MOSI:
		*mask = DBGC_CH585_PA1_MASK;
		return 0;
	case DBGC_CH585_SPI1_SIGNAL_MISO:
		*mask = DBGC_CH585_PA2_MASK;
		return 0;
	case DBGC_CH585_SPI1_SIGNAL_CS:
		*mask = DBGC_CH585_PA3_MASK;
		return 0;
	default:
		return -1;
	}
}

/**
 * @brief 按 WCH EVT GPIOA_ModeCfg() 定义写入一个 GPIO 模式。
 * @param mask 需要配置的 GPIOA 位掩码。
 * @param mode 调用方选择的 GPIO 模式。
 * @return 模式有效时返回零，否则返回非零值。
 */
static int dbgc_ch585_spi1_gpio_apply_mode(uint32_t mask, dbgc_ch585_gpio_mode_t mode)
{
	switch (mode) {
	case DBGC_CH585_GPIO_INPUT_FLOATING:
		R32_PA_PD_DRV &= ~mask;
		R32_PA_PU &= ~mask;
		R32_PA_DIR &= ~mask;
		return 0;
	case DBGC_CH585_GPIO_INPUT_PULL_UP:
		R32_PA_PD_DRV &= ~mask;
		R32_PA_PU |= mask;
		R32_PA_DIR &= ~mask;
		return 0;
	case DBGC_CH585_GPIO_INPUT_PULL_DOWN:
		R32_PA_PD_DRV |= mask;
		R32_PA_PU &= ~mask;
		R32_PA_DIR &= ~mask;
		return 0;
	case DBGC_CH585_GPIO_OUTPUT_PP_5MA:
		R32_PA_PD_DRV &= ~mask;
		R32_PA_DIR |= mask;
		return 0;
	case DBGC_CH585_GPIO_OUTPUT_PP_20MA:
		R32_PA_PD_DRV |= mask;
		R32_PA_DIR |= mask;
		return 0;
	default:
		return -1;
	}
}

/**
 * @brief 配置 SPI1 信号对应 GPIO 的调用方指定模式。
 * @param signal SPI1 信号。
 * @param mode 调用方选择的 GPIO 模式。
 * @return 成功返回零，参数无效时返回非零值。
 */
int dbgc_ch585_spi1_gpio_configure(dbgc_ch585_spi1_signal_t signal, dbgc_ch585_gpio_mode_t mode)
{
	uint32_t mask;

	if (dbgc_ch585_spi1_gpio_mask(signal, &mask) != 0) {
		return -1;
	}

	return dbgc_ch585_spi1_gpio_apply_mode(mask, mode);
}

/**
 * @brief 写入 PA3 片选 GPIO 的原始输出锁存器电平。
 * @param high 非零写高，零写低。
 */
void dbgc_ch585_spi1_gpio_write_cs(uint8_t high)
{
	if (high != 0U) {
		R32_PA_SET = DBGC_CH585_PA3_MASK;
	} else {
		R32_PA_CLR = DBGC_CH585_PA3_MASK;
	}
}
