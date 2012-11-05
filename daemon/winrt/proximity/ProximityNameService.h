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

#include <ns/IpNsProtocol.h>
#include "ProximityListener.h"

namespace ajn {

ref class ProximityNameService sealed {
  public:

    void ConnectionRequestedEventHandler(Platform::Object ^ sender, Windows::Networking::Proximity::ConnectionRequestedEventArgs ^ TriggeredConnectionStateChangedEventArgs);
    /**
     * Increase the number of overlay TCP connections that depends on the current proximity connection
     */
    int32_t IncreaseOverlayTCPConnection();
    /**
     * Decrease the number of overlay TCP connections that depends on the current proximity connection
     */
    int32_t DecreaseOverlayTCPConnection();

  private:
    friend class ProximityTransport;
    static const uint32_t MAX_PROXIMITY_ALT_ID_SIZE = 127;   /**< The Alt ID for AllJoyn. Two devices have the same Alt ID will rendezvous. The length limit is 127 unicode charaters */
    static const uint32_t MAX_DISPLAYNAME_SIZE = 49;         /**< The maximum number of unicode charaters that DisplayName property of PeerFinder allows */
    static const uint32_t TRANSMIT_INTERVAL = 16 * 1000;     /**< The default interval of transmitting well-known name advertisement */
    static const uint32_t DEFAULT_DURATION = (20);           /**< The default lifetime of a found well-known name */
    static const uint32_t DEFAULT_PREASSOCIATION_TTL = 254;      /**< The default ttl used for the well-known names found during service pre-association */

    enum ProximState {
        PROXIM_DISCONNECTED,                                 /**< Not connected to a peer */
        PROXIM_BROWSING,                                     /**< Browsing peers */
        PROXIM_CONNECTING,                                   /**< Connecting to a peer */
        PROXIM_ACCEPTING,                                    /**< Accepting connection from a peer */
        PROXIM_CONNECTED,                                    /**< Connected to a peer */
    };

    struct CurrentP2PConnection {
        ProximState state;                                      /**< The current state if a P2P connection exists */
        qcc::String localIp;                                    /**< The local IPv6 address assigned when a P2P connection is created */
        qcc::String remoteIp;                                   /**< The remote peer's IPv6 address assigned when a P2P connection is created  */
        uint16_t localPort;                                     /**< The local port(service name) assigned when a P2P connection is created */
        uint16_t remotePort;                                    /**< The remote port(service name) assigned when a P2P connection is created */
        qcc::String peerGuid;                                   /**< The GUID (short version, 8 byte) of the remote peer if a P2P connection exists */
        Windows::Networking::Sockets::StreamSocket ^ socket;
        Windows::Storage::Streams::DataReader ^ dataReader;     /**< The data reader associated with the current proximity stream socket connection */
        Windows::Storage::Streams::DataWriter ^ dataWriter;     /**< The data writer associated with the current proximity stream socket connection */
        bool socketClosed;                                      /**< Indicate whether the StreamSocket corresponds to the current P2P connection is closed */
    };

    /**
     * @internal
     * @brief Constructor.
     * @param guid The daemon bus's 128-bit GUID string
     */
    ProximityNameService(const qcc::String & guid);
    ~ProximityNameService();
    QStatus EstasblishProximityConnection(qcc::String guidStr);
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
     * Reset the current proximity connection
     */
    void ResetConnection();
    /**
     * Reset the current proximity connection and start PeerFinder
     */
    void RestartPeerFinder();
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
    bool IsConnected() { return m_currentP2PLink.state == PROXIM_CONNECTED; }
    ProximState GetCurrentState() { return m_currentP2PLink.state; }

    /**
     * Register to receive notification when the proximity connection is broken
     */
    void RegisterProximityListener(ProximityListener* listener);

    /**
     * Stop receiving notification when the proximity connection is broken
     */
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

    /** Does this device support triggered mode (NFC) */
    bool IsTriggeredConnectSupported();

    /** Does this device support browse mode (WIFI-Direct) */
    bool IsBrowseConnectSupported();

    /**
     * Encode advertised well-known names into a string given the limit of the length of PeerFinder::DisplayName property
     */
    Platform::String ^ EncodeWknAdvertisement();
    /**
     * Check whether should browse peers for service discovery
     */
    bool ShouldDoDiscovery();

    bool GetPeerConnectSpec(const qcc::String guid, qcc::String& connectSpec);

    Windows::System::Threading::ThreadPoolTimer ^ m_timer;        /**< The periodical timer for transmitting well-name */
    Windows::Foundation::EventRegistrationToken m_token;          /**< The token used to remove ConnectionRequested event handler */

    Callback<void, const qcc::String&, const qcc::String&, std::vector<qcc::String>&, uint8_t>* m_callback;      /**< The callback to notify the proximity transport about the found name */
    bool m_peerFinderStarted;                                     /**< Whether PeerFinder::Start() is called */
    qcc::String m_namePrefix;                                     /**< The name prefix the daemon tried to discover */
    std::set<qcc::String> m_advertised;                           /**< The well-known names the daemon advertised */
    qcc::String m_sguid;                                          /**< The daemon GUID short string, 8-byte */

    std::map<qcc::String, Windows::Networking::Proximity::PeerInformation ^> m_peersMap;     /**< Mapping guid to PeerInformation, used for establishing P2P connection to a remote peer*/
    bool m_doDiscovery;                                           /**< Whether PeerFinder should browse peers to discover well-known name */
    qcc::Mutex m_mutex;
    uint16_t m_port;                                              /**< The port associated with the name service */
    uint32_t m_tDuration;                                         /**< The lifetime of a found advertised well-known nmae */
    volatile mutable int32_t m_connRefCount;                      /**< Number of overlay TCP connections that depend on current proximity connection */
    std::list<ProximityListener*> m_listeners;                    /**< List of ProximityListeners */
    CurrentP2PConnection m_currentP2PLink;
};

} // namespace ajn

#endif //_ALLJOYN_PROXIMITYNAMESERVICE_H
