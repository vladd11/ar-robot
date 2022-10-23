#ifndef AR_SHOP_ENGINE_H
#define AR_SHOP_ENGINE_H

extern "C" {
#include "../../../build/lua/lua-5.4.4/lua.h"
#include "../../../build/lua/lua-5.4.4/lauxlib.h"
#include "../../../build/lua/lua-5.4.4/lualib.h"
}

#include "mongoose.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <jni.h>
#include <stdlib.h>
#include <fstream>
#include <array>
#include <thread>

#include "arcore_c_api.h"

#include "shaders/default.h"
#include "verts/triangle.h"
#include "background_renderer.h"
#include "plane_renderer.h"
#include "yuv2rgb.h"
#include "ar_ui_renderer.h"
#include "verts/triangle.h"
#include "glm.h"
#include "server.h"

class Engine {
private:
  struct UiAnchor {
    ArAnchor *anchor;
  };
  std::string mStoragePath;
  std::vector<UiAnchor *> mAnchors;
  ArFrame *mArFrame{};
  BackgroundRenderer *mBackgroundRenderer;
  ArUiRenderer *mArUiRenderer;
  PlaneRenderer *mPlaneRenderer;
  ArSession *mArSession{};
  int mDisplayRotation = 0, mDisplayWidth = 1, mDisplayHeight = 1;

  ServerThread *mServerThread;
  bool mInterrupt = false;

  bool IsUvMapsInitialized{};
  float mTransformedUVs[kNumVertices * 2]{};

public:
  lua_State *mLuaState;

  Engine(std::string storagePath, JNIEnv *env);

  ~Engine();

  void init();

  void drawFrame();

  void resize(int rotation, int width, int height);

  void onTouch(float x, float y);

  void resume(JNIEnv *env, jobject context, jobject activity);

  void pause();

  void getTransformMatrixFromAnchor(const ArAnchor &ar_anchor, glm::mat4 *out_model_mat);

  void getCameraPosition(const ArCamera &ar_camera, float *out_pose);

  static int distanceToAnchor(lua_State *L);

  static int log(lua_State *L);
};

#endif //AR_SHOP_ENGINE_H
