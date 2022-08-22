#ifndef AR_SHOP_LOADER_H
#define AR_SHOP_LOADER_H

#include <android/log.h>
#include <cstdlib>
#include <vector>
#include <string>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

bool CompileShader(GLuint *shader, const GLenum type,
                   const GLchar *source);

bool LinkProgram(const GLuint prog);

#endif //AR_SHOP_LOADER_H
