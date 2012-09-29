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

#include <qcc/platform.h>
#include <map>
#include <qcc/String.h>
#include <qcc/StringUtil.h>
#include <utility>
#include <qcc/winrt/utility.h>
#include <collection.h>
#include <Objbase.h>
#include "ProximityScanner.h"

#define QCC_MODULE "PROXIMITY_SCANNER"

using namespace qcc;
using namespace Windows::Networking::Connectivity;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;


namespace ajn {

ProximityScanner::ProximityScanner(BusAttachment& bus) : bus(bus) {
    QCC_DbgTrace(("ProximityScanner::ProximityScanner()"));
}

void ProximityScanner::PrintBSSIDMap(std::map<qcc::String, qcc::String> mymap) {

    std::map<qcc::String, qcc::String>::iterator it;
    for (it = mymap.begin(); it != mymap.end(); it++) {
        //QCC_DbgPrintf(("\n BSSID : %s", it->first.c_str()));
    }
}

#define MAX_GUID_STRING_SIZE 39

void ProximityScanner::Scan(bool request_scan) {
    QCC_DbgTrace(("ProximityScanner::Scan()"));

    // Start with a clean slate
    //
    scanResults.clear();
    ConnectionProfile ^ internetConnectionProfile = NetworkInformation::GetInternetConnectionProfile();
    if (internetConnectionProfile == nullptr) {
        QCC_DbgPrintf(("This device is not connected to Internet."));
        return;
    }

    Platform::String ^ internetProfileName = internetConnectionProfile->ProfileName;
    WCHAR internetNetworkAdapterId[MAX_GUID_STRING_SIZE];
    Platform::String ^ internetNetworkAdapterIdStr = nullptr;
    if (StringFromGUID2(internetConnectionProfile->NetworkAdapter->NetworkAdapterId, internetNetworkAdapterId, ARRAYSIZE(internetNetworkAdapterId))) {
        internetNetworkAdapterIdStr = ref new Platform::String(internetNetworkAdapterId);
    } else {
        QCC_LogError(ER_FAIL, ("Fail to convert GUID to String"));
        return;
    }

    QCC_DbgPrintf(("The currently connected network = %s, NetworkAdapterId =%s", PlatformToMultibyteString(internetProfileName).c_str(), \
                   PlatformToMultibyteString(internetNetworkAdapterIdStr).c_str()));


    IVectorView<LanIdentifier ^> ^ lanIdentifiers = nullptr;
    try {
        lanIdentifiers = NetworkInformation::GetLanIdentifiers();
    } catch (Platform::Exception ^ e) {
        QCC_LogError(ER_OS_ERROR, ("ProximityScanner::Scan() AccessDeniedException: The 'Location' capability should be enabled for the Application to access the LanIdentifier (location information)"));
        // AccessDeniedException will be threw if Location capability is not enabled for the Application
        return;
    }

    QCC_DbgPrintf(("The number of found LanIdentifiers = %d", lanIdentifiers->Size));
    std::for_each(begin(lanIdentifiers), end(lanIdentifiers), [this, internetProfileName, internetNetworkAdapterIdStr](LanIdentifier ^ lanIdentifier) {
                      WCHAR networkAdapterId[MAX_GUID_STRING_SIZE];
                      if (StringFromGUID2(lanIdentifier->NetworkAdapterId, networkAdapterId, ARRAYSIZE(networkAdapterId))) {
                          Platform::String ^ networkAdapterIdStr = ref new Platform::String(networkAdapterId);
                          QCC_DbgPrintf(("LandIdentifier's NetworkAdapterId = %s", PlatformToMultibyteString(networkAdapterIdStr).c_str()));
                          if (networkAdapterIdStr->Equals(internetNetworkAdapterIdStr)) {
                              QCC_DbgPrintf(("Find matched NetworkAdapterId = %s", PlatformToMultibyteString(networkAdapterIdStr).c_str()));
                              qcc::String bssid;
                              auto lanIdVals = lanIdentifier->InfrastructureId->Value;
                              if (lanIdVals->Size != 0) {
                                  bool first = true;
                                  std::for_each(begin(lanIdVals), end(lanIdVals), [this, &first, &bssid](int lanIdVal)
                                                {
                                                    if (!first) {
                                                        bssid.append(':');
                                                    } else {
                                                        first = false;
                                                    }
                                                    // represent the BSSID in lower case
                                                    qcc::String tmpStr = U32ToString(lanIdVal, 16, 2, '0');
                                                    std::for_each(tmpStr.begin(), tmpStr.end(), [&bssid](int c) {
                                                                      bssid.append(tolower(c));
                                                                  });
                                                });
                                  qcc::String ssid = PlatformToMultibyteString(internetProfileName);
                                  scanResults.insert(std::map<std::pair<qcc::String, qcc::String>, bool>::value_type(std::make_pair(bssid, ssid), true));
                                  QCC_DbgPrintf(("Report scan result attached = %d ssid = %s bssid = %s", bssid.c_str(), ssid.c_str()));
                                  return;
                              } else {
                                  QCC_DbgPrintf(("LanIdentifier's size = %d is too small", lanIdVals->Size));
                              }
                          } else {
                              QCC_DbgPrintf(("The NetworkAdapterId (%s) does not match the current Internect connection", PlatformToMultibyteString(networkAdapterIdStr).c_str()));
                          }
                      }
                  });
}

}
