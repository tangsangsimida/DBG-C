add_library(dbgc_cmsis_dap_service STATIC
    src/dbgc_cmsis_dap_service.c
)

target_include_directories(dbgc_cmsis_dap_service PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
    "${CMAKE_CURRENT_LIST_DIR}/../cmsis_dap_bounds/include"
)

target_link_libraries(dbgc_cmsis_dap_service PUBLIC
    dbgc_packet_queue
)
