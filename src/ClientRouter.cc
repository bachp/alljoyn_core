/**
 * @file
 * ClientRouter is a simplified ("client-side only") router that is capable
 * of routing messages between a single remote and a single local endpoint.
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

#include <qcc/platform.h>

#include <qcc/Debug.h>
#include <qcc/Util.h>
#include <qcc/String.h>

#include <alljoyn/Status.h>

#include "Transport.h"
#include "BusEndpoint.h"
#include "LocalTransport.h"
#include "ClientRouter.h"
#include "BusInternal.h"

#define QCC_MODULE "ALLJOYN"

using namespace std;
using namespace qcc;

namespace ajn {

QStatus ClientRouter::PushMessage(Message& msg, BusEndpoint& sender)
{
    QStatus status = ER_OK;

    if (!localEndpoint->IsValid() || !nonLocalEndpoint->IsValid() || !sender->IsValid()) {
        status = ER_BUS_NO_ENDPOINT;
    } else {
        if (sender == BusEndpoint::cast(localEndpoint)) {
            localEndpoint->UpdateSerialNumber(msg);
            status = nonLocalEndpoint->PushMessage(msg);
        } else {
            status = localEndpoint->PushMessage(msg);
        }
    }

    if (ER_OK != status) {
        QCC_DbgHLPrintf(("ClientRouter::PushMessage failed: %s", QCC_StatusText(status)));
    }
    return status;
}

QStatus ClientRouter::RegisterEndpoint(BusEndpoint& endpoint)
{
    bool isLocal = endpoint->GetEndpointType() == ENDPOINT_TYPE_LOCAL;
    bool hadNonLocal = nonLocalEndpoint->IsValid();

    QCC_DbgHLPrintf(("ClientRouter::RegisterEndpoint"));

    /* Keep track of local and (at least one) non-local endpoint */
    if (isLocal) {
        localEndpoint = LocalEndpoint::cast(endpoint);
    } else {
        nonLocalEndpoint = endpoint;
    }

    /* Local and non-local endpoints must have the same unique name */
    if ((isLocal && nonLocalEndpoint->IsValid()) || (!isLocal && localEndpoint->IsValid() && !hadNonLocal)) {
        localEndpoint->SetUniqueName(nonLocalEndpoint->GetUniqueName());
    }

    /* Notify local endpoint we have both a local and at least one non-local endpoint */
    if (localEndpoint->IsValid() && nonLocalEndpoint->IsValid() && (isLocal || !hadNonLocal)) {
        localEndpoint->OnBusConnected();
    }
    return ER_OK;
}

void ClientRouter::UnregisterEndpoint(const String& epName, EndpointType epType)
{
    QCC_DbgHLPrintf(("ClientRouter::UnregisterEndpoint"));

    /* Unregister static endpoints */
    if ((nonLocalEndpoint->GetUniqueName() == epName) && (nonLocalEndpoint->GetEndpointType() == epType)) {
        /*
         * Let the bus know that the nonlocalEndpoint endpoint disconnected
         */
        localEndpoint->GetBus().GetInternal().NonLocalEndpointDisconnected();
        nonLocalEndpoint->Invalidate();
        nonLocalEndpoint = BusEndpoint();
    }

}

BusEndpoint ClientRouter::FindEndpoint(const qcc::String& busname)
{
    return nonLocalEndpoint;
}

ClientRouter::~ClientRouter()
{
    QCC_DbgHLPrintf(("ClientRouter::~ClientRouter()"));
}


}
