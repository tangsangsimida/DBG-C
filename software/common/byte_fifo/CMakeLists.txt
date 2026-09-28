add_library(dbgc_byte_fifo STATIC
    src/dbgc_byte_fifo.c
)

target_include_directories(dbgc_byte_fifo PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
)
