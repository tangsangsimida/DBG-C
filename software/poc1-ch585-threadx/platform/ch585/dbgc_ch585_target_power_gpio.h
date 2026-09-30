#ifndef DBGC_CH585_TARGET_POWER_GPIO_H
#define DBGC_CH585_TARGET_POWER_GPIO_H

#include <stdint.h>

#include "dbgc_ch585_gpio_mode.h"

/**
 * @brief 配置 MCU-001 分配给 TARGET_PWR_EN 的 PB6 GPIO 模式。
 *
 * 调用方必须选择与外部电路兼容的模式，并串行化 GPIOB 配置。本接口不定义目标供电的
 * 有效电平、默认状态、负载开关行为或电气保护策略。
 * @param mode 调用方评审后选择的 GPIO 模式。
 * @return 成功返回零，模式无效时返回非零值。
 */
int dbgc_ch585_target_power_gpio_configure(dbgc_ch585_gpio_mode_t mode);

/**
 * @brief 将 PB6 输出锁存器设为原始高低电平。
 *
 * 调用方须先按电气设计将 PB6 配置为输出，并自行定义高低电平与供电状态的关系。
 * @param high 非零写高，零写低。
 * @return 写入成功返回零。
 */
int dbgc_ch585_target_power_gpio_write(uint8_t high);

/**
 * @brief 读取 PB6 GPIO 输入逻辑电平。
 * @param high 接收原始逻辑电平的输出指针。
 * @return 成功返回零，输出指针为空时返回非零值。
 */
int dbgc_ch585_target_power_gpio_read(uint8_t *high);

#endif
