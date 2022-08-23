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
        helper.cpp
        engine.cpp
        background_renderer.cpp
        ar_ui_renderer.cpp
        glError.cpp
        verts/plane.cpp
        verts/triangle.cpp
        shaders/background.cpp
        shaders/loader.cpp
        shaders/default.cpp

)

find_library(
        log-lib
        log)

find_library(
        gles-lib
        GLESv2
)

target_link_libraries(
        arshop

        ${log-lib}
        ${gles-lib}
        arcore
        glm
)
