cmake_minimum_required(VERSION 3.18.1)

project("arshop")

set(can_use_assembler TRUE)
enable_language(ASM)
add_definitions(-v -DANDROID -DOC_ARM_ASM)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL armv7-a)
    add_definitions(-DARCH_ARM_USE_INTRINSICS -DARCH_ARM_HAVE_VFP)
    set(ASM_SOURCES yuv2rgb/YuvToRgb_neon.S)
endif()

if(CMAKE_SYSTEM_PROCESSOR STREQUAL aarch64)
    add_definitions(-DARCH_ARM_USE_INTRINSICS -DARCH_ARM64_USE_INTRINSICS -DARCH_ARM64_HAVE_NEON)
    set(ASM_SOURCES yuv2rgb/YuvToRgb_advsimd.S)
endif()


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
        base64.cpp
        round_radians.cpp
        glError.cpp

        verts/plane.cpp
        verts/triangle.cpp
        shaders/background.cpp
        shaders/loader.cpp
        shaders/default.cpp

        yuv2rgb/RenderScriptToolkit.cpp
        yuv2rgb/TaskProcessor.cpp
        yuv2rgb/Utils.cpp
        yuv2rgb/YuvToRgb.cpp
        ${ASM_SOURCES}
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

target_link_libraries(
        arshop

        ${log-lib}
        ${android-lib}
        ${gles-lib}
        arcore
        glm
        cpufeatures
)

include(AndroidNdkModules)
android_ndk_import_module_cpufeatures()
