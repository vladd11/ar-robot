//
// Created by vladislav on 8/23/22.
//

#ifndef AR_SHOP_TRIANGLE_H
#define AR_SHOP_TRIANGLE_H

#include <GLES2/gl2.h>

namespace Triangle {
  const GLfloat kVertices[] = {
      0.0f, 0.622008459f, 0.0f,      // top
      -0.5f, -0.311004243f, 0.0f,    // bottom left
      0.5f, -0.311004243f, 0.0f
  };
  const GLfloat kColors[] = {
      0.63671875f, 0.76953125f, 0.22265625f, 1.0f
  };
  const GLuint kNumVertices = 3;
}

#endif //AR_SHOP_TRIANGLE_H
