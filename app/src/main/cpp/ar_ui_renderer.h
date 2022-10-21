//
// Created by vladislav on 8/23/22.
//

#ifndef AR_SHOP_AR_UI_RENDERER_H
#define AR_SHOP_AR_UI_RENDERER_H

#include <GLES2/gl2.h>
#include "gl_util.h"
#include "shaders/default.h"
#include "verts/triangle.h"
#include "glm.h"

#define COORDS_PER_VERTEX 3
#define VERTEX_STRIDE COORDS_PER_VERTEX * sizeof(GLfloat)

class ArUiRenderer {
private:
  GLuint mDefaultProgram{}, mElementBuffer{};

public:
  ArUiRenderer();
  void init();
  void draw(glm::mat<4, 4, glm::f32> mvp) const;
};

#endif //AR_SHOP_AR_UI_RENDERER_H
