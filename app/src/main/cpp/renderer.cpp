#include "renderer.h"

Renderer::Renderer() = default;

Renderer::~Renderer() {
  free(this->display);
  free(this->surface);
  free(this->context);
}

void Renderer::init_display(struct android_app *app) {
  mApp = app;

  const EGLint attribList[] = {
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_BLUE_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_RED_SIZE, 8,
      EGL_NONE
  };
  EGLint format;
  EGLint numConfigs;
  EGLConfig config = nullptr;

  this->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  eglInitialize(display, nullptr, nullptr);

  /* Here, the application chooses the configuration it desires.
   * find the best match if possible, otherwise use the very first one
   */
  eglChooseConfig(display, attribList, nullptr, 0, &numConfigs);
  std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
  assert(supportedConfigs);
  eglChooseConfig(display, attribList, supportedConfigs.get(), numConfigs, &numConfigs);
  assert(numConfigs);
  auto i = 0;
  for (; i < numConfigs; i++) {
    auto &cfg = supportedConfigs[i];
    EGLint r, g, b, d;
    if (eglGetConfigAttrib(display, cfg, EGL_RED_SIZE, &r) &&
        eglGetConfigAttrib(display, cfg, EGL_GREEN_SIZE, &g) &&
        eglGetConfigAttrib(display, cfg, EGL_BLUE_SIZE, &b) &&
        eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &d) &&
        r == 8 && g == 8 && b == 8 && d == 0) {

      config = supportedConfigs[i];
      break;
    }
  }
  if (i == numConfigs) {
    config = supportedConfigs[0];
  }

  if (config == nullptr) {
    LOGW("Unable to initialize EGLConfig");
    return;
  }

  /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
   * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
   * As soon as we picked a EGLConfig, we can safely reconfigure the
   * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
  eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
  surface = eglCreateWindowSurface(display, config, this->mApp->window, nullptr);
  context = eglCreateContext(display, config, nullptr, nullptr);

  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
    LOGW("Unable to eglMakeCurrent");
    return;
  }

  eglQuerySurface(display, surface, EGL_WIDTH, &width);
  eglQuerySurface(display, surface, EGL_HEIGHT, &height);

  // Check openGL on the system
  auto opengl_info = {GL_VENDOR, GL_RENDERER, GL_VERSION, GL_EXTENSIONS};
  for (auto name: opengl_info) {
    auto info = glGetString(name);
    LOGD("OpenGL Info: %s", info);
  }
  glEnable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
}

void Renderer::draw_frame() {
  if (this->display == nullptr) {
    // No display.
    return;
  }

  LOGD("DRAW");

  // Just fill the screen with a color.
  glClearColor(1.0f, 0, 0, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  eglSwapBuffers(this->display, this->surface);
}
