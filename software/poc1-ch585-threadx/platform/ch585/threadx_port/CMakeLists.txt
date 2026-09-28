# This first build probe uses the official upstream RISC-V32/GNU context code.
# CH585-specific interrupt entry, SysTick integration, and HPE handling remain
# explicitly unimplemented until verified against WCH hardware behavior.
set(_tx_src "${CMAKE_SOURCE_DIR}/../third_party/threadx/ports/risc-v32/gnu/src")
set(_tx_inc "${CMAKE_SOURCE_DIR}/../third_party/threadx/ports/risc-v32/gnu/inc")
target_sources(threadx PRIVATE
    "${_tx_src}/tx_initialize_low_level.S"
    "${_tx_src}/tx_thread_context_restore.S"
    "${_tx_src}/tx_thread_context_save.S"
    "${_tx_src}/tx_thread_interrupt_control.S"
    "${_tx_src}/tx_thread_schedule.S"
    "${_tx_src}/tx_thread_stack_build.S"
    "${_tx_src}/tx_thread_system_return.S"
    "${_tx_src}/tx_timer_interrupt.S"
)
target_include_directories(threadx PUBLIC "${_tx_inc}")
target_compile_options(threadx PRIVATE -march=rv32imac -mabi=ilp32 -mcmodel=medany)
