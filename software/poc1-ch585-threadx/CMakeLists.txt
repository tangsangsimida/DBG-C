cmake_minimum_required(VERSION 3.20)
project(dbgc_poc1_ch585_threadx LANGUAGES C ASM)

set(THREADX_ARCH "risc-v32")
set(THREADX_TOOLCHAIN "gnu")
set(THREADX_CUSTOM_PORT "${CMAKE_CURRENT_SOURCE_DIR}/platform/ch585/threadx_port")
set(TX_USER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/platform/ch585/tx_user.h")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../third_party/threadx" threadx)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../common/byte_fifo" byte_fifo)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../common/byte_stream_bridge" byte_stream_bridge)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../common/byte_duplex_bridge" byte_duplex_bridge)

add_executable(dbgc_poc1
    platform/ch585/startup/startup_CH585.S
    platform/ch585/highcode_init.c
    src/main.c
)
target_link_libraries(dbgc_poc1 PRIVATE threadx dbgc_byte_duplex_bridge)
target_include_directories(dbgc_poc1 PRIVATE platform/ch585)
target_compile_options(dbgc_poc1 PRIVATE -march=rv32imac -mabi=ilp32 -mcmodel=medany -ffunction-sections -fdata-sections)
target_link_options(dbgc_poc1 PRIVATE
    -march=rv32imac -mabi=ilp32 -mcmodel=medany
    -T${CMAKE_CURRENT_SOURCE_DIR}/platform/ch585/ld/ch585.ld
    -Wl,--gc-sections -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/dbgc_poc1.map
    -nostartfiles
)
set_target_properties(dbgc_poc1 PROPERTIES SUFFIX ".elf")
