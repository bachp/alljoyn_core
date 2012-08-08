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

/// <summary>
/// Authentication mechanism requests user credentials. If the user name is not an empty string
/// the request is for credentials for that specific user. A count allows the listener to decide
/// whether to allow or reject multiple authentication attempts to the same peer.
/// </summary>
/// <param name="authMechanism">The name of the authentication mechanism issuing the request.</param>
/// <param name="peerName">
/// The name of the remote peer being authenticated.  On the initiating
/// side this will be a well-known-name for the remote peer. On the
/// accepting side this will be the unique bus name for the remote peer.
/// </param>
/// <param name="authCount">Count (starting at 1) of the number of authentication request attempts made.</param>
/// <param name="userName">The user name for the credentials being requested.</param>
/// <param name="credMask">A bit mask identifying the credentials being requested.
/// The application may return none, some or all of the requested credentials.
/// </param>
/// <param name="credentials">The credentials returned.</param>
/// <returns>
///The caller should return true if the request is being accepted or false if the
///requests is being rejected. If the request is rejected the authentication is
///complete.
/// </returns>
public delegate bool AuthListenerRequestCredentialsHandler(Platform::String ^ authMechanism, Platform::String ^ peerName, uint16_t authCount, Platform::String ^ userName, uint16_t credMask, Credentials ^ credentials);

/// <summary>
///Authentication mechanism requests verification of credentials from a remote peer.
/// </summary>
/// <param name="authMechanism">The name of the authentication mechanism issuing the request.</param>
/// <param name="peerName">The name of the remote peer being authenticated. On the initiating
/// side this will be a well-known-name for the remote peer. On the
/// accepting side this will be the unique bus name for the remote peer.</param>
/// <param name="credentials">The credentials to be verified.</param>
/// <returns>
///The listener should return true if the credentials are acceptable or false if the
///credentials are being rejected.
/// </returns>
public delegate bool AuthListenerVerifyCredentialsHandler(Platform::String ^ authMechanism, Platform::String ^ peerName, Credentials ^ credentials);

/// <summary>
///Optional method that if implemented allows an application to monitor security violations. This
///function is called when an attempt to decrypt an encrypted messages failed or when an unencrypted
///message was received on an interface that requires encryption. The message contains only
///header information.
/// </summary>
/// <param name="status">A status code indicating the type of security violation.</param>
/// <param name="msg">The message that cause the security violation.</param>
public delegate void AuthListenerSecurityViolationHandler(QStatus status, Message ^ msg);

/// <summary>
///Reports successful or unsuccessful completion of authentication.
/// </summary>
/// <param name="authMechanism">The name of the authentication mechanism that was used or an empty
/// string if the authentication failed.</param>
/// <param name="peerName">The name of the remote peer being authenticated.  On the initiating
/// side this will be a well-known-name for the remote peer. On the
/// accepting side this will be the unique bus name for the remote peer.</param>
/// <param name="success">true if the authentication was successful, otherwise false.</param>
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

/// <summary>
/// Class to allow authentication mechanisms to interact with the user or application.
/// </summary>
public ref class AuthListener sealed {
  public:
    AuthListener(BusAttachment ^ bus);

    /// <summary>
    ///Called when user credentials are requested.
    /// </summary>
    event AuthListenerRequestCredentialsHandler ^ RequestCredentials
    {
        Windows::Foundation::EventRegistrationToken add(AuthListenerRequestCredentialsHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        bool raise(Platform::String ^ authMechanism, Platform::String ^ peerName, uint16_t authCount, Platform::String ^ userName, uint16_t credMask, Credentials ^ credentials);
    }

    /// <summary>
    ///Called when a remote peer requests verification of credentials.
    /// </summary>
    event AuthListenerVerifyCredentialsHandler ^ VerifyCredentials
    {
        Windows::Foundation::EventRegistrationToken add(AuthListenerVerifyCredentialsHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        bool raise(Platform::String ^ authMechanism, Platform::String ^ peerName, Credentials ^ credentials);
    }

    /// <summary>
    ///Called when an attempt to decrypt an encrypted messages failed or when an unencrypted
    ///message was received on an interface that requires encryption.
    /// </summary>
    event AuthListenerSecurityViolationHandler ^ SecurityViolation
    {
        Windows::Foundation::EventRegistrationToken add(AuthListenerSecurityViolationHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        void raise(QStatus status, Message ^ msg);
    }

    /// <summary>
    ///Called upon successful or unsuccessful completion of authentication.
    /// </summary>
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
    ~AuthListener();

    qcc::ManagedObj<_AuthListener>* _mListener;
    _AuthListener* _listener;
};

}
// AuthListener.h
