#include "ar_ui_renderer.h"

#define TAG "ArUiRenderer"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

ArUiRenderer::ArUiRenderer() = default;
ArUiRenderer::~ArUiRenderer() = default;

void ArUiRenderer::init() {
  mDefaultProgram = DefaultShader::get();
  mRawProgram = RawShader::get();

  glGenBuffers(1, &mElementBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, mElementBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Triangle::kVertices),
               Triangle::kVertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ArUiRenderer::draw(glm::mat<4, 4, glm::f32> mvp, const GLfloat color[]) const {
  glUseProgram(mDefaultProgram);

  GLint vColorLocation = glGetUniformLocation(mDefaultProgram, "vColor");
  glUniform4fv(vColorLocation, 1, color);

  GLint mvpLocation = glGetUniformLocation(mDefaultProgram, "mvp");
  glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));

  glBindBuffer(GL_ARRAY_BUFFER, mElementBuffer);
  glVertexAttribPointer(DefaultShader::vPositionAttrIndex, 3, GL_FLOAT, GL_FALSE, VERTEX_STRIDE,
                        nullptr);
  glEnableVertexAttribArray(DefaultShader::vPositionAttrIndex);

  glDrawArrays(
      GL_TRIANGLES,
      0,
      Triangle::kNumVertices
  );
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  checkGlError("Triangle draw failed");
  glUseProgram(0);
}
