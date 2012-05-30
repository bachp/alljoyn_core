#ifndef _PROTECTED_SESSION_LISTENER_H
#define _PROTECTED_SESSION_LISTENER_H
/**
 * @file
 *
 * This file defines internal state for a ProtectedSessionListener
 */

/******************************************************************************
 * Copyright 2009-2011, Qualcomm Innovation Center, Inc.
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

#ifndef __cplusplus
#error Only include BusInternal.h in C++ code.
#endif

#include <qcc/platform.h>
#include <qcc/Mutex.h>
#include <qcc/String.h>
#include <qcc/Thread.h>
#include <alljoyn/SessionListener.h>

namespace ajn {

class ProtectedSessionListener {
  public:

    ProtectedSessionListener(SessionListener* listener);
    ~ProtectedSessionListener();

    /**
     * Set the listener. If one of internal listener callouts is currently being called this
     * function will block until the callout returns.
     */
    void Set(SessionListener* listener);

    /**
     * Called by the bus when an existing session becomes disconnected.
     *
     * @param sessionId     Id of session that was lost.
     */
    void SessionLost(SessionId sessionId);

    /**
     * Called by the bus when a member of a multipoint session is added.
     *
     * @param sessionId     Id of session whose member(s) changed.
     * @param uniqueName    Unique name of member who was added.
     */
    void SessionMemberAdded(SessionId sessionId, const char* uniqueName);

    /**
     * Called by the bus when a member of a multipoint session is removed.
     *
     * @param sessionId     Id of session whose member(s) changed.
     * @param uniqueName    Unique name of member who was removed.
     */
    void SessionMemberRemoved(SessionId sessionId, const char* uniqueName);

  private:
    SessionListener* listener;
    qcc::Mutex lock;
    int32_t refCount;
};

}

#endif
