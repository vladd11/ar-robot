//
// Created by vladislav on 8/20/22.
//

#ifndef AR_SHOP_SHADERS_H
#define AR_SHOP_SHADERS_H

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <android/log.h>
#include <cstdlib>

#include "shaders/default.cpp"

#define TAG "Shaders"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

bool CompileShader(GLuint *shader, const GLenum type, const GLchar *source);
bool LinkProgram(const GLuint prog);

#endif //AR_SHOP_SHADERS_H
