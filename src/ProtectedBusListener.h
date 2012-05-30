#ifndef _PROTECTED_BUS_LISTENER_H
#define _PROTECTED_BUS_LISTENER_H
/**
 * @file
 *
 * This file defines internal state for a ProtectedBusListener
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
#error Only include ProtectedBusListener.h in C++ code.
#endif

#include <qcc/platform.h>
#include <qcc/Mutex.h>
#include <qcc/String.h>
#include <qcc/Thread.h>
#include <alljoyn/BusListener.h>

namespace ajn {

class ProtectedBusListener {
  public:
    ProtectedBusListener(BusListener* listener);
    ~ProtectedBusListener();

    /**
     * Set the listener. If one of internal listener callouts is currently being called this
     * function will block until the callout returns.
     */
    void Set(BusListener* listener);

    /**
     * Called by the bus when the listener is registered. This give the listener implementation the
     * opportunity to save a reference to the bus.
     *
     * @param bus  The bus the listener is registered with.
     */
    void ListenerRegistered(BusAttachment* bus);

    /**
     * Called by the bus when the listener is unregistered.
     */
    void ListenerUnregistered();

    /**
     * Called by the bus when an external bus is discovered that is advertising a well-known name
     * that this attachment has registered interest in via a DBus call to org.alljoyn.Bus.FindAdvertisedName
     *
     * @param name         A well known name that the remote bus is advertising.
     * @param transport    Transport that received the advertisement.
     * @param namePrefix   The well-known name prefix used in call to FindAdvertisedName that triggered this callback.
     */
    void FoundAdvertisedName(const char* name, TransportMask transport, const char* namePrefix);

    /**
     * Called by the bus when an advertisement previously reported through FoundName has become unavailable.
     *
     * @param name         A well known name that the remote bus is advertising that is of interest to this attachment.
     * @param transport    Transport that stopped receiving the given advertised name.
     * @param namePrefix   The well-known name prefix that was used in a call to FindAdvertisedName that triggered this callback.
     */
    void LostAdvertisedName(const char* name, TransportMask transport, const char* namePrefix);

    /**
     * Called by the bus when the ownership of any well-known name changes.
     *
     * @param busName        The well-known name that has changed.
     * @param previousOwner  The unique name that previously owned the name or NULL if there was no previous owner.
     * @param newOwner       The unique name that now owns the name or NULL if the there is no new owner.
     */
    void NameOwnerChanged(const char* busName, const char* previousOwner, const char* newOwner);

    /**
     * Called when a BusAttachment this listener is registered with is stopping.
     */
    void BusStopping();

    /**
     * Called when a BusAttachment this listener is registered with is has become disconnected from
     * the bus.
     */
    void BusDisconnected();

  private:
    BusListener* listener;
    qcc::Mutex lock;
    int32_t refCount;
};

}

#endif
