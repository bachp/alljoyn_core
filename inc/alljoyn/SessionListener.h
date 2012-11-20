/**
 * @file
 * SessionListener is an abstract base class (interface) implemented by users of the
 * AllJoyn API in order to receive sessions related event information.
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
#ifndef _ALLJOYN_SESSIONLISTENER_H
#define _ALLJOYN_SESSIONLISTENER_H

#ifndef __cplusplus
#error Only include SessionListener.h in C++ code.
#endif

#include <alljoyn/Session.h>

namespace ajn {

/**
 * Abstract base class implemented by AllJoyn users and called by AllJoyn to inform
 * users of session related events.
 */
class SessionListener {
  public:
    /**
     * Virtual destructor for derivable class.
     */
    virtual ~SessionListener() { }

    /**
     * Called by the bus when an existing session becomes disconnected.
     *
     * See also these sample file(s): @n
     * FileTransfer/FileTransferService.cc @n
     * simple/android/client/jni/Client_jni.cpp @n
     * simple/android/service/jni/Service_jni.cpp @n
     *
     * For Windows 8 see also these sample file(s): @n
     * cpp/Basic/Basic_Client/BasicClient/AllJoynObjects.cpp @n
     * cpp/Basic/Basic_Client/BasicClient/AllJoynObjects.h @n
     * cpp/Basic/Basic_Service/BasicService/AllJoynObjects.cpp @n
     * cpp/Basic/Basic_Service/BasicService/AllJoynObjects.h @n
     * cpp/Basic/Name_Change_Client/NameChangeClient/AllJoynObjects.cpp @n
     * cpp/Basic/Name_Change_Client/NameChangeClient/AllJoynObjects.h @n
     * cpp/Basic/Signal_Consumer_Client/SignalConsumerClient/AllJoynObjects.cpp @n
     * cpp/Basic/Signal_Consumer_Client/SignalConsumerClient/AllJoynObjects.h @n
     * cpp/Basic/Signal_Service/SignalService/AllJoynObjects.cpp @n
     * cpp/Basic/Signal_Service/SignalService/AllJoynObjects.h @n
     * cpp/Chat/Chat/AllJoynObjects.cpp @n
     * cpp/Chat/Chat/AllJoynObjects.h @n
     * cpp/Secure/Secure/AllJoynObjects.cpp @n
     * cpp/Secure/Secure/AllJoynObjects.h @n
     * csharp/blank/blank/Common/Listeners.cs @n
     * csharp/BusStress/BusStress/Common/ClientBusListener.cs @n
     * csharp/BusStress/BusStress/Common/ServiceBusListener.cs @n
     * csharp/chat/chat/Common/Listeners.cs @n
     * csharp/chat/chat/MainPage.xaml.cs @n
     * csharp/FileTransfer/Client/Common/Listeners.cs @n
     * csharp/Secure/Secure/Common/Listeners.cs @n
     * csharp/Sessions/Sessions/Common/MyBusListener.cs @n
     * csharp/Sessions/Sessions/Common/SessionOperations.cs @n
     *
     * @param sessionId     Id of session that was lost.
     */
    virtual void SessionLost(SessionId sessionId) { }

    /**
     * Called by the bus when a member of a multipoint session is added.
     *
     * See also these sample file(s): @n
     * FileTransfer/FileTransferService.cc @n
     *
     * For Windows 8 see also these sample file(s): @n
     * cpp/Basic/Basic_Client/BasicClient/AllJoynObjects.cpp @n
     * cpp/Basic/Basic_Client/BasicClient/AllJoynObjects.h @n
     * cpp/Basic/Basic_Service/BasicService/AllJoynObjects.cpp @n
     * cpp/Basic/Basic_Service/BasicService/AllJoynObjects.h @n
     * cpp/Basic/Name_Change_Client/NameChangeClient/AllJoynObjects.cpp @n
     * cpp/Basic/Name_Change_Client/NameChangeClient/AllJoynObjects.h @n
     * cpp/Basic/Signal_Consumer_Client/SignalConsumerClient/AllJoynObjects.cpp @n
     * cpp/Basic/Signal_Consumer_Client/SignalConsumerClient/AllJoynObjects.h @n
     * cpp/Basic/Signal_Service/SignalService/AllJoynObjects.cpp @n
     * cpp/Basic/Signal_Service/SignalService/AllJoynObjects.h @n
     * cpp/Chat/Chat/AllJoynObjects.cpp @n
     * cpp/Chat/Chat/AllJoynObjects.h @n
     * cpp/Secure/Secure/AllJoynObjects.cpp @n
     * cpp/Secure/Secure/AllJoynObjects.h @n
     * csharp/blank/blank/Common/Listeners.cs @n
     * csharp/Secure/Secure/Common/Listeners.cs @n
     * csharp/Sessions/Sessions/Common/MyBusListener.cs @n
     * csharp/Sessions/Sessions/Common/SessionOperations.cs @n
     *
     * @param sessionId     Id of session whose member(s) changed.
     * @param uniqueName    Unique name of member who was added.
     */
    virtual void SessionMemberAdded(SessionId sessionId, const char* uniqueName) { }

    /**
     * Called by the bus when a member of a multipoint session is removed.
     *
     * See also these sample file(s): @n
     * FileTransfer/FileTransferService.cc @n
     *
     * For Windows 8 see also these sample file(s): @n
     * cpp/Basic/Basic_Client/BasicClient/AllJoynObjects.cpp @n
     * cpp/Basic/Basic_Client/BasicClient/AllJoynObjects.h @n
     * cpp/Basic/Basic_Service/BasicService/AllJoynObjects.cpp @n
     * cpp/Basic/Basic_Service/BasicService/AllJoynObjects.h @n
     * cpp/Basic/Name_Change_Client/NameChangeClient/AllJoynObjects.cpp @n
     * cpp/Basic/Name_Change_Client/NameChangeClient/AllJoynObjects.h @n
     * cpp/Basic/Signal_Consumer_Client/SignalConsumerClient/AllJoynObjects.cpp @n
     * cpp/Basic/Signal_Consumer_Client/SignalConsumerClient/AllJoynObjects.h @n
     * cpp/Basic/Signal_Service/SignalService/AllJoynObjects.cpp @n
     * cpp/Basic/Signal_Service/SignalService/AllJoynObjects.h @n
     * cpp/Chat/Chat/AllJoynObjects.cpp @n
     * cpp/Chat/Chat/AllJoynObjects.h @n
     * cpp/Secure/Secure/AllJoynObjects.cpp @n
     * cpp/Secure/Secure/AllJoynObjects.h @n
     * csharp/chat/chat/Common/Listeners.cs @n
     * csharp/FileTransfer/Client/Common/Listeners.cs @n
     * csharp/Secure/Secure/Common/Listeners.cs @n
     * csharp/Sessions/Sessions/Common/MyBusListener.cs @n
     * csharp/Sessions/Sessions/Common/SessionOperations.cs @n
     *
     * @param sessionId     Id of session whose member(s) changed.
     * @param uniqueName    Unique name of member who was removed.
     */
    virtual void SessionMemberRemoved(SessionId sessionId, const char* uniqueName) { }
};

}

#endif
