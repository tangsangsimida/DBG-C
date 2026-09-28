# Keep the ThreadX RISC-V32/GNU context routines and provide an experimental
# CH585 low-level and SysTick adapter. Board behavior remains unverified.
set(_tx_src "${CMAKE_SOURCE_DIR}/../third_party/threadx/ports/risc-v32/gnu/src")
set(_tx_inc "${CMAKE_SOURCE_DIR}/../third_party/threadx/ports/risc-v32/gnu/inc")
target_sources(threadx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/tx_initialize_low_level.c"
    "${CMAKE_CURRENT_LIST_DIR}/tx_initialize_low_level.S"
    "${CMAKE_CURRENT_LIST_DIR}/tx_systick.S"
    "${_tx_src}/tx_thread_context_restore.S"
    "${_tx_src}/tx_thread_context_save.S"
    "${_tx_src}/tx_thread_interrupt_control.S"
    "${_tx_src}/tx_thread_schedule.S"
    "${_tx_src}/tx_thread_stack_build.S"
    "${_tx_src}/tx_thread_system_return.S"
    "${_tx_src}/tx_timer_interrupt.S"
)
target_include_directories(threadx PUBLIC "${_tx_inc}" "${CMAKE_SOURCE_DIR}/platform/ch585")
target_compile_options(threadx PRIVATE -march=rv32imac -mabi=ilp32 -mcmodel=medany)
