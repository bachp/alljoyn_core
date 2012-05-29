/**
 * @file
 * ICEPacketStream is a UDP based implementation of the PacketStream interface.
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

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>

#include <qcc/Event.h>
#include <qcc/Debug.h>
#include <qcc/ScatterGatherList.h>
#include "Stun.h"
#include "ICECandidate.h"
#include "posix/ICEPacketStream.h"


#define QCC_MODULE "PACKET"

using namespace std;
using namespace qcc;

namespace ajn {

PacketDest ICEPacketStream::GetPacketDest(const IPAddress& addr, uint16_t port)
{
    PacketDest pd;

    struct sockaddr_in* sa = reinterpret_cast<struct sockaddr_in*>(&pd.data);
    ::memset(&pd.data, 0, sizeof(pd.data));
    sa->sin_family = AF_INET;
    addr.RenderIPv4Binary(reinterpret_cast<uint8_t*>(&sa->sin_addr.s_addr), IPAddress::IPv4_SIZE);
    sa->sin_port = htons(port);
    return pd;
}

ICEPacketStream::ICEPacketStream(ICESession& iceSession, Stun& stun, _ICECandidate::ICECandidateType candidateType) :
    ipAddress(stun.GetLocalAddr()),
    port(stun.GetLocalPort()),
    remoteAddress(stun.GetRemoteAddr()),
    remotePort(stun.GetRemotePort()),
    sock(stun.GetSocketFD()),
    sourceEvent(&Event::neverSet),
    sinkEvent(&Event::alwaysSet),
    mtuWithStunOverhead(stun.GetMtu()),
    interfaceMtu(stun.GetMtu()),
    iceCandidateType(candidateType),
    hmacKey(reinterpret_cast<const char*>(stun.GetHMACKey()), stun.GetHMACKeyLength(), stun.GetHMACKeyLength()),
    turnUsername(iceSession.GetusernameForShortTermCredential()),
    turnRefreshPeriod(iceSession.GetTURNRefreshPeriod()),
    turnRefreshTimestamp(0),
    stunKeepAlivePeriod(iceSession.GetSTUNKeepAlivePeriod()),
    rxRenderBuf(new uint8_t[interfaceMtu]),
    txRenderBuf(new uint8_t[interfaceMtu])
{
    QCC_DbgTrace(("ICEPacketStream::ICEPacketStream(sock=%d)", sock));

    /* Adjust the mtuWithStunOverhead size to account for the STUN header which would be added in case of communication
     * through the relay server */
    if (iceCandidateType == _ICECandidate::Relayed_Candidate) {
        mtuWithStunOverhead = interfaceMtu - STUN_OVERHEAD;
    }
}

ICEPacketStream::ICEPacketStream() :
    sock(SOCKET_ERROR),
    rxRenderBuf(NULL),
    txRenderBuf(NULL)
{
}

ICEPacketStream::ICEPacketStream(const ICEPacketStream& other) :
    ipAddress(other.ipAddress),
    port(other.port),
    remoteAddress(other.remoteAddress),
    remotePort(other.remotePort),
    sock(other.sock),
    sourceEvent((sock == SOCKET_ERROR) ? &Event::neverSet : new qcc::Event(sock, qcc::Event::IO_READ, false)),
    sinkEvent((sock == SOCKET_ERROR) ? &Event::alwaysSet : new qcc::Event(sock, qcc::Event::IO_WRITE, false)),
    mtuWithStunOverhead(other.mtuWithStunOverhead),
    interfaceMtu(other.interfaceMtu),
    iceCandidateType(other.iceCandidateType),
    hmacKey(other.hmacKey),
    turnUsername(other.turnUsername),
    turnRefreshPeriod(other.turnRefreshPeriod),
    turnRefreshTimestamp(other.turnRefreshTimestamp),
    stunKeepAlivePeriod(other.stunKeepAlivePeriod),
    rxRenderBuf(new uint8_t[interfaceMtu]),
    txRenderBuf(new uint8_t[interfaceMtu])
{
}

ICEPacketStream::~ICEPacketStream()
{
    if (sourceEvent != &Event::neverSet) {
        delete sourceEvent;
        sourceEvent = &Event::neverSet;
    }
    if (sinkEvent != &Event::alwaysSet) {
        delete sinkEvent;
        sinkEvent = &Event::alwaysSet;
    }
    if (rxRenderBuf) {
        delete[] rxRenderBuf;
    }
    if (txRenderBuf) {
        delete[] txRenderBuf;
    }
    Stop();
}

QStatus ICEPacketStream::Start()
{
    QStatus status = ER_OK;

    sourceEvent = new qcc::Event(sock, qcc::Event::IO_READ, false);
    sinkEvent = new qcc::Event(sock, qcc::Event::IO_WRITE, false);

    return status;
}

String ICEPacketStream::GetIPAddr() const
{
    return ipAddress.ToString();
}

QStatus ICEPacketStream::Stop()
{
    return ER_OK;
}

QStatus ICEPacketStream::PushPacketBytes(const void* buf, size_t numBytes, PacketDest& dest, bool controlBytes)
{
    size_t messageMtu = interfaceMtu;
    bool usingTurn = (iceCandidateType == _ICECandidate::Relayed_Candidate);

    if (usingTurn) {
        messageMtu = mtuWithStunOverhead;
    }

    assert(numBytes <= messageMtu);

    QStatus status = ER_OK;

    const void* sendBuf = buf;
    size_t sendBytes = numBytes;

    sendLock.Lock();
    if ((!controlBytes) && (usingTurn)) {
        status = ComposeStunMessage(buf, numBytes, txRenderBuf, sendBytes, remoteAddress, remotePort, turnUsername, hmacKey);
        sendBuf = (const void*)txRenderBuf;
    }

    if (status == ER_OK) {
        const struct sockaddr* sa = reinterpret_cast<const struct sockaddr*>(dest.data);
        size_t sent = sendto(sock, sendBuf, sendBytes, 0, sa, sizeof(struct sockaddr_in));
        status = (sent == sendBytes) ? ER_OK : ER_OS_ERROR;
        if (status != ER_OK) {
            if (sent == (size_t) -1) {
                QCC_LogError(status, ("sendto failed: %s (%d)", ::strerror(errno), errno));
            } else {
                QCC_LogError(status, ("Short udp send: exp=%d, act=%d", numBytes, sent));
            }
        }
    }
    sendLock.Unlock();

    return status;
}

QStatus ICEPacketStream::PullPacketBytes(void* buf, size_t reqBytes, size_t& actualBytes,
                                         PacketDest& sender, uint32_t timeout)
{
    QStatus status = ER_OK;

    size_t messageMtu = interfaceMtu;
    bool usingTurn = (iceCandidateType == _ICECandidate::Relayed_Candidate);

    if (usingTurn) {
        messageMtu = mtuWithStunOverhead;
    }

    assert(reqBytes <= messageMtu);

    void* recvBuf = buf;
    size_t recvBytes = reqBytes;

    if (usingTurn) {
        recvBuf = (void*)rxRenderBuf;
        recvBytes = interfaceMtu;
    }

    struct sockaddr* sa = reinterpret_cast<struct sockaddr*>(&sender.data);
    socklen_t saLen = sizeof(PacketDest);
    size_t rcv = recvfrom(sock, recvBuf, recvBytes, 0, sa, &saLen);
    if (rcv != (size_t) -1) {
        actualBytes = rcv;
    } else {
        status = ER_OS_ERROR;
        QCC_LogError(status, ("recvfrom failed: %s", ::strerror(errno)));
    }

#if 0
    if (!usingHostCandidate) {
        status = StripStunOverhead(rxRenderBuf, actualBytes, buf, actualBytes, ipAddress, port, hmacKey.size());
    }
#endif

    return status;
}

String ICEPacketStream::ToString(const PacketDest& dest) const
{
    const struct sockaddr_in* sa = reinterpret_cast<const struct sockaddr_in*>(dest.data);
    String ret = inet_ntoa(sa->sin_addr);
    ret += " (";
    ret += U32ToString(ntohs(sa->sin_port));
    ret += ")";
    return ret;
}

QStatus ICEPacketStream::ComposeStunMessage(const void* buf, size_t numBytes, uint8_t* renderBuf, size_t& renderSize,
                                            qcc::IPAddress destnAddress, uint16_t destnPort, String userName,
                                            const String& key)
{
    QCC_DbgPrintf(("ICEPacketStream::ComposeStunMessage()"));

    assert(buf != NULL);

    QStatus status = ER_OK;

    ScatterGatherList sg;
    ScatterGatherList msgSG;

    sg.AddBuffer(buf, numBytes);
    sg.SetDataSize(numBytes);

    StunMessage msg(STUN_MSG_INDICATION_CLASS, STUN_MSG_SEND_METHOD, reinterpret_cast<const uint8_t*>(key.c_str()), key.size());
    uint8_t* pos;

    status = msg.AddAttribute(new StunAttributeUsername(userName));
    if (status == ER_OK) {
        status = msg.AddAttribute(new StunAttributeXorPeerAddress(msg, destnAddress, destnPort));
    }
    if (status == ER_OK) {
        status = msg.AddAttribute(new StunAttributeData(sg));
    }
    if (status == ER_OK) {
        status = msg.AddAttribute(new StunAttributeMessageIntegrity(msg));
    }
    if (status == ER_OK) {
        status = msg.AddAttribute(new StunAttributeFingerprint(msg));
    }
    if (status == ER_OK) {
        renderSize = msg.RenderSize();

        pos = renderBuf;

        status = msg.RenderBinary(pos, renderSize, msgSG);

        if (status == ER_OK) {
            QCC_DbgPrintf(("TX: Sending %u octet app data in a %u octet STUN message.",
                           sg.DataSize(), msgSG.DataSize()));
        }
    }

    return status;
}

QStatus ICEPacketStream::StripStunOverhead(uint8_t* recvBuf, size_t rcvdBytes, void* dataBuf, size_t& dataSize,
                                           qcc::IPAddress hostAddress, uint16_t hostPort, size_t keyLen)
{
    QCC_DbgTrace(("ICEPacketStream::StripStunOverhead()"));

    assert(recvBuf != NULL);
    assert(dataBuf != NULL);

    QStatus status = ER_OK;

    if (((rcvdBytes >= StunMessage::MIN_MSG_SIZE) && StunMessage::IsStunMessage(recvBuf, rcvdBytes))) {

        uint16_t rawMsgType = 0;

        const uint8_t* buf = recvBuf;
        size_t bufSize = rcvdBytes;

        StunIOInterface::ReadNetToHost(buf, bufSize, rawMsgType);

        buf = recvBuf;
        bufSize = rcvdBytes;

        if (StunMessage::ExtractMessageMethod(rawMsgType) == STUN_MSG_DATA_METHOD) {
            // parse message and extract DATA attribute contents.
            uint8_t dummyHmac[keyLen];
            StunMessage msg("", dummyHmac, keyLen);
            QStatus status;

            status = msg.Parse(buf, bufSize);
            if (status == ER_OK) {
                StunMessage::const_iterator iter;

                for (iter = msg.Begin(); iter != msg.End(); ++iter) {
                    if ((*iter)->GetType() == STUN_ATTR_DATA) {
                        StunAttributeData* data = reinterpret_cast<StunAttributeData*>(*iter);
                        ScatterGatherList::const_iterator sgiter = data->GetData().Begin();

                        /*
                         * Because the message was parsed, the data
                         * SG-List in the DATA attribute is guaranteed to
                         * be have only a single element.  Furthermore,
                         * that element will refer to a region of memory
                         * that is fully contained within the space
                         * allocated for the StunBuffer that was allocated
                         * above.  Therefore, we just point the sb.buf to
                         * the data region instead of performing a data
                         * copy that will involve overlapping memory
                         * regions.
                         */
                        dataBuf = reinterpret_cast<uint8_t*>(sgiter->buf);
                        dataSize = sgiter->len;
                    }
                    if ((*iter)->GetType() == STUN_ATTR_XOR_PEER_ADDRESS) {
                        StunAttributeXorPeerAddress* sa = reinterpret_cast<StunAttributeXorPeerAddress*>(*iter);
                        IPAddress addr;
                        uint16_t recvPort;
                        sa->GetAddress(addr, recvPort);

                        if ((addr != hostAddress) || (recvPort != hostPort)) {
                            status = ER_FAIL;
                            QCC_LogError(status, ("ICEPacketStream::StripStunOverhead(): STUN message was not intended for this device"));
                        }
                    }
                }
            }
        } else {
            // If there is no STUN_MSG_DATA_METHOD in the response, it means that this is a response for either a NAT keep alive request
            // or a TURN refresh request. We dont need to handle it neither do we need to send the data out to the PacketEngine
            dataSize = 0;
        }

    } else {
        status = ER_FAIL;
        QCC_LogError(status, ("ICEPacketStream::StripStunOverhead(): Received message is not a STUN message"));
    }

    return status;
}

}
