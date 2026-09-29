#ifndef DBGC_CH585_JTAG_GPIO_H
#define DBGC_CH585_JTAG_GPIO_H

#include <stdint.h>

#include "dbgc_ch585_gpio_mode.h"

typedef enum {
	DBGC_CH585_JTAG_SIGNAL_TCK = 0,
	DBGC_CH585_JTAG_SIGNAL_TMS,
	DBGC_CH585_JTAG_SIGNAL_TDI,
	DBGC_CH585_JTAG_SIGNAL_TDO
} dbgc_ch585_jtag_signal_t;

/* Pin mapping follows MCU-001: TCK/TMS/TDI/TDO use PB0/PB1/PB2/PB3. These
 * primitives do not select product electrical policy or provide synchronization;
 * callers must serialize GPIOB access and choose each signal mode explicitly.
 */

/**
 * @brief 配置目标 JTAG 信号对应 GPIO 的方向和电气模式。
 * @param signal 目标 JTAG 信号。
 * @param mode 调用方选择的 GPIO 模式。
 * @return 成功返回零，信号或模式无效时返回非零值。
 */
int dbgc_ch585_jtag_gpio_configure(dbgc_ch585_jtag_signal_t signal, dbgc_ch585_gpio_mode_t mode);

/**
 * @brief 写入目标 JTAG 信号的原始高低电平。
 * @param signal 目标 JTAG 信号。
 * @param high 非零写高，零写低。
 * @return 成功返回零，信号无效时返回非零值。
 */
int dbgc_ch585_jtag_gpio_write(dbgc_ch585_jtag_signal_t signal, uint8_t high);

/**
 * @brief 读取目标 JTAG 信号的原始逻辑电平。
 * @param signal 目标 JTAG 信号。
 * @param high 接收逻辑电平的输出指针。
 * @return 成功返回零，信号无效或输出指针为空时返回非零值。
 */
int dbgc_ch585_jtag_gpio_read(dbgc_ch585_jtag_signal_t signal, uint8_t *high);

#endif
