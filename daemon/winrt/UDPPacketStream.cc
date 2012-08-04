/**
 * @file
 * UDPPacketStream is a UDP based implementation of the PacketStream interface.
 */

/******************************************************************************
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
 ******************************************************************************/

#include <qcc/platform.h>
#include <qcc/Util.h>
#include <errno.h>
#include <assert.h>

#include <qcc/Event.h>
#include <qcc/Debug.h>
#include <qcc/StringUtil.h>
#include "UDPPacketStream.h"

#define QCC_MODULE "PACKET"

using namespace std;
using namespace qcc;

namespace ajn {

PacketDest UDPPacketStream::GetPacketDest(const qcc::String& addr, uint16_t port)
{
    PacketDest pd;
    ::memset((uint8_t*)(&pd), 0, sizeof(PacketDest));
    qcc::IPAddress tmpIpAddr(addr);
    tmpIpAddr.RenderIPv4Binary(pd.pair.ip, qcc::IPAddress::IPv4_SIZE);
    pd.pair.family = QCC_AF_INET;
    pd.pair.port = port;
    return pd;
}

UDPPacketStream::UDPPacketStream(const char* ifaceName, uint16_t port) :
    ifaceName(ifaceName),
    port(port),
    sock(-1),
    sourceEvent(&Event::neverSet),
    sinkEvent(&Event::alwaysSet),
    mtu(0)
{
}

UDPPacketStream::~UDPPacketStream()
{
    if (sourceEvent != &Event::neverSet) {
        delete sourceEvent;
        sourceEvent = &Event::neverSet;
    }
    if (sinkEvent != &Event::alwaysSet) {
        delete sinkEvent;
        sinkEvent = &Event::alwaysSet;
    }
    if (sock >= 0) {
        Close(sock);
        sock = -1;
    }
}

QStatus UDPPacketStream::Start()
{
    QStatus status = ER_OK;
    mtu = 1500; // There is no API to get the MTU in WinRT

    /* Bind socket */
    if (status == ER_OK) {
        //  ((sockaddr_in*)&sa)->sin_port = htons(port);
        status = Bind(sock, ipAddr, port);
        if (status == ER_OK) {
            sourceEvent = new qcc::Event(sock, qcc::Event::IO_READ, false);
            sinkEvent = new qcc::Event(sock, qcc::Event::IO_WRITE, false);
        } else {
            QCC_LogError(status, ("UDPPacketStream bind failed"));
        }
    }

    if (status != ER_OK) {
        Close(sock);
        sock = -1;
    }
    return status;
}

String UDPPacketStream::GetIPAddr() const
{
    // return inet_ntoa(((struct sockaddr_in*)&sa)->sin_addr);
    return ipAddr.ToString();
}

QStatus UDPPacketStream::Stop()
{
    return ER_OK;
}

QStatus UDPPacketStream::PushPacketBytes(const void* buf, size_t numBytes, PacketDest& dest)
{
#if 0
    if (rand() < (RAND_MAX / 100)) {
        printf("Skipping packet with seqNum=0x%x\n", letoh16(*reinterpret_cast<const uint16_t*>((const char*)buf + 4)));
        return ER_OK;
    }
#endif

    assert(numBytes <= mtu);
    size_t sendBytes = numBytes;
    size_t sent = 0;
    qcc::IPAddress ipAddr(dest.pair.ip, IPAddress::IPv4_SIZE);
    QStatus status = qcc::SendTo(sock, ipAddr, dest.pair.port, buf, sendBytes, sent);
    status = (sent == numBytes) ? ER_OK : ER_OS_ERROR;
    if (status != ER_OK) {
        if (sent == (size_t) -1) {
            QCC_LogError(status, ("sendto failed: %s (%d)", ::strerror(errno), errno));
        } else {
            QCC_LogError(status, ("Short udp send: exp=%d, act=%d", numBytes, sent));
        }
    }
    return status;
}

QStatus UDPPacketStream::PullPacketBytes(void* buf, size_t reqBytes, size_t& actualBytes,
                                         PacketDest& sender, uint32_t timeout)
{
    QStatus status = ER_OK;
    assert(reqBytes >= mtu);
    size_t recvBytes = reqBytes;
    qcc::IPAddress ipAddr(sender.pair.ip, IPAddress::IPv4_SIZE);
    status =  qcc::RecvFrom(sock, ipAddr, sender.pair.port, buf, recvBytes, actualBytes);

    if (status != ER_OK) {
        QCC_LogError(status, ("recvfrom failed: %s", ::strerror(errno)));
    }
    return status;
}

String UDPPacketStream::ToString(const PacketDest& dest) const
{
    IPAddress ipAddr(dest.pair.ip, IPAddress::IPv4_SIZE);
    String ret = ipAddr.ToString();
    ret += " (";
    ret += U32ToString(dest.pair.port);
    ret += ")";
    return ret;
}

}
