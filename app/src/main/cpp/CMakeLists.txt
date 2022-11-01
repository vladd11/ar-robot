cmake_minimum_required(VERSION 3.18.1)

project("arshop")

add_library(arcore SHARED IMPORTED)
set_target_properties(
        arcore
        PROPERTIES IMPORTED_LOCATION

        ${ARCORE_LIBPATH}/${ANDROID_ABI}/libarcore_sdk_c.so
        INTERFACE_INCLUDE_DIRECTORIES ${ARCORE_INCLUDE}
)

add_library(glm INTERFACE)
set_target_properties(glm PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${GLM_INCLUDE})

add_library(
        arshop
        SHARED

        # Provides a relative path to your source file(s).
        mongoose.c
        server.cpp
        plane_renderer.cpp
        helper.cpp
        engine.cpp
        background_renderer.cpp
        ar_ui_renderer.cpp
        gl_util.cpp
        bindings.cpp

        safe_queue.cpp
        verts/plane.cpp
        verts/triangle.cpp
        shaders/raw.cpp
        shaders/plane.cpp
        shaders/background.cpp
        shaders/loader.cpp
        shaders/default.cpp

        yuv2rgb.cpp
)

find_library(
        log-lib
        log
)

find_library(
        android-lib
        android
)

find_library(
        gles-lib
        GLESv2
)

add_subdirectory(
        ${LUA_INCLUDE}
        ${LUA_BUILDDIR}
)

target_include_directories(arshop PUBLIC ${LUA_INCLUDE}/src)

target_link_libraries(
        arshop

        lua_static
        ${log-lib}
        ${android-lib}
        ${gles-lib}
        arcore
        glm
)
