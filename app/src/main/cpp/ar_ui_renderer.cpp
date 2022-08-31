#include "ar_ui_renderer.h"

ArUiRenderer::ArUiRenderer() = default;

void ArUiRenderer::init() {
  mDefaultProgram = DefaultShader::compile();
}

void ArUiRenderer::draw(glm::mat<4, 4, glm::f32> mvp) const {
  glUseProgram(mDefaultProgram);

  glEnableVertexAttribArray(DefaultShader::vPositionAttrIndex);

  GLint vColorLocation = glGetUniformLocation(mDefaultProgram, "vColor");
  glUniform4fv(vColorLocation, 1, Triangle::kColors);

  GLint mvpLocation = glGetUniformLocation(mDefaultProgram, "mvp");
  glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));

  glVertexAttribPointer(
      DefaultShader::vPositionAttrIndex,
      COORDS_PER_VERTEX,
      GL_FLOAT,
      false,
      VERTEX_STRIDE,
      Triangle::kVertices
  );

  glDrawArrays(GL_TRIANGLE_STRIP, 0, Triangle::kNumVertices);

  glDisableVertexAttribArray(DefaultShader::vPositionAttrIndex);

  CheckGlError("Triangle draw failed");
  glUseProgram(0);
}