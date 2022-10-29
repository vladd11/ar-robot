#ifndef AR_SHOP_SERVER_H
#define AR_SHOP_SERVER_H

#include "mongoose.h"
#include "android/log.h"
#include "atomic"
#include "string"

static const char *s_listen_on = "ws://0.0.0.0:8000";

class ServerThread {
public:
  std::string *mCodeStr;
  std::atomic<bool> mInterrupt = false;
  std::atomic<bool> mUpdateCode = false;

  ServerThread();
  ~ServerThread();

  void operator()();
};

#endif //AR_SHOP_SERVER_H
