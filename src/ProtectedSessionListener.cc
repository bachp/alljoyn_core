/**
 * @file
 * ProtectedSessionListener
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

#include "ProtectedSessionListener.h"

using namespace ajn;

ProtectedSessionListener::ProtectedSessionListener(SessionListener* _listener)
    : listener(NULL), lock(), refCount(0)
{
    Set(_listener);
}

ProtectedSessionListener::~ProtectedSessionListener()
{
    Set(NULL);
}


void ProtectedSessionListener::Set(SessionListener* _listener)
{
    lock.Lock(MUTEX_CONTEXT);
    /*
     * Clear the current listener to prevent any more calls to this listener.
     */
    this->listener = NULL;
    /*
     * Poll and sleep until the current listener is no longer in use.
     */
    while (refCount) {
        lock.Unlock(MUTEX_CONTEXT);
        qcc::Sleep(10);
        lock.Lock(MUTEX_CONTEXT);
    }
    /*
     * Now set the new listener
     */
    this->listener = _listener;
    lock.Unlock(MUTEX_CONTEXT);
}


void ProtectedSessionListener::SessionLost(SessionId sessionId)
{
    lock.Lock(MUTEX_CONTEXT);
    SessionListener* listener = this->listener;
    ++refCount;
    lock.Unlock(MUTEX_CONTEXT);
    if (listener) {
        listener->SessionLost(sessionId);
    }
    lock.Lock(MUTEX_CONTEXT);
    --refCount;
    lock.Unlock(MUTEX_CONTEXT);
}


void ProtectedSessionListener::SessionMemberAdded(SessionId sessionId, const char* uniqueName)
{
    lock.Lock(MUTEX_CONTEXT);
    SessionListener* listener = this->listener;
    ++refCount;
    lock.Unlock(MUTEX_CONTEXT);
    if (listener) {
        listener->SessionMemberAdded(sessionId, uniqueName);
    }
    lock.Lock(MUTEX_CONTEXT);
    --refCount;
    lock.Unlock(MUTEX_CONTEXT);
}


void ProtectedSessionListener::SessionMemberRemoved(SessionId sessionId, const char* uniqueName)
{
    lock.Lock(MUTEX_CONTEXT);
    SessionListener* listener = this->listener;
    ++refCount;
    lock.Unlock(MUTEX_CONTEXT);
    if (listener) {
        listener->SessionMemberRemoved(sessionId, uniqueName);
    }
    lock.Lock(MUTEX_CONTEXT);
    --refCount;
    lock.Unlock(MUTEX_CONTEXT);
}
