#include <stdint.h>
#include <stdio.h>

#include "dbgc_ch585_target_power_gpio.h"
#include "dbgc_ch585_target_power_gpio_host_regs.h"

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
 * @brief 重置 PB6 GPIO 主机寄存器模型。
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
 * @brief 检查某种 GPIO 模式只修改 PB6 对应寄存器位。
 * @param mode 待检查模式。
 * @param direction 期望方向寄存器。
 * @param pull_up 期望上拉寄存器。
 * @param pull_down_drive 期望下拉/驱动寄存器。
 */
static int check_mode(dbgc_ch585_gpio_mode_t mode, uint32_t direction, uint32_t pull_up,
		      uint32_t pull_down_drive)
{
	reset_registers();
	CHECK(dbgc_ch585_target_power_gpio_configure(mode) == 0);
	CHECK(dbgc_host_R32_PB_DIR == direction);
	CHECK(dbgc_host_R32_PB_PU == pull_up);
	CHECK(dbgc_host_R32_PB_PD_DRV == pull_down_drive);
	return 0;
}

/**
 * @brief 验证 PB6 目标电源控制 GPIO 的原始模式、电平写入和读回。
 * @return 全部检查通过时返回零。
 */
int main(void)
{
	uint8_t high = 0U;
	uint32_t saved_direction;

	CHECK(check_mode(DBGC_CH585_GPIO_INPUT_FLOATING, 0xA5A5A5A5U, 0x5A5A5A1AU, 0x96969696U) ==
	      0);
	CHECK(check_mode(DBGC_CH585_GPIO_INPUT_PULL_UP, 0xA5A5A5A5U, 0x5A5A5A5AU, 0x96969696U) ==
	      0);
	CHECK(check_mode(DBGC_CH585_GPIO_INPUT_PULL_DOWN, 0xA5A5A5A5U, 0x5A5A5A1AU, 0x969696D6U) ==
	      0);
	CHECK(check_mode(DBGC_CH585_GPIO_OUTPUT_PP_5MA, 0xA5A5A5E5U, 0x5A5A5A5AU, 0x96969696U) ==
	      0);
	CHECK(check_mode(DBGC_CH585_GPIO_OUTPUT_PP_20MA, 0xA5A5A5E5U, 0x5A5A5A5AU, 0x969696D6U) ==
	      0);

	reset_registers();
	CHECK(dbgc_ch585_target_power_gpio_write(1U) == 0);
	CHECK(dbgc_host_R32_PB_SET == 0x00000040U);
	CHECK(dbgc_ch585_target_power_gpio_write(0U) == 0);
	CHECK(dbgc_host_R32_PB_CLR == 0x00000040U);
	dbgc_host_R32_PB_PIN = 0x00000040U;
	CHECK(dbgc_ch585_target_power_gpio_read(&high) == 0);
	CHECK(high == 1U);
	dbgc_host_R32_PB_PIN = 0U;
	CHECK(dbgc_ch585_target_power_gpio_read(&high) == 0);
	CHECK(high == 0U);
	CHECK(dbgc_ch585_target_power_gpio_read(NULL) == -1);

	saved_direction = dbgc_host_R32_PB_DIR;
	CHECK(dbgc_ch585_target_power_gpio_configure((dbgc_ch585_gpio_mode_t)5) == -1);
	CHECK(dbgc_host_R32_PB_DIR == saved_direction);

	printf("CH585 Target Power raw GPIO host checks passed: %u checks\n", checks);
	return 0;
}
