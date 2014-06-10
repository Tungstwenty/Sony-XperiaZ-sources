 // Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Copyright (C) 2013 Sony Mobile Communications AB.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "android/jni/jni_utils.h"
#include "base/logging.h"
#include "base/utf_string_conversions.h"
#include "jni.h"

namespace {
JavaVM* sVM = NULL;
android::jni::JniUtilHelper sJniUtilHelper;

JavaVM* getJavaVM() {
  return sVM;
}
}

namespace android {
namespace jni {

JniUtilHelper::JniUtilHelper() :
    m_jniUtilClass(0),
    m_getAutoFillQueryMethodID(0),
    m_contentUrlSizeMethodID(0),
    m_contentUrlStreamMethodID(0),
    m_initialized(false) {
}

JniUtilHelper::~JniUtilHelper() {
  if (m_jniUtilClass) {
    GetJNIEnv()->DeleteGlobalRef(m_jniUtilClass);
  }
}

void JniUtilHelper::init() {
  if (!m_initialized) {
    JNIEnv* env = android::jni::GetJNIEnv();

    jclass localClass = env->FindClass("com/sonymobile/webkit/JniUtil");
    if (!localClass) {
      LOG(ERROR) << "failed to find JniUtil java class";
    }
    m_jniUtilClass = static_cast<jclass>(env->NewGlobalRef(localClass));
    env->DeleteLocalRef(localClass);

    m_getAutoFillQueryMethodID = env->GetStaticMethodID(m_jniUtilClass,
        "getAutofillQueryUrl", "()Ljava/lang/String;");
    m_contentUrlSizeMethodID = env->GetStaticMethodID(m_jniUtilClass,
        "contentUrlSize", "(Ljava/lang/String;)J");
    m_contentUrlStreamMethodID = env->GetStaticMethodID(m_jniUtilClass,
        "contentUrlStream", "(Ljava/lang/String;)Ljava/io/InputStream;");

    m_initialized = true;
  }
}

JniUtilHelper* getJniUtilHelper() {
  return &sJniUtilHelper;
}

// All JNI code assumes this has previously been called with a valid VM
void SetJavaVM(JavaVM* vm)
{
  sVM = vm;

  sJniUtilHelper.init();
}

bool checkException(JNIEnv* env)
{
  if (env->ExceptionCheck() != 0)
  {
    LOG(ERROR) << "*** Uncaught exception returned from Java call!\n";
    env->ExceptionDescribe();
    return true;
  }
  return false;
}

string16 jstringToString16(JNIEnv* env, jstring jstr)
{
  if (!jstr || !env)
    return string16();

  const char* s = env->GetStringUTFChars(jstr, 0);
  if (!s)
    return string16();
  string16 str = UTF8ToUTF16(s);
  env->ReleaseStringUTFChars(jstr, s);
  checkException(env);
  return str;
}

std::string jstringToStdString(JNIEnv* env, jstring jstr)
{
  if (!jstr || !env)
    return std::string();

  const char* s = env->GetStringUTFChars(jstr, 0);
  if (!s)
    return std::string();
  std::string str(s);
  env->ReleaseStringUTFChars(jstr, s);
  checkException(env);
  return str;
}

JNIEnv* GetJNIEnv() {
  JNIEnv* env;
  getJavaVM()->AttachCurrentThread(&env, NULL);
  return env;
}

std::string JstringToStdString(JNIEnv* env, jstring jstr) {
  return jstringToStdString(env, jstr);
}

string16 JstringToString16(JNIEnv* env, jstring jstr)
{
    return jstringToString16(env, jstr);
}

bool CheckException(JNIEnv* env)
{
    return checkException(env);
}

jstring ConvertUTF8ToJavaString(JNIEnv* env, std::string str)
{
    return env->NewStringUTF(str.c_str());
}

void DetachFromVM()
{
    JavaVM* vm = getJavaVM();
    vm->DetachCurrentThread();
}

} // namespace jni
} // namespace android

