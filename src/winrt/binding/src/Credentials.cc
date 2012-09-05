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
        // Create _Credentials
        _Credentials* creds = new _Credentials();
        // Check for allocation error
        if (NULL == creds) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Attach _Credentials to a managed object
        _mCredentials = new qcc::ManagedObj<_Credentials>(creds);
        // Check for allocation error
        if (NULL == _mCredentials) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Store a pointer to _Credentials for convenience
        _credentials = &(**_mCredentials);
        break;
    }

    // Bubble up any QStatus error as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Credentials::Credentials(const ajn::AuthListener::Credentials* creds)
{
    ::QStatus status = ER_OK;

    while (true) {
        // Check creds for invalid values
        if (NULL == creds) {
            status = ER_BAD_ARG_1;
            break;
        }
        // Create _Credentials
        _Credentials* credentials = new _Credentials();
        // Check for allocation error
        if (NULL == credentials) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Get the unmanaged Credentials
        ajn::AuthListener::Credentials* dstCreds = credentials;
        // Invoke the copy constructor
        *dstCreds = *creds;
        // Attach credentials to a managed object
        _mCredentials = new qcc::ManagedObj<_Credentials>(credentials);
        // Check for allocation error
        if (NULL == _mCredentials) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Store a pointer to _Credentials for convenience
        _credentials = &(**_mCredentials);
        break;
    }

    // Bubble up any QStatus error as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Credentials::Credentials(const qcc::ManagedObj<_Credentials>* creds)
{
    ::QStatus status = ER_OK;

    while (true) {
        // Check for invalid values in creds
        if (NULL == creds) {
            status = ER_BAD_ARG_1;
            break;
        }
        // Attach creds to the managed object
        _mCredentials = new qcc::ManagedObj<_Credentials>(*creds);
        if (NULL == _mCredentials) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Store a pointer to _Credentials for convenience
        _credentials = &(**_mCredentials);
        break;
    }

    // Bubble up any QStatus error as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Credentials::~Credentials()
{
    // Delete the managed credentials to adjust ref count
    if (NULL != _mCredentials) {
        delete _mCredentials;
        _mCredentials = NULL;
        _credentials = NULL;
    }
}

bool Credentials::IsSet(uint16_t creds)
{
    // Call the real API
    return _credentials->IsSet(creds);
}

void Credentials::Clear()
{
    // Call the real API
    _credentials->Clear();
}

Platform::String ^ Credentials::Password::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        // Check if value has already been wrapped
        if (nullptr == _credentials->_eventsAndProperties->Password) {
            // Call the real API
            qcc::String strPassword = _credentials->GetPassword();
            // Convert strPassword to Platform::String
            result = MultibyteToPlatformString(strPassword.c_str());
            // Check for conversion error
            if (nullptr == result && !strPassword.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            // Store the result
            _credentials->_eventsAndProperties->Password = result;
        } else {
            // Return Password
            result = _credentials->_eventsAndProperties->Password;
        }
        break;
    }

    // Bubble up any QStatus error as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void Credentials::Password::set(Platform::String ^ value)
{
    ::QStatus status = ER_OK;

    while (true) {
        // Check value for invalid values
        if (nullptr == value) {
            status = ER_BAD_ARG_1;
            break;
        }
        // Convert value to qcc::String
        qcc::String strValue = PlatformToMultibyteString(value);
        // Check for conversion error
        if (strValue.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Call the real API
        _credentials->SetPassword(strValue);
        // Store the result
        _credentials->_eventsAndProperties->Password = value;
        break;
    }

    // Bubble up any QStatus error as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Platform::String ^ Credentials::UserName::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        // Check if the wrapped value already exists
        if (nullptr == _credentials->_eventsAndProperties->UserName) {
            // Call the real API
            qcc::String strUserName = _credentials->GetUserName();
            // Convert return value to Platform::String
            result = MultibyteToPlatformString(strUserName.c_str());
            // Check for conversion error
            if (nullptr == result && !strUserName.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            // Store the result
            _credentials->_eventsAndProperties->UserName = result;
        } else {
            // Return UserName
            result = _credentials->_eventsAndProperties->UserName;
        }
        break;
    }

    // Bubble up any QStatus error as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void Credentials::UserName::set(Platform::String ^ value)
{
    ::QStatus status = ER_OK;

    while (true) {
        // Check for invalid values in value
        if (nullptr == value) {
            status = ER_BAD_ARG_1;
            break;
        }
        // Convert value to qcc::String
        qcc::String strValue = PlatformToMultibyteString(value);
        // Check for conversion error
        if (strValue.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Call the real API
        _credentials->SetUserName(strValue);
        // Store the result
        _credentials->_eventsAndProperties->UserName = value;
        break;
    }

    // Bubble up any QStatus error as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Platform::String ^ Credentials::CertChain::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        // Check if the wrapped value already exists
        if (nullptr == _credentials->_eventsAndProperties->CertChain) {
            // Call the real API
            qcc::String strCertChain = _credentials->GetCertChain();
            // Convert resul to Platform::String
            result = MultibyteToPlatformString(strCertChain.c_str());
            // Check for conversion error
            if (nullptr == result && !strCertChain.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            // Store the result
            _credentials->_eventsAndProperties->CertChain = result;
        } else {
            // Return CertChain
            result = _credentials->_eventsAndProperties->CertChain;
        }
        break;
    }

    // Bubble up any QStatus error as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void Credentials::CertChain::set(Platform::String ^ value)
{
    ::QStatus status = ER_OK;

    while (true) {
        // Check for invalid values in value
        if (nullptr == value) {
            status = ER_BAD_ARG_1;
            break;
        }
        // Convert value to qcc::String
        qcc::String strValue = PlatformToMultibyteString(value);
        // Check for error in conversion
        if (strValue.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Call the real API
        _credentials->SetCertChain(strValue);
        // Store the result
        _credentials->_eventsAndProperties->CertChain = value;
        break;
    }

    // Bubble up any QStatus error as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Platform::String ^ Credentials::PrivateKey::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        // Check if the wrapped value exists
        if (nullptr == _credentials->_eventsAndProperties->PrivateKey) {
            // Call the real API
            qcc::String strPrivateKey = _credentials->GetPrivateKey();
            // Convert result to Platform::String
            result = MultibyteToPlatformString(strPrivateKey.c_str());
            // Check for conversion error
            if (nullptr == result && !strPrivateKey.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            // Store the result
            _credentials->_eventsAndProperties->PrivateKey = result;
        } else {
            // Return PrivateKey
            result = _credentials->_eventsAndProperties->PrivateKey;
        }
        break;
    }

    // Bubble up any QStatus error as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void Credentials::PrivateKey::set(Platform::String ^ value)
{
    ::QStatus status = ER_OK;

    while (true) {
        // Check for invalid values in value
        if (nullptr == value) {
            status = ER_BAD_ARG_1;
            break;
        }
        // Convert value to qcc::String
        qcc::String strValue = PlatformToMultibyteString(value);
        // Check for conversion error
        if (strValue.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Call the real API
        _credentials->SetPrivateKey(strValue);
        // Store the result
        _credentials->_eventsAndProperties->PrivateKey = value;
        break;
    }

    // Bubble up any QStatus error as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Platform::String ^ Credentials::LogonEntry::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        // Check if the wrapped value already exists
        if (nullptr == _credentials->_eventsAndProperties->LogonEntry) {
            // Call the real API
            qcc::String strLogonEntry = _credentials->GetLogonEntry();
            // Convert result to Platform::Strig
            result = MultibyteToPlatformString(strLogonEntry.c_str());
            // Check for conversion error
            if (nullptr == result && !strLogonEntry.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            // Store the result
            _credentials->_eventsAndProperties->LogonEntry = result;
        } else {
            // Return LogonEntry
            result = _credentials->_eventsAndProperties->LogonEntry;
        }
        break;
    }

    // Bubble up any QStatus error as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void Credentials::LogonEntry::set(Platform::String ^ value)
{
    ::QStatus status = ER_OK;

    while (true) {
        // Check for invalid values in value
        if (nullptr == value) {
            status = ER_BAD_ARG_1;
            break;
        }
        // Convert value to qcc::String
        qcc::String strValue = PlatformToMultibyteString(value);
        // Check for conversion error
        if (strValue.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Call the real API
        _credentials->SetLogonEntry(strValue);
        // Store the result
        _credentials->_eventsAndProperties->LogonEntry = value;
        break;
    }

    // Bubble up any QStatus error as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

uint32_t Credentials::Expiration::get()
{
    uint32_t result = (uint32_t)-1;

    // Check if value already exists
    if ((uint32_t)-1 == _credentials->_eventsAndProperties->Expiration) {
        // Call the real API
        result = _credentials->GetExpiration();
        // Return the result
        _credentials->_eventsAndProperties->Expiration = result;
    } else {
        // Return expiration
        result = _credentials->_eventsAndProperties->Expiration;
    }

    return result;
}

void Credentials::Expiration::set(uint32_t value)
{
    // Call the real API
    _credentials->SetExpiration(value);
    // Set the result
    _credentials->_eventsAndProperties->Expiration = value;
}

_Credentials::_Credentials()
    : ajn::AuthListener::Credentials()
{
    ::QStatus status = ER_OK;

    while (true) {
        // Create internal ref class
        _eventsAndProperties = ref new __Credentials();
        // Check for allocation error
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    // Bubble up any QStatus error as an exception
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
