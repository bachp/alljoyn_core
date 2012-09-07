/**
 * @file
 * Singleton for the AllJoyn IP Name Service
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

#ifndef _IP_NAME_SERVICE_H
#define _IP_NAME_SERVICE_H

#ifndef __cplusplus
#error Only include IpNameService.h in C++ code.
#endif

#include <qcc/String.h>
#include <qcc/IPAddress.h>

#include <Status.h>
#include <Callback.h>

namespace ajn {

class IpNameServiceImpl;

/**
 * @brief API to provide an implementation dependent IP (Layer 3) Name Service
 * singleton for AllJoyn.
 */
class IpNameService {
  public:

    static IpNameService& Instance()
    {
        static IpNameService ipNameService;
        return ipNameService;
    }

    QStatus Start();
    bool Started();
    QStatus Stop();
    QStatus Join();

    QStatus Init(
        const qcc::String& guid,
        bool enableIPv4,
        bool enableIPv6,
        bool disableBroadcast,
        bool loopback = false);

    void SetCallback(Callback<void, const qcc::String&, const qcc::String&, std::vector<qcc::String>&, uint8_t>* cb);

    QStatus OpenInterface(const qcc::String& name);
    QStatus OpenInterface(const qcc::IPAddress& address);

    QStatus CloseInterface(const qcc::String& name);
    QStatus CloseInterface(const qcc::IPAddress& address);

    QStatus Enable();
    QStatus Disable();
    bool Enabled();

    QStatus FindAdvertisedName(const qcc::String& wkn);
    QStatus CancelFindAdvertisedName(const qcc::String& wkn);

    QStatus AdvertiseName(const qcc::String& wkn);
    QStatus CancelAdvertiseName(const qcc::String& wkn);

    QStatus SetEndpoints(
        const qcc::String& ipv4address,
        const qcc::String& ipv6address,
        uint16_t port);

    QStatus GetEndpoints(
        qcc::String& ipv4address,
        qcc::String& ipv6address,
        uint16_t& port);

  private:
    IpNameService();
    virtual ~IpNameService();

    IpNameService(const IpNameService& other);
    IpNameService& operator =(const IpNameService& other);

    bool m_constructed;
    bool m_destroyed;
    IpNameServiceImpl* m_pimpl;
};

} // namespace ajn

#endif // _IP_NAME_SERVICE_H
