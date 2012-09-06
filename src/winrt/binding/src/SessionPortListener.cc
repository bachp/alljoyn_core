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
        // Check bus for invalid values
        if (nullptr == bus) {
            status = ER_BAD_ARG_1;
            break;
        }
        // Create _SessionPortListener
        _SessionPortListener* spl = new _SessionPortListener(bus);
        // Check for llocation error
        if (NULL == spl) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Attach spl to managed _SessionPortListener
        _mListener = new qcc::ManagedObj<_SessionPortListener>(spl);
        // Check for allocation error
        if (NULL == _mListener) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Store pointer to _SessionPortListener for convenience
        _listener = &(**_mListener);
        break;
    }

    // Bubble up any QStatus errors as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

SessionPortListener::SessionPortListener(const qcc::ManagedObj<_SessionPortListener>* listener)
{
    ::QStatus status = ER_OK;

    while (true) {
        // Check listener for invalid values
        if (NULL == listener) {
            status = ER_BAD_ARG_1;
            break;
        }
        // Attach listener to managed _SessionPortListener
        _mListener = new qcc::ManagedObj<_SessionPortListener>(*listener);
        // Check for allocation error
        if (NULL == _mListener) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Store pointer to _SessionPortListener for convenience
        _listener = &(**_mListener);
        break;
    }

    // Bubble up any QStatus errors as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

SessionPortListener::~SessionPortListener()
{
    // Delete managed _SessionPortListener to adjust ref count
    if (NULL != _mListener) {
        delete _mListener;
        _mListener = NULL;
        _listener = NULL;
    }
}

Windows::Foundation::EventRegistrationToken SessionPortListener::AcceptSessionJoiner::add(SessionPortListenerAcceptSessionJoinerHandler ^ handler)
{
    // Add handler for AcceptSessionJoiner
    return _listener->_eventsAndProperties->AcceptSessionJoiner::add(handler);
}

void SessionPortListener::AcceptSessionJoiner::remove(Windows::Foundation::EventRegistrationToken token)
{
    // Remove handler for AcceptSessionJoiner
    _listener->_eventsAndProperties->AcceptSessionJoiner::remove(token);
}

bool SessionPortListener::AcceptSessionJoiner::raise(ajn::SessionPort sessionPort, Platform::String ^ joiner, SessionOpts ^ opts)
{
    // Invoke handler for AcceptSessionJoiner
    return _listener->_eventsAndProperties->AcceptSessionJoiner::raise(sessionPort, joiner, opts);
}

Windows::Foundation::EventRegistrationToken SessionPortListener::SessionJoined::add(SessionPortListenerSessionJoinedHandler ^ handler)
{
    // Add handler for SessionJoined
    return _listener->_eventsAndProperties->SessionJoined::add(handler);
}

void SessionPortListener::SessionJoined::remove(Windows::Foundation::EventRegistrationToken token)
{
    // Remove handler for SessionJoined
    _listener->_eventsAndProperties->SessionJoined::remove(token);
}

void SessionPortListener::SessionJoined::raise(ajn::SessionPort sessionPort, ajn::SessionId id, Platform::String ^ joiner)
{
    // Invoke handler for SessionJoined
    _listener->_eventsAndProperties->SessionJoined::raise(sessionPort, id, joiner);
}

BusAttachment ^ SessionPortListener::Bus::get()
{
    // Return Bus from internal ref class
    return _listener->_eventsAndProperties->Bus;
}

_SessionPortListener::_SessionPortListener(BusAttachment ^ bus)
{
    ::QStatus status = ER_OK;

    while (true) {
        // Create internal ref class
        _eventsAndProperties = ref new __SessionPortListener();
        // Check for allocation error
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Add default handler for AcceptSessionJoiner
        _eventsAndProperties->AcceptSessionJoiner += ref new SessionPortListenerAcceptSessionJoinerHandler([&] (ajn::SessionPort sessionPort, Platform::String ^ joiner, SessionOpts ^ opts)->bool {
                                                                                                               return DefaultSessionPortListenerAcceptSessionJoinerHandler(sessionPort, joiner, opts);
                                                                                                           });
        // Add default handler for SessionJoined
        _eventsAndProperties->SessionJoined += ref new SessionPortListenerSessionJoinedHandler([&] (ajn::SessionPort sessionPort, ajn::SessionId id, Platform::String ^ joiner) {
                                                                                                   DefaultSessionPortListenerSessionJoinedHandler(sessionPort, id, joiner);
                                                                                               });
        // Store BusAttachment
        _eventsAndProperties->Bus = bus;
        break;
    }

    // Bubble up any QStatus errors as an exception
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
        // Convert joiner to qcc::String
        qcc::String strJoiner = PlatformToMultibyteString(joiner);
        // Check for conversion failure
        if (nullptr != joiner && strJoiner.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Get unmanaged SessionOpts
        ajn::SessionOpts* sessionOpts = opts->_sessionOpts;
        // Call the real API
        result = ajn::SessionPortListener::AcceptSessionJoiner(sessionPort, strJoiner.c_str(), *sessionOpts);
        break;
    }

    // Bubble up any QStatus errors as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void _SessionPortListener::DefaultSessionPortListenerSessionJoinedHandler(ajn::SessionPort sessionPort, ajn::SessionId id, Platform::String ^ joiner)
{
    ::QStatus status = ER_OK;

    while (true) {
        // Convert joiner to qcc::String
        qcc::String strJoiner = PlatformToMultibyteString(joiner);
        // Check for conversion failure
        if (nullptr != joiner && strJoiner.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Call the real API
        ajn::SessionPortListener::SessionJoined(sessionPort, id, strJoiner.c_str());
        break;
    }

    // Bubble up any QStatus errors as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

bool _SessionPortListener::AcceptSessionJoiner(ajn::SessionPort sessionPort, const char* joiner, const ajn::SessionOpts& opts)
{
    ::QStatus status = ER_OK;
    bool result = false;

    while (true) {
        // Convert joiner to Platform::String
        Platform::String ^ strJoiner = MultibyteToPlatformString(joiner);
        // Check for conversion failure
        if (nullptr == strJoiner && joiner != NULL && joiner[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Create SessionOpts
        SessionOpts ^ sessionOpts = ref new SessionOpts(&opts);
        // Check for allocation error
        if (nullptr == sessionOpts) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Call AccpetSessionJoiner handler through the dispatcher
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     result = _eventsAndProperties->AcceptSessionJoiner(sessionPort, strJoiner, sessionOpts);
                                                                                                                 }));
        break;
    }

    // Bubble up any QStatus errors as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void _SessionPortListener::SessionJoined(ajn::SessionPort sessionPort, ajn::SessionId id, const char* joiner)
{
    ::QStatus status = ER_OK;

    while (true) {
        // Convert joiner to Platform::String
        Platform::String ^ strJoiner = MultibyteToPlatformString(joiner);
        // Check for conversion failure
        if (nullptr == strJoiner && joiner != NULL && joiner[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Call SessionJoined handler through the dispatcher
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     _eventsAndProperties->SessionJoined(sessionPort, id, strJoiner);
                                                                                                                 }));
        break;
    }

    // Bubble up any QStatus errors as an exception
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
