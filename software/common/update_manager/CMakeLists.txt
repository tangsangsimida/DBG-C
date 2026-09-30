add_library(dbgc_update_manager STATIC
	src/dbgc_update_manager.c
)

target_include_directories(dbgc_update_manager PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/include"
)
