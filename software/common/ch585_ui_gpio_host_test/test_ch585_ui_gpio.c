#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "dbgc_ch585_ui_gpio.h"
#include "dbgc_ch585_ui_gpio_host_regs.h"

volatile uint32_t dbgc_host_R32_PB_DIR;
volatile uint32_t dbgc_host_R32_PB_PIN;
volatile uint32_t dbgc_host_R32_PB_CLR;
volatile uint32_t dbgc_host_R32_PB_PU;
volatile uint32_t dbgc_host_R32_PB_PD_DRV;
volatile uint32_t dbgc_host_R32_PB_SET;

static unsigned int checks;

#define CHECK(expression)                                               \
	do {                                                            \
		++checks;                                               \
		if (!(expression)) {                                    \
			fprintf(stderr, "检查失败：%s\n", #expression); \
			return 1;                                       \
		}                                                       \
	} while (0)

/**
 * @brief 重置 GPIOB 主机寄存器模型。
 */
static void reset_registers(void)
{
	dbgc_host_R32_PB_DIR = 0xA5A5A5A5U;
	dbgc_host_R32_PB_PIN = 0U;
	dbgc_host_R32_PB_CLR = 0U;
	dbgc_host_R32_PB_PU = 0x5A5A5A5AU;
	dbgc_host_R32_PB_PD_DRV = 0x96969696U;
	dbgc_host_R32_PB_SET = 0U;
}

/**
 * @brief 验证 PB8 和 PB9 的五种模式均只更新指定 GPIO 位。
 * @param signal UI 信号。
 * @param mask 该信号的 GPIOB 位掩码。
 * @return 全部寄存器检查通过时返回零。
 */
static int check_modes(dbgc_ch585_ui_signal_t signal, uint32_t mask)
{
	static const dbgc_ch585_gpio_mode_t modes[] = {
		DBGC_CH585_GPIO_INPUT_FLOATING,	 DBGC_CH585_GPIO_INPUT_PULL_UP,
		DBGC_CH585_GPIO_INPUT_PULL_DOWN, DBGC_CH585_GPIO_OUTPUT_PP_5MA,
		DBGC_CH585_GPIO_OUTPUT_PP_20MA,
	};
	size_t i;

	for (i = 0U; i < (sizeof(modes) / sizeof(modes[0])); ++i) {
		uint32_t expected_dir = 0xA5A5A5A5U;
		uint32_t expected_pu = 0x5A5A5A5AU;
		uint32_t expected_pd_drv = 0x96969696U;

		switch (modes[i]) {
		case DBGC_CH585_GPIO_INPUT_FLOATING:
			expected_dir &= ~mask;
			expected_pu &= ~mask;
			expected_pd_drv &= ~mask;
			break;
		case DBGC_CH585_GPIO_INPUT_PULL_UP:
			expected_dir &= ~mask;
			expected_pu |= mask;
			expected_pd_drv &= ~mask;
			break;
		case DBGC_CH585_GPIO_INPUT_PULL_DOWN:
			expected_dir &= ~mask;
			expected_pu &= ~mask;
			expected_pd_drv |= mask;
			break;
		case DBGC_CH585_GPIO_OUTPUT_PP_5MA:
			expected_dir |= mask;
			expected_pd_drv &= ~mask;
			break;
		case DBGC_CH585_GPIO_OUTPUT_PP_20MA:
			expected_dir |= mask;
			expected_pd_drv |= mask;
			break;
		default:
			return 1;
		}

		reset_registers();
		CHECK(dbgc_ch585_ui_gpio_configure(signal, modes[i]) == 0);
		CHECK(dbgc_host_R32_PB_DIR == expected_dir);
		CHECK(dbgc_host_R32_PB_PU == expected_pu);
		CHECK(dbgc_host_R32_PB_PD_DRV == expected_pd_drv);
	}

	return 0;
}

/**
 * @brief 验证基础 UI GPIO 的映射、模式和原始电平接口。
 * @return 全部检查通过时返回零。
 */
int main(void)
{
	uint8_t high = 0U;
	uint32_t saved_dir;

	CHECK(check_modes(DBGC_CH585_UI_SIGNAL_KEY_MODE, 0x00000100U) == 0);
	CHECK(check_modes(DBGC_CH585_UI_SIGNAL_LED_TARGET, 0x00000200U) == 0);

	reset_registers();
	CHECK(dbgc_ch585_ui_gpio_write(DBGC_CH585_UI_SIGNAL_KEY_MODE, 1U) == 0);
	CHECK(dbgc_host_R32_PB_SET == 0x00000100U);
	CHECK(dbgc_ch585_ui_gpio_write(DBGC_CH585_UI_SIGNAL_LED_TARGET, 0U) == 0);
	CHECK(dbgc_host_R32_PB_CLR == 0x00000200U);
	dbgc_host_R32_PB_PIN = 0x00000100U;
	CHECK(dbgc_ch585_ui_gpio_read(DBGC_CH585_UI_SIGNAL_KEY_MODE, &high) == 0);
	CHECK(high == 1U);
	dbgc_host_R32_PB_PIN = 0U;
	CHECK(dbgc_ch585_ui_gpio_read(DBGC_CH585_UI_SIGNAL_LED_TARGET, &high) == 0);
	CHECK(high == 0U);
	CHECK(dbgc_ch585_ui_gpio_read(DBGC_CH585_UI_SIGNAL_KEY_MODE, NULL) == -1);

	saved_dir = dbgc_host_R32_PB_DIR;
	CHECK(dbgc_ch585_ui_gpio_configure((dbgc_ch585_ui_signal_t)2,
					   DBGC_CH585_GPIO_INPUT_FLOATING) == -1);
	CHECK(dbgc_host_R32_PB_DIR == saved_dir);
	CHECK(dbgc_ch585_ui_gpio_write((dbgc_ch585_ui_signal_t)2, 1U) == -1);
	CHECK(dbgc_ch585_ui_gpio_read((dbgc_ch585_ui_signal_t)2, &high) == -1);
	CHECK(dbgc_ch585_ui_gpio_configure(DBGC_CH585_UI_SIGNAL_KEY_MODE,
					   (dbgc_ch585_gpio_mode_t)5) == -1);

	printf("CH585 UI GPIO host checks passed: %u checks\n", checks);
	return 0;
}
