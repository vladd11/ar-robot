#include "bindings.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "ConstantFunctionResult" // Lua functions may return const value of results

extern "C" {
#include "lualib.h"
#include "luasocket.h"
}

#include <cmath>

#define TAG "Lua"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#define LUA_ERROR(L, ...) {luaL_error(L, __VA_ARGS__); return 0;}

static const struct luaL_Reg bindings[] = {
    {"setScreenText",   setScreenText},
    {"anchorPose",      anchorPose},
    {"setColor",        setColor},
    {"getAnchorsCount", getAnchorsCount},
    {"swapAnchors",     swapAnchors},
    {"angleToAnchor",   angleToAnchor},
    {"saveAnchor",      saveAnchor},
    {"cameraPose",      cameraPose},
    {"send",            send},
    {"requireSockets",  luaopen_socket_core},
    {"print",           log},
};

lua_State *createLuaState(std::string path) {
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  lua_getglobal(L, "package");
  lua_getfield(L, -1, "path"); // get field "path" from table at top of stack (-1)
  std::string cur_path = lua_tostring(L, -1); // grab path string from top of stack
  path.append("/?.lua;");
  path.append(cur_path); // do your path magic here
  lua_pop(L, 1); // get rid of the string on the stack we just pushed on line 5
  lua_pushstring(L, path.c_str()); // push the new one
  lua_setfield(L, -2, "path"); // set the field "path" in table at -2 with value at top of stack
  lua_pop(L, 1);

  for (luaL_Reg binding: bindings) {
    lua_register(L, binding.name, binding.func);
  }

  return L;
}

/**
 * Pushes 7 values of raw pose from ArPose.
 * Order: (Rotation)WXYZ(Position)XYZ
 */
void lua_pushPose(lua_State *L, ArSession *session, ArPose *pose) {
  float raw[7];
  ArPose_getPoseRaw(session, pose, raw);

  lua_pushnumber(L, raw[3]);
  lua_pushnumber(L, raw[0]);
  lua_pushnumber(L, raw[1]);
  lua_pushnumber(L, raw[2]);

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

inline long long checkAnchorIndex(lua_State *L, Engine *engine, int arg) {
  long long index = luaL_checkinteger(L, arg);
  if (index >= engine->mAnchors.size() || index < 0) {
    luaL_error(L, "Array index is out of range");
    return -1;
  }
  return index;
}

inline long long checkAnchorIndex(lua_State *L, Engine *engine) {
  return checkAnchorIndex(L, engine, 1);
}

int anchorPose(lua_State *L) {
  auto *self = getStruct<Engine *>(L, ENGINE_KEY);

  long long index = checkAnchorIndex(L, self);
  if (index == -1) {
    return 0;
  }

  ArAnchor *anchor = self->mAnchors[index]->anchor;

  ArTrackingState state;
  ArAnchor_getTrackingState(self->getArSession(), anchor, &state);
  if (state == AR_TRACKING_STATE_TRACKING) {
    ArPose *pose;
    ArPose_create(self->getArSession(), nullptr, &pose);
    ArAnchor_getPose(self->getArSession(), anchor, pose);

    lua_pushPose(L, self->getArSession(), pose);

    ArPose_destroy(pose);
  } else {
    for (int i = 0; i < 7; i++) lua_pushnil(L);
  }

  return 7;
}

int cameraPose(lua_State *L) {
  auto *self = getStruct<Engine *>(L, ENGINE_KEY);

  ArCamera *camera;
  ArFrame_acquireCamera(self->getArSession(), self->getArFrame(), &camera);

  ArTrackingState state;
  ArCamera_getTrackingState(self->getArSession(), camera, &state);

  if (state != AR_TRACKING_STATE_TRACKING) {
    for (int i = 0; i < 6; i++) lua_pushnil(L);
  } else {
    ArPose *pose;
    ArPose_create(self->getArSession(), nullptr, &pose);
    ArCamera_getPose(self->getArSession(), camera, pose);

    lua_pushPose(L, self->getArSession(), pose);

    ArPose_destroy(pose);
  }

  return 7;
}

int saveAnchor(lua_State *L) {
  auto *self = getStruct<Engine *>(L, ENGINE_KEY);

  long long index = checkAnchorIndex(L, self);
  if (index == -1) LUA_ERROR(L, "Array index is out of range")

  UiAnchor *uiAnchor = self->mAnchors[index];

  ArTrackingState state;
  ArCamera_getTrackingState(self->getArSession(), self->getArCamera(), &state);

  if (state == AR_TRACKING_STATE_TRACKING) {
    ArAnchor *cloudAnchor;
    if (uiAnchor->cloudAnchor != nullptr) {
      ArAnchor_release(uiAnchor->cloudAnchor);
    }

    ArPose *pose;
    ArPose_create(self->getArSession(), nullptr, &pose);
    ArCamera_getPose(self->getArSession(), self->getArCamera(), pose);

    ArFeatureMapQuality quality;
    ArSession_estimateFeatureMapQualityForHosting(self->getArSession(), pose, &quality);

    ArPose_destroy(pose);

    if (quality == AR_FEATURE_MAP_QUALITY_GOOD) {
      if (ArSession_hostAndAcquireNewCloudAnchorWithTtl(self->getArSession(), uiAnchor->anchor, 365,
                                                        &cloudAnchor) == AR_SUCCESS) {
        uiAnchor->cloudAnchor = cloudAnchor;
        lua_pushboolean(L, true);
      } else LUA_ERROR(L, "NO_ANCHOR")
    } else LUA_ERROR(L, "QUALITY_INSUFFICIENT")
  } else LUA_ERROR(L, "CAMERA_NOT_TRACKING")

  return 1;
}

int send(lua_State *L) {
  size_t len;
  const char *str = luaL_checklstring(L, 1, &len);

  auto *self = getStruct<Engine *>(L, ENGINE_KEY);
  self->getServerThread()->out.enqueue(new std::string(str, len));

  return 0;
}

int angleToAnchor(lua_State *L) {
  auto *self = getStruct<Engine *>(L, ENGINE_KEY);

  long long index = checkAnchorIndex(L, self);
  if (index == -1) {
    return 0;
  }

  ArTrackingState state;
  ArCamera_getTrackingState(self->getArSession(), self->getArCamera(), &state);

  if (state == AR_TRACKING_STATE_TRACKING) {
    ArAnchor *anchor = self->mAnchors[index]->anchor;

    ArAnchor_getTrackingState(self->getArSession(), anchor, &state);
    if (state == AR_TRACKING_STATE_TRACKING) {
      ArPose *anchorPose, *cameraPose;
      ArPose_create(self->getArSession(), nullptr, &anchorPose);
      ArPose_create(self->getArSession(), nullptr, &cameraPose);

      ArAnchor_getPose(self->getArSession(), anchor, anchorPose);
      ArCamera_getPose(self->getArSession(), self->getArCamera(), cameraPose);

      float anchorPoseRaw[7], cameraPoseRaw[7];
      ArPose_getPoseRaw(self->getArSession(), anchorPose, anchorPoseRaw);
      ArPose_getPoseRaw(self->getArSession(), cameraPose, cameraPoseRaw);

      glm::vec2 anchorVector = glm::vec2(anchorPoseRaw[4], anchorPoseRaw[6]);
      // This is projection of angle (between anchor coordinates and (0, 0, -1) camera forward vector)
      // to XZ plane
      // Calculated by [dot product](https://en.wikipedia.org/wiki/Dot_product)
      float anchorAngle = acos(dot(anchorVector, glm::vec2(0, -1)) / glm::length(anchorVector));
      if (anchorVector.x < 0) anchorAngle = -anchorAngle;

      // This is YAW (eq. projection of angle between current rotation and forward vector).
      float cameraAngle = glm::pitch(
          glm::quat(cameraPoseRaw[3], cameraPoseRaw[0], cameraPoseRaw[1], cameraPoseRaw[2]));

      lua_pushnumber(L, std::fmod(anchorAngle - cameraAngle, 2 * M_PI));

      ArPose_destroy(anchorPose);
      ArPose_destroy(cameraPose);
    } else LUA_ERROR(L, "ANCHOR_NOT_TRACkING")
  } else LUA_ERROR(L, "CAMERA_NOT_TRACKING")

  return 1;
}

int getAnchorsCount(lua_State *L) {
  auto *self = getStruct<Engine *>(L, ENGINE_KEY);

  lua_pushinteger(L, (long long) self->mAnchors.size());
  return 1;
}

int swapAnchors(lua_State *L) {
  auto *self = getStruct<Engine *>(L, ENGINE_KEY);

  long long idx = checkAnchorIndex(L, self, 1);
  long long idx1 = checkAnchorIndex(L, self, 2);

  std::swap(self->mAnchors[idx], self->mAnchors[idx1]);

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

int setColor(lua_State *L) {
  lua_Integer id = luaL_checkinteger(L, 1);
  auto R = (float) luaL_checknumber(L, 2);
  auto G = (float) luaL_checknumber(L, 3);
  auto B = (float) luaL_checknumber(L, 4);


  if (R > 1.0 || G > 1.0 || B > 1.0 || R < 0.0 || G < 0.0 || B < 0.0) {
    luaL_error(L, "Invalid color value");
  }

  auto *self = getStruct<Engine *>(L, ENGINE_KEY);
  self->mAnchors[id]->colors = new float[4]{R, G, B, 1.0f};

  return 0;
}


#pragma clang diagnostic pop