#include "helper.h"

inline Engine *native(jlong pointer) {
  return reinterpret_cast<Engine *>(pointer);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_vladd11_arshop_NativeEngine_newNativeEngine(JNIEnv *env, jobject thiz,
                                                     jobject j_asset_manager) {
  AAssetManager *assetManager = AAssetManager_fromJava(env, j_asset_manager);
  return ((jlong) new Engine(assetManager));
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
extern "C"
JNIEXPORT jobject JNICALL
Java_com_vladd11_arshop_NativeEngine_takeFrame(JNIEnv *env, jobject thiz, jlong pointer) {
  //native(pointer)->takeFrame();

  jbyte array[3] = {
      0x0,
      0x0,
      0x0
  };

  jobject result = env->NewDirectByteBuffer(array, 3);

  return result;
}