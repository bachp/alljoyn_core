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

#include "SocketStream.h"

namespace AllJoyn {

/// <summary>
///Handle the received data by a SocketStream object
/// </summary>
public delegate void SocketStreamDataReceivedHandler();

/// <summary>
/// <c>SocketStreamEvent</c> is for notifying a <c>SocketStream</c> object has received incoming data.
/// </summary>
public ref class SocketStreamEvent sealed {
  public:
    /// <summary>
    ///Constructor
    /// </summary>
    /// <param name="sockStream">The <c>SocketStream</c> object</param>
    SocketStreamEvent(SocketStream ^ sockStream);
    /// <summary>
    ///Triggered when SocketStream has received data
    /// </summary>
    event SocketStreamDataReceivedHandler ^ DataReceived;

  private:

    ~SocketStreamEvent();
    void DefaultSocketStreamDataReceivedHandler();
    void SocketEventsChangedHandler(Platform::Object ^ source, int events);
};

}
// SocketStream.h
