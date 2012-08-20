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

#include "BusListener.h"

#include <qcc/String.h>
#include <BusAttachment.h>
#include <qcc/winrt/utility.h>
#include <ObjectReference.h>
#include <AllJoynException.h>
#include <ctxtcall.h>
#include <ppltasks.h>

namespace AllJoyn {

BusListener::BusListener(BusAttachment ^ bus)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == bus) {
            status = ER_BAD_ARG_1;
            break;
        }
        _BusListener* bl = new _BusListener(bus);
        if (NULL == bl) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mListener = new qcc::ManagedObj<_BusListener>(bl);
        if (NULL == _mListener) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _listener = &(**_mListener);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

BusListener::BusListener(const qcc::ManagedObj<_BusListener>* listener)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == listener) {
            status = ER_BAD_ARG_1;
            break;
        }
        _mListener = new qcc::ManagedObj<_BusListener>(*listener);
        if (NULL == _mListener) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _listener = &(**_mListener);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

BusListener::~BusListener()
{
    if (NULL != _mListener) {
        delete _mListener;
        _mListener = NULL;
        _listener = NULL;
    }
}

Windows::Foundation::EventRegistrationToken BusListener::ListenerRegistered::add(BusListenerListenerRegisteredHandler ^ handler)
{
    return _listener->_eventsAndProperties->ListenerRegistered::add(handler);
}

void BusListener::ListenerRegistered::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->ListenerRegistered::remove(token);
}

void BusListener::ListenerRegistered::raise(BusAttachment ^ bus)
{
    return _listener->_eventsAndProperties->ListenerRegistered::raise(bus);
}

Windows::Foundation::EventRegistrationToken BusListener::ListenerUnregistered::add(BusListenerListenerUnregisteredHandler ^ handler)
{
    return _listener->_eventsAndProperties->ListenerUnregistered::add(handler);
}

void BusListener::ListenerUnregistered::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->ListenerUnregistered::remove(token);
}

void BusListener::ListenerUnregistered::raise()
{
    return _listener->_eventsAndProperties->ListenerUnregistered::raise();
}

Windows::Foundation::EventRegistrationToken BusListener::FoundAdvertisedName::add(BusListenerFoundAdvertisedNameHandler ^ handler)
{
    return _listener->_eventsAndProperties->FoundAdvertisedName::add(handler);
}

void BusListener::FoundAdvertisedName::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->FoundAdvertisedName::remove(token);
}

void BusListener::FoundAdvertisedName::raise(Platform::String ^ name, TransportMaskType transport, Platform::String ^ namePrefix)
{
    return _listener->_eventsAndProperties->FoundAdvertisedName::raise(name, transport, namePrefix);
}

Windows::Foundation::EventRegistrationToken BusListener::LostAdvertisedName::add(BusListenerLostAdvertisedNameHandler ^ handler)
{
    return _listener->_eventsAndProperties->LostAdvertisedName::add(handler);
}

void BusListener::LostAdvertisedName::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->LostAdvertisedName::remove(token);
}

void BusListener::LostAdvertisedName::raise(Platform::String ^ name, TransportMaskType transport, Platform::String ^ namePrefix)
{
    return _listener->_eventsAndProperties->LostAdvertisedName::raise(name, transport, namePrefix);
}

Windows::Foundation::EventRegistrationToken BusListener::NameOwnerChanged::add(BusListenerNameOwnerChangedHandler ^ handler)
{
    return _listener->_eventsAndProperties->NameOwnerChanged::add(handler);
}

void BusListener::NameOwnerChanged::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->NameOwnerChanged::remove(token);
}

void BusListener::NameOwnerChanged::raise(Platform::String ^ busName, Platform::String ^ previousOwner, Platform::String ^ newOwner)
{
    return _listener->_eventsAndProperties->NameOwnerChanged::raise(busName, previousOwner, newOwner);
}

Windows::Foundation::EventRegistrationToken BusListener::BusStopping::add(BusListenerBusStoppingHandler ^ handler)
{
    return _listener->_eventsAndProperties->BusStopping::add(handler);
}

void BusListener::BusStopping::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->BusStopping::remove(token);
}

void BusListener::BusStopping::raise()
{
    return _listener->_eventsAndProperties->BusStopping::raise();
}

Windows::Foundation::EventRegistrationToken BusListener::BusDisconnected::add(BusListenerBusDisconnectedHandler ^ handler)
{
    return _listener->_eventsAndProperties->BusDisconnected::add(handler);
}

void BusListener::BusDisconnected::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->BusDisconnected::remove(token);
}

void BusListener::BusDisconnected::raise()
{
    return _listener->_eventsAndProperties->BusDisconnected::raise();
}

BusAttachment ^ BusListener::Bus::get()
{
    return _listener->_eventsAndProperties->Bus;
}

_BusListener::_BusListener(BusAttachment ^ bus)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __BusListener();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->ListenerRegistered += ref new BusListenerListenerRegisteredHandler([&] (BusAttachment ^ bus) {
                                                                                                     DefaultBusListenerListenerRegisteredHandler(bus);
                                                                                                 });
        _eventsAndProperties->ListenerUnregistered += ref new BusListenerListenerUnregisteredHandler([&] () {
                                                                                                         DefaultBusListenerListenerUnregisteredHandler();
                                                                                                     });
        _eventsAndProperties->FoundAdvertisedName += ref new BusListenerFoundAdvertisedNameHandler([&] (Platform::String ^ name, TransportMaskType transport, Platform::String ^ namePrefix) {
                                                                                                       DefaultBusListenerFoundAdvertisedNameHandler(name, transport, namePrefix);
                                                                                                   });
        _eventsAndProperties->LostAdvertisedName += ref new BusListenerLostAdvertisedNameHandler([&] (Platform::String ^ name, TransportMaskType transport, Platform::String ^ namePrefix) {
                                                                                                     DefaultBusListenerLostAdvertisedNameHandler(name, transport, namePrefix);
                                                                                                 });
        _eventsAndProperties->NameOwnerChanged += ref new BusListenerNameOwnerChangedHandler([&] (Platform::String ^ busName, Platform::String ^ previousOwner, Platform::String ^ newOwner) {
                                                                                                 DefaultBusListenerNameOwnerChangedHandler(busName, previousOwner, newOwner);
                                                                                             });
        _eventsAndProperties->BusStopping += ref new BusListenerBusStoppingHandler([&] () {
                                                                                       DefaultBusListenerBusStoppingHandler();
                                                                                   });
        _eventsAndProperties->BusDisconnected += ref new BusListenerBusDisconnectedHandler([&] () {
                                                                                               DefaultBusListenerBusDisconnectedHandler();
                                                                                           });
        _eventsAndProperties->Bus = bus;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_BusListener::~_BusListener()
{
    _eventsAndProperties = nullptr;
}

void _BusListener::DefaultBusListenerListenerRegisteredHandler(BusAttachment ^ bus)
{
    ajn::BusAttachment* b = bus->_busAttachment;
    ajn::BusListener::ListenerRegistered(b);
}

void _BusListener::DefaultBusListenerListenerUnregisteredHandler()
{
    ajn::BusListener::ListenerUnregistered();
}

void _BusListener::DefaultBusListenerFoundAdvertisedNameHandler(Platform::String ^ name, TransportMaskType transport, Platform::String ^ namePrefix)
{
    ::QStatus status = ER_OK;

    while (true) {
        qcc::String strName = PlatformToMultibyteString(name);
        if (nullptr != name && strName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strNamePrefix = PlatformToMultibyteString(namePrefix);
        if (nullptr != namePrefix && strNamePrefix.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::BusListener::FoundAdvertisedName(strName.c_str(), (ajn::TransportMask)(int)transport, strNamePrefix.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void _BusListener::DefaultBusListenerLostAdvertisedNameHandler(Platform::String ^ name, TransportMaskType transport, Platform::String ^ namePrefix)
{
    ::QStatus status = ER_OK;

    while (true) {
        qcc::String strName = PlatformToMultibyteString(name);
        if (nullptr != name && strName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strNamePrefix = PlatformToMultibyteString(namePrefix);
        if (nullptr != namePrefix && strNamePrefix.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::BusListener::LostAdvertisedName(strName.c_str(), (ajn::TransportMask)(int)transport, strNamePrefix.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void _BusListener::DefaultBusListenerNameOwnerChangedHandler(Platform::String ^ busName, Platform::String ^ previousOwner, Platform::String ^ newOwner)
{
    ::QStatus status = ER_OK;

    while (true) {
        qcc::String strBusName = PlatformToMultibyteString(busName);
        if (nullptr != busName && strBusName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strPreviousOwner = PlatformToMultibyteString(previousOwner);
        if (nullptr != previousOwner && strPreviousOwner.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strNewOwner = PlatformToMultibyteString(newOwner);
        if (nullptr != newOwner && strNewOwner.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::BusListener::NameOwnerChanged(strBusName.c_str(), strPreviousOwner.c_str(), strNewOwner.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void _BusListener::DefaultBusListenerBusStoppingHandler()
{
    ajn::BusListener::BusStopping();
}

void _BusListener::DefaultBusListenerBusDisconnectedHandler()
{
    ajn::BusListener::BusDisconnected();
}

void _BusListener::ListenerRegistered(ajn::BusAttachment* bus)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == bus) {
            status = ER_BAD_ARG_1;
            break;
        }
        AllJoyn::BusAttachment ^ ba = ref new AllJoyn::BusAttachment(bus);
        if (nullptr == ba) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     _eventsAndProperties->ListenerRegistered(ba);
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void _BusListener::ListenerUnregistered()
{
    _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                 _eventsAndProperties->ListenerUnregistered();
                                                                                                             }));
}

void _BusListener::FoundAdvertisedName(const char* name, ajn::TransportMask transport, const char* namePrefix)
{
    ::QStatus status = ER_OK;

    while (true) {
        Platform::String ^ strName = MultibyteToPlatformString(name);
        if (nullptr == strName && name != NULL && name[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        Platform::String ^ strNamePrefix = MultibyteToPlatformString(namePrefix);
        if (nullptr == strNamePrefix && namePrefix != NULL && namePrefix[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     _eventsAndProperties->FoundAdvertisedName(strName, (TransportMaskType)(int)transport, strNamePrefix);
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void _BusListener::LostAdvertisedName(const char* name, ajn::TransportMask transport, const char* namePrefix)
{
    ::QStatus status = ER_OK;

    while (true) {
        Platform::String ^ strName = MultibyteToPlatformString(name);
        if (nullptr == strName && name != NULL && name[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        Platform::String ^ strNamePrefix = MultibyteToPlatformString(namePrefix);
        if (nullptr == strNamePrefix && namePrefix != NULL && namePrefix[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     _eventsAndProperties->LostAdvertisedName(strName, (TransportMaskType)(int)transport, strNamePrefix);
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void _BusListener::NameOwnerChanged(const char* busName, const char* previousOwner, const char* newOwner)
{
    ::QStatus status = ER_OK;

    while (true) {
        Platform::String ^ strBusName = MultibyteToPlatformString(busName);
        if (nullptr == strBusName && busName != NULL && busName[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        Platform::String ^ strPreviousOwner = MultibyteToPlatformString(previousOwner);
        if (nullptr == strPreviousOwner && previousOwner != NULL && previousOwner[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        Platform::String ^ strNewOwner = MultibyteToPlatformString(newOwner);
        if (nullptr == strNewOwner && newOwner != NULL && newOwner[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     _eventsAndProperties->NameOwnerChanged(strBusName, strPreviousOwner, strNewOwner);
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void _BusListener::BusStopping()
{
    _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                 _eventsAndProperties->BusStopping();
                                                                                                             }));
}

void _BusListener::BusDisconnected()
{
    _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                 _eventsAndProperties->BusDisconnected();
                                                                                                             }));
}

__BusListener::__BusListener()
{
    Bus = nullptr;
}

__BusListener::~__BusListener()
{
    Bus = nullptr;
}

}
