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

#include <qcc/winrt/SocketWrapper.h>

namespace AllJoyn {
/// <summary>
/// Implementation of socket stream for a raw (non-message based) session.
/// </summary>
public ref class SocketStream sealed {
  public:
    /// <summary>
    /// Duplicate the <c>SocketStream</c> object.
    /// </summary>
    /// <param name="dupSocket">The duplicated socket object</param>
    void SocketDup(Platform::WriteOnlyArray<SocketStream ^> ^ dupSocket);

    /// <summary>
    /// Send data to the socket.
    /// </summary>
    /// <param name="buf">The data buffer containing data to be sent</param>
    /// <param name="len">The amount of data to be sent</param>
    /// <param name="sent">The actual amount of data sent</param>
    void Send(const Platform::Array<uint8> ^ buf, int len, Platform::WriteOnlyArray<int> ^ sent);

    /// <summary>
    /// Receive data from the socket.
    /// </summary>
    /// <param name="buf">The data buffer used to store received data</param>
    /// <param name="len">The amount of data requested</param>
    /// <param name="received">The actual amount of data read</param>
    void Recv(Platform::WriteOnlyArray<uint8> ^ buf, int len, Platform::WriteOnlyArray<int> ^ received);

    /// <summary>
    /// Set the operation mode of the socket object.
    /// </summary>
    /// <param name="block">if true then the socket operations will be blocking, otherwise non-blocking</param>
    void SetBlocking(bool block);

    /// <summary>
    /// Check whether the socket has data available to read.
    /// </summary>
    /// <returns>true if there is data ready for read</returns>
    bool CanRead();

    /// <summary>
    /// Check whether the socket is ready for writing data.
    /// </summary>
    /// <returns>true if the socket is data ready for write</returns>
    bool CanWrite();

    friend ref class BusAttachment;
    friend ref class SocketStreamEvent;

  private:

    SocketStream(qcc::winrt::SocketWrapper ^ sockfd);

    SocketStream();
    ~SocketStream();

    qcc::winrt::SocketWrapper ^ _sockfd;
};

}
// SocketStream.h
