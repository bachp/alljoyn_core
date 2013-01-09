/**
 * @file
 * SessionObj is for implementing the daemon-to-daemon inteface org.alljoyn.Sessionless
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
#ifndef _ALLJOYN_SESSIONLESSOBJ_H
#define _ALLJOYN_SESSIONLESSOBJ_H

#include <qcc/platform.h>

#include <map>
#include <set>

#include <qcc/String.h>
#include <qcc/Timer.h>

#include <alljoyn/BusObject.h>
#include <alljoyn/Message.h>
#include <alljoyn/SessionPortListener.h>
#include <alljoyn/SessionListener.h>

#include "Bus.h"
#include "DaemonRouter.h"
#include "RuleTable.h"
#include "Transport.h"

namespace ajn {

/** Forward Declaration */
class BusController;

/**
 * BusObject responsible for implementing the standard AllJoyn interface org.alljoyn.Sessionless.
 */
class SessionlessObj : public BusObject, public SessionListener, public SessionPortListener,
    public BusAttachment::JoinSessionAsyncCB, public qcc::AlarmListener {

  public:
    /**
     * Constructor
     *
     * @param bus            Bus to associate with org.freedesktop.DBus message handler.
     * @param router         The DaemonRouter associated with the bus.
     * @param busController  Controller that created this object.
     */
    SessionlessObj(Bus& bus, BusController* busController);

    /**
     * Destructor
     */
    ~SessionlessObj();

    /**
     * Initialize and register this DBusObj instance.
     *
     * @return ER_OK if successful.
     */
    QStatus Init();

    /**
     * Stop SessionlessObj.
     *
     * @return ER_OK if successful.
     */
    QStatus Stop();

    /**
     * Join SessionlessObj.
     *
     * @return ER_OK if successful.
     */
    QStatus Join();

    /**
     * Called when object is successfully registered.
     */
    void ObjectRegistered(void);

    /**
     * Add a rule for an endpoint.
     *
     * @param epName   The name of the endpoint that this rule applies to.
     * @param rule     Rule for endpoint
     */
    void AddRule(const qcc::String& epName, Rule& rule);

    /**
     * Remove a rule for an endpoint.
     *
     * @param epName      The name of the endpoint that rule applies to.
     * @param rule        Rule to remove.
     */
    void RemoveRule(const qcc::String& epName, Rule& rule);

    /**
     * Push a sessionless signal.
     *
     * @param msg    Message to be pushed.
     */
    QStatus PushMessage(Message& msg);

    /**
     * NameListener implementation called when a bus name changes ownership.
     *
     * @param busName   Unique or well-known bus name.
     * @param oldOwner  Unique name of old owner of name or NULL if none existed.
     * @param newOwner  Unique name of new owner of name or NULL if none (now) exists.
     */
    void NameOwnerChanged(const char* busName,
                          const char* oldOwner,
                          const char* newOwner);

    /**
     * Receive FoundAdvertisedName signals.
     *
     * @param   member      FoundAdvertisedName interface member.
     * @param   sourcePath  Sender of signal.
     * @param   msg         FoundAdvertisedName message.
     */
    void FoundAdvertisedNameSignalHandler(const InterfaceDescription::Member* member,
                                          const char* sourcePath,
                                          Message& msg);

    /**
     * Accept/reject join attempt on sessionlesss port.
     * Implements SessionPortListener::AceptSessionJoiner.
     *
     * @param port    Session port of join attempt.
     * @param joiner  Unique name of joiner.
     * @param opts    SesionOpts specified by joiner.
     * @return   true if session is accepted. false otherwise.
     */
    bool AcceptSessionJoiner(SessionPort port,
                             const char* joiner,
                             const SessionOpts& opts);

    /**
     * Receive SessionJoined signals.
     *
     * @param   member      FoundAdvertisedName interface member.
     * @param   sourcePath  Sender of signal.
     * @param   msg         FoundAdvertisedName message.
     */
    void SessionJoinedSignalHandler(const InterfaceDescription::Member* member,
                                    const char* sourcePath,
                                    Message& msg);

    /**
     * Receive SessionLost signals.
     *
     * @param   member      SessionLost interface member.
     * @param   sourcePath  Sender of signal.
     * @param   msg         SessionLost message.
     */
    void SessionLostSignalHandler(const InterfaceDescription::Member* member,
                                  const char* sourcePath,
                                  Message& msg);

    /**
     * Process incoming RequestSignals signals from remote daemons.
     *
     * @param member        Interface member for signal
     * @param sourcePath    object path sending the signal.
     * @param msg           The signal message.
     */
    void RequestSignalsSignalHandler(const InterfaceDescription::Member* member,
                                     const char* sourcePath,
                                     Message& msg);

  private:
    /**
     * SessionlessObj worker.
     *
     * @param alarm  The alarm object for the timeout that expired.
     */
    void AlarmTriggered(const qcc::Alarm& alarm, QStatus reason);

    /**
     * JoinSession Callback.
     */
    void JoinSessionCB(QStatus status, SessionId id, const SessionOpts& opts, void* context);

    Bus& bus;                             /**< The bus */
    BusController* busController;         /**< BusController that created this BusObject */
    DaemonRouter& router;                 /**< The router */

    const InterfaceDescription* sessionlessIface;  /**< org.alljoyn.Sessionless interface */

    const InterfaceDescription::Member* requestSignalsSignal;   /**< org.alljoyn.Sessionless.RequestSignal signal */

    qcc::Timer timer;                     /**< Timer object for reaping expired names */

    /* Class used as key for messageMap */
    class MessageMapKey : public qcc::String {
      public:
        MessageMapKey(const char* sender, const char* iface, const char* member) :
            qcc::String(sender, 0, ::strlen(sender) + ::strlen(iface) + ::strlen(member) + 3)
        {
            append(':');
            append(iface);
            append(':');
            append(member);
        }
    };

    /** Storage for sessionless messages waiting to be delivered */
    std::map<MessageMapKey, std::pair<uint32_t, Message> > messageMap;

    /** Count the number of rules (per endpoint) that specify sesionless=TRUE */
    std::map<qcc::String, uint32_t> ruleCountMap;

    /** Track the changeIds of the advertisments coming from other daemons */
    std::map<qcc::String, uint32_t> changeIdMap;

    /** Map of sessionIds to the advertised changeIds that are currently being joined */
    std::map<uint32_t, uint32_t> joinsInProgress;

    qcc::Mutex lock;            /**< Mutex that protects messageMap this obj's data structures */
    uint32_t nextChangeId;      /**< Change id assoc with next pushed signal */
    uint32_t lastAdvChangeId;   /**< Last advertised change id */
    qcc::String lastAdvName;    /**< Last advertised name */
    qcc::String findPrefix;     /**< FindAdvertiseName prefix */
    qcc::String advPrefix;      /**< AdvertiseName prefix */
    bool isDiscoveryStarted;    /**< True when FindAdvetiseName is ongoing */
    SessionOpts sessionOpts;    /**< SessionOpts used by internal session */
    SessionPort sessionPort;    /**< SessionPort used by internal session */
};

}

#endif
