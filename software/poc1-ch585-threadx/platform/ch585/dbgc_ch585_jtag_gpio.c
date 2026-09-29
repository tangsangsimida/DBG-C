#include "dbgc_ch585_jtag_gpio.h"

#if defined(DBGC_CH585_JTAG_GPIO_HOST_TEST)
#include "dbgc_ch585_jtag_gpio_host_regs.h"
#else
#include "wch/CH585SFR.h"
#endif

/**
 * @brief 将 JTAG 信号映射为 MCU 端口 B 位掩码。
 * @param signal 目标 JTAG 信号。
 * @param mask 接收 GPIO 位掩码的输出指针。
 * @return 成功返回零，参数无效时返回非零值。
 */
static int dbgc_ch585_jtag_gpio_mask(dbgc_ch585_jtag_signal_t signal, uint32_t *mask)
{
	if (mask == 0) {
		return -1;
	}

	switch (signal) {
	case DBGC_CH585_JTAG_SIGNAL_TCK:
		*mask = 1UL << 0;
		return 0;
	case DBGC_CH585_JTAG_SIGNAL_TMS:
		*mask = 1UL << 1;
		return 0;
	case DBGC_CH585_JTAG_SIGNAL_TDI:
		*mask = 1UL << 2;
		return 0;
	case DBGC_CH585_JTAG_SIGNAL_TDO:
		*mask = 1UL << 3;
		return 0;
	default:
		return -1;
	}
}

/**
 * @brief 按 WCH EVT GPIOB 寄存器定义配置单个 JTAG GPIO。
 * @param mask GPIO 位掩码。
 * @param mode 调用方选择的 GPIO 模式。
 * @return 成功返回零，模式无效时返回非零值。
 */
static int dbgc_ch585_jtag_gpio_apply_mode(uint32_t mask, dbgc_ch585_gpio_mode_t mode)
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
 * @brief 配置目标 JTAG 信号对应 GPIO 的方向和电气模式。
 * @param signal 目标 JTAG 信号。
 * @param mode 调用方选择的 GPIO 模式。
 * @return 成功返回零，信号或模式无效时返回非零值。
 */
int dbgc_ch585_jtag_gpio_configure(dbgc_ch585_jtag_signal_t signal, dbgc_ch585_gpio_mode_t mode)
{
	uint32_t mask;

	if (dbgc_ch585_jtag_gpio_mask(signal, &mask) != 0) {
		return -1;
	}

	return dbgc_ch585_jtag_gpio_apply_mode(mask, mode);
}

/**
 * @brief 写入目标 JTAG 信号的原始高低电平。
 * @param signal 目标 JTAG 信号。
 * @param high 非零写高，零写低。
 * @return 成功返回零，信号无效时返回非零值。
 */
int dbgc_ch585_jtag_gpio_write(dbgc_ch585_jtag_signal_t signal, uint8_t high)
{
	uint32_t mask;

	if (dbgc_ch585_jtag_gpio_mask(signal, &mask) != 0) {
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
 * @brief 读取目标 JTAG 信号的原始逻辑电平。
 * @param signal 目标 JTAG 信号。
 * @param high 接收逻辑电平的输出指针。
 * @return 成功返回零，信号无效或输出指针为空时返回非零值。
 */
int dbgc_ch585_jtag_gpio_read(dbgc_ch585_jtag_signal_t signal, uint8_t *high)
{
	uint32_t mask;

	if ((high == 0) || (dbgc_ch585_jtag_gpio_mask(signal, &mask) != 0)) {
		return -1;
	}

	*high = ((R32_PB_PIN & mask) != 0U) ? 1U : 0U;
	return 0;
}
