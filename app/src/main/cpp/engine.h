//
// Created by vladislav on 8/22/22.
//

#ifndef AR_SHOP_ENGINE_H
#define AR_SHOP_ENGINE_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <jni.h>
#include <stdlib.h>
#include <fstream>

#include "arcore_c_api.h"

#include "shaders/default.h"
#include "verts/triangle.h"
#include "background_renderer.h"
#include "yuv2rgb.h"
#include "base64.h"
#include "ar_ui_renderer.h"
#include "verts/triangle.h"
#include "glm.h"


class Engine {
private:
  struct UiAnchor {
    ArAnchor *anchor;
  };

  std::vector<UiAnchor *> mAnchors;
  ArFrame *mArFrame{};
  BackgroundRenderer *mBackgroundRenderer;
  ArUiRenderer *mArUiRenderer;
  ArSession *mArSession{};
  int mDisplayRotation = 0, mDisplayWidth = 1, mDisplayHeight = 1;

  bool IsUvMapsInitialized{};
  float mTransformedUVs[kNumVertices * 2]{};

public:
  Engine();
  ~Engine();

  void init();

  void drawFrame();

  void resize(int rotation, int width, int height);

  void onTouch(float x, float y);

  void resume(JNIEnv *env, jobject context, jobject activity);

  void pause();

  void takeFrame(JNIEnv *env, jobject thiz);

  void GetTransformMatrixFromAnchor(const ArAnchor &ar_anchor, glm::mat4 *out_model_mat);
};


#endif //AR_SHOP_ENGINE_H
