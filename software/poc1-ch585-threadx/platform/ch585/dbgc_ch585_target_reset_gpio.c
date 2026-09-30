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

/**
 * @brief 检查目标复位断言时使用的 GPIO 推挽输出模式。
 * @param mode 待检查 GPIO 模式。
 * @return 为 5 mA 或 20 mA 推挽输出模式时返回非零值。
 */
static int dbgc_ch585_target_reset_gpio_is_output_mode(dbgc_ch585_gpio_mode_t mode)
{
	return (mode == DBGC_CH585_GPIO_OUTPUT_PP_5MA) || (mode == DBGC_CH585_GPIO_OUTPUT_PP_20MA);
}

/**
 * @brief 检查释放 Target nRESET 时使用的 GPIO 输入模式。
 * @param mode 待检查 GPIO 模式。
 * @return 为浮空、上拉或下拉输入模式时返回非零值。
 */
static int dbgc_ch585_target_reset_gpio_is_input_mode(dbgc_ch585_gpio_mode_t mode)
{
	return (mode == DBGC_CH585_GPIO_INPUT_FLOATING) || (mode == DBGC_CH585_GPIO_INPUT_PULL_UP);
}

/**
 * @brief 按 CMSIS-DAP nRESET 位值切换 PB5 方向以释放或断言复位。
 * @param bit 零表示断言复位，一表示释放复位。
 * @param output_mode 断言时由调用方选择的推挽输出模式。
 * @param released_mode 释放时由调用方选择的输入模式。
 * @return 成功返回零，参数无效时返回非零值。
 */
int dbgc_ch585_target_reset_gpio_set_nreset(uint8_t bit, dbgc_ch585_gpio_mode_t output_mode,
					    dbgc_ch585_gpio_mode_t released_mode)
{
	if ((bit > 1U) || !dbgc_ch585_target_reset_gpio_is_output_mode(output_mode) ||
	    !dbgc_ch585_target_reset_gpio_is_input_mode(released_mode)) {
		return -1;
	}

	if (bit != 0U) {
		return dbgc_ch585_target_reset_gpio_configure(released_mode);
	}

	if (dbgc_ch585_target_reset_gpio_configure(DBGC_CH585_GPIO_INPUT_FLOATING) != 0) {
		return -1;
	}
	if (dbgc_ch585_target_reset_gpio_write(0U) != 0) {
		return -1;
	}

	return dbgc_ch585_target_reset_gpio_configure(output_mode);
}
