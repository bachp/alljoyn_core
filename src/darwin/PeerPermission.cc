/**
 * @file
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
#include "PeerPermission.h"

using namespace qcc;

#define QCC_MODULE "PEER_PERMISSION"

namespace ajn {

PeerPermission::PeerPermStatus PeerPermission::CanPeerDoCall(Message& message, const qcc::String& permStr)
{
    return PP_ALLOWED;
}

QStatus PeerPermission::PeerAuthAndHandleMethodCall(Message& message, LocalEndpoint* localEp, const MethodTable::Entry* entry, ThreadPool* threadPool, const qcc::String& permStr)
{
    return ER_NOT_IMPLEMENTED;
}

QStatus PeerPermission::PeerAuthAndHandleSignalCall(Message& message, LocalEndpoint* localEp, std::list<SignalTable::Entry>& callList, ThreadPool* threadPool, const qcc::String& permStr)
{
    return ER_NOT_IMPLEMENTED;
}

} // namespace ajn {
