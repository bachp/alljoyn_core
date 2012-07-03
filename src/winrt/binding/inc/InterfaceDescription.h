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

#include <alljoyn/InterfaceDescription.h>
#include <Message.h>
#include <InterfaceMember.h>
#include <InterfaceProperty.h>

namespace AllJoyn {

ref class InterfaceDescription;

public enum class PropAccessType {
    PROP_ACCESS_READ  = ajn::PROP_ACCESS_READ,
    PROP_ACCESS_WRITE = ajn::PROP_ACCESS_WRITE,
    PROP_ACCESS_RW    = ajn::PROP_ACCESS_RW
};

public enum class MemberAnnotationType {
    MEMBER_ANNOTATE_NO_REPLY   = ajn::MEMBER_ANNOTATE_NO_REPLY,
    MEMBER_ANNOTATE_DEPRECATED = ajn::MEMBER_ANNOTATE_DEPRECATED
};

ref class __InterfaceDescription {
  private:
    friend ref class InterfaceDescription;
    friend class _InterfaceDescription;
    __InterfaceDescription();
    ~__InterfaceDescription();

    property Platform::String ^ Name;
};

class _InterfaceDescription {
  protected:
    friend class qcc::ManagedObj<_InterfaceDescription>;
    friend ref class InterfaceDescription;
    friend ref class BusAttachment;
    friend ref class BusObject;
    friend ref class InterfaceMember;
    friend ref class ProxyBusObject;
    _InterfaceDescription(ajn::InterfaceDescription* interfaceDescr);
    ~_InterfaceDescription();

    operator ajn::InterfaceDescription * ();

    __InterfaceDescription ^ _eventsAndProperties;
    ajn::InterfaceDescription* _interfaceDescr;
};

public ref class InterfaceDescription sealed {
  public:
    void AddMember(AllJoynMessageType type, Platform::String ^ name, Platform::String ^ inputSig, Platform::String ^ outSig, Platform::String ^ argNames, uint8_t annotation, Platform::String ^ accessPerms);
    InterfaceMember ^ GetMember(Platform::String ^ name);
    uint32_t GetMembers(Platform::WriteOnlyArray<InterfaceMember ^> ^ members);
    bool HasMember(Platform::String ^ name, Platform::String ^ inSig, Platform::String ^ outSig);
    void AddMethod(Platform::String ^ name, Platform::String ^ inputSig, Platform::String ^ outSig, Platform::String ^ argNames, uint8_t annotation, Platform::String ^ accessPerms);
    InterfaceMember ^ GetMethod(Platform::String ^ name);
    void AddSignal(Platform::String ^ name, Platform::String ^ sig, Platform::String ^ argNames, uint8_t annotation, Platform::String ^ accessPerms);
    InterfaceMember ^ GetSignal(Platform::String ^ name);
    InterfaceProperty ^ GetProperty(Platform::String ^ name);
    uint32_t GetProperties(Platform::WriteOnlyArray<InterfaceProperty ^> ^ props);
    void AddProperty(Platform::String ^ name, Platform::String ^ signature, uint8_t access);
    bool HasProperty(Platform::String ^ name);
    bool HasProperties();
    Platform::String ^ Introspect(uint32_t indent);
    void Activate();
    bool IsSecure();

    property Platform::String ^ Name
    {
        Platform::String ^ get();
    }

  private:
    friend ref class BusAttachment;
    friend ref class BusObject;
    friend ref class InterfaceMember;
    friend ref class ProxyBusObject;
    InterfaceDescription(void* interfaceDescr, bool isManaged);
    ~InterfaceDescription();

    qcc::ManagedObj<_InterfaceDescription>* _mInterfaceDescr;
    _InterfaceDescription* _interfaceDescr;
};

}
// InterfaceDescription.h
