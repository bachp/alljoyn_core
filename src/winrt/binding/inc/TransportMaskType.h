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

namespace AllJoyn {

/// <summary>Bitmask of all transport types</summary>
[Platform::Metadata::Flags]
public enum class TransportMaskType : uint32_t {
    /// <summary>no transports</summary>
    TRANSPORT_NONE      = ajn::TRANSPORT_NONE,
    /// <summary>ANY transport</summary>
    TRANSPORT_ANY       = ajn::TRANSPORT_ANY,
    /// <summary>Local (same device) transport</summary>
    TRANSPORT_LOCAL     = ajn::TRANSPORT_LOCAL,
    /// <summary>Bluetooth transport</summary>
    TRANSPORT_BLUETOOTH = ajn::TRANSPORT_BLUETOOTH,
    /// <summary>Wireless local-area network transport</summary>
    TRANSPORT_WLAN      = ajn::TRANSPORT_WLAN,
    /// <summary>Wireless wide-area network transport</summary>
    TRANSPORT_WWAN      = ajn::TRANSPORT_WWAN,
    /// <summary>Wired local-area network transport</summary>
    TRANSPORT_LAN       = ajn::TRANSPORT_LAN,
    /// <summary>Transport using ICE protocol</summary>
    TRANSPORT_ICE       = ajn::TRANSPORT_ICE
};

}
// TransportMaskType.h
