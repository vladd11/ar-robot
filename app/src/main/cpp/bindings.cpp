#include "bindings.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "engine.h"

#define TAG "ArUiRenderer"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

int angleToAnchor(lua_State *L) {
  lua_pushlightuserdata(L, nullptr);
  lua_gettable(L, LUA_REGISTRYINDEX);
  auto *self = reinterpret_cast<Engine *>(lua_touserdata(L, -1));

  long long index = luaL_checkinteger(L, 1);
  if(index >= self->Anchors().size()) {
    lua_pushnil(L);
    return 1;
  }

  ArAnchor *anchor = self->Anchors()[index]->anchor;

  ArPose *pose;
  ArPose_create(self->ArSession(), nullptr, &pose);
  ArAnchor_getPose(self->ArSession(), anchor, pose);

  float raw[7];
  ArPose_getPoseRaw(self->ArSession(), pose, raw);
  glm::vec3 rot = glm::eulerAngles(glm::quat(raw[0], raw[1], raw[2], raw[3]));

  ArPose_destroy(pose);

  lua_pushnumber(L, rot.x);
  lua_pushnumber(L, rot.y);
  lua_pushnumber(L, rot.z);
  return 3;
}

