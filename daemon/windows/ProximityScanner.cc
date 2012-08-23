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

#pragma comment(lib, "wlanapi.lib")

#include <qcc/platform.h>
#include <map>
#include <stdio.h>
#include <qcc/String.h>
#include <qcc/StringUtil.h>
#include <Status.h>
#include <alljoyn/Message.h>
#include <alljoyn/MsgArg.h>
#include <utility>
#include "ProximityScanner.h"
#include <windows.h>


#define QCC_MODULE "PROXIMITY_SCANNER"

using namespace qcc;

bool notification = false;


namespace ajn {

// This is the callback function that tells us about the Wlan events that we registered for
void ScanNotification(WLAN_NOTIFICATION_DATA*data, VOID*context)
{

}

ProximityScanner::ProximityScanner(BusAttachment& bus) : bus(bus), notification(false) {
    QCC_DbgTrace(("ProximityScanner::ProximityScanner()"));
}

void ProximityScanner::PrintBSSIDMap(std::map<qcc::String, qcc::String> mymap) {

    std::map<qcc::String, qcc::String>::iterator it;
    for (it = mymap.begin(); it != mymap.end(); it++) {
        //QCC_DbgPrintf(("\n BSSID : %s", it->first.c_str()));
    }

}


void ProximityScanner::Scan(bool request_scan) {

}
}