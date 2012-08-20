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

#include "Message.h"

#include <MsgArg.h>
#include <MessageHeaderFields.h>
#include <qcc/winrt/utility.h>
#include <ObjectReference.h>
#include <AllJoynException.h>

namespace AllJoyn {

Message::Message(const ajn::Message* message)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == message) {
            status = ER_BAD_ARG_1;
            break;
        }
        _Message* m = new _Message(message);
        if (NULL == m) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mMessage = new qcc::ManagedObj<_Message>(m);
        if (NULL == _mMessage) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _message = &(**_mMessage);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Message::~Message()
{
    if (NULL != _mMessage) {
        delete _mMessage;
        _mMessage = NULL;
        _message = NULL;
    }
}

bool Message::IsBroadcastSignal()
{
    return ((ajn::_Message*)*_message)->IsBroadcastSignal();
}

bool Message::IsGlobalBroadcast()
{
    return ((ajn::_Message*)*_message)->IsGlobalBroadcast();
}

bool Message::IsExpired(Platform::WriteOnlyArray<uint32_t> ^ tillExpireMS)
{
    ::QStatus status = ER_OK;
    bool result = false;

    while (true) {
        if (nullptr == tillExpireMS || tillExpireMS->Length != 1) {
            status = ER_BAD_ARG_1;
            break;
        }
        uint32_t expires = -1;
        result = ((ajn::_Message*)*_message)->IsExpired(&expires);
        tillExpireMS[0] = expires;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

bool Message::IsUnreliable()
{
    return ((ajn::_Message*)*_message)->IsUnreliable();
}

bool Message::IsEncrypted()
{
    return ((ajn::_Message*)*_message)->IsEncrypted();
}

uint32_t Message::GetArgs(Platform::WriteOnlyArray<MsgArg ^> ^ args)
{
    ::QStatus status = ER_OK;
    size_t result = -1;

    while (true) {
        const ajn::MsgArg* margs = NULL;
        ((ajn::_Message*)*_message)->GetArgs(result, margs);
        if (result > 0 && nullptr != args && args->Length > 0) {
            for (int i = 0; i < args->Length; i++) {
                MsgArg ^ tempMsgArg = ref new MsgArg(&(margs[i]));
                if (nullptr == tempMsgArg) {
                    status = ER_OUT_OF_MEMORY;
                    break;
                }
                args[i] = tempMsgArg;
            }
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;

}

MsgArg ^ Message::GetArg(uint32_t argN)
{
    ::QStatus status = ER_OK;
    MsgArg ^ newArg = nullptr;

    while (true) {
        const ajn::MsgArg* arg = ((ajn::_Message*)*_message)->GetArg(argN);
        if (NULL == arg) {
            status = ER_BAD_ARG_1;
            break;
        }
        newArg = ref new MsgArg(arg);
        if (nullptr == newArg) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return newArg;
}

Platform::String ^ Message::ConvertToString()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        qcc::String ret = ((ajn::_Message*)*_message)->ToString();
        result = MultibyteToPlatformString(ret.c_str());
        if (nullptr == result && !ret.empty()) {
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

Platform::String ^ Message::GetErrorName(Platform::String ^ errorMessage)
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == errorMessage) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strErrorMessage = PlatformToMultibyteString(errorMessage);
        if (strErrorMessage.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String ret = ((ajn::_Message*)*_message)->GetErrorName(&strErrorMessage);
        result = MultibyteToPlatformString(ret.c_str());
        if (nullptr == result && !ret.empty()) {
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

Platform::String ^ Message::AuthMechanism::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _message->_eventsAndProperties->AuthMechanism) {
            qcc::String strAuthMechanism = ((ajn::_Message*)*_message)->GetAuthMechanism();
            result = MultibyteToPlatformString(strAuthMechanism.c_str());
            if (nullptr == result && !strAuthMechanism.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _message->_eventsAndProperties->AuthMechanism = result;
        } else {
            result = _message->_eventsAndProperties->AuthMechanism;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

AllJoynMessageType Message::Type::get()
{
    ::QStatus status = ER_OK;
    AllJoynMessageType result = (AllJoynMessageType)(int)-1;

    while (true) {
        if ((AllJoynMessageType)(int)-1 == _message->_eventsAndProperties->Type) {
            result = (AllJoynMessageType)(int)((ajn::_Message*)*_message)->GetType();
            _message->_eventsAndProperties->Type = result;
        } else {
            result = _message->_eventsAndProperties->Type;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

uint8_t Message::Flags::get()
{
    ::QStatus status = ER_OK;
    uint8_t result = (uint8_t)-1;

    while (true) {
        if ((uint8_t)-1 == _message->_eventsAndProperties->Flags) {
            result = ((ajn::_Message*)*_message)->GetFlags();
            _message->_eventsAndProperties->Flags = result;
        } else {
            result = _message->_eventsAndProperties->Flags;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

uint32_t Message::CallSerial::get()
{
    ::QStatus status = ER_OK;
    uint32_t result = (uint32_t)-1;

    while (true) {
        if ((uint32_t)-1 == _message->_eventsAndProperties->CallSerial) {
            result = ((ajn::_Message*)*_message)->GetCallSerial();
            _message->_eventsAndProperties->CallSerial = result;
        } else {
            result = _message->_eventsAndProperties->CallSerial;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

MessageHeaderFields ^ Message::HeaderFields::get()
{
    ::QStatus status = ER_OK;
    MessageHeaderFields ^ result = nullptr;

    while (true) {
        if (nullptr == _message->_eventsAndProperties->HeaderFields) {
            ajn::HeaderFields headers = ((ajn::_Message*)*_message)->GetHeaderFields();
            result = ref new MessageHeaderFields(&headers);
            if (nullptr == result) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _message->_eventsAndProperties->HeaderFields = result;
        } else {
            result = _message->_eventsAndProperties->HeaderFields;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ Message::Signature::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _message->_eventsAndProperties->Signature) {
            qcc::String strSignature = ((ajn::_Message*)*_message)->GetSignature();
            result = MultibyteToPlatformString(strSignature.c_str());
            if (nullptr == result && !strSignature.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _message->_eventsAndProperties->Signature = result;
        } else {
            result = _message->_eventsAndProperties->Signature;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ Message::ObjectPath::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _message->_eventsAndProperties->ObjectPath) {
            qcc::String strObjectPath = ((ajn::_Message*)*_message)->GetObjectPath();
            result = MultibyteToPlatformString(strObjectPath.c_str());
            if (nullptr == result && !strObjectPath.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _message->_eventsAndProperties->ObjectPath = result;
        } else {
            result = _message->_eventsAndProperties->ObjectPath;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ Message::Interface::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _message->_eventsAndProperties->Interface) {
            qcc::String strInterface = ((ajn::_Message*)*_message)->GetInterface();
            result = MultibyteToPlatformString(strInterface.c_str());
            if (nullptr == result && !strInterface.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _message->_eventsAndProperties->Interface = result;
        } else {
            result = _message->_eventsAndProperties->Interface;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ Message::MemberName::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _message->_eventsAndProperties->MemberName) {
            qcc::String strMemberName = ((ajn::_Message*)*_message)->GetMemberName();
            result = MultibyteToPlatformString(strMemberName.c_str());
            if (nullptr == result && !strMemberName.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _message->_eventsAndProperties->MemberName = result;
        } else {
            result = _message->_eventsAndProperties->MemberName;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

uint32_t Message::ReplySerial::get()
{
    ::QStatus status = ER_OK;
    uint32_t result = (uint32_t)-1;

    while (true) {
        if ((uint32_t)-1 == _message->_eventsAndProperties->ReplySerial) {
            result = ((ajn::_Message*)*_message)->GetReplySerial();
            _message->_eventsAndProperties->ReplySerial = result;
        } else {
            result = _message->_eventsAndProperties->ReplySerial;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ Message::Sender::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _message->_eventsAndProperties->Sender) {
            qcc::String strSender = ((ajn::_Message*)*_message)->GetSender();
            result = MultibyteToPlatformString(strSender.c_str());
            if (nullptr == result && !strSender.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _message->_eventsAndProperties->Sender = result;
        } else {
            result = _message->_eventsAndProperties->Sender;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ Message::RcvEndpointName::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _message->_eventsAndProperties->RcvEndpointName) {
            qcc::String strRcvEndpointName = ((ajn::_Message*)*_message)->GetRcvEndpointName();
            result = MultibyteToPlatformString(strRcvEndpointName.c_str());
            if (nullptr == result && !strRcvEndpointName.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _message->_eventsAndProperties->RcvEndpointName = result;
        } else {
            result = _message->_eventsAndProperties->RcvEndpointName;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ Message::Destination::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _message->_eventsAndProperties->Destination) {
            qcc::String strDestination = ((ajn::_Message*)*_message)->GetDestination();
            result = MultibyteToPlatformString(strDestination.c_str());
            if (nullptr == result && !strDestination.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _message->_eventsAndProperties->Destination = result;
        } else {
            result = _message->_eventsAndProperties->Destination;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

uint32_t Message::CompressionToken::get()
{
    ::QStatus status = ER_OK;
    uint32_t result = (uint32_t)-1;

    while (true) {
        if ((uint32_t)-1 == _message->_eventsAndProperties->CompressionToken) {
            result = ((ajn::_Message*)*_message)->GetCompressionToken();
            _message->_eventsAndProperties->CompressionToken = result;
        } else {
            result = _message->_eventsAndProperties->CompressionToken;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

uint32_t Message::SessionId::get()
{
    ::QStatus status = ER_OK;
    uint32_t result = (uint32_t)-1;

    while (true) {
        if ((uint32_t)-1 == _message->_eventsAndProperties->SessionId) {
            result = ((ajn::_Message*)*_message)->GetSessionId();
            _message->_eventsAndProperties->SessionId = result;
        } else {
            result = _message->_eventsAndProperties->SessionId;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ Message::Description::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _message->_eventsAndProperties->Description) {
            qcc::String strDescription = ((ajn::_Message*)*_message)->Description();
            result = MultibyteToPlatformString(strDescription.c_str());
            if (nullptr == result && !strDescription.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _message->_eventsAndProperties->Description = result;
        } else {
            result = _message->_eventsAndProperties->Description;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

uint32_t Message::Timestamp::get()
{
    ::QStatus status = ER_OK;
    uint32_t result = (uint32_t)-1;

    while (true) {
        if ((uint32_t)-1 == _message->_eventsAndProperties->Timestamp) {
            result = ((ajn::_Message*)*_message)->GetTimeStamp();
            _message->_eventsAndProperties->Timestamp = result;
        } else {
            result = _message->_eventsAndProperties->Timestamp;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

_Message::_Message(const ajn::Message* msg)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __Message();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (NULL == msg) {
            status = ER_BAD_ARG_1;
            break;
        }
        _mMessage = new ajn::Message(*msg);
        if (NULL == _mMessage) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _message = &(**_mMessage);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_Message::~_Message()
{
    _eventsAndProperties = nullptr;
    if (NULL != _mMessage) {
        delete _mMessage;
        _mMessage = NULL;
        _message = NULL;
    }
}

_Message::operator ajn::Message * ()
{
    return _mMessage;
}

_Message::operator ajn::_Message * ()
{
    return (ajn::_Message*)_message;
}

__Message::__Message()
{
    AuthMechanism = nullptr;
    Type = (AllJoynMessageType)(int)-1;
    Flags = (uint8_t)-1;
    CallSerial = (uint32_t)-1;
    HeaderFields = nullptr;
    Signature = nullptr;
    ObjectPath = nullptr;
    Interface = nullptr;
    MemberName = nullptr;
    ReplySerial = (uint32_t)-1;
    Sender = nullptr;
    RcvEndpointName = nullptr;
    Destination = nullptr;
    CompressionToken = (uint32_t)-1;
    SessionId = (uint32_t)-1;
    Description = nullptr;
    Timestamp = (uint32_t)-1;
}

__Message::~__Message()
{
    AuthMechanism = nullptr;
    Type = (AllJoynMessageType)(int)-1;
    Flags = (uint8_t)-1;
    CallSerial = (uint32_t)-1;
    HeaderFields = nullptr;
    Signature = nullptr;
    ObjectPath = nullptr;
    Interface = nullptr;
    MemberName = nullptr;
    ReplySerial = (uint32_t)-1;
    Sender = nullptr;
    RcvEndpointName = nullptr;
    Destination = nullptr;
    CompressionToken = (uint32_t)-1;
    SessionId = (uint32_t)-1;
    Description = nullptr;
    Timestamp = (uint32_t)-1;
}

}
