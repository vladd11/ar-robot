#include "bindings.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "ConstantFunctionResult" // Lua functions may return const value of results

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include <cmath>

#define TAG "Lua"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

void registerLibraryPath(lua_State *L, std::string path) {
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "path"); // get field "path" from table at top of stack (-1)
  std::string cur_path = lua_tostring(L, -1); // grab path string from top of stack
  path.append("/?.lua;");
  path.append(cur_path); // do your path magic here
  lua_pop(L, 1); // get rid of the string on the stack we just pushed on line 5
  lua_pushstring(L, path.c_str()); // push the new one
  lua_setfield(L, -2, "path"); // set the field "path" in table at -2 with value at top of stack
  lua_pop(L, 1);
}

/**
 * Pushes 7 values of raw pose from ArPose.
 * Order: (Rotation)XYZ(Position)XYZ
 */
void lua_pushPose(lua_State *L, ArSession *session, ArPose *pose) {
  float raw[7];
  ArPose_getPoseRaw(session, pose, raw);

  glm::vec3 vec = glm::eulerAngles(glm::quat(raw[3], raw[0], raw[1], raw[2]));
  lua_pushnumber(L, vec.x);
  lua_pushnumber(L, vec.y);
  lua_pushnumber(L, vec.z);

  lua_pushnumber(L, raw[4]);
  lua_pushnumber(L, raw[5]);
  lua_pushnumber(L, raw[6]);
}

int loadCode(lua_State *L, const std::string &code, std::string **outError) {
  LOGD("Loaded new code from peer");

  int error = luaL_loadbufferx(L, code.c_str(), code.length(), "dynamic.lua", nullptr) |
              lua_pcall(L, 0, LUA_MULTRET, 0);
  if (error != LUA_OK) {
    size_t size = 0;
    const char *str = lua_tolstring(L, -1, &size);

    (*outError)->assign(str, size);
    LOGD("%s", (*outError)->c_str());
  }
  return error;
}

inline long long checkAnchorIndex(lua_State *L, Engine *engine) {
  long long index = luaL_checkinteger(L, 1);
  if (index >= engine->Anchors().size() || index < 0) {
    lua_pushnil(L);
    return -1;
  }
  return index;
}

int anchorPose(lua_State *L) {
  auto *self = getStruct<Engine *>(L, ENGINE_KEY);

  long long index = checkAnchorIndex(L, self);
  if (index == -1) {
    return 1;
  }

  ArAnchor *anchor = self->Anchors()[index]->anchor;

  ArTrackingState state;
  ArAnchor_getTrackingState(self->ArSession(), anchor, &state);
  if (state == AR_TRACKING_STATE_TRACKING) {
    ArPose *pose;
    ArPose_create(self->ArSession(), nullptr, &pose);
    ArAnchor_getPose(self->ArSession(), anchor, pose);

    lua_pushPose(L, self->ArSession(), pose);

    ArPose_destroy(pose);
  } else {
    for (int i = 0; i < 6; i++) lua_pushnil(L);
  }

  return 6;
}

int cameraPose(lua_State *L) {
  auto *self = getStruct<Engine *>(L, ENGINE_KEY);

  ArCamera *camera;
  ArFrame_acquireCamera(self->ArSession(), self->ArFrame(), &camera);

  ArTrackingState state;
  ArCamera_getTrackingState(self->ArSession(), camera, &state);

  if (state != AR_TRACKING_STATE_TRACKING) {
    for (int i = 0; i < 6; i++) lua_pushnil(L);
  } else {
    ArPose *pose;
    ArPose_create(self->ArSession(), nullptr, &pose);
    ArCamera_getPose(self->ArSession(), camera, pose);

    lua_pushPose(L, self->ArSession(), pose);

    ArPose_destroy(pose);
  }

  return 6;
}

int saveAnchor(lua_State *L) {
  auto *self = getStruct<Engine *>(L, ENGINE_KEY);

  long long index = checkAnchorIndex(L, self);
  if (index == -1) {
    return 1;
  }

  UiAnchor *uiAnchor = self->Anchors()[index];

  ArTrackingState state;
  ArCamera_getTrackingState(self->ArSession(), self->getArCamera(), &state);

  if (state == AR_TRACKING_STATE_TRACKING) {
    ArAnchor *cloudAnchor;
    if (ArSession_hostAndAcquireNewCloudAnchorWithTtl(self->ArSession(), uiAnchor->anchor, 365,
                                                      &cloudAnchor) == AR_SUCCESS) {
      lua_pushboolean(L, true);
    } else lua_pushnil(L);
  } else lua_pushnil(L);

  return 1;
}

int send(lua_State *L) {
  size_t len;
  const char *str = luaL_checklstring(L, 1, &len);

  auto *self = getStruct<Engine *>(L, ENGINE_KEY);
  self->ServerThread()->out.enqueue(new std::string(str, len));

  return 0;
}

int angleToAnchor(lua_State *L) {
  auto *self = getStruct<Engine *>(L, ENGINE_KEY);

  long long index = checkAnchorIndex(L, self);
  if (index == -1) {
    return 1;
  }

  ArTrackingState state;
  ArCamera_getTrackingState(self->ArSession(), self->getArCamera(), &state);

  if (state == AR_TRACKING_STATE_TRACKING) {
    ArAnchor *anchor = self->Anchors()[index]->anchor;

    ArAnchor_getTrackingState(self->ArSession(), anchor, &state);
    if (state == AR_TRACKING_STATE_TRACKING) {
      ArPose *anchorPose, *cameraPose;
      ArPose_create(self->ArSession(), nullptr, &anchorPose);
      ArPose_create(self->ArSession(), nullptr, &cameraPose);

      ArAnchor_getPose(self->ArSession(), anchor, anchorPose);
      ArCamera_getPose(self->ArSession(), self->getArCamera(), cameraPose);

      float anchorPoseRaw[7], cameraPoseRaw[7];
      ArPose_getPoseRaw(self->ArSession(), anchorPose, anchorPoseRaw);
      ArPose_getPoseRaw(self->ArSession(), cameraPose, cameraPoseRaw);

      glm::vec2 anchorVector = glm::vec2(anchorPoseRaw[4], anchorPoseRaw[6]);
      // This is projection of angle (between anchor coordinates and (0, 0, -1) camera forward vector)
      // to XZ plane
      // Calculated by [dot product](https://en.wikipedia.org/wiki/Dot_product)
      float anchorAngle = acos(dot(anchorVector, glm::vec2(0, -1)) / glm::length(anchorVector));
      if(anchorVector.x < 0) anchorAngle = -anchorAngle;

      // This is YAW (eq. projection of angle between current rotation and forward vector).
      float cameraAngle = glm::pitch(
          glm::quat(cameraPoseRaw[3], cameraPoseRaw[0], cameraPoseRaw[1], cameraPoseRaw[2]));

      lua_pushnumber(L, std::fmod(anchorAngle - cameraAngle, 2 * M_PI));

      ArPose_destroy(anchorPose);
      ArPose_destroy(cameraPose);
    } else lua_pushnil(L);
  } else lua_pushnil(L);

  return 1;
}

int log(lua_State *L) {
  int n = lua_gettop(L); // number of arguments
  for (int i = 1; i <= n; i++) { // for each argument
    size_t l;
    const char *s = luaL_tolstring(L, i, &l); // convert it to string
    LOGD("%s", std::string(s, l).c_str());
    lua_pop(L, 1); // pop result
  }
  return 0;
}


#pragma clang diagnostic pop