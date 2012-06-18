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

#include <alljoyn/AuthListener.h>
#include <alljoyn/Message.h>
#include <qcc/ManagedObj.h>
#include <Status_CPP0x.h>
#include <Status.h>

namespace AllJoyn {

ref class Credentials;
ref class BusAttachment;
ref class Message;

public delegate bool AuthListenerRequestCredentialsHandler(Platform::String ^ authMechanism, Platform::String ^ peerName, uint16_t authCount, Platform::String ^ userName, uint16_t credMask, Credentials ^ credentials);
public delegate bool AuthListenerVerifyCredentialsHandler(Platform::String ^ authMechanism, Platform::String ^ peerName, Credentials ^ credentials);
public delegate void AuthListenerSecurityViolationHandler(QStatus status, Message ^ msg);
public delegate void AuthListenerAuthenticationCompleteHandler(Platform::String ^ authMechanism, Platform::String ^ peerName, bool success);

ref class __AuthListener {
  private:
    friend ref class AuthListener;
    friend class _AuthListener;
    __AuthListener();
    ~__AuthListener();

    event AuthListenerRequestCredentialsHandler ^ RequestCredentials;
    event AuthListenerVerifyCredentialsHandler ^ VerifyCredentials;
    event AuthListenerSecurityViolationHandler ^ SecurityViolation;
    event AuthListenerAuthenticationCompleteHandler ^ AuthenticationComplete;
    property BusAttachment ^ Bus;
};

class _AuthListener : protected ajn::AuthListener {
  protected:
    friend class qcc::ManagedObj<_AuthListener>;
    friend ref class AuthListener;
    friend ref class BusAttachment;
    _AuthListener(BusAttachment ^ bus);
    ~_AuthListener();

    bool DefaultAuthListenerRequestCredentialsHandler(Platform::String ^ authMechanism, Platform::String ^ peerName, uint16_t authCount, Platform::String ^ userName, uint16_t credMask, AllJoyn::Credentials ^ credentials);
    bool DefaultAuthListenerVerifyCredentialsHandler(Platform::String ^ authMechanism, Platform::String ^ peerName, AllJoyn::Credentials ^ credentials);
    void DefaultAuthListenerSecurityViolationHandler(AllJoyn::QStatus status, Message ^ msg);
    void DefaultAuthListenerAuthenticationCompleteHandler(Platform::String ^ authMechanism, Platform::String ^ peerName, bool success);
    bool RequestCredentials(const char* authMechanism, const char* peerName, uint16_t authCount, const char* userName, uint16_t credMask, ajn::AuthListener::Credentials& credentials);
    bool VerifyCredentials(const char* authMechanism, const char* peerName, const Credentials& credentials);
    void SecurityViolation(::QStatus status, const ajn::Message& msg);
    void AuthenticationComplete(const char* authMechanism, const char* peerName, bool success);

    __AuthListener ^ _eventsAndProperties;
};

public ref class AuthListener sealed {
  public:
    AuthListener(BusAttachment ^ bus);
    ~AuthListener();

    event AuthListenerRequestCredentialsHandler ^ RequestCredentials
    {
        Windows::Foundation::EventRegistrationToken add(AuthListenerRequestCredentialsHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        bool raise(Platform::String ^ authMechanism, Platform::String ^ peerName, uint16_t authCount, Platform::String ^ userName, uint16_t credMask, Credentials ^ credentials);
    }

    event AuthListenerVerifyCredentialsHandler ^ VerifyCredentials
    {
        Windows::Foundation::EventRegistrationToken add(AuthListenerVerifyCredentialsHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        bool raise(Platform::String ^ authMechanism, Platform::String ^ peerName, Credentials ^ credentials);
    }

    event AuthListenerSecurityViolationHandler ^ SecurityViolation
    {
        Windows::Foundation::EventRegistrationToken add(AuthListenerSecurityViolationHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        void raise(QStatus status, Message ^ msg);
    }

    event AuthListenerAuthenticationCompleteHandler ^ AuthenticationComplete
    {
        Windows::Foundation::EventRegistrationToken add(AuthListenerAuthenticationCompleteHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        void raise(Platform::String ^ authMechanism, Platform::String ^ peerName, bool success);
    }

    property BusAttachment ^ Bus
    {
        BusAttachment ^ get();
    }

  private:
    friend ref class BusAttachment;
    AuthListener(void* listener, bool isManaged);

    qcc::ManagedObj<_AuthListener>* _mListener;
    _AuthListener* _listener;
};

}
// AuthListener.h
