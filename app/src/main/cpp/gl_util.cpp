#include "gl_util.h"

#define TAG "Engine"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

void checkGlError(const char *operation) {
  bool anyError = false;
  for (GLuint error = glGetError(); error; error = glGetError()) {
    LOGE("after %s() glError (0x%x)\n", operation, error);
    anyError = true;
  }
  if (anyError) {
    abort();
  }
}

bool loadPngToGl(JNIEnv *env) {
  jclass cls = env->FindClass("com/vladd11/arshop/NativeEngine");
  env->CallStaticVoidMethod(cls, env->GetStaticMethodID(cls, "loadImage", "()V"));
  return true;
}

glm::vec3 getPlaneNormal(const ArSession *ar_session,
                         const ArPose &plane_pose) {
  float plane_pose_raw[7] = {0.f};
  ArPose_getPoseRaw(ar_session, &plane_pose, plane_pose_raw);
  glm::quat plane_quaternion(plane_pose_raw[3], plane_pose_raw[0],
                             plane_pose_raw[1], plane_pose_raw[2]);
  // Get normal vector, normal is defined to be positive Y-position in local
  // frame.
  return glm::rotate(plane_quaternion, glm::vec3(0., 1.f, 0.));
}

float calculateDistanceToPlane(const ArSession *arSession,
                               const ArPose &planePose,
                               const ArPose &cameraPose) {
  float rawPlanePose[7];
  ArPose_getPoseRaw(arSession, &planePose, rawPlanePose);
  glm::vec3 plane_position(rawPlanePose[4], rawPlanePose[5],
                           rawPlanePose[6]);
  glm::vec3 normal = getPlaneNormal(arSession, planePose);

  float rawCameraPose[7];
  ArPose_getPoseRaw(arSession, &cameraPose, rawCameraPose);
  glm::vec3 camera_P_plane(rawCameraPose[4] - plane_position.x,
                           rawCameraPose[5] - plane_position.y,
                           rawCameraPose[6] - plane_position.z);
  return glm::dot(normal, camera_P_plane);
}
