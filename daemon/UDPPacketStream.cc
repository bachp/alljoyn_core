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
#include "NetworkInterface.h"

#define QCC_MODULE "PACKET"

using namespace std;
using namespace qcc;

namespace ajn {

UDPPacketStream::UDPPacketStream(const char* ifaceName, uint16_t port) :
    ipAddr(),
    port(port),
    mtu(0),
    sock(-1),
    sourceEvent(&Event::neverSet),
    sinkEvent(&Event::alwaysSet)
{
    QCC_DbgPrintf(("UDPPacketStream::UDPPacketStream(ifaceName='ifaceName', port=%u)", ifaceName, port));

    NetworkInterface nwInterfaces(true);
    QStatus status = nwInterfaces.UpdateNetworkInterfaces();
    if (status == ER_OK) {
        std::vector<qcc::IfConfigEntry>::iterator it = nwInterfaces.liveInterfaces.begin();
        for (; it != nwInterfaces.liveInterfaces.end(); ++it) {
            if (it->m_name == ifaceName) {
                mtu = it->m_mtu;
                ipAddr = qcc::IPAddress(it->m_addr);
            }
        }
    }
}

UDPPacketStream::UDPPacketStream(const qcc::IPAddress& addr, uint16_t port) :
    ipAddr(addr),
    port(port),
    mtu(1472),
    sock(-1),
    sourceEvent(&Event::neverSet),
    sinkEvent(&Event::alwaysSet)
{
    QCC_DbgPrintf(("UDPPacketStream::UDPPacketStream(addr='%s', port=%u)", ipAddr.ToString().c_str(), port));

    NetworkInterface nwInterfaces(true);
    QStatus status = nwInterfaces.UpdateNetworkInterfaces();
    if (status == ER_OK) {
        std::vector<qcc::IfConfigEntry>::iterator it = nwInterfaces.liveInterfaces.begin();
        for (; it != nwInterfaces.liveInterfaces.end(); ++it) {
            if (qcc::IPAddress(it->m_addr) == addr) {
                mtu = it->m_mtu;
            }
        }
    }
}

UDPPacketStream::UDPPacketStream(const qcc::IPAddress& addr, uint16_t port, size_t mtu) :
    ipAddr(addr),
    port(port),
    mtu(mtu),
    sock(-1),
    sourceEvent(&Event::neverSet),
    sinkEvent(&Event::alwaysSet)
{
    QCC_DbgPrintf(("UDPPacketStream::UDPPacketStream(addr='%s', port=%u, mtu=%lu)", ipAddr.ToString().c_str(), port, mtu));
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
    QCC_DbgPrintf(("UDPPacketStream::Start(addr=%s, port=%u)", ipAddr.ToString().c_str(), port));
    /* Create a socket */
    QStatus status = qcc::Socket(ipAddr.GetAddressFamily(), QCC_SOCK_DGRAM, sock);

    /* Bind socket */
    if (status == ER_OK) {
        status = Bind(sock, ipAddr, port);
        if (status == ER_OK) {
            if (port == 0) {
                status = GetLocalAddress(sock, ipAddr, port);
                if (status != ER_OK) {
                    QCC_DbgPrintf(("UDPPacketStream::Start Bind: GetLocalAddress failed"));
                }
            }

            if (status == ER_OK) {
                sourceEvent = new qcc::Event(sock, qcc::Event::IO_READ, false);
                sinkEvent = new qcc::Event(sock, qcc::Event::IO_WRITE, false);
            }
        } else {
            QCC_LogError(status, ("UDPPacketStream bind failed"));
        }
    } else {
        QCC_LogError(status, ("UDPPacketStream socket() failed"));
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
    QCC_DbgPrintf(("UDPPacketStream::Stop()"));
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
    IPAddress ipAddr(dest.ip, dest.addrSize);
    QStatus status = qcc::SendTo(sock, ipAddr, dest.port, buf, sendBytes, sent);
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
    IPAddress tmpIpAddr;
    uint16_t tmpPort = 0;
    status =  qcc::RecvFrom(sock, tmpIpAddr, tmpPort, buf, recvBytes, actualBytes);
    if (ER_OK != status) {
        QCC_LogError(status, ("recvfrom failed: %s", ::strerror(errno)));
    } else {
        tmpIpAddr.RenderIPBinary(sender.ip, IPAddress::IPv6_SIZE);
        sender.addrSize = tmpIpAddr.Size();
        sender.port = tmpPort;
    }
    return status;
}

String UDPPacketStream::ToString(const PacketDest& dest) const
{
    IPAddress ipAddr(dest.ip, dest.addrSize);
    String ret = ipAddr.ToString();
    ret += " (";
    ret += U32ToString(dest.port);
    ret += ")";
    return ret;
}

}
