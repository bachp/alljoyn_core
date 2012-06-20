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
#include <alljoyn/version.h>
#include <qcc/ScatterGatherList.h>
#include "ICECandidatePair.h"
#include "Stun.h"
#include "ICEPacketStream.h"


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

ICEPacketStream::ICEPacketStream(ICESession& iceSession, Stun& stun, const ICECandidatePair& selectedPair) :
    ipAddress(stun.GetLocalAddr()),
    port(stun.GetLocalPort()),
    remoteAddress(selectedPair.remote->GetEndpoint().addr),
    remotePort(selectedPair.remote->GetEndpoint().port),
    remoteMappedAddress(),
    remoteMappedPort(0),
    turnAddress(stun.GetTurnAddr()),
    turnPort(stun.GetTurnPort()),
    relayServerAddress(iceSession.GetRelayServerAddr()),
    relayServerPort(iceSession.GetRelayServerPort()),
    sock(stun.GetSocketFD()),
    sourceEvent(&Event::neverSet),
    sinkEvent(&Event::alwaysSet),
    mtuWithStunOverhead(stun.GetMtu()),
    interfaceMtu(stun.GetMtu()),
    usingTurn((selectedPair.local->GetType() == _ICECandidate::Relayed_Candidate) || (selectedPair.remote->GetType() == _ICECandidate::Relayed_Candidate)),
    localTurn((selectedPair.local->GetType() == _ICECandidate::Relayed_Candidate)),
    localHost((selectedPair.local->GetType() == _ICECandidate::Host_Candidate)),
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
    mtuWithStunOverhead = interfaceMtu - STUN_OVERHEAD;

    /* Set remoteMappedAddress to the remote's most external address (regardless of candidate type) */
    switch (selectedPair.remote->GetType()) {
    case _ICECandidate::Relayed_Candidate:
    case _ICECandidate::ServerReflexive_Candidate:
    case _ICECandidate::PeerReflexive_Candidate:
        remoteMappedAddress = selectedPair.remote->GetMappedAddress().addr;
        remoteMappedPort = selectedPair.remote->GetMappedAddress().port;
        break;

    default:
        remoteMappedAddress = selectedPair.remote->GetEndpoint().addr;
        remoteMappedPort = selectedPair.remote->GetEndpoint().port;
        break;
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
    remoteMappedAddress(other.remoteMappedAddress),
    remoteMappedPort(other.remoteMappedPort),
    turnAddress(other.turnAddress),
    turnPort(other.turnPort),
    relayServerAddress(other.relayServerAddress),
    relayServerPort(other.relayServerPort),
    sock(other.sock),
    sourceEvent((sock == SOCKET_ERROR) ? &Event::neverSet : new qcc::Event(sock, qcc::Event::IO_READ, false)),
    sinkEvent((sock == SOCKET_ERROR) ? &Event::alwaysSet : new qcc::Event(sock, qcc::Event::IO_WRITE, false)),
    mtuWithStunOverhead(other.mtuWithStunOverhead),
    interfaceMtu(other.interfaceMtu),
    usingTurn(other.usingTurn),
    localTurn(other.localTurn),
    localHost(other.localHost),
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

QStatus ICEPacketStream::PushPacketBytes(const void* buf, size_t numBytes, PacketDest& dest)
{
    QCC_DbgTrace(("ICEPacketStream::PushPacketBytes"));

#ifndef NDEBUG
    size_t messageMtu = usingTurn ? mtuWithStunOverhead : interfaceMtu;
    assert(numBytes <= messageMtu);
#endif

    QStatus status = ER_OK;

    const void* sendBuf = buf;
    size_t sendBytes = numBytes;

    sendLock.Lock();
    size_t sent;

    if (usingTurn) {
        ScatterGatherList sgList;
        status = ComposeStunMessage(buf, numBytes, sgList);
        if (status == ER_OK) {
            status = SendToSG(sock, turnAddress, turnPort, sgList, sent);
        } else {
            QCC_LogError(status, ("ComposeStunMessage failed"));
        }
    } else {
        const struct sockaddr* sa = reinterpret_cast<const struct sockaddr*>(dest.data);
        sent = sendto(sock, sendBuf, sendBytes, 0, sa, sizeof(struct sockaddr_in));
        status = (sent == sendBytes) ? ER_OK : ER_OS_ERROR;
        if (status != ER_OK) {
            if (sent == (size_t) -1) {
                QCC_LogError(status, ("sendto failed: %s (%d)", ::strerror(errno), errno));
            } else {
                QCC_LogError(status, ("Short udp send: exp=%d, act=%d", numBytes, sent));
            }
        }
    }

#if 0
    printf("$$$$$$$$$$$ PushBytes(len=%d)", (int) numBytes);
    for (size_t i = 0; i < numBytes; ++i) {
        if ((i % 16) == 0) {
            printf("\n");
        }
        uint8_t upper = *(((const uint8_t*)buf) + i) >> 4;
        uint8_t lower = *(((const uint8_t*)buf) + i) & 0x0F;
        printf("%c%c ", (upper > 9) ? upper + 'A' - 10 : upper + '0', (lower > 9) ? lower + 'A' - 10 : lower + '0');
    }
    printf("\n");
#endif
    sendLock.Unlock();
    return status;
}

QStatus ICEPacketStream::PullPacketBytes(void* buf, size_t reqBytes, size_t& actualBytes,
                                         PacketDest& sender, uint32_t timeout)
{
    QCC_DbgTrace(("ICEPacketStream::PullPacketBytes"));

    QStatus status = ER_OK;

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

    if (usingTurn) {
        status = StripStunOverhead(actualBytes, buf, reqBytes, actualBytes);
    }
#if 0
    printf("$$$$$$$$$$$ PullBytes(len=%d, buf=%p)\n", (int) actualBytes, buf);
    for (size_t i = 0; i < actualBytes; ++i) {
        if ((i % 16) == 0) {
            printf("\n");
        }
        uint8_t upper = *(((const uint8_t*)buf) + i) >> 4;
        uint8_t lower = *(((const uint8_t*)buf) + i) & 0x0F;
        printf("%c%c ", (upper > 9) ? upper + 'A' - 10 : upper + '0', (lower > 9) ? lower + 'A' - 10 : lower + '0');
    }
    printf("\n");
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

QStatus ICEPacketStream::ComposeStunMessage(const void* buf,
                                            size_t numBytes,
                                            ScatterGatherList& msgSG)
{
    QCC_DbgPrintf(("ICEPacketStream::ComposeStunMessage()"));

    assert(buf != NULL);

    QStatus status = ER_OK;

    ScatterGatherList sg;
    sg.AddBuffer(buf, numBytes);
    sg.SetDataSize(numBytes);

    StunMessage msg(STUN_MSG_INDICATION_CLASS, STUN_MSG_SEND_METHOD, reinterpret_cast<const uint8_t*>(hmacKey.c_str()), hmacKey.size());

    status = msg.AddAttribute(new StunAttributeUsername(turnUsername));
    if (status == ER_OK) {
        status = msg.AddAttribute(new StunAttributeXorPeerAddress(msg, remoteMappedAddress, remoteMappedPort));
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
        size_t renderSize = msg.RenderSize();
        assert(renderSize <= interfaceMtu);
        uint8_t* _txRenderBuf = txRenderBuf;
        status = msg.RenderBinary(_txRenderBuf, renderSize, msgSG);
    }

    return status;
}

QStatus ICEPacketStream::SendNATKeepAlive(void)
{
    QCC_DbgTrace(("ICEPacketStream::SendNATKeepAlive()"));

    QStatus status = ER_OK;

    StunMessage msg(STUN_MSG_INDICATION_CLASS, STUN_MSG_BINDING_METHOD, reinterpret_cast<const uint8_t*>(hmacKey.c_str()), hmacKey.size());

    size_t renderSize = msg.RenderSize();
    assert(renderSize <= interfaceMtu);
    ScatterGatherList msgSG;
    size_t sent;

    sendLock.Lock();
    uint8_t* _txRenderBuf = txRenderBuf;
    status = msg.RenderBinary(_txRenderBuf, renderSize, msgSG);

    if (status == ER_OK) {
        status = SendToSG(sock, remoteAddress, remotePort, msgSG, sent);
        QCC_DbgPrintf(("ICEPacketStream::SendNATKeepAlive()(): Sent NAT keep-alive"));
    } else {
        QCC_LogError(status, ("ICEPacketStream::SendNATKeepAlive()(): Failed to send NAT keep-alive"));
    }

    sendLock.Unlock();

    return status;
}

QStatus ICEPacketStream::SendTURNRefresh(uint64_t time)
{
    QCC_DbgTrace(("ICEPacketStream::SendTURNRefresh()"));

    QStatus status = ER_OK;

    StunMessage msg(STUN_MSG_REQUEST_CLASS, STUN_MSG_REFRESH_METHOD, reinterpret_cast<const uint8_t*>(hmacKey.c_str()), hmacKey.size());

    status = msg.AddAttribute(new StunAttributeUsername(turnUsername));

    if (status == ER_OK) {
        status = msg.AddAttribute(new StunAttributeSoftware(String("AllJoyn ") + String(GetVersion())));
    }
    if (status == ER_OK) {
        status = msg.AddAttribute(new StunAttributeLifetime(ajn::TURN_PERMISSION_REFRESH_PERIOD_SECS));
    }
    if (status == ER_OK) {
        status = msg.AddAttribute(new StunAttributeRequestedTransport(ajn::REQUESTED_TRANSPORT_TYPE_UDP));
    }
    if (status == ER_OK) {
        status = msg.AddAttribute(new StunAttributeMessageIntegrity(msg));
    }
    if (status == ER_OK) {
        status = msg.AddAttribute(new StunAttributeFingerprint(msg));
    }
    if (status == ER_OK) {
        size_t renderSize = msg.RenderSize();
        assert(renderSize <= interfaceMtu);
        ScatterGatherList msgSG;
        size_t sent;

        sendLock.Lock();
        uint8_t* _txRenderBuf = txRenderBuf;
        status = msg.RenderBinary(_txRenderBuf, renderSize, msgSG);

        if (status == ER_OK) {
            status = SendToSG(sock, relayServerAddress, relayServerPort, msgSG, sent);
            QCC_DbgPrintf(("ICEPacketStream::SendTURNRefresh(): Sent TURN refresh"));

            // Set the TURN refresh time-stamp
            turnRefreshTimestamp = time;
        } else {
            QCC_LogError(status, ("ICEPacketStream::SendTURNRefresh(): Failed to send TURN refresh"));
        }

        sendLock.Unlock();
    }

    return status;
}

QStatus ICEPacketStream::StripStunOverhead(size_t rcvdBytes, void* dataBuf, size_t dataBufLen, size_t& actualBytes)
{
    QCC_DbgTrace(("ICEPacketStream::StripStunOverhead()"));

    QStatus status = ER_OK;

    size_t _rcvdBytes = rcvdBytes;
    const uint8_t* _rxRenderBuf = rxRenderBuf;

    if (((_rcvdBytes >= StunMessage::MIN_MSG_SIZE) && StunMessage::IsStunMessage(_rxRenderBuf, _rcvdBytes))) {

        uint16_t rawMsgType = 0;

        _rcvdBytes = rcvdBytes;
        _rxRenderBuf = rxRenderBuf;
        StunIOInterface::ReadNetToHost(_rxRenderBuf, _rcvdBytes, rawMsgType);

        if (StunMessage::ExtractMessageMethod(rawMsgType) == STUN_MSG_DATA_METHOD) {
            // parse message and extract DATA attribute contents.
            size_t keyLen  = hmacKey.size();
            uint8_t dummyHmac[keyLen];
            StunMessage msg("", dummyHmac, keyLen);
            QStatus status;

            _rcvdBytes = rcvdBytes;
            _rxRenderBuf = rxRenderBuf;
            status = msg.Parse(_rxRenderBuf, _rcvdBytes);
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
                        assert(dataBufLen >= sgiter->len);
                        actualBytes = ::min(dataBufLen, sgiter->len);
                        ::memcpy(dataBuf, sgiter->buf, actualBytes);
                    }
                }
            }
        } else {
            // If there is no STUN_MSG_DATA_METHOD in the response, it means that this is a response for either a NAT keep alive request
            // or a TURN refresh request. We dont need to handle it neither do we need to send the data out to the PacketEngine
            actualBytes = 0;
        }

    } else {
        status = ER_FAIL;
        QCC_LogError(status, ("ICEPacketStream::StripStunOverhead(): Received message is not a STUN message"));
    }

    return status;
}

}
