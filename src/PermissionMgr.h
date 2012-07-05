/**
 * @file
 * This class is to manage the permission of an endpoint on using transports or invoking method/signal calls on another peer.
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
#ifndef _PERMISSION_MGR_H
#define _PERMISSION_MGR_H

#include "LocalTransport.h"
#include "TransportList.h"
#include <qcc/ThreadPool.h>

namespace ajn {

#define MAX_PERM_CHECKEDCALL_SIZE (512)
/*
 * This class is to impose permission verification on a peer when it tries to invoke a method/signal call.
 * This only applies to the scenario that two peers on the same device connect to the same alljoyn pre-installed daemon.
 */
class PeerPermission {
  public:
    typedef enum  {
        PP_ALLOWED = 1,
        PP_DENIED  = 2,
        PP_PENDING = 3
    } PeerPermStatus;

    /**
     * Called to check permissions for a method/signal call.
     * @param   message        The message containing the call signature
     * @param   permStr        The permissions required to invoke the call
     */
    static PeerPermStatus CanPeerDoCall(Message& message, const qcc::String& permStr);

    /**
     * Inquire daemon for the peer permission and handle the method call accordingly.
     * @param   message        The message containing the method call signature
     * @param   localEp        The local endpoint
     * @param   entry          The method entry in the MethodTable
     * @param   threadpool     The thread pool
     * @param   permStr        The permissions required to invoke the call
     */
    static QStatus PeerAuthAndHandleMethodCall(Message& message, LocalEndpoint* localEp, const MethodTable::Entry* entry, qcc::ThreadPool* threadpool, const qcc::String& permStr);

    /**
     * Inquire daemon for the peer permission and handle the signal call accordingly.
     * @param   message        The message containing the signal call signature
     * @param   localEp        The local endpoint
     * @param   callList       List of recipients that have registered for this signal
     * @param   threadpool     The thread pool
     * @param   permStr        The permissions required to invoke the call
     */
    static QStatus PeerAuthAndHandleSignalCall(Message& message, LocalEndpoint* localEp, std::list<SignalTable::Entry>& callList, qcc::ThreadPool* threadpool, const qcc::String& permStr);
};

class TransportPermission {
  public:
    /**
     * Filter out transports that the endpoint has no permissions to use
     * @param   srcEp         The source endpoint
     * @param   sender        The sender's well-known name string
     * @param   transports    The transport mask
     * @param   callerName    The caller that invokes this method
     */
    static QStatus FilterTransports(BusEndpoint* srcEp, const qcc::String& sender, TransportMask& transports, const char* callerName);

    /**
     * Get transports that the endpoint has no permission to use
     * @param   uid             Uid to check.
     * @param   transList       List of transports available
     * @param   transForbidden  Mask of fobidden transports
     * @param   callerName      The caller that invokes this method
     */
    static void GetForbiddenTransports(uint32_t uid, TransportList& transList, TransportMask& transForbidden, const char* callerName);
};

class PermissionMgr {
  public:
    /**
     * Add an alias ID to a UnixEndpoint User ID
     * @param srcEp     The source endpoint
     * @param origUID   The unique User ID
     * @param aliasUID  The alias User ID
     */
    static uint32_t AddAliasUnixUser(BusEndpoint* srcEp, qcc::String& sender, uint32_t origUID, uint32_t aliasUID);

    /**
     * Cleanup the permission information cache of an enpoint before it exits.
     */
    static QStatus CleanPermissionCache(BusEndpoint& endpoint);
};

} // namespace ajn

#endif //_PERMISSION_MGR_H
