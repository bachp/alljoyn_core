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

#include <Status_CPP0x.h>
#include <alljoyn/Message.h>
#include <qcc/ManagedObj.h>
#include <Message.h>
#include <MsgArg.h>

namespace AllJoyn {

public enum class AllJoynFieldType {
    ALLJOYN_HDR_FIELD_INVALID = ajn::ALLJOYN_HDR_FIELD_INVALID,
    ALLJOYN_HDR_FIELD_PATH = ajn::ALLJOYN_HDR_FIELD_PATH,
    ALLJOYN_HDR_FIELD_INTERFACE = ajn::ALLJOYN_HDR_FIELD_INTERFACE,
    ALLJOYN_HDR_FIELD_MEMBER = ajn::ALLJOYN_HDR_FIELD_MEMBER,
    ALLJOYN_HDR_FIELD_ERROR_NAME = ajn::ALLJOYN_HDR_FIELD_ERROR_NAME,
    ALLJOYN_HDR_FIELD_REPLY_SERIAL = ajn::ALLJOYN_HDR_FIELD_REPLY_SERIAL,
    ALLJOYN_HDR_FIELD_DESTINATION = ajn::ALLJOYN_HDR_FIELD_DESTINATION,
    ALLJOYN_HDR_FIELD_SENDER = ajn::ALLJOYN_HDR_FIELD_SENDER,
    ALLJOYN_HDR_FIELD_SIGNATURE = ajn::ALLJOYN_HDR_FIELD_SIGNATURE,
    ALLJOYN_HDR_FIELD_HANDLES = ajn::ALLJOYN_HDR_FIELD_HANDLES,
    ALLJOYN_HDR_FIELD_TIMESTAMP = ajn::ALLJOYN_HDR_FIELD_TIMESTAMP,
    ALLJOYN_HDR_FIELD_TIME_TO_LIVE = ajn::ALLJOYN_HDR_FIELD_TIME_TO_LIVE,
    ALLJOYN_HDR_FIELD_COMPRESSION_TOKEN = ajn::ALLJOYN_HDR_FIELD_COMPRESSION_TOKEN,
    ALLJOYN_HDR_FIELD_SESSION_ID = ajn::ALLJOYN_HDR_FIELD_SESSION_ID,
    ALLJOYN_HDR_FIELD_UNKNOWN = ajn::ALLJOYN_HDR_FIELD_UNKNOWN
};

ref class __MessageHeaderFields {
  private:
    friend ref class MessageHeaderFields;
    friend class _MessageHeaderFields;
    __MessageHeaderFields();
    ~__MessageHeaderFields();

    property Platform::Array<MsgArg ^> ^ Field;
    property Platform::Array<bool> ^ Compressible;
    property Platform::Array<AllJoynTypeId> ^ FieldType;
};

class _MessageHeaderFields : protected ajn::HeaderFields {
  protected:
    friend class qcc::ManagedObj<_MessageHeaderFields>;
    friend ref class MessageHeaderFields;
    _MessageHeaderFields(ajn::HeaderFields* headers);
    ~_MessageHeaderFields();

    __MessageHeaderFields ^ _eventsAndProperties;
};

public ref class MessageHeaderFields sealed {
  public:
    Platform::String ^ ConvertToString(uint32_t indent);

    property Platform::Array<MsgArg ^> ^ Field
    {
        Platform::Array<MsgArg ^> ^ get();
    }

    property Platform::Array<bool> ^ Compressible
    {
        Platform::Array<bool> ^ get();
    }

    property Platform::Array<AllJoynTypeId> ^ FieldType
    {
        Platform::Array<AllJoynTypeId> ^ get();
    }

  private:
    friend ref class Message;
    MessageHeaderFields(void* messageheaders, bool isManaged);
    ~MessageHeaderFields();

    qcc::ManagedObj<_MessageHeaderFields>* _mMessageHeaderFields;
    _MessageHeaderFields* _messageHeaderFields;
};

}
// MessageHeaderFields.h
