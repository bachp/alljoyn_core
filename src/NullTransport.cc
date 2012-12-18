/**
 * @file
 * NullTransport implementation
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

#include <list>

#include <errno.h>
#include <qcc/Socket.h>
#include <qcc/SocketStream.h>
#include <qcc/String.h>
#include <qcc/StringUtil.h>
#include <qcc/Util.h>
#include <qcc/Debug.h>

#include <alljoyn/BusAttachment.h>

#include "BusInternal.h"
#include "RemoteEndpoint.h"
#include "NullTransport.h"
#include "AllJoynPeerObj.h"

#define QCC_MODULE "NULL_TRANSPORT"

using namespace std;
using namespace qcc;

namespace ajn {

const char* NullTransport::TransportName = "null";

DaemonLauncher* NullTransport::daemonLauncher;

class _NullEndpoint;

typedef ManagedObj<_NullEndpoint> NullEndpoint;

/*
 * The null endpoint simply moves messages between the daemon router to the client router and lets
 * the routers handle it from there. The only wrinkle is that messages forwarded to the daemon may
 * need to be encrypted because in the non-bundled case encryption is done in _Message::Deliver()
 * and that method does not get called in this case.
 */
class _NullEndpoint : public _BusEndpoint {

    friend class NullTransport;

  public:

    _NullEndpoint(BusAttachment& clientBus, BusAttachment& daemonBus);

    ~_NullEndpoint();

    QStatus PushMessage(Message& msg);

    const qcc::String& GetUniqueName() const { return uniqueName; }

    uint32_t GetUserId() const { return qcc::GetUid(); }

    uint32_t GetGroupId() const { return qcc::GetGid(); }

    uint32_t GetProcessId() const { return qcc::GetPid(); }

    bool SupportsUnixIDs() const {
#if defined(QCC_OS_GROUP_WINDOWS) || defined(QCC_OS_GROUP_WINRT)
        return false;
#else
        return true;
#endif
    }

    bool AllowRemoteMessages() { return true; }

    int32_t clientReady;
    BusAttachment& clientBus;
    BusAttachment& daemonBus;

    qcc::String uniqueName;

    /*
     * Register the endpoint with the client on receiving the first message from the daemon.
     */
    inline void CheckRegisterEndpoint()
    {
        if (!clientReady) {
            /* Use atomic increment to avoid race */
            if (IncrementAndFetch(&clientReady) == 1) {
                QCC_DbgHLPrintf(("Registering null endpoint with client"));
                BusEndpoint busEndpoint = BusEndpoint::wrap(this);
                clientBus.GetInternal().GetRouter().RegisterEndpoint(busEndpoint);
            } else {
                DecrementAndFetch(&clientReady);
            }
        }
    }
};

_NullEndpoint::_NullEndpoint(BusAttachment& clientBus, BusAttachment& daemonBus) :
    _BusEndpoint(ENDPOINT_TYPE_NULL),
    clientReady(0),
    clientBus(clientBus),
    daemonBus(daemonBus)
{
    /*
     * We short-circuit all of the normal authentication and hello handshakes and
     * simply get a unique name for the null endpoint directly from the daemon.
     */
    uniqueName = daemonBus.GetInternal().GetRouter().GenerateUniqueName();
    QCC_DbgHLPrintf(("Creating null endpoint %s", uniqueName.c_str()));
}

_NullEndpoint::~_NullEndpoint()
{
    QCC_DbgHLPrintf(("Destroying null endpoint %s", uniqueName.c_str()));
}



QStatus _NullEndpoint::PushMessage(Message& msg)
{
    if (!IsValid()) {
        return ER_BUS_ENDPOINT_CLOSING;
    }

    QStatus status = ER_OK;
    /*
     * In the un-bundled daemon case messages store the name of the endpoint they were received
     * on. As far as the client and daemon routers are concerned the message was received from
     * this endpoint so we must set the received name to the unique name of this endpoint.
     */
    msg->rcvEndpointName = uniqueName;
    /*
     * If the message came from the client forward it to the daemon and visa versa. Note that
     * if the message didn't come from the client it must be assumed that it came from the
     * daemon to handle to the (rare) case of a broadcast signal being sent to multiple bus
     * attachments in a single application.
     */
    if (msg->bus == &clientBus) {
        /*
         * In the non-bundled case messages are encrypted when they are delivered to the daemon
         * endpoint by a call to Message::Deliver. The null transport bypasses Message::Deliver
         * by pushing the messages directly to the daemon router. This means we need to encrypt
         * messages here before we do the push.
         */
        if (msg->encrypt) {
            status = msg->EncryptMessage();
            /* Report authorization failure as a security violation */
            if (status == ER_BUS_NOT_AUTHORIZED) {
                clientBus.GetInternal().GetLocalEndpoint()->GetPeerObj()->HandleSecurityViolation(msg, status);
            }
        }
        if (status == ER_OK) {
            msg->bus = &daemonBus;
            BusEndpoint busEndpoint = BusEndpoint::wrap(this);
            status = daemonBus.GetInternal().GetRouter().PushMessage(msg, busEndpoint);
        } else if (status == ER_BUS_AUTHENTICATION_PENDING) {
            status = ER_OK;
        }
    } else {
        assert(msg->bus == &daemonBus);
        /*
         * Register the endpoint with the client router if needed
         */
        CheckRegisterEndpoint();
        /*
         * We need to clone broadcast signals because each receiving bus attachment must be
         * able to unmarshal the arg list including decrypting and doing header expansion.
         */
        if (msg->IsBroadcastSignal()) {
            Message clone(msg, true /*deep copy*/);
            clone->bus = &clientBus;
            BusEndpoint busEndpoint = BusEndpoint::wrap(this);
            status = clientBus.GetInternal().GetRouter().PushMessage(clone, busEndpoint);
        } else {
            msg->bus = &clientBus;
            BusEndpoint busEndpoint = BusEndpoint::wrap(this);
            status = clientBus.GetInternal().GetRouter().PushMessage(msg, busEndpoint);
        }
    }
    return status;
}

NullTransport::NullTransport(BusAttachment& bus) : bus(bus), running(false), daemonBus(NULL)
{
}

NullTransport::~NullTransport()
{
    Stop();
    Join();
    /* Wait for any threads that are in PushMessage to finish */
    while (endpoint.GetRefCount() > 1) {
        qcc::Sleep(4);
    }
}

QStatus NullTransport::Start()
{
    running = true;
    return ER_OK;
}

QStatus NullTransport::Stop(void)
{
    running = false;
    Disconnect("null:");
    return ER_OK;
}

QStatus NullTransport::Join(void)
{
    if (daemonLauncher) {
        daemonLauncher->Join();
    }
    return ER_OK;
}

QStatus NullTransport::NormalizeTransportSpec(const char* inSpec, qcc::String& outSpec, std::map<qcc::String, qcc::String>& argMap) const
{
    outSpec = inSpec;
    return ER_OK;
}

QStatus NullTransport::LinkBus(BusAttachment* otherBus)
{
    QCC_DbgHLPrintf(("Linking client and daemon busses"));

    assert(otherBus);

    /*
     * Initialize the null endpoint
     */
    NullEndpoint ep(bus, *otherBus);
    /*
     * The compression rules are shared between the client bus and the daemon bus
     */
    bus.GetInternal().OverrideCompressionRules(otherBus->GetInternal().GetCompressionRules());
    /*
     * Register the null endpoint with the daemon router. The endpoint is registered with the client
     * router either below or in PushMessage if a message is received before the call to register
     * the endpoint with the daemon router returns.
     */
    QCC_DbgHLPrintf(("Registering null endpoint with daemon"));

    endpoint = BusEndpoint::cast(ep);
    QStatus status = otherBus->GetInternal().GetRouter().RegisterEndpoint(endpoint);
    if (status != ER_OK) {
        endpoint->Invalidate();
    } else {
        /*
         * Register the endpoint with the client router if needed
         */
        ep->CheckRegisterEndpoint();
    }
    return status;
}

QStatus NullTransport::Connect(const char* connectSpec, const SessionOpts& opts, BusEndpoint& newep)
{
    QStatus status = ER_OK;

    if (!running) {
        return ER_BUS_TRANSPORT_NOT_STARTED;
    }
    if (!daemonLauncher) {
        return ER_BUS_TRANSPORT_NOT_AVAILABLE;
    }

    status = daemonLauncher->Start(this);
    if (status == ER_OK) {
        newep = endpoint;
    }
    return status;
}

QStatus NullTransport::Disconnect(const char* connectSpec)
{
    if (endpoint->IsValid()) {
        NullEndpoint ep = NullEndpoint::cast(endpoint);
        assert(daemonLauncher);
        ep->clientBus.GetInternal().GetRouter().UnregisterEndpoint(ep->GetUniqueName(), ep->GetEndpointType());
        ep->daemonBus.GetInternal().GetRouter().UnregisterEndpoint(ep->GetUniqueName(), ep->GetEndpointType());
        daemonLauncher->Stop(this);
        daemonLauncher->Join();
        ep->Invalidate();
    }
    return ER_OK;
}

void NullTransport::RegisterDaemonLauncher(DaemonLauncher* launcher)
{
    daemonLauncher = launcher;
}

} // namespace ajn
