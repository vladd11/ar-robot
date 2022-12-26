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

struct UiAnchor {
  ArAnchor *anchor;
  ArAnchor *cloudAnchor;
  ArCloudAnchorState prevCloudAnchorState;
  GLfloat *colors;
};

static void *ENGINE_KEY = nullptr;
static GLfloat stateToColor[13][4] = {
    // Errors, that can't be fixed by user, like outdated SDK
    {0.63671875f, 0.76953125f, 0.22265625f, 1.0f},
    {0.63671875f, 0.76953125f, 0.22265625f, 1.0f},
    {0.63671875f, 0.76953125f, 0.22265625f, 1.0f},
    {0.63671875f, 0.76953125f, 0.22265625f, 1.0f},
    {0.63671875f, 0.76953125f, 0.22265625f, 1.0f},
    // Dataset processing failed
    {0,           0,           0.1f,        1},
    {0.63671875f, 0.76953125f, 0.22265625f, 1.0f},
    {0.63671875f, 0.76953125f, 0.22265625f, 1.0f},
    {0.63671875f, 0.76953125f, 0.22265625f, 1.0f},
    {0.63671875f, 0.76953125f, 0.22265625f, 1.0f},
    {0.63671875f, 0.76953125f, 0.22265625f, 1.0f},
    // In progress
    {1,           0,           0,           1},
    // Completed
    {0,           1,           0,           1}
};

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

public:
  static const int ANCHORS_LIMIT = 10;

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

  void getTransformMatrixFromAnchor(const ArAnchor &arAnchor, glm::mat4 *out_model_mat);

  JNICallbacks *getCallbacks() const;

  ArSession *getArSession() const;

  ArCamera *getArCamera() const;

  ArFrame *getArFrame() const;

  ServerThread *getServerThread() const;

  GLuint mRawProgram;
};

#endif //AR_SHOP_ENGINE_H
