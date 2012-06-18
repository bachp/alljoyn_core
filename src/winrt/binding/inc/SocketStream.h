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

public ref class SocketStream sealed {
  public:

    void SocketDup(Platform::WriteOnlyArray<SocketStream ^> ^ dupSocket);

    void Send(const Platform::Array<uint8> ^ buf, int len, Platform::WriteOnlyArray<int> ^ sent);

    void Recv(Platform::WriteOnlyArray<uint8> ^ buf, int len, Platform::WriteOnlyArray<int> ^ received);

    friend ref class BusAttachment;

  private:

    SocketStream(qcc::winrt::SocketWrapper ^ sockfd);

    SocketStream();
    ~SocketStream();

    qcc::winrt::SocketWrapper ^ _sockfd;
};

}
// SocketStream.h
