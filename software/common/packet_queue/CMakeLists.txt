add_library(dbgc_packet_queue STATIC
    src/dbgc_packet_queue.c
)

target_include_directories(dbgc_packet_queue PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
)
