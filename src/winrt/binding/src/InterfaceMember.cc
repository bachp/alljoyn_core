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

#include "InterfaceMember.h"

#include <InterfaceDescription.h>
#include <BusAttachment.h>
#include <qcc/String.h>
#include <qcc/winrt/utility.h>
#include <ObjectReference.h>
#include <AllJoynException.h>

namespace AllJoyn {

InterfaceMember::InterfaceMember(InterfaceDescription ^ iface, AllJoynMessageType type, Platform::String ^ name,
                                 Platform::String ^ signature, Platform::String ^ returnSignature, Platform::String ^ argNames,
                                 uint8_t annotation, Platform::String ^ accessPerms)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == iface) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::InterfaceDescription* id = *(iface->_interfaceDescr);
        if (NULL == id) {
            status = ER_FAIL;
            break;
        }
        if (nullptr == name) {
            status = ER_BAD_ARG_3;
            break;
        }
        qcc::String strName = PlatformToMultibyteString(name);
        if (strName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == signature) {
            status = ER_BAD_ARG_4;
            break;
        }
        qcc::String strSignature = PlatformToMultibyteString(signature);
        if (strSignature.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == returnSignature) {
            status = ER_BAD_ARG_5;
            break;
        }
        qcc::String strReturnSignature = PlatformToMultibyteString(returnSignature);
        if (strReturnSignature.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == argNames) {
            status = ER_BAD_ARG_6;
            break;
        }
        qcc::String strArgNames = PlatformToMultibyteString(argNames);
        if (strArgNames.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strAccessPerms = PlatformToMultibyteString(accessPerms);
        if (nullptr != accessPerms && strAccessPerms.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _InterfaceMember* m = new _InterfaceMember(id, (ajn::AllJoynMessageType)(int)type, strName.c_str(),
                                                   strSignature.c_str(), strReturnSignature.c_str(), strArgNames.c_str(),
                                                   annotation, strAccessPerms.c_str());
        if (NULL == m) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mMember = new qcc::ManagedObj<_InterfaceMember>(m);
        if (NULL == _mMember) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _member = &(**_mMember);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

InterfaceMember::InterfaceMember(void* interfaceMember)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == interfaceMember) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::InterfaceDescription::Member* member = reinterpret_cast<ajn::InterfaceDescription::Member*>(interfaceMember);
        _InterfaceMember* m = new _InterfaceMember(member);
        if (NULL == m) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mMember = new qcc::ManagedObj<_InterfaceMember>(m);
        if (NULL == _mMember) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _member = &(**_mMember);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

InterfaceMember::~InterfaceMember()
{
    if (NULL != _mMember) {
        delete _mMember;
        _mMember = NULL;
        _member = NULL;
    }
}

InterfaceDescription ^ InterfaceMember::Interface::get()
{
    ::QStatus status = ER_OK;
    InterfaceDescription ^ result = nullptr;

    while (true) {
        if (nullptr == _member->_eventsAndProperties->Interface) {
            result = ref new InterfaceDescription(_member->_member->iface);
            if (nullptr == result) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _member->_eventsAndProperties->Interface = result;
        } else {
            result = _member->_eventsAndProperties->Interface;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

AllJoynMessageType InterfaceMember::MemberType::get()
{
    ::QStatus status = ER_OK;
    AllJoynMessageType result = (AllJoynMessageType)(int)-1;

    while (true) {
        if ((AllJoynMessageType)(int)-1 == _member->_eventsAndProperties->MemberType) {
            result = (AllJoynMessageType)(int)_member->_member->memberType;
            _member->_eventsAndProperties->MemberType = result;
        } else {
            result = _member->_eventsAndProperties->MemberType;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ InterfaceMember::Name::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _member->_eventsAndProperties->Name) {
            qcc::String strName = _member->_member->name;
            result = MultibyteToPlatformString(strName.c_str());
            if (nullptr == result && !strName.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _member->_eventsAndProperties->Name = result;
        } else {
            result = _member->_eventsAndProperties->Name;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ InterfaceMember::Signature::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _member->_eventsAndProperties->Signature) {
            qcc::String strSignature = _member->_member->signature;
            result = MultibyteToPlatformString(strSignature.c_str());
            if (nullptr == result && !strSignature.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _member->_eventsAndProperties->Signature = result;
        } else {
            result = _member->_eventsAndProperties->Signature;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ InterfaceMember::ReturnSignature::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _member->_eventsAndProperties->ReturnSignature) {
            qcc::String strReturnSignature = _member->_member->returnSignature;
            result = MultibyteToPlatformString(strReturnSignature.c_str());
            if (nullptr == result && !strReturnSignature.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _member->_eventsAndProperties->ReturnSignature = result;
        } else {
            result = _member->_eventsAndProperties->ReturnSignature;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ InterfaceMember::ArgNames::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _member->_eventsAndProperties->ArgNames) {
            qcc::String strArgNames = _member->_member->argNames;
            result = MultibyteToPlatformString(strArgNames.c_str());
            if (nullptr == result && !strArgNames.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _member->_eventsAndProperties->ArgNames = result;
        } else {
            result = _member->_eventsAndProperties->ArgNames;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

/*
   uint8_t InterfaceMember::Annotation::get()
   {
    ::QStatus status = ER_OK;
    uint8_t result = (uint8_t)-1;

    while (true) {
        if ((uint8_t)-1 == _member->_eventsAndProperties->Annotation) {
            result = _member->_member->annotation;
            _member->_eventsAndProperties->Annotation = result;
        } else {
            result = _member->_eventsAndProperties->Annotation;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
   }
 */

Platform::String ^ InterfaceMember::AccessPerms::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _member->_eventsAndProperties->AccessPerms) {
            qcc::String strAccessPerms = _member->_member->accessPerms;
            result = MultibyteToPlatformString(strAccessPerms.c_str());
            if (nullptr == result && !strAccessPerms.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _member->_eventsAndProperties->AccessPerms = result;
        } else {
            result = _member->_eventsAndProperties->AccessPerms;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

_InterfaceMember::_InterfaceMember(const ajn::InterfaceDescription* iface, ajn::AllJoynMessageType type, const char* name,
                                   const char* signature, const char* returnSignature, const char* argNames,
                                   uint8_t annotation, const char* accessPerms)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __InterfaceMember();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _member = new ajn::InterfaceDescription::Member(iface, type, name,
                                                        signature, returnSignature, argNames,
                                                        annotation, accessPerms);
        if (NULL == _member) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_InterfaceMember::_InterfaceMember(ajn::InterfaceDescription::Member* member)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __InterfaceMember();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (NULL == member) {
            status = ER_BAD_ARG_1;
            break;
        }
        _member = new ajn::InterfaceDescription::Member(member->iface, member->memberType, member->name.c_str(),
                                                        member->signature.c_str(), member->returnSignature.c_str(), member->argNames.c_str(),
                                                        0, member->accessPerms.c_str());
        if (NULL == _member) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_InterfaceMember::~_InterfaceMember()
{
    _eventsAndProperties = nullptr;
    if (NULL != _member) {
        delete _member;
        _member = NULL;
    }
}

_InterfaceMember::operator ajn::InterfaceDescription::Member * ()
{
    return _member;
}

__InterfaceMember::__InterfaceMember()
{
    Interface = nullptr;
    MemberType = (AllJoynMessageType)(int)-1;
    Name = nullptr;
    Signature = nullptr;
    ReturnSignature = nullptr;
    ArgNames = nullptr;
    Annotation = (uint8_t)-1;
    AccessPerms = nullptr;
}

__InterfaceMember::~__InterfaceMember()
{
    Interface = nullptr;
    MemberType = (AllJoynMessageType)(int)-1;
    Name = nullptr;
    Signature = nullptr;
    ReturnSignature = nullptr;
    ArgNames = nullptr;
    Annotation = (uint8_t)-1;
    AccessPerms = nullptr;
}

}
