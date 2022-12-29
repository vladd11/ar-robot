#ifndef AR_SHOP_ENGINE_H
#define AR_SHOP_ENGINE_H

#include "arcore_c_api.h"
#include "background_renderer.h"
#include "ar_ui_renderer.h"
#include "plane_renderer.h"
#include "server.h"
#include <string>
#include <vector>

extern "C" {
#include "lua.h"
}

#define DEFAULT_ANCHOR_COLOR {0.63671875f, 0.76953125f, 0.22265625f, 1.0f}

struct CloudAnchor {
  ArAnchor *anchor;
  ArCloudAnchorState prevCloudAnchorState = AR_CLOUD_ANCHOR_STATE_NONE;
};

struct UiAnchor {
  ArAnchor *anchor;
  CloudAnchor *cloudAnchorState;
  GLfloat *color;

  bool isRelative = false;
  ArPlane *plane;
  glm::vec3 relativePos;
};

static void *ENGINE_KEY = nullptr;
static GLfloat stateToColor[13][4] = {
    // Errors, that can't be fixed by user, like outdated SDK
    DEFAULT_ANCHOR_COLOR,
    DEFAULT_ANCHOR_COLOR,
    DEFAULT_ANCHOR_COLOR,
    DEFAULT_ANCHOR_COLOR,
    DEFAULT_ANCHOR_COLOR,
    // Dataset processing failed
    {0, 0, 0.1f, 1},
    DEFAULT_ANCHOR_COLOR,
    DEFAULT_ANCHOR_COLOR,
    DEFAULT_ANCHOR_COLOR,
    DEFAULT_ANCHOR_COLOR,
    DEFAULT_ANCHOR_COLOR,
    // In progress
    {1, 0, 0, 1},
    // Completed
    {0, 1, 0, 1}
};

const static float MAX_RELATIVE_ANCHOR_DISTANCE = 8; // meters
class Engine {
private:
  JNICallbacks *mCallbacks{};
  std::string mStoragePath;
  ArFrame *mArFrame{};
  ArCamera *mArCamera{};
  BackgroundRenderer *mBackgroundRenderer;
  ArUiRenderer *mArUiRenderer;
  PlaneRenderer *mPlaneRenderer;
  ArSession *mArSession{};
  int mDisplayRotation = 0, mDisplayWidth = 1, mDisplayHeight = 1;

  ServerThread *mServerThread;

  bool mIsUvMapsInitialized{};
  float mTransformedUVs[kNumVertices * 2]{};

  UiAnchor *getSimilarAnchors(ArPlane *plane, glm::vec3 pos);

public:
  std::vector<UiAnchor *> mAnchors;

  lua_State *mLuaState;

  Engine(std::string storagePath, JNICallbacks *callbacks);

  ~Engine();

  void init();

  void drawFrame();

  void resize(int rotation, int width, int height);

  void onTouch(float x, float y);

  void resume(JNIEnv *env, jobject context, jobject activity);

  void pause();

  JNICallbacks *getCallbacks() const;

  ArSession *getArSession() const;

  ArCamera *getArCamera() const;

  ArFrame *getArFrame() const;

  ServerThread *getServerThread() const;
};

#endif //AR_SHOP_ENGINE_H
