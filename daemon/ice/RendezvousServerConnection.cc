/**
 * @file RendezvousServerConnection.cc
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

#include <qcc/platform.h>
#include <qcc/Debug.h>
#include <qcc/Event.h>
#include <qcc/Socket.h>
#include <qcc/SocketTypes.h>
#include <qcc/time.h>
#include <qcc/Stream.h>
#include <qcc/Socket.h>
#include <qcc/SocketStream.h>
#include <qcc/StringSource.h>
#include <qcc/StringSink.h>
#include <qcc/BufferedSource.h>
#include <qcc/Crypto.h>

#if defined(QCC_OS_GROUP_POSIX)
#include <errno.h>
#endif

#include "RendezvousServerConnection.h"

using namespace std;
using namespace qcc;

#define QCC_MODULE "RENDEZVOUS_SERVER_CONNECTION"

namespace ajn {

RendezvousServerConnection::RendezvousServerConnection(String rdvzServer, bool enableIPv6, bool useHttp) :
    onDemandIsConnected(false),
    onDemandConn(NULL),
    persistentIsConnected(false),
    persistentConnectionChanged(false),
    onDemandConnectionChanged(false),
    persistentConn(NULL),
    networkInterface(NULL),
    RendezvousServer(rdvzServer),
    EnableIPv6(enableIPv6),
    UseHTTP(useHttp)
{
    QCC_DbgPrintf(("RendezvousServerConnection::RendezvousServerConnection()\n"));

    /* Instantiate a NetworkInterface object */
    networkInterface = new NetworkInterface(EnableIPv6);

    /* Clear the onDemandInterface and persistentInterface to a known state */
    onDemandInterface.Clear();
    persistentInterface.Clear();
}

RendezvousServerConnection::~RendezvousServerConnection()
{
    QCC_DbgPrintf(("RendezvousServerConnection::~RendezvousServerConnection()\n"));

    /* Disconnect any existing connections */
    Disconnect();

    /* Clear the networkInterface */
    delete networkInterface;
}

QStatus RendezvousServerConnection::Connect(uint8_t interfaceFlags, ConnectionFlag connFlag)
{
    QStatus status = ER_OK;

    /* Return ER_FAIL if the interface flags have been specified to be NONE. We would normally not
     * hit this condition as the Discovery Manager would check that the flags are not NONE before
     * calling this function */
    if (interfaceFlags == NetworkInterface::NONE) {
        status = ER_FAIL;
        QCC_LogError(status, ("RendezvousServerConnection::Connect(): interfaceFlage = NONE"));
        return status;
    }

    /* Return ER_FAIL if the connection flag has been specified to be NONE. We would normally not
     * hit this condition as the Discovery Manager would ensure that the flag is not NONE before
     * calling this function */
    if (connFlag == NONE) {
        status = ER_FAIL;
        QCC_LogError(status, ("RendezvousServerConnection::Connect(): connFlag = NONE"));
        return status;
    }

    /* Update the interfaces */
    networkInterface->UpdateNetworkInterfaces();

    /* Ensure that live interfaces are available before proceeding further */
    if (!networkInterface->IsAnyNetworkInterfaceUp()) {
        status = ER_FAIL;
        QCC_LogError(status, ("RendezvousServerConnection::Connect(): None of the interfaces are up\n"));
        return status;
    }

    QCC_DbgPrintf(("RendezvousServerConnection::Connect(): IsPersistentConnUp() = %d IsOnDemandConnUp() = %d",
                   IsPersistentConnUp(), IsOnDemandConnUp()));

    /* Reconfigure or Set up the requested connections */
    if (connFlag == BOTH) {
        status = SetupConnection(ON_DEMAND_CONNECTION, false);

        if (status != ER_OK) {
            QCC_LogError(status, ("RendezvousServerConnection::Connect(): Unable to setup the on demand connection with the Rendezvous Server\n"));
            return status;
        } else {
            /* We set this flag to true to tell SetupConnection that the we have already chosen the appropriate
             * interface when setting up the on demand connection and that we need not do the same exercise
             * when setting up the persistent connection */
            status = SetupConnection(PERSISTENT_CONNECTION, true);

            if (status != ER_OK) {
                QCC_LogError(status, ("RendezvousServerConnection::Connect(): Unable to setup the persistent connection with the Rendezvous Server\n"));

                /* Disconnect the on demand connection that we just set up */
                Disconnect();

                return status;
            }
        }

    } else if (connFlag == ON_DEMAND_CONNECTION) {
        status = SetupConnection(ON_DEMAND_CONNECTION, false);

        if (status != ER_OK) {
            QCC_LogError(status, ("RendezvousServerConnection::Connect(): Unable to setup the on demand connection with the Rendezvous Server\n"));

            /* Disconnect the persistent connection if it is up */
            Disconnect();

            return status;
        } else {
            /* If the persistent connection is up, ensure that it is on the same interface as the on demand connection */
            if (IsPersistentConnUp()) {
                status = SetupConnection(PERSISTENT_CONNECTION, true);

                if (status != ER_OK) {
                    QCC_LogError(status, ("RendezvousServerConnection::Connect(): Unable to reconfigure the persistent connection with the Rendezvous Server\n"));

                    /* Disconnect the on demand connection */
                    Disconnect();

                    return status;
                }
            }
        }

    } else if (connFlag == PERSISTENT_CONNECTION) {

        status = SetupConnection(PERSISTENT_CONNECTION, false);

        if (status != ER_OK) {
            QCC_LogError(status, ("RendezvousServerConnection::Connect(): Unable to setup the persistent connection with the Rendezvous Server\n"));

            /* Disconnect the persistent connection if it is up */
            Disconnect();

            return status;
        } else {
            /* If the on demand connection is up, ensure that it is on the same interface as the persistent connection */
            if (IsOnDemandConnUp()) {
                status = SetupConnection(ON_DEMAND_CONNECTION, true);

                if (status != ER_OK) {
                    QCC_LogError(status, ("RendezvousServerConnection::Connect(): Unable to reconfigure the on demand connection with the Rendezvous Server\n"));

                    /* Disconnect the persistent connection */
                    Disconnect();

                    return status;
                }
            }
        }
    }

    return status;
}

QStatus RendezvousServerConnection::SetupConnection(ConnectionFlag connFlag, bool useConnectedInterface)
{
    QStatus status = ER_OK;

    HttpConnection** httpConn = NULL;
    ConnInterface* connInterface = NULL;
    bool* isConnected = NULL;
    bool* connectionChanged = NULL;
    ConnInterface* otherConnInterface = NULL;
    String connType;

    if (connFlag == PERSISTENT_CONNECTION) {

        httpConn = &persistentConn;
        connInterface = &persistentInterface;
        isConnected = &persistentIsConnected;
        otherConnInterface = &onDemandInterface;
        connectionChanged = &persistentConnectionChanged;
        connType = String("Persistent Connection");

    } else if (connFlag == ON_DEMAND_CONNECTION) {

        httpConn = &onDemandConn;
        connInterface = &onDemandInterface;
        isConnected = &onDemandIsConnected;
        otherConnInterface = &persistentInterface;
        connectionChanged = &onDemandConnectionChanged;
        connType = String("On Demand Connection");

    }

    SocketFd sockFd;
    HttpConnection* newHttpConn = NULL;

    /* If useConnectedInterface==true, we just use the interface used for the other connection to set up
     * this connection */
    if (useConnectedInterface) {

        /* If the current connection type is already up with the Rendezvous Server, ensure that it is over the
         * same interface type over which we want it to be. If it is,then we do nothing but just return ER_OK */
        if ((isConnected) && (IsSameInterface(connInterface, otherConnInterface))) {
            QCC_DbgPrintf(("RendezvousServerConnection::SetupConnection(): %s is over the intended interface type. Nothing to be done", connType.c_str()));
            return ER_OK;
        }

        /* Set up a new connection over the specified interface */
        status = SetupConnOverInterface(*otherConnInterface, sockFd, &newHttpConn);

        if (status != ER_OK) {
            QCC_LogError(status, ("RendezvousServerConnection::SetupConnection(): Unable to setup a HTTP connection over specified interface with the Server"));

            /* Disconnect all existing connections with the Rendezvous Server to bring the connection status to
             * a known state */
            Disconnect();

            return status;
        }

        /* Tear down the old connection if we were already connected */
        if (isConnected) {

            /* We do not check the return status here because we have already successfully set up a new
             * connection. Its ok if some cleanup has failed */
            CleanConnection(*httpConn, connInterface, isConnected);

        }

        /* Update the connection details in the status variables */
        UpdateConnectionDetails(sockFd, httpConn, newHttpConn, connInterface, otherConnInterface,
                                isConnected, connectionChanged);

    } else {

        if (isConnected) {
            if (IsInterfaceLive(*connInterface)) {
                QCC_DbgPrintf(("RendezvousServerConnection::SetupConnection(): Keeping the current connection with the Rendezvous Server"));
                return ER_OK;
            }
        }

        ConnInterface* newConnInterface = new ConnInterface();

        /* Sift through the available interface types to choose an
         * appropriate interface for connection with the Rendezvous Server*/
        status = ConnectOverAnyInterface(newConnInterface, sockFd, &newHttpConn);

        if (status == ER_OK) {
            /* Tear down the old connection if we were already connected */
            if (isConnected) {

                /* We do not check the return status here because we have already successfully set up a new
                 * connection. Its ok if some cleanup has failed */
                CleanConnection(*httpConn, connInterface, isConnected);

            }

            /* Update the connection details in the status variables */
            UpdateConnectionDetails(sockFd, httpConn, newHttpConn, connInterface, newConnInterface,
                                    isConnected, connectionChanged);

            QCC_DbgPrintf(("RendezvousServerConnection::SetupConnection(): Successfully set up a connection with the Rendezvous Server"));
        } else {
            QCC_LogError(status, ("RendezvousServerConnection::SetupConnection(): Unable to setup a connection with the Rendezvous Server"));
        }
    }

    return status;
}

void RendezvousServerConnection::Disconnect(void)
{
    /* Clean up the persistent connection */
    if (IsPersistentConnUp()) {
        CleanConnection(persistentConn, &persistentInterface, &persistentIsConnected);
    }

    /* Clean up the on demand connection */
    if (IsOnDemandConnUp()) {
        CleanConnection(onDemandConn, &onDemandInterface, &onDemandIsConnected);
    }
}

bool RendezvousServerConnection::IsInterfaceLive(ConnInterface interface)
{
    QCC_DbgPrintf(("RendezvousServerConnection::IsInterfaceLive()"));

    bool isAlive = false;
    uint32_t index;

    if (networkInterface->IsAnyNetworkInterfaceUp()) {
        for (index = 0; index < networkInterface->liveInterfaces.size(); ++index) {
            if ((interface.interfaceProperties.m_name == networkInterface->liveInterfaces[index].m_name)
                && (interface.interfaceProperties.m_addr == networkInterface->liveInterfaces[index].m_addr)) {
                isAlive = true;
                break;
            }
        }
    }


    return isAlive;
}

QStatus RendezvousServerConnection::ConnectOverAnyInterface(ConnInterface* connInterface, SocketFd& sockFd, HttpConnection** httpConn)
{
    QStatus status = ER_FAIL;

    for (uint32_t index = 0; index < networkInterface->liveInterfaces.size(); ++index) {
        connInterface->interfaceProperties = networkInterface->liveInterfaces[index];

        status = SetupConnOverInterface(*connInterface, sockFd, httpConn);

        if (status == ER_OK) {
            QCC_DbgPrintf(("RendezvousServerConnection::ConnectOverAnyInterface(): Successfully setup a connection: httpConn(0x%x)", httpConn));
            return status;
        }
    }

    return status;
}

void RendezvousServerConnection::UpdateConnectionDetails(SocketFd sockFd,
                                                         HttpConnection** oldHttpConn, HttpConnection* newHttpConn,
                                                         ConnInterface* oldConnInterface, ConnInterface* newConnInterface,
                                                         bool* isConnected, bool* connectionChangedFlag)
{
    QCC_DbgPrintf(("RendezvousServerConnection::UpdateConnectionDetails(): oldHttpConn(0x%x) newHttpConn(0x%x)", oldHttpConn, newHttpConn));

    /* Update the Connection interface with the new details */
    *oldConnInterface = *newConnInterface;

    /* Update the socket information in the connection details */
    oldConnInterface->sockFd = sockFd;

    /* Update the HTTP connection details */
    *oldHttpConn = newHttpConn;

    /* Set the isConnected flag to true */
    *isConnected = true;

    /* Set the connectionChangedFlag flag to true */
    *connectionChangedFlag = true;
}

void RendezvousServerConnection::CleanConnection(HttpConnection* httpConn, ConnInterface* connInterface, bool* isConnected)
{
    QCC_DbgPrintf(("RendezvousServerConnection::CleanConnection()"));

    /* Tear down the old HTTP connection */
    if (httpConn) {
        httpConn->Clear();
        delete httpConn;
        httpConn = NULL;
    }

    /* Close the allocated socket used by the old connection */
    if (connInterface->sockFd != -1) {
        Close(connInterface->sockFd);
    }

    /* Clear the connection interface details */
    connInterface->Clear();
    isConnected = false;
}

QStatus RendezvousServerConnection::SetupConnOverInterface(ConnInterface connInterface, SocketFd& sockFd,
                                                           HttpConnection** httpConn)
{
    QStatus status = ER_OK;

    QCC_DbgPrintf(("RendezvousServerConnection::SetupConnOverInterface(): connInterface = %s",
                   PrintConnInterface(connInterface).c_str()));

    if (UseHTTP) {
        status = SetupSockForConn(sockFd, connInterface);

        if (status != ER_OK) {
            QCC_LogError(status, ("RendezvousServerConnection::SetupConnOverInterface(): Unable to setup a socket for connection with the Server"));

            return status;
        }
    }

    /* Set up a new HTTP connection using the socket */
    status = SetupHTTPConn(sockFd, httpConn, connInterface.interfaceProperties.m_addr);

    if (status != ER_OK) {
        QCC_LogError(status, ("RendezvousServerConnection::SetupConnOverInterface(): Unable to setup a HTTP connection with the Server"));

        /* Close the allocated socket */
        if (sockFd != -1) {
            Close(sockFd);
        }

        return status;
    } else {
        QCC_DbgPrintf(("RendezvousServerConnection::SetupConnOverInterface(): Successfully set up a connection. httpConn(0x%x) sockFd(0x%x)", *httpConn, sockFd));
    }

    return status;
}

QStatus RendezvousServerConnection::SetupHTTPConn(SocketFd sockFd, HttpConnection** httpConn, String localIPAddress)
{
    QStatus status = ER_OK;

    QCC_DbgPrintf(("RendezvousServerConnection::SetupHTTPConn(): sockFd = %d", sockFd));

    *httpConn = new HttpConnection(localIPAddress);

    if (!(*httpConn)) {
        status = ER_FAIL;
        QCC_LogError(status, ("RendezvousServerConnection::SetupHTTPConn(): Unable to setup a HTTP connection with the Server"));
        return status;
    }

    status = (*httpConn)->SetHost(RendezvousServer);

    if (status == ER_OK) {
        if (!UseHTTP) {
            (*httpConn)->SetProtocol(HttpConnection::PROTO_HTTPS);
        }

        status = (*httpConn)->Connect(sockFd);

        if (status != ER_OK) {
            *httpConn = NULL;
            QCC_LogError(status, ("RendezvousServerConnection::SetupHTTPConn(): Unable to connect to the Rendezvous Server over HTTP"));
            return status;
        }

        QCC_DbgPrintf(("RendezvousServerConnection::SetupHTTPConn(): Connected to Rendezvous Server. *httpConn(0x%x)\n", *httpConn));
    }

    return status;
}

QStatus RendezvousServerConnection::SetupSockForConn(SocketFd& sockFd, ConnInterface connInterface)
{
    QStatus status = ER_FAIL;

    QCC_DbgPrintf(("RendezvousServerConnection::SetupSockForConn(): connInterface = %s", PrintConnInterface(connInterface).c_str()));

    if (!EnableIPv6) {
        if (connInterface.interfaceProperties.m_family == QCC_AF_INET) {
            status = Socket(QCC_AF_INET, QCC_SOCK_STREAM, sockFd);
            if (status != ER_OK) {
                QCC_LogError(status, ("RendezvousServerConnection::SetupSockForConn(): Socket(AF_INET) failed: %d - %s", errno, strerror(errno)));
            } else {
                QCC_DbgPrintf(("RendezvousServerConnection::SetupSockForConn(): Set up an IPv4 socket %d\n", sockFd));
            }
        }
    } else {
        if (connInterface.interfaceProperties.m_family == QCC_AF_INET6) {
            status = Socket(QCC_AF_INET6, QCC_SOCK_STREAM, sockFd);
            if (status != ER_OK) {
                QCC_LogError(status, ("RendezvousServerConnection::SetupSockForConn(): Socket(AF_INET6) failed: %d - %s", errno, strerror(errno)));
            } else {
                QCC_DbgPrintf(("RendezvousServerConnection::SetupSockForConn(): Set up an IPv6 socket %d\n", sockFd));
            }
        }
    }

    if (status == ER_OK) {
        uint32_t yes = 1;
        // PPN - Use SO_BINDTODEVICE instead of the bind call to connect the socket to an interface
        if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes)) < 0) {
            QCC_LogError(status, ("RendezvousServerConnection::SetupSockForConn(): setsockopt(SO_REUSEADDR) failed: %d - %s",
                                  errno, strerror(errno)));
            Close(sockFd);
            return status;
        } else {
            QCC_DbgPrintf(("RendezvousServerConnection::SetupSockForConn(): setsockopt succeeded\n"));
        }

        IPAddress address(connInterface.interfaceProperties.m_addr);

        //
        // Bind the socket to the IP Address specified in the interface
        //
        status = Bind(sockFd, address, 0);
        if (status != ER_OK) {
            QCC_LogError(status, ("RendezvousServerConnection::SetupSockForConn(): bind(%s) failed", address.ToString().c_str()));
            Close(sockFd);
            return status;
        } else {
            QCC_DbgPrintf(("RendezvousServerConnection::SetupSockForConn(): Socket %d successfully bound to %s\n", sockFd, address.ToString().c_str()));
        }
    }

    return status;
}

String RendezvousServerConnection::PrintConnInterface(ConnInterface interface) {
    String retStr;

    retStr += String("sockFd = ") + U32ToString((uint32_t)interface.sockFd) + String("\n");
    retStr += String("interfaceProperties.m_name = ") + interface.interfaceProperties.m_name + String("\n");
    retStr += String("interfaceProperties.m_addr = ") + interface.interfaceProperties.m_addr + String("\n");
    retStr += String("interfaceProperties.m_family = ") + U32ToString(interface.interfaceProperties.m_family) + String("\n");
    retStr += String("interfaceProperties.m_flags = ") + U32ToString(interface.interfaceProperties.m_flags) + String("\n");
    retStr += String("interfaceProperties.m_index = ") + U32ToString(interface.interfaceProperties.m_index) + String("\n");
    retStr += String("interfaceProperties.m_mtu = ") + U32ToString(interface.interfaceProperties.m_mtu) + String("\n");

    return retStr;
}

bool RendezvousServerConnection::IsSameInterface(ConnInterface* interface, ConnInterface* otherInterface)
{
    bool retVal = false;

    if ((interface->interfaceProperties.m_name == otherInterface->interfaceProperties.m_name)
        && (interface->interfaceProperties.m_addr == otherInterface->interfaceProperties.m_addr)) {
        retVal = true;
    }

    return retVal;
}

QStatus RendezvousServerConnection::SendMessage(bool sendOverPersistentConn, HttpConnection::Method httpMethod, String uri, bool payloadPresent, String payload)
{
    QStatus status = ER_OK;

    QCC_DbgPrintf(("RendezvousServerConnection::SendMessage(): uri = %s payloadPresent = %d sendOverPersistentConn = %d", uri.c_str(), payloadPresent, sendOverPersistentConn));

    HttpConnection* connection = onDemandConn;

    /* Send the message over the persistent connection if sendOverPersistentConn is true or else send it over the On Demand connection */
    if (sendOverPersistentConn) {
        connection = persistentConn;
        QCC_DbgPrintf(("RendezvousServerConnection::SendMessage(): Sending message with Uri %s over Persistent connection 0x%x", uri.c_str(), persistentConn));
        /* If the Persistent connection is not up return */
        if (!IsPersistentConnUp()) {
            status = ER_FAIL;
            QCC_LogError(status, ("RendezvousServerConnection::SendMessage(): The Persistent connection is not up"));
        }
    } else {
        QCC_DbgPrintf(("RendezvousServerConnection::SendMessage(): Sending message with Uri %s over On Demand connection", uri.c_str()));
        /* If the on demand connection is not up return */
        if (!IsOnDemandConnUp()) {
            status = ER_FAIL;
            QCC_LogError(status, ("RendezvousServerConnection::SendMessage(): The On Demand connection is not up"));
        }
    }

    if (status == ER_OK) {

        /* Setup the connection */
        connection->Clear();
        connection->SetRequestHeader("Host", RendezvousServer);
        connection->SetMethod(httpMethod);
        connection->SetUrlPath(uri);
        if (payloadPresent) {
            connection->AddApplicationJsonField(payload);
        }

        /* Send the message */
        status = connection->Send();

        if (status == ER_OK) {
            QCC_DbgPrintf(("RendezvousServerConnection::SendMessage(): Sent the message to the Rendezvous Server successfully"));
        } else {
            QCC_LogError(status, ("RendezvousServerConnection::SendMessage(): Unable to send the message to the Rendezvous Server successfully"));
        }
    }

    return status;
}

QStatus RendezvousServerConnection::FetchResponse(bool isOnDemandConnection, HttpConnection::HTTPResponse& response)
{
    QStatus status = ER_OK;

    QCC_DbgPrintf(("RendezvousServerConnection::FetchResponse(): isOnDemandConnection = %d", isOnDemandConnection));

    HttpConnection* connection = onDemandConn;

    if (isOnDemandConnection) {
        QCC_DbgPrintf(("RendezvousServerConnection::FetchResponse(): Receiving response over On Demand connection"));
        /* If the on demand connection is not up, return */
        if (!IsOnDemandConnUp()) {
            status = ER_FAIL;
            QCC_LogError(status, ("RendezvousServerConnection::FetchResponse(): The On Demand connection is not up"));
        }
    } else {
        connection = persistentConn;
        QCC_DbgPrintf(("RendezvousServerConnection::FetchResponse(): Receiving response over Persistent connection"));
        /* If the Persistent connection is not up, return */
        if (!IsPersistentConnUp()) {
            status = ER_FAIL;
            QCC_LogError(status, ("RendezvousServerConnection::FetchResponse(): The Persistent connection is not up"));
        }
    }

    if (status == ER_OK) {

        if (connection) {
            /* Send the message */
            status = connection->ParseResponse(response);

            if (status == ER_OK) {
                QCC_DbgPrintf(("RendezvousServerConnection::FetchResponse(): Parsed the response successfully"));
            } else {
                QCC_LogError(status, ("RendezvousServerConnection::FetchResponse(): Unable to parse the response successfully"));
                if (status == ER_OS_ERROR) {
                    QCC_LogError(status, ("OS_ERROR: %s", qcc::GetLastErrorString().c_str()));
                }
            }
        }

    }

    return status;
}

} // namespace ajn
