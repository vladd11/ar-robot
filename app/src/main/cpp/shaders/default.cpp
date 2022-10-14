#include "default.h"

namespace DefaultShader {
  static const char *vertexShaderCode = R"(
attribute vec4 vPosition;
uniform mat4 mvp;
void main()
{
  gl_Position = mvp * vPosition;
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

    glBindAttribLocation(program, vPositionAttrIndex, "vPosition");

    LinkProgram(program);

    return program;
  }
}