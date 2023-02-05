#ifndef AR_SHOP_LUASTRUCTS_H
#define AR_SHOP_LUASTRUCTS_H

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

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
#endif //AR_SHOP_LUASTRUCTS_H
