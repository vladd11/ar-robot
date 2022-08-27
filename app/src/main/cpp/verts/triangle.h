//
// Created by vladislav on 8/23/22.
//

#ifndef AR_SHOP_TRIANGLE_H
#define AR_SHOP_TRIANGLE_H

#include <GLES2/gl2.h>

namespace Triangle {
  const GLfloat kVertices[] = {
      -1.0f, -1.0f, 0.0f,
      +1.0f, -1.0f, 0.0f,
      -1.0f, +1.0f, 0.0f,
      +1.0f, +1.0f, 0.0f
  };
  const GLfloat kColors[] = {
      0.63671875f, 0.76953125f, 0.22265625f, 1.0f
  };
  const GLuint kNumVertices = 4;
}

#endif //AR_SHOP_TRIANGLE_H
