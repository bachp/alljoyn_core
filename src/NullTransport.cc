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

#include <alljoyn/BusAttachment.h>

#include "BusInternal.h"
#include "RemoteEndpoint.h"
#include "NullTransport.h"

#define QCC_MODULE "ALLJOYN"

using namespace std;
using namespace qcc;

namespace ajn {

const char* NullTransport::TransportName = "null";

DaemonLauncher* NullTransport::daemonLauncher;

BusAttachment* NullTransport::daemonBus;

class RouterEndpoint;

class NullEndpoint : public BusEndpoint {

  public:

    NullEndpoint(Router& clientRouter, Router& daemonRouter);

    ~NullEndpoint();

    QStatus PushMessage(Message& msg) {
        /*
         * TODO - need to handle encryption
         */
        return daemonRouter.PushMessage(msg, *this);
    }

    const qcc::String& GetUniqueName() const { return uniqueName; }

    uint32_t GetUserId() const { return -1; }
    uint32_t GetGroupId() const { return -1; }
    uint32_t GetProcessId() const { return -1; }
    bool SupportsUnixIDs() const { return false; }
    bool AllowRemoteMessages() { return true; }

    qcc::String uniqueName;
    Router& clientRouter;
    Router& daemonRouter;
    RouterEndpoint* routerEndpoint;
};

class RouterEndpoint : public BusEndpoint {

  public:

    RouterEndpoint(NullEndpoint& nullEndpoint) :
        BusEndpoint(ENDPOINT_TYPE_NULL),
        nullEndpoint(nullEndpoint)
    {
        nullEndpoint.daemonRouter.RegisterEndpoint(*this, false);
    }

    ~RouterEndpoint() {
        nullEndpoint.daemonRouter.UnregisterEndpoint(*this);
    }

    QStatus PushMessage(Message& msg) {
        return nullEndpoint.clientRouter.PushMessage(msg, *this);
    }

    const qcc::String& GetUniqueName() const { return nullEndpoint.uniqueName; }

    uint32_t GetUserId() const { return -1; }
    uint32_t GetGroupId() const { return -1; }
    uint32_t GetProcessId() const { return -1; }
    bool SupportsUnixIDs() const { return false; }
    bool AllowRemoteMessages() { return true; }

    NullEndpoint& nullEndpoint;
};

NullEndpoint::NullEndpoint(Router& clientRouter, Router& daemonRouter) :
    BusEndpoint(ENDPOINT_TYPE_NULL),
    uniqueName(daemonRouter.GenerateUniqueName()),
    clientRouter(clientRouter),
    daemonRouter(daemonRouter),
    routerEndpoint(NULL)
{
    clientRouter.RegisterEndpoint(*this, false);
    routerEndpoint = new RouterEndpoint(*this);
}

NullEndpoint::~NullEndpoint()
{
    delete routerEndpoint;
    clientRouter.UnregisterEndpoint(*this);
}

NullTransport::NullTransport(BusAttachment& bus) : bus(bus), running(false), endpoint(NULL)
{
}

NullTransport::~NullTransport()
{
}

QStatus NullTransport::Start()
{
    running = true;
    return ER_OK;
}

QStatus NullTransport::Stop(void)
{
    running = false;
    if (daemonLauncher) {
        daemonLauncher->Stop();
    }
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

QStatus NullTransport::Connect(const char* connectSpec, const SessionOpts& opts, BusEndpoint** newep)
{
    QStatus status = ER_OK;

    if (!running) {
        return ER_BUS_TRANSPORT_NOT_STARTED;
    }
    if (!daemonLauncher) {
        return ER_BUS_TRANSPORT_NOT_AVAILABLE;
    }
    if (!daemonBus) {
        status = daemonLauncher->Start(daemonBus);
    }
    if (status == ER_OK) {
        endpoint = new NullEndpoint(bus.GetInternal().GetRouter(), daemonBus->GetInternal().GetRouter());
        if (newep) {
            *newep = endpoint;
        }
    }
    return status;
}

QStatus NullTransport::Disconnect(const char* connectSpec)
{
    delete endpoint;
    endpoint = NULL;
    return ER_OK;
}

void NullTransport::RegisterDaemonLauncher(DaemonLauncher* launcher)
{
    daemonLauncher = launcher;
}

} // namespace ajn
