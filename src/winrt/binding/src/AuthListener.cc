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

#include "AuthListener.h"

#include <qcc/String.h>
#include <qcc/winrt/utility.h>
#include <Message.h>
#include <BusAttachment.h>
#include <Credentials.h>
#include <ObjectReference.h>
#include <AllJoynException.h>

namespace AllJoyn {

AuthListener::AuthListener(BusAttachment ^ bus)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == bus) {
            status = ER_BAD_ARG_1;
            break;
        }
        _AuthListener* al = new _AuthListener(bus);
        if (NULL == al) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mListener = new qcc::ManagedObj<_AuthListener>(al);
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

AuthListener::AuthListener(const qcc::ManagedObj<_AuthListener>* listener)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == listener) {
            status = ER_BAD_ARG_1;
            break;
        }
        _mListener = new qcc::ManagedObj<_AuthListener>(*listener);
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

AuthListener::~AuthListener()
{
    if (NULL != _mListener) {
        delete _mListener;
        _mListener = NULL;
        _listener = NULL;
    }
}

void AuthListener::RequestCredentialsResponse(AuthContext ^ authContext, bool accept, Credentials ^ credentials)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == authContext) {
            status = ER_BAD_ARG_1;
            break;
        }
        if (nullptr == credentials) {
            status = ER_BAD_ARG_3;
            break;
        }
        ajn::AuthListener::Credentials* creds = credentials->_credentials;
        status = _listener->RequestCredentialsResponse(authContext->_authContext, accept, *creds);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void AuthListener::VerifyCredentialsResponse(AuthContext ^ authContext, bool accept)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == authContext) {
            status = ER_BAD_ARG_1;
            break;
        }
        status = _listener->VerifyCredentialsResponse(authContext->_authContext, accept);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Windows::Foundation::EventRegistrationToken AuthListener::RequestCredentials::add(AuthListenerRequestCredentialsAsyncHandler ^ handler)
{
    return _listener->_eventsAndProperties->RequestCredentials::add(handler);
}

void AuthListener::RequestCredentials::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->RequestCredentials::remove(token);
}

QStatus AuthListener::RequestCredentials::raise(Platform::String ^ authMechanism, Platform::String ^ peerName, uint16_t authCount, Platform::String ^ userName, uint16_t credMask, AuthContext ^ authContext)
{
    return _listener->_eventsAndProperties->RequestCredentials::raise(authMechanism, peerName, authCount, userName, credMask, authContext);
}

Windows::Foundation::EventRegistrationToken AuthListener::VerifyCredentials::add(AuthListenerVerifyCredentialsAsyncHandler ^ handler)
{
    return _listener->_eventsAndProperties->VerifyCredentials::add(handler);
}

void AuthListener::VerifyCredentials::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->VerifyCredentials::remove(token);
}

QStatus AuthListener::VerifyCredentials::raise(Platform::String ^ authMechanism, Platform::String ^ peerName, AllJoyn::Credentials ^ credentials, AuthContext ^ authContext)
{
    return _listener->_eventsAndProperties->VerifyCredentials::raise(authMechanism, peerName, credentials, authContext);
}

Windows::Foundation::EventRegistrationToken AuthListener::SecurityViolation::add(AuthListenerSecurityViolationHandler ^ handler)
{
    return _listener->_eventsAndProperties->SecurityViolation::add(handler);
}

void AuthListener::SecurityViolation::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->SecurityViolation::remove(token);
}

void AuthListener::SecurityViolation::raise(QStatus status, Message ^ msg)
{
    _listener->_eventsAndProperties->SecurityViolation::raise(status, msg);
}

Windows::Foundation::EventRegistrationToken AuthListener::AuthenticationComplete::add(AuthListenerAuthenticationCompleteHandler ^ handler)
{
    return _listener->_eventsAndProperties->AuthenticationComplete::add(handler);
}

void AuthListener::AuthenticationComplete::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->AuthenticationComplete::remove(token);
}

void AuthListener::AuthenticationComplete::raise(Platform::String ^ authMechanism, Platform::String ^ peerName, bool success)
{
    _listener->_eventsAndProperties->AuthenticationComplete::raise(authMechanism, peerName, success);
}

BusAttachment ^ AuthListener::Bus::get()
{
    return _listener->_eventsAndProperties->Bus;
}

_AuthListener::_AuthListener(BusAttachment ^ bus)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __AuthListener();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->RequestCredentials += ref new AuthListenerRequestCredentialsAsyncHandler([&] (Platform::String ^ authMechanism, Platform::String ^ peerName, uint16_t authCount, Platform::String ^ userName, uint16_t credMask, AuthContext ^ authContext)->QStatus {
                                                                                                           return DefaultAuthListenerRequestCredentialsAsyncHandler(authMechanism, peerName, authCount, userName, credMask, authContext);
                                                                                                       });
        _eventsAndProperties->VerifyCredentials += ref new AuthListenerVerifyCredentialsAsyncHandler([&] (Platform::String ^ authMechanism, Platform::String ^ peerName, AllJoyn::Credentials ^ credentials, AuthContext ^ authContext)->QStatus {
                                                                                                         return DefaultAuthListenerVerifyCredentialsAsyncHandler(authMechanism, peerName, credentials, authContext);
                                                                                                     });
        _eventsAndProperties->SecurityViolation += ref new AuthListenerSecurityViolationHandler([&] (AllJoyn::QStatus status, Message ^ msg) {
                                                                                                    DefaultAuthListenerSecurityViolationHandler(status, msg);
                                                                                                });
        _eventsAndProperties->AuthenticationComplete += ref new AuthListenerAuthenticationCompleteHandler([&] (Platform::String ^ authMechanism, Platform::String ^ peerName, bool success) {
                                                                                                              DefaultAuthListenerAuthenticationCompleteHandler(authMechanism, peerName, success);
                                                                                                          });
        _eventsAndProperties->Bus = bus;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_AuthListener::~_AuthListener()
{
    _eventsAndProperties = nullptr;
}

QStatus _AuthListener::DefaultAuthListenerRequestCredentialsAsyncHandler(Platform::String ^ authMechanism, Platform::String ^ peerName, uint16_t authCount, Platform::String ^ userName, uint16_t credMask, AuthContext ^ authContext)
{
    ::QStatus status = ER_OK;

    while (true) {
        qcc::String strAuthMechanism = PlatformToMultibyteString(authMechanism);
        if (nullptr != authMechanism && strAuthMechanism.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strPeerName = PlatformToMultibyteString(peerName);
        if (nullptr != peerName && strPeerName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strUserName = PlatformToMultibyteString(userName);
        if (nullptr != userName && strUserName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = ajn::AuthListener::RequestCredentialsAsync(strAuthMechanism.c_str(), strPeerName.c_str(), authCount, strUserName.c_str(), credMask, authContext->_authContext);
        break;
    }

    return (AllJoyn::QStatus)status;
}

QStatus _AuthListener::DefaultAuthListenerVerifyCredentialsAsyncHandler(Platform::String ^ authMechanism, Platform::String ^ peerName, AllJoyn::Credentials ^ credentials, AuthContext ^ authContext)
{
    ::QStatus status = ER_OK;

    while (true) {
        qcc::String strAuthMechanism = PlatformToMultibyteString(authMechanism);
        if (nullptr != authMechanism && strAuthMechanism.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strPeerName = PlatformToMultibyteString(peerName);
        if (nullptr != peerName && strPeerName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::AuthListener::Credentials* creds = credentials->_credentials;
        status = ajn::AuthListener::VerifyCredentialsAsync(strAuthMechanism.c_str(), strPeerName.c_str(), *creds, authContext->_authContext);
        break;
    }

    return (AllJoyn::QStatus)status;
}

void _AuthListener::DefaultAuthListenerSecurityViolationHandler(AllJoyn::QStatus status, Message ^ msg)
{
    ajn::Message* m = *(msg->_message);
    ajn::AuthListener::SecurityViolation((::QStatus)(int)status, *m);
}

void _AuthListener::DefaultAuthListenerAuthenticationCompleteHandler(Platform::String ^ authMechanism, Platform::String ^ peerName, bool success)
{
}

::QStatus _AuthListener::RequestCredentialsAsync(const char* authMechanism, const char* peerName, uint16_t authCount, const char* userName, uint16_t credMask, void* authContext)
{
    ::QStatus status = ER_OK;

    while (true) {
        Platform::String ^ strAuthMechanism = MultibyteToPlatformString(authMechanism);
        if (nullptr == strAuthMechanism && authMechanism != NULL && authMechanism[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        Platform::String ^ strPeerName = MultibyteToPlatformString(peerName);
        if (nullptr == strPeerName && peerName != NULL && peerName[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        Platform::String ^ strUserName = MultibyteToPlatformString(userName);
        if (nullptr == strUserName && userName != NULL && userName[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        AuthContext ^ context = ref new AuthContext(authContext);
        if (nullptr == context) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     status = (::QStatus)_eventsAndProperties->RequestCredentials(strAuthMechanism, strPeerName, authCount, strUserName, credMask, context);
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return status;
}

::QStatus _AuthListener::VerifyCredentialsAsync(const char* authMechanism, const char* peerName, const Credentials& credentials, void* authContext)
{
    ::QStatus status = ER_OK;

    while (true) {
        Platform::String ^ strAuthMechanism = MultibyteToPlatformString(authMechanism);
        if (nullptr == strAuthMechanism && authMechanism != NULL && authMechanism[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        Platform::String ^ strPeerName = MultibyteToPlatformString(peerName);
        if (nullptr == strPeerName && peerName != NULL && peerName[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        AllJoyn::Credentials ^ cred = ref new AllJoyn::Credentials(&credentials);
        if (nullptr == cred) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        AuthContext ^ context = ref new AuthContext(authContext);
        if (nullptr == context) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     status = (::QStatus)_eventsAndProperties->VerifyCredentials(strAuthMechanism, strPeerName, cred, context);
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return status;
}

void _AuthListener::SecurityViolation(::QStatus status, const ajn::Message& msg)
{
    while (true) {
        Message ^ message = ref new Message(&msg);
        if (nullptr == message) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     _eventsAndProperties->SecurityViolation((QStatus)(int)status, message);
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void _AuthListener::AuthenticationComplete(const char* authMechanism, const char* peerName, bool success)
{
    ::QStatus status = ER_OK;

    while (true) {
        Platform::String ^ strAuthMechanism = MultibyteToPlatformString(authMechanism);
        if (nullptr == strAuthMechanism && authMechanism != NULL && authMechanism[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        Platform::String ^ strPeerName = MultibyteToPlatformString(peerName);
        if (nullptr == strPeerName && peerName != NULL && peerName[0] != '\0') {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     _eventsAndProperties->AuthenticationComplete(strAuthMechanism, strPeerName, success);
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

__AuthListener::__AuthListener()
{
    Bus = nullptr;
}

__AuthListener::~__AuthListener()
{
    Bus = nullptr;
}

}
