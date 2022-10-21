#include "background_renderer.h"

#define TAG "BackgroundRenderer"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

BackgroundRenderer::BackgroundRenderer() = default;

void BackgroundRenderer::init() {
  glGenTextures(1, &mCameraTextureId);
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, mCameraTextureId);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  mCameraProgram = BackgroundShader::compile();

  mCameraTextureUniform = glGetUniformLocation(mCameraProgram, "sTexture");
  mCameraPositionAttrib = glGetAttribLocation(mCameraProgram, "a_Position");
  mCameraTexCoordAttrib = glGetAttribLocation(mCameraProgram, "a_TexCoord");
}

void BackgroundRenderer::draw(float *mTransformedUVs) const {
  static_assert(std::extent<decltype(kVertices)>::value == kNumVertices * 2,
                "Incorrect kVertices length");
  if (mCameraTextureId == -1) {
    return;
  }

  glDepthMask(GL_FALSE);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, mCameraTextureId);
  glUseProgram(mCameraProgram);
  glUniform1i(mCameraTextureUniform, 0);

  // Set the vertex positions and texture coordinates.
  glVertexAttribPointer(mCameraPositionAttrib, 2, GL_FLOAT, false, 0,
                        kVertices);
  glVertexAttribPointer(mCameraTexCoordAttrib, 2, GL_FLOAT, false, 0,
                        mTransformedUVs);
  glEnableVertexAttribArray(mCameraPositionAttrib);
  glEnableVertexAttribArray(mCameraTexCoordAttrib);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glDisableVertexAttribArray(mCameraPositionAttrib);
  glDisableVertexAttribArray(mCameraTexCoordAttrib);

  glUseProgram(0);
  glDepthMask(GL_TRUE);
  checkGlError("BackgroundRenderer::draw() error");
}
