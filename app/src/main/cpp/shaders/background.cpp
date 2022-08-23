#include "background.h"

#define TAG "BackgroundShader"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

namespace BackgroundShader {
  /*
   * This shader taken from https://github.com/google-ar/arcore-android-sdk
   * Copyright 2017 Google LLC
   *
   * Licensed under the Apache License, Version 2.0 (the "License");
   * you may not use this file except in compliance with the License.
   * You may obtain a copy of the License at
   *
   *   http://www.apache.org/licenses/LICENSE-2.0
   *
   * Unless required by applicable law or agreed to in writing, software
   * distributed under the License is distributed on an "AS IS" BASIS,
   * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   * See the License for the specific language governing permissions and
   * limitations under the License.
   */
  static const char *vertexShaderCode = R"(
attribute vec4 a_Position;
attribute vec2 a_TexCoord;

varying vec2 v_TexCoord;

void main() {
   gl_Position = a_Position;
   v_TexCoord = a_TexCoord;
}
)";

  static const char *fragmentShaderCode = R"(
#extension GL_OES_EGL_image_external : require

precision mediump float;
varying vec2 v_TexCoord;
uniform samplerExternalOES sTexture;


void main() {
    gl_FragColor = texture2D(sTexture, v_TexCoord);
}
)";

  GLuint compile() {
    bool success = true;

    GLuint vertexShader, fragmentShader;
    GLuint program = glCreateProgram();

    success = success | (!CompileShader(&vertexShader, GL_VERTEX_SHADER, vertexShaderCode));
    success = success | (!CompileShader(&fragmentShader, GL_FRAGMENT_SHADER, fragmentShaderCode));

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    success = success | (!LinkProgram(program));

    if(!success) {
      LOGE("Can't compile shader program");
    }
    return program;
  }
}