#include "bindings.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include <cmath>

#define TAG "Lua"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)


glm::vec3 getPoseRotation(ArSession *session, ArPose *pose) {
  float raw[7];
  ArPose_getPoseRaw(session, pose, raw);
  return glm::eulerAngles(glm::quat(raw[0], raw[1], raw[2], raw[3]));
}

int angleToAnchor(lua_State *L) {
  auto *self = getStruct<Engine *>(L, ENGINE_KEY);

  long long index = luaL_checkinteger(L, 1);
  if (index >= self->Anchors().size()) {
    lua_pushnil(L);
    return 1;
  }

  ArAnchor *anchor = self->Anchors()[index]->anchor;

  ArTrackingState state;
  ArAnchor_getTrackingState(self->ArSession(), anchor, &state);
  if (state == AR_TRACKING_STATE_TRACKING) {
    ArPose *pose;
    ArPose_create(self->ArSession(), nullptr, &pose);
    ArAnchor_getPose(self->ArSession(), anchor, pose);

    glm::vec3 rot = getPoseRotation(self->ArSession(), pose);

    ArPose_destroy(pose);

    lua_pushnumber(L, rot.x);
    lua_pushnumber(L, rot.y);
    lua_pushnumber(L, rot.z);
    return 3;
  }
  lua_pushnil(L);
  return 1;
}

int cameraAngle(lua_State *L) {
  auto *self = getStruct<Engine *>(L, ENGINE_KEY);

  ArCamera *camera;
  ArFrame_acquireCamera(self->ArSession(), self->ArFrame(), &camera);

  ArTrackingState state;
  ArCamera_getTrackingState(self->ArSession(), camera, &state);

  int code = 1;
  if (state != AR_TRACKING_STATE_TRACKING) {
    lua_pushnil(L);
  } else {
    ArPose *pose;
    ArPose_create(self->ArSession(), nullptr, &pose);
    ArCamera_getPose(self->ArSession(), camera, pose);

    glm::vec3 rot = getPoseRotation(self->ArSession(), pose);

    lua_pushnumber(L, rot.x);
    lua_pushnumber(L, rot.y);
    lua_pushnumber(L, rot.z);

    ArPose_destroy(pose);
    code = 3;
  }

  ArCamera_release(camera);
  return code;
}

int send(lua_State *L) {
  size_t len;
  const char *str = luaL_checklstring(L, 1, &len);

  auto *self = getStruct<Engine *>(L, ENGINE_KEY);
  self->ServerThread()->out.enqueue(new Message{str, len});

  return 0;
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

