/**
 * @file
 * LaunchdTransport is an implementation of Transport that listens
 * on a launchd created AF_UNIX socket.
 */

/******************************************************************************
 * Copyright 2009-2012, Qualcomm Innovation Center, Inc.
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

#include "DaemonTransport.h"

#include <errno.h>
#include <launch.h>
#include <qcc/StringUtil.h>

#define QCC_MODULE "ALLJOYN"

using namespace std;
using namespace qcc;

namespace ajn {

QStatus DaemonLaunchdTransport::ListenFd(std::map<qcc::String, qcc::String>& serverArgs, qcc::SocketFd& listenFd)
{
    launch_data_t request, response = NULL;
    launch_data_t sockets, fdArray;
    QStatus status;

    request = launch_data_new_string(LAUNCH_KEY_CHECKIN);
    if (request == NULL) {
        status = ER_OS_ERROR;
        QCC_LogError(status, ("Unable to create checkin request"));
        goto exit;
    }
    response = launch_msg(request);
    if (response == NULL) {
        status = ER_OS_ERROR;
        QCC_LogError(status, ("Checkin request failed"));
        goto exit;
    }
    if (launch_data_get_type(response) == LAUNCH_DATA_ERRNO) {
        errno = launch_data_get_errno(response);
        status = ER_OS_ERROR;
        QCC_LogError(status, ("Checkin request failed %s", strerror(errno)));
        goto exit;
    }

    sockets = launch_data_dict_lookup(response, LAUNCH_JOBKEY_SOCKETS);
    if (sockets == NULL) {
        status = ER_OS_ERROR;
        QCC_LogError(status, ("Lookup sockets failed"));
        goto exit;
    }
    if (launch_data_dict_get_count(sockets) > 1) {
        QCC_DbgHLPrintf(("Ignoring additional sockets in launchd plist"));
    }
    fdArray = launch_data_dict_lookup(sockets, "unix_domain_listener");
    if (fdArray == NULL) {
        status = ER_OS_ERROR;
        QCC_LogError(status, ("No listen sockets found"));
        goto exit;
    }
    if (launch_data_array_get_count(fdArray) != 1) {
        status = ER_FAIL;
        QCC_LogError(status, ("Socket 'unix_domain_listener' must have exactly one FD"));
        goto exit;
    }
    listenFd = launch_data_get_fd(launch_data_array_get_index(fdArray, 0));
    status = ER_OK;

exit:
    if (response) {
        launch_data_free(response);
    }
    return status;
}

const char* DaemonTransport::TransportName = "unix";

/*
 * An endpoint class to handle the details of authenticating a connection in
 * the Unix Domain Sockets way.
 */
class DaemonEndpoint : public RemoteEndpoint {

  public:

    DaemonEndpoint(BusAttachment& bus, bool incoming, const qcc::String connectSpec, SocketFd sock) :
        RemoteEndpoint(bus, incoming, connectSpec, stream, DaemonTransport::TransportName),
        userId(-1),
        groupId(-1),
        stream(sock)
    {
    }

    /**
     * Set the user id of the endpoint.
     *
     * @param   userId      User ID number.
     */
    void SetUserId(uint32_t userId) { this->userId = userId; }

    /**
     * Set the group id of the endpoint.
     *
     * @param   groupId     Group ID number.
     */
    void SetGroupId(uint32_t groupId) { this->groupId = groupId; }

    /**
     * Return the user id of the endpoint.
     *
     * @return  User ID number.
     */
    uint32_t GetUserId() const { return userId; }

    /**
     * Return the group id of the endpoint.
     *
     * @return  Group ID number.
     */
    uint32_t GetGroupId() const { return groupId; }

    /**
     * Return the process id of the endpoint.
     *
     * @return  Process ID number.
     */
    uint32_t GetProcessId() const { return 0; }

    /**
     * Indicates if the endpoint supports reporting UNIX style user, group, and process IDs.
     *
     * @return  'true' if UNIX IDs supported, 'false' if not supported.
     */
    bool SupportsUnixIDs() const { return true; }

  private:
    uint32_t userId;
    uint32_t groupId;
    SocketStream stream;
};

static QStatus GetSocketCreds(SocketFd sockFd, uid_t* uid, gid_t* gid)
{
    QStatus status = ER_OK;
    int ret = getpeereid(sockFd, uid, gid);
    if (ret == -1) {
        status = ER_OS_ERROR;
        qcc::Close(sockFd);
    }
    return status;
}

void* DaemonTransport::Run(void* arg)
{
    SocketFd listenFd = (SocketFd)(ptrdiff_t)arg;
    QStatus status = ER_OK;

    Event listenEvent(listenFd, Event::IO_READ, false);

    while (!IsStopping()) {
        status = Event::Wait(listenEvent);
        if (status != ER_OK) {
            QCC_LogError(status, ("Event::Wait failed"));
            break;
        }
        SocketFd newSock;

        status = Accept(listenFd, newSock);

        uid_t uid;
        gid_t gid;

        if (status == ER_OK) {
            status = GetSocketCreds(newSock, &uid, &gid);
        }

        if (status == ER_OK) {
            qcc::String authName;
            qcc::String redirection;
            DaemonEndpoint* conn;

            conn = new DaemonEndpoint(bus, true, "", newSock);
            conn->SetUserId(uid);
            conn->SetGroupId(gid);

            /* Initialized the features for this endpoint */
            conn->GetFeatures().isBusToBus = false;
            conn->GetFeatures().allowRemote = false;
            conn->GetFeatures().handlePassing = true;

            endpointListLock.Lock(MUTEX_CONTEXT);
            endpointList.push_back(conn);
            endpointListLock.Unlock(MUTEX_CONTEXT);
            status = conn->Establish("EXTERNAL", authName, redirection);
            if (status == ER_OK) {
                conn->SetListener(this);
                status = conn->Start();
            }
            if (status != ER_OK) {
                QCC_LogError(status, ("Error starting RemoteEndpoint"));
                endpointListLock.Lock(MUTEX_CONTEXT);
                list<RemoteEndpoint*>::iterator ei = find(endpointList.begin(), endpointList.end(), conn);
                if (ei != endpointList.end()) {
                    endpointList.erase(ei);
                }
                endpointListLock.Unlock(MUTEX_CONTEXT);
                delete conn;
                conn = NULL;
            }
        } else if (ER_WOULDBLOCK == status || ER_READ_ERROR == status) {
            status = ER_OK;
        }

        if (status != ER_OK) {
            QCC_LogError(status, ("Error accepting new connection. Ignoring..."));
        }

    }

    qcc::Close(listenFd);

    QCC_DbgPrintf(("DaemonTransport::Run is exiting status=%s\n", QCC_StatusText(status)));
    return (void*) status;
}

QStatus DaemonTransport::NormalizeTransportSpec(const char* inSpec, qcc::String& outSpec, map<qcc::String, qcc::String>& argMap) const
{
    QStatus status = ParseArguments("launchd", inSpec, argMap);
    qcc::String env = Trim(argMap["env"]);
    if (ER_OK == status) {
        outSpec = "launchd:";
        if (env.empty()) {
            env = "DBUS_LAUNCHD_SESSION_BUS_SOCKET";
        }
        outSpec.append("env=");
        outSpec.append(env);
        argMap["_spec"] = env;
    }

    return status;
}

static QStatus ListenFd(map<qcc::String, qcc::String>& serverArgs, SocketFd& listenFd)
{
    launch_data_t request, response = NULL;
    launch_data_t sockets, fdArray;
    QStatus status;

    request = launch_data_new_string(LAUNCH_KEY_CHECKIN);
    if (request == NULL) {
        status = ER_OS_ERROR;
        QCC_LogError(status, ("Unable to create checkin request"));
        goto exit;
    }
    response = launch_msg(request);
    if (response == NULL) {
        status = ER_OS_ERROR;
        QCC_LogError(status, ("Checkin request failed"));
        goto exit;
    }
    if (launch_data_get_type(response) == LAUNCH_DATA_ERRNO) {
        errno = launch_data_get_errno(response);
        status = ER_OS_ERROR;
        QCC_LogError(status, ("Checkin request failed %s", strerror(errno)));
        goto exit;
    }

    sockets = launch_data_dict_lookup(response, LAUNCH_JOBKEY_SOCKETS);
    if (sockets == NULL) {
        status = ER_OS_ERROR;
        QCC_LogError(status, ("Lookup sockets failed"));
        goto exit;
    }
    if (launch_data_dict_get_count(sockets) > 1) {
        QCC_DbgHLPrintf(("Ignoring additional sockets in launchd plist"));
    }
    fdArray = launch_data_dict_lookup(sockets, "unix_domain_listener");
    if (fdArray == NULL) {
        status = ER_OS_ERROR;
        QCC_LogError(status, ("No listen sockets found"));
        goto exit;
    }
    if (launch_data_array_get_count(fdArray) != 1) {
        status = ER_FAIL;
        QCC_LogError(status, ("Socket 'unix_domain_listener' must have exactly one FD"));
        goto exit;
    }
    listenFd = launch_data_get_fd(launch_data_array_get_index(fdArray, 0));
    status = ER_OK;

exit:
    if (response) {
        launch_data_free(response);
    }
    return status;
}

QStatus DaemonTransport::StartListen(const char* listenSpec)
{

    if (stopping == true) {
        return ER_BUS_TRANSPORT_NOT_STARTED;
    }
    if (IsRunning()) {
        return ER_BUS_ALREADY_LISTENING;
    }
    /* Normalize the listen spec. */
    QStatus status;
    qcc::String normSpec;
    map<qcc::String, qcc::String> serverArgs;
    status = NormalizeTransportSpec(listenSpec, normSpec, serverArgs);
    if (status != ER_OK) {
        QCC_LogError(status, ("DaemonTransport::StartListen(): Invalid launchd listen spec \"%s\"", listenSpec));
        return status;
    }
    SocketFd listenFd = -1;
    status = ListenFd(serverArgs, listenFd);
    if (status == ER_OK) {
        status = Thread::Start((void*)listenFd);
    }
    if ((listenFd != -1) && (status != ER_OK)) {
        qcc::Close(listenFd);
    }
    return status;
}

QStatus DaemonTransport::StopListen(const char* listenSpec)
{
    return Thread::Stop();
}

} // namespace ajn
