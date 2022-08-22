//
// Created by vladislav on 8/22/22.
//

#ifndef AR_SHOP_ENGINE_H
#define AR_SHOP_ENGINE_H

#include <GLES2/gl2.h>
#include "background_renderer.h"

class Engine {
private:
  BackgroundRenderer* mBackgroundRenderer;
public:
  Engine();
  void init();
  void drawFrame();
};


#endif //AR_SHOP_ENGINE_H
