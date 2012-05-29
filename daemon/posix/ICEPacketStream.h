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
#ifndef _ALLJOYN_ICEPACKETSTREAM_H
#define _ALLJOYN_ICEPACKETSTREAM_H

#include <qcc/platform.h>
#include <sys/socket.h>
#include <qcc/String.h>
#include <qcc/IPAddress.h>
#include <Status.h>
#include "Packet.h"
#include "PacketStream.h"
#include "StunMessage.h"
#include "Stun.h"
#include "StunAttribute.h"
#include "ICECandidate.h"
#include "ICESession.h"

namespace ajn {

/**
 * ICEPacketStream is a UDP based implementation of the PacketStream interface.
 */
class ICEPacketStream : public PacketStream {
  public:

    static const uint32_t STUN_OVERHEAD = StunMessage::HEADER_SIZE + StunAttributeXorPeerAddress::ATTR_SIZE_WITH_HEADER +
                                          StunAttribute::ATTR_HEADER_SIZE + StunAttributeMessageIntegrity::ATTR_SIZE_WITH_HEADER +
                                          StunAttributeFingerprint::ATTR_SIZE_WITH_HEADER;

    /** Construct a PacketDest from a addr,port */
    static PacketDest GetPacketDest(const qcc::IPAddress& addr, uint16_t port);

    /** Constructor */
    ICEPacketStream();

    /** Constructor */
    ICEPacketStream(ICESession& iceSession, Stun& stunPtr, _ICECandidate::ICECandidateType candidateType);

    /** Copy constructor */
    ICEPacketStream(const ICEPacketStream& other);

    ICEPacketStream& operator=(const ICEPacketStream& other);

    /** Destructor */
    ~ICEPacketStream();

    /**
     * Start the PacketStream.
     */
    QStatus Start();

    /**
     * Stop the PacketStream.
     */
    QStatus Stop();

    /**
     * Get UDP port.
     */
    uint16_t GetPort() const { return port; }

    /**
     * Get UDP IP addr.
     */
    qcc::String GetIPAddr() const;

    /**
     * Pull bytes from the source.
     * The source is exhausted when ER_NONE is returned.
     *
     * @param buf          Buffer to store pulled bytes
     * @param reqBytes     Number of bytes requested to be pulled from source.
     * @param actualBytes  Actual number of bytes retrieved from source.
     * @param sender       Source type specific representation of the sender of the packet.
     * @param timeout      Time to wait to pull the requested bytes.
     * @return   ER_OK if successful. ER_NONE if source is exhausted. Otherwise an error.
     */
    QStatus PullPacketBytes(void* buf, size_t reqBytes, size_t& actualBytes, PacketDest& sender, uint32_t timeout = qcc::Event::WAIT_FOREVER);

    /**
     * Get the Event indicating that data is available when signaled.
     *
     * @return Event that is signaled when data is available.
     */
    qcc::Event& GetSourceEvent() { return *sourceEvent; }

    /**
     * Get the mtuWithStunOverhead size for this PacketSource.
     *
     * @return MTU of PacketSource
     */
    size_t GetSourceMTU() { return mtuWithStunOverhead; }

    /**
     * Push zero or more bytes into the sink.
     *
     * @param buf          Buffer containing data bytes to be sent
     * @param numBytes     Number of bytes from buf to send to sink. (Must be less that or equal to MTU of PacketSink.)
     * @param dest         Destination for packet bytes.
     * @param controlBytes true iff buf is control data.
     * @return   ER_OK if successful.
     */
    QStatus PushPacketBytes(const void* buf, size_t numBytes, PacketDest& dest, bool controlBytes);

    /**
     * Push zero or more bytes into the sink.
     *
     * @param buf          Buffer containing data bytes to be sent
     * @param numBytes     Number of bytes from buf to send to sink. (Must be less that or equal to MTU of PacketSink.)
     * @param dest         Destination for packet bytes.
     * @return   ER_OK if successful.
     */
    QStatus PushPacketBytes(const void* buf, size_t numBytes, PacketDest& dest) {
        return PushPacketBytes(buf, numBytes, dest, false);
    }

    /**
     * Get the Event that indicates when data can be pushed to sink.
     *
     * @return Event that is signaled when sink can accept more bytes.
     */
    qcc::Event& GetSinkEvent() { return *sinkEvent; }

    /**
     * Get the mtuWithStunOverhead size for this PacketSink.
     *
     * @return MTU of PacketSink
     */
    size_t GetSinkMTU() { return mtuWithStunOverhead; }

    /**
     * Human readable form of UDPPacketDest.
     */
    qcc::String ToString(const PacketDest& dest) const;

    /**
     * Get HMAC key
     *
     * @return hmac key (from ICESession)
     */
    const qcc::String& GetHmacKey() const { return hmacKey; }

    /**
     * Get ICE destination.
     *
     * @return ICE negociated destination
     */
    PacketDest GetICEDestination() const { return GetPacketDest(remoteAddress, remotePort); }

    /**
     * Get ICE destination address.
     *
     * @return ICE negociated IPAddress.
     */
    const IPAddress& GetICERemoteAddr() const { return remoteAddress; }

    /**
     * Get ICE destination port
     *
     * @return ICE negociated IP port.
     */
    uint16_t GetICERemotePort() const { return remotePort; }

    /**
     * Return the ICE candidate type associated with this ICEPacketStream.
     *
     * @return ICECandidateType associated with this ICEPacketStream.
     */
    _ICECandidate::ICECandidateType GetCandidateType() const { return iceCandidateType; }

    /**
     * Return the TURN server's refresh period.
     * This call returns 0 unless the candidate type is Relayed_Candidate.
     *
     * @return TURN server refresh period in milliseconds.
     */
    uint32_t GetTurnRefreshPeriod() const { return turnRefreshPeriod; }

    /**
     * Return the timestamp of the last TURN server's refresh.
     *
     * @return time of last TURN server refresh.
     */
    uint64_t GetTurnRefreshTimestamp() const { return turnRefreshTimestamp; }

    /**
     * Set the timestamp of the last TURN server's refresh.
     *
     * @param time  64-bit timestamp of last TURN refresh.
     */
    void SetTurnRefreshTimestamp(uint64_t time)  { turnRefreshTimestamp = time; }

    /**
     * Return the username used for TURN server authentication.
     *
     * @return TURN server username
     */
    qcc::String GetTurnUsername() const { return turnUsername; }

    /**
     * Return the username used for TURN server authentication.
     *
     * @return TURN server username
     */
    uint32_t GetStunKeepAlivePeriod() const { return stunKeepAlivePeriod; }

    /**
     * Compose a STUN message with the passed in data.
     */
    static QStatus ComposeStunMessage(const void* buf, size_t numBytes, uint8_t* renderBuf, size_t& renderSize,
                                      qcc::IPAddress destnAddress, uint16_t destnPort, String userName,
                                      const qcc::String& key);

    /**
     * Strip STUN overhead from a received message.
     */
    static QStatus StripStunOverhead(uint8_t* recvBuf, size_t rcvdBytes, void* dataBuf, size_t& dataSize,
                                     qcc::IPAddress hostAddress, uint16_t hostPort, size_t keyLen);

  private:
    qcc::IPAddress ipAddress;
    uint16_t port;
    qcc::IPAddress remoteAddress;
    uint16_t remotePort;
    int sock;
    qcc::Event* sourceEvent;
    qcc::Event* sinkEvent;
    size_t mtuWithStunOverhead;
    size_t interfaceMtu;
    _ICECandidate::ICECandidateType iceCandidateType;
    qcc::String hmacKey;
    qcc::String turnUsername;
    uint32_t turnRefreshPeriod;
    uint64_t turnRefreshTimestamp;
    uint32_t stunKeepAlivePeriod;
    Mutex sendLock;
    uint8_t* rxRenderBuf;
    uint8_t* txRenderBuf;
};

}  /* namespace */

#endif

