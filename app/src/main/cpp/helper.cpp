#include "helper.h"

inline Engine *native(jlong pointer) {
  return reinterpret_cast<Engine *>(pointer);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_vladd11_arshop_NativeEngine_newNativeEngine(JNIEnv *env, jobject thiz) {
  return ((jlong) new Engine());
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
JNIEXPORT void JNICALL
Java_com_vladd11_arshop_NativeEngine_takeFrame(JNIEnv *env, jobject thiz, jlong pointer) {
  CaptureResult frame = native(pointer)->takeFrame();
  if (frame.frame == nullptr) {
    jclass clazz = env->GetObjectClass(thiz);
    jmethodID callback = env->GetMethodID(clazz, "onImageCaptured",
                                          "(Ljava/nio/ByteBuffer;II)V");

    env->CallVoidMethod(thiz, callback,
                        nullptr,
                        frame.width, frame.height);
    return;
  }

  jclass clazz = env->GetObjectClass(thiz);
  jmethodID callback = env->GetMethodID(clazz, "onImageCaptured",
                                        "(Ljava/nio/ByteBuffer;II)V");
  env->CallVoidMethod(thiz, callback,
                      env->NewDirectByteBuffer(frame.frame, frame.width * frame.height * 4),
                      frame.width, frame.height);
}
