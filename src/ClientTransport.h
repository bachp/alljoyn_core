/**
 * @file
 * ClientTransport is the transport mechanism between a client and the daemon
 */

/******************************************************************************
 * Copyright 2009-2012, Qualcomm Innovation Center, Inc.
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

#ifndef _ALLJOYN_CLIENTTRANSPORT_H
#define _ALLJOYN_CLIENTTRANSPORT_H

#ifndef __cplusplus
#error Only include ClientTransport.h in C++ code.
#endif

#include <Status.h>

#include <qcc/platform.h>
#include <qcc/String.h>
#include <qcc/Mutex.h>
#include <qcc/Socket.h>
#include <qcc/Thread.h>
#include <qcc/SocketStream.h>
#include <qcc/time.h>

#include "Transport.h"
#include "RemoteEndpoint.h"

namespace ajn {

/**
 * @brief A class for Client Transports used in AllJoyn clients and services.
 *
 * The ClientTransport class has different incarnations depending on the platform.
 */
class ClientTransport : public Transport, public RemoteEndpoint::EndpointListener {

  public:
    /**
     * Create a Client based transport for use by clients and services.
     *
     * @param bus The BusAttachment associated with this endpoint
     */
    ClientTransport(BusAttachment& bus);

    /**
     * Destructor
     */
    virtual ~ClientTransport();

    /**
     * Start the transport and associate it with a router.
     *
     * @return
     *      - ER_OK if successful.
     *      - an error status otherwise.
     */
    QStatus Start();

    /**
     * Stop the transport.
     *
     * @return
     *      - ER_OK if successful.
     *      - an error status otherwise.
     */
    QStatus Stop();

    /**
     * Pend the caller until the transport stops.
     *
     * @return
     *      - ER_OK if successful
     *      - an error status otherwise.
     */
    QStatus Join();

    /**
     * Determine if this transport is running. Running means Start() has been called.
     *
     * @return  Returns true if the transport is running.
     */
    bool IsRunning() { return m_running; }

    /**
     * Normalize a transport specification.
     * Given a transport specification, convert it into a form which is guaranteed to have a one-to-one
     * relationship with a transport.
     *
     * @param inSpec    Input transport connect spec.
     * @param outSpec   Output transport connect spec.
     * @param argMap    Parsed parameter map.
     *
     * @return ER_OK if successful.
     */
    QStatus NormalizeTransportSpec(const char* inSpec, qcc::String& outSpec, std::map<qcc::String, qcc::String>& argMap) const;

    /**
     * Connect to a specified remote AllJoyn/DBus address.
     *
     * @param connectSpec    Transport specific key/value args used to configure the client-side endpoint.
     *                       The form of this string is @c "<transport>:<key1>=<val1>,<key2>=<val2>..."
     * @param opts           Requested sessions opts.
     * @param newep          [OUT] Endpoint created as a result of successful connect.
     * @return
     *      - ER_OK if successful.
     *      - an error status otherwise.
     */
    QStatus Connect(const char* connectSpec, const SessionOpts& opts, RemoteEndpoint** newep);

    /**
     * Disconnect from a specified AllJoyn/DBus address.
     *
     * @param connectSpec    The connectSpec used in Connect.
     *
     * @return
     *      - ER_OK if successful.
     *      - an error status otherwise.
     */
    QStatus Disconnect(const char* connectSpec);

    /**
     * This transport is only used for outbound connection so StartListen always returns an error status.
     *
     * @return Always returns ER_FAIL.
     */
    QStatus StartListen(const char* listenSpec) { return ER_FAIL; }

    /**
     * This transport is only used for outbound connection so StopListen always returns an error status.
     *
     * @return Always returns ER_FAIL.
     */
    QStatus StopListen(const char* listenSpec) { return ER_FAIL; }

    /**
     * Set a listener for transport related events.  There can only be one
     * listener set at a time. Setting a listener implicitly removes any
     * previously set listener.
     *
     * @param listener  Listener for transport related events.
     */
    void SetListener(TransportListener* listener) { m_listener = listener; }

    /**
     * @internal
     * @brief Provide an empty implementation of a discovery function not used
     * by clients or services.
     *
     * @param namePrefix unused parameter.
     */
    void EnableDiscovery(const char* namePrefix) { }

    /**
     * @internal
     * @brief Provide an empty implementation of a discovery function not used
     * by clients or services.
     *
     * @param namePrefix unused parameter.
     */
    void DisableDiscovery(const char* namePrefix) { }

    /**
     * @internal
     * @brief Provide an empty implementation of a discovery function not used
     * by clients or services.
     *
     * @param advertiseName unused parameter.
     * @return ER_OK if successful
     */
    QStatus EnableAdvertisement(const qcc::String& advertiseName) { return ER_NOT_IMPLEMENTED; }

    /**
     * @internal
     * @brief Provide an empty implementation of a discovery function not used
     * by clients or services.
     *
     * @param advertiseName unused parameter.
     * @param nameListEmpty unused parameter.
     */
    void DisableAdvertisement(const qcc::String& advertiseName, bool nameListEmpty) { }

    /**
     * Returns the name of this transport
     */
    const char* GetTransportName() const { return TransportName; }

    /**
     * Get the transport mask for this transport
     *
     * @return the TransportMask for this transport.
     */
    TransportMask GetTransportMask() const { return TRANSPORT_WLAN; }

    /**
     * Get a list of the possible listen specs for a given set of session options.
     * @param[IN]    opts      Session options.
     * @param[OUT]   busAddrs  Set of listen addresses. Always empty for this transport.
     * @return ER_OK if successful.
     */
    QStatus GetListenAddresses(const SessionOpts& opts, std::vector<qcc::String>& busAddrs) const { return ER_OK; }

    /**
     * Indicates whether this transport is used for client-to-bus or bus-to-bus connections.
     *
     * @return  Always returns false, ClientTransports are only used to connect to a local daemon.
     */
    bool IsBusToBus() const { return false; }

    /**
     * Name of transport used in transport specs.
     */
    static const char* TransportName;

    /**
     * Callback for ClientEndpoint exit.
     *
     * @param endpoint   ClientEndpoint instance that has exited.
     */
    void EndpointExit(RemoteEndpoint* endpoint);

  private:
    BusAttachment& m_bus;           /**< The message bus for this transport */
    bool m_running;                 /**< True after Start() has been called, before Stop() */
    bool m_stopping;                /**< True if Stop() has been called but endpoints still exist */
    TransportListener* m_listener;  /**< Registered TransportListener */
    RemoteEndpoint* m_endpoint;     /**< The active endpoint */
    qcc::Mutex m_epLock;            /**< Lock to prevent the endpoint from being destroyed while it is being stopped */
};

} // namespace ajn

#endif // _ALLJOYN_CLIENTTRANSPORT_H
