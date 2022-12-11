// Almost copy-pasted from ARCore samples
#ifndef AR_SHOP_PLANE_RENDERER_H
#define AR_SHOP_PLANE_RENDERER_H

#include <GLES2/gl2.h>
#include <jni.h>
#include "arcore_c_api.h"
#include "glm.h"
#include <vector>

class PlaneRenderer {
private:
  ArSession *mArSession;
  std::vector<glm::vec3> mVertices = {};
  std::vector<GLushort> mTriangles = {};
  glm::mat4 model_mat = glm::mat4(1.0f);
  glm::vec3 normal_vec_ = glm::vec3(0.0f);
  GLuint mTextureId;

  GLuint mShaderProgram;

  GLint mUniformMvpMat;
  GLint mUniformTexture;
  GLint mUniformModelMat;
  GLint mUniformNormalVec;

  void updatePlane(const ArPlane &ar_plane);

public:
  PlaneRenderer() = default;

  ~PlaneRenderer() = default;

  // Sets up OpenGL state used by the plane renderer.  Must be called on the
  // OpenGL thread.
  void init(JNIEnv *env);

  // Updates current AR session
  void resume(ArSession *arSession);

  // Draws the provided plane.
  void draw(const glm::mat4 &projection_mat, const glm::mat4 &view_mat, const ArPlane &ar_plane);
};

#endif //AR_SHOP_PLANE_RENDERER_H
