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
    /// <summary>an invalid header field type</summary>
    ALLJOYN_HDR_FIELD_INVALID = ajn::ALLJOYN_HDR_FIELD_INVALID,
    /// <summary>an object path header field type</summary>
    ALLJOYN_HDR_FIELD_PATH = ajn::ALLJOYN_HDR_FIELD_PATH,
    /// <summary>a message interface header field type</summary>
    ALLJOYN_HDR_FIELD_INTERFACE = ajn::ALLJOYN_HDR_FIELD_INTERFACE,
    /// <summary>a member (message/signal) name header field type</summary>
    ALLJOYN_HDR_FIELD_MEMBER = ajn::ALLJOYN_HDR_FIELD_MEMBER,
    /// <summary>an error name header field type</summary>
    ALLJOYN_HDR_FIELD_ERROR_NAME = ajn::ALLJOYN_HDR_FIELD_ERROR_NAME,
    /// <summary>a reply serial number header field type</summary>
    ALLJOYN_HDR_FIELD_REPLY_SERIAL = ajn::ALLJOYN_HDR_FIELD_REPLY_SERIAL,
    /// <summary>message destination header field type</summary>
    ALLJOYN_HDR_FIELD_DESTINATION = ajn::ALLJOYN_HDR_FIELD_DESTINATION,
    /// <summary>senders well-known name header field type</summary>
    ALLJOYN_HDR_FIELD_SENDER = ajn::ALLJOYN_HDR_FIELD_SENDER,
    /// <summary>message signature header field type</summary>
    ALLJOYN_HDR_FIELD_SIGNATURE = ajn::ALLJOYN_HDR_FIELD_SIGNATURE,
    /// <summary>number of file/socket handles that accompany the message</summary>
    ALLJOYN_HDR_FIELD_HANDLES = ajn::ALLJOYN_HDR_FIELD_HANDLES,


    /// <summary>time stamp header field type</summary>
    ALLJOYN_HDR_FIELD_TIMESTAMP = ajn::ALLJOYN_HDR_FIELD_TIMESTAMP,
    /// <summary>messages time-to-live header field type</summary>
    ALLJOYN_HDR_FIELD_TIME_TO_LIVE = ajn::ALLJOYN_HDR_FIELD_TIME_TO_LIVE,
    /// <summary>message compression token header field type</summary>
    ALLJOYN_HDR_FIELD_COMPRESSION_TOKEN = ajn::ALLJOYN_HDR_FIELD_COMPRESSION_TOKEN,
    /// <summary>Session id field type</summary>
    ALLJOYN_HDR_FIELD_SESSION_ID = ajn::ALLJOYN_HDR_FIELD_SESSION_ID,
    /// <summary>unknown header field type also used as maximum number of header field types.</summary>
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
    _MessageHeaderFields(const ajn::HeaderFields* headers);
    ~_MessageHeaderFields();

    __MessageHeaderFields ^ _eventsAndProperties;
};

public ref class MessageHeaderFields sealed {
  public:
    /// <summary>
    ///Returns a string representation of the header fields.
    /// </summary>
    /// <param name=" indent">
    ///Indentation level.
    /// </param>
    /// <returns>
    ///The string representation of the header fields.
    /// </returns>
    Platform::String ^ ConvertToString(uint32_t indent);

    /// <summary>
    ///The header field values.
    /// </summary>
    property Platform::Array<MsgArg ^> ^ Field
    {
        Platform::Array<MsgArg ^> ^ get();
    }

    /// <summary>
    ///Table to identify which header fields can be compressed.
    /// </summary>
    property Platform::Array<bool> ^ Compressible
    {
        Platform::Array<bool> ^ get();
    }

    /// <summary>
    ///Table to map the header field to a AllJoynTypeId
    /// </summary>
    property Platform::Array<AllJoynTypeId> ^ FieldType
    {
        Platform::Array<AllJoynTypeId> ^ get();
    }

  private:
    friend ref class Message;
    MessageHeaderFields(const ajn::HeaderFields * headers);
    MessageHeaderFields(const qcc::ManagedObj<_MessageHeaderFields>* headers);
    ~MessageHeaderFields();

    qcc::ManagedObj<_MessageHeaderFields>* _mMessageHeaderFields;
    _MessageHeaderFields* _messageHeaderFields;
};

}
// MessageHeaderFields.h
