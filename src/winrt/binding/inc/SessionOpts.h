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

#include <alljoyn/Session.h>
#include <TransportMaskType.h>
#include <qcc/ManagedObj.h>

namespace AllJoyn {

[Platform::Metadata::Flags]
public enum class ProximityType : uint32_t {
    PROXIMITY_ANY      = ajn::SessionOpts::PROXIMITY_ANY,
    PROXIMITY_PHYSICAL = ajn::SessionOpts::PROXIMITY_PHYSICAL,
    PROXIMITY_NETWORK  = ajn::SessionOpts::PROXIMITY_NETWORK
};

public enum class TrafficType {
    /// <summary>Session carries message traffic</summary>
    TRAFFIC_MESSAGES       = ajn::SessionOpts::TRAFFIC_MESSAGES,
    /// <summary>Session carries an unreliable (lossy) byte stream</summary>
    TRAFFIC_RAW_UNRELIABLE = ajn::SessionOpts::TRAFFIC_RAW_UNRELIABLE,
    /// <summary>Session carries a reliable byte stream</summary>
    TRAFFIC_RAW_RELIABLE   = ajn::SessionOpts::TRAFFIC_RAW_RELIABLE
};

public enum class SessionPortType {
    SESSION_PORT_ANY       = ajn::SESSION_PORT_ANY
};

ref class __SessionOpts {
  private:
    friend ref class SessionOpts;
    friend class _SessionOpts;
    __SessionOpts();
    ~__SessionOpts();

    property TrafficType Traffic;
    property bool IsMultipoint;
    property ProximityType Proximity;
    property TransportMaskType TransportMask;
};

class _SessionOpts : protected ajn::SessionOpts {
  protected:
    friend class qcc::ManagedObj<_SessionOpts>;
    friend ref class SessionOpts;
    friend class _SessionPortListener;
    friend ref class BusAttachment;
    _SessionOpts();
    _SessionOpts(ajn::SessionOpts::TrafficType traffic, bool isMultipoint, ajn::SessionOpts::Proximity proximity, ajn::TransportMask transports);
    ~_SessionOpts();

    __SessionOpts ^ _eventsAndProperties;
};

public ref class SessionOpts sealed {
  public:
    SessionOpts();

    /// <summary>
    ///Construct a SessionOpts with specific parameters.
    /// </summary>
    /// <param name="traffic">Type of traffic.</param>
    /// <param name="isMultipoint">true iff session supports multipoint (greater than two endpoints).</param>
    /// <param name="proximity">Proximity constraint bitmask.</param>
    /// <param name="transports">Allowed transport types bitmask.</param>
    SessionOpts(TrafficType traffic, bool isMultipoint, ProximityType proximity, TransportMaskType transports);

    /// <summary>
    ///Traffic accesses the <c>TrafficType</c> value of the <c>SessionOpts</c> object.
    /// </summary>
    property TrafficType Traffic
    {
        TrafficType get();
        void set(TrafficType value);
    }

    /// <summary>
    ///Multi-point session capable.
    ///A session is multi-point if it can be joined multiple times to form a single
    ///session with multi (greater than 2) endpoints. When false, each join attempt
    ///creates a new point-to-point session.
    /// </summary>
    property bool IsMultipoint
    {
        bool get();
        void set(bool value);
    }

    /// <summary>
    ///Proximity accesses the <c>ProximityType</c> value of the <c>SessionOpts</c> object.
    /// </summary>
    property ProximityType Proximity
    {
        ProximityType get();
        void set(ProximityType value);
    }

    /// <summary>
    ///TransportMask accesses the <c>TransportMaskType</c> value of the <c>SessionOpts</c> object.
    /// </summary>
    property TransportMaskType TransportMask
    {
        TransportMaskType get();
        void set(TransportMaskType value);
    }

  private:
    friend ref class BusAttachment;
    friend class _SessionPortListener;
    friend class _BusAttachment;
    SessionOpts(const ajn::SessionOpts * sessionOpts);
    SessionOpts(const qcc::ManagedObj<_SessionOpts>* sessionOpts);
    ~SessionOpts();

    qcc::ManagedObj<_SessionOpts>* _mSessionOpts;
    _SessionOpts* _sessionOpts;
};

}
// SessionOpts.h
