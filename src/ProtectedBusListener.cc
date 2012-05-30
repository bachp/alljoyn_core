/**
 * @file
 * ProtectedBusListener
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

#include "ProtectedBusListener.h"

using namespace ajn;


ProtectedBusListener::ProtectedBusListener(BusListener* _listener)
    : listener(NULL), lock(), refCount(0)
{
    Set(_listener);
}


ProtectedBusListener::~ProtectedBusListener()
{
    Set(NULL);
}

void ProtectedBusListener::Set(BusListener* _listener)
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


void ProtectedBusListener::ListenerRegistered(BusAttachment* bus)
{
    lock.Lock(MUTEX_CONTEXT);
    BusListener* listener = this->listener;
    ++refCount;
    lock.Unlock(MUTEX_CONTEXT);
    if (listener) {
        listener->ListenerRegistered(bus);
    }
    lock.Lock(MUTEX_CONTEXT);
    --refCount;
    lock.Unlock(MUTEX_CONTEXT);
}

void ProtectedBusListener::ListenerUnregistered()
{
    lock.Lock(MUTEX_CONTEXT);
    BusListener* listener = this->listener;
    ++refCount;
    lock.Unlock(MUTEX_CONTEXT);
    if (listener) {
        listener->ListenerUnregistered();
    }
    lock.Lock(MUTEX_CONTEXT);
    --refCount;
    lock.Unlock(MUTEX_CONTEXT);
}


void ProtectedBusListener::FoundAdvertisedName(const char* name, TransportMask transport, const char* namePrefix)
{
    lock.Lock(MUTEX_CONTEXT);
    BusListener* listener = this->listener;
    ++refCount;
    lock.Unlock(MUTEX_CONTEXT);
    if (listener) {
        listener->FoundAdvertisedName(name, transport, namePrefix);
    }
    lock.Lock(MUTEX_CONTEXT);
    --refCount;
    lock.Unlock(MUTEX_CONTEXT);
}


void ProtectedBusListener::LostAdvertisedName(const char* name, TransportMask transport, const char* namePrefix)
{
    lock.Lock(MUTEX_CONTEXT);
    BusListener* listener = this->listener;
    ++refCount;
    lock.Unlock(MUTEX_CONTEXT);
    if (listener) {
        listener->LostAdvertisedName(name, transport, namePrefix);
    }
    lock.Lock(MUTEX_CONTEXT);
    --refCount;
    lock.Unlock(MUTEX_CONTEXT);
}


void ProtectedBusListener::NameOwnerChanged(const char* busName, const char* previousOwner, const char* newOwner)
{
    lock.Lock(MUTEX_CONTEXT);
    BusListener* listener = this->listener;
    ++refCount;
    lock.Unlock(MUTEX_CONTEXT);
    if (listener) {
        listener->NameOwnerChanged(busName, previousOwner, newOwner);
    }
    lock.Lock(MUTEX_CONTEXT);
    --refCount;
    lock.Unlock(MUTEX_CONTEXT);
}


void ProtectedBusListener::BusStopping()
{
    lock.Lock(MUTEX_CONTEXT);
    BusListener* listener = this->listener;
    ++refCount;
    lock.Unlock(MUTEX_CONTEXT);
    if (listener) {
        listener->BusStopping();
    }
    lock.Lock(MUTEX_CONTEXT);
    --refCount;
    lock.Unlock(MUTEX_CONTEXT);
}


void ProtectedBusListener::BusDisconnected()
{
    lock.Lock(MUTEX_CONTEXT);
    BusListener* listener = this->listener;
    ++refCount;
    lock.Unlock(MUTEX_CONTEXT);
    if (listener) {
        listener->BusDisconnected();
    }
    lock.Lock(MUTEX_CONTEXT);
    --refCount;
    lock.Unlock(MUTEX_CONTEXT);
}

