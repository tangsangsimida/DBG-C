#include "dbgc_ch585_spi1_gpio.h"
#include "dbgc_ch585_spi1_gpio_host_regs.h"

#include <stdio.h>

volatile uint32_t dbgc_host_R32_PA_DIR;
volatile uint32_t dbgc_host_R32_PA_SET;
volatile uint32_t dbgc_host_R32_PA_CLR;
volatile uint32_t dbgc_host_R32_PA_PIN;
volatile uint32_t dbgc_host_R32_PA_PU;
volatile uint32_t dbgc_host_R32_PA_PD_DRV;

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
 * @brief 验证 SPI1 四个 GPIO 的位掩码、模式映射和 PA3 原始电平写入。
 * @return 所有主机寄存器模型检查通过时返回零。
 */
int main(void)
{
	const dbgc_ch585_spi1_signal_t signals[] = {
		DBGC_CH585_SPI1_SIGNAL_SCK,
		DBGC_CH585_SPI1_SIGNAL_MOSI,
		DBGC_CH585_SPI1_SIGNAL_MISO,
		DBGC_CH585_SPI1_SIGNAL_CS,
	};
	const uint32_t masks[] = { 1UL << 0, 1UL << 1, 1UL << 2, 1UL << 3 };
	const dbgc_ch585_gpio_mode_t modes[] = {
		DBGC_CH585_GPIO_INPUT_FLOATING,	 DBGC_CH585_GPIO_INPUT_PULL_UP,
		DBGC_CH585_GPIO_INPUT_PULL_DOWN, DBGC_CH585_GPIO_OUTPUT_PP_5MA,
		DBGC_CH585_GPIO_OUTPUT_PP_20MA,
	};
	size_t signal_index;
	size_t mode_index;

	for (signal_index = 0U; signal_index < sizeof(signals) / sizeof(signals[0]);
	     ++signal_index) {
		for (mode_index = 0U; mode_index < sizeof(modes) / sizeof(modes[0]); ++mode_index) {
			dbgc_host_R32_PA_DIR = UINT32_MAX;
			dbgc_host_R32_PA_PU = 0U;
			dbgc_host_R32_PA_PD_DRV = 0U;
			CHECK(dbgc_ch585_spi1_gpio_configure(signals[signal_index],
							     modes[mode_index]) == 0);
			CHECK(dbgc_host_R32_PA_DIR ==
			      ((modes[mode_index] >= DBGC_CH585_GPIO_OUTPUT_PP_5MA) ?
				       UINT32_MAX :
				       (UINT32_MAX & ~masks[signal_index])));
			CHECK(((dbgc_host_R32_PA_PU & masks[signal_index]) != 0U) ==
			      (modes[mode_index] == DBGC_CH585_GPIO_INPUT_PULL_UP));
			CHECK(((dbgc_host_R32_PA_PD_DRV & masks[signal_index]) != 0U) ==
			      ((modes[mode_index] == DBGC_CH585_GPIO_INPUT_PULL_DOWN) ||
			       (modes[mode_index] == DBGC_CH585_GPIO_OUTPUT_PP_20MA)));
			CHECK((dbgc_host_R32_PA_DIR & ~masks[signal_index]) ==
			      (UINT32_MAX & ~masks[signal_index]));
		}
	}

	CHECK(dbgc_ch585_spi1_gpio_configure((dbgc_ch585_spi1_signal_t)-1,
					     DBGC_CH585_GPIO_INPUT_FLOATING) != 0);
	CHECK(dbgc_ch585_spi1_gpio_configure(DBGC_CH585_SPI1_SIGNAL_SCK,
					     (dbgc_ch585_gpio_mode_t)5) != 0);
	dbgc_host_R32_PA_SET = 0U;
	dbgc_host_R32_PA_CLR = 0U;
	dbgc_ch585_spi1_gpio_write_cs(1U);
	CHECK(dbgc_host_R32_PA_SET == (1UL << 3));
	dbgc_ch585_spi1_gpio_write_cs(0U);
	CHECK(dbgc_host_R32_PA_CLR == (1UL << 3));
	printf("CH585 SPI1 GPIO host checks passed: %u checks\n", checks);
	return 0;
}
