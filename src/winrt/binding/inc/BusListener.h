/******************************************************************************
 *
 * Copyright 2011-2012, Qualcomm Innovation Center, Inc.
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
 *
 *****************************************************************************/

#pragma once

#include <alljoyn/BusListener.h>
#include <TransportMaskType.h>
#include <qcc/ManagedObj.h>

namespace AllJoyn {

ref class BusListener;
ref class BusAttachment;

public delegate void BusListenerListenerRegisteredHandler(BusAttachment ^ bus);
public delegate void BusListenerListenerUnregisteredHandler();
public delegate void BusListenerFoundAdvertisedNameHandler(Platform::String ^ name, TransportMaskType transport, Platform::String ^ namePrefix);
public delegate void BusListenerLostAdvertisedNameHandler(Platform::String ^ name, TransportMaskType transport, Platform::String ^ namePrefix);
public delegate void BusListenerNameOwnerChangedHandler(Platform::String ^ busName, Platform::String ^ previousOwner, Platform::String ^ newOwner);
public delegate void BusListenerBusStoppingHandler();
public delegate void BusListenerBusDisconnectedHandler();

ref class __BusListener {
  private:
    friend ref class BusListener;
    friend class _BusListener;
    __BusListener();
    ~__BusListener();

    event BusListenerListenerRegisteredHandler ^ ListenerRegistered;
    event BusListenerListenerUnregisteredHandler ^ ListenerUnregistered;
    event BusListenerFoundAdvertisedNameHandler ^ FoundAdvertisedName;
    event BusListenerLostAdvertisedNameHandler ^ LostAdvertisedName;
    event BusListenerNameOwnerChangedHandler ^ NameOwnerChanged;
    event BusListenerBusStoppingHandler ^ BusStopping;
    event BusListenerBusDisconnectedHandler ^ BusDisconnected;
    property BusAttachment ^ Bus;
};

class _BusListener : protected ajn::BusListener {
  protected:
    friend class qcc::ManagedObj<_BusListener>;
    friend ref class BusListener;
    friend ref class BusAttachment;
    _BusListener(BusAttachment ^ bus);
    ~_BusListener();

    void DefaultBusListenerListenerRegisteredHandler(BusAttachment ^ bus);
    void DefaultBusListenerListenerUnregisteredHandler();
    void DefaultBusListenerFoundAdvertisedNameHandler(Platform::String ^ name, TransportMaskType transport, Platform::String ^ namePrefix);
    void DefaultBusListenerLostAdvertisedNameHandler(Platform::String ^ name, TransportMaskType transport, Platform::String ^ namePrefix);
    void DefaultBusListenerNameOwnerChangedHandler(Platform::String ^ busName, Platform::String ^ previousOwner, Platform::String ^ newOwner);
    void DefaultBusListenerBusStoppingHandler();
    void DefaultBusListenerBusDisconnectedHandler();
    void ListenerRegistered(ajn::BusAttachment* bus);
    void ListenerUnregistered();
    void FoundAdvertisedName(const char* name, ajn::TransportMask transport, const char* namePrefix);
    void LostAdvertisedName(const char* name, ajn::TransportMask transport, const char* namePrefix);
    void NameOwnerChanged(const char* busName, const char* previousOwner, const char* newOwner);
    void BusStopping();
    void BusDisconnected();

    __BusListener ^ _eventsAndProperties;
};

public ref class BusListener sealed {
  public:
    BusListener(BusAttachment ^ bus);

    event BusListenerListenerRegisteredHandler ^ ListenerRegistered
    {
        Windows::Foundation::EventRegistrationToken add(BusListenerListenerRegisteredHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        void raise(BusAttachment ^ bus);
    }

    event BusListenerListenerUnregisteredHandler ^ ListenerUnregistered
    {
        Windows::Foundation::EventRegistrationToken add(BusListenerListenerUnregisteredHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        void raise();
    }

    event BusListenerFoundAdvertisedNameHandler ^ FoundAdvertisedName
    {
        Windows::Foundation::EventRegistrationToken add(BusListenerFoundAdvertisedNameHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        void raise(Platform::String ^ name, TransportMaskType transport, Platform::String ^ namePrefix);
    }

    event BusListenerLostAdvertisedNameHandler ^ LostAdvertisedName
    {
        Windows::Foundation::EventRegistrationToken add(BusListenerLostAdvertisedNameHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        void raise(Platform::String ^ name, TransportMaskType transport, Platform::String ^ namePrefix);
    }

    event BusListenerNameOwnerChangedHandler ^ NameOwnerChanged
    {
        Windows::Foundation::EventRegistrationToken add(BusListenerNameOwnerChangedHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        void raise(Platform::String ^ busName, Platform::String ^ previousOwner, Platform::String ^ newOwner);
    }

    event BusListenerBusStoppingHandler ^ BusStopping
    {
        Windows::Foundation::EventRegistrationToken add(BusListenerBusStoppingHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        void raise();
    }

    event BusListenerBusDisconnectedHandler ^ BusDisconnected
    {
        Windows::Foundation::EventRegistrationToken add(BusListenerBusDisconnectedHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        void raise();
    }

    property BusAttachment ^ Bus
    {
        BusAttachment ^ get();
    }

  private:
    friend ref class BusAttachment;
    BusListener(void* listener, bool isManaged);
    ~BusListener();

    qcc::ManagedObj<_BusListener>* _mListener;
    _BusListener* _listener;
};

}
// BusListener.h
