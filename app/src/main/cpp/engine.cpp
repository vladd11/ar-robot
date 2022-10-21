#include "engine.h"
#include "server.h"

#include <utility>
#include <thread>

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

Engine::Engine(std::string storagePath) {
  mLuaState = luaL_newstate();
  mStoragePath = std::move(storagePath);
  mBackgroundRenderer = new BackgroundRenderer();
  mArUiRenderer = new ArUiRenderer();
}

Engine::~Engine() {
  mInterrupt = true;
  lua_close(mLuaState);
  if (mArSession != nullptr) {
    ArSession_destroy(mArSession);
    ArFrame_destroy(mArFrame);
  }
}

void Engine::init() {
  std::thread thr = std::thread(server_thread, &mInterrupt);
  thr.detach();

  lua_register(mLuaState, "distanceToAnchor", distanceToAnchor);
  luaL_dofile(mLuaState, mStoragePath.append("/script.lua").c_str());

  lua_getglobal(mLuaState, "global");
  int test = 0;
  LOGD("%lld", lua_tointegerx(mLuaState, -1, &test));

  mBackgroundRenderer->init();
  mArUiRenderer->init();
}

int Engine::distanceToAnchor(lua_State *L) {
  LOGD("test");
  return 0;
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

  ArPose *pose = nullptr;
  ArPose_create(mArSession, nullptr, &pose);
  ArCamera_getPose(mArSession, ar_camera, pose);

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
  for (auto &uiAnchor: mAnchors) {
    ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
    ArAnchor_getTrackingState(mArSession, uiAnchor->anchor,
                              &tracking_state);
    if (tracking_state == AR_TRACKING_STATE_TRACKING) {
      getTransformMatrixFromAnchor(*uiAnchor->anchor, &model_mat);

      mArUiRenderer->draw(projection_mat * view_mat * model_mat);
    }
  }
}

void Engine::onTouch(float x, float y) {
  if (mArFrame != nullptr && mArSession != nullptr) {
    ArCamera *arCamera;
    ArFrame_acquireCamera(mArSession, mArFrame, &arCamera);

    ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED; // Default value
    ArCamera_getTrackingState(mArSession, arCamera,
                              &tracking_state);
    if (tracking_state == AR_TRACKING_STATE_TRACKING) {
      float rawMatrix[16];
      ArCamera_getViewMatrix(mArSession, arCamera, rawMatrix);
      glm::mat4 matrix = glm::make_mat4(rawMatrix);
      glm::vec4 newPosition = glm::vec4(1.f, 1.f, -5.f, 0.f) * matrix;

      float newRawPose[7] = {
          0.f,
          0.f,
          0.f,
          1.f,
          newPosition[0],
          newPosition[1],
          newPosition[2]
      };

      ArPose *newPose;
      ArPose_create(mArSession, newRawPose, &newPose);

      ArAnchor *anchor;
      ArSession_acquireNewAnchor(mArSession, newPose, &anchor);

      ArPose_destroy(newPose);

      auto *uiAnchor = new UiAnchor();
      uiAnchor->anchor = anchor;
      mAnchors.push_back(uiAnchor);
    }

    ArCamera_release(arCamera);
  }
}

void Engine::getTransformMatrixFromAnchor(const ArAnchor &ar_anchor, glm::mat4 *out_model_mat) {
  ArPose *pose;

  ArPose_create(mArSession, nullptr, &pose);
  ArAnchor_getPose(mArSession, &ar_anchor, pose);
  ArPose_getMatrix(mArSession, pose, glm::value_ptr(*out_model_mat));

  ArPose_destroy(pose);
}

void Engine::resume(JNIEnv *env, jobject context, jobject activity) {
  if (mArSession == nullptr) {
    ArInstallStatus install_status;
    CHECKANDTHROW(
        ArCoreApk_requestInstall(env, activity, true,
                                 &install_status) == AR_SUCCESS,
        env, "Please install Google Play Services for AR (ARCore).")

    switch (install_status) {
      case AR_INSTALL_STATUS_INSTALLED:
        break;
      case AR_INSTALL_STATUS_INSTALL_REQUESTED:
        return;
    }

    CHECKANDTHROW(ArSession_create(env, context, &mArSession) == AR_SUCCESS, env,
                  "Failed to create AR session")

    ArConfig *arConfig = nullptr;
    ArConfig_create(mArSession, &arConfig);

    ArConfig_setFocusMode(mArSession, arConfig, AR_FOCUS_MODE_AUTO);
    ArConfig_setDepthMode(mArSession, arConfig, AR_DEPTH_MODE_DISABLED);

    CHECK(arConfig)
    CHECK(ArSession_configure(mArSession, arConfig) == AR_SUCCESS)
    ArConfig_destroy(arConfig);

    ArFrame_create(mArSession, &mArFrame);

    ArSession_setDisplayGeometry(mArSession, mDisplayRotation, mDisplayWidth, mDisplayHeight);
  }

  ArCameraConfigFilter *filter;
  ArCameraConfigFilter_create(mArSession, &filter);

  ArCameraConfigList *list;
  ArCameraConfigList_create(mArSession, &list);

  ArSession_getSupportedCameraConfigsWithFilter(mArSession, filter, list);

  int size;
  ArCameraConfigList_getSize(mArSession, list, &size);

  int32_t bestConfigIndex = 0;
  int32_t worstResolution = 0;
  for (int i = 0; i < size; i++) {
    ArCameraConfig *config;
    ArCameraConfig_create(mArSession, &config);

    ArCameraConfigList_getItem(mArSession, list, i, config);

    int width, height;
    ArCameraConfig_getImageDimensions(mArSession, config, &width, &height);
    int resolution = width * height;
    LOGD("Available resolution: %dx%d", width, height);

    if (resolution < worstResolution) {
      worstResolution = resolution;
    } else if (resolution != worstResolution) {
      bestConfigIndex = i;
    }

    ArCameraConfig_destroy(config);
  }

  ArCameraConfig *cameraConfig;

  ArCameraConfig_create(mArSession, &cameraConfig);
  ArCameraConfigList_getItem(mArSession, list, bestConfigIndex, cameraConfig);

  ArSession_setCameraConfig(mArSession, cameraConfig);

  ArCameraConfig_destroy(cameraConfig);
  ArCameraConfigList_destroy(list);
  ArCameraConfigFilter_destroy(filter);

  const ArStatus status = ArSession_resume(mArSession);
  CHECKANDTHROW(status == AR_SUCCESS, env, "Failed to resume AR session.")
}

void Engine::pause() {
  if (mArSession != nullptr) {
    ArSession_pause(mArSession);
  }
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
