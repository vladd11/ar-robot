#ifndef AR_SHOP_BINDINGS_H
#define AR_SHOP_BINDINGS_H

#include "engine.h"

extern "C" {
#include "lua.h"
}

inline int tick(lua_State *L) {
  lua_getglobal(L, "tick");
  return lua_pcall(L, 0, 0, 0);
}

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

void registerLibraryPath(lua_State *L, std::string path);

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
