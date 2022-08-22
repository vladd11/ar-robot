//
// Created by vladislav on 8/22/22.
//

#ifndef AR_SHOP_BACKGROUND_RENDERER_H
#define AR_SHOP_BACKGROUND_RENDERER_H

#include "android/log.h"
#include "GLES2/gl2.h"
#include "shaders/default.h"

#define COORDS_PER_VERTEX 3
#define VERTEX_STRIDE COORDS_PER_VERTEX * sizeof(GLfloat)

class BackgroundRenderer {
private:
  GLuint mDefaultProgram{};

public:
  BackgroundRenderer();
  void init();
  void drawFrame();
};


#endif //AR_SHOP_BACKGROUND_RENDERER_H
