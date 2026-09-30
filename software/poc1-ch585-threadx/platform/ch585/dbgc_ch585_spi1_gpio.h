#ifndef DBGC_CH585_SPI1_GPIO_H
#define DBGC_CH585_SPI1_GPIO_H

#include "dbgc_ch585_gpio_mode.h"

#include <stdint.h>

/**
 * @brief 定义 SPI1 信号名称。
 */
typedef enum {
	DBGC_CH585_SPI1_SIGNAL_SCK = 0,
	DBGC_CH585_SPI1_SIGNAL_MOSI,
	DBGC_CH585_SPI1_SIGNAL_MISO,
	DBGC_CH585_SPI1_SIGNAL_CS
} dbgc_ch585_spi1_signal_t;

/**
 * @brief 配置 SPI1 信号对应 GPIO 的显式输入/输出模式。
 *
 * 数据手册将 SPI1 SCK1、MOSI1、MISO1 分配到 PA0、PA1、PA2；DBG-C MCU-001 将 PA3 分配为 GPIO CS。
 * 模式语义对应 WCH EVT GPIOA_ModeCfg()；本函数不选择电气模式或配置 SPI 控制器。调用方负责
 * 评审并选择与外部电路兼容的模式。
 * @param signal 要配置的 SPI1 信号。
 * @param mode 调用方选择的 GPIO 模式。
 * @return 成功返回零，信号或模式无效时返回非零值。
 */
int dbgc_ch585_spi1_gpio_configure(dbgc_ch585_spi1_signal_t signal, dbgc_ch585_gpio_mode_t mode);

/**
 * @brief 将 PA3 片选 GPIO 输出锁存器写为调用方指定的原始电平。
 * @param high 非零写高，零写低；函数不解释片选有效极性。
 */
void dbgc_ch585_spi1_gpio_write_cs(uint8_t high);

#endif
