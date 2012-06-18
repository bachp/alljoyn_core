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
#include <qcc/ManagedObj.h>
#include <Status_CPP0x.h>
#include <Status.h>

namespace AllJoyn {

public enum class CredentialType {
    CRED_PASSWORD     = ajn::AuthListener::CRED_PASSWORD,
    CRED_USER_NAME    = ajn::AuthListener::CRED_USER_NAME,
    CRED_CERT_CHAIN   = ajn::AuthListener::CRED_CERT_CHAIN,
    CRED_PRIVATE_KEY  = ajn::AuthListener::CRED_PRIVATE_KEY,
    CRED_LOGON_ENTRY  = ajn::AuthListener::CRED_LOGON_ENTRY,
    CRED_EXPIRATION   = ajn::AuthListener::CRED_EXPIRATION,
    CRED_NEW_PASSWORD = ajn::AuthListener::CRED_NEW_PASSWORD,
    CRED_ONE_TIME_PWD = ajn::AuthListener::CRED_ONE_TIME_PWD
};

ref class __Credentials {
  private:
    friend ref class Credentials;
    friend class _Credentials;
    __Credentials();
    ~__Credentials();

    property Platform::String ^ Password;
    property Platform::String ^ UserName;
    property Platform::String ^ CertChain;
    property Platform::String ^ PrivateKey;
    property Platform::String ^ LogonEntry;
    property uint32_t Expiration;
};

class _Credentials : protected ajn::AuthListener::Credentials {
  protected:
    friend class qcc::ManagedObj<_Credentials>;
    friend ref class Credentials;
    friend class _AuthListener;
    _Credentials();
    ~_Credentials();

    __Credentials ^ _eventsAndProperties;
};

public ref class Credentials sealed {
  public:
    Credentials();
    ~Credentials();

    bool IsSet(uint16_t creds);
    void Clear();

    property Platform::String ^ Password
    {
        Platform::String ^ get();
        void set(Platform::String ^ value);
    }

    property Platform::String ^ UserName
    {
        Platform::String ^ get();
        void set(Platform::String ^ value);
    }

    property Platform::String ^ CertChain
    {
        Platform::String ^ get();
        void set(Platform::String ^ value);
    }

    property Platform::String ^ PrivateKey
    {
        Platform::String ^ get();
        void set(Platform::String ^ value);
    }

    property Platform::String ^ LogonEntry
    {
        Platform::String ^ get();
        void set(Platform::String ^ value);
    }

    property uint32_t Expiration
    {
        uint32_t get();
        void set(uint32_t value);
    }

  private:
    friend class _AuthListener;
    Credentials(void* creds, bool isManaged);

    qcc::ManagedObj<_Credentials>* _mCredentials;
    _Credentials* _credentials;
};

}
// Credentials.h
