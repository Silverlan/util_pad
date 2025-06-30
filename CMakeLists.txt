include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

option(CONFIG_PRECOMPILE_LUA_FILES "Precompile Lua-files before adding them to an archive." ON)

set(PROJ_NAME util_pad)
pr_add_library(${PROJ_NAME} STATIC)

pr_add_dependency(${PROJ_NAME} util_versioned_archive TARGET PUBLIC)
pr_add_third_party_dependency(${PROJ_NAME} luajit)

pr_init_module(${PROJ_NAME})

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
