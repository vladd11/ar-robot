#ifndef AR_SHOP_SERVER_H
#define AR_SHOP_SERVER_H

#include "mongoose.h"
#include "android/log.h"
#include "atomic"
#include "vector"
#include "string"
#include "safe_queue.h"

static const char *s_listen_on = "ws://0.0.0.0:8000";

class ServerThread {
public:
  std::vector<mg_connection *> mConnections;
  SafeQueue<std::string *> out;
  std::string *mCodeStr;
  std::atomic<bool> mInterrupt = false;
  std::atomic<bool> mUpdateCode = false;

  ServerThread();

  ~ServerThread();

  void operator()();
};

#endif //AR_SHOP_SERVER_H
