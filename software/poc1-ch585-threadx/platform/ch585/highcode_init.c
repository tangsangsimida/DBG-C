/********************************** (C) COPYRIGHT *******************************
 * File Name          : highcode_init.c
 * Derived from       : CH58x_sys.c, EVT/EXAM/SRC/StdPeriphDriver
 * Original Version   : V1.2
 * Original Date      : 2021/11/17
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include <stdint.h>
#include "wch/CH585SFR.h"
#include "wch/core_riscv.h"
#include "wch/CH58x_clk.h"
#include "wch/CH58x_sys.h"

#ifndef SAFEOPERATE
#define SAFEOPERATE asm volatile("fence.i")
#endif

/* Startup uses this ISR stack symbol from the WCH EVT linker/startup pair. */
static uint32_t isr_stack[512];
uint32_t *xISRStackTop = &isr_stack[512];

/* Keep the WCH EVT reset-time clock initialization sequence. */
__attribute__((section(".highcode_init"), noinline)) void highcode_init(void)
{
	R32_SAFE_MODE_CTRL |= RB_XROM_312M_SEL;
	R8_SAFE_MODE_CTRL &= ~RB_SAFE_AUTO_EN;
	sys_safe_access_enable();
	R32_MISC_CTRL |= 5 | (3 << 25);
	R8_PLL_CONFIG &= ~(1 << 5);
	R8_HFCK_PWR_CTRL |= RB_CLK_RC16M_PON | RB_CLK_PLL_PON;
	R16_CLK_SYS_CFG = CLK_SOURCE_HSI_PLL_62_4MHz;
	R8_FLASH_SCK = R8_FLASH_SCK & (~(1 << 4));
	R8_FLASH_CFG = 0x02;
	R8_XT32M_TUNE = (R8_XT32M_TUNE & (~0x03)) | 0x01;
	R8_CK32K_CONFIG |= RB_CLK_INT32K_PON;
	R8_SAFE_MODE_CTRL |= RB_SAFE_AUTO_EN;
	sys_safe_access_disable();
}
