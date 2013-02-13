/**
 * @file
 */

/******************************************************************************
 * Copyright 2009-2013, Qualcomm Innovation Center, Inc.
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

#include <assert.h>

#include <qcc/Debug.h>
#include <qcc/String.h>
#include <qcc/StringUtil.h>
#include <qcc/atomic.h>
#include <qcc/Thread.h>
#include <qcc/SocketStream.h>
#include <qcc/atomic.h>

#include <alljoyn/BusAttachment.h>
#include <alljoyn/AllJoynStd.h>

#include "Router.h"
#include "RemoteEndpoint.h"
#include "LocalTransport.h"
#include "AllJoynPeerObj.h"
#include "BusInternal.h"

#ifndef NDEBUG
#include <qcc/time.h>
#endif

#define QCC_MODULE "ALLJOYN"

using namespace std;
using namespace qcc;

namespace ajn {

#define ENDPOINT_IS_DEAD_ALERTCODE  1

static uint32_t threadCount = 0;


/**
 * Thread used to receive endpoint data.
 */
class _RemoteEndpoint::RxThread : public qcc::Thread {
  public:
    RxThread(BusAttachment& bus,
             _RemoteEndpoint* ep,
             const String name,
             bool validateSender)
        : qcc::Thread(name.c_str()), bus(bus), ep(ep), validateSender(validateSender), hasRxSessionMsg(false) { }

  protected:
    qcc::ThreadReturn STDCALL Run(void* arg);

  private:
    BusAttachment& bus;       /**< Bus associated with transport */
    _RemoteEndpoint* ep;        /**< The owning endpoint */
    bool validateSender;      /**< If true, the sender field on incomming messages will be overwritten with actual endpoint name */
    bool hasRxSessionMsg;   /**< true iff this endpoint has previously processed a non-control message */
};

/**
 * Thread used to send endpoint data.
 */
class _RemoteEndpoint::TxThread : public qcc::Thread {
  public:
    TxThread(BusAttachment& bus,
             _RemoteEndpoint* ep,
             const String name,
             std::deque<Message>& queue,
             std::deque<Thread*>& waitQueue,
             qcc::Mutex& queueLock)
        : qcc::Thread(name.c_str()), bus(bus), ep(ep), queue(queue), waitQueue(waitQueue), queueLock(queueLock) { }

  protected:
    qcc::ThreadReturn STDCALL Run(void* arg);

  private:
    BusAttachment& bus;       /**< Bus associated with transport */
    _RemoteEndpoint* ep;        /**< The owning endpoint */
    std::deque<Message>& queue;
    std::deque<Thread*>& waitQueue;
    qcc::Mutex& queueLock;
};

class _RemoteEndpoint::Internal {
    friend class _RemoteEndpoint;
    friend class RxThread;
    friend class TxThread;
  public:

    Internal(BusAttachment& bus, _RemoteEndpoint* rep, bool incoming, const qcc::String& connectSpec, Stream* stream, const char* threadName, bool isSocket) :
        bus(bus),
        stream(stream),
        txQueue(),
        txWaitQueue(),
        txQueueLock(),
        exitCount(0),
        rxThread(bus, rep, qcc::String(incoming ? "rx-srv-" : "rx-cli-") + threadName + "-" + U32ToString(threadCount), incoming),
        txThread(bus, rep, qcc::String(incoming ? "tx-srv-" : "tx-cli-") + threadName + "-" + U32ToString(threadCount), txQueue, txWaitQueue, txQueueLock),
        listener(NULL),
        connSpec(connectSpec),
        incoming(incoming),
        processId(-1),
        alljoynVersion(0),
        refCount(0),
        isSocket(isSocket),
        armRxPause(false),
        idleTimeoutCount(0),
        maxIdleProbes(0),
        idleTimeout(0),
        probeTimeout(0),
        threadName(threadName),
        started(false)
    {
        /* Increment threadCount so that each RemoteEndpoint Rx/Tx thread
         * has a different name - useful for identification
         */
        threadCount++;
    }

    ~Internal() {
    }

    BusAttachment& bus;                      /**< Message bus associated with this endpoint */
    qcc::Stream* stream;                     /**< Stream for this endpoint or NULL if uninitialized */

    std::deque<Message> txQueue;             /**< Transmit message queue */
    std::deque<qcc::Thread*> txWaitQueue;    /**< Threads waiting for txQueue to become not-full */
    qcc::Mutex txQueueLock;                  /**< Transmit message queue mutex */
    int32_t exitCount;                       /**< Number of sub-threads (rx and tx) that have exited (atomically incremented) */

    RxThread rxThread;                      /**< Thread used to receive messages from the media */
    TxThread txThread;                      /**< Thread used to send messages to the media */

    EndpointListener* listener;              /**< Listener for thread exit notifications */

    qcc::String connSpec;                    /**< Connection specification for out-going connections */
    bool incoming;                           /**< Indicates if connection is incoming (true) or outgoing (false) */

    Features features;                       /**< Requested and negotiated features of this endpoint */
    uint32_t processId;                      /**< Process id of the process at the remote end of this endpoint */
    uint32_t alljoynVersion;                 /**< AllJoyn version of the process at the remote end of this endpoint */
    int32_t refCount;                        /**< Number of active users of this remote endpoint */
    bool isSocket;                           /**< True iff this endpoint contains a SockStream as its 'stream' member */
    bool armRxPause;                         /**< Pause Rx after receiving next METHOD_REPLY message */

    uint32_t idleTimeoutCount;               /**< Number of consecutive idle timeouts */
    uint32_t maxIdleProbes;                  /**< Maximum number of missed idle probes before shutdown */
    uint32_t idleTimeout;                    /**< RX idle seconds before sending probe */
    uint32_t probeTimeout;                   /**< Probe timeout in seconds */

    String uniqueName;                       /**< Obtained from EndpointAuth */
    String remoteName;                       /**< Obtained from EndpointAuth */
    GUID128 remoteGUID;                      /**< Obtained from EndpointAuth */
    const char* threadName;                  /**< Transport Name for the Endpoint */
    bool started;                            /**< Is this EP started? */
};

void _RemoteEndpoint::SetStream(qcc::Stream* s)
{
    if (internal) {
        internal->stream = s;
    }
}


const qcc::String& _RemoteEndpoint::GetUniqueName() const
{
    if (internal) {
        return internal->uniqueName;
    } else {
        return String::Empty;
    }
}

const qcc::String& _RemoteEndpoint::GetRemoteName() const
{
    if (internal) {
        return internal->remoteName;
    } else {
        return String::Empty;
    }
}

const qcc::GUID128& _RemoteEndpoint::GetRemoteGUID() const
{
    if (internal) {
        return internal->remoteGUID;
    } else {
        static const qcc::GUID128 g;
        return g;
    }
}

qcc::Stream& _RemoteEndpoint::GetStream()
{
    if (internal) {
        return *(internal->stream);
    } else {
        static Stream stream;
        return stream;
    }
}

const qcc::String&  _RemoteEndpoint::GetConnectSpec() const
{
    if (internal) {
        return internal->connSpec;
    } else {
        return String::Empty;
    }
}

bool _RemoteEndpoint::IsIncomingConnection() const
{
    if (internal) {
        return internal->incoming;
    } else {
        return false;
    }
}

_RemoteEndpoint::Features&  _RemoteEndpoint::GetFeatures()
{
    if (internal) {
        return internal->features;
    } else {
        static Features f;
        return f;
    }
}

const _RemoteEndpoint::Features&  _RemoteEndpoint::GetFeatures() const
{
    if (internal) {
        return internal->features;
    } else {
        static Features f;
        return f;
    }
}

QStatus _RemoteEndpoint::Establish(const qcc::String& authMechanisms, qcc::String& authUsed, qcc::String& redirection)
{
    QStatus status = ER_BUS_NO_ENDPOINT;

    if (internal) {
        RemoteEndpoint rep = RemoteEndpoint::wrap(this);
        EndpointAuth auth(internal->bus, rep, internal->incoming);
        status = auth.Establish(authMechanisms, authUsed, redirection);
        if (status == ER_OK) {
            internal->uniqueName = auth.GetUniqueName();
            internal->remoteName = auth.GetRemoteName();
            internal->remoteGUID = auth.GetRemoteGUID();
            internal->features.protocolVersion = auth.GetRemoteProtocolVersion();
        }
    }
    return status;
}

QStatus _RemoteEndpoint::SetLinkTimeout(uint32_t& idleTimeout)
{
    if (internal) {
        internal->idleTimeout = 0;
    }
    return ER_OK;
}

/* Endpoint constructor */
_RemoteEndpoint::_RemoteEndpoint(BusAttachment& bus,
                                 bool incoming,
                                 const qcc::String& connectSpec,
                                 Stream* stream,
                                 const char* threadName,
                                 bool isSocket) :
    _BusEndpoint(ENDPOINT_TYPE_REMOTE)
{
    internal = new Internal(bus, this, incoming, connectSpec, stream, threadName, isSocket);
}

_RemoteEndpoint::~_RemoteEndpoint()
{
    if (internal) {
        Stop();
        Join();
        delete internal;
        internal = NULL;
    }
}

QStatus _RemoteEndpoint::SetLinkTimeout(uint32_t idleTimeout, uint32_t probeTimeout, uint32_t maxIdleProbes)
{
    QCC_DbgTrace(("_RemoteEndpoint::SetLinkTimeout(%u, %u, %u) for %s", idleTimeout, probeTimeout, maxIdleProbes, GetUniqueName().c_str()));

    if (GetRemoteProtocolVersion() >= 3) {
        internal->idleTimeout = idleTimeout;
        internal->probeTimeout = probeTimeout;
        internal->maxIdleProbes = maxIdleProbes;
        return internal->rxThread.Alert();
    } else {
        return ER_ALLJOYN_SETLINKTIMEOUT_REPLY_NO_DEST_SUPPORT;
    }
}

QStatus _RemoteEndpoint::Start()
{
    assert(internal);
    assert(internal->stream);
    QCC_DbgTrace(("_RemoteEndpoint::Start(isBusToBus = %s, allowRemote = %s)",
                  internal->features.isBusToBus ? "true" : "false",
                  internal->features.allowRemote ? "true" : "false"));
    QStatus status;
    Router& router = internal->bus.GetInternal().GetRouter();
    bool isTxStarted = false;
    bool isRxStarted = false;

    if (internal->features.isBusToBus) {
        endpointType = ENDPOINT_TYPE_BUS2BUS;
    }

    /* Set the send timeout for this endpoint */
    internal->stream->SetSendTimeout(120000);

    /* Endpoint needs to be wrapped before we can use it */
    RemoteEndpoint me = RemoteEndpoint::wrap(this);

    /* Start the TX thread */
    status = internal->txThread.Start(this, this);
    isTxStarted = (ER_OK == status);


    /* Register endpoint */
    if (ER_OK == status) {
        BusEndpoint bep = BusEndpoint::cast(me);
        status = router.RegisterEndpoint(bep);
    }

    /* Start the Rx thread */
    if (ER_OK == status) {
        status = internal->rxThread.Start(this, this);
        isRxStarted = (ER_OK == status);
    }

    /* If thread failed to start, then unregister. */
    if (status != ER_OK) {
        if (isTxStarted) {
            internal->txThread.Stop();
            internal->txThread.Join();
        }
        if (isRxStarted) {
            internal->rxThread.Stop();
            internal->rxThread.Join();
        }
        router.UnregisterEndpoint(this->GetUniqueName(), this->GetEndpointType());
        QCC_LogError(status, ("AllJoynRemoteEndoint::Start failed"));
    }

    if (status == ER_OK) {
        internal->started = true;
    }

    return status;
}

void _RemoteEndpoint::SetListener(EndpointListener* listener)
{
    if (internal) {
        internal->listener = listener;
    }

}

QStatus _RemoteEndpoint::Stop(void)
{
    QStatus ret = ER_BUS_NO_ENDPOINT;
    /* Ensure the endpoint is valid */
    if (!internal) {
        return ER_BUS_NO_ENDPOINT;
    }
    QCC_DbgPrintf(("_RemoteEndpoint::Stop(%s) called\n", GetUniqueName().c_str()));

    /* Alert any threads that are on the wait queue */
    internal->txQueueLock.Lock(MUTEX_CONTEXT);
    deque<Thread*>::iterator it = internal->txWaitQueue.begin();
    while (it != internal->txWaitQueue.end()) {
        (*it++)->Alert(ENDPOINT_IS_DEAD_ALERTCODE);
    }
    internal->txQueueLock.Unlock(MUTEX_CONTEXT);

    /*
     * Make the endpoint invalid - this prevents any further use of the endpoint that might delay
     * its ultimate demise.
     */
    Invalidate();
    /*
     * Don't call txThread.Stop() here; the logic in RemoteEndpoint::ThreadExit() takes care of
     * stopping the txThread.
     *
     * See also the comment in UnixTransport::Disconnect() which says that once this function is
     * called, the endpoint must be considered dead (i.e. may have been destroyed).  That comment
     * applied here means that once rxThread.Stop() above is called, this may have been destroyed.
     */

    ret = internal->rxThread.Stop();

    return ret;

}

QStatus _RemoteEndpoint::StopAfterTxEmpty(uint32_t maxWaitMs)
{
    QStatus status;
    /* Init wait time */
    uint32_t startTime = maxWaitMs ? GetTimestamp() : 0;

    /* Ensure the endpoint is valid */
    if (!internal) {
        return ER_BUS_NO_ENDPOINT;
    }

    /* Wait for txqueue to empty before triggering stop */
    internal->txQueueLock.Lock(MUTEX_CONTEXT);
    while (true) {
        if (internal->txQueue.empty() || (maxWaitMs && (qcc::GetTimestamp() > (startTime + maxWaitMs)))) {
            status = Stop();
            break;
        } else {
            internal->txQueueLock.Unlock(MUTEX_CONTEXT);
            qcc::Sleep(5);
            internal->txQueueLock.Lock(MUTEX_CONTEXT);
        }
    }
    internal->txQueueLock.Unlock(MUTEX_CONTEXT);
    return status;
}

QStatus _RemoteEndpoint::PauseAfterRxReply()
{

    if (internal) {
        internal->armRxPause = true;
        return ER_OK;
    } else {
        return ER_BUS_NO_ENDPOINT;
    }
}

QStatus _RemoteEndpoint::Join(void)
{
    /* Note that we don't join txThread and rxThread, rather we let the thread destructors handle
     * this when the RemoteEndpoint destructor is called. The reason for this is tied up in the
     * ThreadExit logic that coordinates the stopping of both rx and tx threads.
     */
    /* Ensure the endpoint is valid */
    if (!internal) {
        return ER_BUS_NO_ENDPOINT;
    }

    /*
     * block until the two threads have been joined
     * but *only* if this REP has started successfully, otherwise we'll wait forever
     * for two threads that have never been spawned
     * Also, wait until this endpoint has successfully unregistered itself from the DaemonRouter.
     */
    if (internal->started) {
        internal->txThread.Join();
        internal->rxThread.Join();
    }

    internal->started = false;
    return ER_OK;
}

void _RemoteEndpoint::ThreadExit(Thread* thread)
{
    /* If one thread stops, the other must too */
    if ((&internal->rxThread == thread) && internal->txThread.IsRunning()) {
        internal->txThread.Stop();

    } else if ((&internal->txThread == thread) && internal->rxThread.IsRunning()) {
        internal->rxThread.Stop();
    } else {
        /* This is notification of a txQueue waiter has died. Remove him */
        internal->txQueueLock.Lock(MUTEX_CONTEXT);
        deque<Thread*>::iterator it = find(internal->txWaitQueue.begin(), internal->txWaitQueue.end(), thread);
        if (it != internal->txWaitQueue.end()) {
            (*it)->RemoveAuxListener(this);
            internal->txWaitQueue.erase(it);
        }
        internal->txQueueLock.Unlock(MUTEX_CONTEXT);

        return;
    }

    /* Unregister endpoint when both rx and tx exit */

    if (2 == IncrementAndFetch(&internal->exitCount)) {
        RemoteEndpoint rep = RemoteEndpoint::wrap(this);
        /* De-register this remote endpoint */
        internal->bus.GetInternal().GetRouter().UnregisterEndpoint(this->GetUniqueName(), this->GetEndpointType());
        if (internal->listener) {
            internal->listener->EndpointExit(rep);
            internal->listener = NULL;
        }
    }
}

static inline bool IsControlMessage(Message& msg)
{
    const char* sender = msg->GetSender();
    size_t offset = ::strlen(sender);
    if (offset >= 2) {
        offset -= 2;
    }
    return (::strcmp(sender + offset, ".1") == 0) ? true : false;
}

void* _RemoteEndpoint::RxThread::Run(void* arg)
{
    QStatus status = ER_OK;
    const bool bus2bus = ENDPOINT_TYPE_BUS2BUS == ep->GetEndpointType();

    Router& router = bus.GetInternal().GetRouter();
    qcc::Event& ev = ep->GetSource().GetSourceEvent();
    /* Receive messages until the socket is disconnected */
    while (!IsStopping() && (ER_OK == status)) {
        uint32_t timeout = (ep->internal->idleTimeoutCount == 0) ? ep->internal->idleTimeout : ep->internal->probeTimeout;
        status = Event::Wait(ev, (timeout > 0) ? (1000 * timeout) : Event::WAIT_FOREVER);
        if (ER_OK == status) {
            Message msg(bus);
            RemoteEndpoint rep = RemoteEndpoint::wrap(ep);
            status = msg->Unmarshal(rep, (validateSender && !bus2bus));
            switch (status) {
            case ER_OK :
                ep->internal->idleTimeoutCount = 0;
                bool isAck;
                if (ep->IsProbeMsg(msg, isAck)) {
                    QCC_DbgPrintf(("%s: Received %s\n", ep->GetUniqueName().c_str(), isAck ? "ProbeAck" : "ProbeReq"));
                    if (!isAck) {
                        /* Respond to probe request */
                        Message probeMsg(bus);
                        status = ep->GenProbeMsg(true, probeMsg);
                        if (status == ER_OK) {
                            status = ep->PushMessage(probeMsg);
                        }
                        QCC_DbgPrintf(("%s: Sent ProbeAck (%s)\n", ep->GetUniqueName().c_str(), QCC_StatusText(status)));
                    }
                } else {
                    BusEndpoint bep  = BusEndpoint::cast(rep);

                    status = router.PushMessage(msg, bep);
                    if (status != ER_OK) {
                        /*
                         * There are five cases where a failure to push a message to the router is ok:
                         *
                         * 1) The message received did not match the expected signature.
                         * 2) The message was a method reply that did not match up to a method call.
                         * 3) A daemon is pushing the message to a connected client or service.
                         * 4) Pushing a message to an endpoint that has closed.
                         * 5) Pushing the first non-control message of a new session (must wait for route to be fully setup)
                         *
                         */
                        if (status == ER_BUS_NO_ROUTE) {
                            int retries = 20;
                            while (!IsStopping() && (status == ER_BUS_NO_ROUTE) && !hasRxSessionMsg && retries--) {
                                qcc::Sleep(10);
                                status = router.PushMessage(msg, bep);
                            }
                        }
                        if ((router.IsDaemon() && !bus2bus) || (status == ER_BUS_SIGNATURE_MISMATCH) || (status == ER_BUS_UNMATCHED_REPLY_SERIAL) || (status == ER_BUS_ENDPOINT_CLOSING)) {
                            QCC_DbgHLPrintf(("Discarding %s: %s", msg->Description().c_str(), QCC_StatusText(status)));
                            status = ER_OK;
                        }
                    }
                    /* Update haxRxSessionMessage */
                    if ((status == ER_OK) && !hasRxSessionMsg && !IsControlMessage(msg)) {
                        hasRxSessionMsg = true;
                    }
                }
                break;

            case ER_BUS_CANNOT_EXPAND_MESSAGE :
                /*
                 * The message could not be expanded so pass it the peer object to request the expansion
                 * rule from the endpoint that sent it.
                 */
                status = bus.GetInternal().GetLocalEndpoint()->GetPeerObj()->RequestHeaderExpansion(msg, rep);
                if ((status != ER_OK) && router.IsDaemon()) {
                    QCC_LogError(status, ("Discarding %s", msg->Description().c_str()));
                    status = ER_OK;
                }
                break;

            case ER_BUS_TIME_TO_LIVE_EXPIRED:
                QCC_DbgHLPrintf(("TTL expired discarding %s", msg->Description().c_str()));
                status = ER_OK;
                break;

            case ER_BUS_INVALID_HEADER_SERIAL:
                /*
                 * Ignore invalid serial numbers for unreliable messages or broadcast messages that come from
                 * bus2bus endpoints as these can be delivered out-of-order or repeated.
                 *
                 * Ignore control messages (i.e. messages targeted at the bus controller)
                 * TODO - need explanation why this is neccessary.
                 *
                 * In all other cases an invalid serial number cause the connection to be dropped.
                 */
                if (msg->IsUnreliable() || msg->IsBroadcastSignal() || IsControlMessage(msg)) {
                    QCC_DbgHLPrintf(("Invalid serial discarding %s", msg->Description().c_str()));
                    status = ER_OK;
                } else {
                    QCC_LogError(status, ("Invalid serial %s", msg->Description().c_str()));
                }
                break;

            case ER_ALERTED_THREAD:
                GetStopEvent().ResetEvent();
                status = ER_OK;
                break;

            default:
                break;
            }
            /* Check pause condition. Block until stopped */
            if (ep->internal->armRxPause && !IsStopping() && (msg->GetType() == MESSAGE_METHOD_RET)) {
                status = Event::Wait(Event::neverSet);
            }
        } else if (status == ER_TIMEOUT) {
            if (ep->internal->idleTimeoutCount++ < ep->internal->maxIdleProbes) {
                Message probeMsg(bus);
                status = ep->GenProbeMsg(false, probeMsg);
                if (status == ER_OK) {
                    status = ep->PushMessage(probeMsg);
                }
                QCC_DbgPrintf(("%s: Sent ProbeReq (%s)\n", ep->GetUniqueName().c_str(), QCC_StatusText(status)));
            } else {
                QCC_DbgPrintf(("%s: Maximum number of idle probe (%d) attempts reached", ep->GetUniqueName().c_str(), ep->internal->maxIdleProbes));
            }
        } else if (status == ER_ALERTED_THREAD) {
            GetStopEvent().ResetEvent();
            status = ER_OK;
        }
    }

    if ((status != ER_OK) && (status != ER_STOPPING_THREAD) && (status != ER_SOCK_OTHER_END_CLOSED) && (status != ER_BUS_STOPPING)) {
        QCC_LogError(status, ("Endpoint Rx thread (%s) exiting", GetName()));
    }

    /* On an unexpected disconnect save the status that cause the thread exit */
    if (ep->disconnectStatus == ER_OK) {
        ep->disconnectStatus = (status == ER_STOPPING_THREAD) ? ER_OK : status;
    }
    ep->Invalidate();

    /* Inform transport of endpoint exit */
    return (void*) status;
}

void* _RemoteEndpoint::TxThread::Run(void* arg)
{
    QStatus status = ER_OK;

    /* Wait for queue to be non-empty */
    while (!IsStopping() && (ER_OK == status)) {
        status = Event::Wait(Event::neverSet);
        if (!IsStopping() && (ER_ALERTED_THREAD == status)) {
            stopEvent.ResetEvent();
            status = ER_OK;
            queueLock.Lock(MUTEX_CONTEXT);
            while ((status == ER_OK) && !queue.empty() && !IsStopping()) {
                /* Get next message */
                Message msg = queue.back();

                /* Alert next thread on wait queue */
                if (0 < waitQueue.size()) {
                    Thread* wakeMe = waitQueue.back();
                    waitQueue.pop_back();
                    status = wakeMe->Alert();
                    if (ER_OK != status) {
                        QCC_LogError(status, ("Failed to alert thread blocked on full tx queue"));
                    }
                }
                queueLock.Unlock(MUTEX_CONTEXT);
                /* Deliver message */
                RemoteEndpoint rep = RemoteEndpoint::wrap(ep);
                status = msg->Deliver(rep);
                /* Report authorization failure as a security violation */
                if (status == ER_BUS_NOT_AUTHORIZED) {
                    bus.GetInternal().GetLocalEndpoint()->GetPeerObj()->HandleSecurityViolation(msg, status);
                    /*
                     * Clear the error after reporting the security violation otherwise we will exit
                     * this thread which will shut down the endpoint.
                     */
                    status = ER_OK;
                }
                queueLock.Lock(MUTEX_CONTEXT);
                queue.pop_back();
            }
            queueLock.Unlock(MUTEX_CONTEXT);
        }
    }
    /* Wake any thread waiting on tx queue availability */
    queueLock.Lock(MUTEX_CONTEXT);
    while (0 < waitQueue.size()) {
        Thread* wakeMe = waitQueue.back();
        QStatus status = wakeMe->Alert();
        if (ER_OK != status) {
            QCC_LogError(status, ("Failed to clear tx wait queue"));
        }
        waitQueue.pop_back();
    }
    queueLock.Unlock(MUTEX_CONTEXT);

    /* On an unexpected disconnect save the status that cause the thread exit */
    if (ep->disconnectStatus == ER_OK) {
        ep->disconnectStatus = (status == ER_STOPPING_THREAD) ? ER_OK : status;
    }
    ep->Invalidate();

    /* Inform transport of endpoint exit */
    return (void*) status;
}

QStatus _RemoteEndpoint::PushMessage(Message& msg)
{
    QCC_DbgTrace(("RemoteEndpoint::PushMessage(serial=%d)", msg->GetCallSerial()));

    static const size_t MAX_TX_QUEUE_SIZE = 30;

    QStatus status = ER_OK;

    /* Remote endpoints can be invalid if they were created with the default
     * constructor or being torn down. Return ER_BUS_NO_ENDPOINT only if the
     * endpoint was created with the default constructor. i.e. internal=NULL
     */
    if (!internal) {
        return ER_BUS_NO_ENDPOINT;
    }
    /*
     * Don't continue if this endpoint is in the process of being closed
     * Otherwise we risk deadlock when sending NameOwnerChanged signal to
     * this dying endpoint
     */
    if (internal->rxThread.IsStopping() || internal->txThread.IsStopping()) {
        return ER_BUS_ENDPOINT_CLOSING;
    }
    internal->txQueueLock.Lock(MUTEX_CONTEXT);
    size_t count = internal->txQueue.size();
    bool wasEmpty = (count == 0);
    if (MAX_TX_QUEUE_SIZE > count) {
        internal->txQueue.push_front(msg);
    } else {
        while (true) {
            /* Remove a queue entry whose TTLs is expired if possible */
            deque<Message>::iterator it = internal->txQueue.begin();
            uint32_t maxWait = 20 * 1000;
            while (it != internal->txQueue.end()) {
                uint32_t expMs;
                if ((*it)->IsExpired(&expMs)) {
                    internal->txQueue.erase(it);
                    break;
                } else {
                    ++it;
                }
                maxWait = (std::min)(maxWait, expMs);
            }
            if (internal->txQueue.size() < MAX_TX_QUEUE_SIZE) {
                /* Check queue wasn't drained while we were waiting */
                if (internal->txQueue.size() == 0) {
                    wasEmpty = true;
                }
                internal->txQueue.push_front(msg);
                status = ER_OK;
                break;
            } else {
                /* This thread will have to wait for room in the queue */
                Thread* thread = Thread::GetThread();
                assert(thread);

                thread->AddAuxListener(this);
                internal->txWaitQueue.push_front(thread);
                internal->txQueueLock.Unlock(MUTEX_CONTEXT);
                status = Event::Wait(Event::neverSet, maxWait);
                internal->txQueueLock.Lock(MUTEX_CONTEXT);

                /* Reset alert status */
                if (ER_ALERTED_THREAD == status) {
                    if (thread->GetAlertCode() == ENDPOINT_IS_DEAD_ALERTCODE) {
                        status = ER_BUS_ENDPOINT_CLOSING;
                    }
                    thread->GetStopEvent().ResetEvent();
                }
                /* Remove thread from wait queue. */
                thread->RemoveAuxListener(this);
                deque<Thread*>::iterator eit = find(internal->txWaitQueue.begin(), internal->txWaitQueue.end(), thread);
                if (eit != internal->txWaitQueue.end()) {
                    internal->txWaitQueue.erase(eit);
                }

                if ((ER_OK != status) && (ER_ALERTED_THREAD != status) && (ER_TIMEOUT != status)) {
                    break;
                }

            }
        }
    }
    internal->txQueueLock.Unlock(MUTEX_CONTEXT);

    if (wasEmpty) {
        status = internal->txThread.Alert();
    }

#ifndef NDEBUG
#undef QCC_MODULE
#define QCC_MODULE "TXSTATS"
    static uint32_t lastTime = 0;
    uint32_t now = GetTimestamp();
    if ((now - lastTime) > 1000) {
        QCC_DbgPrintf(("Tx queue size (%s - %x) = %d", internal->txThread.GetName(), internal->txThread.GetHandle(), count));
        lastTime = now;
    }
#undef QCC_MODULE
#define QCC_MODULE "ALLJOYN"
#endif
    return status;
}

void _RemoteEndpoint::IncrementRef()
{
    int refs = IncrementAndFetch(&internal->refCount);
    QCC_DbgPrintf(("_RemoteEndpoint::IncrementRef(%s) refs=%d\n", GetUniqueName().c_str(), refs));

}

void _RemoteEndpoint::DecrementRef()
{
    int refs = DecrementAndFetch(&internal->refCount);
    QCC_DbgPrintf(("_RemoteEndpoint::DecrementRef(%s) refs=%d\n", GetUniqueName().c_str(), refs));
    if (refs <= 0) {
        Thread* curThread = Thread::GetThread();
        if ((curThread == &internal->rxThread) || (curThread == &internal->txThread)) {
            Stop();
        } else {
            StopAfterTxEmpty(500);
        }
    }
}

bool _RemoteEndpoint::IsProbeMsg(const Message& msg, bool& isAck)
{
    bool ret = false;
    if (0 == ::strcmp(org::alljoyn::Daemon::InterfaceName, msg->GetInterface())) {
        if (0 == ::strcmp("ProbeReq", msg->GetMemberName())) {
            ret = true;
            isAck = false;
        } else if (0 == ::strcmp("ProbeAck", msg->GetMemberName())) {
            ret = true;
            isAck = true;
        }
    }
    return ret;
}

QStatus _RemoteEndpoint::GenProbeMsg(bool isAck, Message msg)
{
    return msg->SignalMsg("", NULL, 0, "/", org::alljoyn::Daemon::InterfaceName, isAck ? "ProbeAck" : "ProbeReq", NULL, 0, 0, 0);
}

}
