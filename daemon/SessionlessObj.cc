/**
 * @file
 * * This file implements the org.alljoyn.Bus and org.alljoyn.Daemon interfaces
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

#include <alljoyn/AllJoynStd.h>
#include <alljoyn/Session.h>

#include "SessionlessObj.h"
#include "BusController.h"

#define QCC_MODULE "SESSIONLESS"

using namespace std;
using namespace qcc;

/** Constants */
#define MAX_JOINSESSION_RETRIES 3

/**
 * Inside window calculation.
 * Returns true if p is in range [beg, beg+sz)
 * This function properly accounts for possible wrap-around in [beg, beg+sz) region.
 */
#define IN_WINDOW(tp, beg, sz, p) (((static_cast<tp>((beg) + (sz)) > (beg)) && ((p) >= (beg)) && ((p) < static_cast<tp>((beg) + (sz)))) || \
                                   ((static_cast<tp>((beg) + (sz)) < (beg)) && !(((p) < (beg)) && (p) >= static_cast<tp>((beg) + (sz)))))

/**
 * IS_GREATER_OR_EQUAL returns true if first (non-type) param is greater than or
 * equal to the second while taking into account the possibility of wrap-around
 */
#define IS_GREATER_OR_EQUAL(tp, a, b) IN_WINDOW(tp, (b), (numeric_limits<tp>::max() >> 1), (a))

/**
 * IS_GREATER returns true if first (non-type) param is greater than the second
 * while taking into account the possibility of wrap-around
 */
#define IS_GREATER(tp, a, b) (IS_GREATER_OR_EQUAL(tp, (a), (b)) && ((a) != (b)))

namespace ajn {

/** Constants */
#define SESSIONLESS_SESSION_PORT 100

/** Interface definitions for org.alljoyn.sessionless */
static const char* ObjectPath = "/org/alljoyn/sl";        /**< Object path */
static const char* InterfaceName = "org.alljoyn.sl";      /**< Interface name */
static const char* WellKnownName = "org.alljoyn.sl";      /**< Well known bus name */


SessionlessObj::SessionlessObj(Bus& bus, BusController* busController) :
    BusObject(ObjectPath, false),
    bus(bus),
    busController(busController),
    router(reinterpret_cast<DaemonRouter&>(bus.GetInternal().GetRouter())),
    sessionlessIface(NULL),
    requestSignalsSignal(NULL),
    timer("sessionless"),
    messageMap(),
    ruleCountMap(),
    changeIdMap(),
    lock(),
    nextChangeId(0),
    lastAdvChangeId(-1),
    isDiscoveryStarted(false),
    sessionOpts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY),
    sessionPort(SESSIONLESS_SESSION_PORT)
{
    /* Initialize findPrefix */
    findPrefix = WellKnownName;
    findPrefix.append('.');

    /* Initialize advPrefix */
    advPrefix = findPrefix;
    advPrefix.append('x');
    advPrefix.append(bus.GetGlobalGUIDShortString());
    advPrefix.append(".x");
}

SessionlessObj::~SessionlessObj()
{
    /* Unbind session port */
    bus.UnbindSessionPort(sessionPort);

    /* Unregister bus object */
    bus.UnregisterBusObject(*this);
}

QStatus SessionlessObj::Init()
{
    QCC_DbgTrace(("SessionlessObj::Init"));

    QStatus status;

    /* Create the org.alljoyn.Sessionless interface */
    InterfaceDescription* intf = NULL;
    status = bus.CreateInterface(InterfaceName, intf);
    if (status != ER_OK) {
        QCC_LogError(status, ("Failed to create interface %s", InterfaceName));
        return status;
    }
    intf->AddSignal("RequestSignals", "u", NULL, 0);
    intf->Activate();

    /* Make this object implement org.alljoyn.Sessionless */
    const InterfaceDescription* sessionlessIntf = bus.GetInterface(InterfaceName);
    if (!sessionlessIntf) {
        status = ER_BUS_NO_SUCH_INTERFACE;
        QCC_LogError(status, ("Failed to get %s interface", InterfaceName));
        return status;
    }

    /* Cache requestSignals interface member */
    requestSignalsSignal = sessionlessIntf->GetMember("RequestSignals");
    assert(requestSignalsSignal);

    /* Register a signal handler for requestSignals */
    status = bus.RegisterSignalHandler(this,
                                       static_cast<MessageReceiver::SignalHandler>(&SessionlessObj::RequestSignalsSignalHandler),
                                       requestSignalsSignal,
                                       NULL);
    if (status != ER_OK) {
        QCC_LogError(status, ("Failed to register RequestSignals signal handler"));
    }

    /* Register signal handler for FoundAdvertisedName */
    /* (If we werent in the daemon, we could just use BusListener, but it doesnt work without the full BusAttachment implementation */
    const InterfaceDescription* ajIntf = bus.GetInterface(org::alljoyn::Bus::InterfaceName);
    assert(ajIntf);
    status = bus.RegisterSignalHandler(this,
                                       static_cast<MessageReceiver::SignalHandler>(&SessionlessObj::FoundAdvertisedNameSignalHandler),
                                       ajIntf->GetMember("FoundAdvertisedName"),
                                       NULL);
    if (status != ER_OK) {
        QCC_LogError(status, ("Failed to register FoundAdvertisedName signal handler"));
    }

    /* Register signal handler for SessionLost */
    /* (If we werent in the daemon, we could just use SessionListener, but it doesnt work without the full BusAttachment implementation */
    status = bus.RegisterSignalHandler(this,
                                       static_cast<MessageReceiver::SignalHandler>(&SessionlessObj::SessionLostSignalHandler),
                                       ajIntf->GetMember("SessionLost"),
                                       NULL);
    if (status != ER_OK) {
        QCC_LogError(status, ("Failed to register SessionLost signal handler"));
    }

    /* Start the worker */
    if (status == ER_OK) {
        status = timer.Start();
    }

    /* Bind the session port and establish self as port listener */
    if (status == ER_OK) {
        status = bus.BindSessionPort(sessionPort, sessionOpts, *this);
    }

    /* Register sessionObj */
    if (ER_OK == status) {
        status = bus.RegisterBusObject(*this);
    }

    return status;
}

QStatus SessionlessObj::Stop()
{
    return timer.Stop();
}

QStatus SessionlessObj::Join()
{
    return timer.Join();
}

void SessionlessObj::ObjectRegistered(void)
{
    QCC_DbgTrace(("SessionlessObj::ObjectRegistered"));

    QStatus status;

    /* Acquire org.alljoyn.Sessionless name */
    uint32_t disposition = DBUS_REQUEST_NAME_REPLY_EXISTS;
    status = router.AddAlias(WellKnownName,
                             bus.GetInternal().GetLocalEndpoint()->GetUniqueName(),
                             DBUS_NAME_FLAG_DO_NOT_QUEUE,
                             disposition,
                             NULL,
                             NULL);
    if ((ER_OK != status) || (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != disposition)) {
        status = (ER_OK == status) ? ER_FAIL : status;
        QCC_LogError(status, ("Failed to register well-known name \"%s\" (disposition=%d)", WellKnownName, disposition));
    }

    /* Add a broadcast Rule rule to receive org.alljoyn.Sessionless signals */
    if (status == ER_OK) {
        status = bus.AddMatch("type='signal',interface='org.alljoyn.Sessionless'");
        if (status != ER_OK) {
            QCC_LogError(status, ("Failed to add match rule for org.alljoyn.Sessionless"));
        }
    }

    /* Must call base class */
    BusObject::ObjectRegistered();

    /* Notify parent */
    busController->ObjectRegistered(this);
}

void SessionlessObj::AddRule(const qcc::String& epName, Rule& rule)
{
    QCC_DbgTrace(("SessionlessObj::AddRule(%s, ...)", epName.c_str()));

    if (rule.sessionless == Rule::SESSIONLESS_TRUE) {
        lock.Lock();
        map<String, uint32_t>::iterator it = ruleCountMap.find(epName);
        if (it == ruleCountMap.end()) {
            ruleCountMap.insert(pair<String, int>(epName, 1));
        } else {
            it->second++;
        }

        if (!isDiscoveryStarted) {
            bus.EnableConcurrentCallbacks();
            QStatus status = bus.FindAdvertisedName(findPrefix.c_str());
            if (status != ER_OK) {
                QCC_LogError(status, ("FindAdvertisedName failed"));
            } else {
                isDiscoveryStarted = true;
            }
        }
        lock.Unlock();
    }
}

void SessionlessObj::RemoveRule(const qcc::String& epName, Rule& rule)
{
    QCC_DbgTrace(("SessionlessObj::RemoveRule(%s, ...)", epName.c_str()));

    if (rule.sessionless == Rule::SESSIONLESS_TRUE) {
        lock.Lock();
        map<String, uint32_t>::iterator it = ruleCountMap.find(epName);
        if (it != ruleCountMap.end()) {
            if (--it->second == 0) {
                ruleCountMap.erase(it);
            }
        }

        if (isDiscoveryStarted && ruleCountMap.empty()) {
            bus.EnableConcurrentCallbacks();
            QStatus status = bus.CancelFindAdvertisedName(findPrefix.c_str());
            if (status != ER_OK) {
                QCC_LogError(status, ("CancelFindAdvertisedName failed"));
            }
            isDiscoveryStarted = false;
        }
        lock.Unlock();
    }
}

QStatus SessionlessObj::PushMessage(Message& msg)
{
    QCC_DbgTrace(("SessionlessObj::PushMessage(%s)", msg->ToString().c_str()));

    /* Validate message */
    if (!msg->IsSessionless()) {
        return ER_FAIL;
    }

    /* Put the message in the map and kick the worker */
    MessageMapKey key(msg->GetSender(), msg->GetInterface(), msg->GetMemberName(), msg->GetObjectPath());
    lock.Lock();
    pair<uint32_t, Message> val(nextChangeId++, msg);
    map<MessageMapKey, pair<uint32_t, Message> >::iterator it = messageMap.find(key);
    if (it == messageMap.end()) {
        messageMap.insert(pair<MessageMapKey, pair<uint32_t, Message> >(key, val));
    } else {
        it->second = val;
    }
    lock.Unlock();
    uint32_t zero = 0;
    SessionlessObj* slObj = this;
    QStatus status = timer.AddAlarm(Alarm(zero, slObj));

    return status;
}

void SessionlessObj::NameOwnerChanged(const char* name,
                                      const char* oldOwner,
                                      const char* newOwner)
{
    QCC_DbgTrace(("SessionlessObj::NameOwnerChanged(%s, %s, %s)", name, oldOwner ? oldOwner : "(null)", newOwner ? newOwner : "(null)"));

    /* Remove entries from ruleCountMap for names exiting from the bus */
    if (oldOwner && !newOwner) {
        lock.Lock();
        map<String, uint32_t>::iterator it = ruleCountMap.find(name);
        if (it != ruleCountMap.end()) {
            ruleCountMap.erase(it);
        }

        /* Stop discovery if nobody is looking for sessionless signals */
        if (isDiscoveryStarted && ruleCountMap.empty()) {
            QStatus status = bus.CancelFindAdvertisedName(findPrefix.c_str());
            if (status != ER_OK) {
                QCC_LogError(status, ("CancelFindAdvertisedName failed"));
            }
            isDiscoveryStarted = false;
        }
        lock.Unlock();
    }
}

void SessionlessObj::FoundAdvertisedNameSignalHandler(const InterfaceDescription::Member* member,
                                                      const char* sourcePath,
                                                      Message& msg)
{
    QCC_DbgTrace(("SessionlessObj::FoundAdvertisedNameSignalHandler(...)"));

    /* Parse the args */
    const char* name;
    TransportMask transport;
    const char* prefix;
    QStatus status = msg->GetArgs("sqs", &name, &transport, &prefix);
    if (status != ER_OK) {
        QCC_LogError(status, ("SessionlessObj::FoundAdvNameSigHnd failed to parse msg args"));
    }

    /* Examine found name to see if we need to connect to it */
    String nameStr(name);
    size_t changePos = nameStr.find_last_of('.');
    size_t guidPos = String::npos;
    uint32_t changeId;

    if (changePos != String::npos) {
        changeId = StringToU32(nameStr.substr(changePos + 2), 16);
        guidPos = nameStr.find_last_of('.', changePos);
    }
    if (guidPos == String::npos) {
        QCC_LogError(ER_FAIL, ("Found invalid name \"%s\"", name));
        return;
    }
    String guid = nameStr.substr(guidPos + 2, changePos - guidPos - 2);
    QCC_DbgPrintf(("Found sessionless adv: guid=%s, changeId=%d", guid.c_str(), changeId));

    /* Join session if we need signals from this advertiser and we aren't already getting them */
    lock.Lock();
    map<String, ChangeIdEntry>::iterator it = changeIdMap.find(guid);
    bool updateChangeIdMap = (it == changeIdMap.end()) || IS_GREATER(uint32_t, changeId, it->second.changeId);
    if (updateChangeIdMap) {
        if ((it == changeIdMap.end()) || !it->second.inProgress) {
            /* Attempt to join session with advertised name */
            SessionOpts opts = sessionOpts;
            opts.transports = transport;
            pair<uint32_t, String>* ctx = new pair<uint32_t, String>(changeId, name);
            QStatus status = bus.JoinSessionAsync(name, sessionPort, this, opts, this, reinterpret_cast<void*>(ctx));
            if (status == ER_OK) {
                if (it == changeIdMap.end()) {
                    changeIdMap.insert(pair<String, ChangeIdEntry>(guid, ChangeIdEntry(name, numeric_limits<uint32_t>::max(), true, 0)));
                } else {
                    it->second.advName = name;
                    it->second.inProgress = true;
                    it->second.retries = 0;
                }
            } else {
                QCC_LogError(status, ("JoinSessionAsync failed"));
                delete ctx;
            }
        } else {
            /* Join already in progress so update to advName */
            it->second.advName = name;
            it->second.retries = 0;
        }
    }
    lock.Unlock();
}

bool SessionlessObj::AcceptSessionJoiner(SessionPort port,
                                         const char* joiner,
                                         const SessionOpts& opts)
{
    QCC_DbgTrace(("SessionlessObj::AcceptSessionJoiner(%d, %s, ...)", port, joiner));
    return true;
}

void SessionlessObj::SessionLostSignalHandler(const InterfaceDescription::Member* member,
                                              const char* sourcePath,
                                              Message& msg)
{
    uint32_t sessionId = 0;
    msg->GetArgs("u", &sessionId);
    QCC_DbgTrace(("SessionlessObj::SessionLostSignalHandler(0x%x)", sessionId));
}

void SessionlessObj::RequestSignalsSignalHandler(const InterfaceDescription::Member* member,
                                                 const char* sourcePath,
                                                 Message& msg)
{
    QCC_DbgTrace(("SessionlessObj::RequestSignalsSignalHandler(%s, %s, ...)", member->name.c_str(), sourcePath));

    /* Enable concurrency since PushMessage could block */
    bus.EnableConcurrentCallbacks();

    /* Send all messages in messageMap whose changeId is >= requested changeId */
    if (msg->GetType() == MESSAGE_SIGNAL) {
        uint32_t fromChangeId;
        QStatus status = msg->GetArgs("u", &fromChangeId);
        if (status == ER_OK) {
            lock.Lock();
            map<MessageMapKey, pair<uint32_t, Message> >::iterator it = messageMap.begin();
            while (it != messageMap.end()) {
                if (IS_GREATER_OR_EQUAL(uint32_t, it->second.first, fromChangeId)) {
                    MessageMapKey key = it->first;
                    lock.Unlock();
                    router.LockNameTable();
                    BusEndpoint ep = router.FindEndpoint(msg->GetSender());
                    if (ep->IsValid()) {
                        router.UnlockNameTable();
                        if (ep->GetEndpointType() == ENDPOINT_TYPE_VIRTUAL) {
                            status = VirtualEndpoint::cast(ep)->PushMessage(it->second.second, msg->GetSessionId());
                        } else {
                            status = ep->PushMessage(it->second.second);
                        }
                    } else {
                        router.UnlockNameTable();
                    }
                    lock.Lock();
                    it = messageMap.upper_bound(key);
                    if (status != ER_OK) {
                        QCC_LogError(status, ("Failed to push sessionless signal to %s", msg->GetDestination()));
                    }
                } else {
                    ++it;
                }
            }
            lock.Unlock();
            /* Close the session */
            status = bus.LeaveSession(msg->GetSessionId());
            if (status != ER_OK) {
                QCC_LogError(status, ("LeaveSession failed"));
            }
        }
    }
}


void SessionlessObj::AlarmTriggered(const Alarm& alarm, QStatus reason)
{
    QCC_DbgTrace(("SessionlessObj::AlarmTriggered(alarm, %s)", QCC_StatusText(reason)));

    QStatus status;

    if (reason == ER_OK) {
        uint32_t tilExpire = ::numeric_limits<uint32_t>::max();
        uint32_t expire;
        uint32_t maxChangeId = 0;
        bool mapIsEmpty = true;

        /* Purge the messageMap of expired messages */
        lock.Lock();
        map<MessageMapKey, pair<uint32_t, Message> >::iterator it = messageMap.begin();
        while (it != messageMap.end()) {
            if (it->second.second->IsExpired(&expire)) {
                messageMap.erase(it++);
            } else {
                maxChangeId = max(maxChangeId, it->second.first);
                tilExpire = min(tilExpire, expire);
                mapIsEmpty = false;
                ++it;
            }
        }
        lock.Unlock();

        /* Change advertisment if map is empty or if maxChangeId > lastAdvChangeId */
        if (mapIsEmpty || IS_GREATER(uint32_t, maxChangeId, lastAdvChangeId)) {

            /* Cancel previous advertisment */
            if (!lastAdvName.empty()) {
                status = bus.CancelAdvertiseName(lastAdvName.c_str(), TRANSPORT_ANY & ~TRANSPORT_ICE & ~TRANSPORT_LOCAL);
                if (status != ER_OK) {
                    QCC_LogError(status, ("Failed to cancel advertisment for \"%s\"", lastAdvName.c_str()));
                }

                /* Cancel previous name */
                status = bus.ReleaseName(lastAdvName.c_str());
                if (status != ER_OK) {
                    QCC_LogError(status, ("Failed to release name \"%s\"", lastAdvName.c_str()));
                }
            }

            /* Acqure new name and advertise */
            if (!mapIsEmpty) {
                lastAdvName = advPrefix + U32ToString(maxChangeId, 16);

                status = bus.RequestName(lastAdvName.c_str(), DBUS_NAME_FLAG_DO_NOT_QUEUE);
                if (status == ER_OK) {
                    status = bus.AdvertiseName(lastAdvName.c_str(), TRANSPORT_ANY & ~TRANSPORT_ICE & ~TRANSPORT_LOCAL);
                }

                if (status != ER_OK) {
                    QCC_LogError(status, ("Failed to request/advertise \"%s\"", lastAdvName.c_str()));
                    lastAdvName.clear();
                }
                lastAdvChangeId = maxChangeId;
            } else {
                /* Map is empty. No advertisment. */
                lastAdvName.clear();
            }
        }

        /* Rearm alarm */
        if (tilExpire != ::numeric_limits<uint32_t>::max()) {
            SessionlessObj* slObj = this;
            timer.AddAlarm(Alarm(tilExpire, slObj));
        }
    }
}

void SessionlessObj::JoinSessionCB(QStatus status, SessionId id, const SessionOpts& opts, void* context)
{
    pair<uint32_t, String>* ctx1 = reinterpret_cast<pair<uint32_t, String>*>(context);

    QCC_DbgTrace(("SessionlessObj::JoinSessionCB(%s, 0x%x, creator=%s, changeId=%d)", QCC_StatusText(status), id, ctx1->second.c_str(), ctx1->first));

    /* Extract guid from creator name */
    String guid;
    size_t changePos = ctx1->second.find_last_of('.');
    if (changePos != String::npos) {
        size_t guidPos = ctx1->second.find_last_of('.', changePos);
        if (guidPos != String::npos) {
            guid = ctx1->second.substr(guidPos + 2, changePos - guidPos - 2);
        }
    }
    if (guid.empty()) {
        QCC_LogError(ER_FAIL, ("Cant extract guid from name \"%s\"", ctx1->second.c_str()));
        return;
    }

    /* Send out RequestSignals message if join was successful. Otherwise retry. */
    lock.Lock();
    map<String, ChangeIdEntry>::iterator cit = changeIdMap.find(guid);
    if (cit != changeIdMap.end()) {
        String advName = cit->second.advName;
        uint32_t requestChangeId = cit->second.changeId + 1;
        if (status == ER_OK) {
            /* Update changeIdMap */
            cit->second.changeId = ctx1->first;
            cit->second.inProgress = false;
        } else {
            /* Retry JoinSession if retries aren't exhausted */
            if (cit->second.retries++ < MAX_JOINSESSION_RETRIES) {
                pair<uint32_t, String>* ctx2 = new pair<uint32_t, String>(ctx1->first, advName);
                QStatus tStatus = bus.JoinSessionAsync(advName.c_str(), sessionPort, this, opts, this, reinterpret_cast<void*>(ctx2));
                if (tStatus == ER_OK) {
                    QCC_DbgPrintf(("Retrying joinsession failure (%s)", QCC_StatusText(status)));
                } else {
                    QCC_LogError(tStatus, ("JoinSessionAsync to %s failed", ctx2->second.c_str()));
                    delete ctx2;
                }
            } else {
                cit->second.inProgress = false;
                QCC_LogError(status, ("Exhausted joinSession retries to %s", advName.c_str()));
            }
        }
        lock.Unlock();

        if (status == ER_OK) {
            /* Add/replace sessionless adv name for remote daemon */
            String guid = advName.substr(::strlen(WellKnownName) + 2, qcc::GUID128::SHORT_SIZE);
            busController->GetAllJoynObj().SetAdvNameAlias(guid, opts.transports, advName);

            /* Send the signal if join was successful */
            MsgArg args[1];
            args[0].Set("u", requestChangeId);
            QCC_DbgPrintf(("Sending RequestSignals (changeId=%d) to %s\n", ctx1->first, advName.c_str()));
            status = Signal(advName.c_str(), id, *requestSignalsSignal, args, ArraySize(args));
            if (status != ER_OK) {
                QCC_LogError(status, ("Failed to send signal to %s", advName.c_str()));
            }
        }
    } else {
        lock.Unlock();
        QCC_LogError(ER_FAIL, ("Missing entry in changeIdMap for %s", guid.c_str()));
    }

    delete ctx1;
}

}
