#include "loader.h"

#include <vector>
#include <string>

#define TAG "ShaderLoader"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

/**
 * @param shader Pointer to shader
 * @param type Type of shader
 * @param source Shader code. Should be terminated with null char (\0, R-strings already matches this)
 * @return true if success, else returns false.
 */
bool CompileShader(GLuint *shader, const GLenum type,
                           const GLchar *source) {
  if (source == nullptr) return false;

  *shader = glCreateShader(type);
  glShaderSource(*shader, 1, &source, nullptr);
  glCompileShader(*shader);

#ifdef GLDEBUG
  GLint logLength;
  glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
  if (logLength > 0) {
    auto *log = (GLchar *)malloc(logLength);
    glGetShaderInfoLog(*shader, logLength, &logLength, log);
    LOGD("Shader get log:\n%s", log);
    free(log);
  }
#endif

  GLint status;
  glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
  if (status == 0) {
    glDeleteShader(*shader);
    return false;
  }

  return true;
}

bool LinkProgram(const GLuint prog) {
  GLint status;

  glLinkProgram(prog);

#ifdef GLDEBUG
  GLint logLength;
  glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
  if (logLength > 0) {
    auto *log = (GLchar *)malloc(logLength);
    glGetProgramInfoLog(prog, logLength, &logLength, log);
    LOGD("Program link log:\n%s", log);
    free(log);
  }
#endif

  glGetProgramiv(prog, GL_LINK_STATUS, &status);
  if (status == 0) {
    LOGD("Program link failed\n");
    return false;
  }

  return true;
}
