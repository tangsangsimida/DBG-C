#include "dbgc_ch585_vtref_adc.h"
#include "dbgc_ch585_vtref_adc_host_regs.h"

#include <stdio.h>

volatile uint32_t dbgc_host_R32_PA_DIR;
volatile uint32_t dbgc_host_R32_PA_PU;
volatile uint32_t dbgc_host_R32_PA_PD_DRV;
volatile uint32_t dbgc_host_R32_PIN_IN_DIS;
volatile uint8_t dbgc_host_R8_TKEY_CFG;
volatile uint8_t dbgc_host_R8_ADC_CHANNEL;
volatile uint8_t dbgc_host_R8_ADC_CFG;
volatile uint8_t dbgc_host_R8_ADC_CONVERT;
volatile uint16_t dbgc_host_R16_ADC_DATA;
volatile uint16_t dbgc_host_R16_CLK_SYS_CFG;

static unsigned int checks;
static unsigned int starts;
static unsigned int start_busy_reads;
static unsigned int eoc_busy_reads;
static unsigned int keep_start_busy;
static unsigned int keep_eoc_busy;

#define CHECK(condition)                                                               \
	do {                                                                           \
		++checks;                                                              \
		if (!(condition)) {                                                    \
			fprintf(stderr, "第 %u 项检查失败：%s\n", checks, #condition); \
			return 1;                                                      \
		}                                                                      \
	} while (0)

/**
 * @brief 模拟 WCH ADC 转换启动写入。
 */
void dbgc_ch585_vtref_adc_host_start_conversion(void)
{
	++starts;
	dbgc_host_R16_ADC_DATA = (starts == 1U) ? 0x0123U : 0xFABCU;
	dbgc_host_R8_ADC_CONVERT |= RB_ADC_START;
}

/**
 * @brief 按测试设置推进 ADC 转换状态。
 * @return 模拟转换状态寄存器。
 */
uint8_t dbgc_ch585_vtref_adc_host_read_conversion_status(void)
{
	int start_was_set = (dbgc_host_R8_ADC_CONVERT & RB_ADC_START) != 0U;

	if ((dbgc_host_R8_ADC_CONVERT & RB_ADC_START) != 0U) {
		if (keep_start_busy != 0U) {
			return dbgc_host_R8_ADC_CONVERT;
		}
		if (start_busy_reads != 0U) {
			--start_busy_reads;
			return dbgc_host_R8_ADC_CONVERT;
		}
		dbgc_host_R8_ADC_CONVERT &= (uint8_t)~RB_ADC_START;
		if ((keep_eoc_busy != 0U) || (eoc_busy_reads != 0U)) {
			dbgc_host_R8_ADC_CONVERT |= RB_ADC_EOC_X;
		}
	}

	if ((start_was_set == 0) && ((dbgc_host_R8_ADC_CONVERT & RB_ADC_EOC_X) != 0U)) {
		if (keep_eoc_busy != 0U) {
			return dbgc_host_R8_ADC_CONVERT;
		}
		if (eoc_busy_reads != 0U) {
			--eoc_busy_reads;
			return dbgc_host_R8_ADC_CONVERT;
		}
		dbgc_host_R8_ADC_CONVERT &= (uint8_t)~RB_ADC_EOC_X;
	}

	return dbgc_host_R8_ADC_CONVERT;
}

/**
 * @brief 返回脚本化 ADC 原始码。
 * @return 测试设置的 ADC 数据寄存器值。
 */
uint16_t dbgc_ch585_vtref_adc_host_read_data(void)
{
	return dbgc_host_R16_ADC_DATA;
}

/**
 * @brief 验证 PA4/A0 初始化、有限轮询、超时恢复及原始码掩码。
 * @return 所有主机寄存器模型检查通过时返回零。
 */
int main(void)
{
	uint16_t raw_code = 0xAAAAU;

	dbgc_host_R32_PA_DIR = 0xFFFFFFFFU;
	dbgc_host_R32_PA_PU = 0xFFFFFFFFU;
	dbgc_host_R32_PA_PD_DRV = 0xFFFFFFFFU;
	dbgc_host_R32_PIN_IN_DIS = 0U;
	dbgc_host_R8_TKEY_CFG = RB_TKEY_PWR_ON;
	dbgc_host_R8_ADC_CHANNEL = 0xFFU;
	dbgc_host_R8_ADC_CFG = 0U;
	dbgc_host_R8_ADC_CONVERT = 0U;
	dbgc_host_R16_ADC_DATA = 0U;
	dbgc_host_R16_CLK_SYS_CFG = 0U;

	CHECK(dbgc_ch585_vtref_adc_read_raw(&raw_code, 1U) == DBGC_CH585_VTREF_ADC_INVALID_STATE);
	CHECK(raw_code == 0xAAAAU);
	CHECK(dbgc_ch585_vtref_adc_initialize((dbgc_ch585_adc_sample_clock_t)4,
					      DBGC_CH585_ADC_GAIN_0DB) ==
	      DBGC_CH585_VTREF_ADC_INVALID_ARGUMENT);
	dbgc_host_R16_CLK_SYS_CFG = 1U << 9;
	CHECK(dbgc_ch585_vtref_adc_initialize(DBGC_CH585_ADC_SAMPLE_CLOCK_OPTION_0,
					      DBGC_CH585_ADC_GAIN_0DB) ==
	      DBGC_CH585_VTREF_ADC_UNSUPPORTED_CONFIGURATION);
	dbgc_host_R16_CLK_SYS_CFG = 0U;
	CHECK(dbgc_ch585_vtref_adc_initialize(DBGC_CH585_ADC_SAMPLE_CLOCK_OPTION_0,
					      (dbgc_ch585_adc_gain_t)4) ==
	      DBGC_CH585_VTREF_ADC_INVALID_ARGUMENT);
	CHECK(dbgc_ch585_vtref_adc_initialize(DBGC_CH585_ADC_SAMPLE_CLOCK_OPTION_0,
					      DBGC_CH585_ADC_GAIN_0DB) == DBGC_CH585_VTREF_ADC_OK);
	CHECK((dbgc_host_R32_PA_DIR & (1UL << 4)) == 0U);
	CHECK((dbgc_host_R32_PA_PU & (1UL << 4)) == 0U);
	CHECK((dbgc_host_R32_PA_PD_DRV & (1UL << 4)) == 0U);
	CHECK((dbgc_host_R32_PIN_IN_DIS & (1UL << 4)) != 0U);
	CHECK((dbgc_host_R8_TKEY_CFG & RB_TKEY_PWR_ON) == 0U);
	CHECK(dbgc_host_R8_ADC_CHANNEL == 0U);
	CHECK(dbgc_host_R8_ADC_CFG == (RB_ADC_BUF_EN | 0x20U | RB_ADC_POWER_ON));
	CHECK((dbgc_host_R8_ADC_CONVERT & RB_ADC_PGA_GAIN2) == 0U);
	CHECK(dbgc_ch585_vtref_adc_initialize(DBGC_CH585_ADC_SAMPLE_CLOCK_OPTION_3,
					      DBGC_CH585_ADC_GAIN_PLUS_12DB) ==
	      DBGC_CH585_VTREF_ADC_OK);
	CHECK(dbgc_host_R8_ADC_CFG == (RB_ADC_BUF_EN | 0xC0U | 0x10U | RB_ADC_POWER_ON));
	CHECK((dbgc_host_R8_ADC_CONVERT & RB_ADC_PGA_GAIN2) != 0U);

	CHECK(dbgc_ch585_vtref_adc_read_raw(NULL, 1U) == DBGC_CH585_VTREF_ADC_INVALID_ARGUMENT);
	CHECK(dbgc_ch585_vtref_adc_read_raw(&raw_code, 0U) ==
	      DBGC_CH585_VTREF_ADC_INVALID_ARGUMENT);
	dbgc_host_R8_ADC_CONVERT = RB_ADC_START;
	CHECK(dbgc_ch585_vtref_adc_read_raw(&raw_code, 1U) == DBGC_CH585_VTREF_ADC_BUSY);
	CHECK(starts == 0U);
	dbgc_host_R8_ADC_CONVERT = 0U;

	start_busy_reads = 1U;
	eoc_busy_reads = 1U;
	dbgc_host_R16_ADC_DATA = 0xFABCU;
	CHECK(dbgc_ch585_vtref_adc_read_raw(&raw_code, 2U) == DBGC_CH585_VTREF_ADC_OK);
	CHECK(starts == 2U);
	CHECK(raw_code == 0x0ABCU);

	start_busy_reads = 2U;
	CHECK(dbgc_ch585_vtref_adc_read_raw(&raw_code, 1U) ==
	      DBGC_CH585_VTREF_ADC_POLL_LIMIT_REACHED);
	CHECK(starts == 3U);
	CHECK(dbgc_ch585_vtref_adc_initialize(DBGC_CH585_ADC_SAMPLE_CLOCK_OPTION_0,
					      DBGC_CH585_ADC_GAIN_0DB) ==
	      DBGC_CH585_VTREF_ADC_BUSY);
	CHECK(dbgc_ch585_vtref_adc_read_raw(&raw_code, 2U) == DBGC_CH585_VTREF_ADC_OK);
	CHECK(starts == 3U);

	eoc_busy_reads = 3U;
	CHECK(dbgc_ch585_vtref_adc_read_raw(&raw_code, 1U) ==
	      DBGC_CH585_VTREF_ADC_POLL_LIMIT_REACHED);
	CHECK(starts == 4U);
	CHECK(dbgc_ch585_vtref_adc_read_raw(&raw_code, 1U) ==
	      DBGC_CH585_VTREF_ADC_POLL_LIMIT_REACHED);
	CHECK(dbgc_ch585_vtref_adc_read_raw(&raw_code, 1U) == DBGC_CH585_VTREF_ADC_OK);
	CHECK(starts == 4U);

	keep_start_busy = 1U;
	CHECK(dbgc_ch585_vtref_adc_read_raw(&raw_code, 2U) ==
	      DBGC_CH585_VTREF_ADC_POLL_LIMIT_REACHED);
	CHECK(dbgc_ch585_vtref_adc_read_raw(&raw_code, 1U) ==
	      DBGC_CH585_VTREF_ADC_POLL_LIMIT_REACHED);
	CHECK(starts == 5U);
	keep_start_busy = 0U;
	dbgc_host_R8_ADC_CONVERT &= (uint8_t)~RB_ADC_START;
	CHECK(dbgc_ch585_vtref_adc_read_raw(&raw_code, 1U) == DBGC_CH585_VTREF_ADC_OK);
	CHECK(starts == 5U);

	keep_eoc_busy = 1U;
	CHECK(dbgc_ch585_vtref_adc_read_raw(&raw_code, 1U) ==
	      DBGC_CH585_VTREF_ADC_POLL_LIMIT_REACHED);
	CHECK(dbgc_ch585_vtref_adc_read_raw(&raw_code, 1U) ==
	      DBGC_CH585_VTREF_ADC_POLL_LIMIT_REACHED);
	CHECK(starts == 6U);
	keep_eoc_busy = 0U;
	dbgc_host_R8_ADC_CONVERT &= (uint8_t)~RB_ADC_EOC_X;
	CHECK(dbgc_ch585_vtref_adc_read_raw(&raw_code, 1U) == DBGC_CH585_VTREF_ADC_OK);
	CHECK(starts == 6U);

	printf("CH585 VTref ADC host checks passed: %u checks\n", checks);
	return 0;
}
