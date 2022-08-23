//
// Created by vladislav on 8/23/22.
//

#include "ar_ui_renderer.h"

ArUiRenderer::ArUiRenderer() = default;

void ArUiRenderer::init() {
  mDefaultProgram = DefaultShader::compile();
  initializationTime = std::chrono::system_clock::now();
}

void ArUiRenderer::draw(glm::mat<4, 4, glm::f32> projection) {
  glUseProgram(mDefaultProgram);

  glEnableVertexAttribArray(DefaultShader::vPositionAttrIndex);

  GLint vColorLocation = glGetUniformLocation(mDefaultProgram, "vColor");
  glUniform4fv(vColorLocation, 1, Triangle::kColors);

  GLint mvpLocation = glGetUniformLocation(mDefaultProgram, "mvp");
  glm::mat4 trans = glm::mat4(1.0f);

  std::chrono::duration<float> elapsed = std::chrono::system_clock::now() - initializationTime;
  trans = glm::rotate<float>(trans, static_cast<float>(elapsed.count()),
                             glm::vec3(0.f, 0.f, 0.01f));
  trans = trans * projection;
  glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(trans));

  glVertexAttribPointer(
      DefaultShader::vPositionAttrIndex,
      COORDS_PER_VERTEX,
      GL_FLOAT,
      false,
      VERTEX_STRIDE,
      Triangle::kVertices
  );

  glDrawArrays(GL_TRIANGLES, 0, 3);

  glDisableVertexAttribArray(DefaultShader::vPositionAttrIndex);

  CheckGlError("Triangle draw failed");
  glUseProgram(0);
}