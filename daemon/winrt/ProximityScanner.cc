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
    Platform::String ^ internetProfileName = internetConnectionProfile->ProfileName;
    std::map<Platform::String ^, Platform::String ^> profileNames;
    auto connectionProfiles = NetworkInformation::GetConnectionProfiles();
    std::for_each(begin(connectionProfiles), end(connectionProfiles), [this, &profileNames](ConnectionProfile ^ profile) {
                      WCHAR networkAdapterId[MAX_GUID_STRING_SIZE];
                      if (StringFromGUID2(profile->NetworkAdapter->NetworkAdapterId, networkAdapterId, ARRAYSIZE(networkAdapterId))) {
                          Platform::String ^ strAdaptertId = ref new Platform::String(networkAdapterId);
                          profileNames[strAdaptertId] = profile->ProfileName;
                      }
                  });

    if (profileNames.size() == 0) {
        QCC_DbgPrintf(("No Network Connection Profiles Found."));
        return;
    }
    IVectorView<LanIdentifier ^> ^ lanIdentifiers = nullptr;
    try {
        lanIdentifiers = NetworkInformation::GetLanIdentifiers();
    } catch (Platform::Exception ^ e) {
        QCC_LogError(ER_OS_ERROR, ("ProximityScanner::Scan() AccessDeniedException: The 'Location' capability should be enabled for the Application to access the LanIdentifier (location information)"));
        // AccessDeniedException will be threw if Location capability is not enabled for the Application
        return;
    }
    std::for_each(begin(lanIdentifiers), end(lanIdentifiers), [this, &profileNames, internetProfileName](LanIdentifier ^ lanIdentifier) {
                      WCHAR networkAdapterId[MAX_GUID_STRING_SIZE];
                      if (StringFromGUID2(lanIdentifier->NetworkAdapterId, networkAdapterId, ARRAYSIZE(networkAdapterId))) {
                          Platform::String ^ networkAdapterIdStr = ref new Platform::String(networkAdapterId);
                          std::map<Platform::String ^, Platform::String ^>::iterator it = profileNames.find(networkAdapterIdStr);
                          if (it != profileNames.end()) {
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
                                  qcc::String ssid = PlatformToMultibyteString(it->second);
                                  bool attached = (internetProfileName == it->second) ? true : false;
                                  // only return the WIFI AP currently connected to.
                                  if (attached) {
                                      scanResults.insert(std::map<std::pair<qcc::String, qcc::String>, bool>::value_type(std::make_pair(bssid, ssid), attached));
                                      return;
                                  }
                              }
                          }
                      }
                  });
}

}
