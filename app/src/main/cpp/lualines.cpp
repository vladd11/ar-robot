#include "lualines.h"

#include "engine.h"
#include "android/log.h"

void openLines(lua_State *L) {
  lua_register(L, "newLine", newLine);
  lua_register(L, "pushLine", push);
  lua_register(L, "drawLine", drawLine);
}

#define TAG "LuaLines"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static GLfloat maxLineWidth = 1.0f;

void initGL() {
  GLfloat lineWidthRange[2] = {0.0f, 0.0f};
  glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, lineWidthRange);
  LOGD("Maximum line width for this GPU: %f", lineWidthRange[1]);
  maxLineWidth = lineWidthRange[1] - 1.0f;
}

int newLine(lua_State *L) {
  auto *self = getStruct<Engine *>(L, ENGINE_KEY);

  // Max numbers of edges in graph is n^2
  // indices[0] is
  auto size = (size_t) (sizeof(GLuint) * (pow(self->mAnchors.size(), 2) + 1));
  auto *indices = (GLuint *) lua_newuserdata(L, size);
  memset(indices, 0, size);
  return 1;
}

int push(lua_State *L) {
  auto *indices = static_cast<GLuint *>(lua_touserdata(L, 1));
  indices[0]++;

  indices[indices[0]] = (GLuint) luaL_checkinteger(L, 2);

  return 0;
}

const GLfloat color[] = {1.0, 1.0, 1.0, 1.0f};

int drawLine(lua_State *L) {
  auto *self = getStruct<Engine *>(L, ENGINE_KEY);

  auto *indices = static_cast<GLuint *>(lua_touserdata(L, 1));
  indices = &indices[1];

  glUseProgram(RawShader::get());
  glLineWidth(maxLineWidth);

  GLint vColorLocation = glGetUniformLocation(RawShader::get(), "vColor");
  glUniform4fv(vColorLocation, 1, color);

  float points[indices[-1] * 3];
  int j = 0;
  for (int i = 0; i < indices[-1]; i++) {
    points[j] = self->mPositions[indices[i] * 3];
    j++;
    points[j] = self->mPositions[indices[i] * 3 + 1];
    j++;
    points[j] = self->mPositions[indices[i] * 3 + 2];
    j++;
  }

  glVertexAttribPointer(DefaultShader::vPositionAttrIndex, 3, GL_FLOAT, false,
                        COORDS_PER_VERTEX * sizeof(GLfloat),
                        points);
  glEnableVertexAttribArray(DefaultShader::vPositionAttrIndex);

  glDrawArrays(GL_LINES, 0, (int) indices[-1]);

  return 0;
}

