/******************************************************************************
 * Copyright 2009-2012, Qualcomm Innovation Center, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 ******************************************************************************/

#include <qcc/platform.h>
#include <qcc/String.h>
#include <android/log.h>
#include <qcc/Log.h>
#include <jni.h>

#define LOG_TAG  "P2pHelperService"
#ifndef LOGD
#define LOGD(...) (__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#endif

#ifndef LOGI
#define LOGI(...) (__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#endif

#ifndef LOGE
#define LOGE(...) (__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#endif

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm,
                                  void* reserved)
{
    /* Set AllJoyn logging */
    QCC_SetLogLevels("ALLJOYN=7;ALL=1");
    QCC_UseOSLogging(true);

    JNIEnv* env;
    if (!vm) {
        LOGD("VM is NULL\n");
    }
    jint jret = vm->GetEnv((void**)&env, JNI_VERSION_1_2);
    if (JNI_EDETACHED == jret) {
        vm->AttachCurrentThread(&env, NULL);
        LOGD("Attached to VM thread \n");
    }

    jclass clazz;
    clazz = env->FindClass("org/alljoyn/bus/p2p/service/P2pHelperService");
    if (!clazz) {
        LOGE("***** Unable to FindClass P2pHelperService **********");
        env->ExceptionDescribe();
        return JNI_ERR;
    } else {
        LOGI("org/alljoyn/jni/P2pHelperService loaded SUCCESSFULLY");
    }

    if (JNI_EDETACHED == jret) {
        vm->DetachCurrentThread();
    }

    return JNI_VERSION_1_2;
}

#ifdef __cplusplus
}
#endif
