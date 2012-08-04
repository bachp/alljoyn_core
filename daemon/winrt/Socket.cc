/**
 * @file
 *
 * Define the abstracted socket interface for WinRT
 */

/******************************************************************************
 * Copyright 2012, Qualcomm Innovation Center, Inc.
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

#include <qcc/platform.h>

#include <qcc/IPAddress.h>
#include <qcc/Socket.h>
#include <qcc/IfConfig.h>
#include <qcc/Util.h>
#include <qcc/Thread.h>
#include <qcc/StringUtil.h>
#include <qcc/windows/utility.h>
#include "ScatterGatherList.h"

#include <Status.h>

#define QCC_MODULE "NETWORK"

namespace qcc {

QStatus SendSG(SocketFd sockfd, const ScatterGatherList& sg, size_t& sent)
{
    QStatus status;
    uint8_t* tmpBuf = new uint8_t[sg.MaxDataSize()];
    sg.CopyToBuffer(tmpBuf, sg.MaxDataSize());
    status = Send(sockfd, tmpBuf, sg.DataSize(), sent);
    delete[] tmpBuf;
    return status;
}

QStatus SendToSG(SocketFd sockfd, IPAddress& remoteAddr, uint16_t remotePort,
                 const ScatterGatherList& sg, size_t& sent)
{
    QStatus status;
    uint8_t* tmpBuf = new uint8_t[sg.MaxDataSize()];
    sg.CopyToBuffer(tmpBuf, sg.MaxDataSize());
    status = SendTo(sockfd, remoteAddr, remotePort, tmpBuf, sg.DataSize(), sent);
    delete[] tmpBuf;
    return status;
}

QStatus RecvSG(SocketFd sockfd, ScatterGatherList& sg, size_t& received)
{
    QStatus status = ER_OK;
    uint8_t* tmpBuf = new uint8_t[sg.MaxDataSize()];
    QCC_DbgTrace(("RecvSG(sockfd = %d, sg = <>, received = <>)", sockfd));

    status = Recv(sockfd, tmpBuf, sg.MaxDataSize(), received);
    if (ER_OK == status) {
        sg.CopyFromBuffer(tmpBuf, received);
    }
    QCC_DbgPrintf(("Received %u bytes", received));
    delete[] tmpBuf;
    return status;
}


QStatus RecvFromSG(SocketFd sockfd, IPAddress& remoteAddr, uint16_t& remotePort,
                   ScatterGatherList& sg, size_t& received)
{
    QStatus status = ER_OK;
    uint8_t* tmpBuf = new uint8_t[sg.MaxDataSize()];
    QCC_DbgTrace(("RecvToSG(sockfd = %d, remoteAddr = %s, remotePort = %u, sg = <>, sent = <>)",
                  sockfd, remoteAddr.ToString().c_str(), remotePort));

    status = RecvFrom(sockfd, remoteAddr, remotePort, tmpBuf, sg.MaxDataSize(), received);
    if (ER_OK == status) {
        sg.CopyFromBuffer(tmpBuf, received);
    }
    QCC_DbgPrintf(("Received %u bytes", received));
    delete[] tmpBuf;
    return status;
}

}