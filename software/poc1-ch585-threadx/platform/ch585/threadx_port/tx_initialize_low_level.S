/********************************** (C) COPYRIGHT *******************************
 * Copyright (c) 2024 Nanjing Qinheng Microelectronics Co., Ltd.
 * The interrupt-controller conventions follow the WCH CH585 EVT.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

    .section .text.tx_initialize_low_level,"ax",@progbits
    .align  2
    .global _tx_initialize_low_level
    .type   _tx_initialize_low_level, @function

_tx_initialize_low_level:
    /* Preserve the caller's SP as ThreadX's system stack pointer. */
    la      t0, _tx_thread_system_stack_ptr
    sw      sp, 0(t0)

    addi    sp, sp, -16
    sw      ra, 12(sp)
    call    tx_ch585_low_level_setup
    lw      ra, 12(sp)
    addi    sp, sp, 16
    ret
    .size   _tx_initialize_low_level, .-_tx_initialize_low_level
