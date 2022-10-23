#include "raw.h"

namespace RawShader {
  static const char *vertexShaderCode = R"(
attribute vec3 vPosition;
void main()
{
  gl_Position = vec4(vPosition, 1.0f);
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
    checkGlError("RawShader::compile");
    return program;
  }
}