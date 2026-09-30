#ifndef DBGC_CH585_UI_GPIO_H
#define DBGC_CH585_UI_GPIO_H

#include <stdint.h>

#include "dbgc_ch585_gpio_mode.h"

/**
 * @brief 定义已分配给基础用户交互功能的 GPIO 信号。
 */
typedef enum {
	DBGC_CH585_UI_SIGNAL_KEY_MODE = 0,
	DBGC_CH585_UI_SIGNAL_LED_TARGET
} dbgc_ch585_ui_signal_t;

/**
 * @brief 配置 PB8 KEY_MODE 或 PB9 LED_TARGET 的 GPIO 模式。
 *
 * 调用方负责选择电气兼容的模式。本接口不配置按键去抖、唤醒策略、LED 极性、默认态，
 * 也不修改 PB18/PB19 的 RF 天线开关复用。
 * @param signal 已分配的 UI 信号。
 * @param mode 调用方选择的 GPIO 模式。
 * @return 成功返回零，信号或模式无效时返回非零值。
 */
int dbgc_ch585_ui_gpio_configure(dbgc_ch585_ui_signal_t signal, dbgc_ch585_gpio_mode_t mode);

/**
 * @brief 写入 PB8 或 PB9 的原始输出锁存器电平。
 * @param signal 已分配的 UI 信号。
 * @param high 非零写高，零写低。
 * @return 成功返回零，信号无效时返回非零值。
 */
int dbgc_ch585_ui_gpio_write(dbgc_ch585_ui_signal_t signal, uint8_t high);

/**
 * @brief 读取 PB8 或 PB9 的原始 GPIO 电平。
 * @param signal 已分配的 UI 信号。
 * @param high 接收原始逻辑电平的输出指针。
 * @return 成功返回零，信号或输出指针无效时返回非零值。
 */
int dbgc_ch585_ui_gpio_read(dbgc_ch585_ui_signal_t signal, uint8_t *high);

#endif
