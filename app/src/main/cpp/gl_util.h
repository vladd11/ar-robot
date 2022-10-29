//
// Created by vladislav on 8/22/22.
//

#ifndef AR_SHOP_GL_UTIL_H
#define AR_SHOP_GL_UTIL_H

#include <android/log.h>
#include <android/bitmap.h>
#include <GLES2/gl2.h>
#include <sstream>
#include <jni.h>
#include "glm.h"
#include "arcore_c_api.h"

void checkGlError(const char *operation);

glm::vec3 getPlaneNormal(const ArSession *ar_session,
                         const ArPose &plane_pose);

float calculateDistanceToPlane(const ArSession *arSession,
                               const ArPose &planePose,
                               const ArPose &cameraPose);

bool loadPngToGl(JNIEnv *env);

#endif //AR_SHOP_GL_UTIL_H
