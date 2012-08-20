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

#include "InterfaceDescription.h"

#include <InterfaceMember.h>
#include <InterfaceProperty.h>
#include <BusAttachment.h>
#include <qcc/String.h>
#include <qcc/winrt/utility.h>
#include <ObjectReference.h>
#include <Collection.h>
#include <AllJoynException.h>

namespace AllJoyn {

InterfaceDescription::InterfaceDescription(const ajn::InterfaceDescription* interfaceDescr)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == interfaceDescr) {
            status = ER_BAD_ARG_1;
            break;
        }
        _InterfaceDescription* intdescr = new _InterfaceDescription(interfaceDescr);
        if (NULL == intdescr) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mInterfaceDescr = new qcc::ManagedObj<_InterfaceDescription>(intdescr);
        if (NULL == _mInterfaceDescr) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _interfaceDescr = &(**_mInterfaceDescr);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

InterfaceDescription::InterfaceDescription(const qcc::ManagedObj<_InterfaceDescription>* interfaceDescr)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == interfaceDescr) {
            status = ER_BAD_ARG_1;
            break;
        }
        _mInterfaceDescr = new qcc::ManagedObj<_InterfaceDescription>(*interfaceDescr);
        if (NULL == _mInterfaceDescr) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _interfaceDescr = &(**_mInterfaceDescr);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

InterfaceDescription::~InterfaceDescription()
{
    if (NULL != _mInterfaceDescr) {
        delete _mInterfaceDescr;
        _mInterfaceDescr = NULL;
        _interfaceDescr = NULL;
    }
}

void InterfaceDescription::AddMember(AllJoynMessageType type,
                                     Platform::String ^ name,
                                     Platform::String ^ inputSig,
                                     Platform::String ^ outSig,
                                     Platform::String ^ argNames,
                                     uint8_t annotation,
                                     Platform::String ^ accessPerms)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == name) {
            status = ER_BAD_ARG_2;
            break;
        }
        qcc::String strName = PlatformToMultibyteString(name);
        if (strName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strInputSig = PlatformToMultibyteString(inputSig);
        if (nullptr != inputSig && strInputSig.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strOutSig = PlatformToMultibyteString(outSig);
        if (nullptr != outSig && strOutSig.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == argNames) {
            status = ER_BAD_ARG_5;
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
        status = ((ajn::InterfaceDescription*)*_interfaceDescr)->AddMember((ajn::AllJoynMessageType)(int)type, strName.c_str(), strInputSig.c_str(), strOutSig.c_str(),
                                                                           strArgNames.c_str(), annotation, strAccessPerms.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void InterfaceDescription::AddMemberWithMap(AllJoynMessageType type,
                                            Platform::String ^ name,
                                            Platform::String ^ inputSig,
                                            Platform::String ^ outSig,
                                            Platform::String ^ argNames,
                                            Windows::Foundation::Collections::IMapView<Platform::String ^, Platform::String ^> ^ annotations,
                                            Platform::String ^ accessPerms)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == name) {
            status = ER_BAD_ARG_2;
            break;
        }
        qcc::String strName = PlatformToMultibyteString(name);
        if (strName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strInputSig = PlatformToMultibyteString(inputSig);
        if (nullptr != inputSig && strInputSig.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strOutSig = PlatformToMultibyteString(outSig);
        if (nullptr != outSig && strOutSig.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == argNames) {
            status = ER_BAD_ARG_5;
            break;
        }
        qcc::String strArgNames = PlatformToMultibyteString(argNames);
        if (strArgNames.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::InterfaceDescription::AnnotationsMap annotationsMap;
        for (Platform::Collections::InputIterator<Windows::Foundation::Collections::IKeyValuePair<Platform::String ^, Platform::String ^> ^> iter = Windows::Foundation::Collections::begin(annotations);
             iter != Windows::Foundation::Collections::end(annotations);
             ++iter) {
            Windows::Foundation::Collections::IKeyValuePair<Platform::String ^, Platform::String ^> ^ kvp = *iter;
            qcc::String strKey = PlatformToMultibyteString(kvp->Key);
            if (nullptr != kvp->Key && strKey.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            qcc::String strValue = PlatformToMultibyteString(kvp->Value);
            if (nullptr != kvp->Value && strValue.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            annotationsMap[strKey] = strValue;
        }
        if (ER_OK != status) {
            break;
        }
        qcc::String strAccessPerms = PlatformToMultibyteString(accessPerms);
        if (nullptr != accessPerms && strAccessPerms.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = ((ajn::InterfaceDescription*)*_interfaceDescr)->AddMember((ajn::AllJoynMessageType)(int)type,
                                                                           strName.c_str(),
                                                                           strInputSig.c_str(),
                                                                           strOutSig.c_str(),
                                                                           strArgNames.c_str(),
                                                                           annotationsMap,
                                                                           strAccessPerms.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void InterfaceDescription::AddMemberAnnotation(Platform::String ^ member, Platform::String ^ name, Platform::String ^ value)
{
    ::QStatus status = ER_OK;

    while (true) {
        qcc::String strMember = PlatformToMultibyteString(member);
        if (nullptr != member && strMember.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strName = PlatformToMultibyteString(name);
        if (nullptr != name && strName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strValue = PlatformToMultibyteString(value);
        if (nullptr != value && strValue.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = ((ajn::InterfaceDescription*)*_interfaceDescr)->AddMemberAnnotation(strMember.c_str(), strName, strValue);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Platform::String ^ InterfaceDescription::GetMemberAnnotation(Platform::String ^ member, Platform::String ^ name)
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        qcc::String strMember = PlatformToMultibyteString(member);
        if (nullptr != member && strMember.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strName = PlatformToMultibyteString(name);
        if (nullptr != name && strName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strValue;
        if (((ajn::InterfaceDescription*)*_interfaceDescr)->GetMemberAnnotation(strMember.c_str(), strName, strValue)) {
            result = MultibyteToPlatformString(strValue.c_str());
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

InterfaceMember ^ InterfaceDescription::GetMember(Platform::String ^ name)
{
    ::QStatus status = ER_OK;
    InterfaceMember ^ im = nullptr;

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
        const ajn::InterfaceDescription::Member* member = ((ajn::InterfaceDescription*)*_interfaceDescr)->GetMember(strName.c_str());
        if (NULL == member) {
            status = ER_FAIL;
            break;
        }
        im = ref new InterfaceMember((void*)member);
        if (nullptr == im) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return im;
}

uint32_t InterfaceDescription::GetMembers(Platform::WriteOnlyArray<InterfaceMember ^> ^ members)
{
    ::QStatus status = ER_OK;
    ajn::InterfaceDescription::Member** memberArray = NULL;
    size_t result = -1;

    while (true) {
        if (nullptr != members &&  members->Length >  0) {
            memberArray = new ajn::InterfaceDescription::Member * [members->Length];
            if (NULL == memberArray) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
        }
        result = ((ajn::InterfaceDescription*)*_interfaceDescr)->GetMembers((const ajn::InterfaceDescription::Member**)memberArray, (nullptr != members) ? members->Length : 0);
        if (result > 0 && NULL != memberArray) {
            for (int i = 0; i < result; i++) {
                InterfaceMember ^ tempMember = ref new InterfaceMember((void*)memberArray[i]);
                if (nullptr == tempMember) {
                    status = ER_OUT_OF_MEMORY;
                    break;
                }
                members[i] = tempMember;
            }
        }
        break;
    }

    if (NULL != memberArray) {
        delete [] memberArray;
        memberArray = NULL;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

bool InterfaceDescription::HasMember(Platform::String ^ name, Platform::String ^ inSig, Platform::String ^ outSig)
{
    ::QStatus status = ER_OK;
    bool result = false;

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
        qcc::String strInputSig = PlatformToMultibyteString(inSig);
        if (nullptr != inSig && strInputSig.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strOutSig = PlatformToMultibyteString(outSig);
        if (nullptr != outSig && strOutSig.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        result = ((ajn::InterfaceDescription*)*_interfaceDescr)->HasMember(strName.c_str(), strInputSig.c_str(), strOutSig.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void InterfaceDescription::AddMethod(Platform::String ^ name,
                                     Platform::String ^ inputSig,
                                     Platform::String ^ outSig,
                                     Platform::String ^ argNames,
                                     uint8_t annotation,
                                     Platform::String ^ accessPerms)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == name) {
            status = ER_BAD_ARG_2;
            break;
        }
        qcc::String strName = PlatformToMultibyteString(name);
        if (strName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strInputSig = PlatformToMultibyteString(inputSig);
        if (nullptr != inputSig && strInputSig.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strOutSig = PlatformToMultibyteString(outSig);
        if (nullptr != outSig && strOutSig.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == argNames) {
            status = ER_BAD_ARG_5;
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
        status = ((ajn::InterfaceDescription*)*_interfaceDescr)->AddMethod(strName.c_str(), strInputSig.c_str(), strOutSig.c_str(),
                                                                           strArgNames.c_str(), annotation, strAccessPerms.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void InterfaceDescription::AddMethodWithMap(Platform::String ^ name,
                                            Platform::String ^ inputSig,
                                            Platform::String ^ outSig,
                                            Platform::String ^ argNames,
                                            Windows::Foundation::Collections::IMapView<Platform::String ^, Platform::String ^> ^ annotations,
                                            Platform::String ^ accessPerms)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == name) {
            status = ER_BAD_ARG_2;
            break;
        }
        qcc::String strName = PlatformToMultibyteString(name);
        if (strName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strInputSig = PlatformToMultibyteString(inputSig);
        if (nullptr != inputSig && strInputSig.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strOutSig = PlatformToMultibyteString(outSig);
        if (nullptr != outSig && strOutSig.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == argNames) {
            status = ER_BAD_ARG_5;
            break;
        }
        qcc::String strArgNames = PlatformToMultibyteString(argNames);
        if (strArgNames.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::InterfaceDescription::AnnotationsMap annotationsMap;
        for (Platform::Collections::InputIterator<Windows::Foundation::Collections::IKeyValuePair<Platform::String ^, Platform::String ^> ^> iter = Windows::Foundation::Collections::begin(annotations);
             iter != Windows::Foundation::Collections::end(annotations);
             ++iter) {
            Windows::Foundation::Collections::IKeyValuePair<Platform::String ^, Platform::String ^> ^ kvp = *iter;
            qcc::String strKey = PlatformToMultibyteString(kvp->Key);
            if (nullptr != kvp->Key && strKey.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            qcc::String strValue = PlatformToMultibyteString(kvp->Value);
            if (nullptr != kvp->Value && strValue.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            annotationsMap[strKey] = strValue;
        }
        if (ER_OK != status) {
            break;
        }
        qcc::String strAccessPerms = PlatformToMultibyteString(accessPerms);
        if (nullptr != accessPerms && strAccessPerms.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = ((ajn::InterfaceDescription*)*_interfaceDescr)->AddMethod(strName.c_str(),
                                                                           strInputSig.c_str(),
                                                                           strOutSig.c_str(),
                                                                           strArgNames.c_str(),
                                                                           annotationsMap,
                                                                           strAccessPerms.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

InterfaceMember ^ InterfaceDescription::GetMethod(Platform::String ^ name)
{
    ::QStatus status = ER_OK;
    InterfaceMember ^ im = nullptr;

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
        const ajn::InterfaceDescription::Member* member = ((ajn::InterfaceDescription*)*_interfaceDescr)->GetMethod(strName.c_str());
        if (NULL == member) {
            status = ER_FAIL;
            break;
        }
        im = ref new InterfaceMember((void*)member);
        if (nullptr == im) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return im;
}

void InterfaceDescription::AddSignal(Platform::String ^ name, Platform::String ^ sig, Platform::String ^ argNames, uint8_t annotation, Platform::String ^ accessPerms)
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
        if (nullptr == sig) {
            status = ER_BAD_ARG_2;
            break;
        }
        qcc::String strSig = PlatformToMultibyteString(sig);
        if (strSig.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == argNames) {
            status = ER_BAD_ARG_3;
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
        status = ((ajn::InterfaceDescription*)*_interfaceDescr)->AddSignal(strName.c_str(), strSig.c_str(), strArgNames.c_str(), annotation, strAccessPerms.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

InterfaceMember ^ InterfaceDescription::GetSignal(Platform::String ^ name)
{
    ::QStatus status = ER_OK;
    InterfaceMember ^ im = nullptr;

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
        const ajn::InterfaceDescription::Member* member = ((ajn::InterfaceDescription*)*_interfaceDescr)->GetSignal(strName.c_str());
        if (NULL == member) {
            status = ER_FAIL;
            break;
        }
        im = ref new InterfaceMember((void*)member);
        if (nullptr == im) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return im;
}

InterfaceProperty ^ InterfaceDescription::GetProperty(Platform::String ^ name)
{
    ::QStatus status = ER_OK;
    InterfaceProperty ^ ip = nullptr;

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
        const ajn::InterfaceDescription::Property* property = ((ajn::InterfaceDescription*)*_interfaceDescr)->GetProperty(strName.c_str());
        if (NULL == property) {
            status = ER_FAIL;
            break;
        }
        ip = ref new InterfaceProperty((void*)property);
        if (nullptr == ip) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return ip;
}

uint32_t InterfaceDescription::GetProperties(Platform::WriteOnlyArray<InterfaceProperty ^> ^ props)
{
    ::QStatus status = ER_OK;
    ajn::InterfaceDescription::Property** propertyArray = NULL;
    size_t result = -1;

    while (true) {
        if (nullptr != props && props->Length > 0) {
            propertyArray = new ajn::InterfaceDescription::Property * [props->Length];
            if (NULL == propertyArray) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
        }
        result = ((ajn::InterfaceDescription*)*_interfaceDescr)->GetProperties((const ajn::InterfaceDescription::Property**)propertyArray, (nullptr != props) ? props->Length : 0);
        if (result > 0 && NULL != propertyArray) {
            for (int i = 0; i < result; i++) {
                InterfaceProperty ^ tempProperty = ref new InterfaceProperty((void*)propertyArray[i]);
                if (nullptr == tempProperty) {
                    status = ER_OUT_OF_MEMORY;
                    break;
                }
                props[i] = tempProperty;
            }
        }
        break;
    }

    if (NULL != propertyArray) {
        delete [] propertyArray;
        propertyArray = NULL;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void InterfaceDescription::AddProperty(Platform::String ^ name, Platform::String ^ signature, uint8_t access)
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
        status = ((ajn::InterfaceDescription*)*_interfaceDescr)->AddProperty(strName.c_str(), strSignature.c_str(), access);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void InterfaceDescription::AddPropertyAnnotation(Platform::String ^ member, Platform::String ^ name, Platform::String ^ value)
{
    ::QStatus status = ER_OK;

    while (true) {
        qcc::String strMember = PlatformToMultibyteString(member);
        if (nullptr != member && strMember.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strName = PlatformToMultibyteString(name);
        if (nullptr != name && strName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strValue = PlatformToMultibyteString(value);
        if (nullptr != value && strValue.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = ((ajn::InterfaceDescription*)*_interfaceDescr)->AddPropertyAnnotation(strMember.c_str(), strName, strValue);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Platform::String ^ InterfaceDescription::GetPropertyAnnotation(Platform::String ^ member, Platform::String ^ name)
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        qcc::String strMember = PlatformToMultibyteString(member);
        if (nullptr != member && strMember.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strName = PlatformToMultibyteString(name);
        if (nullptr != name && strName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strValue;
        if (((ajn::InterfaceDescription*)*_interfaceDescr)->GetPropertyAnnotation(strMember.c_str(), strName, strValue)) {
            result = MultibyteToPlatformString(strValue.c_str());
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

bool InterfaceDescription::HasProperty(Platform::String ^ name)
{
    ::QStatus status = ER_OK;
    bool result = false;

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
        result = ((ajn::InterfaceDescription*)*_interfaceDescr)->HasProperty(strName.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

bool InterfaceDescription::HasProperties()
{
    return ((ajn::InterfaceDescription*)*_interfaceDescr)->HasProperties();
}

Platform::String ^ InterfaceDescription::Introspect(uint32_t indent)
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        qcc::String strResult = ((ajn::InterfaceDescription*)*_interfaceDescr)->Introspect(indent);
        result = MultibyteToPlatformString(strResult.c_str());
        if (nullptr == result && !strResult.empty()) {
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

void InterfaceDescription::Activate()
{
    ((ajn::InterfaceDescription*)*_interfaceDescr)->Activate();
}

bool InterfaceDescription::IsSecure()
{
    return ((ajn::InterfaceDescription*)*_interfaceDescr)->IsSecure();
}

void InterfaceDescription::AddAnnotation(Platform::String ^ name, Platform::String ^ value)
{
    ::QStatus status = ER_OK;

    while (true) {
        qcc::String strName = PlatformToMultibyteString(name);
        if (nullptr != name && strName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strValue = PlatformToMultibyteString(value);
        if (nullptr != value && strValue.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = ((ajn::InterfaceDescription*)*_interfaceDescr)->AddAnnotation(strName, strValue);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Platform::String ^ InterfaceDescription::GetAnnotation(Platform::String ^ name)
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        qcc::String strName = PlatformToMultibyteString(name);
        if (nullptr != name && strName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strValue;
        if (((ajn::InterfaceDescription*)*_interfaceDescr)->GetAnnotation(strName, strValue)) {
            result = MultibyteToPlatformString(strValue.c_str());
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ InterfaceDescription::Name::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _interfaceDescr->_eventsAndProperties->Name) {
            qcc::String name = ((ajn::InterfaceDescription*)*_interfaceDescr)->GetName();
            result = MultibyteToPlatformString(name.c_str());
            if (nullptr == result && !name.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _interfaceDescr->_eventsAndProperties->Name = result;
        } else {
            result = _interfaceDescr->_eventsAndProperties->Name;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

_InterfaceDescription::_InterfaceDescription(const ajn::InterfaceDescription* interfaceDescr)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __InterfaceDescription();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _interfaceDescr = interfaceDescr;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_InterfaceDescription::~_InterfaceDescription()
{
    _eventsAndProperties = nullptr;
}

_InterfaceDescription::operator ajn::InterfaceDescription * ()
{
    return (ajn::InterfaceDescription*)_interfaceDescr;
}

__InterfaceDescription::__InterfaceDescription()
{
    Name = nullptr;
}

__InterfaceDescription::~__InterfaceDescription()
{
    Name = nullptr;
}

}
