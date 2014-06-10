
// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Copyright (C) 2013 Sony Mobile Communications AB.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef jni_utils_h
#define jni_utils_h

#include <string>

#include "base/base_api.h"
#include "base/string16.h"
#include "nativehelper/jni.h"

namespace android {

namespace jni { // To avoid name conflict with similar functions in webkit

class JniUtilHelper {
public:
  JniUtilHelper();
  ~JniUtilHelper();

  // init has to be called on webkit's main tread or the UI thread
  void init();

  jclass m_jniUtilClass;
  jmethodID m_getAutoFillQueryMethodID;
  jmethodID m_contentUrlSizeMethodID;
  jmethodID m_contentUrlStreamMethodID;
private:
  bool m_initialized;
};

void BASE_API SetJavaVM(JavaVM* vm);

JniUtilHelper* getJniUtilHelper();

// Get the JNI environment for the current thread.
JNIEnv* GetJNIEnv();

// Convert Java String to std::string. On null input, returns an empty string.
std::string JstringToStdString(JNIEnv* env, jstring jstr);

string16 JstringToString16(JNIEnv* env, jstring jstr);

jstring ConvertUTF8ToJavaString(JNIEnv* env, std::string str);

bool CheckException(JNIEnv*);

void DetachFromVM();

} // namespace jni

} // namespace android
#endif // jni_utils_h
