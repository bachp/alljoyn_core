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

#include "Credentials.h"

#include <qcc/String.h>
#include <qcc/winrt/utility.h>
#include <ObjectReference.h>
#include <AllJoynException.h>

namespace AllJoyn {

Credentials::Credentials()
{
    ::QStatus status = ER_OK;

    while (true) {
        _Credentials* creds = new _Credentials();
        if (NULL == creds) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mCredentials = new qcc::ManagedObj<_Credentials>(creds);
        if (NULL == _mCredentials) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _credentials = &(**_mCredentials);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Credentials::Credentials(const ajn::AuthListener::Credentials* creds)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == creds) {
            status = ER_BAD_ARG_1;
            break;
        }
        _Credentials* credentials = new _Credentials();
        if (NULL == credentials) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::AuthListener::Credentials* dstCreds = credentials;
        *dstCreds = *creds;
        _mCredentials = new qcc::ManagedObj<_Credentials>(credentials);
        if (NULL == _mCredentials) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _credentials = &(**_mCredentials);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Credentials::Credentials(const qcc::ManagedObj<_Credentials>* creds)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == creds) {
            status = ER_BAD_ARG_1;
            break;
        }
        _mCredentials = new qcc::ManagedObj<_Credentials>(*creds);
        if (NULL == _mCredentials) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _credentials = &(**_mCredentials);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Credentials::~Credentials()
{
    if (NULL != _mCredentials) {
        delete _mCredentials;
        _mCredentials = NULL;
        _credentials = NULL;
    }
}

bool Credentials::IsSet(uint16_t creds)
{
    return _credentials->IsSet(creds);
}

void Credentials::Clear()
{
    _credentials->Clear();
}

Platform::String ^ Credentials::Password::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _credentials->_eventsAndProperties->Password) {
            qcc::String strPassword = _credentials->GetPassword();
            result = MultibyteToPlatformString(strPassword.c_str());
            if (nullptr == result && !strPassword.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _credentials->_eventsAndProperties->Password = result;
        } else {
            result = _credentials->_eventsAndProperties->Password;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void Credentials::Password::set(Platform::String ^ value)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == value) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strValue = PlatformToMultibyteString(value);
        if (strValue.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _credentials->SetPassword(strValue);
        _credentials->_eventsAndProperties->Password = value;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Platform::String ^ Credentials::UserName::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _credentials->_eventsAndProperties->UserName) {
            qcc::String strUserName = _credentials->GetUserName();
            result = MultibyteToPlatformString(strUserName.c_str());
            if (nullptr == result && !strUserName.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _credentials->_eventsAndProperties->UserName = result;
        } else {
            result = _credentials->_eventsAndProperties->UserName;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void Credentials::UserName::set(Platform::String ^ value)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == value) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strValue = PlatformToMultibyteString(value);
        if (strValue.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _credentials->SetUserName(strValue);
        _credentials->_eventsAndProperties->UserName = value;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Platform::String ^ Credentials::CertChain::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _credentials->_eventsAndProperties->CertChain) {
            qcc::String strCertChain = _credentials->GetCertChain();
            result = MultibyteToPlatformString(strCertChain.c_str());
            if (nullptr == result && !strCertChain.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _credentials->_eventsAndProperties->CertChain = result;
        } else {
            result = _credentials->_eventsAndProperties->CertChain;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void Credentials::CertChain::set(Platform::String ^ value)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == value) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strValue = PlatformToMultibyteString(value);
        if (strValue.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _credentials->SetCertChain(strValue);
        _credentials->_eventsAndProperties->CertChain = value;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Platform::String ^ Credentials::PrivateKey::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _credentials->_eventsAndProperties->PrivateKey) {
            qcc::String strPrivateKey = _credentials->GetPrivateKey();
            result = MultibyteToPlatformString(strPrivateKey.c_str());
            if (nullptr == result && !strPrivateKey.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _credentials->_eventsAndProperties->PrivateKey = result;
        } else {
            result = _credentials->_eventsAndProperties->PrivateKey;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void Credentials::PrivateKey::set(Platform::String ^ value)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == value) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strValue = PlatformToMultibyteString(value);
        if (strValue.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _credentials->SetPrivateKey(strValue);
        _credentials->_eventsAndProperties->PrivateKey = value;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Platform::String ^ Credentials::LogonEntry::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _credentials->_eventsAndProperties->LogonEntry) {
            qcc::String strLogonEntry = _credentials->GetLogonEntry();
            result = MultibyteToPlatformString(strLogonEntry.c_str());
            if (nullptr == result && !strLogonEntry.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _credentials->_eventsAndProperties->LogonEntry = result;
        } else {
            result = _credentials->_eventsAndProperties->LogonEntry;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void Credentials::LogonEntry::set(Platform::String ^ value)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == value) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strValue = PlatformToMultibyteString(value);
        if (strValue.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _credentials->SetLogonEntry(strValue);
        _credentials->_eventsAndProperties->LogonEntry = value;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

uint32_t Credentials::Expiration::get()
{
    uint32_t result = (uint32_t)-1;

    if ((uint32_t)-1 == _credentials->_eventsAndProperties->Expiration) {
        result = _credentials->GetExpiration();
        _credentials->_eventsAndProperties->Expiration = result;
    } else {
        result = _credentials->_eventsAndProperties->Expiration;
    }

    return result;
}

void Credentials::Expiration::set(uint32_t value)
{
    _credentials->SetExpiration(value);
    _credentials->_eventsAndProperties->Expiration = value;
}

_Credentials::_Credentials()
    : ajn::AuthListener::Credentials()
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __Credentials();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_Credentials::~_Credentials()
{
    _eventsAndProperties = nullptr;
}

__Credentials::__Credentials()
{
    Password = nullptr;
    UserName = nullptr;
    CertChain = nullptr;
    PrivateKey = nullptr;
    LogonEntry = nullptr;
    Expiration = -1;
}

__Credentials::~__Credentials()
{
    Password = nullptr;
    UserName = nullptr;
    CertChain = nullptr;
    PrivateKey = nullptr;
    LogonEntry = nullptr;
    Expiration = -1;
}

}
