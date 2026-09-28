/********************************** (C) COPYRIGHT *******************************
 * Copyright (c) 2024 Nanjing Qinheng Microelectronics Co., Ltd.
 * The following HPE control follows the WCH QingKe V3C FreeRTOS EVT port.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

    .section .highcode.tx_systick_isr,"ax",@progbits
    .align  2
    .global tx_systick_isr
    .type   tx_systick_isr, @function

tx_systick_isr:
    /* ThreadX RISC-V32/GNU interrupt frame: 32 words, RA at word 28. */
    addi    sp, sp, -(32 * 4)
    sw      t0, 19 * 4(sp)

    /* WCH FreeRTOS EVT port disables HPE with CSR 0x804 bit 5 before
       software-managed context switching. */
    li      t0, 0x20
    csrs    0x804, t0
    lw      t0, 19 * 4(sp)
    sw      ra, 28 * 4(sp)

    call    _tx_thread_context_save
    call    _tx_timer_interrupt

    /* CH585 EVT core_riscv.h places SysTick SR at base plus one word. */
    li      t0, 0xE000F004
    sw      zero, 0(t0)
    la      t0, _tx_thread_context_restore
    jr      t0
    .size   tx_systick_isr, .-tx_systick_isr
