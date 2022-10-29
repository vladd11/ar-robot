#include "server.h"

#define TAG "Server"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

ServerThread::ServerThread() {
  mCodeStr = new std::string();
}

ServerThread::~ServerThread() = default;

void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_OPEN) {
    // c->is_hexdumping = 1;
  } else if (ev == MG_EV_HTTP_MSG) {
    auto *hm = (struct mg_http_message *) ev_data;
    // Upgrade to websocket. From now on, a connection is a full-duplex
    // Websocket connection, which will receive MG_EV_WS_MSG events.
    mg_ws_upgrade(c, hm, nullptr);
  } else if (ev == MG_EV_WS_OPEN) {
    auto self = reinterpret_cast<ServerThread *>(fn_data);
    self->mConnections.push_back(c);
  } else if (ev == MG_EV_WS_MSG) {
    // Got websocket frame. Received data is wm->data. Echo it back!
    auto *wm = (struct mg_ws_message *) ev_data;

    auto self = reinterpret_cast<ServerThread *>(fn_data);
    self->mCodeStr->assign(wm->data.ptr, wm->data.len);
    self->mUpdateCode = true;
  }
  (void) fn_data;
}

void ServerThread::operator()() {
  mg_mgr mgr{};
  mg_mgr_init(&mgr);
  mg_http_listen(&mgr, s_listen_on, fn, this);

  LOGD("Listening port 8000 for any commands");
  while (!mInterrupt) {
    mg_mgr_poll(&mgr, 100);

    Message *msg = out.dequeue();
    if (msg != nullptr) {
      for (mg_connection *connection: mConnections) {
        mg_ws_send(connection, msg->buf, msg->len, WEBSOCKET_OP_TEXT);
      }
    }
  }

  LOGD("Stopping Websockets server");
  mg_mgr_free(&mgr);
}
