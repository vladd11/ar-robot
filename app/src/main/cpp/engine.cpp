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
  for (auto &uiAnchor: mAnchors) {
    ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
    ArAnchor_getTrackingState(mArSession, uiAnchor->anchor,
                              &tracking_state);
    if (tracking_state == AR_TRACKING_STATE_TRACKING) {
      GetTransformMatrixFromAnchor(*uiAnchor->anchor, &model_mat);

      mArUiRenderer->draw(projection_mat * view_mat * model_mat);
    }
  }
}

glm::vec3 GetPlaneNormal(const ArSession &ar_session,
                         const ArPose &plane_pose) {
  float plane_pose_raw[7] = {0.f};
  ArPose_getPoseRaw(&ar_session, &plane_pose, plane_pose_raw);
  glm::quat plane_quaternion(plane_pose_raw[3], plane_pose_raw[0],
                             plane_pose_raw[1], plane_pose_raw[2]);
  // Get normal vector, normal is defined to be positive Y-position in local
  // frame.
  return glm::rotate(plane_quaternion, glm::vec3(0., 1.f, 0.));
}

float CalculateDistanceToPlane(const ArSession &ar_session,
                               const ArPose &plane_pose,
                               const ArPose &camera_pose) {
  float plane_pose_raw[7] = {0.f};
  ArPose_getPoseRaw(&ar_session, &plane_pose, plane_pose_raw);
  glm::vec3 plane_position(plane_pose_raw[4], plane_pose_raw[5],
                           plane_pose_raw[6]);
  glm::vec3 normal = GetPlaneNormal(ar_session, plane_pose);

  float camera_pose_raw[7] = {0.f};
  ArPose_getPoseRaw(&ar_session, &camera_pose, camera_pose_raw);
  glm::vec3 camera_P_plane(camera_pose_raw[4] - plane_position.x,
                           camera_pose_raw[5] - plane_position.y,
                           camera_pose_raw[6] - plane_position.z);
  return glm::dot(normal, camera_P_plane);
}

void Engine::onTouch(float x, float y) {
  if (mArFrame != nullptr && mArSession != nullptr) {
    ArHitResultList *hit_result_list = nullptr;
    ArHitResultList_create(mArSession, &hit_result_list);
    CHECK(hit_result_list);
    ArFrame_hitTest(mArSession, mArFrame, x, y, hit_result_list);

    int32_t hit_result_list_size = 0;
    ArHitResultList_getSize(mArSession, hit_result_list,
                            &hit_result_list_size);

    ArHitResult *ar_hit_result = nullptr;
    LOGD("Hit Result List size is %i", hit_result_list_size);
    for (int32_t i = 0; i < hit_result_list_size; ++i) {
      ArHitResult *ar_hit = nullptr;
      ArHitResult_create(mArSession, &ar_hit);
      ArHitResultList_getItem(mArSession, hit_result_list, i, ar_hit);

      if (ar_hit == nullptr) {
        LOGE("HelloArApplication::OnTouch ArHitResultList_getItem error");
        return;
      }

      ArTrackable *ar_trackable = nullptr;
      ArHitResult_acquireTrackable(mArSession, ar_hit, &ar_trackable);
      ArTrackableType ar_trackable_type = AR_TRACKABLE_NOT_VALID;
      ArTrackable_getType(mArSession, ar_trackable, &ar_trackable_type);
      // Creates an anchor if a plane or an oriented point was hit.
      if (AR_TRACKABLE_PLANE == ar_trackable_type) {
        ArPose *hit_pose = nullptr;
        ArPose_create(mArSession, nullptr, &hit_pose);
        ArHitResult_getHitPose(mArSession, ar_hit, hit_pose);
        int32_t in_polygon = 0;
        ArPlane *ar_plane = ArAsPlane(ar_trackable);
        ArPlane_isPoseInPolygon(mArSession, ar_plane, hit_pose, &in_polygon);

        // Use hit pose and camera pose to check if hittest is from the
        // back of the plane, if it is, no need to create the anchor.
        ArPose *camera_pose = nullptr;
        ArPose_create(mArSession, nullptr, &camera_pose);
        ArCamera *ar_camera;
        ArFrame_acquireCamera(mArSession, mArFrame, &ar_camera);
        ArCamera_getPose(mArSession, ar_camera, camera_pose);
        ArCamera_release(ar_camera);
        float normal_distance_to_plane = CalculateDistanceToPlane(
            *mArSession, *hit_pose, *camera_pose);

        ArPose_destroy(hit_pose);
        ArPose_destroy(camera_pose);

        if (!in_polygon || normal_distance_to_plane < 0) {
          continue;
        }

        ar_hit_result = ar_hit;
        break;
      } else if (AR_TRACKABLE_POINT == ar_trackable_type) {
        ArPoint *ar_point = ArAsPoint(ar_trackable);
        ArPointOrientationMode mode;
        ArPoint_getOrientationMode(mArSession, ar_point, &mode);
        if (AR_POINT_ORIENTATION_ESTIMATED_SURFACE_NORMAL == mode) {
          ar_hit_result = ar_hit;
          break;
        }
      }
    }

    if (ar_hit_result) {
      // Note that the application is responsible for releasing the anchor
      // pointer after using it. Call ArAnchor_release(anchor) to release.
      ArAnchor *anchor = nullptr;
      if (ArHitResult_acquireNewAnchor(mArSession, ar_hit_result, &anchor) !=
          AR_SUCCESS) {
        LOGE(
            "HelloArApplication::OnTouch ArHitResult_acquireNewAnchor error");
        return;
      }

      ArTrackingState tracking_state = AR_TRACKING_STATE_STOPPED;
      ArAnchor_getTrackingState(mArSession, anchor, &tracking_state);
      if (tracking_state != AR_TRACKING_STATE_TRACKING) {
        ArAnchor_release(anchor);
        return;
      }

      ArTrackable *arTrackable = nullptr;
      ArHitResult_acquireTrackable(mArSession, ar_hit_result, &arTrackable);
      // Assign a color to the object for rendering based on the trackable type
      // this anchor attached to. For AR_TRACKABLE_POINT, it's blue color, and
      // for AR_TRACKABLE_PLANE, it's green color.
      auto *uiAnchor = new UiAnchor();
      uiAnchor->anchor = anchor;
      uiAnchor->trackable = arTrackable;
      mAnchors.push_back(uiAnchor);

      ArHitResult_destroy(ar_hit_result);

      ArHitResultList_destroy(hit_result_list);
      hit_result_list = nullptr;
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

void Engine::resume(JNIEnv *env, jobject context, jobject activity) {
  if (mArSession == nullptr) {
    CHECKANDTHROW(ArSession_create(env, context, &mArSession) == AR_SUCCESS, env,
                  "Failed to create AR session")

    ArConfig *ar_config = nullptr;
    ArConfig_create(mArSession, &ar_config);

    ArConfig_setDepthMode(mArSession, ar_config, AR_DEPTH_MODE_DISABLED);

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
