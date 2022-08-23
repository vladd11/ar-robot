#include "engine.h"
#include "verts/triangle.h"

#define TAG "Engine"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#define CHECK(condition)                                                   \
  if (!(condition)) {                                                      \
    LOGE("*** CHECK FAILED at %s:%d: %s", __FILE__, __LINE__, #condition); \
    abort();                                                               \
  }

#define CHECKANDTHROW(condition, env, msg, ...)                            \
  if (!(condition)) {                                                      \
    LOGE("*** CHECK FAILED at %s:%d: %s", __FILE__, __LINE__, #condition); \
    jclass c = env->FindClass("java/lang/RuntimeException");               \
    env->ThrowNew(c, msg);                                                 \
    return;                                                                \
  }

Engine::Engine() {
  mBackgroundRenderer = new BackgroundRenderer();
}

void Engine::init() {
  mBackgroundRenderer->init();
}

void Engine::drawFrame() {
  // Render the scene.
  glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  if (mArSession == nullptr) return;

  ArSession_setCameraTextureName(mArSession,
                                 mBackgroundRenderer->mCameraTextureId);

  // Update session to get current frame and render camera background.
  if (ArSession_update(mArSession, frame) != AR_SUCCESS) {
    LOGE("OnDrawFrame ArSession_update error");
  }

  ArCamera *ar_camera;
  ArFrame_acquireCamera(mArSession, frame, &ar_camera);

  // If display rotation changed (also includes view size change), we need to
  // re-query the uv coordinates for the on-screen portion of the camera image.
  int32_t geometry_changed = 0;
  ArFrame_getDisplayGeometryChanged(mArSession, frame, &geometry_changed);
  if (geometry_changed != 0 || !IsUvMapsInitialized) {
    ArFrame_transformCoordinates2d(
        mArSession, frame, AR_COORDINATES_2D_OPENGL_NORMALIZED_DEVICE_COORDINATES,
        kNumVertices, kVertices, AR_COORDINATES_2D_TEXTURE_NORMALIZED,
        mTransformedUVs);
    IsUvMapsInitialized = true;
  }

  int64_t frame_timestamp;
  ArFrame_getTimestamp(mArSession, frame, &frame_timestamp);
  if (frame_timestamp == 0) {
    // Suppress rendering if the camera did not produce the first frame yet.
    // This is to avoid drawing possible leftover data from previous sessions if
    // the texture is reused.
    return;
  }

  mBackgroundRenderer->drawFrame(mTransformedUVs);
}

bool Engine::IsDepthSupported() {
  int32_t is_supported = 0;
  ArSession_isDepthModeSupported(mArSession, AR_DEPTH_MODE_AUTOMATIC,
                                 &is_supported);
  return is_supported;
}

void Engine::resume(JNIEnv *env, jobject context, jobject activity) {
  if (mArSession == nullptr) {
    CHECKANDTHROW(ArSession_create(env, context, &mArSession) == AR_SUCCESS, env,
                  "Failed to create AR session");

    ArConfig *ar_config = nullptr;
    ArConfig_create(mArSession, &ar_config);

    if (IsDepthSupported()) {
      ArConfig_setDepthMode(mArSession, ar_config, AR_DEPTH_MODE_AUTOMATIC);
    } else {
      ArConfig_setDepthMode(mArSession, ar_config, AR_DEPTH_MODE_DISABLED);
    }

    CHECK(ar_config)
    CHECK(ArSession_configure(mArSession, ar_config) == AR_SUCCESS)
    ArConfig_destroy(ar_config);

    ArFrame_create(mArSession, &frame);

    ArSession_setDisplayGeometry(mArSession, mDisplayRotation, mDisplayWidth, mDisplayHeight);
  }
  const ArStatus status = ArSession_resume(mArSession);
  CHECKANDTHROW(status == AR_SUCCESS, env, "Failed to resume AR session.");
}

void Engine::resize(int rotation, int width, int height) {
  LOGD("OnSurfaceChanged(%d, %d)", width, height);
  glViewport(0, 0, width, height);
  mDisplayRotation = rotation;
  mDisplayWidth = width;
  mDisplayHeight = height;
  if (mArSession != nullptr) {
    ArSession_setDisplayGeometry(mArSession, rotation, width, height);
  }
}
