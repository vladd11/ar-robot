#include "helper.h"

extern "C" JNIEXPORT jlong JNICALL
Java_com_vladd11_arshop_NativeRenderer_newNativeEngine(JNIEnv *env, jobject thiz) {
  return ((jlong) new Engine());
}


extern "C"
JNIEXPORT void JNICALL
Java_com_vladd11_arshop_NativeRenderer_onSurfaceCreated(JNIEnv *env, jobject thiz, jlong pointer) {
  auto *renderer = (Engine *) pointer;
  renderer->init();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vladd11_arshop_NativeRenderer_onDrawFrame(JNIEnv *env, jobject thiz, jlong pointer) {
  auto *renderer = (Engine *) pointer;
  renderer->drawFrame();
}