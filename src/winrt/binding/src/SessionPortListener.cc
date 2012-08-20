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

#include "SessionPortListener.h"

#include <SessionOpts.h>
#include <ObjectReference.h>
#include <AllJoynException.h>
#include <BusAttachment.h>
#include <Status.h>

namespace AllJoyn {

SessionPortListener::SessionPortListener(BusAttachment ^ bus)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == bus) {
            status = ER_BAD_ARG_1;
            break;
        }
        _SessionPortListener* spl = new _SessionPortListener(bus);
        if (NULL == spl) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mListener = new qcc::ManagedObj<_SessionPortListener>(spl);
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

SessionPortListener::SessionPortListener(const qcc::ManagedObj<_SessionPortListener>* listener)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == listener) {
            status = ER_BAD_ARG_1;
            break;
        }
        _mListener = new qcc::ManagedObj<_SessionPortListener>(*listener);
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

SessionPortListener::~SessionPortListener()
{
    if (NULL != _mListener) {
        delete _mListener;
        _mListener = NULL;
        _listener = NULL;
    }
}

Windows::Foundation::EventRegistrationToken SessionPortListener::AcceptSessionJoiner::add(SessionPortListenerAcceptSessionJoinerHandler ^ handler)
{
    return _listener->_eventsAndProperties->AcceptSessionJoiner::add(handler);
}

void SessionPortListener::AcceptSessionJoiner::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->AcceptSessionJoiner::remove(token);
}

bool SessionPortListener::AcceptSessionJoiner::raise(ajn::SessionPort sessionPort, Platform::String ^ joiner, SessionOpts ^ opts)
{
    return _listener->_eventsAndProperties->AcceptSessionJoiner::raise(sessionPort, joiner, opts);
}

Windows::Foundation::EventRegistrationToken SessionPortListener::SessionJoined::add(SessionPortListenerSessionJoinedHandler ^ handler)
{
    return _listener->_eventsAndProperties->SessionJoined::add(handler);
}

void SessionPortListener::SessionJoined::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->SessionJoined::remove(token);
}

void SessionPortListener::SessionJoined::raise(ajn::SessionPort sessionPort, ajn::SessionId id, Platform::String ^ joiner)
{
    _listener->_eventsAndProperties->SessionJoined::raise(sessionPort, id, joiner);
}

BusAttachment ^ SessionPortListener::Bus::get()
{
    return _listener->_eventsAndProperties->Bus;
}

_SessionPortListener::_SessionPortListener(BusAttachment ^ bus)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __SessionPortListener();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->AcceptSessionJoiner += ref new SessionPortListenerAcceptSessionJoinerHandler([&] (ajn::SessionPort sessionPort, Platform::String ^ joiner, SessionOpts ^ opts)->bool {
                                                                                                               return DefaultSessionPortListenerAcceptSessionJoinerHandler(sessionPort, joiner, opts);
                                                                                                           });
        _eventsAndProperties->SessionJoined += ref new SessionPortListenerSessionJoinedHandler([&] (ajn::SessionPort sessionPort, ajn::SessionId id, Platform::String ^ joiner) {
                                                                                                   DefaultSessionPortListenerSessionJoinedHandler(sessionPort, id, joiner);
                                                                                               });
        _eventsAndProperties->Bus = bus;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_SessionPortListener::~_SessionPortListener()
{
    _eventsAndProperties = nullptr;
}

bool _SessionPortListener::DefaultSessionPortListenerAcceptSessionJoinerHandler(ajn::SessionPort sessionPort, Platform::String ^ joiner, SessionOpts ^ opts)
{
    ::QStatus status = ER_OK;
    bool result = false;

    while (true) {
        qcc::String strJoiner = PlatformToMultibyteString(joiner);
        if (nullptr != joiner && strJoiner.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::SessionOpts* sessionOpts = opts->_sessionOpts;
        result = ajn::SessionPortListener::AcceptSessionJoiner(sessionPort, strJoiner.c_str(), *sessionOpts);
        break;
    }
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
    return result;
}

void _SessionPortListener::DefaultSessionPortListenerSessionJoinedHandler(ajn::SessionPort sessionPort, ajn::SessionId id, Platform::String ^ joiner)
{
    ::QStatus status = ER_OK;

    while (true) {
        qcc::String strJoiner = PlatformToMultibyteString(joiner);
        if (nullptr != joiner && strJoiner.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::SessionPortListener::SessionJoined(sessionPort, id, strJoiner.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

bool _SessionPortListener::AcceptSessionJoiner(ajn::SessionPort sessionPort, const char* joiner, const ajn::SessionOpts& opts)
{
    ::QStatus status = ER_OK;
    bool result = false;

    while (true) {
        Platform::String ^ strJoiner = MultibyteToPlatformString(joiner);
        if (nullptr == strJoiner && joiner != NULL && joiner[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        SessionOpts ^ sessionOpts = ref new SessionOpts(&opts);
        if (nullptr == sessionOpts) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     result = _eventsAndProperties->AcceptSessionJoiner(sessionPort, strJoiner, sessionOpts);
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void _SessionPortListener::SessionJoined(ajn::SessionPort sessionPort, ajn::SessionId id, const char* joiner)
{
    ::QStatus status = ER_OK;

    while (true) {
        Platform::String ^ strJoiner = MultibyteToPlatformString(joiner);
        if (nullptr == strJoiner && joiner != NULL && joiner[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     _eventsAndProperties->SessionJoined(sessionPort, id, strJoiner);
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

__SessionPortListener::__SessionPortListener()
{
    Bus = nullptr;
}

__SessionPortListener::~__SessionPortListener()
{
    Bus = nullptr;
}

}
