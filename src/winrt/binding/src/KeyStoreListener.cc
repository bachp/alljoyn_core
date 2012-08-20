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

#include "KeyStoreListener.h"

#include <BusAttachment.h>
#include <qcc/String.h>
#include <qcc/winrt/utility.h>
#include <ObjectReference.h>
#include <AllJoynException.h>

namespace AllJoyn {

KeyStoreListener::KeyStoreListener(BusAttachment ^ bus)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == bus) {
            status = ER_BAD_ARG_1;
            break;
        }
        _KeyStoreListener* ksl = new _KeyStoreListener(bus);
        if (NULL == ksl) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mListener = new qcc::ManagedObj<_KeyStoreListener>(ksl);
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

KeyStoreListener::KeyStoreListener(const qcc::ManagedObj<_KeyStoreListener>* listener)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == listener) {
            status = ER_BAD_ARG_1;
            break;
        }
        _mListener = new qcc::ManagedObj<_KeyStoreListener>(*listener);
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

KeyStoreListener::~KeyStoreListener()
{
    if (NULL != _mListener) {
        delete _mListener;
        _mListener = NULL;
        _listener = NULL;
    }
}

Windows::Foundation::EventRegistrationToken KeyStoreListener::GetKeys::add(KeyStoreListenerGetKeysHandler ^ handler)
{
    return _listener->_eventsAndProperties->GetKeys::add(handler);
}

void KeyStoreListener::GetKeys::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->GetKeys::remove(token);
}

Platform::String ^ KeyStoreListener::GetKeys::raise()
{
    return _listener->_eventsAndProperties->GetKeys::raise();
}

Windows::Foundation::EventRegistrationToken KeyStoreListener::GetPassword::add(KeyStoreListenerGetPasswordHandler ^ handler)
{
    return _listener->_eventsAndProperties->GetPassword::add(handler);
}

void KeyStoreListener::GetPassword::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->GetPassword::remove(token);
}

Platform::String ^ KeyStoreListener::GetPassword::raise()
{
    return _listener->_eventsAndProperties->GetPassword::raise();
}

Windows::Foundation::EventRegistrationToken KeyStoreListener::PutKeys::add(KeyStoreListenerPutKeysHandler ^ handler)
{
    return _listener->_eventsAndProperties->PutKeys::add(handler);
}

void KeyStoreListener::PutKeys::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->PutKeys::remove(token);
}

void KeyStoreListener::PutKeys::raise(Platform::String ^ keys)
{
    return _listener->_eventsAndProperties->PutKeys::raise(keys);
}

BusAttachment ^ KeyStoreListener::Bus::get()
{
    return _listener->_eventsAndProperties->Bus;
}

_KeyStoreListener::_KeyStoreListener(BusAttachment ^ bus)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __KeyStoreListener();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->GetKeys += ref new KeyStoreListenerGetKeysHandler([&] ()->Platform::String ^ {
                                                                                    return DefaultKeyStoreListenerGetKeysHandler();
                                                                                });
        _eventsAndProperties->GetPassword += ref new KeyStoreListenerGetPasswordHandler([&] ()->Platform::String ^ {
                                                                                            return DefaultKeyStoreListenerGetPasswordHandler();
                                                                                        });
        _eventsAndProperties->PutKeys += ref new KeyStoreListenerPutKeysHandler([&] (Platform::String ^ keys) {
                                                                                    DefaultKeyStoreListenerPutKeysHandler(keys);
                                                                                });
        _eventsAndProperties->Bus = bus;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_KeyStoreListener::~_KeyStoreListener()
{
    _eventsAndProperties = nullptr;
}

Platform::String ^ _KeyStoreListener::DefaultKeyStoreListenerGetKeysHandler()
{
    return nullptr;
}

Platform::String ^ _KeyStoreListener::DefaultKeyStoreListenerGetPasswordHandler()
{
    return nullptr;
}

void _KeyStoreListener::DefaultKeyStoreListenerPutKeysHandler(Platform::String ^ keys)
{
}

::QStatus _KeyStoreListener::LoadRequest(ajn::KeyStore& keyStore)
{
    ::QStatus status = ER_FAIL;

    _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                 while (true) {
                                                                                                                     Platform::String ^ source = _eventsAndProperties->GetKeys();
                                                                                                                     if (nullptr == source) {
                                                                                                                         status = ER_FAIL;
                                                                                                                         break;
                                                                                                                     }
                                                                                                                     qcc::String strSource = PlatformToMultibyteString(source);
                                                                                                                     if (strSource.empty()) {
                                                                                                                         status = ER_OUT_OF_MEMORY;
                                                                                                                         break;
                                                                                                                     }
                                                                                                                     Platform::String ^ password = _eventsAndProperties->GetPassword();
                                                                                                                     if (nullptr == password) {
                                                                                                                         status = ER_FAIL;
                                                                                                                         break;
                                                                                                                     }
                                                                                                                     qcc::String strPassword = PlatformToMultibyteString(password);
                                                                                                                     if (strPassword.empty()) {
                                                                                                                         status = ER_OUT_OF_MEMORY;
                                                                                                                         break;
                                                                                                                     }
                                                                                                                     status = PutKeys(keyStore, strSource, strPassword);
                                                                                                                     break;
                                                                                                                 }
                                                                                                             }));

    return status;
}

::QStatus _KeyStoreListener::StoreRequest(ajn::KeyStore& keyStore)
{
    ::QStatus status = ER_FAIL;

    _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                 while (true) {
                                                                                                                     qcc::String sink;
                                                                                                                     status = GetKeys(keyStore, sink);
                                                                                                                     if (ER_OK != status) {
                                                                                                                         break;
                                                                                                                     }
                                                                                                                     Platform::String ^ strSink = MultibyteToPlatformString(sink.c_str());
                                                                                                                     if (nullptr == strSink && !sink.empty()) {
                                                                                                                         status = ER_OUT_OF_MEMORY;
                                                                                                                         break;
                                                                                                                     }
                                                                                                                     _eventsAndProperties->PutKeys(strSink);
                                                                                                                     status = ER_OK;
                                                                                                                     break;
                                                                                                                 }
                                                                                                             }));

    return status;
}

__KeyStoreListener::__KeyStoreListener()
{
    Bus = nullptr;
}

__KeyStoreListener::~__KeyStoreListener()
{
    Bus = nullptr;
}

}
