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
#include <MsgArg.h>

namespace AllJoyn {

ref class MessageHeaderFields;

public enum class AllJoynMessageType {
    MESSAGE_INVALID     = ajn::AllJoynMessageType::MESSAGE_INVALID,
    MESSAGE_METHOD_CALL = ajn::AllJoynMessageType::MESSAGE_METHOD_CALL,
    MESSAGE_METHOD_RET  = ajn::AllJoynMessageType::MESSAGE_METHOD_RET,
    MESSAGE_ERROR       = ajn::AllJoynMessageType::MESSAGE_ERROR,
    MESSAGE_SIGNAL      = ajn::AllJoynMessageType::MESSAGE_SIGNAL,
};

public enum class AllJoynFlagType {
    ALLJOYN_FLAG_NO_REPLY_EXPECTED  = ajn::ALLJOYN_FLAG_NO_REPLY_EXPECTED,
    ALLJOYN_FLAG_AUTO_START         = ajn::ALLJOYN_FLAG_AUTO_START,
    ALLJOYN_FLAG_ALLOW_REMOTE_MSG   = ajn::ALLJOYN_FLAG_ALLOW_REMOTE_MSG,
    ALLJOYN_FLAG_GLOBAL_BROADCAST   = ajn::ALLJOYN_FLAG_GLOBAL_BROADCAST,
    ALLJOYN_FLAG_COMPRESSED         = ajn::ALLJOYN_FLAG_COMPRESSED,
    ALLJOYN_FLAG_ENCRYPTED          = ajn::ALLJOYN_FLAG_ENCRYPTED
};

ref class __Message {
  private:
    friend ref class Message;
    friend class _Message;
    __Message();
    ~__Message();

    property Platform::String ^ AuthMechanism;
    property AllJoynMessageType Type;
    property uint8_t Flags;
    property uint32_t CallSerial;
    property MessageHeaderFields ^ HeaderFields;
    property Platform::String ^ Signature;
    property Platform::String ^ ObjectPath;
    property Platform::String ^ Interface;
    property Platform::String ^ MemberName;
    property uint32_t ReplySerial;
    property Platform::String ^ Sender;
    property Platform::String ^ RcvEndpointName;
    property Platform::String ^ Destination;
    property uint32_t CompressionToken;
    property uint32_t SessionId;
    property Platform::String ^ Description;
    property uint32_t Timestamp;
};

class _Message {
  protected:
    friend class qcc::ManagedObj<_Message>;
    friend ref class Message;
    friend class _AuthListener;
    friend ref class BusObject;
    friend class _BusObject;
    _Message(ajn::Message* msg);
    ~_Message();

    operator ajn::Message * ();
    operator ajn::_Message * ();

    __Message ^ _eventsAndProperties;
    ajn::Message* _mMessage;
    ajn::_Message* _message;
};

public ref class Message sealed {
  public:
    bool IsBroadcastSignal();
    bool IsGlobalBroadcast();
    bool IsExpired(Platform::WriteOnlyArray<uint32_t> ^ tillExpireMS);
    bool IsUnreliable();
    bool IsEncrypted();
    uint32_t GetArgs(Platform::WriteOnlyArray<MsgArg ^> ^ args);
    MsgArg ^ GetArg(uint32_t argN);
    Platform::String ^ ConvertToString();
    Platform::String ^ GetErrorName(Platform::String ^ errorMessage);

    property Platform::String ^ AuthMechanism
    {
        Platform::String ^ get();
    }

    property AllJoynMessageType Type
    {
        AllJoynMessageType get();
    }

    property uint8_t Flags
    {
        uint8_t get();
    }

    property uint32_t CallSerial
    {
        uint32_t get();
    }

    property MessageHeaderFields ^ HeaderFields
    {
        MessageHeaderFields ^ get();
    }

    property Platform::String ^ Signature
    {
        Platform::String ^ get();
    }

    property Platform::String ^ ObjectPath
    {
        Platform::String ^ get();
    }

    property Platform::String ^ Interface
    {
        Platform::String ^ get();
    }

    property Platform::String ^ MemberName
    {
        Platform::String ^ get();
    }

    property uint32_t ReplySerial
    {
        uint32_t get();
    }

    property Platform::String ^ Sender
    {
        Platform::String ^ get();
    }

    property Platform::String ^ RcvEndpointName
    {
        Platform::String ^ get();
    }

    property Platform::String ^ Destination
    {
        Platform::String ^ get();
    }

    property uint32_t CompressionToken
    {
        uint32_t get();
    }

    property uint32_t SessionId
    {
        uint32_t get();
    }

    property Platform::String ^ Description
    {
        Platform::String ^ get();
    }

    property uint32_t Timestamp
    {
        uint32_t get();
    }

  private:
    friend class _AuthListener;
    friend ref class BusObject;
    friend class _BusObject;
    friend class _MessageReceiver;
    Message(void* message, bool isManaged);
    ~Message();

    qcc::ManagedObj<_Message>* _mMessage;
    _Message* _message;
};

}
// Message.h
