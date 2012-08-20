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

#include "InterfaceProperty.h"

#include <BusAttachment.h>
#include <qcc/String.h>
#include <qcc/winrt/utility.h>
#include <ObjectReference.h>
#include <AllJoynException.h>

namespace AllJoyn {

InterfaceProperty::InterfaceProperty(Platform::String ^ name, Platform::String ^ signature, uint8_t access)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == name) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strName = PlatformToMultibyteString(name);
        if (strName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == signature) {
            status = ER_BAD_ARG_2;
            break;
        }
        qcc::String strSignature = PlatformToMultibyteString(signature);
        if (strSignature.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _InterfaceProperty* p = new _InterfaceProperty(strName.c_str(), strSignature.c_str(), access);
        if (NULL == p) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mProperty = new qcc::ManagedObj<_InterfaceProperty>(p);
        if (NULL == _mProperty) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _property = &(**_mProperty);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

InterfaceProperty::InterfaceProperty(void* interfaceProperty)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == interfaceProperty) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::InterfaceDescription::Property* property = reinterpret_cast<ajn::InterfaceDescription::Property*>(interfaceProperty);
        _InterfaceProperty* p = new _InterfaceProperty(property->name.c_str(), property->signature.c_str(), property->access);
        if (NULL == p) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mProperty = new qcc::ManagedObj<_InterfaceProperty>(p);
        if (NULL == _mProperty) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _property = &(**_mProperty);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

InterfaceProperty::~InterfaceProperty()
{
    if (NULL != _mProperty) {
        delete _mProperty;
        _mProperty = NULL;
        _property = NULL;
    }
}

Platform::String ^ InterfaceProperty::Name::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _property->_eventsAndProperties->Name) {
            qcc::String strName = _property->_property->name;
            result = MultibyteToPlatformString(strName.c_str());
            if (nullptr == result && !strName.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _property->_eventsAndProperties->Name = result;
        } else {
            result = _property->_eventsAndProperties->Name;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ InterfaceProperty::Signature::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _property->_eventsAndProperties->Signature) {
            qcc::String strSignature = _property->_property->signature;
            result = MultibyteToPlatformString(strSignature.c_str());
            if (nullptr == result && !strSignature.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _property->_eventsAndProperties->Signature = result;
        } else {
            result = _property->_eventsAndProperties->Signature;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

uint8_t InterfaceProperty::Access::get()
{
    ::QStatus status = ER_OK;
    uint8_t result = (uint8_t)-1;

    while (true) {
        if ((uint8_t)-1 == _property->_eventsAndProperties->Access) {
            result = _property->_property->access;
            _property->_eventsAndProperties->Access = result;
        } else {
            result = _property->_eventsAndProperties->Access;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

_InterfaceProperty::_InterfaceProperty(const char* name, const char* signature, uint8_t access)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __InterfaceProperty();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _property = new ajn::InterfaceDescription::Property(name, signature, access);
        if (NULL == _property) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_InterfaceProperty::_InterfaceProperty(ajn::InterfaceDescription::Property* property)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __InterfaceProperty();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (NULL == property) {
            status = ER_BAD_ARG_1;
            break;
        }
        _property = new ajn::InterfaceDescription::Property(property->name.c_str(), property->signature.c_str(), property->access);
        if (NULL == _property) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_InterfaceProperty::~_InterfaceProperty()
{
    _eventsAndProperties = nullptr;
    if (NULL != _property) {
        delete _property;
        _property = NULL;
    }
}

_InterfaceProperty::operator ajn::InterfaceDescription::Property * ()
{
    return _property;
}

__InterfaceProperty::__InterfaceProperty()
{
    Name = nullptr;
    Signature = nullptr;
    Access = (uint8_t)-1;
}

__InterfaceProperty::~__InterfaceProperty()
{
    Name = nullptr;
    Signature = nullptr;
    Access = (uint8_t)-1;
}

}
