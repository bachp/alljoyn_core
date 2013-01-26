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
#include "PermissionDB.h"
#include <qcc/String.h>
#include <qcc/Mutex.h>
#include <qcc/String.h>
#include <alljoyn/DBusStd.h>
#include <alljoyn/AllJoynStd.h>
#include <map>

using namespace qcc;

#define QCC_MODULE "PERMISSION_MGR"

namespace ajn {

QStatus TransportPermission::FilterTransports(BusEndpoint& srcEp, const qcc::String& sender, TransportMask& transports, const char* callerName)
{
    QCC_DbgPrintf(("TransportPermission::FilterTransports() callerName(%s)", callerName));
    QStatus status = ER_OK;
    if (srcEp->IsValid()) {
        if (transports & TRANSPORT_BLUETOOTH) {
            bool allowed = PermissionDB::GetDB().IsBluetoothAllowed(srcEp->GetUserId());
            if (!allowed) {
                transports ^= TRANSPORT_BLUETOOTH;
                QCC_LogError(ER_ALLJOYN_ACCESS_PERMISSION_WARNING, ("AllJoynObj::%s() WARNING: No permission to use Bluetooth", (callerName == NULL) ? "" : callerName));
            }
        }
        if (transports & TRANSPORT_WLAN) {
            bool allowed = PermissionDB::GetDB().IsWifiAllowed(srcEp->GetUserId());
            if (!allowed) {
                transports ^= TRANSPORT_WLAN;
                QCC_LogError(ER_ALLJOYN_ACCESS_PERMISSION_WARNING, ("AllJoynObj::%s() WARNING: No permission to use Wifi", ((callerName == NULL) ? "" : callerName)));
            }
        }
        if (transports & TRANSPORT_ICE) {
            bool allowed = PermissionDB::GetDB().IsWifiAllowed(srcEp->GetUserId());
            if (!allowed) {
                transports ^= TRANSPORT_ICE;
                QCC_LogError(ER_ALLJOYN_ACCESS_PERMISSION_WARNING, ("AllJoynObj::%s() WARNING: No permission to use Wifi for ICE", ((callerName == NULL) ? "" : callerName)));
            }
        }
        if (transports == 0) {
            status = ER_BUS_NO_TRANSPORTS;
        }
    } else {
        status = ER_BUS_NO_ENDPOINT;
        QCC_LogError(ER_BUS_NO_ENDPOINT, ("AllJoynObj::CheckTransportsPermission No Bus Endpoint found for Sender %s", sender.c_str()));
    }
    return status;
}

uint32_t PermissionMgr::AddAliasUnixUser(BusEndpoint& srcEp, qcc::String& sender, uint32_t origUID, uint32_t aliasUID)
{
    QCC_DbgHLPrintf(("PermissionMgr::AddAliasUnixUser() origUID(%d), aliasUID(%d)", origUID, aliasUID));
    QStatus status = ER_OK;
    uint32_t replyCode = ALLJOYN_ALIASUNIXUSER_REPLY_SUCCESS;
    if (!(srcEp->IsValid())) {
        status = ER_BUS_NO_ENDPOINT;
        QCC_LogError(status, ("AliasUnixUser Failed to find endpoint for sender=%s", sender.c_str()));
        replyCode = ALLJOYN_ALIASUNIXUSER_REPLY_FAILED;
    } else {
        origUID = srcEp->GetUserId();
        if (origUID == (uint32_t)-1 || aliasUID == (uint32_t)-1) {
            QCC_LogError(ER_FAIL, ("AliasUnixUser Invalid user id origUID=%d aliasUID=%d", origUID, aliasUID));
            replyCode = ALLJOYN_ALIASUNIXUSER_REPLY_FAILED;
        }
    }

    if (replyCode == ALLJOYN_ALIASUNIXUSER_REPLY_SUCCESS) {
        if (PermissionDB::GetDB().AddAliasUnixUser(origUID, aliasUID) != ER_OK) {
            replyCode = ALLJOYN_ALIASUNIXUSER_REPLY_FAILED;
        }
    }
    return replyCode;
}

QStatus PermissionMgr::CleanPermissionCache(BusEndpoint& endpoint)
{
    QCC_DbgHLPrintf(("PermissionMgr::CleanPermissionCache()"));
    return PermissionDB::GetDB().RemovePermissionCache(endpoint);
}

} // namespace ajn {
