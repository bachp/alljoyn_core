/**
 * @file
 * ProximityNameService implements a wrapper layer to utilize the WinRT proximity API
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

#ifndef _ALLJOYN_PROXIMITYNAMESERVICE_H
#define _ALLJOYN_PROXIMITYNAMESERVICE_H

#include <algorithm>
#include <vector>
#include <set>
#include <list>
#include <Status.h>
#include <Callback.h>
#include <qcc/platform.h>
#include <qcc/String.h>
#include <qcc/Mutex.h>
#include <qcc/Debug.h>

#include "NsProtocol.h"

namespace ajn {

ref class ProximityNameService sealed {
  public:
    ~ProximityNameService();
    void ConnectionRequestedEventHandler(Platform::Object ^ sender, Windows::Networking::Proximity::ConnectionRequestedEventArgs ^ TriggeredConnectionStateChangedEventArgs);

  private:
    friend class ProximityTransport;
    static const uint32_t MAX_DISPLAYNAME_SIZE = 49;
    static const uint32_t MAX_CONNECT_RETRY = 3;
    static const uint32_t TRANSMIT_INTERVAL = 10 * 1000;
    static const uint32_t DEFAULT_DURATION = (120);

    enum ProximState {
        PROXIM_DISCONNECTED,
        PROXIM_BROWSING,
        PROXIM_CONNECTING,
        PROXIM_ACCEPTING,
        PROXIM_CONNECTED,
    };

    ProximityNameService(const qcc::String & guid);
    void SetCallback(Callback<void, const qcc::String&, const qcc::String&, std::vector<qcc::String>&, uint8_t>* cb);
    void EnableDiscovery(const qcc::String& namePrefix);
    void DisableDiscovery(const qcc::String& namePrefix);
    void EnableAdvertisement(const qcc::String& name);
    void DisableAdvertisement(std::vector<qcc::String>& wkns);
    QStatus GetEndpoints(qcc::String& ipv6address, uint16_t& port)
    {
        QStatus status = ER_OK;
        if (!m_listenAddr.empty()) {
            ipv6address = m_listenAddr;
            port = m_port;
        } else {
            status = ER_FAIL;
        }
        return status;
    }
    void SetEndpoints(const qcc::String& ipv6address, const uint16_t port);

    void SendProtocolMessage(Header& header);
    /**
     * @internal
     * @brief Do something with a received protocol message.
     */
    void HandleProtocolMessage(uint8_t const* const buffer, uint32_t nbytes, qcc::IPAddress address);

    /**
     * @internal
     * @brief Do something with a received protocol question.
     */
    void HandleProtocolQuestion(WhoHas whoHas, qcc::IPAddress address);

    /**
     * @internal
     * @brief Do something with a received protocol answer.
     */
    void HandleProtocolAnswer(IsAt isAt, uint32_t timer, qcc::IPAddress address);

    void Start();
    void Stop() { }
    void BrowsePeers();
    QStatus Connect(Windows::Networking::Proximity::PeerInformation ^ peerInfo);
    QStatus Listen();
    void Reset();
    void Restart();
    void StartReader();
    void SocketError(qcc::String& errMsg);

    void StartMaintainanceTimer();
    void TimerCallback(Windows::System::Threading::ThreadPoolTimer ^ timer);
    void Retransmit();
    Callback<void, const qcc::String&, const qcc::String&, std::vector<qcc::String>&, uint8_t>* m_callback;

    bool IsBrowseConnectSupported();
    bool IsTriggeredConnectSupported();
    Platform::String ^ EncodeWknAdvertisement();
    bool ShouldDoDiscovery();
    bool MatchNamePrefix(qcc::String wkn);

    Windows::Foundation::Collections::IVectorView<Windows::Networking::Proximity::PeerInformation ^> ^ m_peerInformationList;
    Windows::Networking::Proximity::PeerInformation ^ m_requestingPeer;
    Windows::Networking::Sockets::StreamSocket ^ m_socket;
    Windows::Storage::Streams::DataReader ^ m_dataReader;
    Windows::Storage::Streams::DataWriter ^ m_dataWriter;
    Windows::System::Threading::ThreadPoolTimer ^ m_timer;

    bool m_triggeredConnectSupported;
    bool m_browseConnectSupported;
    bool m_peerFinderStarted;
    bool m_socketClosed;
    std::set<qcc::String> m_namePrefixs;
    std::set<qcc::String> m_advertised;
    qcc::String m_guid;
    bool m_locateStarted;
    ProximState m_currentState;
    bool m_doDiscovery;
    uint32_t m_connectRetries;
    qcc::Mutex m_mutex;
    uint16_t m_port;
    uint32_t m_tDuration;
    qcc::String m_listenAddr;
};

} // namespace ajn

#endif //_ALLJOYN_PROXIMITYNAMESERVICE_H