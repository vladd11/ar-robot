#ifndef AR_SHOP_BINDINGS_H
#define AR_SHOP_BINDINGS_H

#include "engine.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

/**
 * Call when Cloud Anchor state was changed
 * @param idx ID in Engine::Anchors list
 * @param state State of anchor (ArCloudAnchorState enum)
 * @param anchorId Cloud Anchor ID (from ARCore API)
 */
inline int onAnchorUpdate(lua_State *L, int idx, int state, char *anchorId) {
  lua_getglobal(L, "onAnchorUpdate");
  lua_pushnumber(L, idx);
  lua_pushnumber(L, state);
  lua_pushstring(L, anchorId);
  return lua_pcall(L, 3, 0, 0);
}

inline void printLuaError(lua_State *L) {
  __android_log_print(ANDROID_LOG_ERROR, "Lua", "%s", lua_tostring(L, -1));
}

inline void tick(lua_State *L) {
  lua_getglobal(L, "tick");
  if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
    printLuaError(L);
  }
}

/**
 * Takes 3 arguments: RGB colors in range [0.0, 1.0].
 */
int setColor(lua_State *L);

int getAnchorsCount(lua_State *L);

int swapAnchors(lua_State *L);

int angleToAnchor(lua_State *L);

int anchorPose(lua_State *L);

int saveAnchor(lua_State *L);

int loadCode(lua_State *L, const std::string &code, std::string **outError);

/**
 * Get Euler rotation of AR camera (in radians).
 * @return nil if camera isn't tracking, else XYZ euler rotation in rads.
 */
int cameraPose(lua_State *L);

/**
 * Broadcasts message to all connected clients.
 */
int send(lua_State *L);

int log(lua_State *L);

int setText(lua_State *L);

lua_State *createLuaState(std::string path);

template<class T>
T getStruct(lua_State *L, void *key) {
  lua_pushlightuserdata(L, key);
  lua_gettable(L, LUA_REGISTRYINDEX);
  return reinterpret_cast<T>(lua_touserdata(L, -1));
}

template<class T>
void pushStruct(lua_State *L, T struct_, void *key) {
  lua_pushlightuserdata(L, key);
  lua_pushlightuserdata(L, struct_);
  lua_settable(L, LUA_REGISTRYINDEX);
}

#endif //AR_SHOP_BINDINGS_H
