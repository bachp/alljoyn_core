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

#include <alljoyn/MsgArg.h>
#include <qcc/String.h>
#include <map>
#include <list>
#include <qcc/ManagedObj.h>
#include <qcc/Mutex.h>

namespace AllJoyn {

public enum class AllJoynTypeId {
    ALLJOYN_INVALID          = ajn::ALLJOYN_INVALID,
    ALLJOYN_ARRAY            = ajn::ALLJOYN_ARRAY,
    ALLJOYN_BOOLEAN          = ajn::ALLJOYN_BOOLEAN,
    ALLJOYN_DOUBLE           = ajn::ALLJOYN_DOUBLE,
    ALLJOYN_DICT_ENTRY       = ajn::ALLJOYN_DICT_ENTRY,
    ALLJOYN_SIGNATURE        = ajn::ALLJOYN_SIGNATURE,
    ALLJOYN_HANDLE           = ajn::ALLJOYN_HANDLE,
    ALLJOYN_INT32            = ajn::ALLJOYN_INT32,
    ALLJOYN_INT16            = ajn::ALLJOYN_INT16,
    ALLJOYN_OBJECT_PATH      = ajn::ALLJOYN_OBJECT_PATH,
    ALLJOYN_UINT16           = ajn::ALLJOYN_UINT16,
    ALLJOYN_STRUCT           = ajn::ALLJOYN_STRUCT,
    ALLJOYN_STRING           = ajn::ALLJOYN_STRING,
    ALLJOYN_UINT64           = ajn::ALLJOYN_UINT64,
    ALLJOYN_UINT32           = ajn::ALLJOYN_UINT32,
    ALLJOYN_VARIANT          = ajn::ALLJOYN_VARIANT,
    ALLJOYN_INT64            = ajn::ALLJOYN_INT64,
    ALLJOYN_BYTE             = ajn::ALLJOYN_BYTE,
    ALLJOYN_STRUCT_OPEN      = ajn::ALLJOYN_STRUCT_OPEN,
    ALLJOYN_STRUCT_CLOSE     = ajn::ALLJOYN_STRUCT_CLOSE,
    ALLJOYN_DICT_ENTRY_OPEN  = ajn::ALLJOYN_DICT_ENTRY_OPEN,
    ALLJOYN_DICT_ENTRY_CLOSE = ajn::ALLJOYN_DICT_ENTRY_CLOSE,
    ALLJOYN_BOOLEAN_ARRAY    = ajn::ALLJOYN_BOOLEAN_ARRAY,
    ALLJOYN_DOUBLE_ARRAY     = ajn::ALLJOYN_DOUBLE_ARRAY,
    ALLJOYN_INT32_ARRAY      = ajn::ALLJOYN_INT32_ARRAY,
    ALLJOYN_INT16_ARRAY      = ajn::ALLJOYN_INT16_ARRAY,
    ALLJOYN_UINT16_ARRAY     = ajn::ALLJOYN_UINT16_ARRAY,
    ALLJOYN_UINT64_ARRAY     = ajn::ALLJOYN_UINT64_ARRAY,
    ALLJOYN_UINT32_ARRAY     = ajn::ALLJOYN_UINT32_ARRAY,
    ALLJOYN_INT64_ARRAY      = ajn::ALLJOYN_INT64_ARRAY,
    ALLJOYN_BYTE_ARRAY       = ajn::ALLJOYN_BYTE_ARRAY,
    ALLJOYN_WILDCARD         = ajn::ALLJOYN_WILDCARD
};

ref class __MsgArg {
  private:
    friend ref class MsgArg;
    friend class _MsgArg;
    __MsgArg();
    ~__MsgArg();

    property Object ^ Value;
    property Object ^ Key;
};

class _MsgArg : protected ajn::MsgArg {
  protected:
    friend class qcc::ManagedObj<_MsgArg>;
    friend ref class MsgArg;
    friend class _BusObject;
    friend ref class BusObject;
    friend ref class ProxyBusObject;
    _MsgArg();
    ~_MsgArg();

    ::QStatus BuildArray(ajn::MsgArg* arry, const qcc::String elemSig, const Platform::Array<Platform::Object ^> ^ args, int32_t& argIndex);
    ::QStatus VBuildArgs(const char*& signature, uint32_t sigLen, ajn::MsgArg* arg,  int32_t maxCompleteTypes, const Platform::Array<Platform::Object ^> ^ args, int32_t& argIndex, int32_t recursionLevel);
    void SetObject(AllJoyn::MsgArg ^ msgArg, bool isKey);

    __MsgArg ^ _eventsAndProperties;
    std::map<void*, void*> _refMap;
    std::list<qcc::String> _strRef;
    std::list<void*> _msgScratch;
};

public ref class MsgArg sealed {
  public:
    MsgArg();
    MsgArg(Platform::String ^ signature, const Platform::Array<Platform::Object ^> ^ args);

    property Object ^ Value
    {
        Platform::Object ^ get();
    }

    property Object ^ Key
    {
        Platform::Object ^ get();
    }

    static void SetTypeCoercionMode(Platform::String ^ mode);

  private:
    friend class _MsgArg;
    friend ref class BusObject;
    friend class _BusObject;
    friend ref class ProxyBusObject;
    friend ref class Message;
    friend ref class MessageHeaderFields;
    MsgArg(void* msgarg, bool isManaged);
    ~MsgArg();

    qcc::ManagedObj<_MsgArg>* _mMsgArg;
    _MsgArg* _msgArg;
};

}
// MsgArg.h
