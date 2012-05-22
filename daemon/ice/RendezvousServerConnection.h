/**
 * @file RendezvousServerConnection.h
 *
 * This file defines a class that handles the connection with the Rendezvous Server.
 *
 */

/******************************************************************************
 * Copyright 2012 Qualcomm Innovation Center, Inc.
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

#ifndef _RENDEZVOUSSERVERCONNECTION_H
#define _RENDEZVOUSSERVERCONNECTION_H

#include <qcc/platform.h>
#include <qcc/Socket.h>
#include <qcc/SocketTypes.h>
#include <qcc/String.h>
#include <qcc/Event.h>

#include "HttpConnection.h"
#include "NetworkInterface.h"

using namespace std;
using namespace qcc;

namespace ajn {
/*This class handles the connection with the Rendezvous Server.*/
class RendezvousServerConnection {

  public:

    /**
     * @internal
     * @brief Enum specifying the connections that need to be
     * established with the Rendezvous Server.
     */
    typedef enum _ConnectionFlag {

        /* Do not establish any connection */
        NONE = 0,

        /* Establish only the on demand connection */
        ON_DEMAND_CONNECTION,

        /* Establish only the persistent connection */
        PERSISTENT_CONNECTION,

        /* Establish both the connections */
        BOTH

    } ConnectionFlag;

    /**
     * @internal
     * @brief Class specifying the properties used to describe an interface.
     */
    class ConnInterface {
      public:
        qcc::IfConfigEntry interfaceProperties; /* Interface specific properties*/
        SocketFd sockFd; /**< The socket we are using for connection to the Rendezvous Server */

        void Clear() {
            interfaceProperties.m_name = String();
            interfaceProperties.m_addr = String();
            interfaceProperties.m_family = QCC_AF_UNSPEC;
            interfaceProperties.m_flags = 0;
            interfaceProperties.m_index = 0;
            interfaceProperties.m_mtu = 0;
            sockFd = -1;
        }
    };

    /**
     * @internal
     * @brief Constructor.
     */
    RendezvousServerConnection(String rdvzServer, bool enableIPv6, bool useHttp);

    /**
     * @internal
     * @brief Destructor.
     */
    ~RendezvousServerConnection();

    /**
     * @internal
     * @brief Connect to the Rendezvous Server after gathering the
     * latest interface details.
     *
     * @param connFlag - Flag specifying the type of connection to be set up
     * @param interfaceFlags - Interface flags specifying the permissible interface types for the connection
     *
     * @return ER_OK or ER_FAIL
     */
    QStatus Connect(uint8_t interfaceFlags, ConnectionFlag connFlag);

    /**
     * @internal
     * @brief Disconnect from the Rendezvous Server
     *
     * @return None
     */
    void Disconnect(void);

    /**
     * @internal
     * @brief Returns if the interface is still live
     *
     * @return None
     */
    bool IsInterfaceLive(ConnInterface interface);

    /**
     * @internal
     * @brief Connect to the Rendezvous Server over any available live interface
     *
     * @return ER_OK or ER_FAIL
     */
    QStatus ConnectOverAnyInterface(ConnInterface* connInterface, SocketFd& sockFd, HttpConnection** httpConn);

    /**
     * @internal
     * @brief Update the connection details
     *
     * @return None
     */
    void UpdateConnectionDetails(SocketFd sockFd,
                                 HttpConnection** oldHttpConn, HttpConnection* newHttpConn,
                                 ConnInterface* oldConnInterface, ConnInterface* newConnInterface,
                                 bool* isConnected, bool* connectionChangedFlag);

    /**
     * @internal
     * @brief Clean up an HTTP connection
     *
     * @return None
     */
    void CleanConnection(HttpConnection* httpConn, ConnInterface* connInterface, bool* isConnected);

    /**
     * @internal
     * @brief Set up a HTTP connection over a specified interface
     *
     * @return ER_OK or ER_FAIL
     */
    QStatus SetupConnOverInterface(ConnInterface connInterface, SocketFd& sockFd,
                                   HttpConnection** httpConn);

    /**
     * @internal
     * @brief Set up a HTTP connection with the Rendezvous Server
     *
     * @return ER_OK or ER_FAIL
     */
    QStatus SetupHTTPConn(SocketFd sockFd, HttpConnection** httpConn, String localIPAddress);

    /**
     * @internal
     * @brief Set up a socket for HTTP connection with the Rendezvous Server
     *
     * @return ER_OK or ER_FAIL
     */
    QStatus SetupSockForConn(SocketFd& sockFd, ConnInterface connInterface);

    /**
     * @internal
     * @brief Set up a HTTP connection with the Rendezvous Server
     *
     * @param connFlag - Flag specifying the type of connection to be set up
     * @param useCurrentInterface - Boolean indicating is the interface type used for the other connection
     *
     * @return ER_OK or ER_FAIL
     */
    QStatus SetupConnection(ConnectionFlag connFlag, bool useConnectedInterface);

    /**
     * @internal
     * @brief Utility function to print the interface details.
     */
    String PrintConnInterface(ConnInterface interface);

    /**
     * @internal
     * @brief Utility function that checks if the two interface specs passed in are the same.
     */
    bool IsSameInterface(ConnInterface* interface, ConnInterface* otherInterface);

    /**
     * @internal
     * @brief Function indicating if the on demand connection is up with the
     * Rendezvous Server.
     */
    bool IsOnDemandConnUp() { return onDemandIsConnected; };

    /**
     * @internal
     * @brief Function indicating if the persistent connection is up with the
     * Rendezvous Server.
     */
    bool IsPersistentConnUp() { return persistentIsConnected; };

    /**
     * @internal
     * @brief Function indicating if either or both the persistent and on demand connections are up with the
     * Rendezvous Server.
     */
    bool IsConnectedToServer() { return (onDemandIsConnected | persistentIsConnected); };

    /**
     * @internal
     * @brief Send a message to the Server
     */
    QStatus SendMessage(bool sendOverPersistentConn, HttpConnection::Method httpMethod, String uri, bool payloadPresent, String payload);

    /**
     * @internal
     * @brief Receive a response from the Server
     */
    QStatus FetchResponse(bool isOnDemandConnection, HttpConnection::HTTPResponse& response);

    /**
     * @internal
     * @brief Reset the persistentConnectionChanged flag
     */
    void ResetPersistentConnectionChanged(void) { persistentConnectionChanged = false; };

    /**
     * @internal
     * @brief Reset the onDemandConnectionChanged flag
     */
    void ResetOnDemandConnectionChanged(void) { onDemandConnectionChanged = false; };

    /**
     * @internal
     * @brief Return the value of persistentConnectionChanged flag
     */
    bool GetPersistentConnectionChanged(void) { return persistentConnectionChanged; };

    /**
     * @internal
     * @brief Return the value of onDemandConnectionChanged flag
     */
    bool GetOnDemandConnectionChanged(void) { return onDemandConnectionChanged; };

    /**
     * @internal
     * @brief Return onDemandConn
     */
    HttpConnection* GetOnDemandConn() { return onDemandConn; };

    /**
     * @internal
     * @brief Return persistentConn
     */
    HttpConnection* GetPersistentConn() { return persistentConn; };

    /**
     * @internal
     * @brief Return onDemandConn
     */
    Event& GetOnDemandSourceEvent() { return onDemandConn->GetResponseSource().GetSourceEvent(); };

    /**
     * @internal
     * @brief Return persistentConn
     */
    Event& GetPersistentSourceEvent() { return persistentConn->GetResponseSource().GetSourceEvent(); };

    /**
     * @internal
     * @brief Return On Demand connection socket
     */
    SocketFd GetOnDemandConnSocket() { return onDemandInterface.sockFd; };

    /**
     * @internal
     * @brief Return Persistent connection socket
     */
    SocketFd GetPersistentConnSocket() { return persistentInterface.sockFd; };

  private:

    /**
     * @internal
     * @brief Boolean indicating if the on demand connection is up.
     */
    bool onDemandIsConnected;

    /**
     * @internal
     * @brief The HTTP connection that is used to send messages to the
     * Rendezvous Server.
     */
    HttpConnection* onDemandConn;

    /**
     * @internal
     * @brief Boolean indicating if the persistent connection is up.
     */
    bool persistentIsConnected;

    /**
     * @internal
     * @brief Boolean indicating if the persistent connection has changed from
     * what it was previously.
     */
    bool persistentConnectionChanged;

    /**
     * @internal
     * @brief Boolean indicating if the On Demand connection has changed from
     * what it was previously.
     */
    bool onDemandConnectionChanged;

    /**
     * @internal
     * @brief The HTTP connection that is used to send GET messages to the
     * Rendezvous Server and receive responses from the same.
     */
    HttpConnection* persistentConn;

    /**
     * @internal
     * @brief Interface object used to get the network information from the kernel.
     */
    NetworkInterface* networkInterface;

    /**
     * @internal
     * @brief Information about the interface that is currently being used for on demand connection with
     * the Rendezvous Server.
     */
    ConnInterface onDemandInterface;

    /**
     * @internal
     * @brief Information about the interface that is currently being used for persistent connection with
     * the Rendezvous Server.
     */
    ConnInterface persistentInterface;

    /**
     * @internal
     * @brief Rendezvous Server address.
     */
    String RendezvousServer;

    /**
     * @internal
     * @brief Boolean indicating if IPv6 addressing mode is supported.
     */
    bool EnableIPv6;

    /**
     * @internal
     * @brief Boolean indicating if HTTP protocol needs to be used for connection.
     */
    bool UseHTTP;
};

} //namespace ajn


#endif //_RENDEZVOUSSERVERCONNECTION_H
