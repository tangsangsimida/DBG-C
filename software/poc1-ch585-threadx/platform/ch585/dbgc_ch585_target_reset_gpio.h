#ifndef DBGC_CH585_TARGET_RESET_GPIO_H
#define DBGC_CH585_TARGET_RESET_GPIO_H

#include <stdint.h>

#include "dbgc_ch585_gpio_mode.h"

/**
 * @brief PB5/QFN48 脚 19 按 MCU-001 分配给 Target_nRESET。
 *
 * 调用方选择电气模式和原始高低电平。本接口不定义复位脉宽、输出拓扑或默认状态；
 * 对 GPIOB 的模式修改必须由调用方串行化。
 */
int dbgc_ch585_target_reset_gpio_configure(dbgc_ch585_gpio_mode_t mode);
int dbgc_ch585_target_reset_gpio_write(uint8_t high);
int dbgc_ch585_target_reset_gpio_read(uint8_t *high);

/**
 * @brief 按 CMSIS-DAP nRESET 位值以 GPIO 方向切换方式模拟开漏控制。
 *
 * bit 为零时先释放为浮空输入、写入低电平锁存值，再切换到调用方指定的推挽输出模式；
 * bit 为一时切换到调用方指定的输入模式以释放线路。释放后的目标高电平依赖外部电路，
 * 本接口不配置上拉电阻、电平转换或同步机制。调用方须串行化 GPIOB 模式修改。
 * @param bit CMSIS-DAP nRESET 位值，零表示断言复位，一表示释放复位。
 * @param output_mode 断言时使用的推挽输出模式，只接受 5 mA 或 20 mA 档。
 * @param released_mode 释放时使用的输入模式，只接受浮空或上拉输入。
 * @return 成功返回零，参数无效时返回非零值。
 */
int dbgc_ch585_target_reset_gpio_set_nreset(uint8_t bit, dbgc_ch585_gpio_mode_t output_mode,
					    dbgc_ch585_gpio_mode_t released_mode);

#endif
