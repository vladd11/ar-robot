cmake_minimum_required(VERSION 3.18.1)

project("arshop")

add_library(
        arshop
        SHARED

        # Provides a relative path to your source file(s).
        activity.cpp
        renderer.cpp
        shaders.cpp

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
)
