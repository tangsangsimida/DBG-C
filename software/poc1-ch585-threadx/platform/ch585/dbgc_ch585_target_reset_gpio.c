#include "dbgc_ch585_target_reset_gpio.h"

#if defined(DBGC_CH585_TARGET_RESET_GPIO_HOST_TEST)
#include "dbgc_ch585_target_reset_gpio_host_regs.h"
#else
#include "wch/CH585SFR.h"
#endif

#define DBGC_CH585_PB5_MASK (1UL << 5)

/**
 * @brief 配置 Target nRESET GPIO 的输入输出模式。
 * @param mode 调用方选择的 GPIO 电气模式。
 * @return 配置成功时返回零，否则返回非零值。
 */
int dbgc_ch585_target_reset_gpio_configure(dbgc_ch585_gpio_mode_t mode)
{
	switch (mode) {
	case DBGC_CH585_GPIO_INPUT_FLOATING:
		R32_PB_PD_DRV &= ~DBGC_CH585_PB5_MASK;
		R32_PB_PU &= ~DBGC_CH585_PB5_MASK;
		R32_PB_DIR &= ~DBGC_CH585_PB5_MASK;
		return 0;
	case DBGC_CH585_GPIO_INPUT_PULL_UP:
		R32_PB_PD_DRV &= ~DBGC_CH585_PB5_MASK;
		R32_PB_PU |= DBGC_CH585_PB5_MASK;
		R32_PB_DIR &= ~DBGC_CH585_PB5_MASK;
		return 0;
	case DBGC_CH585_GPIO_INPUT_PULL_DOWN:
		R32_PB_PD_DRV |= DBGC_CH585_PB5_MASK;
		R32_PB_PU &= ~DBGC_CH585_PB5_MASK;
		R32_PB_DIR &= ~DBGC_CH585_PB5_MASK;
		return 0;
	case DBGC_CH585_GPIO_OUTPUT_PP_5MA:
		R32_PB_PD_DRV &= ~DBGC_CH585_PB5_MASK;
		R32_PB_DIR |= DBGC_CH585_PB5_MASK;
		return 0;
	case DBGC_CH585_GPIO_OUTPUT_PP_20MA:
		R32_PB_PD_DRV |= DBGC_CH585_PB5_MASK;
		R32_PB_DIR |= DBGC_CH585_PB5_MASK;
		return 0;
	default:
		return -1;
	}
}

/**
 * @brief 写入 Target nRESET GPIO 的原始高低电平。
 * @param high 非零写高，零写低；此接口不定义复位有效电平。
 * @return 写入成功时返回零，否则返回非零值。
 */
int dbgc_ch585_target_reset_gpio_write(uint8_t high)
{
	if (high != 0U) {
		R32_PB_SET = DBGC_CH585_PB5_MASK;
	} else {
		R32_PB_CLR = DBGC_CH585_PB5_MASK;
	}

	return 0;
}

/**
 * @brief 读取 Target nRESET GPIO 的原始逻辑电平。
 * @param high 接收逻辑电平的输出指针。
 * @return 读取成功时返回零，否则返回非零值。
 */
int dbgc_ch585_target_reset_gpio_read(uint8_t *high)
{
	if (high == 0) {
		return -1;
	}

	*high = ((R32_PB_PIN & DBGC_CH585_PB5_MASK) != 0U) ? 1U : 0U;
	return 0;
}
