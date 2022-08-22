//
// Created by vladislav on 8/21/22.
//

#ifndef AR_SHOP_RENDERER_H
#define AR_SHOP_RENDERER_H

#include <EGL/egl.h>
#include <android/log.h>
#include <cstdlib>
#include <cassert>
#include <memory>
#include <android_native_app_glue.h>
#include <GLES2/gl2.h>

#define TAG "Activity"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)

class Renderer {
private:
  EGLint width{}, height{};
  EGLDisplay display{};
  EGLSurface surface{};
  EGLContext context{};

public:
  struct android_app *mApp{};
  Renderer();
  ~Renderer();

  void init_display(struct android_app *app);
  void draw_frame();
};


#endif //AR_SHOP_RENDERER_H
