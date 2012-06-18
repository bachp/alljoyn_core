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

#include <alljoyn/SessionListener.h>
#include <alljoyn/Session.h>
#include <qcc/ManagedObj.h>

namespace AllJoyn {

ref class BusAttachment;

public delegate void SessionListenerSessionLostHandler(ajn::SessionId sessionId);
public delegate void SessionListenerSessionMemberAddedHandler(ajn::SessionId sessionId, Platform::String ^ uniqueName);
public delegate void SessionListenerSessionMemberRemovedHandler(ajn::SessionId sessionId, Platform::String ^ uniqueName);

ref class __SessionListener {
  private:
    friend ref class SessionListener;
    friend class _SessionListener;
    __SessionListener();
    ~__SessionListener();

    event SessionListenerSessionLostHandler ^ SessionLost;
    event SessionListenerSessionMemberAddedHandler ^ SessionMemberAdded;
    event SessionListenerSessionMemberRemovedHandler ^ SessionMemberRemoved;
    property BusAttachment ^ Bus;
};

class _SessionListener : protected ajn::SessionListener {
  protected:
    friend class qcc::ManagedObj<_SessionListener>;
    friend ref class SessionListener;
    friend ref class BusAttachment;
    _SessionListener(BusAttachment ^ bus);
    ~_SessionListener();

    void DefaultSessionListenerSessionLostHandler(ajn::SessionId sessionId);
    void DefaultSessionListenerSessionMemberAddedHandler(ajn::SessionId sessionId, Platform::String ^ uniqueName);
    void DefaultSessionListenerSessionMemberRemovedHandler(ajn::SessionId sessionId, Platform::String ^ uniqueName);
    void SessionLost(ajn::SessionId sessionId);
    void SessionMemberAdded(ajn::SessionId sessionId,  const char* uniqueName);
    void SessionMemberRemoved(ajn::SessionId sessionId,  const char* uniqueName);

    __SessionListener ^ _eventsAndProperties;
};

public ref class SessionListener sealed {
  public:
    SessionListener(BusAttachment ^ bus);
    ~SessionListener();

    event SessionListenerSessionLostHandler ^ SessionLost
    {
        Windows::Foundation::EventRegistrationToken add(SessionListenerSessionLostHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        void raise(ajn::SessionId sessionId);
    }

    event SessionListenerSessionMemberAddedHandler ^ SessionMemberAdded
    {
        Windows::Foundation::EventRegistrationToken add(SessionListenerSessionMemberAddedHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        void raise(ajn::SessionId sessionId, Platform::String ^ uniqueName);
    }

    event SessionListenerSessionMemberRemovedHandler ^ SessionMemberRemoved
    {
        Windows::Foundation::EventRegistrationToken add(SessionListenerSessionMemberRemovedHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        void raise(ajn::SessionId sessionId, Platform::String ^ uniqueName);
    }

    property BusAttachment ^ Bus
    {
        BusAttachment ^ get();
    }

  private:
    friend ref class BusAttachment;
    SessionListener(void* listener, bool isManaged);

    qcc::ManagedObj<_SessionListener>* _mListener;
    _SessionListener* _listener;
};

}
// SessionListener.h
