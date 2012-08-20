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

#include <alljoyn/SessionPortListener.h>
#include <alljoyn/Session.h>
#include <qcc/String.h>
#include <qcc/winrt/utility.h>
#include <qcc/ManagedObj.h>

namespace AllJoyn {

ref class SessionOpts;
ref class BusAttachment;

/// <summary>
///Accept or reject an incoming JoinSession request. The session does not exist until
///after this function returns.
/// </summary>
/// <remarks>
///This callback is only used by session creators. Therefore it is only called on listeners
///passed to BusAttachment::BindSessionPort.
/// </remarks>
/// <param name="sessionPort">Session port that was joined.</param>
/// <param name="joiner">Unique name of potential joiner.</param>
/// <param name="opts">Session options requested by the joiner.</param>
/// <returns>
///Return true if JoinSession request is accepted. false if rejected.
/// </returns>
public delegate bool SessionPortListenerAcceptSessionJoinerHandler(ajn::SessionPort sessionPort, Platform::String ^ joiner, SessionOpts ^ opts);

/// <summary>
///Called by the bus when a session has been successfully joined. The session is now fully up.
/// </summary>
/// <remarks>
///This callback is only used by session creators. Therefore it is only called on listeners
///passed to BusAttachment::BindSessionPort.
/// </remarks>
/// <param name="sessionPort">Session port that was joined.</param>
/// <param name="id">Id of session.</param>
/// <param name="joiner">Unique name of the joiner.</param>
public delegate void SessionPortListenerSessionJoinedHandler(ajn::SessionPort sessionPort, ajn::SessionId id, Platform::String ^ joiner);

ref class __SessionPortListener {
  private:
    friend ref class SessionPortListener;
    friend class _SessionPortListener;
    __SessionPortListener();
    ~__SessionPortListener();

    event SessionPortListenerAcceptSessionJoinerHandler ^ AcceptSessionJoiner;
    event SessionPortListenerSessionJoinedHandler ^ SessionJoined;
    property BusAttachment ^ Bus;
};

class _SessionPortListener : protected ajn::SessionPortListener {
  protected:
    friend class qcc::ManagedObj<_SessionPortListener>;
    friend ref class SessionPortListener;
    friend ref class BusAttachment;
    _SessionPortListener(BusAttachment ^ bus);
    ~_SessionPortListener();

    bool DefaultSessionPortListenerAcceptSessionJoinerHandler(ajn::SessionPort sessionPort, Platform::String ^ joiner, SessionOpts ^ opts);
    void DefaultSessionPortListenerSessionJoinedHandler(ajn::SessionPort sessionPort, ajn::SessionId id, Platform::String ^ joiner);
    bool AcceptSessionJoiner(ajn::SessionPort sessionPort, const char* joiner, const ajn::SessionOpts& opts);
    void SessionJoined(ajn::SessionPort sessionPort, ajn::SessionId id, const char* joiner);

    __SessionPortListener ^ _eventsAndProperties;
};

public ref class SessionPortListener sealed {
  public:
    SessionPortListener(BusAttachment ^ bus);

    /// <summary>
    ///Called when the a JoinSession request has been made.
    /// </summary>
    event SessionPortListenerAcceptSessionJoinerHandler ^ AcceptSessionJoiner
    {
        Windows::Foundation::EventRegistrationToken add(SessionPortListenerAcceptSessionJoinerHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        bool raise(ajn::SessionPort sessionPort, Platform::String ^ joiner, SessionOpts ^ opts);
    }

    /// <summary>
    ///Called when the session has been successfully joined.
    /// </summary>
    event SessionPortListenerSessionJoinedHandler ^ SessionJoined
    {
        Windows::Foundation::EventRegistrationToken add(SessionPortListenerSessionJoinedHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        void raise(ajn::SessionPort sessionPort, ajn::SessionId id, Platform::String ^ joiner);
    }

    /// <summary>
    ///Retrieve the <c>BusAttachment</c> object related to this <c>SessionPortListener</c>.
    /// </summary>
    property BusAttachment ^ Bus
    {
        BusAttachment ^ get();
    }

  private:
    friend ref class BusAttachment;
    SessionPortListener(const qcc::ManagedObj<_SessionPortListener>* listener);
    ~SessionPortListener();

    qcc::ManagedObj<_SessionPortListener>* _mListener;
    _SessionPortListener* _listener;
};

}
// SessionPortListener.h
