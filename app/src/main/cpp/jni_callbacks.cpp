#include "jni_callbacks.h"
#include "string"

#define THIZ_CLS mEnv->FindClass("com/vladd11/arshop/NativeEngine")

static JavaVM *vm;

JNIEXPORT jint JNI_OnLoad(JavaVM *jvm, void *reserved) {
  vm = jvm;
  return JNI_VERSION_1_6;
}

JNICallbacks::JNICallbacks(JNIEnv *env, jobject thiz) {
  mEnv = env;
  mThiz = thiz;
}

JNICallbacks::~JNICallbacks() = default;

void JNICallbacks::loadPngToGL() {
  mEnv->CallStaticVoidMethod(THIZ_CLS, mEnv->GetStaticMethodID(THIZ_CLS, "loadImage", "()V"));
}

void JNICallbacks::setText(const char *str) {
  jstring jstr = mEnv->NewStringUTF(str);
  mEnv->CallVoidMethod(mThiz, mEnv->GetMethodID(mEnv->GetObjectClass(mThiz), "setText",
                                                "(Ljava/lang/String;)V"),
                       jstr);
  mEnv->DeleteLocalRef(jstr);
}

void JNICallbacks::update(JNIEnv *env, jobject thiz) {
  mEnv = env;
  mThiz = thiz;
}

void JNICallbacks::attach() {
  vm->AttachCurrentThread(&mEnv, nullptr);
}
