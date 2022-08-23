//
// Created by vladislav on 8/22/22.
//

#ifndef AR_SHOP_BACKGROUND_RENDERER_H
#define AR_SHOP_BACKGROUND_RENDERER_H

#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "verts/plane.h"
#include "shaders/background.h"
#include "glError.h"

#define COORDS_PER_VERTEX 3
#define VERTEX_STRIDE COORDS_PER_VERTEX * sizeof(GLfloat)

class BackgroundRenderer {
private:
  GLuint mDefaultProgram{}, mCameraProgram{};
  GLint mCameraTextureUniform{}, mCameraPositionAttrib{}, mCameraTexCoordAttrib{};

public:
  GLuint mCameraTextureId{};

  BackgroundRenderer();

  void init();

  void draw(float *mTransformedUVs);
};


#endif //AR_SHOP_BACKGROUND_RENDERER_H
