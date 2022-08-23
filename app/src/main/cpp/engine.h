//
// Created by vladislav on 8/22/22.
//

#ifndef AR_SHOP_ENGINE_H
#define AR_SHOP_ENGINE_H

#include <GLES2/gl2.h>
#include <jni.h>
#include <stdlib.h>

#include "shaders/default.h"
#include "verts/triangle.h"
#include "arcore_c_api.h"
#include "background_renderer.h"

class Engine {
private:
  GLuint mDefaultProgram, mTextureId;
  GLint mTextureUniform;
  ArFrame *frame{};
  BackgroundRenderer *mBackgroundRenderer;
  ArSession *mArSession{};
  int mDisplayRotation = 0, mDisplayWidth = 1, mDisplayHeight = 1;

  bool IsUvMapsInitialized{};
  float mTransformedUVs[kNumVertices * 2]{};

public:
  Engine();

  void init();

  void drawFrame();

  void resize(int rotation, int width, int height);

  void resume(JNIEnv *env, jobject context, jobject activity);

  bool IsDepthSupported();
};


#endif //AR_SHOP_ENGINE_H
