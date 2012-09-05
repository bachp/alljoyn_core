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

ref class __InterfaceProperty {
  private:
    friend ref class InterfaceProperty;
    friend class _InterfaceProperty;
    __InterfaceProperty();
    ~__InterfaceProperty();

    property Platform::String ^ Name;
    property Platform::String ^ Signature;
    property uint8_t Access;
};

class _InterfaceProperty {
  protected:
    friend class qcc::ManagedObj<_InterfaceProperty>;
    friend ref class InterfaceProperty;
    _InterfaceProperty(const char* name, const char* signature, uint8_t access);
    _InterfaceProperty(const ajn::InterfaceDescription::Property* property);
    ~_InterfaceProperty();

    operator ajn::InterfaceDescription::Property * ();

    __InterfaceProperty ^ _eventsAndProperties;
    ajn::InterfaceDescription::Property* _property;
};

public ref class InterfaceProperty sealed {
  public:
    /// <summary>
    ///Constructor
    /// </summary>
    /// <param name="name">Name of the property</param>
    /// <param name="signature">Signature of the property</param>
    /// <param name="access">Access flags for the property</param>
    InterfaceProperty(Platform::String ^ name, Platform::String ^ signature, uint8_t access);

    /// <summary
    ///>Name of the property
    /// </summary>
    property Platform::String ^ Name
    {
        Platform::String ^  get();
    }

    /// <summary>
    ///Signature of the property
    /// </summary>
    property Platform::String ^ Signature
    {
        Platform::String ^  get();
    }

    /// <summary>
    ///Access flags for the property
    /// </summary>
    property uint8_t Access
    {
        uint8_t get();
    }

  private:
    friend ref class InterfaceDescription;
    InterfaceProperty(const ajn::InterfaceDescription::Property * interfaceProperty);
    ~InterfaceProperty();

    qcc::ManagedObj<_InterfaceProperty>* _mProperty;
    _InterfaceProperty* _property;
};

}
// InterfaceProperty.h
