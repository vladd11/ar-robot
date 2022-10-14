set(LUA_LIB_lua-5.4.4S
        "lua-5.4.4/lapi.c"
        "lua-5.4.4/lcode.c"
        "lua-5.4.4/lctype.c"
        "lua-5.4.4/ldebug.c"
        "lua-5.4.4/ldo.c"
        "lua-5.4.4/ldump.c"
        "lua-5.4.4/lfunc.c"
        "lua-5.4.4/lgc.c"
        "lua-5.4.4/llex.c"
        "lua-5.4.4/lmem.c"
        "lua-5.4.4/lobject.c"
        "lua-5.4.4/lopcodes.c"
        "lua-5.4.4/lparser.c"
        "lua-5.4.4/lstate.c"
        "lua-5.4.4/lstring.c"
        "lua-5.4.4/ltable.c"
        "lua-5.4.4/ltm.c"
        "lua-5.4.4/lundump.c"
        "lua-5.4.4/lvm.c"
        "lua-5.4.4/lzio.c"
        "lua-5.4.4/lauxlib.c"
        "lua-5.4.4/lbaselib.c"
        "lua-5.4.4/lcorolib.c"
        "lua-5.4.4/ldblib.c"
        "lua-5.4.4/liolib.c"
        "lua-5.4.4/lmathlib.c"
        "lua-5.4.4/loadlib.c"
        "lua-5.4.4/loslib.c"
        "lua-5.4.4/lstrlib.c"
        "lua-5.4.4/ltablib.c"
        "lua-5.4.4/lutf8lib.c"
        "lua-5.4.4/linit.c"
        )

set(TARGETS_TO_INSTALL lua_internal lua_include)

if (LUA_BUILD_AS_CXX)
    set_source_files_properties(${LUA_LIB_lua-5.4.4S} "lua-5.4.4/lua.c" "lua-5.4.4/luac.c" PROPERTIES LANGUAGE CXX)
endif ()

add_library(lua_internal INTERFACE)

add_library(lua_include INTERFACE)

target_include_directories(lua_include INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

target_link_libraries(lua_internal INTERFACE lua_include)

if (LUA_ENABLE_SHARED)
    add_library(lua_shared SHARED ${LUA_LIB_lua-5.4.4S})
    target_link_libraries(lua_shared PRIVATE lua_internal PUBLIC lua_include)
    set_target_properties(lua_shared PROPERTIES
            VERSION "${PACKAGE_VERSION}"
            )
    if (WIN32)
        target_compile_definitions(lua_shared PRIVATE LUA_BUILD_AS_DLL)
    endif ()
    list(APPEND TARGETS_TO_INSTALL lua_shared)
    if (BUILD_SHARED_LIBS)
        add_library(Lua::Library ALIAS lua_shared)
    elseif (NOT TOP_LEVEL)
        set_target_properties(lua_shared PROPERTIES
                EXCLUDE_FROM_ALL ON
                )
    endif ()
endif ()

add_library(lua_static STATIC ${LUA_LIB_lua-5.4.4S})
target_link_libraries(lua_static PRIVATE lua_internal PUBLIC lua_include)
set_target_properties(lua_static PROPERTIES
        VERSION "${PACKAGE_VERSION}"
        )
list(APPEND TARGETS_TO_INSTALL lua_static)
if (NOT BUILD_SHARED_LIBS OR NOT LUA_ENABLE_SHARED)
    add_library(Lua::Library ALIAS lua_static)
endif ()


if (UNIX)
    if (NOT EMSCRIPTEN)
        find_library(LIBM m)
        #TODO: Redo this with find_package
        if (NOT LIBM)
            message(FATAL_ERROR "libm not found and is required by lua")
        endif ()
        target_link_libraries(lua_internal INTERFACE m)

        target_compile_definitions(lua_internal
                INTERFACE LUA_USE_POSIX
                )
        if (LUA_SUPPORT_DL)
            target_compile_definitions(lua_internal INTERFACE "LUA_USE_DLOPEN")
            target_link_libraries(lua_internal INTERFACE dl)
        endif ()
    endif ()

    target_compile_options(lua_internal
            INTERFACE "-Wall" "-Wextra"
            )
elseif (Win32)
    target_compile_options(lua_internal
            INTERFACE "/Wall"
            )
endif ()

if (LUA_BUILD_BINARY)
    include(CheckIncludeFile)
    CHECK_INCLUDE_FILE("readline/readline.h" HAVE_READLINE_READLINE_H)

    add_executable(lua "lua-5.4.4/lua.c")
    # Can not use lua_shared because some symbols are not exported
    target_link_libraries(lua PRIVATE lua_static)
    set_target_properties(lua PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
            )
    if (HAVE_READLINE_READLINE_H)
        target_compile_definitions(lua PRIVATE "LUA_USE_READLINE")
        target_link_libraries(lua PUBLIC readline)
    endif ()
    list(APPEND TARGETS_TO_INSTALL lua)
endif ()

if (LUA_BUILD_COMPILER)
    add_executable(luac "lua-5.4.4/luac.c")
    target_link_libraries(luac PRIVATE lua_static)
    set_target_properties(luac PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
            )
    list(APPEND TARGETS_TO_INSTALL luac)
endif ()

install(TARGETS ${TARGETS_TO_INSTALL}
        EXPORT LuaTargets
        )

install(DIRECTORY include/ TYPE INCLUDE)

include(CMakePackageConfigHelpers)

get_target_property(LUA_EXPORT_LIBRARY Lua::Library ALIASED_TARGET)
write_basic_package_version_file(
        LuaConfigVersion.cmake
        VERSION 5.4.4
        COMPATIBILITY SameMajorVersion
)

install(EXPORT LuaTargets
        FILE LuaTargets.cmake
        DESTINATION "lib/cmake"
        NAMESPACE Lua::
        )

configure_package_config_file(
        LuaConfig.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/LuaConfig.cmake"
        INSTALL_DESTINATION "lib/cmake"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/LuaConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/LuaConfigVersion.cmake"
        DESTINATION "lib/cmake"
        )