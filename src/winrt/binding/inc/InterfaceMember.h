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
    /// <summary>
    /// constructor
    /// </summary>
    /// <param name="iface">Interface that this member belongs to</param>
    /// <param name="type">Member type</param>
    /// <param name="name">Member name</param>
    /// <param name="signature">Method call IN arguments (NULL for signals)</param>
    /// <param name="returnSignature">Signal or method call OUT arguments</param>
    /// <param name="argNames">Comma separated list of argument names - can be NULL</param>
    /// <param name="annotation">Exclusive OR of flags MEMBER_ANNOTATE_NO_REPLY and MEMBER_ANNOTATE_DEPRECATED</param>
    /// <param name="accessPerms">Required permissions to invoke this call</param>
    InterfaceMember(InterfaceDescription ^ iface, AllJoynMessageType type, Platform::String ^ name,
                    Platform::String ^ signature, Platform::String ^ returnSignature, Platform::String ^ argNames,
                    uint8_t annotation, Platform::String ^ accessPerms);

    /// <summary>
    ///Interface that this member belongs to
    /// </summary>
    property InterfaceDescription ^ Interface
    {
        InterfaceDescription ^ get();
    }

    /// <summary>
    ///Member type
    /// </summary>
    property AllJoynMessageType MemberType
    {
        AllJoynMessageType get();
    }

    /// <summary>
    ///Member name
    /// </summary>
    property Platform::String ^ Name
    {
        Platform::String ^ get();
    }

    /// <summary>
    ///Method call IN arguments (NULL for signals)
    /// </summary>
    property Platform::String ^ Signature
    {
        Platform::String ^ get();
    }

    /// <summary>
    ///Signal or method call OUT arguments
    /// </summary>
    property Platform::String ^ ReturnSignature
    {
        Platform::String ^ get();
    }

    /// <summary>
    ///Comma separated list of argument names - can be NULL
    /// </summary>
    property Platform::String ^ ArgNames
    {
        Platform::String ^ get();
    }

    /// <summary>
    ///Exclusive OR of flags MEMBER_ANNOTATE_NO_REPLY and MEMBER_ANNOTATE_DEPRECATED
    /// </summary>
    //property uint8_t Annotation
    //{
    //    uint8_t get();
    //}

    /// <summary>
    ///Required permissions to invoke this call
    /// </summary>
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
    ~InterfaceMember();

    qcc::ManagedObj<_InterfaceMember>* _mMember;
    _InterfaceMember* _member;
};

}
// InterfaceMember.h
