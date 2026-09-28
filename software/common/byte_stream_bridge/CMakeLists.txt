add_library(dbgc_byte_stream_bridge STATIC
    src/dbgc_byte_stream_bridge.c
)

target_include_directories(dbgc_byte_stream_bridge PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
)
target_link_libraries(dbgc_byte_stream_bridge PUBLIC dbgc_byte_fifo)
