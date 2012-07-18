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
#include <qcc/GUID.h>

#include "NsProtocol.h"
#include "ProximityListener.h"

namespace ajn {

ref class ProximityNameService sealed {
  public:
    void ConnectionRequestedEventHandler(Platform::Object ^ sender, Windows::Networking::Proximity::ConnectionRequestedEventArgs ^ TriggeredConnectionStateChangedEventArgs);

  private:
    friend class ProximityTransport;
    static Platform::String ^ PROXIMITY_ALT_ID_ALLJOYN;             /**< The Alt ID for AllJoyn. Two devices have the same Alt ID will rendezvous. The length limit is 127 unicode charaters */
    static const uint32_t MAX_DISPLAYNAME_SIZE = 49;         /**< The maximum number of unicode charaters that DisplayName property of PeerFinder allows */
    static const uint32_t MAX_CONNECT_RETRY = 3;             /**< The number of retry when connecting to a peer */
    static const uint32_t TRANSMIT_INTERVAL = 10 * 1000;     /**< The default interval of transmitting well-known name advertisement */
    static const uint32_t DEFAULT_DURATION = (12);           /**< The default lifetime of a found well-known name */

    enum ProximState {
        PROXIM_DISCONNECTED,                                 /**< Not connected to a peer */
        PROXIM_BROWSING,                                     /**< Browsing peers */
        PROXIM_CONNECTING,                                   /**< Connecting to a peer */
        PROXIM_ACCEPTING,                                    /**< Accepting connection from a peer */
        PROXIM_CONNECTED,                                    /**< Connected to a peer */
    };

    ProximityNameService(const qcc::String & guid);
    ~ProximityNameService();
    void SetCallback(Callback<void, const qcc::String&, const qcc::String&, std::vector<qcc::String>&, uint8_t>* cb);
    void EnableDiscovery(const qcc::String& namePrefix);
    void DisableDiscovery(const qcc::String& namePrefix);
    void EnableAdvertisement(const qcc::String& name);
    void DisableAdvertisement(std::vector<qcc::String>& wkns);

    /**
     * @internal
     * @brief Send the protocol message over the proximity connection.
     */
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

    /**
     * Start Proximity name service
     */
    void Start();
    /**
     * Stop Proximity name service
     */
    void Stop();
    /**
     * Browse proximity peers to discover service
     */
    void BrowsePeers();
    /**
     * Connect to a proximity peer
     */
    QStatus Connect(Windows::Networking::Proximity::PeerInformation ^ peerInfo);
    /**
     * Reset the current proximity connection
     */
    void Reset();
    /**
     * Reset the current proximity connection and re-browse proximity peers
     */
    void Restart();
    /**
     * Start the loop to read data over the created proximity connection
     */
    void StartReader();
    /**
     * Handle errors of the proximity stream socket
     */
    void SocketError(qcc::String& errMsg);

    QStatus GetEndpoints(qcc::String& ipv6address, uint16_t& port);
    void SetEndpoints(const qcc::String& ipv6address, const uint16_t port);
    /**
     * Increase the number of overlay TCP connections that depends on the current proximity connection
     */
    int32_t IncreaseOverlayTCPConnection();
    /**
     * Decrease the number of overlay TCP connections that depends on the current proximity connection
     */
    int32_t DecreaseOverlayTCPConnection();
    bool IsConnected() { return m_currentState == PROXIM_CONNECTED; }
    ProximState GetCurrentState() { return m_currentState; }
    void RegisterProximityListener(ProximityListener* listener);
    void UnRegisterProximityListener(ProximityListener* listener);
    /**
     * Notify the proximity listeners when the proximity connection is broken
     */
    void NotifyDisconnected();

    /**
     * Start a timer that triggers periodically to transmit well-known names to peers
     */
    void StartMaintainanceTimer();
    void TimerCallback(Windows::System::Threading::ThreadPoolTimer ^ timer);
    /**
     * Transmit well-known names to peers
     */
    void TransmitMyWKNs();
    /**
     * Inquire a connected peer to discover service
     */
    void Locate(const qcc::String& namePrefix);

    bool IsBrowseConnectSupported();
    bool IsTriggeredConnectSupported();
    /**
     * Encode advertised well-known names into a string given the limit of the length of PeerFinder::DisplayName property
     */
    Platform::String ^ EncodeWknAdvertisement();
    /**
     * Check whether should browse peers for service discovery
     */
    bool ShouldDoDiscovery();
    /**
     * Check whether a found well-known name matches the prefix
     */
    bool MatchNamePrefix(qcc::String wkn);

    Windows::Foundation::Collections::IVectorView<Windows::Networking::Proximity::PeerInformation ^> ^ m_peerInformationList;  /** The list of peers already found */
    Windows::Networking::Proximity::PeerInformation ^ m_requestingPeer;   /**< The peer that has requested connection */
    Windows::Networking::Sockets::StreamSocket ^ m_socket;        /**< The current proximity stream socket connection if connnected to a peer */
    Windows::Storage::Streams::DataReader ^ m_dataReader;         /**< The data reader associated with the current proximity stream socket connection */
    Windows::Storage::Streams::DataWriter ^ m_dataWriter;         /**< The data writer associated with the current proximity stream socket connection */
    Windows::System::Threading::ThreadPoolTimer ^ m_timer;        /**< The periodical timer for transmitting well-name */
    Windows::Foundation::EventRegistrationToken m_token;          /**< The token used to remove ConnectionRequested event handler */

    Callback<void, const qcc::String&, const qcc::String&, std::vector<qcc::String>&, uint8_t>* m_callback;      /**< The callback to notify the proximity transport about the found name */
    bool m_triggeredConnectSupported;                             /**< Does this device support triggered mode (NFC) */
    bool m_browseConnectSupported;                                /**< Does this device support browse mode (WIFI-Direct) */
    bool m_peerFinderStarted;                                     /**< Whether PeerFinder::Start() is called */
    bool m_socketClosed;                                          /**< Whether the proximity stream socket is closed */
    std::set<qcc::String> m_namePrefixs;                          /**< The name prefixs the daemon tried to discover */
    std::set<qcc::String> m_advertised;                           /**< The well-known names the daemon advertised */
    qcc::String m_guid;                                           /**< The daemon GUID string */
    qcc::String m_sguid;                                               /**< The daemon GUID short string */
    bool m_locateStarted;                                         /**< Whether already being browsing peers */
    ProximState m_currentState;                                   /**< The current proximity connection state */
    bool m_doDiscovery;                                           /**< Whether PeerFinder should browse peers to discover well-known name */
    uint32_t m_connectRetries;                                    /**< The number of tried connecting to a found peer */
    qcc::Mutex m_mutex;
    uint16_t m_port;                                              /**< The port associated with the name service */
    uint32_t m_tDuration;                                         /**< The lifetime of a found advertised well-known nmae */
    qcc::String m_listenAddr;                                     /**< The listen address */
    int32_t m_tcpConnCount;                                      /**< Number of overlay TCP connections that depend on current proximity connection */
    std::list<ProximityListener*> m_listeners;                    /**< List of ProximityListeners */
};

} // namespace ajn

#endif //_ALLJOYN_PROXIMITYNAMESERVICE_H