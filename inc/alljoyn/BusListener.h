/**
 * @file
 * BusListener is an abstract base class (interface) implemented by users of the
 * AllJoyn API in order to asynchronously receive bus  related event information.
 */

/******************************************************************************
 * Copyright 2009-2011, Qualcomm Innovation Center, Inc.
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
#ifndef _ALLJOYN_BUSLISTENER_H
#define _ALLJOYN_BUSLISTENER_H

#ifndef __cplusplus
#error Only include BusListener.h in C++ code.
#endif

#include <alljoyn/TransportMask.h>

namespace ajn {

/**
 * Foward declaration.
 */
class BusAttachment;
class MsgArg;

/**
 * Abstract base class implemented by AllJoyn users and called by AllJoyn to inform
 * users of bus related events.
 */
class BusListener {
  public:
    /**
     * Virtual destructor for derivable class.
     */
    virtual ~BusListener() { }

    /**
     * Called by the bus when the listener is registered. This gives the listener implementation the
     * opportunity to save a reference to the bus.
     *
     * See also these sample file(s):
     * FileTransfer\FileTransferClient.cc
     * FileTransfer\FileTransferService.cc
     *
     * For Windows 8 see also these sample file(s):
     * cpp\AllJoynStreaming\tests\csharp\MediaPlayerApp\MainPage.xaml.cs
     * cpp\AllJoynStreaming\tests\csharp\MediaServerApp\MainPage.xaml.cs
     * cpp\Basic\Basic_Client\BasicClient\AllJoynObjects.cpp
     * cpp\Basic\Basic_Client\BasicClient\AllJoynObjects.h
     * cpp\Basic\Basic_Service\BasicService\AllJoynObjects.cpp
     * cpp\Basic\Basic_Service\BasicService\AllJoynObjects.h
     * cpp\Basic\Name_Change_Client\NameChangeClient\AllJoynObjects.cpp
     * cpp\Basic\Name_Change_Client\NameChangeClient\AllJoynObjects.h
     * cpp\Basic\Signal_Consumer_Client\SignalConsumerClient\AllJoynObjects.cpp
     * cpp\Basic\Signal_Consumer_Client\SignalConsumerClient\AllJoynObjects.h
     * cpp\Basic\Signal_Service\SignalService\AllJoynObjects.cpp
     * cpp\Basic\Signal_Service\SignalService\AllJoynObjects.h
     * cpp\Chat\Chat\AllJoynObjects.cpp
     * cpp\Chat\Chat\AllJoynObjects.h
     * cpp\Secure\Secure\AllJoynObjects.cpp
     * cpp\Secure\Secure\AllJoynObjects.h
     * csharp\Basic\Basic_Client\BasicClient\Common\BasicClientBusListener.cs
     * csharp\Basic\Basic_Client\BasicClient\Common\Class1.cs
     * csharp\Basic\Basic_Service\BasicService\Common\BasicServiceBusListener.cs
     * csharp\Basic\Name_Change_Client\NameChangeClient\Common\NameChangeBusListener.cs
     * csharp\Basic\Signal_Consumer_Client\SignalConsumerClient\Common\SignalConsumerBusListener.cs
     * csharp\Basic\Signal_Service\SignalService\Common\SignalServiceBusListener.cs
     * csharp\blank\blank\Common\Listeners.cs
     * csharp\BusStress\BusStress\Common\ClientBusListener.cs
     * csharp\BusStress\BusStress\Common\ServiceBusListener.cs
     * csharp\chat\chat\Common\Listeners.cs
     * csharp\FileTransfer\Client\Common\Listeners.cs
     * csharp\Secure\Secure\Common\Listeners.cs
     * csharp\Sessions\Sessions\Common\MyBusListener.cs
     *
     * @param bus  The bus the listener is registered with.
     */
    virtual void ListenerRegistered(BusAttachment* bus) { }

    /**
     * Called by the bus when the listener is unregistered.
     *
     * See also these sample file(s):
     * FileTransfer\FileTransferClient.cc
     * FileTransfer\FileTransferService.cc
     *
     * For Windows 8 see also these sample file(s):
     * cpp\AllJoynStreaming\tests\csharp\MediaPlayerApp\MainPage.xaml.cs
     * cpp\AllJoynStreaming\tests\csharp\MediaServerApp\MainPage.xaml.cs
     * cpp\Basic\Basic_Client\BasicClient\AllJoynObjects.cpp
     * cpp\Basic\Basic_Client\BasicClient\AllJoynObjects.h
     * cpp\Basic\Basic_Service\BasicService\AllJoynObjects.cpp
     * cpp\Basic\Basic_Service\BasicService\AllJoynObjects.h
     * cpp\Basic\Name_Change_Client\NameChangeClient\AllJoynObjects.cpp
     * cpp\Basic\Name_Change_Client\NameChangeClient\AllJoynObjects.h
     * cpp\Basic\Signal_Consumer_Client\SignalConsumerClient\AllJoynObjects.cpp
     * cpp\Basic\Signal_Consumer_Client\SignalConsumerClient\AllJoynObjects.h
     * cpp\Basic\Signal_Service\SignalService\AllJoynObjects.cpp
     * cpp\Basic\Signal_Service\SignalService\AllJoynObjects.h
     * cpp\Chat\Chat\AllJoynObjects.cpp
     * cpp\Chat\Chat\AllJoynObjects.h
     * cpp\Secure\Secure\AllJoynObjects.cpp
     * cpp\Secure\Secure\AllJoynObjects.h
     * csharp\Basic\Basic_Client\BasicClient\Common\BasicClientBusListener.cs
     * csharp\Basic\Basic_Client\BasicClient\Common\Class1.cs
     * csharp\Basic\Basic_Service\BasicService\Common\BasicServiceBusListener.cs
     * csharp\Basic\Name_Change_Client\NameChangeClient\Common\NameChangeBusListener.cs
     * csharp\Basic\Signal_Consumer_Client\SignalConsumerClient\Common\SignalConsumerBusListener.cs
     * csharp\Basic\Signal_Service\SignalService\Common\SignalServiceBusListener.cs
     * csharp\blank\blank\Common\Listeners.cs
     * csharp\BusStress\BusStress\Common\ClientBusListener.cs
     * csharp\BusStress\BusStress\Common\ServiceBusListener.cs
     * csharp\chat\chat\Common\Listeners.cs
     * csharp\FileTransfer\Client\Common\Listeners.cs
     * csharp\Secure\Secure\Common\Listeners.cs
     * csharp\Sessions\Sessions\Common\MyBusListener.cs
     */
    virtual void ListenerUnregistered() { }

    /**
     * Called by the bus when an external bus is discovered that is advertising a well-known name
     * that this attachment has registered interest in via a DBus call to org.alljoyn.Bus.FindAdvertisedName
     *
     * See also these sample file(s):
     * basic\basic_client.cc
     * basic\nameChange_client.cc
     * basic\README.windows.txt
     * basic\signalConsumer_client.cc
     * chat\android\jni\Chat_jni.cpp
     * chat\linux\chat.cc
     * FileTransfer\FileTransferClient.cc
     * FileTransfer\FileTransferService.cc
     * secure\DeskTopSharedKSClient.cc
     * simple\android\client\jni\Client_jni.cpp
     * windows\chat\ChatLib32\ChatClasses.cpp
     * windows\chat\ChatLib32\ChatClasses.h
     * windows\Client\Client.cpp
     * windows\PhotoChat\AllJoynBusLib\AllJoynConnection.cpp
     * windows\PhotoChat\AllJoynBusLib\AllJoynConnection.h
     *
     * For Windows 8 see also these sample file(s):
     * cpp\AllJoynStreaming\tests\csharp\MediaPlayerApp\MainPage.xaml.cs
     * cpp\AllJoynStreaming\tests\csharp\MediaServerApp\MainPage.xaml.cs
     * cpp\Basic\Basic_Client\BasicClient\AllJoynObjects.cpp
     * cpp\Basic\Basic_Client\BasicClient\AllJoynObjects.h
     * cpp\Basic\Basic_Service\BasicService\AllJoynObjects.cpp
     * cpp\Basic\Basic_Service\BasicService\AllJoynObjects.h
     * cpp\Basic\Name_Change_Client\NameChangeClient\AllJoynObjects.cpp
     * cpp\Basic\Name_Change_Client\NameChangeClient\AllJoynObjects.h
     * cpp\Basic\Signal_Consumer_Client\SignalConsumerClient\AllJoynObjects.cpp
     * cpp\Basic\Signal_Consumer_Client\SignalConsumerClient\AllJoynObjects.h
     * cpp\Basic\Signal_Service\SignalService\AllJoynObjects.cpp
     * cpp\Basic\Signal_Service\SignalService\AllJoynObjects.h
     * cpp\Chat\Chat\AllJoynObjects.cpp
     * cpp\Chat\Chat\AllJoynObjects.h
     * cpp\Secure\Secure\AllJoynObjects.cpp
     * cpp\Secure\Secure\AllJoynObjects.h
     * csharp\Basic\Basic_Client\BasicClient\Common\BasicClientBusListener.cs
     * csharp\Basic\Basic_Client\BasicClient\Common\Class1.cs
     * csharp\Basic\Basic_Service\BasicService\Common\BasicServiceBusListener.cs
     * csharp\Basic\Name_Change_Client\NameChangeClient\Common\NameChangeBusListener.cs
     * csharp\Basic\Signal_Consumer_Client\SignalConsumerClient\Common\SignalConsumerBusListener.cs
     * csharp\Basic\Signal_Service\SignalService\Common\SignalServiceBusListener.cs
     * csharp\blank\blank\Common\Listeners.cs
     * csharp\blank\blank\MainPage.xaml.cs
     * csharp\BusStress\BusStress\Common\ClientBusListener.cs
     * csharp\BusStress\BusStress\Common\ServiceBusListener.cs
     * csharp\chat\chat\Common\Listeners.cs
     * csharp\FileTransfer\Client\App.xaml.cs
     * csharp\FileTransfer\Client\Common\Listeners.cs
     * csharp\FileTransfer\Client\MainPage.xaml.cs
     * csharp\Secure\Secure\Common\Client.cs
     * csharp\Secure\Secure\Common\Listeners.cs
     * csharp\Sessions\Sessions\Common\MyBusListener.cs
     * csharp\Sessions\Sessions\Common\SessionOperations.cs
     *
     * @param name         A well known name that the remote bus is advertising.
     * @param transport    Transport that received the advertisement.
     * @param namePrefix   The well-known name prefix used in call to FindAdvertisedName that triggered this callback.
     */
    virtual void FoundAdvertisedName(const char* name, TransportMask transport, const char* namePrefix) { }

    /**
     * Called by the bus when an advertisement previously reported through FoundName has become unavailable.
     *
     * See also these sample file(s):
     * chat\linux\chat.cc
     * FileTransfer\FileTransferClient.cc
     * FileTransfer\FileTransferService.cc
     * simple\android\client\jni\Client_jni.cpp
     *
     * For Windows 8 see also these sample file(s):
     * cpp\AllJoynStreaming\tests\csharp\MediaPlayerApp\MainPage.xaml.cs
     * cpp\AllJoynStreaming\tests\csharp\MediaServerApp\MainPage.xaml.cs
     * cpp\Basic\Basic_Client\BasicClient\AllJoynObjects.cpp
     * cpp\Basic\Basic_Client\BasicClient\AllJoynObjects.h
     * cpp\Basic\Basic_Service\BasicService\AllJoynObjects.cpp
     * cpp\Basic\Basic_Service\BasicService\AllJoynObjects.h
     * cpp\Basic\Name_Change_Client\NameChangeClient\AllJoynObjects.cpp
     * cpp\Basic\Name_Change_Client\NameChangeClient\AllJoynObjects.h
     * cpp\Basic\Signal_Consumer_Client\SignalConsumerClient\AllJoynObjects.cpp
     * cpp\Basic\Signal_Consumer_Client\SignalConsumerClient\AllJoynObjects.h
     * cpp\Basic\Signal_Service\SignalService\AllJoynObjects.cpp
     * cpp\Basic\Signal_Service\SignalService\AllJoynObjects.h
     * cpp\Chat\Chat\AllJoynObjects.cpp
     * cpp\Chat\Chat\AllJoynObjects.h
     * cpp\Secure\Secure\AllJoynObjects.cpp
     * cpp\Secure\Secure\AllJoynObjects.h
     * csharp\Basic\Basic_Client\BasicClient\Common\BasicClientBusListener.cs
     * csharp\Basic\Basic_Client\BasicClient\Common\Class1.cs
     * csharp\Basic\Basic_Service\BasicService\Common\BasicServiceBusListener.cs
     * csharp\Basic\Name_Change_Client\NameChangeClient\Common\NameChangeBusListener.cs
     * csharp\Basic\Signal_Consumer_Client\SignalConsumerClient\Common\SignalConsumerBusListener.cs
     * csharp\Basic\Signal_Service\SignalService\Common\SignalServiceBusListener.cs
     * csharp\blank\blank\Common\Listeners.cs
     * csharp\BusStress\BusStress\Common\ClientBusListener.cs
     * csharp\BusStress\BusStress\Common\ServiceBusListener.cs
     * csharp\chat\chat\Common\Listeners.cs
     * csharp\FileTransfer\Client\App.xaml.cs
     * csharp\FileTransfer\Client\Common\Listeners.cs
     * csharp\FileTransfer\Client\MainPage.xaml.cs
     * csharp\Secure\Secure\Common\Listeners.cs
     * csharp\Sessions\Sessions\Common\MyBusListener.cs
     * csharp\Sessions\Sessions\Common\SessionOperations.cs
     *
     * @param name         A well known name that the remote bus is advertising that is of interest to this attachment.
     * @param transport    Transport that stopped receiving the given advertised name.
     * @param namePrefix   The well-known name prefix that was used in a call to FindAdvertisedName that triggered this callback.
     */
    virtual void LostAdvertisedName(const char* name, TransportMask transport, const char* namePrefix) { }

    /**
     * Called by the bus when the ownership of any well-known name changes.
     *
     * See also these sample file(s):
     * basic\basic_client.cc
     * basic\basic_service.cc
     * basic\nameChange_client.cc
     * basic\signalConsumer_client.cc
     * basic\signal_service.cc
     * chat\android\jni\Chat_jni.cpp
     * chat\linux\chat.cc
     * FileTransfer\FileTransferClient.cc
     * FileTransfer\FileTransferService.cc
     * secure\DeskTopSharedKSClient.cc
     * secure\DeskTopSharedKSService.cc
     * simple\android\client\jni\Client_jni.cpp
     * simple\android\service\jni\Service_jni.cpp
     * windows\chat\ChatLib32\ChatClasses.cpp
     * windows\chat\ChatLib32\ChatClasses.h
     * windows\Client\Client.cpp
     * windows\PhotoChat\AllJoynBusLib\AllJoynConnection.cpp
     * windows\PhotoChat\AllJoynBusLib\AllJoynConnection.h
     *
     * For Windows 8 see also these sample file(s):
     * cpp\AllJoynStreaming\src\MediaSource.cc
     * cpp\AllJoynStreaming\tests\csharp\MediaPlayerApp\MainPage.xaml.cs
     * cpp\AllJoynStreaming\tests\csharp\MediaServerApp\MainPage.xaml.cs
     * cpp\Basic\Basic_Client\BasicClient\AllJoynObjects.cpp
     * cpp\Basic\Basic_Client\BasicClient\AllJoynObjects.h
     * cpp\Basic\Basic_Service\BasicService\AllJoynObjects.cpp
     * cpp\Basic\Basic_Service\BasicService\AllJoynObjects.h
     * cpp\Basic\Name_Change_Client\NameChangeClient\AllJoynObjects.cpp
     * cpp\Basic\Name_Change_Client\NameChangeClient\AllJoynObjects.h
     * cpp\Basic\Signal_Consumer_Client\SignalConsumerClient\AllJoynObjects.cpp
     * cpp\Basic\Signal_Consumer_Client\SignalConsumerClient\AllJoynObjects.h
     * cpp\Basic\Signal_Service\SignalService\AllJoynObjects.cpp
     * cpp\Basic\Signal_Service\SignalService\AllJoynObjects.h
     * cpp\Chat\Chat\AllJoynObjects.cpp
     * cpp\Chat\Chat\AllJoynObjects.h
     * cpp\Secure\Secure\AllJoynObjects.cpp
     * cpp\Secure\Secure\AllJoynObjects.h
     * csharp\Basic\Basic_Client\BasicClient\Common\BasicClientBusListener.cs
     * csharp\Basic\Basic_Client\BasicClient\Common\Class1.cs
     * csharp\Basic\Basic_Service\BasicService\Common\BasicServiceBusListener.cs
     * csharp\Basic\Name_Change_Client\NameChangeClient\Common\NameChangeBusListener.cs
     * csharp\Basic\Signal_Consumer_Client\SignalConsumerClient\Common\SignalConsumerBusListener.cs
     * csharp\Basic\Signal_Service\SignalService\Common\SignalServiceBusListener.cs
     * csharp\blank\blank\Common\Listeners.cs
     * csharp\BusStress\BusStress\Common\ClientBusListener.cs
     * csharp\BusStress\BusStress\Common\ServiceBusListener.cs
     * csharp\chat\chat\Common\Listeners.cs
     * csharp\FileTransfer\Client\Common\Listeners.cs
     * csharp\Secure\Secure\Common\Listeners.cs
     * csharp\Secure\Secure\Common\Service.cs
     * csharp\Sessions\Sessions\Common\MyBusListener.cs
     *
     * @param busName        The well-known name that has changed.
     * @param previousOwner  The unique name that previously owned the name or NULL if there was no previous owner.
     * @param newOwner       The unique name that now owns the name or NULL if the there is no new owner.
     */
    virtual void NameOwnerChanged(const char* busName, const char* previousOwner, const char* newOwner) { }

    /**
     * Called by the bus when the value of a property changes if that property has annotation
     *
     * @param propName       The well-known name that has changed.
     * @param propValue      The new value of the property; NULL if not present
     */
    virtual void PropertyChanged(const char* propName, const MsgArg* propValue) { }

    /**
     * Called when a BusAttachment this listener is registered with is stopping.
     *
     * See also these sample file(s):
     * FileTransfer\FileTransferClient.cc
     * FileTransfer\FileTransferService.cc
     *
     * For Windows 8 see also these sample file(s):
     * cpp\AllJoynStreaming\tests\csharp\MediaPlayerApp\MainPage.xaml.cs
     * cpp\AllJoynStreaming\tests\csharp\MediaServerApp\MainPage.xaml.cs
     * cpp\Basic\Basic_Client\BasicClient\AllJoynObjects.cpp
     * cpp\Basic\Basic_Client\BasicClient\AllJoynObjects.h
     * cpp\Basic\Basic_Service\BasicService\AllJoynObjects.cpp
     * cpp\Basic\Basic_Service\BasicService\AllJoynObjects.h
     * cpp\Basic\Name_Change_Client\NameChangeClient\AllJoynObjects.cpp
     * cpp\Basic\Name_Change_Client\NameChangeClient\AllJoynObjects.h
     * cpp\Basic\Signal_Consumer_Client\SignalConsumerClient\AllJoynObjects.cpp
     * cpp\Basic\Signal_Consumer_Client\SignalConsumerClient\AllJoynObjects.h
     * cpp\Basic\Signal_Service\SignalService\AllJoynObjects.cpp
     * cpp\Basic\Signal_Service\SignalService\AllJoynObjects.h
     * cpp\Chat\Chat\AllJoynObjects.cpp
     * cpp\Chat\Chat\AllJoynObjects.h
     * cpp\Secure\Secure\AllJoynObjects.cpp
     * cpp\Secure\Secure\AllJoynObjects.h
     * csharp\Basic\Basic_Client\BasicClient\Common\BasicClientBusListener.cs
     * csharp\Basic\Basic_Client\BasicClient\Common\Class1.cs
     * csharp\Basic\Basic_Service\BasicService\Common\BasicServiceBusListener.cs
     * csharp\Basic\Name_Change_Client\NameChangeClient\Common\NameChangeBusListener.cs
     * csharp\Basic\Signal_Consumer_Client\SignalConsumerClient\Common\SignalConsumerBusListener.cs
     * csharp\Basic\Signal_Service\SignalService\Common\SignalServiceBusListener.cs
     * csharp\blank\blank\Common\Listeners.cs
     * csharp\BusStress\BusStress\Common\ClientBusListener.cs
     * csharp\BusStress\BusStress\Common\ServiceBusListener.cs
     * csharp\chat\chat\Common\Listeners.cs
     * csharp\FileTransfer\Client\Common\Listeners.cs
     * csharp\Secure\Secure\Common\Listeners.cs
     * csharp\Sessions\Sessions\Common\MyBusListener.cs
     */
    virtual void BusStopping() { }

    /**
     * Called when a BusAttachment this listener is registered with has become disconnected from
     * the bus.
     *
     * See also these sample file(s):
     * FileTransfer\FileTransferClient.cc
     * FileTransfer\FileTransferService.cc
     *
     * For Windows 8 see also these sample file(s):
     * cpp\AllJoynStreaming\tests\csharp\MediaPlayerApp\MainPage.xaml.cs
     * cpp\AllJoynStreaming\tests\csharp\MediaServerApp\MainPage.xaml.cs
     * cpp\Basic\Basic_Client\BasicClient\AllJoynObjects.cpp
     * cpp\Basic\Basic_Client\BasicClient\AllJoynObjects.h
     * cpp\Basic\Basic_Service\BasicService\AllJoynObjects.cpp
     * cpp\Basic\Basic_Service\BasicService\AllJoynObjects.h
     * cpp\Basic\Name_Change_Client\NameChangeClient\AllJoynObjects.cpp
     * cpp\Basic\Name_Change_Client\NameChangeClient\AllJoynObjects.h
     * cpp\Basic\Signal_Consumer_Client\SignalConsumerClient\AllJoynObjects.cpp
     * cpp\Basic\Signal_Consumer_Client\SignalConsumerClient\AllJoynObjects.h
     * cpp\Basic\Signal_Service\SignalService\AllJoynObjects.cpp
     * cpp\Basic\Signal_Service\SignalService\AllJoynObjects.h
     * cpp\Chat\Chat\AllJoynObjects.cpp
     * cpp\Chat\Chat\AllJoynObjects.h
     * cpp\Secure\Secure\AllJoynObjects.cpp
     * cpp\Secure\Secure\AllJoynObjects.h
     * csharp\Basic\Basic_Client\BasicClient\Common\BasicClientBusListener.cs
     * csharp\Basic\Basic_Client\BasicClient\Common\Class1.cs
     * csharp\Basic\Basic_Service\BasicService\Common\BasicServiceBusListener.cs
     * csharp\Basic\Name_Change_Client\NameChangeClient\Common\NameChangeBusListener.cs
     * csharp\Basic\Signal_Consumer_Client\SignalConsumerClient\Common\SignalConsumerBusListener.cs
     * csharp\Basic\Signal_Service\SignalService\Common\SignalServiceBusListener.cs
     * csharp\blank\blank\Common\Listeners.cs
     * csharp\BusStress\BusStress\Common\ClientBusListener.cs
     * csharp\BusStress\BusStress\Common\ServiceBusListener.cs
     * csharp\chat\chat\Common\Listeners.cs
     * csharp\FileTransfer\Client\Common\Listeners.cs
     * csharp\Secure\Secure\Common\Listeners.cs
     * csharp\Sessions\Sessions\Common\MyBusListener.cs
     */
    virtual void BusDisconnected() { }

};

}

#endif
