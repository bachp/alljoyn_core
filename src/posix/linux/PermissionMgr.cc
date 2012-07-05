/**
 * @file
 * This class is to manage the permission of an endpoint on using transports or invoking method/signal calls on another peer
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
#include "PermissionMgr.h"
#include <alljoyn/AllJoynStd.h>

#define QCC_MODULE "PERMISSION_MGR"

namespace ajn {

PeerPermission::PeerPermStatus PeerPermission::CanPeerDoCall(Message& message, const qcc::String& permStr)
{
    return PP_ALLOWED;
}

QStatus PeerPermission::PeerAuthAndHandleMethodCall(Message& message, LocalEndpoint* localEp, const MethodTable::Entry* entry, qcc::ThreadPool* threadPool, const qcc::String& permStr)
{
    return ER_NOT_IMPLEMENTED;
}

QStatus PeerPermission::PeerAuthAndHandleSignalCall(Message& message, LocalEndpoint* localEp, std::list<SignalTable::Entry>& callList, qcc::ThreadPool* threadPool, const qcc::String& permStr)
{
    return ER_NOT_IMPLEMENTED;
}

QStatus TransportPermission::FilterTransports(BusEndpoint* srcEp, const qcc::String& sender, TransportMask& transports, const char* callerName)
{
    return ER_OK;
}

void TransportPermission::GetForbiddenTransports(uint32_t uid, TransportList& transList, TransportMask& transForbidden, const char* callerName)
{
    transForbidden = 0;
}

uint32_t PermissionMgr::AddAliasUnixUser(BusEndpoint* srcEp, qcc::String& sender, uint32_t origUID, uint32_t aliasUID)
{
    return ALLJOYN_ALIASUNIXUSER_REPLY_NO_SUPPORT;
}

QStatus PermissionMgr::CleanPermissionCache(BusEndpoint& endpoint)
{
    return ER_OK;
}

} // namespace ajn {
