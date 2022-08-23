#include "engine.h"

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
  mArUiRenderer = new ArUiRenderer();
}

Engine::~Engine() {
  if (mArSession != nullptr) {
    ArSession_destroy(mArSession);
    ArFrame_destroy(mArFrame);
  }
}

void Engine::init() {
  mBackgroundRenderer->init();
  mArUiRenderer->init();
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

  // Update session to get current mArFrame and render camera background.
  if (ArSession_update(mArSession, mArFrame) != AR_SUCCESS) {
    LOGE("OnDrawFrame ArSession_update error");
  }

  ArCamera *ar_camera;
  ArFrame_acquireCamera(mArSession, mArFrame, &ar_camera);

  // If display rotation changed (also includes view size change), we need to
  // re-query the uv coordinates for the on-screen portion of the camera image.
  int32_t geometry_changed = 0;
  ArFrame_getDisplayGeometryChanged(mArSession, mArFrame, &geometry_changed);
  if (geometry_changed != 0 || !IsUvMapsInitialized) {
    ArFrame_transformCoordinates2d(
        mArSession, mArFrame, AR_COORDINATES_2D_OPENGL_NORMALIZED_DEVICE_COORDINATES,
        kNumVertices, kVertices, AR_COORDINATES_2D_TEXTURE_NORMALIZED,
        mTransformedUVs);
    IsUvMapsInitialized = true;
  }

  glm::mat4 view_mat;
  glm::mat4 projection_mat;
  ArCamera_getViewMatrix(mArSession, ar_camera, glm::value_ptr(view_mat));
  ArCamera_getProjectionMatrix(mArSession, ar_camera, /*near=*/0.1f, /*far=*/100.f,
                               glm::value_ptr(projection_mat));

  int64_t frame_timestamp;
  ArFrame_getTimestamp(mArSession, mArFrame, &frame_timestamp);
  if (frame_timestamp != 0) {
    mBackgroundRenderer->draw(mTransformedUVs);
  }

  ArTrackingState camera_tracking_state;
  ArCamera_getTrackingState(mArSession, ar_camera, &camera_tracking_state);
  ArCamera_release(ar_camera);

  ArTrackableList *plane_list = nullptr;
  ArTrackableList_create(mArSession, &plane_list);
  CHECK(plane_list != nullptr)

  ArTrackableType plane_tracked_type = AR_TRACKABLE_PLANE;
  ArSession_getAllTrackables(mArSession, plane_tracked_type, plane_list);

  int32_t plane_list_size = 0;
  ArTrackableList_getSize(mArSession, plane_list, &plane_list_size);

  for (int i = 0; i < plane_list_size; ++i) {
    ArTrackable *ar_trackable = nullptr;
    ArTrackableList_acquireItem(mArSession, plane_list, i, &ar_trackable);
    ArPlane *ar_plane = ArAsPlane(ar_trackable);
    ArTrackingState out_tracking_state;
    ArTrackable_getTrackingState(mArSession, ar_trackable,
                                 &out_tracking_state);

    ArPlane *subsume_plane;
    ArPlane_acquireSubsumedBy(mArSession, ar_plane, &subsume_plane);
    if (subsume_plane != nullptr) {
      ArTrackable_release(ArAsTrackable(subsume_plane));
      ArTrackable_release(ar_trackable);
      continue;
    }

    if (ArTrackingState::AR_TRACKING_STATE_TRACKING != out_tracking_state) {
      ArTrackable_release(ar_trackable);
      continue;
    }
    ArTrackable_release(ar_trackable);
  }

  ArTrackableList_destroy(plane_list);
  plane_list = nullptr;

  glm::mat4 model_mat(1.0f);
  for (auto &colored_anchor: mAnchors) {
    ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
    ArAnchor_getTrackingState(mArSession, colored_anchor.anchor,
                              &tracking_state);
    if (tracking_state == AR_TRACKING_STATE_TRACKING) {
      GetTransformMatrixFromAnchor(*colored_anchor.anchor, &model_mat);
      mArUiRenderer->draw(projection_mat);
    }
  }
}

void Engine::GetTransformMatrixFromAnchor(const ArAnchor &ar_anchor, glm::mat4 *out_model_mat) {
  ArPose *pose;

  ArPose_create(mArSession, nullptr, &pose);
  ArAnchor_getPose(mArSession, &ar_anchor, pose);
  ArPose_getMatrix(mArSession, pose, glm::value_ptr(*out_model_mat));

  ArPose_destroy(pose);
}

bool Engine::IsDepthSupported() {
  int32_t is_supported = 0;
  ArSession_isDepthModeSupported(mArSession, AR_DEPTH_MODE_AUTOMATIC, &is_supported);
  return is_supported;
}

void Engine::resume(JNIEnv *env, jobject context, jobject activity) {
  if (mArSession == nullptr) {
    CHECKANDTHROW(ArSession_create(env, context, &mArSession) == AR_SUCCESS, env,
                  "Failed to create AR session")

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

    ArFrame_create(mArSession, &mArFrame);

    ArSession_setDisplayGeometry(mArSession, mDisplayRotation, mDisplayWidth, mDisplayHeight);
  }
  const ArStatus status = ArSession_resume(mArSession);
  CHECKANDTHROW(status == AR_SUCCESS, env, "Failed to resume AR session.")
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
