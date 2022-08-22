#include "default.h"

namespace DefaultShader {
  static const char *vertexShaderCode = R"(
attribute vec4 vPosition;
void main()
{
  gl_Position = vPosition;
}
)";

  static const char *fragmentShaderCode = R"(
precision mediump float;
uniform vec4 vColor;
void main() {
  gl_FragColor = vColor;
}
)";

  GLuint compile() {
    GLuint vertexShader, fragmentShader;
    GLuint program = glCreateProgram();
    CompileShader(&vertexShader, GL_VERTEX_SHADER, vertexShaderCode);
    CompileShader(&fragmentShader, GL_FRAGMENT_SHADER, fragmentShaderCode);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    LinkProgram(program);

    return program;
  }
}