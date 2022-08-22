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
}