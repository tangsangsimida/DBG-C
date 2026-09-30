#include "dbgc_ch585_ui_gpio.h"

#if defined(DBGC_CH585_UI_GPIO_HOST_TEST)
#include "dbgc_ch585_ui_gpio_host_regs.h"
#else
#include "wch/CH585SFR.h"
#endif

#define DBGC_CH585_PB8_MASK (1UL << 8)
#define DBGC_CH585_PB9_MASK (1UL << 9)

/**
 * @brief 将已分配的 UI 信号映射为 GPIOB 位掩码。
 * @param signal UI 信号。
 * @param mask 接收位掩码的输出指针。
 * @return 成功返回零，信号或指针无效时返回非零值。
 */
static int dbgc_ch585_ui_gpio_mask(dbgc_ch585_ui_signal_t signal, uint32_t *mask)
{
	if (mask == 0) {
		return -1;
	}

	switch (signal) {
	case DBGC_CH585_UI_SIGNAL_KEY_MODE:
		*mask = DBGC_CH585_PB8_MASK;
		return 0;
	case DBGC_CH585_UI_SIGNAL_LED_TARGET:
		*mask = DBGC_CH585_PB9_MASK;
		return 0;
	default:
		return -1;
	}
}

/**
 * @brief 按 WCH EVT GPIOB_ModeCfg() 对应语义配置一个 GPIO 模式。
 * @param mask GPIOB 位掩码。
 * @param mode 调用方选择的 GPIO 模式。
 * @return 成功返回零，模式无效时返回非零值。
 */
static int dbgc_ch585_ui_gpio_apply_mode(uint32_t mask, dbgc_ch585_gpio_mode_t mode)
{
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
 * @brief 配置 PB8 KEY_MODE 或 PB9 LED_TARGET 的调用方指定模式。
 * @param signal UI 信号。
 * @param mode GPIO 模式。
 * @return 成功返回零，信号或模式无效时返回非零值。
 */
int dbgc_ch585_ui_gpio_configure(dbgc_ch585_ui_signal_t signal, dbgc_ch585_gpio_mode_t mode)
{
	uint32_t mask;

	if (dbgc_ch585_ui_gpio_mask(signal, &mask) != 0) {
		return -1;
	}

	return dbgc_ch585_ui_gpio_apply_mode(mask, mode);
}

/**
 * @brief 写入已分配 UI GPIO 的原始输出锁存器电平。
 * @param signal UI 信号。
 * @param high 非零写高，零写低。
 * @return 成功返回零，信号无效时返回非零值。
 */
int dbgc_ch585_ui_gpio_write(dbgc_ch585_ui_signal_t signal, uint8_t high)
{
	uint32_t mask;

	if (dbgc_ch585_ui_gpio_mask(signal, &mask) != 0) {
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
 * @brief 读取已分配 UI GPIO 的原始逻辑电平。
 * @param signal UI 信号。
 * @param high 接收逻辑电平的输出指针。
 * @return 成功返回零，信号或指针无效时返回非零值。
 */
int dbgc_ch585_ui_gpio_read(dbgc_ch585_ui_signal_t signal, uint8_t *high)
{
	uint32_t mask;

	if ((high == 0) || (dbgc_ch585_ui_gpio_mask(signal, &mask) != 0)) {
		return -1;
	}

	*high = ((R32_PB_PIN & mask) != 0U) ? 1U : 0U;
	return 0;
}
