/**
 * @file
 * ProximityScanner provides the scan results used by the Discovery framework and Rendezvous server
 */

/******************************************************************************
 * Copyright 2012, Qualcomm Innovation Center, Inc.
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
#ifndef _PROXIMITY_SCANNER_H_
#define _PROXIMITY_SCANNER_H_

#include <qcc/platform.h>
#include <map>

#include <qcc/Timer.h>
#include <qcc/Event.h>
#include <qcc/String.h>
#include <qcc/Thread.h>
#include <alljoyn/BusAttachment.h>

//
// We need JNI only when we are doing Java bindings
//

#if defined (QCC_OS_ANDROID)

#include <jni.h>

extern JavaVM* proxJVM;
extern JNIEnv* psenv;
extern jclass CLS_AllJoynAndroidExt;
extern jmethodID MID_AllJoynAndroidExt_Scan;
extern jclass CLS_ScanResultMessage;

#endif

class ProximityScanEngine;

namespace ajn {

class ProximityScanner {

  public:

    ProximityScanner(BusAttachment& bus);
    //void Scan();
    void Scan(bool request_scan);
    void PrintBSSIDMap(std::map<qcc::String, qcc::String>);
    std::map<std::pair<qcc::String, qcc::String>, bool> scanResults;
    BusAttachment& bus;


};

}

#endif

