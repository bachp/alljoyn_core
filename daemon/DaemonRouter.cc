/**
 * @file
 * Router is responsible for taking inbound messages and routing them
 * to an appropriate set of endpoints.
 */

/******************************************************************************
 * Copyright 2009-2013, Qualcomm Innovation Center, Inc.
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

#include <assert.h>

#include <qcc/Debug.h>
#include <qcc/Logger.h>
#include <qcc/String.h>
#include <qcc/Util.h>
#include <qcc/atomic.h>

#include <alljoyn/Status.h>

#include "BusController.h"
#include "BusEndpoint.h"
#include "DaemonRouter.h"
#include "EndpointHelper.h"

#define QCC_MODULE "ALLJOYN"

using namespace std;
using namespace qcc;


namespace ajn {


DaemonRouter::DaemonRouter() : ruleTable(), nameTable(), busController(NULL)
{
}

DaemonRouter::~DaemonRouter()
{
}

static inline QStatus SendThroughEndpoint(Message& msg, BusEndpoint& ep, SessionId sessionId)
{
    QStatus status;
    if ((sessionId != 0) && (ep->GetEndpointType() == ENDPOINT_TYPE_VIRTUAL)) {
        status = VirtualEndpoint::cast(ep)->PushMessage(msg, sessionId);
    } else {
        status = ep->PushMessage(msg);
    }
    if ((status != ER_OK) && (status != ER_BUS_ENDPOINT_CLOSING)) {
        QCC_LogError(status, ("SendThroughEndpoint(dest=%s, ep=%s, id=%u) failed", msg->GetDestination(), ep->GetUniqueName().c_str(), sessionId));
    }
    return status;
}

QStatus DaemonRouter::PushMessage(Message& msg, BusEndpoint& origSender)
{
    /*
     * Reference count protects local endpoint from being deregistered while in use.
     */
    if (!localEndpoint->IsValid()) {
        return ER_BUS_ENDPOINT_CLOSING;
    }

    QStatus status = ER_OK;
    BusEndpoint sender = origSender;
    bool replyExpected = (msg->GetType() == MESSAGE_METHOD_CALL) && ((msg->GetFlags() & ALLJOYN_FLAG_NO_REPLY_EXPECTED) == 0);
    const char* destination = msg->GetDestination();
    SessionId sessionId = msg->GetSessionId();
    bool isSessionless = msg->GetFlags() & ALLJOYN_FLAG_SESSIONLESS;

    /*
     * Sessionless messages don't have a session id even though they have a dedicated
     * session to travel over. Therefore, get the sessionId from the endpoint if possible.
     */
    if (isSessionless && (sender->GetEndpointType() == ENDPOINT_TYPE_BUS2BUS)) {
        /*
         * The bus controller is responsible for "some" routing of sessionless signals.
         * Specifically, sessionless signals that are received solely to "catch-up" a
         * newly connected local client are routed directly to that client by the BusController.
         * If RouteSessionlessSignal returns true, then the BusController has successfully
         * routed the sessionless signal to its destination and no further action should be
         * taken here. If it returns false, then the normal routing procedure should
         * attempt to deliver the message.
         */
        if (busController->GetSessionlessObj().RouteSessionlessMessage(RemoteEndpoint::cast(sender)->GetSessionId(), msg)) {
            return ER_OK;
        }
    }

    /*
     * If the message originated at the local endpoint check if the serial number needs to be
     * updated before queuing the message on a remote endpoint.
     */
    if (origSender == localEndpoint) {
        localEndpoint->UpdateSerialNumber(msg);
    }

    bool destinationEmpty = destination[0] == '\0';
    if (!destinationEmpty) {
        nameTable.Lock();
        BusEndpoint destEndpoint = nameTable.FindEndpoint(destination);
        if (destEndpoint->IsValid()) {
            /* If this message is coming from a bus-to-bus ep, make sure the receiver is willing to receive it */
            if (!((sender->GetEndpointType() == ENDPOINT_TYPE_BUS2BUS) && !destEndpoint->AllowRemoteMessages())) {
                /*
                 * If the sender doesn't allow remote messages reject method calls that go off
                 * device and require a reply because the reply will be blocked and this is most
                 * definitely not what the sender expects.
                 */
                if ((destEndpoint->GetEndpointType() == ENDPOINT_TYPE_VIRTUAL) && replyExpected && !sender->AllowRemoteMessages()) {
                    QCC_DbgPrintf(("Blocking method call from %s to %s (serial=%d) because caller does not allow remote messages",
                                   msg->GetSender(),
                                   destEndpoint->GetUniqueName().c_str(),
                                   msg->GetCallSerial()));
                    msg->ErrorMsg(msg, "org.alljoyn.Bus.Blocked", "Method reply would be blocked because caller does not allow remote messages");
                    BusEndpoint busEndpoint = BusEndpoint::cast(localEndpoint);
                    PushMessage(msg, busEndpoint);
                } else {
                    nameTable.Unlock();
                    status = SendThroughEndpoint(msg, destEndpoint, sessionId);
                    nameTable.Lock();
                }
            } else {
                QCC_DbgPrintf(("Blocking message from %s to %s (serial=%d) because receiver does not allow remote messages",
                               msg->GetSender(),
                               destEndpoint->GetUniqueName().c_str(),
                               msg->GetCallSerial()));
                /* If caller is expecting a response return an error indicating the method call was blocked */
                if (replyExpected) {
                    qcc::String description("Remote method calls blocked for bus name: ");
                    description += destination;
                    msg->ErrorMsg(msg, "org.alljoyn.Bus.Blocked", description.c_str());
                    BusEndpoint busEndpoint = BusEndpoint::cast(localEndpoint);
                    PushMessage(msg, busEndpoint);
                }
            }
            if ((ER_OK != status) && (ER_BUS_ENDPOINT_CLOSING != status)) {
                QCC_LogError(status, ("BusEndpoint::PushMessage failed"));
            }
            nameTable.Unlock();
        } else {
            nameTable.Unlock();
            if ((msg->GetFlags() & ALLJOYN_FLAG_AUTO_START) &&
                (sender->GetEndpointType() != ENDPOINT_TYPE_BUS2BUS) &&
                (sender->GetEndpointType() != ENDPOINT_TYPE_NULL)) {

                status = busController->StartService(msg, sender);
            } else {
                status = ER_BUS_NO_ROUTE;
            }
            if (status != ER_OK) {
                if (replyExpected) {
                    QCC_LogError(status, ("Returning error %s no route to %s", msg->Description().c_str(), destination));
                    /* Need to let the sender know its reply message cannot be passed on. */
                    qcc::String description("Unknown bus name: ");
                    description += destination;
                    msg->ErrorMsg(msg, "org.freedesktop.DBus.Error.ServiceUnknown", description.c_str());
                    BusEndpoint busEndpoint = BusEndpoint::cast(localEndpoint);
                    PushMessage(msg, busEndpoint);
                } else {
                    QCC_LogError(status, ("Discarding %s no route to %s:%d", msg->Description().c_str(), destination, sessionId));
                }
            }
        }
    } else if (sessionId == 0) {
        /*
         * The message has an empty destination field and no session is specified so this is a
         * regular broadcast message.
         */
        nameTable.Lock();
        ruleTable.Lock();
        RuleIterator it = ruleTable.Begin();
        while (it != ruleTable.End()) {
            if (it->second.IsMatch(msg)) {
                BusEndpoint dest = it->first;
                QCC_DbgPrintf(("Routing %s (%d) to %s", msg->Description().c_str(), msg->GetCallSerial(), dest->GetUniqueName().c_str()));
                /*
                 * If the message originated locally or the destination allows remote messages
                 * forward the message, otherwise silently ignore it.
                 */
                if (!((sender->GetEndpointType() == ENDPOINT_TYPE_BUS2BUS) && !dest->AllowRemoteMessages())) {
                    ruleTable.Unlock();
                    nameTable.Unlock();
                    QStatus tStatus = SendThroughEndpoint(msg, dest, sessionId);
                    status = (status == ER_OK) ? tStatus : status;
                    nameTable.Lock();
                    ruleTable.Lock();
                }
                it = ruleTable.AdvanceToNextEndpoint(dest);
            } else {
                ++it;
            }
        }
        ruleTable.Unlock();
        nameTable.Unlock();

        if (msg->IsSessionless()) {
            /* Give "locally generated" sessionless message to SessionlessObj */
            if (sender->GetEndpointType() != ENDPOINT_TYPE_BUS2BUS) {
                status = busController->PushSessionlessMessage(msg);
            }
        } else if (msg->IsGlobalBroadcast()) {
            /* Route global broadcast to all bus-to-bus endpoints that aren't the sender of the message */
            m_b2bEndpointsLock.Lock(MUTEX_CONTEXT);
            set<RemoteEndpoint>::iterator it = m_b2bEndpoints.begin();
            while (it != m_b2bEndpoints.end()) {
                RemoteEndpoint ep = *it;
                if (ep != origSender) {
                    m_b2bEndpointsLock.Unlock(MUTEX_CONTEXT);
                    BusEndpoint busEndpoint = BusEndpoint::cast(ep);
                    QStatus tStatus = SendThroughEndpoint(msg, busEndpoint, sessionId);
                    status = (status == ER_OK) ? tStatus : status;
                    m_b2bEndpointsLock.Lock(MUTEX_CONTEXT);
                    it = m_b2bEndpoints.lower_bound(ep);
                }
                if (it != m_b2bEndpoints.end()) {
                    ++it;
                }
            }
            m_b2bEndpointsLock.Unlock(MUTEX_CONTEXT);
        }

    } else {
        /*
         * The message has an empty destination field and a session id was specified so this is a
         * session multicast message.
         */
        sessionCastSetLock.Lock(MUTEX_CONTEXT);
        RemoteEndpoint lastB2b;
        /* We need to obtain the first entry in the sessionCastSet that has the id equal to 'sessionId'
         * and the src equal to 'msg->GetSender()'.
         * Note: sce.id has been set to sessionId - 1. Since the src is compared first, and session Ids
         * are integers, upper_bound will return the iterator to the first element with the desired or
         * greater src(if not present) and desired or greater id in most cases.
         */
        SessionCastEntry sce(sessionId - 1, msg->GetSender());
        set<SessionCastEntry>::iterator sit = sessionCastSet.upper_bound(sce);
        bool foundDest = false;

        /* In other cases, it may return the iterator to an element that has the desired src and
         * (sessionId - 1). In that case iterate, until the id is less than the desired one.
         */
        while ((sit != sessionCastSet.end()) && (sit->src == sce.src) && (sit->id < sessionId)) {
            sit++;
        }

        while ((sit != sessionCastSet.end()) && (sit->id == sessionId) && (sit->src == sce.src)) {
            if (sit->b2bEp != lastB2b) {
                foundDest = true;
                lastB2b = sit->b2bEp;
                SessionCastEntry entry = *sit;
                BusEndpoint ep = sit->destEp;
                sessionCastSetLock.Unlock(MUTEX_CONTEXT);
                QStatus tStatus = SendThroughEndpoint(msg, ep, sessionId);
                status = (status == ER_OK) ? tStatus : status;
                sessionCastSetLock.Lock(MUTEX_CONTEXT);
                sit = sessionCastSet.lower_bound(entry);
            }
            if (sit != sessionCastSet.end()) {
                ++sit;
            }
        }
        if (!foundDest) {
            status = ER_BUS_NO_ROUTE;
        }
        sessionCastSetLock.Unlock(MUTEX_CONTEXT);
    }

    return status;
}

void DaemonRouter::GetBusNames(vector<qcc::String>& names) const
{
    nameTable.GetBusNames(names);
}

BusEndpoint DaemonRouter::FindEndpoint(const qcc::String& busName)
{
    BusEndpoint ep = nameTable.FindEndpoint(busName);
    if (!ep->IsValid()) {
        m_b2bEndpointsLock.Lock(MUTEX_CONTEXT);
        for (set<RemoteEndpoint>::const_iterator it = m_b2bEndpoints.begin(); it != m_b2bEndpoints.end(); ++it) {
            if ((*it)->GetUniqueName() == busName) {
                RemoteEndpoint rep = *it;
                ep = BusEndpoint::cast(rep);
                break;
            }
        }
        m_b2bEndpointsLock.Unlock(MUTEX_CONTEXT);
    }
    return ep;
}

QStatus DaemonRouter::AddRule(BusEndpoint& endpoint, Rule& rule)
{
    QStatus status = ruleTable.AddRule(endpoint, rule);

    /* Allow busController to examine this rule */
    if (status == ER_OK) {
        busController->AddRule(endpoint->GetUniqueName(), rule);
    }

    return status;
}

QStatus DaemonRouter::RemoveRule(BusEndpoint& endpoint, Rule& rule)
{
    QStatus status = ruleTable.RemoveRule(endpoint, rule);

    /* Allow busController to examine rule being removed */
    busController->RemoveRule(endpoint->GetUniqueName(), rule);

    return status;
}

QStatus DaemonRouter::RegisterEndpoint(BusEndpoint& endpoint)
{
    QCC_DbgTrace(("DaemonRouter::RegisterEndpoint(%s, %d)", endpoint->GetUniqueName().c_str(), endpoint->GetEndpointType()));
    QStatus status = ER_OK;

    /* Keep track of local endpoint */
    if (endpoint->GetEndpointType() == ENDPOINT_TYPE_LOCAL) {
        localEndpoint = LocalEndpoint::cast(endpoint);
    }

    if (endpoint->GetEndpointType() == ENDPOINT_TYPE_BUS2BUS) {
        /* AllJoynObj is in charge of managing bus-to-bus endpoints and their names */
        RemoteEndpoint busToBusEndpoint = RemoteEndpoint::cast(endpoint);
        status = busController->GetAllJoynObj().AddBusToBusEndpoint(busToBusEndpoint);

        /* Add to list of bus-to-bus endpoints */
        m_b2bEndpointsLock.Lock(MUTEX_CONTEXT);
        m_b2bEndpoints.insert(busToBusEndpoint);
        m_b2bEndpointsLock.Unlock(MUTEX_CONTEXT);
    } else {
        /* Bus-to-client endpoints appear directly on the bus */
        nameTable.AddUniqueName(endpoint);
    }

    /* Notify local endpoint that it is connected */
    if (endpoint == localEndpoint) {
        localEndpoint->OnBusConnected();
    }

    return status;
}

void DaemonRouter::UnregisterEndpoint(const qcc::String& epName, EndpointType epType)
{
    QCC_DbgTrace(("UnregisterEndpoint: %s", epName.c_str()));

    /* Attempt to get the endpoint */
    nameTable.Lock();
    BusEndpoint endpoint = FindEndpoint(epName);
    nameTable.Unlock();

    if (ENDPOINT_TYPE_BUS2BUS == endpoint->GetEndpointType()) {
        /* Inform bus controller of bus-to-bus endpoint removal */
        RemoteEndpoint busToBusEndpoint = RemoteEndpoint::cast(endpoint);

        busController->GetAllJoynObj().RemoveBusToBusEndpoint(busToBusEndpoint);

        /* Remove the bus2bus endpoint from the list */
        m_b2bEndpointsLock.Lock(MUTEX_CONTEXT);
        set<RemoteEndpoint>::iterator it = m_b2bEndpoints.begin();
        while (it != m_b2bEndpoints.end()) {
            RemoteEndpoint rep = *it;
            if (rep == busToBusEndpoint) {
                m_b2bEndpoints.erase(it);
                break;
            }
            ++it;
        }
        m_b2bEndpointsLock.Unlock(MUTEX_CONTEXT);

        /* Remove entries from sessionCastSet with same b2bEp */
        sessionCastSetLock.Lock(MUTEX_CONTEXT);
        set<SessionCastEntry>::iterator sit = sessionCastSet.begin();
        while (sit != sessionCastSet.end()) {
            set<SessionCastEntry>::iterator doomed = sit;
            ++sit;
            if (doomed->b2bEp == endpoint) {
                sessionCastSet.erase(doomed);
            }
        }
        sessionCastSetLock.Unlock(MUTEX_CONTEXT);
    } else {
        /* Remove any session routes */
        RemoveSessionRoutes(endpoint->GetUniqueName().c_str(), 0);
        /* Remove endpoint from names and rules */
        nameTable.RemoveUniqueName(endpoint->GetUniqueName());
        RemoveAllRules(endpoint);
        PermissionMgr::CleanPermissionCache(endpoint);
    }
    /*
     * If the local endpoint is being deregistered this indicates the router is being shut down.
     */
    if (endpoint == localEndpoint) {
        localEndpoint->Invalidate();
    }
}

QStatus DaemonRouter::AddSessionRoute(SessionId id, BusEndpoint& srcEp, RemoteEndpoint* srcB2bEp, BusEndpoint& destEp, RemoteEndpoint& destB2bEp, SessionOpts* optsHint)
{
    QCC_DbgTrace(("DaemonRouter::AddSessionRoute(%u, %s, %s, %s, %s, %s)", id, srcEp->GetUniqueName().c_str(), srcB2bEp ? (*srcB2bEp)->GetUniqueName().c_str() : "<none>", destEp->GetUniqueName().c_str(), destB2bEp->GetUniqueName().c_str(), optsHint ? "opts" : "NULL"));

    QStatus status = ER_OK;
    if (id == 0) {
        return ER_BUS_NO_SESSION;
    }
    if (destEp->GetEndpointType() == ENDPOINT_TYPE_VIRTUAL) {
        if (destB2bEp->IsValid()) {
            status = VirtualEndpoint::cast(destEp)->AddSessionRef(id, destB2bEp);
        } else if (optsHint) {
            status = VirtualEndpoint::cast(destEp)->AddSessionRef(id, optsHint, destB2bEp);
        } else {
            status = ER_BUS_NO_SESSION;
        }
        if (status != ER_OK) {
            QCC_LogError(status, ("AddSessionRef(this=%s, %u, %s%s) failed", destEp->GetUniqueName().c_str(),
                                  id, destB2bEp->IsValid() ? "" : "opts, ", destB2bEp->GetUniqueName().c_str()));
        }
    }
    /*
     * srcB2bEp is only NULL when srcEP is non-virtual
     */
    if ((status == ER_OK) && srcB2bEp) {
        assert(srcEp->GetEndpointType() == ENDPOINT_TYPE_VIRTUAL);
        status = VirtualEndpoint::cast(srcEp)->AddSessionRef(id, *srcB2bEp);
        if (status != ER_OK) {
            assert(destEp->GetEndpointType() == ENDPOINT_TYPE_VIRTUAL);
            QCC_LogError(status, ("AddSessionRef(this=%s, %u, %s) failed", srcEp->GetUniqueName().c_str(), id, (*srcB2bEp)->GetUniqueName().c_str()));
            VirtualEndpoint::cast(destEp)->RemoveSessionRef(id);
        }
    }

    /* Set sessionId on B2B endpoints */
    if (status == ER_OK) {
        if (srcB2bEp) {
            (*srcB2bEp)->SetSessionId(id);
        }
        destB2bEp->SetSessionId(id);
    }

    /* Add sessionCast entries */
    if (status == ER_OK) {
        sessionCastSetLock.Lock(MUTEX_CONTEXT);
        SessionCastEntry entry(id, srcEp->GetUniqueName(), destB2bEp, destEp);
        sessionCastSet.insert(entry);
        if (srcB2bEp) {
            sessionCastSet.insert(SessionCastEntry(id, destEp->GetUniqueName(), *srcB2bEp, srcEp));
        } else {
            RemoteEndpoint none;
            sessionCastSet.insert(SessionCastEntry(id, destEp->GetUniqueName(), none, srcEp));
        }
        sessionCastSetLock.Unlock(MUTEX_CONTEXT);
    }
    return status;
}

QStatus DaemonRouter::RemoveSessionRoute(SessionId id, BusEndpoint& srcEp, BusEndpoint& destEp)
{
    QStatus status = ER_OK;
    RemoteEndpoint srcB2bEp;
    RemoteEndpoint destB2bEp;
    if (id == 0) {
        return ER_BUS_NO_SESSION;
    }

    /* Update virtual endpoint state */
    if (destEp->GetEndpointType() == ENDPOINT_TYPE_VIRTUAL) {
        VirtualEndpoint vDestEp = VirtualEndpoint::cast(destEp);
        destB2bEp = vDestEp->GetBusToBusEndpoint(id);
        vDestEp->RemoveSessionRef(id);
    }
    if (srcEp->GetEndpointType() == ENDPOINT_TYPE_VIRTUAL) {
        VirtualEndpoint vSrcEp = VirtualEndpoint::cast(srcEp);
        srcB2bEp = vSrcEp->GetBusToBusEndpoint(id);
        vSrcEp->RemoveSessionRef(id);
    }

    /* Remove entries from sessionCastSet */
    if (status == ER_OK) {
        sessionCastSetLock.Lock(MUTEX_CONTEXT);
        SessionCastEntry entry(id, srcEp->GetUniqueName(), destB2bEp, destEp);
        set<SessionCastEntry>::iterator it = sessionCastSet.find(entry);
        if (it != sessionCastSet.end()) {
            sessionCastSet.erase(it);
        }

        SessionCastEntry entry2(id, destEp->GetUniqueName(), srcB2bEp, srcEp);
        set<SessionCastEntry>::iterator it2 = sessionCastSet.find(entry2);
        if (it2 != sessionCastSet.end()) {
            sessionCastSet.erase(it2);
        }
        sessionCastSetLock.Unlock(MUTEX_CONTEXT);
    }
    return status;
}

void DaemonRouter::RemoveSessionRoutes(const char* src, SessionId id)
{
    String srcStr = src;
    BusEndpoint ep = FindEndpoint(srcStr);

    sessionCastSetLock.Lock(MUTEX_CONTEXT);
    set<SessionCastEntry>::const_iterator it = sessionCastSet.begin();
    while (it != sessionCastSet.end()) {
        if (((it->id == id) || (id == 0)) && ((it->src == src) || (it->destEp == ep))) {
            if ((it->id != 0) && (it->destEp->GetEndpointType() == ENDPOINT_TYPE_VIRTUAL)) {
                BusEndpoint destEp = it->destEp;
                VirtualEndpoint::cast(destEp)->RemoveSessionRef(it->id);
            }
            sessionCastSet.erase(it++);
        } else {
            ++it;
        }
    }
    sessionCastSetLock.Unlock(MUTEX_CONTEXT);
}

}
