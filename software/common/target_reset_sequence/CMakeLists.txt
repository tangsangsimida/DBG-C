add_library(dbgc_target_reset_sequence STATIC
    src/dbgc_target_reset_sequence.c
)
target_include_directories(dbgc_target_reset_sequence PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
