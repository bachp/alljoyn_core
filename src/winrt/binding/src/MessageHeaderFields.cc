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

#include "MessageHeaderFields.h"

#include <qcc/winrt/utility.h>
#include <ObjectReference.h>
#include <AllJoynException.h>

namespace AllJoyn {

MessageHeaderFields::MessageHeaderFields(const ajn::HeaderFields* headers)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == headers) {
            status = ER_BAD_ARG_1;
            break;
        }
        _MessageHeaderFields* hf = new _MessageHeaderFields(headers);
        if (NULL == hf) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mMessageHeaderFields = new qcc::ManagedObj<_MessageHeaderFields>(hf);
        if (NULL == _mMessageHeaderFields) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _messageHeaderFields = &(**_mMessageHeaderFields);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

MessageHeaderFields::MessageHeaderFields(const qcc::ManagedObj<_MessageHeaderFields>* headers)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == headers) {
            status = ER_BAD_ARG_1;
            break;
        }
        _mMessageHeaderFields = new qcc::ManagedObj<_MessageHeaderFields>(*headers);
        if (NULL == _mMessageHeaderFields) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _messageHeaderFields = &(**_mMessageHeaderFields);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

MessageHeaderFields::~MessageHeaderFields()
{
    if (NULL != _mMessageHeaderFields) {
        delete _mMessageHeaderFields;
        _mMessageHeaderFields = NULL;
        _messageHeaderFields = NULL;
    }
}

Platform::String ^ MessageHeaderFields::ConvertToString(uint32_t indent)
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        qcc::String ret = _messageHeaderFields->ToString(indent);
        result = MultibyteToPlatformString(ret.c_str());
        if (nullptr == result && !ret.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::Array<MsgArg ^> ^ MessageHeaderFields::Field::get()
{
    ::QStatus status = ER_OK;
    Platform::Array<MsgArg ^> ^ result = nullptr;

    while (true) {
        if (nullptr == _messageHeaderFields->_eventsAndProperties->Field) {
            result = ref new Platform::Array<MsgArg ^>(ajn::ALLJOYN_HDR_FIELD_UNKNOWN);
            if (nullptr == result) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            for (int i = 0; i < ajn::ALLJOYN_HDR_FIELD_UNKNOWN; i++) {
                MsgArg ^ tempArg = ref new MsgArg(&(_messageHeaderFields->field[i]));
                if (nullptr == tempArg) {
                    status = ER_OUT_OF_MEMORY;
                    break;
                }
                result[i] = tempArg;
            }
            _messageHeaderFields->_eventsAndProperties->Field = result;
        } else {
            result = _messageHeaderFields->_eventsAndProperties->Field;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::Array<bool> ^ MessageHeaderFields::Compressible::get()
{
    ::QStatus status = ER_OK;
    Platform::Array<bool> ^ result = nullptr;

    while (true) {
        if (nullptr == _messageHeaderFields->_eventsAndProperties->Compressible) {
            result = ref new Platform::Array<bool>(ajn::ALLJOYN_HDR_FIELD_UNKNOWN + 1);
            if (nullptr == result) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            for (int i = 0; i < ajn::ALLJOYN_HDR_FIELD_UNKNOWN + 1; i++) {
                result[i] = _messageHeaderFields->Compressible[i];
            }
            _messageHeaderFields->_eventsAndProperties->Compressible = result;
        } else {
            result = _messageHeaderFields->_eventsAndProperties->Compressible;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::Array<AllJoynTypeId> ^ MessageHeaderFields::FieldType::get()
{
    ::QStatus status = ER_OK;
    Platform::Array<AllJoynTypeId> ^ result = nullptr;

    while (true) {
        if (nullptr == _messageHeaderFields->_eventsAndProperties->FieldType) {
            result = ref new Platform::Array<AllJoynTypeId>(ajn::ALLJOYN_HDR_FIELD_UNKNOWN + 1);
            if (nullptr == result) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            for (int i = 0; i < ajn::ALLJOYN_HDR_FIELD_UNKNOWN + 1; i++) {
                result[i] = (AllJoynTypeId)(int)_messageHeaderFields->FieldType[i];
            }
            _messageHeaderFields->_eventsAndProperties->FieldType = result;
        } else {
            result = _messageHeaderFields->_eventsAndProperties->FieldType;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

_MessageHeaderFields::_MessageHeaderFields(const ajn::HeaderFields* headers)
    : ajn::HeaderFields(*headers)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __MessageHeaderFields();
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

_MessageHeaderFields::~_MessageHeaderFields()
{
    _eventsAndProperties = nullptr;
}

__MessageHeaderFields::__MessageHeaderFields()
{
    Field = nullptr;
    Compressible = nullptr;
    FieldType = nullptr;
}

__MessageHeaderFields::~__MessageHeaderFields()
{
    Field = nullptr;
    Compressible = nullptr;
    FieldType = nullptr;
}

}
