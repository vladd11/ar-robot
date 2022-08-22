#include "activity.h"

static void handle_cmd(struct android_app *app, int32_t cmd) {
  LOGD("handle_cmd called with %i", cmd);
  auto renderer = (Renderer *) app->userData;
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      // The window is being shown, get it ready.
      if (app->window != nullptr && renderer) {
        renderer->init_display(app);
        renderer->draw_frame();
      }
      break;
    case APP_CMD_TERM_WINDOW:
      // The window is being hidden or closed, clean it up.
      delete (renderer);
      break;
    case APP_CMD_LOST_FOCUS:
      // STOP all rendering there
      renderer->draw_frame();
      break;
    default:
      break;
  }
}

void android_main(struct android_app *app) {
  auto *renderer = new Renderer();

  app->userData = &renderer;
  app->onAppCmd = handle_cmd;

  while (true) {
    int ident;
    int events;
    struct android_poll_source *source;

    while ((ident = ALooper_pollAll(-1, nullptr, &events,
                                    (void **) &source)) >= 0) {
      if (source != nullptr) {
        source->process(app, source);
      }

      renderer->draw_frame();
    }
  }
}
