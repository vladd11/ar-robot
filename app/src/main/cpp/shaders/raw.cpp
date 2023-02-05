#include "raw.h"

namespace RawShader {
  static const char *vertexShaderCode = R"(
#version 100
attribute vec3 vPosition;
void main()
{
  gl_Position = vec4(vPosition, 1.0);
}
)";

  static const char *fragmentShaderCode = R"(
#version 100
precision mediump float;
uniform vec4 vColor;
void main() {
  gl_FragColor = vColor;
}
)";

  static GLuint program = 0;

  GLuint get() {
    if (program == 0) program = glCreateProgram();
    else return program;
    GLuint vertexShader, fragmentShader;
    CompileShader(&vertexShader, GL_VERTEX_SHADER, vertexShaderCode);
    CompileShader(&fragmentShader, GL_FRAGMENT_SHADER, fragmentShaderCode);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glBindAttribLocation(program, vPositionAttrIndex, "vPosition");

    LinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    checkGlError("RawShader::get");
    return program;
  }
}