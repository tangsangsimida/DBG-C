#include "dbgc_ch585_swd_gpio.h"

#if defined(DBGC_CH585_SWD_GPIO_HOST_TEST)
#include "dbgc_ch585_swd_gpio_host_regs.h"
#else
#include "wch/CH585SFR.h"
#endif

#define DBGC_CH585_PB0_MASK (1UL << 0)
#define DBGC_CH585_PB1_MASK (1UL << 1)

/**
 * @brief 根据 SWD 信号返回 MCU 端口 B 的 GPIO 位掩码。
 * @param signal SWDIO 或 SWCLK 信号。
 * @param mask 接收位掩码的输出指针。
 * @return 参数有效时返回零，否则返回非零值。
 */
static int dbgc_ch585_swd_gpio_mask(dbgc_ch585_swd_signal_t signal, uint32_t *mask)
{
	if (mask == 0) {
		return -1;
	}

	switch (signal) {
	case DBGC_CH585_SWD_SIGNAL_SWDIO:
		*mask = DBGC_CH585_PB1_MASK;
		return 0;
	case DBGC_CH585_SWD_SIGNAL_SWCLK:
		*mask = DBGC_CH585_PB0_MASK;
		return 0;
	default:
		return -1;
	}
}

/**
 * @brief 配置目标 SWD GPIO 的输入输出模式。
 * @param signal 要配置的 SWD 信号。
 * @param mode 调用方选择的 GPIO 电气模式。
 * @return 配置成功时返回零，否则返回非零值。
 */
int dbgc_ch585_swd_gpio_configure(dbgc_ch585_swd_signal_t signal, dbgc_ch585_gpio_mode_t mode)
{
	uint32_t mask;

	if (dbgc_ch585_swd_gpio_mask(signal, &mask) != 0) {
		return -1;
	}

	switch (mode) {
	case DBGC_CH585_GPIO_INPUT_FLOATING:
		R32_PB_PD_DRV &= ~mask;
		R32_PB_PU &= ~mask;
		R32_PB_DIR &= ~mask;
		return 0;
	case DBGC_CH585_GPIO_INPUT_PULL_UP:
		R32_PB_PD_DRV &= ~mask;
		R32_PB_PU |= mask;
		R32_PB_DIR &= ~mask;
		return 0;
	case DBGC_CH585_GPIO_INPUT_PULL_DOWN:
		R32_PB_PD_DRV |= mask;
		R32_PB_PU &= ~mask;
		R32_PB_DIR &= ~mask;
		return 0;
	case DBGC_CH585_GPIO_OUTPUT_PP_5MA:
		R32_PB_PD_DRV &= ~mask;
		R32_PB_DIR |= mask;
		return 0;
	case DBGC_CH585_GPIO_OUTPUT_PP_20MA:
		R32_PB_PD_DRV |= mask;
		R32_PB_DIR |= mask;
		return 0;
	default:
		return -1;
	}
}

/**
 * @brief 写入目标 SWD GPIO 的原始高低电平。
 * @param signal 要写入的 SWD 信号。
 * @param high 非零写高，零写低。
 * @return 写入成功时返回零，否则返回非零值。
 */
int dbgc_ch585_swd_gpio_write(dbgc_ch585_swd_signal_t signal, uint8_t high)
{
	uint32_t mask;

	if (dbgc_ch585_swd_gpio_mask(signal, &mask) != 0) {
		return -1;
	}

	if (high != 0U) {
		R32_PB_SET = mask;
	} else {
		R32_PB_CLR = mask;
	}

	return 0;
}

/**
 * @brief 读取目标 SWD GPIO 的原始逻辑电平。
 * @param signal 要读取的 SWD 信号。
 * @param high 接收逻辑电平的输出指针。
 * @return 读取成功时返回零，否则返回非零值。
 */
int dbgc_ch585_swd_gpio_read(dbgc_ch585_swd_signal_t signal, uint8_t *high)
{
	uint32_t mask;

	if ((high == 0) || (dbgc_ch585_swd_gpio_mask(signal, &mask) != 0)) {
		return -1;
	}

	*high = ((R32_PB_PIN & mask) != 0U) ? 1U : 0U;
	return 0;
}
