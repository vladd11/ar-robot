#ifndef AR_SHOP_LUALINES_H
#define AR_SHOP_LUALINES_H

#include "luastructs.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

void openLines(lua_State *L);

void initGL();

int newLine(lua_State *L);

int push(lua_State *L);

int drawLine(lua_State *L);

#endif
