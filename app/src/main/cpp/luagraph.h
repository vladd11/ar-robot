#ifndef AR_SHOP_LUAGRAPH_H
#define AR_SHOP_LUAGRAPH_H

extern "C" {
#include "lua.h"
}

int createGraph(lua_State *L);

int deleteGraph(lua_State *L);

int setAdjWeight(lua_State *L);

int buildRoute(lua_State *L);

int connect(lua_State *L);

int getAdj(lua_State *L);

int getAdjCount(lua_State *L);

#endif //AR_SHOP_LUAGRAPH_H
