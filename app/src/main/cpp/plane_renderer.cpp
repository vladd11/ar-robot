#include "plane_renderer.h"
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <string>
#include <cstdint>

#include "shaders/plane.h"
#include "gl_util.h"
#include <arcode_cxx_api.hpp>

#define TAG "PlaneRenderer"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

void PlaneRenderer::init(JNIEnv *env) {
  mShaderProgram = PlaneShader::compile();
  if (!mShaderProgram) {
    LOGE("Could not create program.");
  }

  mUniformMvpMat = glGetUniformLocation(mShaderProgram, "mvp");
  mUniformTexture = glGetUniformLocation(mShaderProgram, "texture");
  mUniformModelMat = glGetUniformLocation(mShaderProgram, "model_mat");
  mUniformNormalVec = glGetUniformLocation(mShaderProgram, "normal");

  glGenTextures(1, &mTextureId);
  glBindTexture(GL_TEXTURE_2D, mTextureId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  loadPngToGl(env);
  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);

  checkGlError("plane_renderer::init()");
}

void PlaneRenderer::resume(ArSession *arSession) {
  mArSession = arSession;
}

void PlaneRenderer::draw(const glm::mat4 &projection_mat,
                         const glm::mat4 &view_mat, const ArPlane &ar_plane) {
  if (!mShaderProgram) {
    LOGE("shader_program is null.");
    return;
  }

  updatePlane(ar_plane);

  glUseProgram(mShaderProgram);
  glDepthMask(GL_FALSE);

  glActiveTexture(GL_TEXTURE0);
  glUniform1i(mUniformTexture, 0);
  glBindTexture(GL_TEXTURE_2D, mTextureId);

  // Compose final mvp matrix for this plane renderer.
  glUniformMatrix4fv(mUniformMvpMat, 1, GL_FALSE,
                     glm::value_ptr(projection_mat * view_mat * model_mat));

  glUniformMatrix4fv(mUniformModelMat, 1, GL_FALSE,
                     glm::value_ptr(model_mat));
  glUniform3f(mUniformNormalVec, normal_vec_.x, normal_vec_.y, normal_vec_.z);

  glEnableVertexAttribArray(PlaneShader::vPositionAttrIndex);
  glVertexAttribPointer(PlaneShader::vPositionAttrIndex, 3, GL_FLOAT, GL_FALSE, 0,
                        mVertices.data());

  glEnable(GL_BLEND);

  // Textures are loaded with premultiplied alpha
  // (https://developer.android.com/reference/android/graphics/BitmapFactory.Options#inPremultiplied),
  // so we use the premultiplied alpha blend factors.
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glDrawElements(GL_TRIANGLES, (int) mTriangles.size(), GL_UNSIGNED_SHORT,
                 mTriangles.data());

  glDisable(GL_BLEND);
  glUseProgram(0);
  glDepthMask(GL_TRUE);
  checkGlError("plane_renderer::draw()");
}

void PlaneRenderer::updatePlane(const ArPlane &ar_plane) {
  // The following code generates a triangle mesh filling a convex polygon,
  // including a feathered edge for blending.
  //
  // The indices shown in the diagram are used in comments below.
  // _______________     0_______________1
  // |             |      |4___________5|
  // |             |      | |         | |
  // |             | =>   | |         | |
  // |             |      | |         | |
  // |             |      |7-----------6|
  // ---------------     3---------------2

  mVertices.clear();
  mTriangles.clear();

  int32_t polygon_length;
  ArPlane_getPolygonSize(mArSession, &ar_plane, &polygon_length);

  if (polygon_length == 0) {
    LOGE("PlaneRenderer::UpdatePlane, no valid plane polygon is found");
    return;
  }

  const int32_t vertices_size = polygon_length / 2;
  std::vector<glm::vec2> raw_vertices(vertices_size);
  ArPlane_getPolygon(mArSession, &ar_plane,
                     glm::value_ptr(raw_vertices.front()));

  // Fill vertex 0 to 3. Note that the vertex.xy are used for x and z
  // position. vertex.z is used for alpha. The outer polygon's alpha
  // is 0.
  for (int32_t i = 0; i < vertices_size; ++i) {
    mVertices.emplace_back(raw_vertices[i].x, raw_vertices[i].y, 0.0f);
  }

  Ar::Pose pose(mArSession, nullptr);
  ArPlane_getCenterPose(mArSession, &ar_plane, *pose);
  pose.getMatrix(mArSession, glm::value_ptr(model_mat));
  normal_vec_ = getPlaneNormal(mArSession, **pose);

  // Feather distance 0.2 meters.
  const float kFeatherLength = 0.2f;
  // Feather scale over the distance between plane center and vertices.
  const float kFeatherScale = 0.2f;

  // Fill vertex 4 to 7, with alpha set to 1.
  for (int32_t i = 0; i < vertices_size; ++i) {
    // Vector from plane center to current point.
    glm::vec2 v = raw_vertices[i];
    const float scale =
        1.0f - std::min((kFeatherLength / glm::length(v)), kFeatherScale);
    const glm::vec2 result_v = scale * v;

    mVertices.emplace_back(result_v.x, result_v.y, 1.0f);
  }

  const auto vertices_length = (int32_t) mVertices.size();
  const int32_t half_vertices_length = vertices_length / 2;

  // Generate triangle (4, 5, 6) and (4, 6, 7).
  for (int i = half_vertices_length + 1; i < vertices_length - 1; ++i) {
    mTriangles.push_back(half_vertices_length);
    mTriangles.push_back(i);
    mTriangles.push_back(i + 1);
  }

  // Generate triangle (0, 1, 4), (4, 1, 5), (5, 1, 2), (5, 2, 6),
  // (6, 2, 3), (6, 3, 7), (7, 3, 0), (7, 0, 4)
  for (int i = 0; i < half_vertices_length; ++i) {
    mTriangles.push_back(i);
    mTriangles.push_back((i + 1) % half_vertices_length);
    mTriangles.push_back(i + half_vertices_length);

    mTriangles.push_back(i + half_vertices_length);
    mTriangles.push_back((i + 1) % half_vertices_length);
    mTriangles.push_back((i + half_vertices_length + 1) % half_vertices_length +
                         half_vertices_length);
  }
}