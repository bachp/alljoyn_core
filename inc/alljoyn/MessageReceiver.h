#ifndef _ALLJOYN_MESSAGERECEIVER_H
#define _ALLJOYN_MESSAGERECEIVER_H

#include <qcc/platform.h>

#include <alljoyn/Message.h>

/**
 * @file
 * MessageReceiver is a base class implemented by any class
 * which wishes to receive AllJoyn messages
 */

/******************************************************************************
 * Copyright 2010-2011, Qualcomm Innovation Center, Inc.
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
 ******************************************************************************/

namespace ajn {

/**
 * %MessageReceiver is a pure-virtual base class that is implemented by any
 * class that wishes to receive AllJoyn messages from the AllJoyn library.
 *
 * Received messages can be either signals, method_replies or errors.
 */
class MessageReceiver {
  public:
    /** Destructor */
    virtual ~MessageReceiver() { }

    /**
     * MethodHandlers are %MessageReceiver methods which are called by AllJoyn library
     * to forward AllJoyn method_calls to AllJoyn library users.
     *
     * See also these sample file(s):
     * basic\basic_service.cc
     * secure\DeskTopSharedKSService.cc
     * simple\android\service\jni\Service_jni.cpp
     * windows\PhotoChat\AllJoynBusLib\AllJoynConnection.cpp
     * windows\Service\Service.cpp
     *
     * For Windows 8 see also these sample file(s):
     * cpp\AllJoynStreaming\src\MediaSource.cc
     * cpp\Basic\Basic_Service\BasicService\AllJoynObjects.cpp
     * cpp\Secure\Secure\AllJoynObjects.cpp
     * csharp\Basic\Basic_Service\BasicService\Common\BasicServiceBusObject.cs
     * csharp\BusStress\BusStress\Common\ServiceBusObject.cs
     * csharp\Secure\Secure\Common\SecureBusObject.cs
     * javascript\Basic\Basic_Service\BasicService\js\AlljoynObjects.js
     * javascript\Basic\Basic_Service\BasicService\js\script1.js
     * javascript\Secure\Secure\js\Service.js
     *
     * @param member    Method interface member entry.
     * @param message   The received method call message.
     */
    typedef void (MessageReceiver::* MethodHandler)(const InterfaceDescription::Member* member, Message& message);

    /**
     * ReplyHandlers are %MessageReceiver methods which are called by AllJoyn library
     * to forward AllJoyn method_reply and error responses to AllJoyn library users.
     *
     * See also these sample file(s):
     * windows\Service\Service.cpp
     *
     * @param message   The received message.
     * @param context   User-defined context passed to MethodCall and returned upon reply.
     */
    typedef void (MessageReceiver::* ReplyHandler)(Message& message, void* context);

    /**
     * SignalHandlers are %MessageReceiver methods which are called by AllJoyn library
     * to forward AllJoyn received signals to AllJoyn library users.
     *
     * See also these sample file(s):
     * basic\signalConsumer_client.cc
     * chat\android\jni\Chat_jni.cpp
     * chat\linux\chat.cc
     * FileTransfer\FileTransferClient.cc
     * windows\chat\ChatLib32\ChatClasses.cpp
     * windows\chat\ChatLib32\ChatClasses.h
     * windows\PhotoChat\AllJoynBusLib\AllJoynConnection.cpp
     * windows\PhotoChat\AllJoynBusLib\AllJoynConnection.h
     *
     * For Windows 8 see also these sample file(s):
     * cpp\AllJoynStreaming\src\MediaSink.cc
     * cpp\Basic\Signal_Consumer_Client\SignalConsumerClient\AllJoynObjects.cpp
     * cpp\Basic\Signal_Consumer_Client\SignalConsumerClient\AllJoynObjects.h
     * cpp\Chat\Chat\AllJoynObjects.cpp
     * cpp\Chat\Chat\AllJoynObjects.h
     * csharp\Basic\Signal_Consumer_Client\SignalConsumerClient\Common\SignalConsumerBusListener.cs
     * csharp\chat\chat\Common\ChatSessionObject.cs
     * csharp\FileTransfer\Client\Common\FileTransferBusObject.cs
     * csharp\Sessions\Sessions\Common\MyBusObject.cs
     * javascript\Basic\Signal_Consumer_Client\SignalConsumerClient\js\AlljoynObjects.js
     * javascript\chat\chat\js\alljoyn.js
     *
     * @param member    Method or signal interface member entry.
     * @param srcPath   Object path of signal emitter.
     * @param message   The received message.
     */
    typedef void (MessageReceiver::* SignalHandler)(const InterfaceDescription::Member* member, const char* srcPath, Message& message);

};

}

#endif
