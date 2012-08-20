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

#include "SessionListener.h"

#include <qcc/String.h>
#include <qcc/winrt/utility.h>
#include <ObjectReference.h>
#include <AllJoynException.h>
#include <BusAttachment.h>
#include <Status.h>

namespace AllJoyn {

SessionListener::SessionListener(BusAttachment ^ bus)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == bus) {
            status = ER_BAD_ARG_1;
            break;
        }
        _SessionListener* sl = new _SessionListener(bus);
        if (NULL == sl) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mListener = new qcc::ManagedObj<_SessionListener>(sl);
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

SessionListener::SessionListener(const qcc::ManagedObj<_SessionListener>* listener)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == listener) {
            status = ER_BAD_ARG_1;
            break;
        }
        _mListener = new qcc::ManagedObj<_SessionListener>(*listener);
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

SessionListener::~SessionListener()
{
    if (NULL != _mListener) {
        delete _mListener;
        _mListener = NULL;
        _listener = NULL;
    }
}

Windows::Foundation::EventRegistrationToken SessionListener::SessionLost::add(SessionListenerSessionLostHandler ^ handler)
{
    return _listener->_eventsAndProperties->SessionLost::add(handler);
}

void SessionListener::SessionLost::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->SessionLost::remove(token);
}

void SessionListener::SessionLost::raise(ajn::SessionId sessionId)
{
    return _listener->_eventsAndProperties->SessionLost::raise(sessionId);
}

Windows::Foundation::EventRegistrationToken SessionListener::SessionMemberAdded::add(SessionListenerSessionMemberAddedHandler ^ handler)
{
    return _listener->_eventsAndProperties->SessionMemberAdded::add(handler);
}

void SessionListener::SessionMemberAdded::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->SessionMemberAdded::remove(token);
}

void SessionListener::SessionMemberAdded::raise(ajn::SessionId sessionId, Platform::String ^ uniqueName)
{
    return _listener->_eventsAndProperties->SessionMemberAdded::raise(sessionId, uniqueName);
}

Windows::Foundation::EventRegistrationToken SessionListener::SessionMemberRemoved::add(SessionListenerSessionMemberRemovedHandler ^ handler)
{
    return _listener->_eventsAndProperties->SessionMemberRemoved::add(handler);
}

void SessionListener::SessionMemberRemoved::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->SessionMemberRemoved::remove(token);
}

void SessionListener::SessionMemberRemoved::raise(ajn::SessionId sessionId, Platform::String ^ uniqueName)
{
    return _listener->_eventsAndProperties->SessionMemberRemoved::raise(sessionId, uniqueName);
}

BusAttachment ^ SessionListener::Bus::get()
{
    return _listener->_eventsAndProperties->Bus;
}

_SessionListener::_SessionListener(BusAttachment ^ bus)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __SessionListener();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->SessionLost += ref new SessionListenerSessionLostHandler([&] (ajn::SessionId sessionId) {
                                                                                           DefaultSessionListenerSessionLostHandler(sessionId);
                                                                                       });
        _eventsAndProperties->SessionMemberAdded += ref new SessionListenerSessionMemberAddedHandler([&] (ajn::SessionId sessionId, Platform::String ^ uniqueName) {
                                                                                                         DefaultSessionListenerSessionMemberAddedHandler(sessionId, uniqueName);
                                                                                                     });
        _eventsAndProperties->SessionMemberRemoved += ref new SessionListenerSessionMemberRemovedHandler([&] (ajn::SessionId sessionId, Platform::String ^ uniqueName) {
                                                                                                             DefaultSessionListenerSessionMemberRemovedHandler(sessionId, uniqueName);
                                                                                                         });
        _eventsAndProperties->Bus = bus;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_SessionListener::~_SessionListener()
{
    _eventsAndProperties = nullptr;
}

void _SessionListener::DefaultSessionListenerSessionLostHandler(ajn::SessionId sessionId)
{
    ajn::SessionListener::SessionLost(sessionId);
}

void _SessionListener::DefaultSessionListenerSessionMemberAddedHandler(ajn::SessionId sessionId, Platform::String ^ uniqueName)
{
    ::QStatus status = ER_OK;

    while (true) {
        qcc::String strUniqueName = PlatformToMultibyteString(uniqueName);
        if (nullptr != uniqueName && strUniqueName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::SessionListener::SessionMemberAdded(sessionId, strUniqueName.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void _SessionListener::DefaultSessionListenerSessionMemberRemovedHandler(ajn::SessionId sessionId, Platform::String ^ uniqueName)
{
    ::QStatus status = ER_OK;

    while (true) {
        qcc::String strUniqueName = PlatformToMultibyteString(uniqueName);
        if (nullptr != uniqueName && strUniqueName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::SessionListener::SessionMemberRemoved(sessionId, strUniqueName.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void _SessionListener::SessionLost(ajn::SessionId sessionId)
{
    _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                 _eventsAndProperties->SessionLost(sessionId);
                                                                                                             }));
}

void _SessionListener::SessionMemberAdded(ajn::SessionId sessionId,  const char* uniqueName)
{
    ::QStatus status = ER_OK;

    while (true) {
        Platform::String ^ strUniqueName = MultibyteToPlatformString(uniqueName);
        if (nullptr == strUniqueName && NULL != uniqueName && uniqueName[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     _eventsAndProperties->SessionMemberAdded(sessionId, strUniqueName);
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void _SessionListener::SessionMemberRemoved(ajn::SessionId sessionId,  const char* uniqueName)
{
    ::QStatus status = ER_OK;

    while (true) {
        Platform::String ^ strUniqueName = MultibyteToPlatformString(uniqueName);
        if (nullptr == strUniqueName && NULL != uniqueName && uniqueName[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     _eventsAndProperties->SessionMemberRemoved(sessionId, strUniqueName);
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

__SessionListener::__SessionListener()
{
    Bus = nullptr;
}

__SessionListener::~__SessionListener()
{
    Bus = nullptr;
}

}
