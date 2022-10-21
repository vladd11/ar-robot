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

static JavaVM *jvm;

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  jvm = vm;
  return JNI_VERSION_1_6;
}

bool loadPngToGl(JNIEnv *env) {
  jvm->AttachCurrentThread(&env, nullptr);
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

float calculateDistanceToPlane(const ArSession *ar_session,
                               const ArPose &plane_pose,
                               const ArPose &camera_pose) {
  float plane_pose_raw[7] = {0.f};
  ArPose_getPoseRaw(ar_session, &plane_pose, plane_pose_raw);
  glm::vec3 plane_position(plane_pose_raw[4], plane_pose_raw[5],
                           plane_pose_raw[6]);
  glm::vec3 normal = getPlaneNormal(ar_session, plane_pose);

  float camera_pose_raw[7] = {0.f};
  ArPose_getPoseRaw(ar_session, &camera_pose, camera_pose_raw);
  glm::vec3 camera_P_plane(camera_pose_raw[4] - plane_position.x,
                           camera_pose_raw[5] - plane_position.y,
                           camera_pose_raw[6] - plane_position.z);
  return glm::dot(normal, camera_P_plane);
}
