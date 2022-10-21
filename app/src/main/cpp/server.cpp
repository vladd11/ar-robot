#include "server.h"

#define TAG "Server"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static const char *s_listen_on = "ws://0.0.0.0:8000";

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_OPEN) {
    // c->is_hexdumping = 1;
  } else if (ev == MG_EV_HTTP_MSG) {
    auto *hm = (struct mg_http_message *) ev_data;
      // Upgrade to websocket. From now on, a connection is a full-duplex
      // Websocket connection, which will receive MG_EV_WS_MSG events.
      mg_ws_upgrade(c, hm, nullptr);
  } else if (ev == MG_EV_WS_MSG) {
    // Got websocket frame. Received data is wm->data. Echo it back!
    auto *wm = (struct mg_ws_message *) ev_data;
    mg_ws_send(c, wm->data.ptr, wm->data.len, WEBSOCKET_OP_TEXT);
  }
  (void) fn_data;
}

void server_thread(const bool *interrupt) {
  mg_mgr mgr{};
  mg_mgr_init(&mgr);
  mg_http_listen(&mgr, s_listen_on, fn, nullptr);
  while (!*interrupt) mg_mgr_poll(&mgr, 1000);
  mg_mgr_free(&mgr);
}
