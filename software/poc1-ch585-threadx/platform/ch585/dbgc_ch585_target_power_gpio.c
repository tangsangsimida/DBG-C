#include "dbgc_ch585_target_power_gpio.h"

#if defined(DBGC_CH585_TARGET_POWER_GPIO_HOST_TEST)
#include "dbgc_ch585_target_power_gpio_host_regs.h"
#else
#include "wch/CH585SFR.h"
#endif

#define DBGC_CH585_PB6_MASK (1UL << 6)

/**
 * @brief 配置 PB6 的方向及 WCH EVT 定义的 GPIO 电气模式。
 * @param mode 调用方选择的 GPIO 模式。
 * @return 成功返回零，模式无效时返回非零值。
 */
int dbgc_ch585_target_power_gpio_configure(dbgc_ch585_gpio_mode_t mode)
{
	switch (mode) {
	case DBGC_CH585_GPIO_INPUT_FLOATING:
		R32_PB_PD_DRV &= ~DBGC_CH585_PB6_MASK;
		R32_PB_PU &= ~DBGC_CH585_PB6_MASK;
		R32_PB_DIR &= ~DBGC_CH585_PB6_MASK;
		return 0;
	case DBGC_CH585_GPIO_INPUT_PULL_UP:
		R32_PB_PD_DRV &= ~DBGC_CH585_PB6_MASK;
		R32_PB_PU |= DBGC_CH585_PB6_MASK;
		R32_PB_DIR &= ~DBGC_CH585_PB6_MASK;
		return 0;
	case DBGC_CH585_GPIO_INPUT_PULL_DOWN:
		R32_PB_PD_DRV |= DBGC_CH585_PB6_MASK;
		R32_PB_PU &= ~DBGC_CH585_PB6_MASK;
		R32_PB_DIR &= ~DBGC_CH585_PB6_MASK;
		return 0;
	case DBGC_CH585_GPIO_OUTPUT_PP_5MA:
		R32_PB_PD_DRV &= ~DBGC_CH585_PB6_MASK;
		R32_PB_DIR |= DBGC_CH585_PB6_MASK;
		return 0;
	case DBGC_CH585_GPIO_OUTPUT_PP_20MA:
		R32_PB_PD_DRV |= DBGC_CH585_PB6_MASK;
		R32_PB_DIR |= DBGC_CH585_PB6_MASK;
		return 0;
	default:
		return -1;
	}
}

/**
 * @brief 写入 PB6 输出锁存器的原始逻辑电平。
 * @param high 非零写高，零写低。
 * @return 写入成功返回零。
 */
int dbgc_ch585_target_power_gpio_write(uint8_t high)
{
	if (high != 0U) {
		R32_PB_SET = DBGC_CH585_PB6_MASK;
	} else {
		R32_PB_CLR = DBGC_CH585_PB6_MASK;
	}

	return 0;
}

/**
 * @brief 读取 PB6 的原始逻辑电平。
 * @param high 接收逻辑电平的输出指针。
 * @return 成功返回零，输出指针为空时返回非零值。
 */
int dbgc_ch585_target_power_gpio_read(uint8_t *high)
{
	if (high == 0) {
		return -1;
	}

	*high = ((R32_PB_PIN & DBGC_CH585_PB6_MASK) != 0U) ? 1U : 0U;
	return 0;
}
