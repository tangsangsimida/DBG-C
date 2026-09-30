#include "dbgc_ch585_vtref_adc.h"

#if defined(DBGC_CH585_VTREF_ADC_HOST_TEST)
#include "dbgc_ch585_vtref_adc_host_regs.h"
#else
#include "wch/CH585SFR.h"
#endif

#define DBGC_CH585_PA4_MASK (1UL << 4)
#define DBGC_CH585_ADC_CHANNEL_A0 0U
#define DBGC_CH585_ADC_SAMPLE_CLOCK_MASK 0x03U
#define DBGC_CH585_ADC_CLOCK_DEPENDENCY_MASK (1U << 9)
#define DBGC_CH585_ADC_GAIN_DIRECTION 0x10U

static uint8_t dbgc_ch585_vtref_adc_initialized;
static uint8_t dbgc_ch585_vtref_adc_conversion_pending;
static uint8_t dbgc_ch585_vtref_adc_discard_first_conversion;

/**
 * @brief 启动一次外部 ADC 转换。
 */
static void dbgc_ch585_vtref_adc_start_conversion(void)
{
#if defined(DBGC_CH585_VTREF_ADC_HOST_TEST)
	dbgc_ch585_vtref_adc_host_start_conversion();
#else
	R8_ADC_CONVERT |= RB_ADC_START;
#endif
}

/**
 * @brief 读取 ADC 转换状态寄存器。
 * @return ADC 转换控制寄存器当前值。
 */
static uint8_t dbgc_ch585_vtref_adc_read_conversion_status(void)
{
#if defined(DBGC_CH585_VTREF_ADC_HOST_TEST)
	return dbgc_ch585_vtref_adc_host_read_conversion_status();
#else
	return R8_ADC_CONVERT;
#endif
}

/**
 * @brief 读取 ADC 转换数据寄存器。
 * @return ADC 数据寄存器当前值。
 */
static uint16_t dbgc_ch585_vtref_adc_read_data(void)
{
#if defined(DBGC_CH585_VTREF_ADC_HOST_TEST)
	return dbgc_ch585_vtref_adc_host_read_data();
#else
	return R16_ADC_DATA;
#endif
}

/**
 * @brief 检查采样时钟选项是否由 WCH EVT ADC 枚举声明。
 * @param sample_clock 待检查的采样时钟选项。
 * @return 有效时返回非零值。
 */
static int dbgc_ch585_vtref_adc_sample_clock_is_valid(dbgc_ch585_adc_sample_clock_t sample_clock)
{
	return (uint32_t)sample_clock <= DBGC_CH585_ADC_SAMPLE_CLOCK_MASK;
}

/**
 * @brief 检查 PGA 增益是否由 WCH EVT ADC 枚举声明。
 * @param gain 待检查的 PGA 增益。
 * @return 有效时返回非零值。
 */
static int dbgc_ch585_vtref_adc_gain_is_valid(dbgc_ch585_adc_gain_t gain)
{
	switch (gain) {
	case DBGC_CH585_ADC_GAIN_MINUS_12DB:
	case DBGC_CH585_ADC_GAIN_MINUS_6DB:
	case DBGC_CH585_ADC_GAIN_0DB:
	case DBGC_CH585_ADC_GAIN_PLUS_6DB:
	case DBGC_CH585_ADC_GAIN_PLUS_6DB_ALT:
	case DBGC_CH585_ADC_GAIN_PLUS_12DB:
	case DBGC_CH585_ADC_GAIN_PLUS_18DB:
	case DBGC_CH585_ADC_GAIN_PLUS_24DB:
		return 1;
	default:
		return 0;
	}
}

/**
 * @brief 配置 PA4 浮空输入并关闭其数字输入。
 */
static void dbgc_ch585_vtref_adc_configure_pin(void)
{
	R32_PA_PD_DRV &= ~DBGC_CH585_PA4_MASK;
	R32_PA_PU &= ~DBGC_CH585_PA4_MASK;
	R32_PA_DIR &= ~DBGC_CH585_PA4_MASK;
	R32_PIN_IN_DIS |= DBGC_CH585_PA4_MASK;
}

/**
 * @brief 初始化 PA4/A0 外部单端 ADC。
 * @param sample_clock WCH EVT 外部 ADC 采样时钟选项。
 * @param gain WCH EVT ADC PGA 增益选项。
 * @return 配置状态。
 */
dbgc_ch585_vtref_adc_status_t
dbgc_ch585_vtref_adc_initialize(dbgc_ch585_adc_sample_clock_t sample_clock,
				dbgc_ch585_adc_gain_t gain)
{
	uint8_t gain_config;

	if (!dbgc_ch585_vtref_adc_sample_clock_is_valid(sample_clock) ||
	    !dbgc_ch585_vtref_adc_gain_is_valid(gain)) {
		return DBGC_CH585_VTREF_ADC_INVALID_ARGUMENT;
	}
	if ((sample_clock == DBGC_CH585_ADC_SAMPLE_CLOCK_OPTION_0) &&
	    ((R16_CLK_SYS_CFG & DBGC_CH585_ADC_CLOCK_DEPENDENCY_MASK) != 0U)) {
		return DBGC_CH585_VTREF_ADC_UNSUPPORTED_CONFIGURATION;
	}
	if (dbgc_ch585_vtref_adc_conversion_pending != 0U) {
		return DBGC_CH585_VTREF_ADC_BUSY;
	}

	dbgc_ch585_vtref_adc_configure_pin();
	R8_TKEY_CFG &= (uint8_t)~RB_TKEY_PWR_ON;
	R8_ADC_CHANNEL = DBGC_CH585_ADC_CHANNEL_A0;
	gain_config = (uint8_t)gain;
	R8_ADC_CFG = RB_ADC_BUF_EN | ((uint8_t)sample_clock << 6) | ((gain_config & 0x0FU) << 4);
	if ((gain_config & DBGC_CH585_ADC_GAIN_DIRECTION) != 0U) {
		R8_ADC_CONVERT |= RB_ADC_PGA_GAIN2;
	} else {
		R8_ADC_CONVERT &= (uint8_t)~RB_ADC_PGA_GAIN2;
	}
	R8_ADC_CFG |= RB_ADC_POWER_ON;
	dbgc_ch585_vtref_adc_initialized = 1U;
	dbgc_ch585_vtref_adc_discard_first_conversion = 1U;
	return DBGC_CH585_VTREF_ADC_OK;
}

/**
 * @brief 在有限轮询预算内取得一个未校准 ADC 原始码。
 * @param raw_code 接收原始码的指针。
 * @param poll_limit 每个转换阶段允许的最大轮询次数。
 * @return 采样状态。
 */
dbgc_ch585_vtref_adc_status_t dbgc_ch585_vtref_adc_read_raw(uint16_t *raw_code, uint32_t poll_limit)
{
	uint32_t poll_index;
	uint8_t discard_conversion;
	uint8_t conversion_status = 0U;

	if ((raw_code == 0) || (poll_limit == 0U)) {
		return DBGC_CH585_VTREF_ADC_INVALID_ARGUMENT;
	}
	if (dbgc_ch585_vtref_adc_initialized == 0U) {
		return DBGC_CH585_VTREF_ADC_INVALID_STATE;
	}

	discard_conversion = dbgc_ch585_vtref_adc_discard_first_conversion;
	while (1) {
		if (dbgc_ch585_vtref_adc_conversion_pending == 0U) {
			if ((R8_ADC_CONVERT & RB_ADC_START) != 0U) {
				return DBGC_CH585_VTREF_ADC_BUSY;
			}
			dbgc_ch585_vtref_adc_start_conversion();
			dbgc_ch585_vtref_adc_conversion_pending = 1U;
		}

		for (poll_index = 0U; poll_index < poll_limit; ++poll_index) {
			conversion_status = dbgc_ch585_vtref_adc_read_conversion_status();
			if ((conversion_status & RB_ADC_START) == 0U) {
				break;
			}
		}
		if ((conversion_status & RB_ADC_START) != 0U) {
			return DBGC_CH585_VTREF_ADC_POLL_LIMIT_REACHED;
		}

		for (poll_index = 0U; poll_index < poll_limit; ++poll_index) {
			conversion_status = dbgc_ch585_vtref_adc_read_conversion_status();
			if ((conversion_status & RB_ADC_EOC_X) == 0U) {
				dbgc_ch585_vtref_adc_conversion_pending = 0U;
				if (discard_conversion != 0U) {
					dbgc_ch585_vtref_adc_discard_first_conversion = 0U;
					discard_conversion = 0U;
					break;
				}
				*raw_code = dbgc_ch585_vtref_adc_read_data() & RB_ADC_DATA;
				return DBGC_CH585_VTREF_ADC_OK;
			}
		}
		if ((conversion_status & RB_ADC_EOC_X) != 0U) {
			return DBGC_CH585_VTREF_ADC_POLL_LIMIT_REACHED;
		}
	}

	return DBGC_CH585_VTREF_ADC_INVALID_STATE;
}
