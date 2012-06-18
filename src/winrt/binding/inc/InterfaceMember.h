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

namespace AllJoyn {

ref class InterfaceDescription;

ref class __InterfaceMember {
  private:
    friend ref class InterfaceMember;
    friend class _InterfaceMember;
    __InterfaceMember();
    ~__InterfaceMember();

    property InterfaceDescription ^ Interface;
    property AllJoynMessageType MemberType;
    property Platform::String ^ Name;
    property Platform::String ^ Signature;
    property Platform::String ^ ReturnSignature;
    property Platform::String ^ ArgNames;
    property uint8_t Annotation;
    property Platform::String ^ AccessPerms;
};

class _InterfaceMember {
  protected:
    friend class qcc::ManagedObj<_InterfaceMember>;
    friend ref class InterfaceMember;
    friend ref class BusAttachment;
    friend ref class BusObject;
    friend class _BusObject;
    friend ref class ProxyBusObject;
    _InterfaceMember(const ajn::InterfaceDescription* iface, ajn::AllJoynMessageType type, const char* name,
                     const char* signature, const char* returnSignature, const char* argNames,
                     uint8_t annotation, const char* accessPerms);
    _InterfaceMember(ajn::InterfaceDescription::Member* member);
    ~_InterfaceMember();

    operator ajn::InterfaceDescription::Member * ();

    __InterfaceMember ^ _eventsAndProperties;
    ajn::InterfaceDescription::Member* _member;
};

public ref class InterfaceMember sealed {
  public:
    InterfaceMember(InterfaceDescription ^ iface, AllJoynMessageType type, Platform::String ^ name,
                    Platform::String ^ signature, Platform::String ^ returnSignature, Platform::String ^ argNames,
                    uint8_t annotation, Platform::String ^ accessPerms);
    InterfaceMember::~InterfaceMember();

    property InterfaceDescription ^ Interface
    {
        InterfaceDescription ^ get();
    }

    property AllJoynMessageType MemberType
    {
        AllJoynMessageType get();
    }

    property Platform::String ^ Name
    {
        Platform::String ^ get();
    }

    property Platform::String ^ Signature
    {
        Platform::String ^ get();
    }

    property Platform::String ^ ReturnSignature
    {
        Platform::String ^ get();
    }

    property Platform::String ^ ArgNames
    {
        Platform::String ^ get();
    }

    property uint8_t Annotation
    {
        uint8_t get();
    }

    property Platform::String ^ AccessPerms
    {
        Platform::String ^ get();
    }

  private:
    friend ref class BusAttachment;
    friend ref class BusObject;
    friend class _BusObject;
    friend ref class ProxyBusObject;
    friend class _MessageReceiver;
    friend ref class InterfaceDescription;
    InterfaceMember(void* interfaceMember);

    qcc::ManagedObj<_InterfaceMember>* _mMember;
    _InterfaceMember* _member;
};

}
// InterfaceMember.h
