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

AuthListener::AuthListener(void* listener, bool isManaged)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == listener) {
            status = ER_BAD_ARG_1;
            break;
        }
        if (!isManaged) {
            status = ER_FAIL;
            break;
        }
        qcc::ManagedObj<_AuthListener>* mal = reinterpret_cast<qcc::ManagedObj<_AuthListener>*>(listener);
        _mListener = new qcc::ManagedObj<_AuthListener>(*mal);
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
    // Make sure consumers are following the documentation
    if (!IsDestructedRefCount(this)) {
        QCC_THROW_EXCEPTION(ER_FAIL);
    }

    if (NULL != _mListener) {
        delete _mListener;
        _mListener = NULL;
        _listener = NULL;
    }
}

Windows::Foundation::EventRegistrationToken AuthListener::RequestCredentials::add(AuthListenerRequestCredentialsHandler ^ handler)
{
    return _listener->_eventsAndProperties->RequestCredentials::add(handler);
}

void AuthListener::RequestCredentials::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->RequestCredentials::remove(token);
}

bool AuthListener::RequestCredentials::raise(Platform::String ^ authMechanism, Platform::String ^ peerName, uint16_t authCount, Platform::String ^ userName, uint16_t credMask, Credentials ^ credentials)
{
    return _listener->_eventsAndProperties->RequestCredentials::raise(authMechanism, peerName, authCount, userName, credMask, credentials);
}

Windows::Foundation::EventRegistrationToken AuthListener::VerifyCredentials::add(AuthListenerVerifyCredentialsHandler ^ handler)
{
    return _listener->_eventsAndProperties->VerifyCredentials::add(handler);
}

void AuthListener::VerifyCredentials::remove(Windows::Foundation::EventRegistrationToken token)
{
    _listener->_eventsAndProperties->VerifyCredentials::remove(token);
}

bool AuthListener::VerifyCredentials::raise(Platform::String ^ authMechanism, Platform::String ^ peerName, Credentials ^ credentials)
{
    return _listener->_eventsAndProperties->VerifyCredentials::raise(authMechanism, peerName, credentials);
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
        _eventsAndProperties->RequestCredentials += ref new AuthListenerRequestCredentialsHandler([&] (Platform::String ^ authMechanism, Platform::String ^ peerName, uint16_t authCount, Platform::String ^ userName, uint16_t credMask, AllJoyn::Credentials ^ credentials)->bool {
                                                                                                      return DefaultAuthListenerRequestCredentialsHandler(authMechanism, peerName, authCount, userName, credMask, credentials);
                                                                                                  });
        _eventsAndProperties->VerifyCredentials += ref new AuthListenerVerifyCredentialsHandler([&] (Platform::String ^ authMechanism, Platform::String ^ peerName, AllJoyn::Credentials ^ credentials)->bool {
                                                                                                    return DefaultAuthListenerVerifyCredentialsHandler(authMechanism, peerName, credentials);
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

bool _AuthListener::DefaultAuthListenerRequestCredentialsHandler(Platform::String ^ authMechanism, Platform::String ^ peerName, uint16_t authCount, Platform::String ^ userName, uint16_t credMask, AllJoyn::Credentials ^ credentials)
{
    return false;
}

bool _AuthListener::DefaultAuthListenerVerifyCredentialsHandler(Platform::String ^ authMechanism, Platform::String ^ peerName, AllJoyn::Credentials ^ credentials)
{
    ::QStatus status = ER_OK;
    bool result = false;

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
        result = ajn::AuthListener::VerifyCredentials(strAuthMechanism.c_str(), strPeerName.c_str(), *creds);
        break;
    }

    return result;
}

void _AuthListener::DefaultAuthListenerSecurityViolationHandler(AllJoyn::QStatus status, Message ^ msg)
{
    ajn::Message* m = *(msg->_message);
    ajn::AuthListener::SecurityViolation((::QStatus)(int)status, *m);
}

void _AuthListener::DefaultAuthListenerAuthenticationCompleteHandler(Platform::String ^ authMechanism, Platform::String ^ peerName, bool success)
{
}

bool _AuthListener::RequestCredentials(const char* authMechanism, const char* peerName, uint16_t authCount, const char* userName, uint16_t credMask, ajn::AuthListener::Credentials& credentials)
{
    ::QStatus status = ER_OK;
    bool result = false;

    try {
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
            AllJoyn::Credentials ^ cred = ref new AllJoyn::Credentials((void*)&credentials, false);
            if (nullptr == cred) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                         result = _eventsAndProperties->RequestCredentials(strAuthMechanism, strPeerName, authCount, strUserName, credMask, cred);
                                                                                                                     }));
            ajn::AuthListener::Credentials* credsOut = cred->_credentials;
            ((Credentials &)credentials) = *credsOut;
            break;
        }

        if (ER_OK != status) {
            QCC_THROW_EXCEPTION(status);
        }
    } catch (...) {
        // Do nothing
    }

    return result;
}

bool _AuthListener::VerifyCredentials(const char* authMechanism, const char* peerName, const Credentials& credentials)
{
    ::QStatus status = ER_OK;
    bool result = false;

    try {
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
            AllJoyn::Credentials ^ cred = ref new AllJoyn::Credentials((void*)&credentials, false);
            if (nullptr == cred) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                         result = _eventsAndProperties->VerifyCredentials(strAuthMechanism, strPeerName, cred);
                                                                                                                     }));
            ajn::AuthListener::Credentials* credsOut = cred->_credentials;
            ((Credentials &)credentials) = *credsOut;
            break;
        }

        if (ER_OK != status) {
            QCC_THROW_EXCEPTION(status);
        }
    } catch (...) {
        // Do nothing
    }

    return result;
}

void _AuthListener::SecurityViolation(::QStatus status, const ajn::Message& msg)
{
    try {
        while (true) {
            Message ^ message = ref new Message((void*)&msg, true);
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
    } catch (...) {
        // Do nothing
    }
}

void _AuthListener::AuthenticationComplete(const char* authMechanism, const char* peerName, bool success)
{
    ::QStatus status = ER_OK;

    try {
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
    } catch (...) {
        // Do nothing
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
