#ifndef DBGC_CH585_VTREF_ADC_H
#define DBGC_CH585_VTREF_ADC_H

#include <stdint.h>

/**
 * @brief 定义与 WCH EVT ADC_SampClkTypeDef 数值对应的采样时钟选项。
 *
 * 采样频率受 R16_CLK_SYS_CFG[9] 影响；选项 0 在该位为 1 时不可用。
 * EVT 对应值依次为 SampleFreq_8、SampleFreq_8_or_4、
 * SampleFreq_5_33_or_2_67 和 SampleFreq_4_or_2。数据手册 EVT 注释给出的采样频率
 * 依次为 8M、8M/4M、5.33M/2.67M 和 4M/2M；选项 1 至 3 的斜线值随该配置位取值变化。
 */
typedef enum {
	DBGC_CH585_ADC_SAMPLE_CLOCK_OPTION_0 = 0,
	DBGC_CH585_ADC_SAMPLE_CLOCK_OPTION_1 = 1,
	DBGC_CH585_ADC_SAMPLE_CLOCK_OPTION_2 = 2,
	DBGC_CH585_ADC_SAMPLE_CLOCK_OPTION_3 = 3
} dbgc_ch585_adc_sample_clock_t;

/**
 * @brief 定义 WCH EVT 声明的 ADC PGA 增益选项。
 */
typedef enum {
	DBGC_CH585_ADC_GAIN_MINUS_12DB = 0x00,
	DBGC_CH585_ADC_GAIN_MINUS_6DB = 0x01,
	DBGC_CH585_ADC_GAIN_0DB = 0x02,
	DBGC_CH585_ADC_GAIN_PLUS_6DB = 0x03,
	DBGC_CH585_ADC_GAIN_PLUS_6DB_ALT = 0x10,
	DBGC_CH585_ADC_GAIN_PLUS_12DB = 0x11,
	DBGC_CH585_ADC_GAIN_PLUS_18DB = 0x12,
	DBGC_CH585_ADC_GAIN_PLUS_24DB = 0x13
} dbgc_ch585_adc_gain_t;

/**
 * @brief 定义 CH585 Target VTref ADC 原始采样结果。
 */
typedef enum {
	DBGC_CH585_VTREF_ADC_OK = 0,
	DBGC_CH585_VTREF_ADC_INVALID_ARGUMENT,
	DBGC_CH585_VTREF_ADC_INVALID_STATE,
	DBGC_CH585_VTREF_ADC_BUSY,
	DBGC_CH585_VTREF_ADC_POLL_LIMIT_REACHED,
	DBGC_CH585_VTREF_ADC_UNSUPPORTED_CONFIGURATION
} dbgc_ch585_vtref_adc_status_t;

/**
 * @brief 配置 PA4/A0 外部单端 ADC，并使用调用方选择的采样时钟和 PGA。
 *
 * 配置期间独占 ADC 外设。依据 EVT 示例，初始化后的第一次转换会在首次读取时丢弃。
 * 此函数不校准、不换算电压，也不选择产品默认增益或时钟。
 * 调用方须串行调用本模块所有 API，不得从 ISR 调用。
 * @param sample_clock WCH EVT 外部 ADC 采样时钟选项。
 * @param gain WCH EVT ADC PGA 增益选项。
 * 选项 0 与 R16_CLK_SYS_CFG[9] 为 1 的组合返回不支持。
 * @return 配置状态。
 */
dbgc_ch585_vtref_adc_status_t
dbgc_ch585_vtref_adc_initialize(dbgc_ch585_adc_sample_clock_t sample_clock,
				dbgc_ch585_adc_gain_t gain);

/**
 * @brief 读取 PA4/A0 的一个未校准、未换算 ADC 原始码。
 *
 * poll_limit 是每个转换阶段最多读取状态寄存器的次数，不是时间单位。达到上限后
 * 保留待完成转换状态；后续调用继续等待该转换，不会重复触发转换。
 * @param raw_code 接收 12 位原始 ADC 码的指针。
 * @param poll_limit 每个转换阶段允许的最大状态轮询次数。
 * @return 采样状态。
 */
dbgc_ch585_vtref_adc_status_t dbgc_ch585_vtref_adc_read_raw(uint16_t *raw_code,
							    uint32_t poll_limit);

#endif
