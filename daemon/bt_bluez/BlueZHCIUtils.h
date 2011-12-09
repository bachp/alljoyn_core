/**
 * @file
 * Utility functions for tweaking Bluetooth behavior via BlueZ.
 */

/******************************************************************************
 * Copyright 2009-2011, Qualcomm Innovation Center, Inc.
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

#ifndef _ALLJOYN_BLUEZHCIUTILS_H
#define _ALLJOYN_BLUEZHCIUTILS_H

#include <qcc/platform.h>

#include <qcc/Socket.h>

#include "BDAddress.h"
#include "BTTransportConsts.h"

#include <Status.h>

namespace ajn {
namespace bluez {

/**
 * Set the L2CAP mtu to something better than the BT 1.0 default value.
 */
void ConfigL2capMTU(qcc::SocketFd sockFd);

void ConfigL2capMaster(qcc::SocketFd sockFd);

/**
 * Find out if a given device is our piconet master
 *
 * @param deviceId    The Bluetooth device id
 * @param addr        The device we are checking
 * @param master      [out] Returns true of the device identified by addr is the master
 */
QStatus IsMaster(uint16_t deviceId, const BDAddress& addr, bool& master);

/**
 * Request as role (master or slave) for the link to a specific device
 *
 * @param deviceId    The Bluetooth device id
 * @param addr        The device link we are setting the role on
 * @param role        The role we are requesting
 */
QStatus RequestBTRole(uint16_t deviceId, const BDAddress& addr, bt::BluetoothRole role);

/**
 * Request to enter sniff mode for the link to a specific device. The time values are all specified
 * in slots (0.625 milliseconds).
 *
 * @param deviceId       The Bluetooth device id
 * @param addr           The device link we are putting into sniff mode
 * @param minInterval    The minimum sniff interval specified as slots
 * @param maxInterval    The maximum sniff interval specified as slots
 * @param attemptTO      The sniff attempt timeou specified as slotst
 * @param sniffTO        The sniff timeou specified as slotst
 */
QStatus RequestEnterSniffMode(uint16_t deviceId,
                              const BDAddress& addr,
                              uint16_t minInterval,
                              uint16_t maxInterval,
                              uint16_t attemptTO,
                              uint16_t sniffTO);

/**
 * Request to exit sniff mode for the link to a specific device
 *
 * @param deviceId    The Bluetooth device id
 * @param addr        The device link we are taking out of sniff mode
 */
QStatus RequestExitSniffMode(uint16_t deviceId, const BDAddress& addr);

/**
 * @param deviceId    The Bluetooth device id
 * @param window      The inquiry window in milliseconds (10 .. 2560)
 * @param interval    The inquiry interval in milliseconds (11 .. 2560)
 * @param interlaced  If true use interlaced inquiry.
 */
QStatus ConfigureInquiryScan(uint16_t deviceId,
                             uint16_t window,
                             uint16_t interval,
                             bool interlaced,
                             int8_t txPower);

/**
 * @param deviceId      The Bluetooth device id
 * @param minPeriod     Value in range 2..0xFFFE expressed as multiple of 1.28 seconds
 * @param maxPeriod     Value in range 3..0xFFFF expressed as multiple of 1.28 seconds
 * @param length        Value in range 1..0x30  (0 will turn off periodic inquiry)
 * @param maxResponses  0 means no limit
 */
QStatus ConfigurePeriodicInquiry(uint16_t deviceId,
                                 uint16_t minPeriod,
                                 uint16_t maxPeriod,
                                 uint8_t length,
                                 uint8_t maxResponses);


QStatus ConfigureSimplePairingDebugMode(uint16_t deviceId, bool enable);

QStatus ConfigureClassOfDevice(uint16_t deviceId, uint32_t cod);

} // namespace bluez
} // namespace ajn

#endif
