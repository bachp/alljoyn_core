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

#include <qcc/IPAddress.h>
#include <qcc/platform.h>
#include <qcc/String.h>
#include <qcc/Debug.h>
#include <qcc/Thread.h>
#include <qcc/Util.h>
#include <qcc/winrt/utility.h>

#include "ProximityNameService.h"
#include "ppltasks.h"

#define QCC_MODULE "PROXIMITY_NAME_SERVICE"

using namespace std;
using namespace qcc;
using namespace Platform;
using namespace Windows::Networking::Proximity;
using namespace Windows::Networking::Sockets;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage::Streams;
using namespace Windows::System::Threading;

namespace ajn {

Platform::String ^ ProximityNameService::PROXIMITY_ALT_ID_ALLJOYN = L"alljoyn";

ProximityNameService::ProximityNameService(const qcc::String& guid) :
    m_requestingPeer(nullptr),
    m_socket(nullptr),
    m_dataReader(nullptr),
    m_dataWriter(nullptr),
    m_timer(nullptr),
    m_triggeredConnectSupported(false),
    m_browseConnectSupported(false),
    m_peerFinderStarted(false),
    m_locateStarted(false),
    m_doDiscovery(false),
    m_socketClosed(false),
    m_connectRetries(0),
    m_currentState(PROXIM_DISCONNECTED),
    m_port(0),
    m_tDuration(DEFAULT_DURATION),
    m_listenAddr(qcc::String::Empty),
    m_tcpConnCount(0)
{
    GUID128 id(guid);
    m_guid = id.ToString();
    m_sguid = id.ToShortString();
}

ProximityNameService::~ProximityNameService()
{
    QCC_DbgPrintf(("ProximityNameService::~ProximityNameService()"));
    Reset();
    if (m_timer != nullptr) {
        m_timer->Cancel();
        m_timer = nullptr;
    }
}

void ProximityNameService::SetCallback(Callback<void, const qcc::String&, const qcc::String&, vector<qcc::String>&, uint8_t>* cb)
{
    QCC_DbgPrintf(("ProximityNameService::SetCallback()"));

    delete m_callback;
    m_callback = cb;
}

bool ProximityNameService::ShouldDoDiscovery()
{
    if (m_namePrefixs.size() == 0) {
        return false;
    }
    if (m_namePrefixs.size() > 0 && m_advertised.size() == 0) {
        return true;
    }
    std::set<qcc::String>::iterator pIt =  m_namePrefixs.begin();
    for (; pIt != m_namePrefixs.end(); pIt++) {
        bool bMatched = false;
        std::set<qcc::String>::iterator mIt =  m_advertised.begin();
        for (; mIt != m_advertised.end(); mIt++) {
            if (mIt->size() < pIt->size()) {
                continue;
            }
            if (mIt->compare(0, pIt->size(), *pIt) == 0) {
                bMatched = true;
            }
        }
        if (!bMatched) {
            return true;
        }
    }
    return false;
}

void ProximityNameService::Start()
{
    QCC_DbgPrintf(("ProximityNameService::Start()"));
    Reset();
    Windows::Foundation::EventRegistrationToken m_token = PeerFinder::ConnectionRequested += ref new TypedEventHandler<Platform::Object ^, Windows::Networking::Proximity::ConnectionRequestedEventArgs ^>(this, \
                                                                                                                                                                                                           &ProximityNameService::ConnectionRequestedEventHandler, CallbackContext::Same);
}

void ProximityNameService::Stop()
{
    QCC_DbgPrintf(("ProximityNameService::Stop()"));
    if (m_tcpConnCount > 0) {
        NotifyDisconnected();
    }
    Reset();
    m_currentState = PROXIM_DISCONNECTED;

    PeerFinder::ConnectionRequested -= m_token;
}

void ProximityNameService::ConnectionRequestedEventHandler(Platform::Object ^ sender, ConnectionRequestedEventArgs ^ TriggeredConnectionStateChangedEventArgs)
{
    QCC_DbgPrintf(("ProximityNameService::ConnectionRequestedEventHandler() m_currentState(%d)", m_currentState));
    if (m_currentState == PROXIM_CONNECTING) {
        return;
    }
    m_requestingPeer = TriggeredConnectionStateChangedEventArgs->PeerInformation;
    auto op = PeerFinder::ConnectAsync(m_requestingPeer);

    Concurrency::task<StreamSocket ^> connectTask(op);
    connectTask.then([this](Concurrency::task<StreamSocket ^> resultTask) {
                         QStatus status = ER_OK;
                         try {
                             m_socket = resultTask.get();
                             m_socketClosed = false;
                             m_currentState = PROXIM_CONNECTED;
                             m_dataReader = ref new DataReader(m_socket->InputStream);
                             m_dataWriter = ref new DataWriter(m_socket->OutputStream);
                             StartReader();
                             qcc::String addrStr = PlatformToMultibyteString(m_socket->Information->LocalAddress->CanonicalName);
                             size_t pos = addrStr.find_first_of('%');
                             if (qcc::String::npos != pos) {
                                 addrStr = addrStr.substr(0, pos);
                             }
                             m_listenAddr = addrStr;
                             TransmitMyWKNs();
                             StartMaintainanceTimer();
                         } catch (Exception ^ e) {
                             status = ER_FAIL;
                             Restart();
                             m_currentState = PROXIM_DISCONNECTED;
                             qcc::String err = PlatformToMultibyteString(e->Message);
                             QCC_LogError(status, ("ConnectionRequestedEventHandler ConnectAsync() Error (%s)", err.c_str()));
                         }
                     });
}

bool ProximityNameService::IsTriggeredConnectSupported()
{
    return (PeerFinder::SupportedDiscoveryTypes & PeerDiscoveryTypes::Triggered) ==
           PeerDiscoveryTypes::Triggered;
}

bool ProximityNameService::IsBrowseConnectSupported()
{
    return (PeerFinder::SupportedDiscoveryTypes & PeerDiscoveryTypes::Browse) ==
           PeerDiscoveryTypes::Browse;
}

void ProximityNameService::EnableAdvertisement(const qcc::String& name)
{
    QCC_DbgPrintf(("ProximityNameService::EnableAdvertisement (%s)", name.c_str()));
    m_mutex.Lock(MUTEX_CONTEXT);
    try {
        if (IsBrowseConnectSupported()) {
            if (!PeerFinder::AlternateIdentities->HasKey(L"Browse")) {
                PeerFinder::AlternateIdentities->Insert(L"Browse", PROXIMITY_ALT_ID_ALLJOYN);
            }

            m_advertised.insert(name);
            m_doDiscovery = ShouldDoDiscovery();

            if (IsConnected()) {
                QCC_DbgPrintf(("EnableAdvertisement() already connected, TransmitMyWKNs Immidiately"));
                TransmitMyWKNs();
                m_mutex.Unlock(MUTEX_CONTEXT);
                return;
            }

            PeerFinder::Stop();
            PeerFinder::DisplayName = EncodeWknAdvertisement();
            Windows::Networking::Proximity::PeerFinder::Start();
            m_peerFinderStarted = true;
            QCC_DbgPrintf(("EnableAdvertisement Now DisplayName is (%s)", PlatformToMultibyteString(PeerFinder::DisplayName).c_str()));
        }
    } catch (Exception ^ e) {
        QCC_LogError(ER_FAIL, ("EnableAdvertisement() Error (%s)", PlatformToMultibyteString(e->Message).c_str()));
    }
    m_mutex.Unlock(MUTEX_CONTEXT);
}

void ProximityNameService::DisableAdvertisement(vector<qcc::String>& wkns)
{
    QCC_DbgPrintf(("ProximityNameService::DisableAdvertisement()"));
    m_mutex.Lock(MUTEX_CONTEXT);
    try {
        if (IsBrowseConnectSupported()) {
            Platform::String ^ updatedName;
            bool changed = false;
            for (uint32_t i = 0; i < wkns.size(); ++i) {
                set<qcc::String>::iterator j = m_advertised.find(wkns[i]);
                if (j != m_advertised.end()) {
                    m_advertised.erase(j);
                    changed = true;
                }
            }
            if (!changed) {
                m_mutex.Unlock(MUTEX_CONTEXT);
                return;
            }
            m_doDiscovery = ShouldDoDiscovery();
            if (IsConnected()) {
                QCC_DbgPrintf(("DisableAdvertisement() already connected, TransmitMyWKNs Immidiately"));
                TransmitMyWKNs();
                m_mutex.Unlock(MUTEX_CONTEXT);
                return;
            }
            if (m_advertised.size() == 0) {
                updatedName = L"NA";
            } else {
                updatedName = EncodeWknAdvertisement();
            }

            PeerFinder::Stop();
            PeerFinder::DisplayName = updatedName;
            PeerFinder::Start();
            m_peerFinderStarted = true;
        }
    } catch (Exception ^ e) {
        QCC_LogError(ER_FAIL, ("DisableAdvertisement() Error (%s)", PlatformToMultibyteString(e->Message).c_str()));
    }
    m_mutex.Unlock(MUTEX_CONTEXT);
}

void ProximityNameService::EnableDiscovery(const qcc::String& namePrefix)
{
    QCC_DbgPrintf(("ProximityNameService::EnableDiscovery (%s)", namePrefix.c_str()));
    m_mutex.Lock(MUTEX_CONTEXT);
    try {
        if (IsBrowseConnectSupported()) {
            qcc::String actualPrefix;
            if (namePrefix[namePrefix.size() - 1] == '*') {
                m_namePrefixs.insert(namePrefix.substr(0, namePrefix.size() - 1));
            } else {
                m_namePrefixs.insert(namePrefix);
            }

            m_doDiscovery = ShouldDoDiscovery();

            if (IsConnected()) {
                QCC_DbgPrintf(("EnableDiscovery() already connected, Locate() Immidiately"));
                Locate(namePrefix);
                m_mutex.Unlock(MUTEX_CONTEXT);
                return;
            }

            if (m_locateStarted) {
                m_mutex.Unlock(MUTEX_CONTEXT);
                return;
            } else {
                m_locateStarted = true;
            }

            if (!PeerFinder::AlternateIdentities->HasKey(L"Browse")) {
                PeerFinder::AlternateIdentities->Insert(L"Browse", PROXIMITY_ALT_ID_ALLJOYN);
            }
            if (!m_peerFinderStarted) {
                PeerFinder::DisplayName = L"NA";
                Windows::Networking::Proximity::PeerFinder::Start();
                m_peerFinderStarted = true;
            }
            if (m_doDiscovery && m_currentState == PROXIM_DISCONNECTED) {
                BrowsePeers();
            }
        }
    } catch (Exception ^ e) {
        QCC_LogError(ER_FAIL, ("EnableDiscovery() Error (%s)", PlatformToMultibyteString(e->Message).c_str()));
    }
    m_mutex.Unlock(MUTEX_CONTEXT);
}

void ProximityNameService::DisableDiscovery(const qcc::String& namePrefix)
{
    QCC_DbgPrintf(("ProximityNameService::DisableDiscovery (%s)", namePrefix.c_str()));
    m_mutex.Lock(MUTEX_CONTEXT);
    m_namePrefixs.erase(namePrefix);
    m_doDiscovery = ShouldDoDiscovery();
    m_mutex.Unlock(MUTEX_CONTEXT);
}

void ProximityNameService::BrowsePeers()
{
    QCC_DbgPrintf(("ProximityNameService::BrowsePeers()"));
    if (!IsBrowseConnectSupported()) {
        m_currentState = PROXIM_DISCONNECTED;
        return;
    }
    m_currentState = PROXIM_BROWSING;
    auto op = PeerFinder::FindAllPeersAsync();
    Concurrency::task<IVectorView<PeerInformation ^>^> findAllPeersTask(op);

    findAllPeersTask.then([this](Concurrency::task<IVectorView<PeerInformation ^>^> resultTask)
                          {
                              try{
                                  m_peerInformationList = resultTask.get();
                                  bool foundDesiredPeer = false;
                                  QCC_DbgPrintf(("m_peerInformationList size (%d)", m_peerInformationList->Size));
                                  if (m_peerInformationList->Size > 0) {
                                      unsigned int i = 0;
                                      for (; i < m_peerInformationList->Size; i++) {
                                          Platform::String ^ platStr = m_peerInformationList->GetAt(i)->DisplayName;
                                          qcc::String mbStr = PlatformToMultibyteString(platStr);
                                          QCC_DbgPrintf(("Peer (%d) DisplayName = (%s)", i, mbStr.c_str()));
                                          size_t startPos = 0;
                                          if (mbStr.compare("NA") == 0) {
                                              continue;
                                          }

                                          size_t pos = mbStr.find_first_of(';');
                                          if (pos == qcc::String::npos) {
                                              QCC_LogError(ER_OS_ERROR, ("; is expected in (%s)", mbStr.c_str()));
                                              continue;
                                          }
#ifdef ENCODE_SHORT_GUID
                                          QCC_DbgPrintf(("Parse short GUID string"));
                                          // short version, 48-bit (8 bytes)
                                          assert(pos == GUID128::SHORT_SIZE);
                                          qcc::String guidStr = mbStr.substr(0, GUID128::SHORT_SIZE);
                                          pos += GUID128::SHORT_SIZE;
                                          startPos = ++pos;
                                          pos = mbStr.find_first_of(';', startPos);
                                          if (pos == qcc::String::npos) {
                                              continue;
                                          }

#endif
                                          // decode the count of well-known names
                                          qcc::String countStr = mbStr.substr(startPos, (pos - startPos));
                                          size_t count = atoi(countStr.c_str());
                                          QCC_DbgPrintf(("The peer has (%d) well-known names", count));

                                          std::vector<qcc::String> nameList;
                                          while (true) {
                                              startPos = ++pos;
                                              pos = mbStr.find_first_of(';', startPos);
                                              if (pos != qcc::String::npos) {
                                                  qcc::String wkn = mbStr.substr(startPos, (pos - startPos));
                                                  nameList.push_back(wkn);
                                                  count--;
                                                  if (MatchNamePrefix(wkn)) {
                                                      foundDesiredPeer = true;
                                                      break;
                                                  }
                                              } else {
                                                  break;
                                              }
                                          }

                                          if (foundDesiredPeer) {
                                              break;
                                          }
                                      }

                                      PeerInformation ^ peerInfo = nullptr;
                                      if (foundDesiredPeer) {
                                          peerInfo = m_peerInformationList->GetAt(i);
                                          QCC_DbgPrintf(("Connecting to Peer (%d) foundDesiredPeer(%d)", i, foundDesiredPeer));


                                          for (m_connectRetries = 0; m_connectRetries < MAX_CONNECT_RETRY; m_connectRetries++) {
                                              if (Connect(peerInfo) == ER_OK) {
                                                  // if connected successfully, then stop browsing peers
                                                  return;
                                              }
                                          }
                                          // There must be something wrong happens, then restart and continue browse peers.
                                          QCC_LogError(ER_OS_ERROR, ("Connect to Peer fail DisplayName(%s)", PlatformToMultibyteString(peerInfo->DisplayName).c_str()));
                                          Restart();
                                          m_currentState = PROXIM_DISCONNECTED;
                                      }
                                  }
                              } catch (Exception ^ e) {
                                  Restart();
                                  qcc::String err = PlatformToMultibyteString(e->Message);
                                  QCC_LogError(ER_OS_ERROR, ("Exception (%s) occurred while finding peer", err.c_str()));
                              }

                              if (m_doDiscovery && ((m_currentState == PROXIM_DISCONNECTED) || (m_currentState == PROXIM_BROWSING))) {
                                  qcc::Sleep(100 + qcc::Rand16() % 512);
                                  BrowsePeers();
                              }
                          });
}

QStatus ProximityNameService::Connect(PeerInformation ^ peerInfo)
{
    QCC_DbgPrintf(("ProximityNameService::Connect()"));
    QStatus status = ER_OK;
    if (peerInfo == nullptr) {
        status = ER_FAIL;
        QCC_LogError(status, ("PeerInformation is nullptr"));
        return status;
    }
    m_currentState = PROXIM_CONNECTING;
    try {
        auto op = PeerFinder::ConnectAsync(peerInfo);
        Concurrency::task<StreamSocket ^> connectTask(op);
        connectTask.wait();
        m_socket = connectTask.get();
    } catch (Exception ^ e) {
        status = ER_FAIL;
        Restart();
        qcc::String err = PlatformToMultibyteString(e->Message);
        QCC_LogError(status, ("ProximityNameService::Connect Error (%s)", err.c_str()));
        return status;
    }
    if (status == ER_OK) {
        m_socketClosed = false;
        m_currentState = PROXIM_CONNECTED;
        m_dataReader = ref new DataReader(m_socket->InputStream);
        m_dataWriter = ref new DataWriter(m_socket->OutputStream);
        StartReader();
        qcc::String addrStr = PlatformToMultibyteString(m_socket->Information->LocalAddress->CanonicalName);
        size_t pos = addrStr.find_first_of('%');
        if (qcc::String::npos != pos) {
            addrStr = addrStr.substr(0, pos);
        }
        m_listenAddr = addrStr;
        TransmitMyWKNs();
        StartMaintainanceTimer();
    }

    return status;
}

void ProximityNameService::Restart()
{
    Reset();
    PeerFinder::Start();
    m_peerFinderStarted = true;
}

void ProximityNameService::Reset()
{
    QCC_DbgPrintf(("ProximityNameService::Reset()"));
    if (m_peerFinderStarted) {
        PeerFinder::Stop();
        m_peerFinderStarted = false;
        if (m_socket != nullptr) {
            m_socketClosed = true;
            delete m_socket;
            m_socket = nullptr;
            m_dataReader = nullptr;
            m_dataWriter = nullptr;
            m_listenAddr = qcc::String::Empty;
        }
    }
}

Platform::String ^ ProximityNameService::EncodeWknAdvertisement()
{
    qcc::String encodedStr;
#ifdef ENCODE_SHORT_GUID
    QCC_DbgPrintf(("Encode short GUID string"));
    encodedStr.append(m_sguid);
    encodedStr.append(";");
#endif
    // encode the number of local well-known names
    uint8_t nWkns = m_advertised.size();
    char buf[12];
    snprintf(buf, 12, "%d;", nWkns);
    encodedStr.append(buf);

    assert(encodedStr.size() <= MAX_DISPLAYNAME_SIZE);
    std::set<qcc::String>::iterator it = m_advertised.begin();
    while (it != m_advertised.end()) {
        if ((encodedStr.size() + it->size() + 1) <= MAX_DISPLAYNAME_SIZE) {
            encodedStr.append(*it);
            encodedStr.append(";");
            it++;
        } else {
            break;
        }
    }
    Platform::String ^ platformStr = MultibyteToPlatformString(encodedStr.c_str());
    return platformStr;
}

bool ProximityNameService::MatchNamePrefix(qcc::String wkn)
{
    bool result = false;
    std::set<qcc::String>::iterator it = m_namePrefixs.begin();
    for (; it != m_namePrefixs.end(); it++) {
        if (wkn.compare(0, it->size(), *it) == 0) {
            result = true;
            break;
        }
    }
    return result;
}

void ProximityNameService::StartMaintainanceTimer()
{
    QCC_DbgPrintf(("ProximityNameService::StartMaintainanceTimer(interval = %d)", TRANSMIT_INTERVAL));
    #define HUNDRED_NANOSECONDS_PER_MILLISECOND 10000
    Windows::Foundation::TimeSpan ts = { TRANSMIT_INTERVAL* HUNDRED_NANOSECONDS_PER_MILLISECOND };
    m_timer = ThreadPoolTimer::CreatePeriodicTimer(ref new TimerElapsedHandler([&] (ThreadPoolTimer ^ timer) {
                                                                                   TimerCallback(timer);
                                                                               }), ts);
    assert(m_timer != nullptr);
}

void ProximityNameService::TimerCallback(Windows::System::Threading::ThreadPoolTimer ^ timer)
{
    TransmitMyWKNs();
}

void ProximityNameService::StartReader()
{
    QCC_DbgPrintf(("ProximityNameService::StartReader()"));
    Concurrency::task<unsigned int> loadTask(m_dataReader->LoadAsync(sizeof(unsigned int)));
    loadTask.then([this](Concurrency::task<unsigned int> stringBytesTask)
                  {
                      try{
                          unsigned int bytesRead = stringBytesTask.get();
                          if (bytesRead > 0) {
                              unsigned int nbytes = (unsigned int)m_dataReader->ReadUInt32();
                              Concurrency::task<unsigned int> loadStringTask(m_dataReader->LoadAsync(nbytes));
                              loadStringTask.then([this, nbytes](Concurrency::task<unsigned int> resultTask) {
                                                      try{
                                                          unsigned int bytesRead = resultTask.get();
                                                          if (bytesRead > 0) {
                                                              Platform::Array<unsigned char> ^ buffer = ref new Platform::Array<unsigned char>(nbytes);
                                                              m_dataReader->ReadBytes(buffer);
                                                              qcc::String addrStr = PlatformToMultibyteString(m_socket->Information->RemoteAddress->CanonicalName);
                                                              size_t pos = addrStr.find_first_of('%');
                                                              if (qcc::String::npos != pos) {
                                                                  addrStr = addrStr.substr(0, pos);
                                                              }
                                                              qcc::IPAddress address(addrStr);
                                                              HandleProtocolMessage(buffer->Data, nbytes, address);
                                                              StartReader();
                                                          } else {
                                                              qcc::String err = "The remote side closed the socket";
                                                              SocketError(err);
                                                          }
                                                      }catch (Exception ^ e) {
                                                          if (!m_socketClosed) {
                                                              qcc::String err = "Failed to read from socket: ";
                                                              err += PlatformToMultibyteString(e->Message).c_str();
                                                              SocketError(err);
                                                          }
                                                      }
                                                  });
                          } else {
                              qcc::String err = "The remote side closed the socket";
                              SocketError(err);
                          }
                      }catch (Exception ^ e) {
                          if (!m_socketClosed) {
                              qcc::String err = "Failed to read from socket: ";
                              err += PlatformToMultibyteString(e->Message).c_str();
                              SocketError(err);
                          }
                      }
                  });
}

void ProximityNameService::SocketError(qcc::String& errMsg)
{
    QCC_LogError(ER_FAIL, ("ProximityNameService::SocketError (%s)", errMsg.c_str()));
    if (m_tcpConnCount > 0) {
        NotifyDisconnected();
    }

    if (!m_socketClosed) {
        m_socketClosed = true;
        delete m_socket;
        m_socket = nullptr;
        m_listenAddr = qcc::String::Empty;
        m_currentState = PROXIM_DISCONNECTED;
    }

    // TODO Should re-browser peers for connection?
}

QStatus ProximityNameService::GetEndpoints(qcc::String& ipv6address, uint16_t& port)
{
    QCC_DbgPrintf(("ProximityNameService::GetEndpoints()"));
    QStatus status = ER_OK;
    if (!m_listenAddr.empty()) {
        ipv6address = m_listenAddr;
        port = m_port;
    } else {
        status = ER_FAIL;
        QCC_LogError(status, ("The listen address is empy"));
    }
    return status;
}

void ProximityNameService::SetEndpoints(const qcc::String& ipv6address, const uint16_t port)
{
    QCC_DbgPrintf(("ProximityNameService::SetEndpoints(port(%d))", port));
    m_port = port;
}

int32_t ProximityNameService::IncreaseOverlayTCPConnection()
{
    ++m_tcpConnCount;
    QCC_DbgPrintf(("ProximityNameService::IncreaseOverlayTCPConnection(%d)", m_tcpConnCount));
    return m_tcpConnCount;
}

int32_t ProximityNameService::DecreaseOverlayTCPConnection() {
    --m_tcpConnCount;
    QCC_DbgPrintf(("ProximityNameService::DecreaseOverlayTCPConnection(%d)", m_tcpConnCount));
    return m_tcpConnCount;
}

void ProximityNameService::RegisterProximityListener(ProximityListener* listener)
{
    QCC_DbgPrintf(("ProximityNameService::RegisterProximityListener(%p)", listener));
    m_listeners.push_back(listener);
}

void ProximityNameService::UnRegisterProximityListener(ProximityListener* listener)
{
    QCC_DbgPrintf(("ProximityNameService::UnRegisterProximityListener(%p)", listener));
    m_listeners.remove(listener);
}

void ProximityNameService::NotifyDisconnected()
{
    QCC_DbgPrintf(("ProximityNameService::NotifyDisconnected()"));
    std::list<ProximityListener*>::iterator it = m_listeners.begin();
    for (; it != m_listeners.end(); it++) {
        (*it)->OnProximityDisconnected();
    }
}

extern bool WildcardMatch(qcc::String str, qcc::String pat);

void ProximityNameService::Locate(const qcc::String& namePrefix)
{
    QCC_DbgHLPrintf(("ProximityNameService::Locate(): %s", namePrefix.c_str()));

    //
    // Send a request to the network over our multicast channel,
    // asking for anyone who supports the specified well-known name.
    //
    WhoHas whoHas;
    whoHas.SetTcpFlag(true);
    whoHas.SetIPv6Flag(true);
    whoHas.AddName(namePrefix);

    Header header;
    header.SetVersion(0);
    header.SetTimer(m_tDuration);
    header.AddQuestion(whoHas);

    // Send the message out over the proximity link.
    //
    SendProtocolMessage(header);
}

void ProximityNameService::TransmitMyWKNs(void)
{
    QCC_DbgPrintf(("ProximityNameService::TransmitMyWKNs() m_currentState(%d)", m_currentState));
    if (m_currentState != PROXIM_CONNECTED) {
        return;
    }
    //
    // We need a valid port before we send something out to the local subnet.
    // Note that this is the daemon contact port, not the name service port
    // to which we send advertisements.
    //
    if (m_port == 0) {
        QCC_DbgPrintf(("ProximityNameService::TransmitMyWKNs(): Port not set"));
        return;
    }

    //
    // There are at least two threads wandering through the advertised list.
    // We are running short on toes, so don't shoot any more off by not being
    // thread-unaware.
    //
    m_mutex.Lock();

    //
    // The underlying protocol is capable of identifying both TCP and UDP
    // services.  Right now, the only possibility is TCP.
    //
    IsAt isAt;
    isAt.SetTcpFlag(true);
    isAt.SetUdpFlag(false);

    //
    // Always send the provided daemon GUID out with the reponse.
    //
    isAt.SetGuid(m_guid);

    //
    // Send a protocol message describing the entire (complete) list of names
    // we have.
    //
    isAt.SetCompleteFlag(true);

    isAt.SetPort(m_port);

    //
    // Add all of our adversised names to the protocol answer message.
    //
    for (set<qcc::String>::iterator i = m_advertised.begin(); i != m_advertised.end(); ++i) {
        isAt.AddName(*i);
    }
    m_mutex.Unlock();

    //
    // The header ties the whole protocol message together.  By setting the
    // timer, we are asking for everyone who hears the message to remember
    // the advertisements for that number of seconds.
    //
    Header header;
    header.SetVersion(0);
    header.SetTimer(m_tDuration);
    header.AddAnswer(isAt);

    //
    // Send the message out over the proximity link.
    //
    SendProtocolMessage(header);
}

void ProximityNameService::SendProtocolMessage(Header& header)
{
    QCC_DbgPrintf(("ProximityNameService::SendProtocolMessage()"));
    if (!m_socketClosed && m_socket != nullptr && m_dataWriter != nullptr) {
        size_t size = header.GetSerializedSize();
        uint8_t* buffer = new uint8_t[size];
        header.Serialize(buffer);
        Platform::Array<unsigned char>^ byteArry = ref new Platform::Array<unsigned char>(buffer, size);
        m_dataWriter->WriteUInt32(size);
        m_dataWriter->WriteBytes(byteArry);

        concurrency::task<unsigned int> storeTask(m_dataWriter->StoreAsync());
        storeTask.then([this](concurrency::task<unsigned int> resultTask)
                       {
                           try {
                               unsigned int nBytesWritten = resultTask.get();
                               if (nBytesWritten == 0) {
                                   qcc::String errMsg = "The remote side closed the socket";
                                   SocketError(errMsg);
                               }
                           } catch (Platform::Exception ^ e) {
                               qcc::String errMsg = "Fail to send message with Error";
                               errMsg += PlatformToMultibyteString(e->Message);
                               SocketError(errMsg);
                           }
                       });
    } else {
        QCC_DbgPrintf(("ProximityNameService::SendProtocolMessage m_socketClosed(%d) m_socket(%p) m_dataWriter(%p)", m_socketClosed, m_socket, m_dataWriter));
    }
}

void ProximityNameService::HandleProtocolQuestion(WhoHas whoHas, qcc::IPAddress address)
{
    QCC_DbgHLPrintf(("ProximityNameService::HandleProtocolQuestion()"));

    //
    // There are at least two threads wandering through the advertised list.
    // We are running short on toes, so don't shoot any more off by not being
    // thread-unaware.
    //
    m_mutex.Lock();

    //
    // Loop through the names we are being asked about, and if we have
    // advertised any of them, we are going to need to respond to this
    // question.
    //
    bool respond = false;
    for (uint32_t i = 0; i < whoHas.GetNumberNames(); ++i) {
        qcc::String wkn = whoHas.GetName(i);

        //
        // Zero length strings are unmatchable.  If you want to do a wildcard
        // match, you've got to send a wildcard character.
        //
        if (wkn.size() == 0) {
            continue;
        }

        //
        // check to see if this name on the list of names we advertise.
        //
        for (set<qcc::String>::iterator j = m_advertised.begin(); j != m_advertised.end(); ++j) {

            //
            // The requested name comes in from the WhoHas message and we
            // allow wildcards there.
            //
            if (WildcardMatch((*j), wkn)) {
                QCC_DbgHLPrintf(("ProximityNameService::HandleProtocolQuestion(): request for %s does not match my %s",
                                 wkn.c_str(), (*j).c_str()));
                continue;
            } else {
                respond = true;
                break;
            }
        }

        //
        // If we find a match, don't bother going any further since we need
        // to respond in any case.
        //
        if (respond) {
            break;
        }
    }

    m_mutex.Unlock();

    //
    // Since any response we send must include all of the advertisements we
    // are exporting; this just means to TransmitMyWKNs all of our advertisements.
    //
    if (respond) {
        TransmitMyWKNs();
    }
}

void ProximityNameService::HandleProtocolAnswer(IsAt isAt, uint32_t timer, qcc::IPAddress address)
{
    QCC_DbgHLPrintf(("ProximityNameService::HandleProtocolAnswer()"));

    //
    // If there are no callbacks we can't tell the user anything about what is
    // going on the net, so it's pointless to go any further.
    //

    if (m_callback == 0) {
        QCC_DbgHLPrintf(("ProximityNameService::HandleProtocolAnswer(): No callback, so nothing to do"));
        return;
    }

    vector<qcc::String> wkn;

    for (uint8_t i = 0; i < isAt.GetNumberNames(); ++i) {
        QCC_DbgHLPrintf(("ProximityNameService::HandleProtocolAnswer(): Got well-known name %s", isAt.GetName(i).c_str()));
        wkn.push_back(isAt.GetName(i));
    }

    //
    // Life is easier if we keep these things sorted.  Don't rely on the source
    // (even though it is really us) to do so.
    //
    sort(wkn.begin(), wkn.end());

    qcc::String guid = isAt.GetGuid();
    QCC_DbgHLPrintf(("ProximityNameService::HandleProtocolAnswer(): Got GUID %s", guid.c_str()));

    //
    // We always get an address since we got the message over a call to
    // recvfrom().  This will either be an IPv4 or an IPv6 address.  We can
    // also get an IPv4 and/or an IPv6 address in the answer message itself.
    // We have from one to three addresses of different flavors that we need
    // to communicate back to the daemon.  It is convenient for the daemon
    // to get these addresses in the form of a "listen-spec".  These look like,
    // "tcp:addr=x, port=y".  The daemon is going to keep track of unique
    // combinations of these and must be able to handle multiple identical
    // reports since we will be getting keepalives.  What we need to do then
    // is to send a callback with a listen-spec for every address we find.
    // If we get all three addresses, we'll do three callbacks with different
    // listen-specs.
    //
    qcc::String recvfromAddress, ipv4address, ipv6address;

    recvfromAddress = address.ToString();
    QCC_DbgHLPrintf(("ProximityNameService::HandleProtocolAnswer(): Got IP %s from protocol", recvfromAddress.c_str()));

    if (isAt.GetIPv4Flag()) {
        ipv4address = isAt.GetIPv4();
        QCC_DbgHLPrintf(("ProximityNameService::HandleProtocolAnswer(): Got IPv4 %s from message", ipv4address.c_str()));
    }

    if (isAt.GetIPv6Flag()) {
        ipv6address = isAt.GetIPv6();
        QCC_DbgHLPrintf(("ProximityNameService::HandleProtocolAnswer(): Got IPv6 %s from message", ipv6address.c_str()));
    }

    uint16_t port = isAt.GetPort();
    QCC_DbgHLPrintf(("ProximityNameService::HandleProtocolAnswer(): Got port %d from message", port));

    //
    // The longest bus address we can generate is going to be the larger
    // of an IPv4 or IPv6 address:
    //
    // "tcp:addr=255.255.255.255,port=65535"
    // "tcp:addr=ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff,port=65535"
    //
    // or 60 characters long including the trailing '\0'
    //
    char addrbuf[70];

    //
    // Call back with the address we got via recvfrom unless it is overridden by the address in the
    // message. An ipv4 address in the message overrides an ipv4 recvfrom address, an ipv6 address in
    // the message overrides an ipv6 recvfrom address.
    //
    if ((address.IsIPv4() && !ipv4address.size()) || (address.IsIPv6() && !ipv6address.size())) {
        snprintf(addrbuf, sizeof(addrbuf), "proximity:addr=%s,port=%d", recvfromAddress.c_str(), port);
        QCC_DbgHLPrintf(("ProximityNameService::HandleProtocolAnswer(): Calling back with %s", addrbuf));
        qcc::String busAddress(addrbuf);

        if (m_callback) {
            (*m_callback)(busAddress, guid, wkn, timer);
        }
    }

    //
    // If we received an IPv4 address in the message, call back with that one.
    //
    if (ipv4address.size()) {
        snprintf(addrbuf, sizeof(addrbuf), "proximity:addr=%s,port=%d", ipv4address.c_str(), port);
        QCC_DbgHLPrintf(("ProximityNameService::HandleProtocolAnswer(): Calling back with %s", addrbuf));
        qcc::String busAddress(addrbuf);

        if (m_callback) {
            (*m_callback)(busAddress, guid, wkn, timer);
        }
    }

    //
    // If we received an IPv6 address in the message, call back with that one.
    //
    if (ipv6address.size()) {
        snprintf(addrbuf, sizeof(addrbuf), "proximity:addr=%s,port=%d", ipv6address.c_str(), port);
        QCC_DbgHLPrintf(("ProximityNameService::HandleProtocolAnswer(): Calling back with %s", addrbuf));
        qcc::String busAddress(addrbuf);

        if (m_callback) {
            (*m_callback)(busAddress, guid, wkn, timer);
        }
    }
}

void ProximityNameService::HandleProtocolMessage(uint8_t const* buffer, uint32_t nbytes, qcc::IPAddress address)
{
    QCC_DbgHLPrintf(("ProximityNameService::HandleProtocolMessage(0x%x, %d, %s)", buffer, nbytes, address.ToString().c_str()));

    Header header;
    size_t bytesRead = header.Deserialize(buffer, nbytes);
    if (bytesRead != nbytes) {
        QCC_DbgPrintf(("ProximityNameService::HandleProtocolMessage(): Deserialize(): Error"));
        return;
    }

    //
    // We only understand version zero packets for now.
    //
    if (header.GetVersion() != 0) {
        QCC_DbgPrintf(("ProximityNameService::HandleProtocolMessage(): Unknown version: Error"));
        return;
    }

    //
    // If the received packet contains questions, see if we can answer them.
    // We have the underlying device in loopback mode so we can get receive
    // our own questions.  We usually don't have an answer and so we don't
    // reply, but if we do have the requested names, we answer ourselves
    // to pass on this information to other interested bystanders.
    //
    for (uint8_t i = 0; i < header.GetNumberQuestions(); ++i) {
        HandleProtocolQuestion(header.GetQuestion(i), address);
    }

    //
    // If the received packet contains answers, see if they are answers to
    // questions we think are interesting.  Make sure we are not talking to
    // ourselves unless we are told to for debugging purposes
    //
    for (uint8_t i = 0; i < header.GetNumberAnswers(); ++i) {
        IsAt isAt = header.GetAnswer(i);
        HandleProtocolAnswer(isAt, header.GetTimer(), address);
    }
}
} // namespace ajn

