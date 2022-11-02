#ifndef AR_SHOP_ENGINE_H
#define AR_SHOP_ENGINE_H

#include "arcore_c_api.h"
#include "background_renderer.h"
#include "ar_ui_renderer.h"
#include "plane_renderer.h"
#include "server.h"
#include <cstdlib>
#include <string>
#include <vector>

extern "C" {
#include "lua.h"
}

struct UiAnchor {
  ArAnchor *anchor;
  ArAnchor *cloudAnchor;
};

static void *ENGINE_KEY = nullptr;

class Engine {
private:
  std::string mStoragePath;
  std::vector<UiAnchor *> mAnchors;
  ArFrame *mArFrame{};
  ArCamera *mArCamera;
  BackgroundRenderer *mBackgroundRenderer;
  ArUiRenderer *mArUiRenderer;
  PlaneRenderer *mPlaneRenderer;
  ArSession *mArSession{};
  int mDisplayRotation = 0, mDisplayWidth = 1, mDisplayHeight = 1;

  ServerThread *mServerThread;

  bool IsUvMapsInitialized{};
  float mTransformedUVs[kNumVertices * 2]{};

public:
  std::vector<UiAnchor *> Anchors() const;

  ArSession *ArSession() const;

  ArCamera *getArCamera() const;

  ArFrame *ArFrame() const;

  ServerThread *ServerThread() const;

  static const int ANCHORS_LIMIT = 10;
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
};

#endif //AR_SHOP_ENGINE_H
