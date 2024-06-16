include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

option(CONFIG_PRECOMPILE_LUA_FILES "Precompile Lua-files before adding them to an archive." ON)

set(PROJ_NAME util_pad)
pr_add_library(${PROJ_NAME} STATIC)

pr_add_dependency(${PROJ_NAME} util_versioned_archive TARGET PUBLIC)
if(WIN32)
	pr_add_external_dependency(${PROJ_NAME} lua LIBRARY)
endif()

pr_add_headers(${PROJ_NAME} "include/")
pr_add_sources(${PROJ_NAME} "src/")

pr_add_compile_definitions(
	${PROJ_NAME}
		-DUVA_STATIC
)

if(CONFIG_PRECOMPILE_LUA_FILES)
	pr_add_dependency(${PROJ_NAME} LuaJIT HEADER_ONLY)
	pr_add_compile_definitions(
		${PROJ_NAME}
			-DUPAD_LUA_PRECOMPILE -DUSE_LUAJIT
	)
endif()

pr_finalize(${PROJ_NAME})
