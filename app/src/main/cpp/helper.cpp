#include "helper.h"

#define TAG "Helper"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

inline Engine *native(jlong pointer) {
  return reinterpret_cast<Engine *>(pointer);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_vladd11_arshop_NativeEngine_newNativeEngine(JNIEnv *env, jobject thiz,
                                                     jstring jStoragePath) {
  std::string storagePath;
  storagePath = env->GetStringUTFChars(jStoragePath, nullptr);
  return ((jlong) new Engine(storagePath, env));
}

extern "C" JNIEXPORT void JNICALL
Java_com_vladd11_arshop_NativeEngine_onSurfaceCreated(JNIEnv *env, jobject thiz,
                                                      jlong pointer) {
  native(pointer)->init();
}

extern "C" JNIEXPORT void JNICALL
Java_com_vladd11_arshop_NativeEngine_onDrawFrame(JNIEnv *env, jobject thiz, jlong pointer) {
  native(pointer)->drawFrame();
}

extern "C" JNIEXPORT void JNICALL
Java_com_vladd11_arshop_NativeEngine_onResume(JNIEnv *env, jobject thiz, jlong pointer,
                                              jobject context, jobject activity) {
  native(pointer)->resume(env, context, activity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_vladd11_arshop_NativeEngine_onSurfaceChanged(JNIEnv *env, jobject thiz, jlong pointer,
                                                      jint rotation, jint width, jint height) {
  native(pointer)->resize(rotation, width, height);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_vladd11_arshop_NativeEngine_onTouch(JNIEnv *env, jobject thiz, jlong pointer, jfloat x,
                                             jfloat y) {
  native(pointer)->onTouch(x, y);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vladd11_arshop_NativeEngine_onPause(JNIEnv *env, jobject thiz, jlong pointer) {
  native(pointer)->pause();
}
