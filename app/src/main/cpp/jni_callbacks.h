#ifndef AR_SHOP_JNI_CALLBACKS_H
#define AR_SHOP_JNI_CALLBACKS_H

#include <stdexcept>
#include "jni.h"

class JNICallbacks {
private:
  JNIEnv *mEnv{};
  jobject mThiz{};

public:
  JNICallbacks(JNIEnv *env, jobject thiz);

  ~JNICallbacks();

  void loadPngToGL();

  void attach();

  void setText(const char *str);

  void update(JNIEnv *env, jobject thiz);
};

#endif
