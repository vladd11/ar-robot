#include "engine.h"

#include "mongoose.h"

#include <android/log.h>
#include <thread>
#include <arcode_cxx_api.hpp>

#include "background_renderer.h"
#include "plane_renderer.h"
#include "ar_ui_renderer.h"
#include "verts/triangle.h"
#include "glm.h"
#include "server.h"
#include "bindings.h"
#include "lualines.h"

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

Engine::Engine(std::string storagePath, JNICallbacks *callbacks) {
  mCallbacks = callbacks;
  mStoragePath = std::move(storagePath);
  mLuaState = createLuaState(mStoragePath);
  openLines(mLuaState);
  pushStruct(mLuaState, this, (void *) ENGINE_KEY);

  mServerThread = new class ServerThread();
  mBackgroundRenderer = new BackgroundRenderer();
  mPlaneRenderer = new PlaneRenderer();
  mArUiRenderer = new ArUiRenderer();
}

Engine::~Engine() {
  mServerThread->mInterrupt = true;
  lua_close(mLuaState);
  if (mArSession != nullptr) {
    ArSession_destroy(mArSession);
    ArFrame_destroy(mArFrame);
  }
  delete mCallbacks;
  delete mBackgroundRenderer;
  delete mArUiRenderer;
  delete mServerThread;
}

void Engine::init() {
  initGL();
  mCallbacks->attach();
  std::thread thr{[this] {
      (*mServerThread)();
  }};
  thr.detach();

  if (luaL_dofile(mLuaState, mStoragePath.append("/script.lua").c_str()) != LUA_OK) {
    printLuaError(mLuaState);
  }

  mPlaneRenderer->init(mCallbacks);
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

  ArFrame_acquireCamera(mArSession, mArFrame, &mArCamera);

  Ar::Pose pose(mArSession, nullptr);
  ArCamera_getPose(mArSession, mArCamera, *pose);

  // If display rotation changed (also includes view size change), we need to
  // re-query the uv coordinates for the on-screen portion of the camera image.
  int32_t geometry_changed = 0;
  ArFrame_getDisplayGeometryChanged(mArSession, mArFrame, &geometry_changed);
  if (geometry_changed != 0 || !mIsUvMapsInitialized) {
    ArFrame_transformCoordinates2d(
        mArSession, mArFrame, AR_COORDINATES_2D_OPENGL_NORMALIZED_DEVICE_COORDINATES,
        kNumVertices, kVertices, AR_COORDINATES_2D_TEXTURE_NORMALIZED,
        mTransformedUVs);
    mIsUvMapsInitialized = true;
  }

  ArCamera_getViewMatrix(mArSession, mArCamera, glm::value_ptr(mViewMatrix));
  ArCamera_getProjectionMatrix(mArSession, mArCamera, /*near=*/0.1f, /*far=*/100.f,
                               glm::value_ptr(mProjectionMatrix));

  int64_t frame_timestamp;
  ArFrame_getTimestamp(mArSession, mArFrame, &frame_timestamp);
  if (frame_timestamp != 0) {
    mBackgroundRenderer->draw(mTransformedUVs);
  }

  ArTrackingState camera_tracking_state;
  ArCamera_getTrackingState(mArSession, mArCamera, &camera_tracking_state);

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
    mPlaneRenderer->draw(mProjectionMatrix, mViewMatrix, *ar_plane);

    ArTrackable_release(ar_trackable);
  }

  ArTrackableList_destroy(plane_list);
  plane_list = nullptr;

  size_t count = mAnchors.size();

  mPositions.resize(count * 3);

  glm::mat4 model_mat(1.0f);

  int i = 0;
  for (auto &uiAnchor: mAnchors) {
    ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
    ArAnchor_getTrackingState(mArSession, uiAnchor->anchor,
                              &tracking_state);
    if (tracking_state == AR_TRACKING_STATE_TRACKING) {
      Ar::Pose anchorPose(mArSession, nullptr);

      ArAnchor_getPose(mArSession, uiAnchor->anchor, *anchorPose);
      anchorPose.getMatrix(glm::value_ptr(model_mat));

      if (uiAnchor->isRelative) model_mat[3] += glm::vec4(uiAnchor->relativePos, 0);

      glm::mat4 mvp = mProjectionMatrix * mViewMatrix * model_mat;
      glm::vec4 vec = mvp[3] / mvp[3][3];

      mPositions[i] = vec.x;
      mPositions[i + 1] = vec.y;
      mPositions[i + 2] = vec.z;

      if (uiAnchor->cloudAnchorState->anchor != nullptr) {
        ArCloudAnchorState state;
        ArAnchor_getCloudAnchorState(mArSession, uiAnchor->cloudAnchorState->anchor, &state);
        uiAnchor->color = stateToColor[state + 10];
        if (state != uiAnchor->cloudAnchorState->prevCloudAnchorState) {
          char *id;
          ArAnchor_acquireCloudAnchorId(mArSession, uiAnchor->cloudAnchorState->anchor, &id);

          onAnchorUpdate(mLuaState, i / 3, state, id);
          ArString_release(id);

          uiAnchor->cloudAnchorState->prevCloudAnchorState = state;
        }
      }

      mArUiRenderer->draw(mvp, uiAnchor->color);
      i += 3;
    }
  }

  if (mServerThread->mUpdateCode) {
    auto *str = new std::string();
    if (loadCode(mLuaState, *mServerThread->mCodeStr, &str) != LUA_OK) {
      mServerThread->out.enqueue(str);
    } else delete str;
    mServerThread->mUpdateCode = false;
  }
  tick(mLuaState);

  ArCamera_release(mArCamera);
}

void Engine::onTouch(float x, float y) {
  for (int i = 0, idx = 0; i < mPositions.size(); i += 3, ++idx) {
    float screenX = (mPositions[i] + 1) * (float) mDisplayWidth / 2;
    float screenY = (float) mDisplayHeight - ((mPositions[i + 1] + 1) * (float) mDisplayHeight / 2);
    if(glm::distance(glm::vec2(screenX, screenY), glm::vec2(x, y)) < 120) {
      touch(mLuaState, idx);
      return;
    }
  }

  if (mArFrame != nullptr && mArSession != nullptr) {
    ArHitResultList *list;
    ArHitResultList_create(mArSession, &list);
    CHECK(list)

    ArFrame_hitTest(mArSession, mArFrame, x, y, list);

    int size = 0;
    ArHitResultList_getSize(mArSession, list, &size);

    ArHitResult *finalHit = nullptr;
    ArPose *hitPose = nullptr;
    ArPlane *plane;

    for (int i = 0; i < size; i++) {
      ArHitResult *hit = nullptr;
      ArHitResult_create(mArSession, &hit);
      ArHitResultList_getItem(mArSession, list, i, hit);

      if (hit == nullptr) {
        LOGE("Can't get hit on line %i %s", __LINE__, __FILE__);
        ArHitResultList_destroy(list);
        return;
      }

      ArTrackable *ar_trackable = nullptr;
      ArHitResult_acquireTrackable(mArSession, hit, &ar_trackable);
      ArTrackableType trackableType = AR_TRACKABLE_NOT_VALID;
      ArTrackable_getType(mArSession, ar_trackable, &trackableType);
      // Creates an anchor if a plane or an oriented point was hit.
      if (trackableType == AR_TRACKABLE_PLANE) {
        ArPose_create(mArSession, nullptr, &hitPose);
        ArHitResult_getHitPose(mArSession, hit, hitPose);
        int32_t insidePolygon = 0;
        plane = ArAsPlane(ar_trackable);
        ArPlane_isPoseInPolygon(mArSession, plane, hitPose, &insidePolygon);

        // Use hit pose and camera pose to check if hittest is from the
        // back of the plane, if it is, no need to create the anchor.
        Ar::Pose cameraPose(mArSession, nullptr);

        ArCamera *arCamera;
        ArFrame_acquireCamera(mArSession, mArFrame, &arCamera);
        ArCamera_getPose(mArSession, arCamera, *cameraPose);
        ArCamera_release(arCamera);
        float distanceToPlane = calculateDistanceToPlane(
            mArSession, *hitPose, **cameraPose);

        if (!insidePolygon || distanceToPlane < 0) {
          ArPose_destroy(hitPose);
          hitPose = nullptr;
          continue;
        }

        finalHit = hit;
        break;
      } else if (trackableType == AR_TRACKABLE_POINT) {
        ArPoint *arPoint = ArAsPoint(ar_trackable);
        ArPointOrientationMode mode;
        ArPoint_getOrientationMode(mArSession, arPoint, &mode);
        if (AR_POINT_ORIENTATION_ESTIMATED_SURFACE_NORMAL == mode) {
          ArPose_create(mArSession, nullptr, &hitPose);
          ArPoint_getPose(mArSession, arPoint, hitPose);

          finalHit = hit;
          plane = nullptr;
          break;
        }
      }
    }
    ArHitResultList_destroy(list);

    if (finalHit) {
      UiAnchor *anchor = nullptr;

      if (plane != nullptr) {
        Ar::Pose thisPose(mArSession, nullptr);
        ArHitResult_getHitPose(mArSession, finalHit, *thisPose);

        float rawThisPose[7];
        thisPose.getPoseRaw(rawThisPose);
        glm::vec3 thisPoseVec(rawThisPose[4], rawThisPose[5], rawThisPose[6]);

        UiAnchor *relAnchor = getSimilarAnchors(plane, thisPoseVec);
        if (relAnchor != nullptr) {
          Ar::Pose anchorPose(mArSession, nullptr);
          ArAnchor_getPose(mArSession, relAnchor->anchor, *anchorPose);

          float rawPose[7];
          anchorPose.getPoseRaw(rawPose);
          glm::vec3 anchorVecPose = glm::vec3(rawPose[4], rawPose[5], rawPose[6]);

          anchor = new UiAnchor{
              .anchor = relAnchor->anchor,
              .cloudAnchorState=relAnchor->cloudAnchorState,
              .color=new GLfloat[]DEFAULT_ANCHOR_COLOR,
              .isRelative=true,
              .plane=relAnchor->plane,
              .relativePos=thisPoseVec - anchorVecPose
          };
        }
      }

      if (anchor == nullptr) {
        anchor = new UiAnchor();
        if (ArHitResult_acquireNewAnchor(mArSession, finalHit, &anchor->anchor) != AR_SUCCESS) {
          LOGD("Can't acquire new anchor");
          return;
        }

        ArTrackingState state = AR_TRACKING_STATE_STOPPED;
        ArAnchor_getTrackingState(mArSession, anchor->anchor, &state);
        if (state != AR_TRACKING_STATE_TRACKING) {
          ArAnchor_release(anchor->anchor);
          return;
        }

        ArTrackable *ar_trackable = nullptr;
        ArHitResult_acquireTrackable(mArSession, finalHit, &ar_trackable);
        ArHitResult_destroy(finalHit);

        anchor->cloudAnchorState = new CloudAnchor();
        anchor->isRelative = false;
        anchor->plane = plane;
        anchor->color = new GLfloat[4]
            DEFAULT_ANCHOR_COLOR;
        anchor->plane = ArAsPlane(ar_trackable);
      }

      mAnchors.push_back(anchor);
    }
  }
}

UiAnchor *Engine::getSimilarAnchors(ArPlane *plane, glm::vec3 pos) {
  ArTrackableList *trackableList;
  ArTrackableList_create(mArSession, &trackableList);
  ArSession_getAllTrackables(mArSession, AR_TRACKABLE_PLANE, trackableList);

  int trackableListSize;
  ArTrackableList_getSize(mArSession, trackableList, &trackableListSize);

  for (int j = 0; j < trackableListSize; ++j) {
    ArTrackable *trackable;
    ArTrackableList_acquireItem(mArSession, trackableList, j, &trackable);

    ArPlane *trackPlane = ArAsPlane(trackable);
    ArPlane *parentPlane;
    ArPlane_acquireSubsumedBy(mArSession, trackPlane, &parentPlane);
    if (parentPlane == nullptr) parentPlane = trackPlane;

    if (parentPlane == plane) {
      for (UiAnchor *uiAnchor: mAnchors) {
        if (uiAnchor->isRelative) continue;

        Ar::Pose pose(mArSession, nullptr);
        ArAnchor_getPose(mArSession, uiAnchor->anchor, *pose);

        float rawPose[7];
        pose.getPoseRaw(rawPose);
        if (glm::distance(glm::vec3(rawPose[4], rawPose[5], rawPose[6]), pos) >
            MAX_RELATIVE_ANCHOR_DISTANCE) {
          continue;
        }

        if (uiAnchor->plane == plane) {
          ArTrackableList_destroy(trackableList);
          return uiAnchor;
        }
      }
    }
  }

  ArTrackableList_destroy(trackableList);
  return nullptr;
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

    ArConfig_setCloudAnchorMode(mArSession, arConfig, AR_CLOUD_ANCHOR_MODE_ENABLED);
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

  mPlaneRenderer->resume(mArSession);
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

ArSession *Engine::getArSession() const {
  return mArSession;
}

ArFrame *Engine::getArFrame() const {
  return mArFrame;
}

ServerThread *Engine::getServerThread() const {
  return mServerThread;
}

ArCamera *Engine::getArCamera() const {
  return mArCamera;
}

JNICallbacks *Engine::getCallbacks() const {
  return mCallbacks;
}
