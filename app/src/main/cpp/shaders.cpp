#include "shaders.h"

bool LinkProgram(const GLuint prog) {
  GLint status;

  glLinkProgram(prog);

#ifdef GLDEBUG
  GLint logLength;
  glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
  if (logLength > 0) {
    auto *log = (GLchar *) malloc(logLength);
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

bool CompileShader(GLuint *shader, const GLenum type, const GLchar *source) {
  if (source == nullptr) return false;

  *shader = glCreateShader(type);

  glShaderSource(*shader, 1, &source, nullptr);
  glCompileShader(*shader);

#ifdef GLDEBUG
  GLint logLength;
  glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
  if (logLength > 0) {
    auto *log = (GLchar *) malloc(logLength);
    glGetShaderInfoLog(*shader, logLength, &logLength, log);
    LOGD("Shader compile log:\n%s", log);
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

void initShaders() {
  GLuint vertexShader, fragmentShader;
  GLuint program = glCreateProgram();
  LOGD("Created Shader %d", program);

  if (!CompileShader(&vertexShader, GL_VERTEX_SHADER,
                     DefaultShader::vertexShaderCode)) {
    LOGD("Failed to compile vertex vertexShader");
    glDeleteProgram(program);
  }

  if (!CompileShader(&fragmentShader, GL_FRAGMENT_SHADER,
                     DefaultShader::fragmentShaderCode)) {
    LOGD("Failed to compile fragment fragmentShader");
    glDeleteProgram(program);
  }


  // Attach vertex vertexShader to program
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);

  glBindAttribLocation(program, 0, "vPosition");

  // Link program
  if (!LinkProgram(program)) {
    LOGD("Failed to link program: %d", program);

    if (vertexShader) {
      glDeleteShader(vertexShader);
    }
    if (program) {
      glDeleteProgram(program);
    }
  }
}
