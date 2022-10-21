#ifndef AR_SHOP_SERVER_H
#define AR_SHOP_SERVER_H

#include "mongoose.h"
#include "android/log.h"
#include "atomic"
#include "string"

static const char *s_listen_on = "ws://0.0.0.0:8000";

class ServerThread {
private:
  const bool *mInterrupt;

public:
  std::string *mCodeStr;
  std::atomic<bool> mUpdateCode = false;

  ServerThread(const bool *interrupt) {
    mCodeStr = new std::string();
    mInterrupt = interrupt;
  };

  ~ServerThread() = default;

  void operator()();
};

#endif //AR_SHOP_SERVER_H
