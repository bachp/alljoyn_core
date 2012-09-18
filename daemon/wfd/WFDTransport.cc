/**
 * @file
 * WFDTransport is an implementation of WFDTransportBase for daemons.
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

#include <qcc/platform.h>
#include <qcc/IPAddress.h>
#include <qcc/Socket.h>
#include <qcc/SocketStream.h>
#include <qcc/String.h>
#include <qcc/StringUtil.h>
#include <qcc/IfConfig.h>

#include <alljoyn/BusAttachment.h>
#include <alljoyn/TransportMask.h>
#include <alljoyn/Session.h>

#include "BusInternal.h"
#include "RemoteEndpoint.h"
#include "Router.h"
#include "DaemonConfig.h"
#include "ns/IpNameService.h"
#include "WFDTransport.h"

/*
 * How the transport fits into the system
 * ======================================
 *
 * AllJoyn provides the concept of a Transport which provides a relatively
 * abstract way for the daemon to use different network mechanisms for getting
 * Messages from place to another.  Conceptually, think of, for example, a Unix
 * transport that moves bits using unix domain sockets, a Bluetooth transport
 * that moves bits over a Bluetooth link and a WFD transport that moves Messages
 * over a WFD connection.
 *
 * In networking 101, one discovers that BSD sockets is oriented toward clients
 * and servers.  There are different sockets calls required for a program
 * implementing a server-side part and a client side part.  The server-side
 * listens for incoming connection requests and the client-side initiates the
 * requests.  AllJoyn clients are bus attachments that our Applications may use
 * and these can only initiate connection requests to AllJoyn daemons.  Although
 * dameons may at first blush appear as the service side of a typical BSD
 * sockets client-server pair, it turns out that while daemons obviously must
 * listen for incoming connections, they also must be able to initiate
 * connection requests to other daemons.  It turns out that there is very little
 * in the way of common code when comparing the client version of a WFD
 * transport and a daemon version.  Therefore you will find a WFDTransport
 * class in here the dameon directory and a client version, called simply
 * WFDTransport, in the src directory.
 *
 * This file is the WFDTransport.  It needs to act as both a client and a
 * server explains the presence of both connect-like methods and listen-like
 * methods here.
 *
 * A fundamental idiom in the AllJoyn system is that of a thread.  Active
 * objects in the system that have threads wandering through them will implement
 * Start(), Stop() and Join() methods.  These methods work together to manage
 * the autonomous activities that can happen in a WFDTransport.  These
 * activities are carried out by so-called hardware threads.  POSIX defines
 * functions used to control hardware threads, which it calls pthreads.  Many
 * threading packages use similar constructs.
 *
 * In a threading package, a start method asks the underlying system to arrange
 * for the start of thread execution.  Threads are not necessarily running when
 * the start method returns, but they are being *started*.  Some time later, a
 * thread of execution appears in a thread run function, at which point the
 * thread is considered *running*.  In the case of the WFDTransport, the Start() method
 * spins up a thread to run the BSD sockets' server accept loop.  This
 * also means that as soon as Start() is executed, a thread may be using underlying
 * socket file descriptors and one must be very careful about convincing the
 * accept loop thread to exit before deleting the resources.
 *
 * In generic threads packages, executing a stop method asks the underlying
 * system to arrange for a thread to end its execution.  The system typically
 * sends a message to the thread to ask it to stop doing what it is doing.  The
 * thread is running until it responds to the stop message, at which time the
 * run method exits and the thread is considered *stopping*.  The
 * WFDTransport provides a Stop() method to do exactly that.
 *
 * Note that neither of Start() nor Stop() are synchronous in the sense that one
 * has actually accomplished the desired effect upon the return from a call.  Of
 * particular interest is the fact that after a call to Stop(), threads will
 * still be *running* for some non-deterministic time.
 *
 * In order to wait until all of the threads have actually stopped, a blocking
 * call is required.  In threading packages this is typically called join, and
 * our corresponding method is called Join().  A user of the DaemonWfdTransport
 * must assume that immediately after a call to Start() is begun, and until a
 * call to Join() returns, there may be threads of execution wandering anywhere
 * in the DaemonWfdTransport and in any callback registered by the caller.
 *
 * The high-level process for how an advertisement translates into a transport
 * Connect() is a bit opaque, so we paint a high-level picture here.
 *
 * First, a service (that will be handling RPC calls and emitting signals)
 * acquires a name on the bus, binds a session and calls AdvertiseName.  This
 * filters down (possibly through language bindings) to the AllJoyn object, into
 * the transports on the transport list (the WFD transport is one of those) and
 * eventually to the IpNameService::AdvertiseName() method we call since we are
 * an IP-based transport.  The IP name service will multicast the advertisements
 * to other daemons listening on our device's connected networks.
 *
 * A client that is interested in using the service calls the discovery
 * method FindAdvertisedName.  This filters down (possibly through
 * language bindings) to the AllJoyn object, into the transports on the
 * transport list (us) and we eventually call IpNameService::FindAdvertisedName()
 * since we are an IP-based transport.  The IP name service multicasts the
 * discovery message to other daemons listening on our networks.
 *
 * The daemon remembers which clients have expressed interest in which services,
 * and expects name services to call back with the bus addresses of daemons they
 * find which have the associated services.  In version zero of the protocol,
 * the only endpoint type supported was a WFD endpoint.  In the case of version
 * one, we have four, so we now see "different" bus addresses coming from the
 * name service and "different" connect specs coming from AllJoyn proper.
 *
 * When a new advertisement is received (because we called our listener's
 * Found() method here, the bus address is "hidden" from interested clients and
 * replaced with a more generic TransportMask bit (for us it will be
 * TRANSPORT_WFD).  The client either responds by ignoring the advertisement,
 * waits to accumulate more answers or joins a session to the implied
 * daemon/service.  A reference to a SessionOpts object is provided as a
 * parameter to a JoinSession call if the client wants to connect.  This
 * SessionOpts reference is passed down into the transport (selected by the
 * TransportMask) into the Connect() method which is used to establish the
 * connection.
 *
 * The four different connection mechanisms can be viewed as a matrix;
 *
 *                                                      IPv4               IPv6
 *                                                 ---------------    ---------------
 *     TRAFFIC MESSAGES | TRAFFIC_RAW_RELIABLE  |   Reliable IPv4      Reliable IPv6
 *     TRAFFIC_RAW_UNRELIABLE                   |  Unreliable IPv4    Unreliable IPv6
 *
 * The bits in the provided SessionOpts select the row, but the column is left
 * free (unspecified).  This means that it is up to the transport to figure out
 * which one to use.  Clearly, if only one of the two address flavors is
 * possible (known from examining the returned bus address which is called a
 * connect spec in the Connect() method) the transport should choose that one.
 * If both IPv4 or IPv6 are available, it is up to the transport (again, us) to
 * choose the "best" method since we don't bother clients with that level of
 * detail.  We (WFD) generally choose IPv6 when given the choice since DHCP on
 * IPv4 is sometimes problematic in some networks.
 *
 * Internals
 * =========
 *
 * We spend a lot of time on the threading aspects of the transport since they
 * are often the hardest part to get right and are complicated.  This is where
 * the bugs live.
 *
 * As mentioned above, the AllJoyn system uses the concept of a Transport.  You
 * are looking at the WFDTransport.  Each transport also has the concept
 * of an Endpoint.  The most important function fo an endpoint is to provide
 * non-blocking semantics to higher level code.  This is provided by a transmit
 * thread on the write side which can block without blocking the higher level
 * code, and a receive thread which can similarly block waiting for data without
 * blocking the higher level code.
 *
 * Endpoints are specialized into the LocalEndpoint and the RemoteEndpoint
 * classes.  LocalEndpoint represents a connection from a router to the local
 * bus attachment or daemon (within the "current" process).  A RemoteEndpoint
 * represents a connection from a router to a remote attachment or daemon.  By
 * definition, the WFDTransport provides RemoteEndpoint functionality.
 *
 * RemoteEndpoints are further specialized according to the flavor of the
 * corresponding transport, and so you will see a WFDEndpoint class
 * defined below which provides functionality to send messages from the local
 * router to a destination off of the local process using a WFD transport
 * mechanism.
 *
 * RemoteEndpoints use AllJoyn stream objects to actually move bits.  This
 * is a thin layer on top of a Socket (which is another thin layer on top of
 * a BSD socket) that provides PushBytes() adn PullBytes() methods.  Remote
 * endpoints also provide the transmit thread and receive threads mentioned
 * above.
 *
 * The job of the receive thread is to loop waiting for bytes to appear on the
 * input side of the stream and to unmarshal them into AllJoyn Messages.  Once
 * an endpoint has a message, it calls into the Message router (PushMessage) to
 * arrange for delivery.  The job of the transmit thread is to loop waiting for
 * Messages to appear on its transmit queue.  When a Message is put on the queue
 * by a Message router, the transmit thread will pull it off and marshal it,
 * then it will write the bytes to the transport mechanism.
 *
 * The WFDEndpoint inherits the infrastructure requred to do most of its
 * work from the more generic RemoteEndpoint class.  It needs to do specific
 * WFD-related work and also provide for authenticating the endpoint before it
 * is allowed to start pumping messages.  Authentication means running some
 * mysterious (to us) process that may involve some unknown number of challenge
 * and response messsages being exchanged between the client and server side of
 * the connection.  Since we cannot block a caller waiting for authentication,
 * this must done on another thread; and this must be done before the
 * RemoteEndpoint is Start()ed -- before its transmit and receive threads are
 * started, lest they start pumping messages and interfering with the
 * authentication process.
 *
 * Authentication can, of course, succeed or fail based on timely interaction
 * between the two sides, but it can also be abused in a denial of service
 * attack.  If a client simply starts the process but never responds, it could
 * tie up a daemon's resources, and coordinated action could bring down a
 * daemon.  Because of this, we need to provide a way to reach in and abort
 * authentications that are "taking too long."
 *
 * As described above, a daemon can listen for inbound connections and it can
 * initiate connections to remote daemons.  Authentication must happen in both
 * cases.
 *
 * If you consider all that is happening, we are talking about a complicated
 * system of many threads that are appearing and disappearing in the system at
 * unpredictable times.  These threads have dependencies in the resources
 * associated with them (sockets and events in particular).  These resources may
 * have further dependencies that must be respected.  For example, Events may
 * have references to Sockets.  The Sockets must not be released before the
 * Events are released, because the events would be left with stale handles.  An
 * even scarier case is if an underlying Socket FD is reused at just the wrong
 * time, it would be possible to switch a Socket FD from one connection to
 * another out from under an Event without its knowledge.
 *
 * To summarize, consider the following "big picture' view of the transport.  A
 * single WFDTransport is constructed if the daemon TransportList
 * indicates that WFD support is required.  The high-level daemon code (see
 * bbdaemon.cc for example) builds a TransportFactoryContainer that is
 * initialized with a factory that knows how to make WFDTransport objects
 * if they are needed, and associates the factory with the string "wfd".  The
 * daemon also constructs "server args" which may contain the string "wfd" or
 * "bluetooth" or "unix".  If the factory container provides a "wfd" factory and
 * the server args specify a "wfd" transport is needed then a WFDTransport
 * object is instantiated and entered into the daemon's internal transport list
 * (list of available transports).  Also provided for each transport is an abstract
 * address to listen for incoming connection requests on.
 *
 * When the daemon is brought up, its TransportList is Start()ed.  The transport
 * specs string (e.g., "unix:abstract=alljoyn;wfd:;bluetooth:") is provided to
 * TransportList::Start() as a parameter.  The transport specs string is parsed
 * and in the example above, results in "unix" transports, "wfd" transports and
 * "bluetooth" transports being instantiated and started.  As mentioned
 * previously "wfd" in the daemon translates into WFDTransport.  Once the
 * desired transports are instantiated, each is Start()ed in turn.  In the case
 * of the WFDTransport, this will start the server accept loop.  Initially
 * there are no sockets to listen on.
 *
 * The daemon then needs to start listening on some inbound addresses and ports.
 * This is done by the StartListen() command which you can find in bbdaemon, for
 * example.  This alwo takes the same king of server args string shown above but
 * this time the address and port information are used.  For example, one might
 * use the string "wfd:addr=0.0.0.0,port=9955;" to specify which address and
 * port to listen to.  This Bus::StartListen() call is translated into a
 * WFDTransport::StartListen() call which is provided with the string
 * which we call a "listen spec".  Our StartListen() will create a Socket, bind
 * the socket to the address and port provided and save the new socket on a list
 * of "listenFds." It will then Alert() the already running server accept loop
 * thread -- see WFDTransport::Run().  Each time through the server accept
 * loop, Run() will examine the list of listenFds and will associate an Event
 * with the corresponding socketFd and wait for connection requests.
 *
 * There is a complementary call to stop listening on addresses.  Since the
 * server accept loop is depending on the associated sockets, StopListen must
 * not close those Sockets, it must ask the server accept loop to do so in a
 * coordinated way.
 *
 * When an inbound connection request is received, the accept loop will wake up
 * and create a WFDEndpoint for the *proposed* new connection.  Recall
 * that an endpoint is not brought up immediately, but an authentication step
 * must be performed.  The server accept loop starts this process by placing the
 * new WFDEndpoint on an authList, or list of authenticating endpoints.
 * It then calls the endpoint Authenticate() method which spins up an
 * authentication thread and returns immediately.  This process transfers the
 * responsibility for the connection and its resources to the authentication
 * thread.  Authentication can succeed, fail, or take to long and be aborted.
 *
 * If authentication succeeds, the authentication thread calls back into the
 * WFDTransport's Authenticated() method.  Along with indicating that
 * authentication has completed successfully, this transfers ownership of the
 * WFDEndpoint back to the WFDTransport from the authentication
 * thread.  At this time, the WFDEndpoint is Start()ed which spins up
 * the transmit and receive threads and enables Message routing across the
 * transport.
 *
 * If the authentication fails, the authentication thread simply sets a the
 * WFDEndpoint state to FAILED and exits.  The server accept loop looks at
 * authenticating endpoints (those on the authList)each time through its loop.
 * If an endpoint has failed authentication, and its thread has actually gone
 * away (or more precisely is at least going away in such a way that it will
 * never touch the endpoint data structure again).  This means that the endpoint
 * can be deleted.
 *
 * If the authentication takes "too long" we assume that a denial of service
 * attack in in progress.  We call AuthStop() on such an endpoint which will most
 * likely induce a failure (unless we happen to call abort just as the endpoint
 * actually finishes the authentication which is highly unlikely but okay).
 * This AuthStop() will cause the endpoint to be scavenged using the above mechanism
 * the next time through the accept loop.
 *
 * A daemon transport can accept incoming connections, and it can make outgoing
 * connections to another daemon.  This case is simpler than the accept case
 * since it is expected that a socket connect can block, so it is possible to do
 * authentication in the context of the thread calling Connect().  Connect() is
 * provided a so-called "connect spec" which provides an IP address ("r4addr=xxxx"),
 * port ("r4port=yyyy") in a String.
 *
 * A check is always made to catch an attempt for the daemon to connect to
 * itself which is a system-defined error (it causes the daemon grief, so we
 * avoid it here by looking to see if one of the listenFds is listening on an
 * interface that corresponds to the address in the connect spec).
 *
 * If the connect is allowed, we do the usual BSD sockets thing where we create
 * a socket and connect to the specified remote address.  The DBus spec says that
 * all connections must begin with one uninterpreted byte so we send that.  This
 * byte is only meaningful in Unix domain sockets transports, but we must send it
 * anyway.
 *
 * The next step is to create a WFDEndpoint and to put it on the endpointList.
 * Note that the endpoint doesn't go on the authList as in the server case, it
 * goes on the list of active endpoints.  This is because a failure to authenticate
 * on the client side results in a call to EndpointExit which is the same code path as
 * a failure when the endpoint is up.  The failing endpoint must be on the endpoint
 * list in order to allow authentication errors to be propagated back to higher-level
 * code in a meaningful context.  Once the endpoint is stored on the list, Connect()
 * starts client-side Authentication with the remote (server) side.  If Authentication
 * succeeds, the endpoint is Start()ed which will spin up the rx and tx threads that
 * start Message routing across the link.  The endpoint is left on the endpoint list
 * in this case.  If authentication fails, the endpoint is removed from the active
 * list.  This is thread-safe since there is no authentication thread running because
 * the authentication was done in the context of the thread calling Connect() which
 * is the one deleting the endpoint; and no rx or tx thread is spun up if the
 * authentication fails.
 *
 * Shutting the WFDTransport down involves orchestrating the orderly termination
 * of:
 *
 *   1) Threads that may be running in the server accept loop with associated Events
 *      and their dependent socketFds stored in the listenFds list.
 *   2) Threads that may be running authentication with associated endpoint objects,
 *      streams and SocketFds.  These threads are accessible through endpoint objects
 *      stored on the authList.
 *   3) Threads that may be running the rx and tx loops in endpoints which are up and
 *      running, transporting routable Messages through the system.
 *
 * Note that we also have to understand and deal with the fact that threads
 * running in state (2) above, will exit and depend on the server accept loop to
 * scavenge the associated objects off of the authList and delete them.  This
 * means that the server accept loop cannot be Stop()ped until the authList is
 * empty.  We further have to understand that threads running in state (3) above
 * will depend on the hooked EndpointExit function to dispose of associated
 * resources.  This will happen in the context of either the transmit or receive
 * thread (the last to go).  We can't delete the transport until all of its
 * associated endpoint threads are Join()ed.  Also, since the server accept loop
 * is looking at the list of listenFDs, we must be careful about deleting those
 * sockets out from under the server thread.  The system should call
 * StopListen() on all of the listen specs it called StartListen() on; but we
 * need to be prepared to clean up any "unstopped" listen specs in a coordinated
 * way.  This, in turn, means that the server accept loop cannot be Stop()ped
 * until all of the listenFds are cleaned up.
 *
 * There are a lot of dependencies here, so be careful when making changes to
 * the thread and resource management here.  It's quite easy to shoot yourself
 * in multiple feet you never knew you had if you make an unwise modification,
 * and sometimes the results are tiny little time-bombs set to go off in
 * completely unrelated code (if, for example, a socket is deleted and reused
 * by another piece of code while the transport still has an event referencing
 * the socket now used by the other module).
 */

#define QCC_MODULE "WFD"

using namespace std;
using namespace qcc;

const uint32_t WFD_LINK_TIMEOUT_PROBE_ATTEMPTS       = 1;
const uint32_t WFD_LINK_TIMEOUT_PROBE_RESPONSE_DELAY = 10;
const uint32_t WFD_LINK_TIMEOUT_MIN_LINK_TIMEOUT     = 40;

namespace ajn {

/**
 * Name of transport used in transport specs.
 */
const char* WFDTransport::TransportName = "wfd";

/*
 * An endpoint class to handle the details of authenticating a connection in a
 * way that avoids denial of service attacks.
 */
class WFDEndpoint : public RemoteEndpoint {
  public:
    /**
     * There are three threads that can be running around in this data
     * structure.  An auth thread is run before the endpoint is started in order
     * to handle the security stuff that must be taken care of before messages
     * can start passing.  This enum reflects the states of the authentication
     * process and the state can be found in m_authState.  Once authentication
     * is complete, the auth thread must go away, but it must also be joined,
     * which is indicated by the AUTH_DONE state.  The other threads are the
     * endpoint RX and TX threads, which are dealt with by the EndpointState.
     */
    enum AuthState {
        AUTH_ILLEGAL = 0,
        AUTH_INITIALIZED,    /**< This endpoint structure has been allocated but no auth thread has been run */
        AUTH_AUTHENTICATING, /**< We have spun up an authentication thread and it has begun running our user function */
        AUTH_FAILED,         /**< The authentication has failed and the authentication thread is exiting immidiately */
        AUTH_SUCCEEDED,      /**< The auth process (Establish) has succeeded and the connection is ready to be started */
        AUTH_DONE,           /**< The auth thread has been successfully shut down and joined */
    };

    /**
     * There are three threads that can be running around in this data
     * structure.  Two threads, and RX thread and a TX thread are used to pump
     * messages through an endpoint.  These threads cannot be run until the
     * authentication process has completed.  This enum reflects the states of
     * the endpoint RX and TX threads and can be found in m_epState.  The auth
     * thread is dealt with by the AuthState enum above.  These threads must be
     * joined when they exit, which is indicated by the EP_DONE state.
     */
    enum EndpointState {
        EP_ILLEGAL = 0,
        EP_INITIALIZED,      /**< This endpoint structure has been allocated but not used */
        EP_FAILED,           /**< Starting the RX and TX threads has failed and this endpoint is not usable */
        EP_STARTED,          /**< The RX and TX threads have been started (they work as a unit) */
        EP_STOPPING,         /**< The RX and TX threads are stopping (have run ThreadExit) but have not been joined */
        EP_DONE              /**< The RX and TX threads have been shut down and joined */
    };

    /**
     * Connections can either be created as a result of a Connect() or an Accept().
     * If a connection happens as a result of a connect it is the active side of
     * a connection.  If a connection happens because of an Accpet() it is the
     * passive side of a connection.  This is important because of reference
     * counting of bus-to-bus endpoints.
     */
    enum SideState {
        SIDE_ILLEGAL = 0,
        SIDE_INITIALIZED,    /**< This endpoint structure has been allocated but don't know if active or passive yet */
        SIDE_ACTIVE,         /**< This endpoint is the active side of a connection */
        SIDE_PASSIVE         /**< This endpoint is the passive side of a connection */
    };

    WFDEndpoint(WFDTransport* transport,
                BusAttachment& bus,
                bool incoming,
                const qcc::String connectSpec,
                qcc::SocketFd sock,
                const qcc::IPAddress& ipAddr,
                uint16_t port)
        : RemoteEndpoint(bus, incoming, connectSpec, &m_stream, "wfd"),
        m_transport(transport),
        m_sideState(SIDE_INITIALIZED),
        m_authState(AUTH_INITIALIZED),
        m_epState(EP_INITIALIZED),
        m_tStart(qcc::Timespec(0)),
        m_authThread(this, transport),
        m_stream(sock),
        m_ipAddr(ipAddr),
        m_port(port),
        m_wasSuddenDisconnect(!incoming) { }

    virtual ~WFDEndpoint() { }

    void SetStartTime(qcc::Timespec tStart) { m_tStart = tStart; }
    qcc::Timespec GetStartTime(void) { return m_tStart; }
    QStatus Authenticate(void);
    void AuthStop(void);
    void AuthJoin(void);
    const qcc::IPAddress& GetIPAddress() { return m_ipAddr; }
    uint16_t GetPort() { return m_port; }

    SideState GetSideState(void) { return m_sideState; }

    void SetActive(void)
    {
        m_sideState = SIDE_ACTIVE;
    }

    void SetPassive(void)
    {
        m_sideState = SIDE_PASSIVE;
    }


    AuthState GetAuthState(void) { return m_authState; }

    void SetAuthDone(void)
    {
        m_authState = AUTH_DONE;
    }

    void SetAuthenticating(void)
    {
        m_authState = AUTH_AUTHENTICATING;
    }

    EndpointState GetEpState(void) { return m_epState; }

    void SetEpFailed(void)
    {
        m_epState = EP_FAILED;
    }

    void SetEpStarted(void)
    {
        m_epState = EP_STARTED;
    }

    void SetEpStopping(void)
    {
        assert(m_epState == EP_STARTED);
        m_epState = EP_STOPPING;
    }

    void SetEpDone(void)
    {
        assert(m_epState == EP_FAILED || m_epState == EP_STOPPING);
        m_epState = EP_DONE;
    }

    bool IsSuddenDisconnect() { return m_wasSuddenDisconnect; }
    void SetSuddenDisconnect(bool val) { m_wasSuddenDisconnect = val; }

    QStatus SetLinkTimeout(uint32_t& linkTimeout)
    {
        QStatus status = ER_OK;
        if (linkTimeout > 0) {
            uint32_t to = max(linkTimeout, WFD_LINK_TIMEOUT_MIN_LINK_TIMEOUT);
            to -= WFD_LINK_TIMEOUT_PROBE_RESPONSE_DELAY * WFD_LINK_TIMEOUT_PROBE_ATTEMPTS;
            status = RemoteEndpoint::SetLinkTimeout(to, WFD_LINK_TIMEOUT_PROBE_RESPONSE_DELAY, WFD_LINK_TIMEOUT_PROBE_ATTEMPTS);
            if ((status == ER_OK) && (to > 0)) {
                linkTimeout = to + WFD_LINK_TIMEOUT_PROBE_RESPONSE_DELAY * WFD_LINK_TIMEOUT_PROBE_ATTEMPTS;
            }

        } else {
            RemoteEndpoint::SetLinkTimeout(0, 0, 0);
        }
        return status;
    }

    /*
     * Return true if the auth thread is STARTED, RUNNING or STOPPING.  A true
     * response means the authentication thread is in a state that indicates
     * a possibility it might touch the endpoint data structure.  This means
     * don't delete the endpoint if this method returns true.  This method
     * indicates nothing about endpoint rx and tx thread state.
     */
    bool IsAuthThreadRunning(void)
    {
        return m_authThread.IsRunning();
    }

  private:
    class AuthThread : public qcc::Thread {
      public:
        AuthThread(WFDEndpoint* conn, WFDTransport* trans) : Thread("auth"), m_transport(trans) { }
      private:
        virtual qcc::ThreadReturn STDCALL Run(void* arg);

        WFDTransport* m_transport;
    };

    WFDTransport* m_transport;  /**< The server holding the connection */
    volatile SideState m_sideState;   /**< Is this an active or passive connection */
    volatile AuthState m_authState;   /**< The state of the endpoint authentication process */
    volatile EndpointState m_epState; /**< The state of the endpoint authentication process */
    qcc::Timespec m_tStart;           /**< Timestamp indicating when the authentication process started */
    AuthThread m_authThread;          /**< Thread used to do blocking calls during startup */
    qcc::SocketStream m_stream;       /**< Stream used by authentication code */
    qcc::IPAddress m_ipAddr;          /**< Remote IP address. */
    uint16_t m_port;                  /**< Remote port. */
    bool m_wasSuddenDisconnect;       /**< If true, assumption is that any disconnect is unexpected due to lower level error */
};

QStatus WFDEndpoint::Authenticate(void)
{
    QCC_DbgTrace(("WFDEndpoint::Authenticate()"));
    /*
     * Start the authentication thread.
     */
    QStatus status = m_authThread.Start(this);
    if (status != ER_OK) {
        m_authState = AUTH_FAILED;
    }
    return status;
}

void WFDEndpoint::AuthStop(void)
{
    QCC_DbgTrace(("WFDEndpoint::AuthStop()"));

    /*
     * Ask the auth thread to stop executing.  The only ways out of the thread
     * run function will set the state to either AUTH_SUCCEEDED or AUTH_FAILED.
     * There is a very small chance that we will send a stop to the thread after
     * it has successfully authenticated, but we expect that this will result in
     * an AUTH_FAILED state for the vast majority of cases.  In this case, we
     * notice that the thread failed the next time through the main server run
     * loop, join the thread via AuthJoin below and delete the endpoint.  Note
     * that this is a lazy cleanup of the endpoint.
     */
    m_authThread.Stop();
}

void WFDEndpoint::AuthJoin(void)
{
    QCC_DbgTrace(("WFDEndpoint::AuthJoin()"));

    /*
     * Join the auth thread to stop executing.  All threads must be joined in
     * order to communicate their return status.  The auth thread is no exception.
     * This is done in a lazy fashion from the main server accept loop, where we
     * cleanup every time through the loop.
     */
    m_authThread.Join();
}

void* WFDEndpoint::AuthThread::Run(void* arg)
{
    QCC_DbgTrace(("WFDEndpoint::AuthThread::Run()"));

    WFDEndpoint* conn = reinterpret_cast<WFDEndpoint*>(arg);

    conn->m_authState = AUTH_AUTHENTICATING;

    /*
     * We're running an authentication process here and we are cooperating with
     * the main server thread.  This thread is running in an object that is
     * allocated on the heap, and the server is managing these objects so we
     * need to coordinate getting all of this cleaned up.
     *
     * There is a state variable that only we write.  The server thread only
     * reads this variable, so there are no data sharing issues.  If there is an
     * authentication failure, this thread sets that state variable to
     * AUTH_FAILED and then exits.  The server holds a list of currently
     * authenticating connections and will look for AUTH_FAILED connections when
     * it runs its Accept loop.  If it finds one, it will AuthJoin() this
     * thread.  Since we set AUTH_FAILED immediately before exiting, there will
     * be no problem having the server block waiting for the Join() to complete.
     * We fail authentication here and let the server clean up after us, lazily.
     *
     * If we succeed in the authentication process, we set the state variable
     * to AUTH_SUCEEDED and then call back into the server telling it that we are
     * up and running.  It needs to take us off of the list of authenticating
     * connections and put us on the list of running connections.  This thread
     * will quickly go away and will be replaced by the RX and TX threads of
     * the running RemoteEndpoint.
     *
     * If we are running an authentication process, we are probably ultimately
     * blocked on a socket.  We expect that if the server is asked to shut
     * down, it will run through its list of authenticating connections and
     * AuthStop() each one.  That will cause a thread Stop() which should unblock
     * all of the reads and return an error which will eventually pop out here
     * with an authentication failure.
     *
     * Finally, if the server decides we've spent too much time here and we are
     * actually a denial of service attack, it can close us down by doing an
     * AuthStop() on the authenticating endpoint.  This will do a thread Stop()
     * on the auth thread of the endpoint which will pop out of here as an
     * authentication failure as well.  The only ways out of this method must be
     * with state = AUTH_FAILED or state = AUTH_SUCCEEDED.
     */
    uint8_t byte;
    size_t nbytes;

    /*
     * Eat the first byte of the stream.  This is required to be zero by the
     * DBus protocol.  It is used in the Unix socket implementation to carry
     * out-of-band capabilities, but is discarded here.  We do this here since
     * it involves a read that can block.
     */
    QStatus status = conn->m_stream.PullBytes(&byte, 1, nbytes);
    if ((status != ER_OK) || (nbytes != 1) || (byte != 0)) {
        conn->m_stream.Close();
        QCC_LogError(status, ("Failed to read first byte from stream"));

        /*
         * Management of the resources used by the authentication thread is done
         * in one place, by the server Accept loop.  The authentication thread
         * writes its state into the connection and the server Accept loop reads
         * this state.  As soon as we set this state to AUTH_FAILED, we are
         * telling the Accept loop that we are done with the conn data
         * structure.  That thread is then free to do anything it wants with the
         * connection, including deleting it, so we are not allowed to touch
         * conn after setting this state.
         *
         * In addition to releasing responsibility for the conn data structure,
         * when we set the state to AUTH_SUCCEEDED we are telling the server
         * accept loop that we are exiting now and so it can Join() on us (the
         * authentication thread) without being worried about blocking since the
         * next thing we do is exit.
         */
        conn->m_authState = AUTH_FAILED;
        return (void*)ER_FAIL;
    }

    /* Initialized the features for this endpoint */
    conn->GetFeatures().isBusToBus = false;
    conn->GetFeatures().isBusToBus = false;
    conn->GetFeatures().handlePassing = false;

    /* Run the actual connection authentication code. */
    qcc::String authName;
    qcc::String redirection;
    status = conn->Establish("ANONYMOUS", authName, redirection);
    if (status != ER_OK) {
        conn->m_stream.Close();
        QCC_LogError(status, ("Failed to establish WFD endpoint"));

        /*
         * Management of the resources used by the authentication thread is done
         * in one place, by the server Accept loop.  The authentication thread
         * writes its state into the connection and the server Accept loop reads
         * this state.  As soon as we set this state to AUTH_FAILED, we are
         * telling the Accept loop that we are done with the conn data
         * structure.  That thread is then free to do anything it wants with the
         * connection, including deleting it, so we are not allowed to touch
         * conn after setting this state.
         *
         * In addition to releasing responsibility for the conn data structure,
         * when we set the state to AUTH_SUCCEEDED we are telling the server
         * accept loop that we are exiting now and so it can Join() on us (the
         * authentication thread) without being worried about blocking since the
         * next thing we do is exit.
         */
        conn->m_authState = AUTH_FAILED;
        return (void*)status;
    }

    /*
     * Tell the transport that the authentication has succeeded and that it can
     * now bring the connection up.
     */
    conn->m_transport->Authenticated(conn);

    QCC_DbgTrace(("WFDEndpoint::AuthThread::Run(): Returning"));

    /*
     * We are now done with the authentication process.  We have succeeded doing
     * the authentication and we may or may not have succeeded in starting the
     * endpoint TX and RX threads depending on what happened down in
     * Authenticated().  What concerns us here is that we are done with this
     * thread (the authentication thread) and we are about to exit.  Before
     * exiting, we must tell server accept loop that we are done with this data
     * structure.  As soon as we set this state to AUTH_SUCCEEDED that thread is
     * then free to do anything it wants with the connection, including deleting
     * it, so we are not allowed to touch conn after setting this state.
     *
     * In addition to releasing responsibility for the conn data structure, when
     * we set the state to AUTH_SUCCEEDED we are telling the server accept loop
     * that we are exiting now and so it can Join() the authentication thread
     * without being worried about blocking since the next thing we do is exit.
     */
    conn->m_authState = AUTH_SUCCEEDED;
    return (void*)status;
}

WFDTransport::WFDTransport(BusAttachment& bus)
    : Thread("WFDTransport"), m_bus(bus), m_stopping(false), m_listener(0),
    m_foundCallback(this, m_listener),
    m_isAdvertising(false), m_isDiscovering(false), m_isListening(false), m_isNsEnabled(false), m_listenPort(0),
    m_p2pHelperInterface(0), m_myP2pHelperListener(0), m_goHandle(-1)
{
    QCC_DbgTrace(("WFDTransport::WFDTransport()"));
    /*
     * We know we are daemon code, so we'd better be running with a daemon
     * router.  This is assumed elsewhere.
     */
    assert(m_bus.GetInternal().GetRouter().IsDaemon());
}

WFDTransport::~WFDTransport()
{
    QCC_DbgTrace(("WFDTransport::~WFDTransport()"));
    Stop();
    Join();

    QCC_DbgTrace(("WFDTransport::~WFDTransport(): deleting m_p2pHelperInterface"));

    delete m_p2pHelperInterface;
    m_p2pHelperInterface = NULL;

    QCC_DbgTrace(("WFDTransport::~WFDTransport(): deleting m_p2pHelperListener"));

    delete m_myP2pHelperListener;
    m_myP2pHelperListener = NULL;
}

void WFDTransport::Authenticated(WFDEndpoint* conn)
{
    QCC_DbgTrace(("WFDTransport::Authenticated()"));

    /*
     * If Authenticated() is being called, it is as a result of the
     * authentication thread telling us that it has succeeded.  What we need to
     * do here is to try and Start() the endpoint which will spin up its TX and
     * RX threads and register the endpoint with the daemon router.  As soon as
     * we call Start(), we are transferring responsibility for error reporting
     * through endpoint ThreadExit() function.  This will percolate out our
     * EndpointExit function.  It will expect to find <conn> on the endpoint
     * list so we move it from the authList to the endpointList before calling
     * Start.
     */
    m_endpointListLock.Lock(MUTEX_CONTEXT);

    set<WFDEndpoint*>::iterator i = find(m_authList.begin(), m_authList.end(), conn);
    assert(i != m_authList.end() && "WFDTransport::Authenticated(): Conn not on m_authList");

    /*
     * Note here that we have not yet marked the authState as AUTH_SUCCEEDED so
     * this is a point in time where the authState can be AUTH_AUTHENTICATING
     * and the endpoint can be on the endpointList and not the authList.
     */
    m_authList.erase(i);
    m_endpointList.insert(conn);

    m_endpointListLock.Unlock(MUTEX_CONTEXT);

    conn->SetListener(this);
    QStatus status = conn->Start();
    if (status != ER_OK) {
        QCC_LogError(status, ("WFDTransport::Authenticated(): Failed to start WFD endpoint"));
        /*
         * We were unable to start up the endpoint for some reason.  As soon as
         * we set this state to EP_FAILED, we are telling the server accept loop
         * that we tried to start the connection but it failed.  This connection
         * is now useless and is a candidate for cleanup.  This will be
         * prevented until authState changes from AUTH_AUTHENTICATING to
         * AUTH_SUCCEEDED.  This may be a little confusing, but the
         * authentication process has really succeeded but the endpoint start
         * has failed.  The combination of status in this case will be
         * AUTH_SUCCEEDED and EP_FAILED.  Once this state is detected by the
         * server accept loop it is then free to do anything it wants with the
         * connection, including deleting it.
         */
        conn->SetEpFailed();
    } else {
        /*
         * We were able to successfully start up the endpoint.  As soon as we
         * set this state to EP_STARTED, we are telling the server accept loop
         * that there are TX and RX threads wandering around in this endpoint.
         */
        conn->SetEpStarted();
    }
}

QStatus WFDTransport::Start()
{
    /*
     * We rely on the status of the server accept thead as the primary
     * gatekeeper.
     *
     * A true response from IsRunning tells us that the server accept thread is
     * STARTED, RUNNING or STOPPING.
     *
     * When a thread is created it is in state INITIAL.  When an actual tread is
     * spun up as a result of Start(), it becomes STARTED.  Just before the
     * user's Run method is called, the thread becomes RUNNING.  If the Run
     * method exits, the thread becomes STOPPING.  When the thread is Join()ed
     * it becomes DEAD.
     *
     * IsRunning means that someone has called Thread::Start() and the process
     * has progressed enough that the thread has begun to execute.  If we get
     * multiple Start() calls calls on multiple threads, this test may fail to
     * detect multiple starts in a failsafe way and we may end up with multiple
     * server accept threads running.  We assume that since Start() requests
     * come in from our containing transport list it will not allow concurrent
     * start requests.
     */
    if (IsRunning()) {
        QCC_LogError(ER_BUS_BUS_ALREADY_STARTED, ("WFDTransport::Start(): Already started"));
        return ER_BUS_BUS_ALREADY_STARTED;
    }

    m_stopping = false;

    /*
     * Get the guid from the bus attachment which will act as the globally unique
     * ID of the daemon.
     */
    qcc::String guidStr = m_bus.GetInternal().GetGlobalGUID().ToString();

    /*
     * We're a WFD transport, and WFD is an IP protocol, so we want to use the IP
     * name service for our advertisement and discovery work.  When we acquire
     * the name service, we are basically bumping a reference count and starting
     * it if required.
     */
    IpNameService::Instance().Acquire(guidStr);

    /*
     * Tell the name service to call us back on our FoundCallback method when
     * we hear about a new well-known bus name.
     */
    IpNameService::Instance().SetCallback(TRANSPORT_WFD,
                                          new CallbackImpl<FoundCallback, void, const qcc::String&, const qcc::String&, std::vector<qcc::String>&, uint8_t>
                                              (&m_foundCallback, &FoundCallback::Found));

    /*
     * Start the server accept loop through the thread base class.  This will
     * close or open the IsRunning() gate we use to control access to our
     * public API.
     */
    return Thread::Start();
}

QStatus WFDTransport::Stop(void)
{
    QCC_DbgTrace(("WFDTransport::Stop()"));

    /*
     * It is legal to call Stop() more than once, so it must be possible to
     * call Stop() on a stopped transport.
     */
    m_stopping = true;

    /*
     * Tell the name service to stop calling us back if it's there (we may get
     * called more than once in the chain of destruction) so the pointer is not
     * required to be non-NULL.
     */
    IpNameService::Instance().SetCallback(TRANSPORT_WFD, NULL);

    /*
     * Tell the server accept loop thread to shut down through the thead
     * base class.
     */
    QStatus status = Thread::Stop();
    if (status != ER_OK) {
        QCC_LogError(status, ("WFDTransport::Stop(): Failed to Stop() server thread"));
        return status;
    }

    m_endpointListLock.Lock(MUTEX_CONTEXT);

    /*
     * Ask any authenticating ACTIVE endpoints to shut down and return to the
     * caller.  By its presence on the m_activeEndpointsThreadList, we know that
     * an external (from the point of this module) thread is authenticating and
     * is probably blocked waiting for the other side to respond.  We can't call
     * Stop() to stop that thread from running, we have to Alert() it to make it
     * pop out of its blocking calls.
     */
    for (set<Thread*>::iterator i = m_activeEndpointsThreadList.begin(); i != m_activeEndpointsThreadList.end(); ++i) {
        (*i)->Alert();
    }

    /*
     * Ask any authenticating endpoints to shut down and exit their threads.  By its
     * presence on the m_authList, we know that the endpoint is authenticating and
     * the authentication thread has responsibility for dealing with the endpoint
     * data structure.  We call Stop() to stop that thread from running.  The
     * endpoint Rx and Tx threads will not be running yet.
     */
    for (set<WFDEndpoint*>::iterator i = m_authList.begin(); i != m_authList.end(); ++i) {
        (*i)->AuthStop();
    }

    /*
     * Ask any running endpoints to shut down and exit their threads.  By its
     * presence on the m_endpointList, we know that authentication is compete and
     * the Rx and Tx threads have responsibility for dealing with the endpoint
     * data structure.  We call Stop() to stop those threads from running.  Since
     * the connnection is on the m_endpointList, we know that the authentication
     * thread has handed off responsibility.
     */
    for (set<WFDEndpoint*>::iterator i = m_endpointList.begin(); i != m_endpointList.end(); ++i) {
        (*i)->Stop();
    }

    m_endpointListLock.Unlock(MUTEX_CONTEXT);

    return ER_OK;
}

QStatus WFDTransport::Join(void)
{
    QCC_DbgTrace(("WFDTransport::Join()"));

    /*
     * It is legal to call Join() more than once, so it must be possible to
     * call Join() on a joined transport and also on a joined name service.
     */
    QStatus status = Thread::Join();
    if (status != ER_OK) {
        return status;
    }

    /*
     * Tell the IP name service instance that we will no longer be making calls
     * and it may shut down if we were the last transport.  This release can
     * be thought of as a reference counted Stop()/Join() so it is appropriate
     * to make it here since we are expecting the possibility of blocking.
     */
    IpNameService::Instance().Release();

    /*
     * A required call to Stop() that needs to happen before this Join will ask
     * all of the endpoints to stop; and will also cause any authenticating
     * endpoints to stop.  We still need to wait here until all of the threads
     * running in those endpoints actually stop running.
     *
     * Since Stop() is a request to stop, and this is what has ultimately been
     * done to both authentication threads and Rx and Tx threads, it is possible
     * that a thread is actually running after the call to Stop().  If that
     * thead happens to be an authenticating endpoint, it is possible that an
     * authentication actually completes after Stop() is called.  This will move
     * a connection from the m_authList to the m_endpointList, so we need to
     * make sure we wait for all of the connections on the m_authList to go away
     * before we look for the connections on the m_endpointlist.
     */
    m_endpointListLock.Lock(MUTEX_CONTEXT);

    /*
     * Any authenticating endpoints have been asked to shut down and exit their
     * authentication threads in a previously required Stop().  We need to
     * Join() all of these auth threads here.
     */
    set<WFDEndpoint*>::iterator it = m_authList.begin();
    while (it != m_authList.end()) {
        WFDEndpoint* ep = *it;
        m_authList.erase(it);
        m_endpointListLock.Unlock(MUTEX_CONTEXT);
        ep->AuthJoin();
        delete ep;
        m_endpointListLock.Lock(MUTEX_CONTEXT);
        it = m_authList.upper_bound(ep);
    }


    /*
     * Any running endpoints have been asked it their threads in a previously
     * required Stop().  We need to Join() all of thesse threads here.  This
     * Join() will wait on the endpoint rx and tx threads to exit as opposed to
     * the joining of the auth thread we did above.
     */
    it = m_endpointList.begin();
    while (it != m_endpointList.end()) {
        WFDEndpoint* ep = *it;
        m_endpointList.erase(it);
        m_endpointListLock.Unlock(MUTEX_CONTEXT);
        ep->Join();
        delete ep;
        m_endpointListLock.Lock(MUTEX_CONTEXT);
        it = m_endpointList.upper_bound(ep);
    }

    m_endpointListLock.Unlock(MUTEX_CONTEXT);

    m_stopping = false;
    return ER_OK;
}

/*
 * The default interface for the name service to use.  The wildcard character
 * means to listen and transmit over all interfaces that are up and multicast
 * capable, with any IP address they happen to have.  This default also applies
 * to the search for listen address interfaces.
 */
static const char* INTERFACES_DEFAULT = "*";

QStatus WFDTransport::GetListenAddresses(const SessionOpts& opts, std::vector<qcc::String>& busAddrs) const
{
    QCC_DbgTrace(("WFDTransport::GetListenAddresses()"));

    /*
     * We are given a session options structure that defines the kind of
     * transports that are being sought.  WFD provides reliable traffic as
     * understood by the session options, so we only return someting if
     * the traffic type is TRAFFIC_MESSAGES or TRAFFIC_RAW_RELIABLE.  It's
     * not an error if we don't match, we just don't have anything to offer.
     */
    if (opts.traffic != SessionOpts::TRAFFIC_MESSAGES && opts.traffic != SessionOpts::TRAFFIC_RAW_RELIABLE) {
        QCC_DbgPrintf(("WFDTransport::GetListenAddresses(): traffic mismatch"));
        return ER_OK;
    }

    /*
     * The other session option that we need to filter on is the transport
     * bitfield.  We have no easy way of figuring out if we are a wireless
     * local-area, wireless wide-area, wired local-area or local transport,
     * but we do exist, so we respond if the caller is asking for any of
     * those: cogito ergo some.
     */
    if (!(opts.transports & (TRANSPORT_WLAN | TRANSPORT_WWAN | TRANSPORT_LAN))) {
        QCC_DbgPrintf(("WFDTransport::GetListenAddresses(): transport mismatch"));
        return ER_OK;
    }

    /*
     * The name service is initialized by the call to Init() in our Start()
     * method and then started there.  It is Stop()ped in our Stop() method and
     * joined in our Join().  In the case of a call here, the transport will
     * probably be started, and we will probably find the name service started,
     * but there is no requirement to ensure this.  If m_ns is NULL, we need to
     * complain so the user learns to Start() the transport before calling
     * IfConfig.  A call to IsRunning() here is superfluous since we really
     * don't care about anything but the name service in this method.
     */
    if (IpNameService::Instance().Started() == false) {
        QCC_LogError(ER_BUS_TRANSPORT_NOT_STARTED, ("WFDTransport::GetListenAddresses(): NameService not started"));
        return ER_BUS_TRANSPORT_NOT_STARTED;
    }

    /*
     * Our goal is here is to match a list of interfaces provided in the
     * configuration database (or a wildcard) to a list of interfaces that are
     * IFF_UP in the system.  The first order of business is to get the list of
     * interfaces in the system.  We do that using a convenient OS-inependent
     * call into the name service.
     *
     * We can't cache this list since it may change as the phone wanders in
     * and out of range of this and that and the underlying IP addresses change
     * as DHCP doles out whatever it feels like at any moment.
     */
    QCC_DbgPrintf(("WFDTransport::GetListenAddresses(): IfConfig()"));

    std::vector<qcc::IfConfigEntry> entries;
    QStatus status = qcc::IfConfig(entries);
    if (status != ER_OK) {
        QCC_LogError(status, ("WFDTransport::GetListenAddresses(): ns.IfConfig() failed"));
        return status;
    }

    /*
     * The next thing to do is to get the list of interfaces from the config
     * file.  These are required to be formatted in a comma separated list,
     * with '*' being a wildcard indicating that we want to match any interface.
     * If there is no configuration item, we default to something rational.
     */
    QCC_DbgPrintf(("WFDTransport::GetListenAddresses(): GetProperty()"));
    qcc::String interfaces = DaemonConfig::Access()->Get("ip_name_service/property@interfaces");
    if (interfaces.size() == 0) {
        interfaces = INTERFACES_DEFAULT;
    }

    /*
     * Check for wildcard anywhere in the configuration string.  This trumps
     * anything else that may be there and ensures we get only one copy of
     * the addresses if someone tries to trick us with "*,*".
     */
    bool haveWildcard = false;
    const char*wildcard = "*";
    size_t i = interfaces.find(wildcard);
    if (i != qcc::String::npos) {
        QCC_DbgPrintf(("WFDTransport::GetListenAddresses(): wildcard search"));
        haveWildcard = true;
        interfaces = wildcard;
    }

    /*
     * Walk the comma separated list from the configuration file and and try
     * to mach it up with interfaces actually found in the system.
     */
    while (interfaces.size()) {
        /*
         * We got a comma-separated list, so we need to work our way through
         * the list.  Each entry in the list  may be  an interface name, or a
         * wildcard.
         */
        qcc::String currentInterface;
        size_t i = interfaces.find(",");
        if (i != qcc::String::npos) {
            currentInterface = interfaces.substr(0, i);
            interfaces = interfaces.substr(i + 1, interfaces.size() - i - 1);
        } else {
            currentInterface = interfaces;
            interfaces.clear();
        }

        QCC_DbgPrintf(("WFDTransport::GetListenAddresses(): looking for interface %s", currentInterface.c_str()));

        /*
         * Walk the list of interfaces that we got from the system and see if
         * we find a match.
         */
        for (uint32_t i = 0; i < entries.size(); ++i) {
            QCC_DbgPrintf(("WFDTransport::GetListenAddresses(): matching %s", entries[i].m_name.c_str()));
            /*
             * To match a configuration entry, the name of the interface must:
             *
             *   - match the name in the currentInterface (or be wildcarded);
             *   - be UP which means it has an IP address assigned;
             *   - not be the LOOPBACK device and therefore be remotely available.
             */
            uint32_t mask = qcc::IfConfigEntry::UP | qcc::IfConfigEntry::LOOPBACK;

            uint32_t state = qcc::IfConfigEntry::UP;

            if ((entries[i].m_flags & mask) == state) {
                QCC_DbgPrintf(("WFDTransport::GetListenAddresses(): %s has correct state", entries[i].m_name.c_str()));
                if (haveWildcard || entries[i].m_name == currentInterface) {
                    QCC_DbgPrintf(("WFDTransport::GetListenAddresses(): %s has correct name", entries[i].m_name.c_str()));
                    /*
                     * This entry matches our search criteria, so we need to
                     * turn the IP address that we found into a busAddr.  We
                     * must be a WFD transport, and we have an IP address
                     * already in a string, so we can easily put together the
                     * desired busAddr.
                     */
                    QCC_DbgTrace(("WFDTransport::GetListenAddresses(): %s match found", entries[i].m_name.c_str()));
                    /*
                     * We know we have an interface that speaks IP and
                     * which has an IP address we can pass back. We know
                     * it is capable of receiving incoming connections, but
                     * the $64,000 questions are, does it have a listener
                     * and what port is that listener listening on.
                     *
                     * There is one name service associated with the daemon
                     * WFD transport, and it is advertising at most one port.
                     * It may be advertising that port over multiple
                     * interfaces, but there is currently just one port being
                     * advertised.  If multiple listeners are created, the
                     * name service only advertises the lastly set port.  In
                     * the future we may need to add the ability to advertise
                     * different ports on different interfaces, but the answer
                     * is simple now.  Ask the name service for the one port
                     * it is advertising and that must be the answer.
                     */
                    qcc::String ipv4address;
                    qcc::String ipv6address;
                    uint16_t reliableIpv4Port, reliableIpv6Port, unreliableIpv4Port, unreliableIpv6port;
                    IpNameService::Instance().Enabled(TRANSPORT_WFD,
                                                      reliableIpv4Port, reliableIpv6Port,
                                                      unreliableIpv4Port, unreliableIpv6port);
                    /*
                     * If the port is zero, then it hasn't been set and this
                     * implies that WFDTransport::StartListen hasn't
                     * been called and there is no listener for this transport.
                     * We should only return an address if we have a listener.
                     */
                    if (reliableIpv4Port) {
                        /*
                         * Now put this information together into a bus address
                         * that the rest of the AllJoyn world can understand.
                         * (Note: only IPv4 "reliable" addresses are supported
                         * at this time.)
                         */
                        if (!entries[i].m_addr.empty() && (entries[i].m_family == QCC_AF_INET)) {
                            qcc::String busAddr = "wfd:r4addr=" + entries[i].m_addr + ","
                                                  "r4port=" + U32ToString(reliableIpv4Port) + ","
                                                  "family=ipv4";
                            busAddrs.push_back(busAddr);
                        }
                    }
                }
            }
        }
    }

    /*
     * If we can get the list and walk it, we have succeeded.  It is not an
     * error to have no available interfaces.  In fact, it is quite expected
     * in a phone if it is not associated with an access point over wi-fi.
     */
    QCC_DbgPrintf(("WFDTransport::GetListenAddresses(): done"));
    return ER_OK;
}

void WFDTransport::EndpointExit(RemoteEndpoint* ep)
{
    /*
     * This is a callback driven from the remote endpoint thread exit function.
     * Our WFDEndpoint inherits from class RemoteEndpoint and so when
     * either of the threads (transmit or receive) of one of our endpoints exits
     * for some reason, we get called back here.  We only get called if either
     * the tx or rx thread exits, which implies that they have been run.  It
     * turns out that in the case of an endpoint receiving a connection, it
     * means that authentication has succeeded.  In the case of an endpoint
     * doing the connect, the EndpointExit may have resulted from an
     * authentication error since authentication is done in the context of the
     * Connect()ing thread and may be reported through EndpointExit.
     */
    QCC_DbgTrace(("WFDTransport::EndpointExit()"));

    WFDEndpoint* tep = static_cast<WFDEndpoint*>(ep);
    assert(tep);

    /*
     * The endpoint can exit if it was asked to by us in response to a
     * Disconnect() from higher level code, or if it got an error from the
     * underlying transport.  We need to notify upper level code if the
     * disconnect is due to an event from the transport.
     */
    if (m_listener && tep->IsSuddenDisconnect()) {
        m_listener->BusConnectionLost(tep->GetConnectSpec());
    }

    /*
     * If this is an active connection, what has happened is that the reference
     * count on the underlying RemoteEndpoint has been decremented to zero and
     * the Stop() function of the endpoint has been called.  This means that
     * we are done with the endpoint and it should be cleaned up.  Marking
     * the connection as active prevented the passive side cleanup, so we need
     * to deal with cleanup now.
     */
    tep->SetPassive();

    /*
     * Mark the endpoint as no longer running.  Since we are called from
     * the RemoteEndpoint ThreadExit routine, we know it has stopped both
     * the RX and TX threads and we can Join them in a timely manner.
     */
    tep->SetEpStopping();

    /*
     * If this endpoint was running over a Wi-Fi P2P interface, we have to
     * tear that link down "manually."
     */
    QCC_DbgPrintf(("WFDTransport::EndpointExit(): Check for P2P endpoint exiting"));
    P2PConnectionInfo* info = GetP2PInfoForEndpoint(tep);
    if (info) {
        QCC_DbgPrintf(("WFDTransport::EndpointExit(): P2P endpoint is exiting"));
        /*
         * This is ugly, but since there can be only one P2P link, we are just
         * using the m_goHandle variable.
         */
        QCC_DbgPrintf(("WFDTransport::EndpointExit(): ReleaseLinkAsync()"));
        m_p2pHelperInterface->ReleaseLinkAsync(m_goHandle);
        m_goHandle = -1;

#if 0
        /*
         * We don't currently have any way to know if this was a GO or STA or
         * what kind of advertisements or discovery operations are happening
         * but the least we can turn off the interface.
         */
        qcc::String interface = info->GetInterface();
        QCC_DbgPrintf(("WFDTransport::EndpointExit(): CloseInterface()"));
        QStatus status = IpNameService::Instance().CloseInterface(TRANSPORT_WFD, interface);
        if (status != ER_OK) {
            QCC_LogError(status, ("WFDTransport::Disconnect(): Cannot close interface \"%s\"", interface.c_str()));
        }
#endif
    }

    /*
     * Wake up the server accept loop so that it deals with our passing immediately.
     */
    Alert();
}

void WFDTransport::ManageEndpoints(Timespec tTimeout)
{
    m_endpointListLock.Lock(MUTEX_CONTEXT);

    /*
     * Run through the list of connections on the authList and cleanup
     * any that are no longer running or are taking too long to authenticate
     * (we assume a denial of service attack in this case).
     */
    set<WFDEndpoint*>::iterator i = m_authList.begin();
    while (i != m_authList.end()) {
        WFDEndpoint* ep = *i;
        WFDEndpoint::AuthState authState = ep->GetAuthState();

        if (authState == WFDEndpoint::AUTH_FAILED) {
            /*
             * The endpoint has failed authentication and the auth thread is
             * gone or is going away.  Since it has failed there is no way this
             * endpoint is going to be started so we can get rid of it as soon
             * as we Join() the (failed) authentication thread.
             */
            QCC_DbgHLPrintf(("WFDTransport::ManageEndpoints(): Scavenging failed authenticator"));
            m_authList.erase(i);
            m_endpointListLock.Unlock(MUTEX_CONTEXT);
            ep->AuthJoin();
            delete ep;
            m_endpointListLock.Lock(MUTEX_CONTEXT);
            i = m_authList.upper_bound(ep);
            continue;
        }

        Timespec tNow;
        GetTimeNow(&tNow);

        if (ep->GetStartTime() + tTimeout < tNow) {
            /*
             * This endpoint is taking too long to authenticate.  Stop the
             * authentication process.  The auth thread is still running, so we
             * can't just delete the connection, we need to let it stop in its
             * own time.  What that thread will do is to set AUTH_FAILED and
             * exit.  we will then clean it up the next time through this loop.
             * In the hope that the thread can exit and we can catch its exit
             * here and now, we take our thread off the OS ready list (Sleep)
             * and let the other thread run before looping back.
             */
            QCC_DbgHLPrintf(("WFDTransport::ManageEndpoints(): Scavenging slow authenticator"));
            ep->AuthStop();
            qcc::Sleep(1);
        }
        ++i;
    }

    /*
     * We've handled the authList, so now run through the list of connections on
     * the endpointList and cleanup any that are no longer running or Join()
     * authentication threads that have successfully completed.
     */
    i = m_endpointList.begin();
    while (i != m_endpointList.end()) {
        WFDEndpoint* ep = *i;

        /*
         * We are only managing passive connections here, or active connections
         * that are done and are explicitly ready to be cleaned up.
         */
        WFDEndpoint::SideState sideState = ep->GetSideState();
        if (sideState == WFDEndpoint::SIDE_ACTIVE) {
            ++i;
            continue;
        }

        WFDEndpoint::AuthState authState = ep->GetAuthState();
        WFDEndpoint::EndpointState endpointState = ep->GetEpState();

        if (authState == WFDEndpoint::AUTH_SUCCEEDED) {
            /*
             * The endpoint has succeeded authentication and the auth thread is
             * gone or is going away.  Take this opportunity to join the auth
             * thread.  Since the auth thread promised not to touch the state
             * after setting AUTH_SUCCEEEDED, we can safely change the state
             * here since we now own the conn.  We do this through a method call
             * to enable this single special case where we are allowed to set
             * the state.
             */
            QCC_DbgHLPrintf(("WFDTransport::ManageEndpoints(): Scavenging failed authenticator"));
            m_endpointListLock.Unlock(MUTEX_CONTEXT);
            ep->AuthJoin();
            ep->SetAuthDone();
            m_endpointListLock.Lock(MUTEX_CONTEXT);
            i = m_endpointList.upper_bound(ep);
            continue;
        }

        /*
         * There are two possibilities for the disposition of the RX and
         * TX threads.  First, they were never successfully started.  In
         * this case, the epState will be EP_FAILED.  If we find this, we
         * can just remove the useless endpoint from the list and delete
         * it.  Since the threads were never started, they must not be
         * joined.
         */
        if (endpointState == WFDEndpoint::EP_FAILED) {
            m_endpointList.erase(i);
            m_endpointListLock.Unlock(MUTEX_CONTEXT);
            delete ep;
            m_endpointListLock.Lock(MUTEX_CONTEXT);
            i = m_endpointList.upper_bound(ep);
            continue;
        }

        /*
         * The second possibility for the disposition of the RX and
         * TX threads is that they were successfully started but
         * have been stopped for some reason, either because of a
         * Disconnect() or a network error.  In this case, the
         * epState will be EP_STOPPING, which was set in the
         * EndpointExit function.  If we find this, we need to Join
         * the endpoint threads, remove the endpoint from the
         * endpoint list and delete it.  Note that we are calling
         * the endpoint Join() to join the TX and RX threads and not
         * the endpoint AuthJoin() to join the auth thread.
         */
        if (endpointState == WFDEndpoint::EP_STOPPING) {
            m_endpointList.erase(i);
            m_endpointListLock.Unlock(MUTEX_CONTEXT);
            ep->Join();
            delete ep;
            m_endpointListLock.Lock(MUTEX_CONTEXT);
            i = m_endpointList.upper_bound(ep);
            continue;
        }
        ++i;
    }
    m_endpointListLock.Unlock(MUTEX_CONTEXT);
}

void* WFDTransport::Run(void* arg)
{
    QCC_DbgTrace(("WFDTransport::Run()"));

    /*
     * We need to find the defaults for our connection limits.  These limits
     * can be specified in the configuration database with corresponding limits
     * used for DBus.  If any of those are present, we use them, otherwise we
     * provide some hopefully reasonable defaults.
     */
    DaemonConfig* config = DaemonConfig::Access();

    /*
     * tTimeout is the maximum amount of time we allow incoming connections to
     * mess about while they should be authenticating.  If they take longer
     * than this time, we feel free to disconnect them as deniers of service.
     */
    Timespec tTimeout = config->Get("limit@auth_timeout", ALLJOYN_AUTH_TIMEOUT_DEFAULT);

    /*
     * maxAuth is the maximum number of incoming connections that can be in
     * the process of authenticating.  If starting to authenticate a new
     * connection would mean exceeding this number, we drop the new connection.
     */
    uint32_t maxAuth = config->Get("limit@max_incomplete_connections", ALLJOYN_MAX_INCOMPLETE_CONNECTIONS_WFD_DEFAULT);

    /*
     * maxConn is the maximum number of active connections possible over the
     * WFD transport.  If starting to process a new connection would mean
     * exceeding this number, we drop the new connection.
     */
    uint32_t maxConn = config->Get("limit@max_completed_connections", ALLJOYN_MAX_COMPLETED_CONNECTIONS_WFD_DEFAULT);

    QStatus status = ER_OK;

    while (!IsStopping()) {

        /*
         * We did an Acquire on the name service in our Start() method which
         * ultimately caused this thread to run.  If we were the first transport
         * to Acquire() the name service, it will have done a Start() to crank
         * up its own run thread.  Just because we did that Start() before we
         * did our Start(), it does not necessarily mean that thread will come
         * up and run before us.  If we happen to come up before our name service
         * we'll hang around until it starts to run.  After all, nobody is going
         * to attempt to connect until we advertise something, and we need the
         * name service to advertise.
         */
        if (IpNameService::Instance().Started() == false) {
            QCC_DbgTrace(("WFDTransport::Run(): Wait for IP name service"));
            qcc::Sleep(1);
            continue;
        }

        /*
         * Each time through the loop we create a set of events to wait on.
         * We need to wait on the stop event and all of the SocketFds of the
         * addresses and ports we are listening on.  If the list changes, the
         * code that does the change Alert()s this thread and we wake up and
         * re-evaluate the list of SocketFds.
         */
        m_listenFdsLock.Lock(MUTEX_CONTEXT);
        vector<Event*> checkEvents, signaledEvents;
        checkEvents.push_back(&stopEvent);
        for (list<pair<qcc::String, SocketFd> >::const_iterator i = m_listenFds.begin(); i != m_listenFds.end(); ++i) {
            checkEvents.push_back(new Event(i->second, Event::IO_READ, false));
        }
        m_listenFdsLock.Unlock(MUTEX_CONTEXT);

        /*
         * We have our list of events, so now wait for something to happen
         * on that list (or get alerted).
         */
        signaledEvents.clear();

        status = Event::Wait(checkEvents, signaledEvents);
        if (ER_OK != status) {
            QCC_LogError(status, ("Event::Wait failed"));
            break;
        }

        /*
         * We're back from our Wait() so one of three things has happened.  Our
         * thread has been asked to Stop(), our thread has been Alert()ed, or
         * one of the socketFds we are listening on for connecte events has
         * becomed signalled.
         *
         * If we have been asked to Stop(), or our thread has been Alert()ed,
         * the stopEvent will be on the list of signalled events.  The
         * difference can be found by a call to IsStopping() which is found
         * above.  An alert means that a request to start or stop listening
         * on a given address and port has been queued up for us.
         */
        for (vector<Event*>::iterator i = signaledEvents.begin(); i != signaledEvents.end(); ++i) {
            /*
             * In order to rationalize management of resources, we manage the
             * various lists in one place on one thread.  This thread is a
             * convenient victim, so we do it here.
             */
            ManageEndpoints(tTimeout);

            /*
             * Reset an existing Alert() or Stop().  If it's an alert, we
             * will deal with looking for the incoming listen requests at
             * the bottom of the server loop.  If it's a stop we will
             * exit the next time through the top of the server loop.
             */
            if (*i == &stopEvent) {
                stopEvent.ResetEvent();
                continue;
            }

            /*
             * Since the current event is not the stop event, it must reflect at
             * least one of the SocketFds we are waiting on for incoming
             * connections.  Go ahead and Accept() the new connection on the
             * current SocketFd.
             */
            IPAddress remoteAddr;
            uint16_t remotePort;
            SocketFd newSock;

            while (true) {
                status = Accept((*i)->GetFD(), remoteAddr, remotePort, newSock);
                if (status != ER_OK) {
                    break;
                }

                QCC_DbgHLPrintf(("WFDTransport::Run(): Accepting connection newSock=%d", newSock));

                QCC_DbgPrintf(("WFDTransport::Run(): maxAuth == %d", maxAuth));
                QCC_DbgPrintf(("WFDTransport::Run(): maxConn == %d", maxConn));
                QCC_DbgPrintf(("WFDTransport::Run(): mAuthList.size() == %d", m_authList.size()));
                QCC_DbgPrintf(("WFDTransport::Run(): mEndpointList.size() == %d", m_endpointList.size()));
                assert(m_authList.size() + m_endpointList.size() <= maxConn);

                /*
                 * Do we have a slot available for a new connection?  If so, use
                 * it.
                 */
                m_endpointListLock.Lock(MUTEX_CONTEXT);
                if ((m_authList.size() < maxAuth) && (m_authList.size() + m_endpointList.size() < maxConn)) {
                    WFDEndpoint* conn = new WFDEndpoint(this, m_bus, true, "", newSock, remoteAddr, remotePort);
                    conn->SetPassive();
                    Timespec tNow;
                    GetTimeNow(&tNow);
                    conn->SetStartTime(tNow);
                    /*
                     * By putting the connection on the m_authList, we are
                     * transferring responsibility for the connection to the
                     * Authentication thread.  Therefore, we must check that the
                     * thread actually started running to ensure the handoff
                     * worked.  If it didn't we need to deal with the connection
                     * here.  Since there are no threads running we can just
                     * pitch the connection.
                     */
                    std::pair<std::set<WFDEndpoint*>::iterator, bool> ins = m_authList.insert(conn);
                    status = conn->Authenticate();
                    if (status != ER_OK) {
                        m_authList.erase(ins.first);
                        delete conn;
                    }
                    conn = NULL;
                    m_endpointListLock.Unlock(MUTEX_CONTEXT);
                } else {
                    m_endpointListLock.Unlock(MUTEX_CONTEXT);
                    qcc::Shutdown(newSock);
                    qcc::Close(newSock);
                    status = ER_AUTH_FAIL;
                    QCC_LogError(status, ("WFDTransport::Run(): No slot for new connection"));
                }
            }

            /*
             * Accept returns ER_WOULDBLOCK when all of the incoming connections have been handled
             */
            if (ER_WOULDBLOCK == status) {
                status = ER_OK;
            }

            if (status != ER_OK) {
                QCC_LogError(status, ("WFDTransport::Run(): Error accepting new connection. Ignoring..."));
            }
        }

        /*
         * We're going to loop back and create a new list of checkEvents that
         * reflect the current state, so we need to delete the checkEvents we
         * created on this iteration.
         */
        for (vector<Event*>::iterator i = checkEvents.begin(); i != checkEvents.end(); ++i) {
            if (*i != &stopEvent) {
                delete *i;
            }
        }

        /*
         * If we're not stopping, we always check for queued requests to start
         * and stop listening on address and port combinations (listen specs).
         * We need to change the state of the sockets in one place (here) to
         * ensure that we don't ever end up with Events that contain references
         * to closed Sockets; and this is the one place were we can be assured
         * we don't have those Events live.
         *
         * When we loop back to the top of the server accept loop, we will
         * re-evaluate the list of listenFds and create new Events based on the
         * current state of the list (after we remove or add anything here).
         *
         * We also take this opportunity to run the state machine that deals
         * with whether or not to enable WFD listeners and the name service
         * UDP listeners.
         */
        RunListenMachine();
    }

    /*
     * If we're stopping, it is our responsibility to clean up the list of FDs
     * we are listening to.  Since we've gotten a Stop() and are exiting the
     * server loop, and FDs are added in the server loop, this is the place to
     * get rid of them.  We don't have to take the list lock since a Stop()
     * request to the WFDTransport is required to lock out any new
     * requests that may possibly touch the listen FDs list.
     */
    m_listenFdsLock.Lock(MUTEX_CONTEXT);
    for (list<pair<qcc::String, SocketFd> >::iterator i = m_listenFds.begin(); i != m_listenFds.end(); ++i) {
        qcc::Shutdown(i->second);
        qcc::Close(i->second);
    }
    m_listenFds.clear();
    m_listenFdsLock.Unlock(MUTEX_CONTEXT);

    QCC_DbgPrintf(("WFDTransport::Run is exiting status=%s", QCC_StatusText(status)));
    return (void*) status;
}

/*
 * The purpose of this code is really to ensure that we don't have any listeners
 * active on Android systems if we have no ongoing advertisements.  This is to
 * satisfy a requirement driven from the Android Compatibility Test Suite (CTS)
 * which fails systems that have processes listening for WFD connections when
 * the test is run.
 *
 * Listeners and advertisements are interrelated.  In order to Advertise a
 * service, the name service must have an endpoint to include in its
 * advertisements; and there must be at least one listener running and ready to
 * receive connections before telling the name service to advertise.
 *
 * Discovery requests do not require listeners be present per se before being
 * forwarded to the name service.  A discovery request will ulitmately lead to a
 * bus-to-bus connection once a remote daemon has been discovered; but the local
 * side will always start the connection.  Sessions throw a bit of a monkey
 * wrench in the works, though.  Since a JoinSession request is sent to the
 * (already connected) remote daemon and it decides what to do, we don't want to
 * arbitrarily constrain the remote daemon by disallowing it to try and connect
 * back to the local daemon.  For this reason, we do require listeners to be
 * present before discovery starts.
 *
 * So the goal is to not have active listeners in the system unless there are
 * outstanding advertisements or discovery requests, but we cannot have
 * outstanding advertisements or discovery requests until there are active
 * listeners.  Some care is obviously required here to accomplish this
 * seemingly inconsistent behavior.
 *
 * We call the state of no outstanding advertisements and not outstanding
 * discovery requests "Name Service Quiescent".  In this case, the name service
 * must be disabled so that it doesn't interact with the network and cause a CTS
 * failure.  As soon as a either a discovery request or an advertisement request
 * is started, we need to enable the name service to recieve and send network
 * packets, which will cause the daemon process to begin listening on the name
 * service well-known UDP port.
 *
 * Before an advertisement or a discovery request can acutally be sent over the
 * wire, we must start a listener which will receive connection requests, and
 * we must provide the name service with endpoint information that it can include
 * in its advertisement.  So, from the name service and network perspective,
 * listens must preceed advertisements.
 *
 * In order to accomplish the CTS requirements, however, advertisements must
 * preceed listens.  It turns out that this is how the high-level system wants
 * to work.  Essentually, the system calls StartListen at the beginning of time
 * (when the daemon is first brought up) and it calls StopListen at the end of
 * time (when the daemon is going down).  Advertisements and discovery requests
 * come and go in between as clients and services come up and go down.
 *
 * To deal with this time-inversion, we save a list of all listen requests, a
 * list of all advertisement requests and a list of all discovery requests.  At
 * the beginning of time we get one or more StartListen calls and save the
 * listen specs, but do not actually do the socket operations to start the
 * corresponding socket-level listens.  When the first advertisement or
 * discovery request comes in from the higher-level code, we first start all of
 * the saved listens and then enable the name service and ask it to start
 * advertising or discovering as appropriate.  Further advertisements and
 * discovery requests are also saved, but the calls to the name service are
 * passed through when it is not quiescent.
 *
 * We keep track of the disable advertisement and discovery calls as well.  Each
 * time an advertisement or discover operation is disabled, we remove the
 * corresponding entry in the associated list.  As soon as all advertisements
 * and discovery operations are disabled, we disable the name service and remove
 * our WFD listeners, and therefore remove all listeners from the system.  Since
 * we have a saved a list of listeners, they can be restarted if another
 * advertisement or discovery request comes in.
 *
 * We need to do all of this in one place (here) to make it easy to keep the
 * state of the transport (us) and the name service consistent.  We are
 * basically a state machine handling the following transitions:
 *
 *   START_LISTEN_INSTANCE: An instance of a StartListen() has happened so we
 *     need to add the associated listen spec to our list of listeners and be
 *     ready for a subsequent advertisement.  We expect these to happen at the
 *     beginning of time; but there is nothing preventing a StartListen after we
 *     start advertising.  In this case we need to execute the start listen.
 *
 *   STOP_LISTEN_INSTANCE: An instance of a StopListen() has happened so we need
 *     to remove the listen spec from our list of listeners.  We expect these to
 *     happen at the end of time; but there is nothing preventing a StopListen
 *     at any other time.  In this case we need to execute the stop listen and
 *     remove the specified listener immediately
 *
 *   ENABLE_ADVERTISEMENT_INSTANCE: An instance of an EnableAdvertisement() has
 *     happened.  If there are no other ongoing advertisements, we need to
 *     enable the stored listeners, pass the endpoint information down to the
 *     name servcie, enable the name service communication with the outside
 *     world if it is disabled and finally pass the advertisement down to the
 *     name service.  If there are other ongoing advertisements we just pass
 *     down the new advertisement.  It is an AllJoyn system programming error to
 *     start advertising before starting at least one listen.
 *
 *   DISABLE_ADVERTISEMENT_INSTANCE: An instance of a DisableAdvertisement()
 *     call has happened.  We always want to pass the corresponding Cancel down
 *     to the name service.  If we decide that this is the last of our ongoing
 *     advertisements, we need to continue and disable the name service from
 *     talking to the outside world.  For completeness, we remove endpoint
 *     information from the name service.  Finally, we shut down our WFD
 *     transport listeners.
 *
 *   ENABLE_DISCOVERY_INSTANCE: An instance of an EnableDiscovery() has
 *     happened.  This is a fundamentally different request than an enable
 *     advertisement.  We don't need any listeners to be present in order to do
 *     discovery, but the name service must be enabled so it can send and
 *     receive WHO-HAS packets.  If the name service communications are
 *     disabled, we need to enable them.  In any case we pass the request down
 *     to the name service.
 *
 *   DISABLE_DISCOVERY_INSTANCE: An instance of a DisableDiscovery() call has
 *     happened.  There is no corresponding disable call in the name service,
 *     but we do have to decide if we want to disable the name service to keep
 *     it from listening.  We do so if this is the last discovery instance and
 *     there are no other advertisements.
 *
 * There are four member variables that reflect the state of the transport
 * and name service with respect to this code:
 *
 *   m_isListening:  The list of listeners is reflected by currently listening
 *     sockets.  We have network infrastructure in place to receive inbound
 *     connection requests.
 *
 *   m_isNsEnabled:  The name service is up and running and listening on its
 *     sockets for incoming requests.
 *
 *   m_isAdvertising: The list of advertisements is reflected by current
 *     advertisements in the name service.  if we are m_isAdvertising then
 *     m_isNsEnabled must be true.
 *
 *   m_isDiscovering: The list of discovery requests has been sent to the name
 *     service.  if we are m_isDiscovering then m_isNsEnabled must be true.
 */
void WFDTransport::RunListenMachine(void)
{
    QCC_DbgPrintf(("WFDTransport::RunListenMachine()"));

    while (m_listenRequests.empty() == false) {
        QCC_DbgPrintf(("WFDTransport::RunListenMachine(): Do request."));

        /*
         * Pull a request to do a listen request off of the queue of requests.
         * These requests relate to starting and stopping discovery and
         * advertisements; and also whether or not to listen for inbound
         * connections.
         */
        m_listenRequestsLock.Lock(MUTEX_CONTEXT);
        ListenRequest listenRequest = m_listenRequests.front();
        m_listenRequests.pop();
        m_listenRequestsLock.Unlock(MUTEX_CONTEXT);

        /*
         * Do some consistency checks to make sure we're not confused about what
         * is going on.
         *
         * First, if we are not listening, then we had better not think we're
         * advertising or discovering.  If we are not listening, then the name
         * service must not be enabled and sending or responding to external
         * daemons.
         */
        if (m_isListening == false) {
            assert(m_isAdvertising == false);
            assert(m_isDiscovering == false);
            assert(m_isNsEnabled == false);
        }

        /*
         * If we think the name service is enabled, it had better think it is
         * enabled.  It must be enabled either because we are advertising or we
         * are discovering.  If we are advertising or discovering, then there
         * must be listeners waiting for connections as a result of those
         * advertisements or discovery requests.  If there are listeners, then
         * there must be a non-zero listenPort.
         */
        if (m_isNsEnabled) {
            assert(m_isAdvertising || m_isDiscovering);
            assert(m_isListening);
            assert(m_listenPort);
        }

        /*
         * If we think we are advertising, we'd better have an entry in the
         * advertisements list to make us advertise, and there must be listeners
         * waiting for inbound connections as a result of those advertisements.
         * If we are advertising the name service had better be enabled.
         */
        if (m_isAdvertising) {
            assert(!m_advertising.empty());
            assert(m_isListening);
            assert(m_listenPort);
            assert(m_isNsEnabled);
        }

        /*
         * If we are discovering, we'd better have an entry in the discovering
         * list to make us discover, and there must be listeners waiting for
         * inbound connections as a result of session operations driven by those
         * discoveries.  If we are discovering the name service had better be
         * enabled.
         */
        if (m_isDiscovering) {
            assert(!m_discovering.empty());
            assert(m_isListening);
            assert(m_listenPort);
            assert(m_isNsEnabled);
        }

        /*
         * Now that are sure we have a consistent view of the world, let's do
         * what needs to be done.
         */
        switch (listenRequest.m_requestOp) {
        case START_LISTEN_INSTANCE:
            StartListenInstance(listenRequest);
            break;

        case STOP_LISTEN_INSTANCE:
            StopListenInstance(listenRequest);
            break;

        case ENABLE_ADVERTISEMENT_INSTANCE:
            EnableAdvertisementInstance(listenRequest);
            break;

        case DISABLE_ADVERTISEMENT_INSTANCE:
            DisableAdvertisementInstance(listenRequest);
            break;

        case ENABLE_DISCOVERY_INSTANCE:
            EnableDiscoveryInstance(listenRequest);
            break;

        case DISABLE_DISCOVERY_INSTANCE:
            DisableDiscoveryInstance(listenRequest);
            break;
        }
    }
}

void WFDTransport::StartListenInstance(ListenRequest& listenRequest)
{
    QCC_DbgPrintf(("WFDTransport::StartListenInstance()"));

    /*
     * We have a new StartListen request, so save the listen spec so we
     * can restart the listen if we stop advertising.
     */
    NewListenOp(START_LISTEN, listenRequest.m_requestParam);

    /*
     * If we're running on Windows, we always start listening immediately
     * since Windows uses WFD as the client to daemon communication link.
     *
     * On other operating systems (i.e. Posix) we use unix domain sockets and so
     * we can delay listening to passify the Android Compatibility Test Suite.
     * We do this unless we have any outstanding advertisements or discovery
     * operations in which case we start up the listens immediately.
     */
    if (m_isAdvertising || m_isDiscovering) {
        DoStartListen(listenRequest.m_requestParam);
    }
}

void WFDTransport::StopListenInstance(ListenRequest& listenRequest)
{
    QCC_DbgPrintf(("WFDTransport::StopListenInstance()"));

    /*
     * We have a new StopListen request, so we need to remove this
     * particular listen spec from our lists so it will not be
     * restarted.
     */
    bool empty = NewListenOp(STOP_LISTEN, listenRequest.m_requestParam);

    /*
     * If we have just removed the last listener, we have a problem if
     * we have active advertisements.  This is because we will be
     * advertising soon to be non-existent endpoints.  The question is,
     * what do we want to do about it.  We could just ignore it since
     * since clients receiving advertisements may just try to connect to
     * a non-existent endpoint and fail.  It does seem better to log an
     * error and then cancel any outstanding advertisements since they
     * are soon to be meaningless.
     */
    if (empty && m_isAdvertising) {
        QCC_LogError(ER_FAIL, ("WFDTransport::StopListenInstance(): No listeners with outstanding advertisements."));
        for (list<qcc::String>::iterator i = m_advertising.begin(); i != m_advertising.end(); ++i) {
            IpNameService::Instance().CancelAdvertiseName(TRANSPORT_WFD, *i);
        }
    }

    /*
     * Execute the code that will actually tear down the specified
     * listening endpoint.  Note that we always stop listening
     * immediately since that is Good (TM) from a power and CTS point of
     * view.  We only delay starting to listen.
     */
    DoStopListen(listenRequest.m_requestParam);
}

void WFDTransport::EnableAdvertisementInstance(ListenRequest& listenRequest)
{
    QCC_DbgPrintf(("WFDTransport::EnableAdvertisementInstance()"));

    /*
     * We have a new advertisement request to deal with.  The first
     * order of business is to save the well-known name away for
     * use later.
     */
    bool isFirst;
    NewAdvertiseOp(ENABLE_ADVERTISEMENT, listenRequest.m_requestParam, isFirst);

    /*
     * If it turned out that is the first advertisement on our list, we need to
     * prepare before actually doing the advertisement.
     */
    if (isFirst) {

        /*
         * If we don't have any listeners up and running, we need to get them
         * up.  If this is a Windows box, the listeners will start running
         * immediately and will never go down, so they may already be running.
         */
        if (!m_isListening) {
            for (list<qcc::String>::iterator i = m_listening.begin(); i != m_listening.end(); ++i) {
                DoStartListen(*i);
                assert(m_listenPort);
                m_isListening = true;
            }
        }

        /*
         * We can only enable the requested advertisement if there is something
         * listening inbound connections on.  Therefore, we should only enable
         * the name service if there is a listener.  This catches the case where
         * there was no StartListen() done before the first advertisement.
         */
        if (m_isListening) {
            if (!m_isNsEnabled) {
                IpNameService::Instance().Enable(TRANSPORT_WFD, m_listenPort, 0, 0, 0);
                m_isNsEnabled = true;
            }
        } else {
            QCC_LogError(ER_FAIL, ("WFDTransport::EnableAdvertisementInstance(): Advertise with no WFD listeners"));
            return;
        }
    }

    /*
     * We think we're ready to send the advertisement.  Are we really?
     */
    assert(m_isListening);
    assert(m_listenPort);
    assert(m_isNsEnabled);
    assert(IpNameService::Instance().Started() && "WFDTransport::EnableAdvertisementInstance(): IpNameService not started");

    QStatus status = IpNameService::Instance().AdvertiseName(TRANSPORT_WFD, listenRequest.m_requestParam);
    if (status != ER_OK) {
        QCC_LogError(status, ("WFDTransport::EnableAdvertisementInstance(): Failed to advertise \"%s\"", listenRequest.m_requestParam.c_str()));
    }

    /*
     * We've advertised the name with the multicast name service so now we need
     * to deal with the possibility that we are on a system which supports Wi-Fi
     * P2P.  In this case, the supported system is Android and the only way to
     * talk to P2P is via the Android Application Framework.  This means we need
     * to call out to a service that is implemented in a process which has the
     * framework present.  We use a helper to make life easier for us.
     *
     * We lazily create the helper since there's no easy way for us to know when
     * the initialization sequence has completed enough for us to find the DBus
     * interface.  If some service is advertising, though, we must be
     * initialized since a service fhas connected to the daemon and is doing an
     * advertise name.
     */
    if (m_p2pHelperInterface == NULL) {
        QCC_DbgPrintf(("WFDTransport::EnableAdvertisementInstance(): new P2PHelperInterface"));
        m_p2pHelperInterface = new P2PHelperInterface();
        m_p2pHelperInterface->Init(&m_bus);

        QCC_DbgPrintf(("WFDTransport::EnableAdvertisementInstance(): new P2PHelperListener"));
        m_myP2pHelperListener = new MyP2PHelperListener(this);
        m_p2pHelperInterface->SetListener(m_myP2pHelperListener);
    }

    /*
     * If we are on an Android system, we need to call back into the helper
     * service to make this happen.
     *
     * First, if we are going to advertise a name, it means we are a service.
     * In this case, we need to act as the group owner (GO) of a P2P group which
     * AllJoyn clients will join as STA nodes when they discover the name over
     * Wi-Fi P2P.  We want to instantiate a group (act as a GO) whenever we start
     * advertising and keep the group up and running until we get a cancel
     * advertise name.
     *
     * We need to create the GO irrespective of whether or not a concurrent
     * infrastructure mode interface is up or not.  This is so we can connect via
     * Wi-Fi P2P if the corresponding AP is blocking communication between STAs
     * for example.
     *
     * The call to EstablishLink returns a handle for the group, so we have to
     * wait for the response.  There is a lot of action happening in the main
     * thread of the WFD transport so the thread is constantly in the Alerted
     * state, so we use an asynchrounous method call.  Note that we are blocking
     * the the WFDTransport main thread here, so inbound connections will be
     * delayed for the time it takes to get the response.  The response comes
     * back in MyP2PHelperListener::HandleEstablishLinkReply() found in
     * WFDTransport.h, BTW.
     */
    if (!m_isAdvertising) {
        m_establishLinkEvent.ResetEvent();
        m_establishLinkResult = false;
        qcc::String localDevice("");
        QCC_DbgPrintf(("WFDTransport::EnableAdvertisementInstance(): EstablishLink()"));
        status = m_p2pHelperInterface->EstablishLinkAsync(localDevice, P2PHelperInterface::DEVICE_MUST_BE_GO);
        if (status == ER_OK) {
            /*
             * Only wait for a rational amount of time for this RPC to complete.
             * If the system is working, this should happen very quickly, so
             * we'll pick the generic five second timeout out of the air.
             */
            uint32_t msWaited = 0;

            for (;;) {
                QCC_DbgPrintf(("WFDTransport::EnableAdvertisementInstance(): Gag, polling m_establishLinkResult"));
                if (m_establishLinkResult) {
                    break;
                }

                if (msWaited > 5000) {
                    status = ER_TIMEOUT;
                    break;
                }

                qcc::Sleep(100);
                msWaited += 100;
            }
        } else {
            QCC_LogError(status, ("WFDTransport::EnableAdvertisementInstance(): EstablishLink for GO fails"));
        }

        if (status != ER_OK || m_goHandle < P2PHelperInterface::P2P_OK) {
            /*
             * We were unable to start a P2P Group, but this doesn't mean that
             * the entire advertisement process has not been successful.  It is
             * possible that there are existing groups (P2P, legacy, soft AP,
             * ethernet, etc.) that are working just fine, so we do not error
             * out if we can't create the group -- we just report the error and
             * don't try to pre-association advertise the corresponding service.
             * We will have advertised over the multicast name service at this
             * point.
             */
            QCC_LogError(status, ("WFDTransport::EnableAdvertisementInstance(): Could not start P2P group"));
        } else {
            /*
             * The P2P group has come up, or the service assures us that it will
             * during a group owner negotiaion, so we can confidently advertise
             * the corresponding service over pre-association service discovery.
             * There's not much we can do if the Android Application Framework
             * balks, so we just throw the RPC over the fence and hope it sticks.
             */
            QCC_DbgPrintf(("WFDTransport::EnableAdvertisementInstance(): AdvertiseNameAsync()"));
            status = m_p2pHelperInterface->AdvertiseNameAsync(listenRequest.m_requestParam,
                                                              m_bus.GetInternal().GetGlobalGUID().ToString());
            if (status != ER_OK) {
                QCC_LogError(status, ("WFDTransport::EnableAdvertisementInstance(): Failed to advertise \"%s\" to P2P",
                                      listenRequest.m_requestParam.c_str()));
            }
        }
    }

    QCC_DbgPrintf(("WFDTransport::EnableAdvertisementInstance(): Done"));
    m_isAdvertising = true;
}

void WFDTransport::DisableAdvertisementInstance(ListenRequest& listenRequest)
{
    QCC_DbgPrintf(("WFDTransport::DisableAdvertisementInstance()"));

    /*
     * We have a new disable advertisement request to deal with.  The first
     * order of business is to remove the well-known name from our saved list.
     */
    bool isFirst;
    bool isEmpty = NewAdvertiseOp(DISABLE_ADVERTISEMENT, listenRequest.m_requestParam, isFirst);

    /*
     * We always cancel any advertisement to allow the name service to
     * send out its lost advertisement message.
     */
    QStatus status = IpNameService::Instance().CancelAdvertiseName(TRANSPORT_WFD, listenRequest.m_requestParam);
    if (status != ER_OK) {
        QCC_LogError(status, ("WFDTransport::DisableAdvertisementInstance(): Failed to Cancel \"%s\"", listenRequest.m_requestParam.c_str()));
    }

    /*
     * We've advertised the name with the multicast name service so now we need
     * to deal with the possibility that we are on a system which supports Wi-Fi
     * P2P.  In this case, the supported system is Android and the only way to
     * talk to P2P is via the Android Application Framework.  This means we need
     * to call out to a service that is implemented in a process which has the
     * framework present.  We use a helper to make life easier for us.
     *
     * We lazily create the helper since there's no easy way for us to know when
     * the initialization sequence has completed enough for us to find the DBus
     * interface.  If some service is advertising, though, we must be
     * initialized since a service fhas connected to the daemon and is doing an
     * advertise name.
     */
    if (m_p2pHelperInterface == NULL) {
        QCC_DbgPrintf(("WFDTransport::DisableAdvertisementInstance(): new P2PHelperInterface"));
        m_p2pHelperInterface = new P2PHelperInterface();
        m_p2pHelperInterface->Init(&m_bus);

        QCC_DbgPrintf(("WFDTransport::DisableAdvertisementInstance(): new P2PHelperListener"));
        m_myP2pHelperListener = new MyP2PHelperListener(this);
        m_p2pHelperInterface->SetListener(m_myP2pHelperListener);
    }

    /*
     * If we are on an Android system, we need to call back into the helper
     * service to make this happen.  If the framework balks at the request to
     * release the link, there's not much we can do about it.
     */
    if (isEmpty && m_isAdvertising) {
        QCC_DbgPrintf(("WFDTransport::DisableAdvertisementInstance(): ReleaseLinkAsync()"));
        m_p2pHelperInterface->ReleaseLinkAsync(m_goHandle);
        m_goHandle = -1;
    }

    QCC_DbgPrintf(("WFDTransport::DisableAdvertisementInstance(): CancelAdvertiseNameAsync()"));
    status = m_p2pHelperInterface->CancelAdvertiseNameAsync(listenRequest.m_requestParam.c_str(),
                                                            m_bus.GetInternal().GetGlobalGUID().ToString().c_str());
    if (status != ER_OK) {
        QCC_LogError(status, ("WFDTransport::DisableAdvertisementInstance(): Failed to cancel advertise \"%s\" to P2P",
                              listenRequest.m_requestParam.c_str()));
    }

    /*
     * If it turns out that this was the last advertisement on our list, we need
     * to think about disabling our listeners and turning off the name service.
     * We only to this if there are no discovery instances in progress.
     */
    if (isEmpty && !m_isDiscovering) {

        /*
         * Since the cancel advertised name has been sent, we can disable the
         * name service.  We do this by telling it we don't want it to be
         * enabled on any of the possible ports.
         */
        IpNameService::Instance().Enable(TRANSPORT_WFD, 0, 0, 0, 0);
        m_isNsEnabled = false;

        /*
         * If we had the name service running, we must have had listeners
         * waiting for connections due to the name service.  We need to stop
         * them all now, but only if we are not running on a Windows box.
         * Windows needs the listeners running at all times since it uses
         * WFD for the client to daemon connections.
         */
        for (list<qcc::String>::iterator i = m_listening.begin(); i != m_listening.end(); ++i) {
            DoStopListen(*i);
        }

        m_isListening = false;
        m_listenPort = 0;
    }

    if (isEmpty) {
        m_isAdvertising = false;
    }
}

void WFDTransport::EnableDiscoveryInstance(ListenRequest& listenRequest)
{
    QCC_DbgPrintf(("WFDTransport::EnableDiscoveryInstance()"));

    /*
     * We have a new discovery request to deal with.  The first
     * order of business is to save the well-known name away for
     * use later.
     */
    bool isFirst;
    NewDiscoveryOp(ENABLE_DISCOVERY, listenRequest.m_requestParam, isFirst);

    /*
     * If it turned out that is the first discovery request on our list, we need
     * to prepare before actually doing the discovery.
     */
    if (isFirst) {

        /*
         * If we don't have any listeners up and running, we need to get them
         * up.  If this is a Windows box, the listeners will start running
         * immediately and will never go down, so they may already be running.
         */
        if (!m_isListening) {
            for (list<qcc::String>::iterator i = m_listening.begin(); i != m_listening.end(); ++i) {
                DoStartListen(*i);
                assert(m_listenPort);
                m_isListening = true;
            }
        }

        /*
         * We can only enable the requested advertisement if there is something
         * listening inbound connections on.  Therefore, we should only enable
         * the name service if there is a listener.  This catches the case where
         * there was no StartListen() done before the first discover.
         */
        if (m_isListening) {
            if (!m_isNsEnabled) {
                IpNameService::Instance().Enable(TRANSPORT_WFD, m_listenPort, 0, 0, 0);
                m_isNsEnabled = true;
            }
        } else {
            QCC_LogError(ER_FAIL, ("WFDTransport::EnableDiscoveryInstance(): Discover with no WFD listeners"));
            return;
        }
    }

    /*
     * We think we're ready to send the FindAdvertisedName.  Are we really?
     */
    assert(m_isListening);
    assert(m_listenPort);
    assert(m_isNsEnabled);
    assert(IpNameService::Instance().Started() && "WFDTransport::EnableDiscoveryInstance(): IpNameService not started");

    /*
     * When a bus name is advertised, the source may append a string that
     * identifies a specific instance of advertised name.  For example, one
     * might advertise something like
     *
     *   com.mycompany.myproduct.0123456789ABCDEF
     *
     * as a specific instance of the bus name,
     *
     *   com.mycompany.myproduct
     *
     * Clients of the system will want to be able to discover all specific
     * instances, so they need to do a wildcard search for bus name strings
     * that match the non-specific name, for example,
     *
     *   com.mycompany.myproduct*
     *
     * We automatically append the name service wildcard character to the end
     * of the provided string (which we call the namePrefix) before sending it
     * to the name service which forwards the request out over the net.
     */
    qcc::String starred = listenRequest.m_requestParam;
    starred.append('*');

    QStatus status = IpNameService::Instance().FindAdvertisedName(TRANSPORT_WFD, starred);
    if (status != ER_OK) {
        QCC_LogError(status, ("WFDTransport::EnableDiscoveryInstance(): Failed to begin discovery with multicast NS \"%s\"", starred.c_str()));
    }

    /*
     * We've tried to find the name with the multicast name service so now we
     * need to deal with the possibility that we are on a system which supports
     * Wi-Fi P2P.  In this case, the supported system is Android and the only
     * way to talk to P2P is via the Android Application Framework.  This means
     * we need to call out to a service that is implemented in a process which
     * has the framework present.  We use a helper to make life easier for us.
     *
     * We lazily create the helper since there's no easy way for us to know when
     * the initialization sequence has completed enough for us to find the DBus
     * interface.  If some service is doing discovery, though, we must be
     * initialized since a service fhas connected to the daemon and is doing an
     * advertise name.
     */
    if (m_p2pHelperInterface == NULL) {
        QCC_DbgPrintf(("WFDTransport::EnableDiscoveryInstance(): new P2PHelperInterface"));
        m_p2pHelperInterface = new P2PHelperInterface();
        m_p2pHelperInterface->Init(&m_bus);

        QCC_DbgPrintf(("WFDTransport::EnableDiscoveryInstance(): new P2PHelperListener"));
        m_myP2pHelperListener = new MyP2PHelperListener(this);
        m_p2pHelperInterface->SetListener(m_myP2pHelperListener);
    }

    /*
     * If we are on an Android system, we need to call back into the helper
     * service to make this happen.  We don't hang around waiting or a response
     * since there's not much we can do if it fails.
     */
    QCC_DbgPrintf(("WFDTransport::EnableDiscoveryInstance(): FindAdvertisedNameAsync()"));
    status = m_p2pHelperInterface->FindAdvertisedNameAsync(starred);
    if (status != ER_OK) {
        QCC_LogError(status, ("WFDTransport::EnableDiscoveryInstance(): Failed to begin discovery with P2P \"%s\"", starred.c_str()));
    }

    m_isDiscovering = true;
}

void WFDTransport::DisableDiscoveryInstance(ListenRequest& listenRequest)
{
    QCC_DbgPrintf(("WFDTransport::DisableDiscoveryInstance()"));

    /*
     * We have a new disable discovery request to deal with.  The first
     * order of business is to remove the well-known name from our saved list.
     */
    bool isFirst;
    bool isEmpty = NewDiscoveryOp(DISABLE_DISCOVERY, listenRequest.m_requestParam, isFirst);

    qcc::String starred = listenRequest.m_requestParam;
    starred.append('*');

    /*
     * We've tried to find the name with the multicast name service so now we
     * need to deal with the possibility that we are on a system which supports
     * Wi-Fi P2P.  In this case, the supported system is Android and the only
     * way to talk to P2P is via the Android Application Framework.  This means
     * we need to call out to a service that is implemented in a process which
     * has the framework present.  We use a helper to make life easier for us.
     *
     * We lazily create the helper since there's no easy way for us to know when
     * the initialization sequence has completed enough for us to find the DBus
     * interface.  If some service is doing discovery, though, we must be
     * initialized since a service fhas connected to the daemon and is doing an
     * advertise name.
     */
    if (m_p2pHelperInterface == NULL) {
        QCC_DbgPrintf(("WFDTransport::DisableDiscoveryInstance(): new P2PHelperInterface"));
        m_p2pHelperInterface = new P2PHelperInterface();
        m_p2pHelperInterface->Init(&m_bus);

        QCC_DbgPrintf(("WFDTransport::DisableDiscoveryInstance(): new P2PHelperListener"));
        m_myP2pHelperListener = new MyP2PHelperListener(this);
        m_p2pHelperInterface->SetListener(m_myP2pHelperListener);
    }

    /*
     * If we are on an Android system, we need to call back into the helper
     * service to make this happen.
     */
    QCC_DbgPrintf(("WFDTransport::DisableDiscoveryInstance(): CancelFindAdvertisedNameAsync()"));
    QStatus status = m_p2pHelperInterface->CancelFindAdvertisedNameAsync(starred);
    if (status != ER_OK) {
        QCC_LogError(status, ("WFDTransport::DisableDiscoveryInstance(): Failed to cancel discovery with P2P \"%s\"", starred.c_str()));
    }

    /*
     * There is no state in the name service with respect to ongoing discovery.
     * A discovery request just causes it to send a WHO-HAS message, so thre
     * is nothing to cancel down there.
     *
     * However, if it turns out that this was the last discovery operation on
     * our list, we need to think about disabling our listeners and turning off
     * the name service.  We only to this if there are no advertisements in
     * progress.
     */
    if (isEmpty && !m_isAdvertising) {

        IpNameService::Instance().Enable(TRANSPORT_WFD, 0, 0, 0, 0);
        m_isNsEnabled = false;

        /*
         * If we had the name service running, we must have had listeners
         * waiting for connections due to the name service.  We need to stop
         * them all now, but only if we are not running on a Windows box.
         * Windows needs the listeners running at all times since it uses
         * WFD for the client to daemon connections.
         */
        for (list<qcc::String>::iterator i = m_listening.begin(); i != m_listening.end(); ++i) {
            DoStopListen(*i);
        }

        m_isListening = false;
        m_listenPort = 0;
    }

    if (isEmpty) {
        m_isDiscovering = false;
    }
}

/*
 * The default address for use in listen specs.  INADDR_ANY means to listen
 * for WFD connections on any interfaces that are currently up or any that may
 * come up in the future.
 */
static const char* ADDR4_DEFAULT = "0.0.0.0";

#if 0
/*
 * The WFD transport does not support IPv6 at this time
 */
static const char* ADDR6_DEFAULT = "0::0";
#endif

/*
 * The default port for use in listen specs.  This port is used by the WFD
 * listener to listen for incoming connection requests.  This is the default
 * port for a "reliable" IPv4 listener since being able to deal with IPv4
 * connection requests is required as part of the definition of the WFD
 * transport.
 *
 * All other mechanisms (unreliable IPv4, reliable IPv6, unreliable IPv6)
 * rely on the presence of an u4port, r6port, and u6port respectively to
 * enable those mechanisms if possible.
 */
static const uint16_t PORT_DEFAULT = 9955;

QStatus WFDTransport::NormalizeListenSpec(const char* inSpec, qcc::String& outSpec, map<qcc::String, qcc::String>& argMap) const
{
    qcc::String family;

    /*
     * We don't make any calls that require us to be in any particular state
     * with respect to threading so we don't bother to call IsRunning() here.
     *
     * Take the string in inSpec, which must start with "wfd:" and parse it,
     * looking for comma-separated "key=value" pairs and initialize the
     * argMap with those pairs.
     *
     * There are lots of legal possibilities for an IP-based transport, but
     * all we are going to recognize is the "reliable IPv4 mechanism" and
     * so we will summarily pitch everything else.
     *
     * We expect to end up with a normalized outSpec that looks something
     * like:
     *
     *     "wfd:r4addr=0.0.0.0,r4port=9955"
     *
     * That's all.  Everything else, family, port, addr, u4addr, etc.,
     * is summarily pitched.
     */
    QStatus status = ParseArguments(GetTransportName(), inSpec, argMap);
    if (status != ER_OK) {
        return status;
    }

    map<qcc::String, qcc::String>::iterator iter;

    /*
     * The family, addr and port keys are deprecated since a transport can now
     * support both IPv4 and IPv6 connections.  Log errors, but just ignore the
     * deprecated keys.
     */
    iter = argMap.find("family");
    if (iter != argMap.end()) {
        QCC_LogError(ER_BUS_BAD_TRANSPORT_ARGS,
                     ("WFDTransport::NormalizeListenSpec(): The key \"family\" is deprecated and ignored."));
        argMap.erase(iter);
    }

    iter = argMap.find("addr");
    if (iter != argMap.end()) {
        QCC_LogError(ER_BUS_BAD_TRANSPORT_ARGS,
                     ("WFDTransport::NormalizeListenSpec(): The key \"addr\" is deprecated and ignored."));
        argMap.erase(iter);
    }

    iter = argMap.find("port");
    if (iter != argMap.end()) {
        QCC_LogError(ER_BUS_BAD_TRANSPORT_ARGS,
                     ("WFDTransport::NormalizeListenSpec(): The key \"port\" is deprecated and ignored."));
        argMap.erase(iter);
    }

    /*
     * Transports, by definition, may support reliable Ipv4, unreliable IPv4,
     * reliable IPv6 and unreliable IPv6 mechanisms to move bits.  In this
     * incarnation, the WFD transport will only support reliable IPv4; so we
     * log errors and ignore any requests for other mechanisms.
     */
    iter = argMap.find("u4addr");
    if (iter != argMap.end()) {
        QCC_LogError(ER_BUS_BAD_TRANSPORT_ARGS,
                     ("WFDTransport::NormalizeListenSpec(): The mechanism implied by \"u4addr\" is not supported."));
        argMap.erase(iter);
    }

    iter = argMap.find("u4port");
    if (iter != argMap.end()) {
        QCC_LogError(ER_BUS_BAD_TRANSPORT_ARGS,
                     ("WFDTransport::NormalizeListenSpec(): The mechanism implied by \"u4port\" is not supported."));
        argMap.erase(iter);
    }

    iter = argMap.find("r6addr");
    if (iter != argMap.end()) {
        QCC_LogError(ER_BUS_BAD_TRANSPORT_ARGS,
                     ("WFDTransport::NormalizeListenSpec(): The mechanism implied by \"r6addr\" is not supported."));
        argMap.erase(iter);
    }

    iter = argMap.find("r6port");
    if (iter != argMap.end()) {
        QCC_LogError(ER_BUS_BAD_TRANSPORT_ARGS,
                     ("WFDTransport::NormalizeListenSpec(): The mechanism implied by \"r6port\" is not supported."));
        argMap.erase(iter);
    }

    iter = argMap.find("u6addr");
    if (iter != argMap.end()) {
        QCC_LogError(ER_BUS_BAD_TRANSPORT_ARGS,
                     ("WFDTransport::NormalizeListenSpec(): The mechanism implied by \"u6addr\" is not supported."));
        argMap.erase(iter);
    }

    iter = argMap.find("u6port");
    if (iter != argMap.end()) {
        QCC_LogError(ER_BUS_BAD_TRANSPORT_ARGS,
                     ("WFDTransport::NormalizeListenSpec(): The mechanism implied by \"u6port\" is not supported."));
        argMap.erase(iter);
    }

    /*
     * Now, begin normalizing what we want to see in a listen spec.
     *
     * All listen specs must start with the name of the transport followed by
     * a colon.
     */
    outSpec = GetTransportName() + qcc::String(":");

    /*
     * The WFD transport must absolutely support the IPv4 "reliable" mechanism
     * (WFD).  We therefore must provide an r4addr either from explicit keys or
     * generated from the defaults.
     */
    iter = argMap.find("r4addr");
    if (iter != argMap.end()) {
        /*
         * We have a value associated with the "r4addr" key.  Run it through a
         * conversion function to make sure it's a valid value and to get into
         * in a standard representation.
         */
        IPAddress addr;
        status = addr.SetAddress(iter->second, false);
        if (status == ER_OK) {
            /*
             * The r4addr had better be an IPv4 address, otherwise we bail.
             */
            if (!addr.IsIPv4()) {
                QCC_LogError(ER_BUS_BAD_TRANSPORT_ARGS,
                             ("WFDTransport::NormalizeListenSpec(): The r4addr \"%s\" is not a legal IPv4 address.",
                              iter->second.c_str()));
                return ER_BUS_BAD_TRANSPORT_ARGS;
            }
            iter->second = addr.ToString();
            outSpec.append("r4addr=" + addr.ToString());
        } else {
            QCC_LogError(ER_BUS_BAD_TRANSPORT_ARGS,
                         ("WFDTransport::NormalizeListenSpec(): The r4addr \"%s\" is not a legal IPv4 address.",
                          iter->second.c_str()));
            return ER_BUS_BAD_TRANSPORT_ARGS;
        }
    } else {
        /*
         * We have no value associated with an "r4addr" key.  Use the default
         * IPv4 listen address for the outspec and create a new key for the
         * map.
         */
        outSpec.append("r4addr=" + qcc::String(ADDR4_DEFAULT));
        argMap["r4addr"] = ADDR4_DEFAULT;
    }

    /*
     * The WFD transport must absolutely support the IPv4 "reliable" mechanism
     * (WFD).  We therefore must provide an r4port either from explicit keys or
     * generated from the defaults.
     */
    iter = argMap.find("r4port");
    if (iter != argMap.end()) {
        /*
         * We have a value associated with the "r4port" key.  Run it through a
         * conversion function to make sure it's a valid value.  We put it into
         * a 32 bit int to make sure it will actually fit into a 16-bit port
         * number.
         */
        uint32_t port = StringToU32(iter->second);
        if (port <= 0xffff) {
            outSpec.append(",r4port=" + iter->second);
        } else {
            QCC_LogError(ER_BUS_BAD_TRANSPORT_ARGS,
                         ("WFDTransport::NormalizeListenSpec(): The key \"r4port\" has a bad value \"%s\".", iter->second.c_str()));
            return ER_BUS_BAD_TRANSPORT_ARGS;
        }
    } else {
        /*
         * We have no value associated with an "r4port" key.  Use the default
         * IPv4 listen port for the outspec and create a new key for the map.
         */
        qcc::String portString = U32ToString(PORT_DEFAULT);
        outSpec += ",r4port=" + portString;
        argMap["r4port"] = portString;
    }

    return ER_OK;
}

QStatus WFDTransport::NormalizeTransportSpec(const char* inSpec, qcc::String& outSpec, map<qcc::String, qcc::String>& argMap) const
{
    QCC_DbgPrintf(("WFDTransport::NormalizeTransportSpec"));

    QStatus status;

    /*
     * We don't make any calls that require us to be in any particular state
     * with respect to threading so we don't bother to call IsRunning() here.
     *
     * Take the string in inSpec, which must start with "wfd:" and parse it,
     * looking for comma-separated "key=value" pairs and initialize the
     * argMap with those pairs.
     */
    QCC_DbgPrintf(("WFDTransport::NormalizeTransportSpec(): ParseArguments()"));
    status = ParseArguments(GetTransportName(), inSpec, argMap);
    if (status != ER_OK) {
        return status;
    }

    map<qcc::String, qcc::String>::iterator iter;

    /*
     * The big special case here is if there is a guid present in the spec.  If
     * there is, then this is a spec corresponding to a Wi-Fi Direct pre-
     * association discovery event.  Since this is pre-association, there cannot
     * be an IP address, and therefore addr, port and family would be
     * meaningless even if they were there.  In this case, normalizing a spec
     * with a guid in it means ignoring anything else that might be there.
     */
    iter = argMap.find("guid");
    if (iter != argMap.end()) {
        QCC_DbgPrintf(("WFDTransport::NormalizeTransportSpec(): Found guid"));
        qcc::String guidString = iter->second;
        argMap.clear();
        argMap["guid"] = guidString;
        outSpec = "wfd:guid=" + guidString;
        return ER_OK;
    }

    /*
     * Aside from the presence of the guid, the only fundamental difference
     * between a listenSpec and a transportSpec (actually a connectSpec) is that
     * a connectSpec must have a valid and specific address IP address to
     * connect to (i.e., INADDR_ANY isn't a valid IP address to connect to).
     * This means that we can just call NormalizeListenSpec to get everything
     * into standard form.
     */
    status = NormalizeListenSpec(inSpec, outSpec, argMap);
    if (status != ER_OK) {
        return status;
    }

    /*
     * Since there is no guid present if we've fallen through to here, the only
     * difference between a connectSpec and a listenSpec is that a connectSpec
     * requires the presence of a non-default IP address.  So we just check for
     * the default addresses and fail if we find one.
     */
    map<qcc::String, qcc::String>::iterator i = argMap.find("r4addr");
    assert(i != argMap.end());
    if ((i->second == ADDR4_DEFAULT)) {
        QCC_LogError(ER_BUS_BAD_TRANSPORT_ARGS,
                     ("WFDTransport::NormalizeTransportSpec(): The r4addr may not be the default address."));
        return ER_BUS_BAD_TRANSPORT_ARGS;
    }

    return ER_OK;
}

/*
 * Given a GUID string, determine which device corresponds to the daemon that
 * GUID and ask the Wi-Fi P2P helper to form a connection with that device.
 * Then, block until we receive an indication (positive or negative) back
 * from the request has some resolution.
 */
QStatus WFDTransport::CreateTemporaryNetwork(const qcc::String& guid, qcc::String& interface)
{
    QCC_DbgHLPrintf(("WFDTransport::CreateTemporaryNetwork(\"%s\")", guid.c_str()));

    /*
     * Look up the device on which the provided GUID resides.  This information
     * came in with the FoundAdvertisedName signal, which was directed back to
     * applications, which in turn did a JoinSession which drove the Connect
     * which led us here.
     */
    qcc::String device;
    QCC_DbgPrintf(("WFDTransport::CreateTemporaryNetwork(): Looking for device matching GUID %s", guid.c_str()));

    m_deviceListLock.Lock(MUTEX_CONTEXT);
    for (DeviceList::iterator i = m_deviceList.begin(); i != m_deviceList.end(); ++i) {
        if (i->first == guid) {
            device = i->second;
            QCC_DbgPrintf(("WFDTransport::CreateTemporaryNetwork(): Device %s matches GUID %s", device.c_str(), guid.c_str()));
            break;
        }
    }
    m_deviceListLock.Unlock(MUTEX_CONTEXT);

    /*
     * Once we have the device we want to connect to, we ask the Wi-Fi P2P
     * helper to join the P2P group owned by the device we found.  In order
     * to make Wi-Fi P2P work remotely reasonably, the device that advertises
     * a service must act as the group owner (GO) and the device which uses
     * the service must act as a station (STA) node.
     */
    if (!device.empty()) {

        QCC_DbgPrintf(("WFDTransport::CreatetTemporaryNetwork(): Establishing link to device %s", device.c_str()));

        /*
         * Since we are going to be interacting with a separate thread to get
         * responses to our requests, we need to make sure we have our
         * information available so that second thread can find it.  We need to
         * get this done BEFORE we can allow a callback that may happen
         * immediately.  So, we take the lock that is shared with the callback
         * before making the EstablishLink call.  This means that, if the
         * callback happens immediately, it will block.  We need to fill out the
         * data structure that the callback will use, and when we get this
         * finished put it on the list that the callback will examine.  We then
         * release the lock, allowing the response callback to access the
         * structure and do its job.
         */
        QCC_DbgPrintf(("WFDTransport::CreateTemporaryNetwork(): Block callbacks"));
        m_deviceRequestListLock.Lock(MUTEX_CONTEXT);

        m_establishLinkEvent.ResetEvent();
        m_establishLinkResult = false;

        QCC_DbgPrintf(("WFDTransport::CreateTemporaryNetwork(): EstablishLink()"));
        QStatus status = m_p2pHelperInterface->EstablishLinkAsync(device.c_str(), P2PHelperInterface::DEVICE_MUST_BE_STA);
        if (status == ER_OK) {
            /*
             * Only wait for a rational amount of time for this RPC to complete.
             * Getting the handle back for the link should be a farily quick
             * operation, so we'll pick the generic five second timeout out of
             * the air.  The really time consuming part comes later when the
             * link is actually tried to be brought up.  The result of that
             * operation (until either OnLinkEstablished or OnLinkError happens)
             * can be very time consuming.  We're just getting a handle here.
             */
            uint32_t msWaited = 0;

            for (;;) {
                QCC_DbgPrintf(("WFDTransport::CreateTemporaryNetwork(): Gag, polling m_establishLinkResult"));
                if (m_establishLinkResult) {
                    break;
                }

                if (msWaited > 5000) {
                    status = ER_TIMEOUT;
                    break;
                }

                qcc::Sleep(100);
                msWaited += 100;
            }
        } else {
            QCC_LogError(status, ("WFDTransport::CreateTemporaryNetwork(): EstablishLinkAsync for STA fails"));
            m_deviceRequestListLock.Unlock(MUTEX_CONTEXT);
            return status;
        }

        /*
         * Since we can't be both GO and STA, m_goHandle is now actually the STA
         * handle.  Note that this is a handle and does not mean that a link has
         * come up.  We still need to wait for the OnLinkEstablished or
         * OnLinkError signals to come in and tell us what actually happened.
         *
         * XXX FIXME STA GO stuff is confusing.  Dangerous.
         */
        int32_t handle = m_goHandle;

        /*
         * We have a handle for the link we want to establish, but we don't have
         * a result for the actual link establishment operation.  We need to
         * identify ourselves as a thread waiting for a result for a particular
         * link establishment request.  The request is identified by the handle,
         * we just got and we are going to want to wait on an event; so we wrap
         * those two things up in an object and stash it on a list available to
         * the callback that will happen when the request is acted upon.
         */
        qcc::Event deviceEvent;

        P2pDeviceRequest deviceRequest(handle, &deviceEvent);
        m_deviceRequestList.push_back(deviceRequest);

        /*
         * We've created and pushed the object that will allow a response from
         * the EstablishLink to be matched with our thread.  Now, we can release
         * the response list lock so the response can be handled as described
         * above.  This will let the response handler run unmolested.
         */
        QCC_DbgPrintf(("WFDTransport::CreateTemporaryNetwork(): Allow callbacks"));
        m_deviceRequestListLock.Unlock(MUTEX_CONTEXT);

        /*
         * We need to wait a rational amount of time for the link establishment
         * request to be acted upon.  Unfortunately, a rational amount of time
         * is pretty unreasonable.  We need to wait for the GO negotiation to
         * complete, and we may need to wait until a human being on the other
         * side gets around to accepting a connection request popup on the
         * remote device.  We pull two minutes out of the air as a rational
         * amount of time to wait.
         */
        Timespec tTimeout = 120000;
        Timespec tStart;
        GetTimeNow(&tStart);

        /*
         * A one second repeating timer.
         */
        qcc::Event timerEvent(1000, 1000);

        /*
         * We need to wait on both the timer event and the device event.
         *
         * XXX FIXME Don't poll every second, it's silly.
         */
        for (;;) {
            vector<qcc::Event*> checkEvents, signaledEvents;
            checkEvents.push_back(&deviceEvent);
            checkEvents.push_back(&timerEvent);

            QCC_DbgPrintf(("WFDTransport::CreateTemporaryNetwork(): Wait for something to happen"));
            QStatus status = qcc::Event::Wait(checkEvents, signaledEvents);
            if (status != ER_OK && status != ER_TIMEOUT) {
                break;
            }

            /*
             * We are only concerned with two possible events here.  Normally,
             * we'll either see the timer firing or a response notification. In
             * astoundingly rare cases, both events can be set at the same time.
             */
            bool timer = false;
            bool response = false;
            for (vector<qcc::Event*>::iterator i = signaledEvents.begin(); i != signaledEvents.end(); ++i) {
                if (*i == &deviceEvent) {
                    QCC_DbgPrintf(("WFDTransport::CreateTemporaryNetwork(): Device Event happened"));
                    response = true;
                } else if (*i == &timerEvent) {
                    QCC_DbgPrintf(("WFDTransport::CreateTemporaryNetwork(): Timer Event happened"));
                    timer = true;
                }
            }

            /*
             * If the timer popped, check for a timeout.  These are two
             * different things since the timer pops every second and the
             * timeout is some larger number of seconds.  The backwards logic is
             * because the Timespec class doesn't define the needed operator and
             * I'm too lazy to go write it.
             */
            bool timeout = false;
            if (timer) {
                Timespec tNow;
                GetTimeNow(&tNow);

                if (tStart + tTimeout < tNow) {
                    QCC_DbgPrintf(("WFDTransport::CreateTemporaryNetwork(): Timeout happened"));
                    timeout = true;
                }
            }

            /*
             * We need to run through the list of requests in the case of a
             * timeout to find the object to remove (we are timing out the
             * request after all).  We also need to run through the list in the
             * case of a response in order to find the response code and then to
             * remove the request object since we are done.
             */
            if (timeout || response) {
                QCC_DbgPrintf(("WFDTransport::CreateTemporaryNetwork(): Timeout or response"));
                m_deviceRequestListLock.Lock(MUTEX_CONTEXT);

                /*
                 * The eventual result of the EstablishLink() operation.
                 */
                int establishResult = -1;

                for (P2pDeviceRequestList::iterator i = m_deviceRequestList.begin(); i != m_deviceRequestList.end(); ++i) {

                    /*
                     * The response is keyed to the handle we got when we
                     * started this process by calling EstablishLink.
                     */
                    if (i->GetHandle() == handle) {
                        if (response) {
                            establishResult = i->GetResult();
                            QCC_DbgPrintf(("WFDTransport::CreateTemporaryNetwork(): Got result %d", establishResult));

                            /*
                             * We will need the network interface name when we
                             * try to turn on the multicast name service to
                             * discover the IP address and port of the remote
                             * daemon.
                             */
                            if (establishResult == P2PHelperInterface::P2P_OK) {
                                QCC_DbgPrintf(("WFDTransport::CreateTemporaryNetwork(): GetInterfaceNameFromHandleAsync()"));
                                m_getInterfaceNameFromHandleEvent.ResetEvent();
                                m_getInterfaceNameFromHandleResult = false;
                                status = m_p2pHelperInterface->GetInterfaceNameFromHandleAsync(handle);
                                if (status == ER_OK) {
                                    /*
                                     * Only wait for a rational amount of time
                                     * for this RPC to complete.  If the system
                                     * is working, this should happen very
                                     * quickly, so we'll pick the generic five
                                     * second timeout out of the air.
                                     */
                                    uint32_t msWaited = 0;

                                    for (;;) {
                                        QCC_DbgPrintf(("WFDTransport::CreateTemporaryNetwork(): "
                                                       "Waiting on m_getInterfacenameFromHandleResult"));

                                        if (m_getInterfaceNameFromHandleResult) {
                                            break;
                                        }

                                        if (msWaited > 5000) {
                                            status = ER_TIMEOUT;
                                            break;
                                        }

                                        qcc::Sleep(100);
                                        msWaited += 100;
                                    }
                                } else {
                                    QCC_LogError(status,
                                                 ("WFDTransport::EnableAdvertisementInstance(): EstablishLink for GO fails"));
                                }

                                if (status == ER_OK) {
                                    interface = m_foundInterface;
                                } else {
                                    interface = m_foundInterface;
                                }
                            }
                        }

                        QCC_DbgPrintf(("WFDTransport::CreateTemporaryNetwork(): Erasing entry"));
                        m_deviceRequestList.erase(i);
                        break;
                    }
                }

                m_deviceRequestListLock.Unlock(MUTEX_CONTEXT);

                /*
                 * We are now done with our request.  We have either had a
                 * response in which case response is true and establishResult
                 * holds the result of the EstablishLink call, or we had a
                 * timeout in which timeout is true.  In either of these cases
                 * we have already cleaned up after ourseves and can return.
                 * Note that a response trumps a timeout.  If neither of these
                 * cases happened, we need to loop back and wait again.
                 */
                if (response) {
                    QCC_DbgPrintf(("WFDTransport::CreateTemporaryNetwork(): Exiting on response"));
                    if (establishResult < 0) {
                        QCC_LogError(status, ("WFDTransport::CreateTemporaryNetwork(): Error connecting to \"%s\"", device.c_str()));
                        return ER_FAIL;
                    } else {
                        return ER_OK;
                    }
                }

                if (timeout) {
                    QCC_LogError(status, ("WFDTransport::CreateTemporaryNetwork(): Timeout connecting to \"%s\"", device.c_str()));
                    return ER_TIMEOUT;
                }
            }
        }
    }

    assert(false && "WFDTransport::CreateTemporaryNetwork(): Cannot happen");
    return ER_FAIL;
}

/*
 * This is the local side of the callback corresponding to a successful
 * completion of the EstablishLink call in CreateTemporaryNetwork() above.
 *
 * OnLinkEstablished is just an OnLinkError with an error code indicating
 * success.
 */
void WFDTransport::OnLinkEstablished(int handle)
{
    QCC_DbgHLPrintf(("WFDTransport::OnLinkEstablished(%d)", handle));
    OnLinkError(handle, P2PHelperInterface::P2P_OK);
}

/*
 * This is the local side of the callback corresponding to a completion of the
 * EstablishLink call in CreateTemporaryNetwork() above.
 *
 * The basic flow is that CreateTemporaryNetwork() will create an event for us
 * to bug and will save it in an object and stick it on a list of such objects.
 * Thes objects are keyed by the handle returned by EstablishLink.  Our job is
 * to look up the object by handle, save an error code and bug the event to wake
 * up the thread that blocked in CreateTemporaryNetwork waiting for us.
 *
 * If we can't find a thread waiting on an operation for this handle, there's
 * not much that can be done.
 */
void WFDTransport::OnLinkError(int handle, int error)
{
    QCC_DbgHLPrintf(("WFDTransport::OnLinkError(%d, %d)", handle, error));

    m_deviceRequestListLock.Lock(MUTEX_CONTEXT);

    for (P2pDeviceRequestList::iterator i = m_deviceRequestList.begin(); i != m_deviceRequestList.end(); ++i) {
        if (i->GetHandle() == handle) {
            QCC_DbgPrintf(("WFDTransport::OnLInkError(): Found handle %d", handle));
            i->SetResult(error);
            i->SetEvent();
            break;
        }
    }

    m_deviceRequestListLock.Unlock(MUTEX_CONTEXT);
}

/*
 * This is the local side of the callback corresponding to a completion of the
 * EstablishLink call in CreateTemporaryNetwork() above.
 *
 * This callback indicates that the handle represents either a GO being created
 * during the advertise process, or a STA being created during the connect
 * process.  Since the cases are mutually exclusive we just let m_goHandle be
 * the one handle.  This is somewhat confusing and needs changing.
 *
 * The basic flow for both processes is that the advertise or the connect will
 * create an event for us to bug when the reply to its asynchronous call
 * completes.  Our job is to save the result of the operation (the handle) and
 * bug the event to wake up the thread that blocked in the advertise or connect.
 */
void WFDTransport::HandleEstablishLinkReply(int handle)
{
    QCC_DbgHLPrintf(("WFDTransport::HandleEstablishLinkReply(%d)", handle));
    m_goHandle = handle;
    m_establishLinkEvent.SetEvent();
    m_establishLinkResult = true;
}

/*
 * This is the local side of the callback corresponding to a completion of the
 * EstablishLink call in CreateTemporaryNetwork() above.
 *
 * The basic flow is that CreateTemporaryNetwork() will create an event for us
 * to bug when the reply to its asyncrhonous call completes.  Our job is to save
 * the result of the operation (the handle) and bug the event to wake up the
 * thread that blocked in CreateTemporaryNetwork waiting for us.
 */
void WFDTransport::HandleGetInterfaceNameFromHandleReply(qcc::String interface)
{
    QCC_DbgHLPrintf(("WFDTransport::HandleGetInterfaceNameFromHandleReply(\"%s\")", interface.c_str()));
    m_foundInterface = interface;
    m_getInterfaceNameFromHandleEvent.SetEvent();
    m_getInterfaceNameFromHandleResult = true;
}

/*
 * This is where the Wi-Fi P2P network meets the rest of the system.  This
 * process started with a FoundAdvertisedName relating to a Wi-Fi P2P pre
 * association service discovery event coming in from the Android Java framework
 * (via the P2P helper service).  This FoundAdvertisedName was sent up to any
 * AllJoyn applications that might have been looking for it, and the application
 * must've responded with a JoinSession.  This was converted into a Connect call
 * to this transport.  Connect noticed that it was being asked to connect to a
 * device which was discovered using Wi-Fi P2P and it called
 * CreateTemporaryNetwork to get the connection to the device GO formed.
 * Connect is now ready to do the actual WFD connection, but it has absolutely
 * no idea what IP address and port to connect to.  It is our job here to find
 * that information and create a connect spec (see NormalizeTransportSpec) with
 * that information.
 *
 * To find this information, we need to get the multicast name service to
 * discover the IP address and port associated with the GUID, which is the GUID
 * of the daemon we will need to connect to.
 *
 * The name service has a thread that is watching for interfaces to go down and
 * come up, but this runs periodically and so this delay might take up to
 * fifteen seconds to run.  The remote daemon is going to be sending name
 * service keep-alives every 40 seconds.  It could take upwards of a minute to
 * get this process complete, and if the name service is not configured to look
 * at Wi-Fi P2P interfaces, it could never complete.
 *
 * What we need to do is to ask the name service explicitly to open the
 * interface that was created in CreateTemporaryNetwork, and cause a WHO-HAS
 * message to be sent out that interface looking for our target daemon.
 *
 * Once we have sent the WHO-HAS probe, we need to wait until an IS-AT response
 * has been received and sent from the name service up through its callback to
 * this transport.
 */
QStatus WFDTransport::CreateConnectSpec(const qcc::String& interface, const qcc::String& guid, qcc::String& connectSpec)
{
    QCC_DbgPrintf(("WFDTransport::CreateConnectSpec(\"%s\", \"%s\", ...)", interface.c_str(), guid.c_str()));

    assert(IpNameService::Instance().Started() && "WFDTransport::CreateConnectSpec(): IpNameService not started");

    /*
     * Open the interface in the name service to enable it to transmit and receive
     * name service messages on the new Wi-Fi P2P group.
     */
    QCC_DbgPrintf(("WFDTransport::CreateConnectSpec(): OpenInterface()"));
    QStatus status = IpNameService::Instance().OpenInterface(TRANSPORT_WFD, interface);
    if (status != ER_OK) {
        QCC_LogError(status, ("WFDTransport::CreateConnectSpec(): Cannot open interface \"%s\"", interface.c_str()));
        return status;
    }

    /*
     * We are going to depend on the multicast name service callbacks
     * to tell us when the remote daemon address and port is discovered.
     * This info is going to come in on another thread.
     *
     * Since we are going to be interacting with a separate thread to get
     * responses to our requests, we need to make sure we have our information
     * available so that second thread can find it.  We need to get this done
     * BEFORE we can allow a callback that may happen immediately or we may miss
     * it.  This can result in a 40 second delay.  So, we take lock that is
     * shared with the callback before making the Locate call.  This does mean
     * that, if the callback happens immediately, it will block the name
     * service.  We need to fill out the data structure that the callback will
     * use, put it on the list that the callback will examine.  We then release
     * the lock, allowing the response callback to access the structure and do
     * its job.  We need to do this all without delay.
     */
    QCC_DbgPrintf(("WFDTransport::CreatetConnectSpec(): Block callbacks"));
    m_addressRequestListLock.Lock(MUTEX_CONTEXT);

    /*
     * We need to identify ourselves as a thread waiting for a result for a
     * particular name service result.  The result is identified by the GUID of
     * the remote daemon, and we are going to eventually be waiting on an event.
     * We wrap those two things up in an object and stash it on a list available
     * to the callback that will happen when a name service callback is fired.
     */
    qcc::Event addressEvent;

    P2pAddressRequest addressRequest(guid, &addressEvent);
    m_addressRequestList.push_back(addressRequest);

    /*
     * Ask all of the daemons to respond with their GUIDs, IP addresses and ports.
     */
    QCC_DbgPrintf(("WFDTransport::CreateConnectSpec(): Locate(*)"));
    status = IpNameService::Instance().FindAdvertisedName(TRANSPORT_WFD, "*");
    if (status != ER_OK) {
        QCC_LogError(status, ("WFDTransport::CreateConnectSpec(): Cannot open locate service \"*\""));
        return status;
    }

    /*
     * We've created and pushed the object that will allow a callback from the
     * name service to be matched with our thread.  Now, we can make the name
     * service request and release the response list lock so the response can be
     * handled as described above.  This will let the response handler run
     * unmolested.
     */
    QCC_DbgPrintf(("WFDTransport::CreateConnectSpec(): Allow callbacks"));
    m_addressRequestListLock.Unlock(MUTEX_CONTEXT);

    /*
     * We'll wait as long as 15 seconds for a response (the group to be
     * created).  This is because the Locate call to the name service will be
     * retried three times, every five seconds by default.  The next opportunity
     * to receive a name service response may be 40 seconds after that, which is
     * getting pretty far out there.
     */
    Timespec tTimeout = 15000;
    Timespec tStart;
    GetTimeNow(&tStart);

    /*
     * A one second repeating timer.
     */
    qcc::Event timerEvent(1000, 1000);

    /*
     * We need to wait on both the timer event and the device event.
     *
     * XXX FIXME Don't poll, it's silly.
     */
    for (;;) {
        vector<qcc::Event*> checkEvents, signaledEvents;
        checkEvents.push_back(&addressEvent);
        checkEvents.push_back(&timerEvent);

        QCC_DbgPrintf(("WFDTransport::CreatetConnectSpec(): Wait for something to happen"));
        QStatus status = qcc::Event::Wait(checkEvents, signaledEvents);
        if (status != ER_OK && status != ER_TIMEOUT) {
            break;
        }

        /*
         * We are only concerned with two possible events here.  Normally,
         * we'll either see the timer firing or a response notification. In
         * astoundingly rare cases, both events can be set at the same time.
         */
        bool timer = false;
        bool response = false;
        for (vector<qcc::Event*>::iterator i = signaledEvents.begin(); i != signaledEvents.end(); ++i) {
            if (*i == &addressEvent) {
                QCC_DbgPrintf(("WFDTransport::CreateConnectSpec(): Address Event happened"));
                response = true;
            } else if (*i == &timerEvent) {
                QCC_DbgPrintf(("WFDTransport::CreateConnectSpec(): Timer Event happened"));
                timer = true;
            }
        }

        /*
         * If the timer popped, check for a timeout.  These are two different
         * things since the timer pops every second and the timeout is some
         * larger number of seconds.  The backwards logic is because the
         * Timespec class doesn't define the needed operator and I'm too lazy to
         * go write it.
         */
        bool timeout = false;
        if (timer) {
            Timespec tNow;
            GetTimeNow(&tNow);

            if (tStart + tTimeout < tNow) {
                QCC_DbgPrintf(("WFDTransport::CreateConnectSpec(): Timeout happened"));
                timeout = true;
            }
        }

        /*
         * We need to run through the list of requests in the case of a timeout
         * to find the object to remove (we are timing out the request after
         * all).  We also need to run through the list in the case of a response
         * to find the address and port and then to remove the request object.
         */
        if (timeout || response) {
            QCC_DbgPrintf(("WFDTransport::CreateConnectSpec(): Timeout or response"));
            m_addressRequestListLock.Lock(MUTEX_CONTEXT);

            /*
             * These two little puppies are what all of this trouble is about.
             */
            qcc::String address;
            qcc::String port;

            for (P2pAddressRequestList::iterator i = m_addressRequestList.begin(); i != m_addressRequestList.end(); ++i) {

                /*
                 * The response is keyed to the guid of the remote daemon.
                 */
                if (i->GetGuid() == guid) {
                    if (response) {
                        address = i->GetAddress();
                        port = i->GetPort();
                        QCC_DbgPrintf(("WFDTransport::CreatetConnectSpec(): Found %s:%s", address.c_str(), port.c_str()));
                    }
                    QCC_DbgPrintf(("WFDTransport::CreateConnectSpec(): Erasing entry"));
                    m_addressRequestList.erase(i);
                    break;
                }
            }

            m_addressRequestListLock.Unlock(MUTEX_CONTEXT);

            /*
             * We are now done with our request.  We have either had a response
             * in which case response is true and address and port hold the
             * information we're after, or we had a timeout in which timeout is
             * true.  In either of these cases we have already cleaned up after
             * ourseves and can return.  Note that a response trumps a timeout.
             * If neither of these cases happened, we need to loop back and wait
             * again.
             */
            if (response) {
                QCC_DbgPrintf(("WFDTransport::CreatetConnectSpec(): Exiting on response"));

                /*
                 * We now need to create a normalized transport spec out of the
                 * address and port we found.  The input spec is just going to
                 * be "wfd:addr=<address>,port=<port> and we let the normalize
                 * function make it prettier.  What we're interested in is the
                 * output of this function, which is the connect spec we're
                 * after.
                 */
                qcc::String spec, normSpec;
                spec = "wfd:addr=" + address + ",port=" + port;
                map<qcc::String, qcc::String> argMap;
                status = NormalizeTransportSpec(spec.c_str(), normSpec, argMap);
                if (ER_OK != status) {
                    QCC_LogError(status, ("WFDTransport::CreateConnectSpec(): Invalid connect spec \"%s\"", spec.c_str()));
                    return status;
                }

                connectSpec = normSpec;
                return ER_OK;
            }

            if (timeout) {
                QCC_LogError(status, ("WFDTransport::CreateConnectSpec(): Timeout looking for \"%s\"", guid.c_str()));
                return ER_TIMEOUT;
            }
        }
    }

    assert(false && "WFDTransport::CreateConnectSpec(): Cannot happen");
    return ER_FAIL;
}

/*
 * This is the callback corresponding to a location of a remote daemon
 * identified by its GUID.  The provided busAddr contains the IP address and
 * port on which the daemon is listening, and thus provides enough information
 * to put together a connect spec for reaching that daemon.  There may be a
 * CreateConnectSpec call blocked waiting for this address and port to appear.
 *
 * The basic flow is that CreateConnectSpec() will create an event for us to bug
 * and will save it in an object and stick it on a list of such objects.  These
 * objects are keyed by the guid of the remote daemon.  Our job is to look up
 * the object by guid, save the found address and port and bug the event to wake
 * up the thread that blocked in CreateConnectSpec waiting for us to wake it.
 */
void WFDTransport::OnFound(const qcc::String& busAddr, const qcc::String& guid)
{
    QCC_DbgHLPrintf(("WFDTransport::OnFound(\"%s\", \"%s\")", busAddr.c_str(), guid.c_str()));

    m_addressRequestListLock.Lock(MUTEX_CONTEXT);

    for (P2pAddressRequestList::iterator i = m_addressRequestList.begin(); i != m_addressRequestList.end(); ++i) {
        if (i->GetGuid() == guid) {
            QCC_DbgPrintf(("WFDTransport::OnFound(): Found guid \"%s\"", guid.c_str()));

            /*
             * Instead of fighting with the name servcie callback, we just use
             * what it gives us, even though it means a little redundancy.
             */
            qcc::String a("r4addr=");
            qcc::String p(",r4port=");

            /*
             * Find the index of the busAddr string where the string "r4addr=" starts.
             */
            size_t j = busAddr.find(a);
            if (j == String::npos) {
                continue;
            }

            /*
             * Point the index to the point where the actual address starts.
             */
            j += a.size();

            /*
             * Find the index of the busAddr string where the string "r4port=" starts.
             */
            size_t k = busAddr.find(p);
            if (k == String::npos) {
                continue;
            }

            /*
             * The address we're interested in is between the end of "r4addr=" and
             * the start of "r4port="
             */
            i->SetAddress(busAddr.substr(j, k - j));

            /*
             * The port we're interested in is between the end of "r4port=" and
             * the end of the string (we known this because we looked at how it
             * is made).
             */
            i->SetPort(busAddr.substr(k + p.size(), busAddr.size()));

            /*
             * Set the wake event to wake up a thread blocked in
             * CreateConnectSpec waiting for this information.
             */
            i->SetEvent();
            break;
        }
    }

    m_addressRequestListLock.Unlock(MUTEX_CONTEXT);
}

QStatus WFDTransport::Connect(const char* connectSpec, const SessionOpts& opts, BusEndpoint** newep)
{
    QCC_DbgHLPrintf(("WFDTransport::Connect(): %s", connectSpec));

    QStatus status;
    bool isConnected = false;

    /*
     * We only want to allow this call to proceed if we have a running server
     * accept thread that isn't in the process of shutting down.  We use the
     * thread response from IsRunning to give us an idea of what our server
     * accept (Run) thread is doing.  See the comment in Start() for details
     * about what IsRunning actually means, which might be subtly different from
     * your intuitition.
     *
     * If we see IsRunning(), the thread might actually have gotten a Stop(),
     * but has not yet exited its Run routine and become STOPPING.  To plug this
     * hole, we need to check IsRunning() and also m_stopping, which is set in
     * our Stop() method.
     */
    if (IsRunning() == false || m_stopping == true) {
        QCC_LogError(ER_BUS_TRANSPORT_NOT_STARTED, ("WFDTransport::Connect(): Not running or stopping; exiting"));
        return ER_BUS_TRANSPORT_NOT_STARTED;
    }

    /*
     * If we pass the IsRunning() gate above, we must have a server accept
     * thread spinning up or shutting down but not yet joined.  Since the name
     * service is started before the server accept thread is spun up, and
     * deleted after it is joined, we must have a started name service or someone
     * isn't playing by the rules; so an assert is appropriate here.
     */
    assert(IpNameService::Instance().Started() && "WFDTransport::Connect(): IpNameService not started");

    /*
     * Parse and normalize the connectArgs.  When connecting to the outside
     * world, there are no reasonable defaults and so the addr and port keys
     * MUST be present.
     */
    qcc::String normSpec;
    map<qcc::String, qcc::String> argMap;
    status = NormalizeTransportSpec(connectSpec, normSpec, argMap);
    if (ER_OK != status) {
        QCC_LogError(status, ("WFDTransport::Connect(): Invalid WFD connect spec \"%s\"", connectSpec));
        return status;
    }

    qcc::String interface;
    bool wasP2P = false;

    /*
     * If we find a GUID in the argument map, it means a pre-association service
     * discovery event drove this connect.  There may or may not be an
     * associated physical network, no IP address, port, etc., so we probably
     * have a lot of work to do before we can attempt any kind of connection.
     * Since there is no addressing information, we need to conjure it up before
     * proceeding.
     */
    map<qcc::String, qcc::String>::iterator iter = argMap.find("guid");
    if (iter != argMap.end()) {
        QCC_DbgPrintf(("WFDTransport::Connect(): CreateTemporaryNetwork()"));
        wasP2P = true;

        /*
         * The first thing to do is to make sure the temporary network (AKA Wi-Fi
         * P2P Group) is created and ready to go.  It may or may not be there,
         * but we create it if we need it.
         */
        status = CreateTemporaryNetwork(iter->second, interface);
        if (status != ER_OK) {
            QCC_LogError(status, ("WFDTransport::Connect(): Could not create temporary network to \"%s\"", normSpec.c_str()));
            return status;
        }

        /*
         * We have a temporary network, but we have no idea what the IP address
         * and port of the remote endpoint is.  We have to discover that
         * information and then come up with a new connect spec that reflects
         * this new info.  We need to wait until we find an IP address and port
         * that corresponds to the daemon represented by the GUID.
         */
        qcc::String newSpec;
        QCC_DbgPrintf(("WFDTransport::Connect(): CreateConnectSpec()"));
        status = CreateConnectSpec(interface, iter->second, newSpec);
        if (status != ER_OK) {
            QCC_LogError(status, ("WFDTransport::Connect(): Could not get connection info for \"%s\"", normSpec.c_str()));
            return status;
        }

        /*
         * If we have rewritten the connect spec for the connection, we need to
         * reflect this in the normSpec and argMap that were originally derived
         * from the connectSpec passed in.
         */
        argMap.clear();
        normSpec.clear();

        QCC_DbgPrintf(("WFDTransport::Connect(): NormalizeTransportSpec()"));
        status = NormalizeTransportSpec(newSpec.c_str(), normSpec, argMap);
        if (ER_OK != status) {
            QCC_LogError(status, ("WFDTransport::Connect(): Invalid derived WFD connect spec \"%s\"", newSpec.c_str()));
            return status;
        }
    }

    /*
     * These fields (addr, port, family) are all guaranteed to be present now
     * and an underlying network (even if it is Wi-Fi P2P) is assumed to be
     * up and functioning.
     */
    IPAddress ipAddr(argMap.find("r4addr")->second);
    uint16_t port = StringToU32(argMap["r4port"]);

    /*
     * The semantics of the Connect method tell us that we want to connect to a
     * remote daemon.  WFD will happily allow us to connect to ourselves, but
     * this is not always possible in the various transports AllJoyn may use.
     * To avoid unnecessary differences, we do not allow a requested connection
     * to "ourself" to succeed.
     *
     * The code here is not a failsafe way to prevent this since thre are going
     * to be multiple processes involved that have no knowledge of what the
     * other is doing (for example, the wireless supplicant and this daemon).
     * This means we can't synchronize and there will be race conditions that
     * can cause the tests for selfness to fail.  The final check is made in the
     * bus hello protocol, which will abort the connection if it detects it is
     * conected to itself.  We just attempt to short circuit the process where
     * we can and not allow connections to proceed that will be bound to fail.
     *
     * One defintion of a connection to ourself is if we find that a listener
     * has has been started via a call to our own StartListener() with the same
     * connectSpec as we have now.  This is the simple case, but it also turns
     * out to be the uncommon case.
     *
     * It is perfectly legal to start a listener using the INADDR_ANY address,
     * which tells the system to listen for connections on any network interface
     * that happens to be up or that may come up in the future.  This is the
     * default listen address and is the most common case.  If this option has
     * been used, we expect to find a listener with a normalized adresss that
     * looks like "r4addr=0.0.0.0,port=y".  If we detect this kind of connectSpec
     * we have to look at the currently up interfaces and see if any of them
     * match the address provided in the connectSpec.  If so, we are attempting
     * to connect to ourself and we must fail that request.
     */
    char anyspec[64];
    snprintf(anyspec, sizeof(anyspec), "%s:r4addr=0.0.0.0,r4port=%u", GetTransportName(), port);

    qcc::String normAnySpec;
    map<qcc::String, qcc::String> normArgMap;
    status = NormalizeListenSpec(anyspec, normAnySpec, normArgMap);
    if (ER_OK != status) {
        QCC_LogError(status, ("WFDTransport::Connect(): Invalid INADDR_ANY connect spec"));
        return status;
    }

    /*
     * Look to see if we are already listening on the provided connectSpec
     * either explicitly or via the INADDR_ANY address.
     */
    QCC_DbgHLPrintf(("WFDTransport::Connect(): Checking for connection to self"));
    m_listenFdsLock.Lock(MUTEX_CONTEXT);
    bool anyEncountered = false;
    for (list<pair<qcc::String, SocketFd> >::iterator i = m_listenFds.begin(); i != m_listenFds.end(); ++i) {
        QCC_DbgHLPrintf(("WFDTransport::Connect(): Checking listenSpec %s", i->first.c_str()));

        /*
         * If the provided connectSpec is already explicitly listened to, it is
         * an error.
         */
        if (i->first == normSpec) {
            m_listenFdsLock.Unlock(MUTEX_CONTEXT);
            QCC_DbgHLPrintf(("WFDTransport::Connect(): Explicit connection to self"));
            return ER_BUS_ALREADY_LISTENING;
        }

        /*
         * If we are listening to INADDR_ANY and the supplied port, then we have
         * to look to the currently UP interfaces to decide if this call is bogus
         * or not.  Set a flag to remind us.
         */
        if (i->first == normAnySpec) {
            QCC_DbgHLPrintf(("WFDTransport::Connect(): Possible implicit connection to self detected"));
            anyEncountered = true;
        }
    }
    m_listenFdsLock.Unlock(MUTEX_CONTEXT);

    /*
     * If we are listening to INADDR_ANY, we are going to have to see if any
     * currently UP interfaces have an address that matches the connectSpec
     * addr.
     */
    if (anyEncountered) {
        QCC_DbgHLPrintf(("WFDTransport::Connect(): Checking for implicit connection to self"));
        std::vector<qcc::IfConfigEntry> entries;
        QStatus status = qcc::IfConfig(entries);

        /*
         * Only do the check for self-ness if we can get interfaces to check.
         * This is a non-fatal error since we know that there is an end-to-end
         * check happening in the bus hello exchange, so if there is a problem
         * it will simply be detected later.
         */
        if (status == ER_OK) {
            /*
             * Loop through the network interface entries looking for an UP
             * interface that has the same IP address as the one we're trying to
             * connect to.  We know any match on the address will be a hit since
             * we matched the port during the listener check above.  Since we
             * have a listener listening on *any* UP interface on the specified
             * port, a match on the interface address with the connect address
             * is a hit.
             */
            for (uint32_t i = 0; i < entries.size(); ++i) {
                QCC_DbgHLPrintf(("WFDTransport::Connect(): Checking interface %s", entries[i].m_name.c_str()));
                if (entries[i].m_flags & qcc::IfConfigEntry::UP) {
                    QCC_DbgHLPrintf(("WFDTransport::Connect(): Interface UP with addresss %s", entries[i].m_addr.c_str()));
                    IPAddress foundAddr(entries[i].m_addr);
                    if (foundAddr == ipAddr) {
                        QCC_DbgHLPrintf(("WFDTransport::Connect(): Attempted connection to self; exiting"));
                        return ER_BUS_ALREADY_LISTENING;
                    }
                }
            }
        }
    }

    /*
     * This is a new not previously satisfied connection request, so attempt
     * to connect to the remote WFD address and port specified in the connectSpec.
     */
    SocketFd sockFd = -1;
    status = Socket(QCC_AF_INET, QCC_SOCK_STREAM, sockFd);
    if (status == ER_OK) {
        /* Turn off Nagle */
        status = SetNagle(sockFd, false);
    }

    if (status == ER_OK) {
        /*
         * We got a socket, now tell WFD to connect to the remote address and
         * port.
         */
        status = qcc::Connect(sockFd, ipAddr, port);
        if (status == ER_OK) {
            /*
             * We now have a WFD connection established, but DBus (the wire
             * protocol which we are using) requires that every connection,
             * irrespective of transport, start with a single zero byte.  This
             * is so that the Unix-domain socket transport used by DBus can pass
             * SCM_RIGHTS out-of-band when that byte is sent.
             */
            uint8_t nul = 0;
            size_t sent;

            status = Send(sockFd, &nul, 1, sent);
            if (status != ER_OK) {
                QCC_LogError(status, ("WFDTransport::Connect(): Failed to send initial NUL byte"));
            }
            isConnected = true;
        } else {
            QCC_LogError(status, ("WFDTransport::Connect(): Failed"));
        }
    } else {
        QCC_LogError(status, ("WFDTransport::Connect(): qcc::Socket() failed"));
    }

    WFDEndpoint* conn = NULL;
    if (status == ER_OK) {
        /*
         * The underlying transport mechanism is started, but we need to create
         * a WFDEndpoint object that will orchestrate the movement of data
         * across the transport.
         */
        conn = new WFDEndpoint(this, m_bus, false, normSpec, sockFd, ipAddr, port);

        /*
         * On the active side of a connection, we don't need an authentication
         * thread to run since we have the caller thread to fill that role.
         */
        conn->SetActive();
        conn->SetAuthenticating();

        /*
         * Initialize the "features" for this endpoint
         */
        conn->GetFeatures().isBusToBus = true;
        conn->GetFeatures().allowRemote = m_bus.GetInternal().AllowRemoteMessages();
        conn->GetFeatures().handlePassing = false;

        qcc::String authName;
        qcc::String redirection;

        /*
         * This is a little tricky.  We usually manage endpoints in one place
         * using the main server accept loop thread.  This thread expects
         * endpoints to have an RX thread and a TX thread running, and these
         * threads are expected to run through the EndpointExit function when
         * they are stopped.  The general endpoint management uses these
         * mechanisms.  However, we are about to get into a state where we are
         * off trying to start an endpoint, but we are using another thread
         * which has called into WFDTransport::Connect().  We are about to do
         * blocking I/O in the authentication establishment dance, but we can't
         * just kill off this thread since it isn't ours for the whacking.  If
         * the transport is stopped, we do however need a way to stop an
         * in-process establishment.  It's not reliable to just close a socket
         * out from uder a thread, so we really need to Alert() the thread
         * making the blocking calls.  So we keep a separate list of Thread*
         * that may need to be Alert()ed and run through that list when the
         * transport is stopping.  This will cause the I/O calls in Establish()
         * to return and we can then allow the "external" threads to return
         * and avoid nasty deadlocks.
         */
        Thread* thread = GetThread();
        m_endpointListLock.Lock(MUTEX_CONTEXT);
        m_activeEndpointsThreadList.insert(thread);
        m_endpointListLock.Unlock(MUTEX_CONTEXT);

        /*
         * Go ahead and do the authentication in the context of this thread.  Even
         * though we don't have the server accept loop thread watching this endpoint
         * we keep we keep the states consistent since the endpoint will eventually
         * to there.
         */
        status = conn->Establish("ANONYMOUS", authName, redirection);
        if (status == ER_OK) {
            conn->SetListener(this);
            status = conn->Start();
            if (status == ER_OK) {
                conn->SetEpStarted();
                conn->SetAuthDone();
            } else {
                conn->SetEpFailed();
                conn->SetAuthDone();
            }
        }

        /*
         * If we have a successful authentication, we pass the connection off to the
         * server accept loop to manage.
         */
        if (status == ER_OK) {
            m_endpointListLock.Lock(MUTEX_CONTEXT);
            m_endpointList.insert(conn);
            m_endpointListLock.Unlock(MUTEX_CONTEXT);
        } else {
            QCC_LogError(status, ("WFDTransport::Connect(): Starting the WFDEndpoint failed"));

            /*
             * Although the destructor of a remote endpoint includes a Stop and Join
             * call, there are no running threads since Start() failed.
             */
            delete conn;
            conn = NULL;
        }

        /*
         * In any case, we are done with blocking I/O on the current thread, so
         * we need to remove its pointer from the list we kept around to break it
         * out of blocking I/O.  If we were successful, the WFDEndpoint was passed
         * to the m_endpointList, where the main server accept loop will deal with
         * it using its RX and TX thread-based mechanisms.  If we were unsuccessful
         * the WFDEndpoint was destroyed and we will return an error below after
         * cleaning up the underlying socket.
         */
        m_endpointListLock.Lock(MUTEX_CONTEXT);
        set<Thread*>::iterator i = find(m_activeEndpointsThreadList.begin(), m_activeEndpointsThreadList.end(), thread);
        assert(i != m_activeEndpointsThreadList.end() && "WFDTransport::Connect(): Thread* not on m_activeEndpointsThreadList");
        m_activeEndpointsThreadList.erase(i);
        m_endpointListLock.Unlock(MUTEX_CONTEXT);
    }

    /*
     * If we got an error, we need to cleanup the socket and zero out the
     * returned endpoint.  If we got this connection and its endpoint up without
     * a problem, we return a pointer to the new endpoint.  We aren't going to
     * clean it up since it is an active connection, so we can safely pass the
     * endoint back up to higher layers.
     */
    if (status != ER_OK) {
        if (isConnected) {
            qcc::Shutdown(sockFd);
        }
        if (sockFd >= 0) {
            qcc::Close(sockFd);
        }
        if (newep) {
            *newep = NULL;
        }

        /*
         * XXX FIXME Very confusing to see GO handle in connect.
         */
        QCC_DbgPrintf(("WFDTransport::Connect(): ReleaseLinkAsync()"));
        status = m_p2pHelperInterface->ReleaseLinkAsync(m_goHandle);
        if (status != ER_OK) {
            QCC_LogError(status, ("WFDTransport::Connect(): ReleaseLink fails"));
        }

    } else {
        if (wasP2P) {
            QCC_DbgPrintf(("WFDTransport::Connect(): RememberP2PConnection()"));
            RememberP2PConnection(conn, m_goHandle, interface);
        }

        if (newep) {
            assert(conn && "WFDTransport::Connect(): If the conn is up, the conn pointer should be non-NULL");
            *newep = conn;
        }
    }

    return status;
}

QStatus WFDTransport::Disconnect(const char* connectSpec)
{
    QCC_DbgHLPrintf(("WFDTransport::Disconnect(): %s", connectSpec));

    /*
     * We only want to allow this call to proceed if we have a running server
     * accept thread that isn't in the process of shutting down.  We use the
     * thread response from IsRunning to give us an idea of what our server
     * accept (Run) thread is doing, and by extension the endpoint threads which
     * must be running to properly clean up.  See the comment in Start() for
     * details about what IsRunning actually means, which might be subtly
     * different from your intuitition.
     *
     * If we see IsRunning(), the thread might actually have gotten a Stop(),
     * but has not yet exited its Run routine and become STOPPING.  To plug this
     * hole, we need to check IsRunning() and also m_stopping, which is set in
     * our Stop() method.
     */
    if (IsRunning() == false || m_stopping == true) {
        QCC_LogError(ER_BUS_TRANSPORT_NOT_STARTED, ("WFDTransport::Disconnect(): Not running or stopping; exiting"));
        return ER_BUS_TRANSPORT_NOT_STARTED;
    }

    /*
     * If we pass the IsRunning() gate above, we must have a server accept
     * thread spinning up or shutting down but not yet joined.  Since the name
     * service is started before the server accept thread is spun up, and
     * stopped after it is stopped, we must have a started name service or
     * someone isn't playing by the rules; so an assert is appropriate here.
     */
    assert(IpNameService::Instance().Started() && "WFDTransport::Disconnect(): IpNameService not started");

    /*
     * Higher level code tells us which connection is refers to by giving us the
     * same connect spec it used in the Connect() call.  We have to determine the
     * address and port in exactly the same way
     */
    qcc::String normSpec;
    map<qcc::String, qcc::String> argMap;
    QStatus status = NormalizeTransportSpec(connectSpec, normSpec, argMap);
    if (ER_OK != status) {
        QCC_LogError(status, ("WFDTransport::Disconnect(): Invalid WFD connect spec \"%s\"", connectSpec));
        return status;
    }

    IPAddress ipAddr(argMap.find("r4addr")->second); // Guaranteed to be there.
    uint16_t port = StringToU32(argMap["r4port"]);   // Guaranteed to be there.

    /*
     * Stop the remote endpoint.  Be careful here since calling Stop() on the
     * WFDEndpoint is going to cause the transmit and receive threads of the
     * underlying RemoteEndpoint to exit, which will cause our EndpointExit()
     * to be called, which will walk the list of endpoints and delete the one
     * we are stopping.  Once we poke ep->Stop(), the pointer to ep must be
     * considered dead.
     */
    status = ER_BUS_BAD_TRANSPORT_ARGS;
    m_endpointListLock.Lock(MUTEX_CONTEXT);
    for (set<WFDEndpoint*>::iterator i = m_endpointList.begin(); i != m_endpointList.end(); ++i) {
        if ((*i)->GetPort() == port && (*i)->GetIPAddress() == ipAddr) {
            WFDEndpoint* ep = *i;
            ep->SetSuddenDisconnect(false);
            m_endpointListLock.Unlock(MUTEX_CONTEXT);

            QCC_DbgPrintf(("WFDTransport::Disconnect(): GetP2pInfoForEndpoint()"));
            P2PConnectionInfo* info = GetP2PInfoForEndpoint(ep);
            assert(info && "WFDTransport::Disconnect(): Can't find info for supposedly alive P2P connection");

            QCC_DbgPrintf(("WFDTransport::Disconnect(): ReleaseLinkAsync()"));
            QStatus status = m_p2pHelperInterface->ReleaseLinkAsync(info->GetHandle());
            if (status != ER_OK) {
                QCC_LogError(status, ("WFDTransport::Disconnect(): Cannot ReleaseLinkAsync"));
            }

            /*
             * Hideous, horrifying use of GO handle name even in STA.
             */
            m_goHandle = -1;

            qcc::String interface = info->GetInterface();
            QCC_DbgPrintf(("WFDTransport::Disconnect(): CloseInterface()"));
            status = IpNameService::Instance().CloseInterface(TRANSPORT_WFD, interface);
            if (status != ER_OK) {
                QCC_LogError(status, ("WFDTransport::Disconnect(): Cannot close interface \"%s\"", interface.c_str()));
            }

            return ep->Stop();
        }
    }
    m_endpointListLock.Unlock(MUTEX_CONTEXT);
    return status;
}

QStatus WFDTransport::StartListen(const char* listenSpec)
{
    QCC_DbgPrintf(("WFDTransport::StartListen()"));

    /*
     * We only want to allow this call to proceed if we have a running server
     * accept thread that isn't in the process of shutting down.  We use the
     * thread response from IsRunning to give us an idea of what our server
     * accept (Run) thread is doing.  See the comment in Start() for details
     * about what IsRunning actually means, which might be subtly different from
     * your intuitition.
     *
     * If we see IsRunning(), the thread might actually have gotten a Stop(),
     * but has not yet exited its Run routine and become STOPPING.  To plug this
     * hole, we need to check IsRunning() and also m_stopping, which is set in
     * our Stop() method.
     */
    if (IsRunning() == false || m_stopping == true) {
        QCC_LogError(ER_BUS_TRANSPORT_NOT_STARTED, ("WFDTransport::StartListen(): Not running or stopping; exiting"));
        return ER_BUS_TRANSPORT_NOT_STARTED;
    }

    /*
     * Normalize the listen spec.  Although this looks like a connectSpec it is
     * different in that reasonable defaults are possible.  We do the
     * normalization here so we can report an error back to the caller.
     */
    qcc::String normSpec;
    map<qcc::String, qcc::String> argMap;
    QStatus status = NormalizeListenSpec(listenSpec, normSpec, argMap);
    if (status != ER_OK) {
        QCC_LogError(status, ("WFDTransport::StartListen(): Invalid WFD listen spec \"%s\"", listenSpec));
        return status;
    }

    QCC_DbgPrintf(("WFDTransport::StartListen(): r4addr = \"%s\", r4port = \"%s\"",
                   argMap["r4addr"].c_str(), argMap["r4port"].c_str()));

    /*
     * The daemon code is in a state where it lags in functionality a bit with
     * respect to the common code.  Common supports the use of IPv6 addresses
     * but the name service is not quite ready for prime time.  Until the name
     * service can properly distinguish between various cases, we fail any
     * request to listen on an IPv6 address.
     */
    IPAddress ipAddress;
    status = ipAddress.SetAddress(argMap["r4addr"].c_str());
    if (status != ER_OK) {
        QCC_LogError(status, ("WFDTransport::StartListen(): Unable to SetAddress(\"%s\")", argMap["r4addr"].c_str()));
        return status;
    }

    if (ipAddress.IsIPv6()) {
        status = ER_INVALID_ADDRESS;
        QCC_LogError(status, ("WFDTransport::StartListen(): IPv6 address (\"%s\") in \"r4addr\" not allowed", argMap["r4addr"].c_str()));
        return status;
    }

    /*
     * Because we are sending a *request* to start listening on a given
     * normalized listen spec to another thread, and the server thread starts
     * and stops listening on given listen specs when it decides to eventually
     * run, it is be possible for a calling thread to send multiple requests to
     * start or stop listening on the same listenSpec before the server thread
     * responds.
     *
     * In order to deal with these two timelines, we keep a list of normalized
     * listenSpecs that we have requested to be started, and not yet requested
     * to be removed.  This list (the mListenSpecs) must be consistent with
     * client requests to start and stop listens.  This list is not necessarily
     * consistent with what is actually being listened on.  That is a separate
     * list called mListenFds.
     *
     * So, check to see if someone has previously requested that the address and
     * port in question be listened on.  We need to do this here to be able to
     * report an error back to the caller.
     */
    m_listenSpecsLock.Lock(MUTEX_CONTEXT);
    for (list<qcc::String>::iterator i = m_listenSpecs.begin(); i != m_listenSpecs.end(); ++i) {
        if (*i == normSpec) {
            m_listenSpecsLock.Unlock(MUTEX_CONTEXT);
            return ER_BUS_ALREADY_LISTENING;
        }
    }
    m_listenSpecsLock.Unlock(MUTEX_CONTEXT);

    QueueStartListen(normSpec);
    return ER_OK;
}

void WFDTransport::QueueStartListen(qcc::String& normSpec)
{
    QCC_DbgPrintf(("WFDTransport::QueueStartListen()"));

    /*
     * In order to start a listen, we send the server accept thread a message
     * containing the START_LISTEN_INSTANCE request code and the normalized
     * listen spec which specifies the address and port instance to listen on.
     */
    ListenRequest listenRequest;
    listenRequest.m_requestOp = START_LISTEN_INSTANCE;
    listenRequest.m_requestParam = normSpec;

    m_listenRequestsLock.Lock(MUTEX_CONTEXT);
    m_listenRequests.push(listenRequest);
    m_listenRequestsLock.Unlock(MUTEX_CONTEXT);

    /*
     * Wake the server accept loop thread up so it will process the request we
     * just queued.
     */
    Alert();
}

void WFDTransport::DoStartListen(qcc::String& normSpec)
{
    QCC_DbgPrintf(("WFDTransport::DoStartListen()"));

    /*
     * Since the name service is created before the server accept thread is spun
     * up, and stopped when it is stopped, we must have a started name service or
     * someone isn't playing by the rules; so an assert is appropriate here.
     */
    assert(IpNameService::Instance().Started() && "WFDTransport::DoStartListen(): IpNameService not started");

    /*
     * Parse the normalized listen spec.  The easiest way to do this is to
     * re-normalize it.  If there's an error at this point, we have done
     * something wrong since the listen spec was presumably successfully
     * normalized before sending it in -- so we assert.
     */
    qcc::String spec;
    map<qcc::String, qcc::String> argMap;
    QStatus status = NormalizeListenSpec(normSpec.c_str(), spec, argMap);
    assert(status == ER_OK && "WFDTransport::DoStartListen(): Invalid WFD listen spec");

    QCC_DbgPrintf(("WFDTransport::DoStartListen(): r4addr = \"%s\", r4port = \"%s\"",
                   argMap["r4addr"].c_str(), argMap["r4port"].c_str()));

    m_listenFdsLock.Lock(MUTEX_CONTEXT);

    /*
     * Figure out what local address and port the listener should use.
     */
    IPAddress listenAddr(argMap["r4addr"]);
    uint16_t listenPort = StringToU32(argMap["r4port"]);
    bool ephemeralPort = (listenPort == 0);

    /*
     * If we're going to listen on an address, we are going to listen on a
     * corresponding network interface.  We need to convince the name service to
     * send advertisements out over that interface, or nobody will know to
     * connect to the listening daemon.  The expected use case is that the
     * daemon does exactly one StartListen() which listens to INADDR_ANY
     * (listens for inbound connections over any interface) and the name service
     * is controlled by a separate configuration item that selects which
     * interfaces are used in discovery.  Since IP addresses in a mobile
     * environment are dynamic, listening on the ANY address is the only option
     * that really makes sense, and this is the only case in which the current
     * implementation will really work.
     *
     * So, we need to get the configuration item telling us which network
     * interfaces we should run the name service over.  The item can specify an
     * IP address, in which case the name service waits until that particular
     * address comes up and then uses the corresponding net device if it is
     * multicast-capable.  The item can also specify an interface name.  In this
     * case the name service waits until it finds the interface IFF_UP and
     * multicast capable with an assigned IP address and then starts using the
     * interface.  If the configuration item contains "*" (the wildcard) it is
     * interpreted as meaning all multicast-capable interfaces.  If the
     * configuration item is empty (not assigned in the configuration database)
     * it defaults to "*".
     */
    qcc::String interfaces = DaemonConfig::Access()->Get("ip_name_service/property@interfaces", INTERFACES_DEFAULT);

    while (interfaces.size()) {
        qcc::String currentInterface;
        size_t i = interfaces.find(",");
        if (i != qcc::String::npos) {
            currentInterface = interfaces.substr(0, i);
            interfaces = interfaces.substr(i + 1, interfaces.size() - i - 1);
        } else {
            currentInterface = interfaces;
            interfaces.clear();
        }

        /*
         * If we were given an IP address, use it to find the interface names
         * otherwise use the interface name that was specified. Note we need
         * to disallow hostnames otherwise SetAddress will attempt to treat
         * the interface name as a host name and start doing DNS lookups.
         */
        IPAddress currentAddress;
        if (currentAddress.SetAddress(currentInterface, false) == ER_OK) {
            status = IpNameService::Instance().OpenInterface(TRANSPORT_WFD, currentAddress);
        } else {
            status = IpNameService::Instance().OpenInterface(TRANSPORT_WFD, currentInterface);
        }
        if (status != ER_OK) {
            QCC_LogError(status, ("WFDTransport::DoStartListen(): OpenInterface() failed for %s", currentInterface.c_str()));
        }
    }
    /*
     * We have the name service work out of the way, so we can now create the
     * WFD listener sockets and set SO_REUSEADDR/SO_REUSEPORT so we don't have
     * to wait for four minutes to relaunch the daemon if it crashes.
     */
    SocketFd listenFd = -1;
    status = Socket(QCC_AF_INET, QCC_SOCK_STREAM, listenFd);
    if (status != ER_OK) {
        m_listenFdsLock.Unlock(MUTEX_CONTEXT);
        QCC_LogError(status, ("WFDTransport::DoStartListen(): Socket() failed"));
        return;
    }
    /*
     * Set the SO_REUSEADDR socket option so we don't have to wait for four
     * minutes while the endpoint is in TIME_WAIT if we crash (or control-C).
     */
    status = qcc::SetReuseAddress(listenFd, true);
    if (status != ER_OK && status != ER_NOT_IMPLEMENTED) {
        m_listenFdsLock.Unlock(MUTEX_CONTEXT);
        QCC_LogError(status, ("WFDTransport::DoStartListen(): SetReuseAddress() failed"));
        qcc::Close(listenFd);
        return;
    }
    /*
     * We call accept in a loop so we need the listenFd to non-blocking
     */
    status = qcc::SetBlocking(listenFd, false);
    if (status != ER_OK) {
        m_listenFdsLock.Unlock(MUTEX_CONTEXT);
        QCC_LogError(status, ("WFDTransport::DoStartListen(): SetBlocking() failed"));
        qcc::Close(listenFd);
        return;
    }
    /*
     * Bind the socket to the listen address and start listening for incoming
     * connections on it.
     */
    if (ephemeralPort) {
        /*
         * First try binding to the default port
         */
        listenPort = PORT_DEFAULT;
        status = Bind(listenFd, listenAddr, listenPort);
        if (status != ER_OK) {
            listenPort = 0;
            status = Bind(listenFd, listenAddr, listenPort);
        }
    } else {
        status = Bind(listenFd, listenAddr, listenPort);
    }

    if (status == ER_OK) {
        /*
         * If the port was not set (or set to zero) then we will have bound an ephemeral port. If
         * so call GetLocalAddress() to update the connect spec with the port allocated by bind.
         */
        if (ephemeralPort) {
            qcc::GetLocalAddress(listenFd, listenAddr, listenPort);
            normSpec = "wfd:addr=" + argMap["r4addr"] + "," ",r4port=" + U32ToString(listenPort);
        }
        status = qcc::Listen(listenFd, MAX_LISTEN_CONNECTIONS);
        if (status == ER_OK) {
            QCC_DbgPrintf(("WFDTransport::DoStartListen(): Listening on %s/%d", argMap["r4addr"].c_str(), listenPort));
            m_listenFds.push_back(pair<qcc::String, SocketFd>(normSpec, listenFd));
        } else {
            QCC_LogError(status, ("WFDTransport::DoStartListen(): Listen failed"));
        }
    } else {
        QCC_LogError(status, ("WFDTransport::DoStartListen(): Failed to bind to %s/%d", listenAddr.ToString().c_str(), listenPort));
    }

    /*
     * The IP name service is very flexible about what to advertise.  It assumes
     * that a so-called transport is going to be doing the advertising.  An IP
     * transport, by definition, has a reliable data transmission capability and
     * an unreliable data transmission capability.  In the IP world, reliable
     * data is sent using WFD and unreliable data is sent using UDP (the Packet
     * Engine in the AllJoyn world).  Also, IP implies either IPv4 or IPv6
     * addressing.
     *
     * In the WFDTransport, we only support reliable data transfer over IPv4
     * addresses, so we leave all of the other possibilities turned off (provide
     * a zero port).  Remember the port we enabled so we can re-enable the name
     * service if listeners come and go.
     */
    m_listenPort = listenPort;
    IpNameService::Instance().Enable(TRANSPORT_WFD, listenPort, 0, 0, 0);
    m_listenFdsLock.Unlock(MUTEX_CONTEXT);

    /*
     * Signal the (probably) waiting run thread so it will wake up and add this
     * new socket to its list of sockets it is waiting for connections on.
     */
    if (status == ER_OK) {
        Alert();
    }
}

QStatus WFDTransport::StopListen(const char* listenSpec)
{
    QCC_DbgPrintf(("WFDTransport::StopListen()"));

    /*
     * We only want to allow this call to proceed if we have a running server
     * accept thread that isn't in the process of shutting down.  We use the
     * thread response from IsRunning to give us an idea of what our server
     * accept (Run) thread is doing.  See the comment in Start() for details
     * about what IsRunning actually means, which might be subtly different from
     * your intuitition.
     *
     * If we see IsRunning(), the thread might actually have gotten a Stop(),
     * but has not yet exited its Run routine and become STOPPING.  To plug this
     * hole, we need to check IsRunning() and also m_stopping, which is set in
     * our Stop() method.
     */
    if (IsRunning() == false || m_stopping == true) {
        QCC_LogError(ER_BUS_TRANSPORT_NOT_STARTED, ("WFDTransport::StopListen(): Not running or stopping; exiting"));
        return ER_BUS_TRANSPORT_NOT_STARTED;
    }

    /*
     * Normalize the listen spec.  We are going to use the name string that was
     * put together for the StartListen call to find the listener instance to
     * stop, so we need to do it exactly the same way.
     */
    qcc::String normSpec;
    map<qcc::String, qcc::String> argMap;
    QStatus status = NormalizeListenSpec(listenSpec, normSpec, argMap);
    if (status != ER_OK) {
        QCC_LogError(status, ("WFDTransport::StopListen(): Invalid WFD listen spec \"%s\"", listenSpec));
        return status;
    }

    /*
     * Because we are sending a *request* to stop listening on a given
     * normalized listen spec to another thread, and the server thread starts
     * and stops listening on given listen specs when it decides to eventually
     * run, it is be possible for a calling thread to send multiple requests to
     * start or stop listening on the same listenSpec before the server thread
     * responds.
     *
     * In order to deal with these two timelines, we keep a list of normalized
     * listenSpecs that we have requested to be started, and not yet requested
     * to be removed.  This list (the mListenSpecs) must be consistent with
     * client requests to start and stop listens.  This list is not necessarily
     * consistent with what is actually being listened on.  That is reflected by
     * a separate list called mListenFds.
     *
     * We consult the list of listen spects for duplicates when starting to
     * listen, and we make sure that a listen spec is on the list before
     * queueing a request to stop listening.  Asking to stop listening on a
     * listen spec we aren't listening on is not an error, since the goal of the
     * user is to not listen on a given address and port -- and we aren't.
     */
    m_listenSpecsLock.Lock(MUTEX_CONTEXT);
    for (list<qcc::String>::iterator i = m_listenSpecs.begin(); i != m_listenSpecs.end(); ++i) {
        if (*i == normSpec) {
            m_listenSpecs.erase(i);
            QueueStopListen(normSpec);
            break;
        }
    }
    m_listenSpecsLock.Unlock(MUTEX_CONTEXT);

    return ER_OK;
}

void WFDTransport::QueueStopListen(qcc::String& normSpec)
{
    QCC_DbgPrintf(("WFDTransport::QueueStopListen()"));

    /*
     * In order to stop a listen, we send the server accept thread a message
     * containing the STOP_LISTEN_INTANCE request code and the normalized listen
     * spec which specifies the address and port instance to stop listening on.
     */
    ListenRequest listenRequest;
    listenRequest.m_requestOp = STOP_LISTEN_INSTANCE;
    listenRequest.m_requestParam = normSpec;

    m_listenRequestsLock.Lock(MUTEX_CONTEXT);
    m_listenRequests.push(listenRequest);
    m_listenRequestsLock.Unlock(MUTEX_CONTEXT);

    /*
     * Wake the server accept loop thread up so it will process the request we
     * just queued.
     */
    Alert();
}

void WFDTransport::DoStopListen(qcc::String& normSpec)
{
    QCC_DbgPrintf(("WFDTransport::DoStopListen()"));

    /*
     * Since the name service is started before the server accept thread is spun
     * up, and stopped after it is stopped, we must have a started name service or
     * someone isn't playing by the rules; so an assert is appropriate here.
     */
    assert(IpNameService::Instance().Started() && "WFDTransport::DoStopListen(): IpNameService not started");

    /*
     * Find the (single) listen spec and remove it from the list of active FDs
     * used by the server accept loop (run thread).  This is okay to do since
     * we are assuming that, since we should only be called in the context of
     * the server accept loop, it knows that an FD will be deleted here.
     */
    m_listenFdsLock.Lock(MUTEX_CONTEXT);
    qcc::SocketFd stopFd = -1;
    bool found = false;
    for (list<pair<qcc::String, SocketFd> >::iterator i = m_listenFds.begin(); i != m_listenFds.end(); ++i) {
        if (i->first == normSpec) {
            stopFd = i->second;
            m_listenFds.erase(i);
            found = true;
            break;
        }
    }
    m_listenFdsLock.Unlock(MUTEX_CONTEXT);

    /*
     * If we took a socketFD off of the list of active FDs, we need to tear it
     * down and alert the server accept loop that the list of FDs on which it
     * is listening has changed.
     */
    if (found) {
        qcc::Shutdown(stopFd);
        qcc::Close(stopFd);
    }
}

bool WFDTransport::NewDiscoveryOp(DiscoveryOp op, qcc::String namePrefix, bool& isFirst)
{
    QCC_DbgPrintf(("WFDTransport::NewDiscoveryOp()"));

    bool first = false;

    if (op == ENABLE_DISCOVERY) {
        QCC_DbgPrintf(("WFDTransport::NewDiscoveryOp(): Registering discovery of namePrefix \"%s\"", namePrefix.c_str()));
        first = m_advertising.empty();
        m_discovering.push_back(namePrefix);
    } else {
        list<qcc::String>::iterator i = find(m_discovering.begin(), m_discovering.end(), namePrefix);
        if (i == m_discovering.end()) {
            QCC_DbgPrintf(("WFDTransport::NewDiscoveryOp(): Cancel of non-existent namePrefix \"%s\"", namePrefix.c_str()));
        } else {
            QCC_DbgPrintf(("WFDTransport::NewDiscoveryOp(): Unregistering discovery of namePrefix \"%s\"", namePrefix.c_str()));
            m_discovering.erase(i);
        }
    }

    isFirst = first;
    return m_discovering.empty();
}

bool WFDTransport::NewAdvertiseOp(AdvertiseOp op, qcc::String name, bool& isFirst)
{
    QCC_DbgPrintf(("WFDTransport::NewAdvertiseOp()"));

    bool first = false;

    if (op == ENABLE_ADVERTISEMENT) {
        QCC_DbgPrintf(("WFDTransport::NewAdvertiseOp(): Registering advertisement of namePrefix \"%s\"", name.c_str()));
        first = m_advertising.empty();
        m_advertising.push_back(name);
    } else {
        list<qcc::String>::iterator i = find(m_advertising.begin(), m_advertising.end(), name);
        if (i == m_advertising.end()) {
            QCC_DbgPrintf(("WFDTransport::NewAdvertiseOp(): Cancel of non-existent name \"%s\"", name.c_str()));
        } else {
            QCC_DbgPrintf(("WFDTransport::NewAdvertiseOp(): Unregistering advertisement of namePrefix \"%s\"", name.c_str()));
            m_advertising.erase(i);
        }
    }

    isFirst = first;
    return m_advertising.empty();
}

bool WFDTransport::NewListenOp(ListenOp op, qcc::String normSpec)
{
    QCC_DbgPrintf(("WFDTransport::NewListenOp()"));

    if (op == START_LISTEN) {
        QCC_DbgPrintf(("WFDTransport::NewListenOp(): Registering listen of normSpec \"%s\"", normSpec.c_str()));
        m_listening.push_back(normSpec);

    } else {
        list<qcc::String>::iterator i = find(m_listening.begin(), m_listening.end(), normSpec);
        if (i == m_listening.end()) {
            QCC_DbgPrintf(("WFDTransport::NewAdvertiseOp(): StopListen of non-existent spec \"%s\"", normSpec.c_str()));
        } else {
            QCC_DbgPrintf(("WFDTransport::NewAdvertiseOp(): StopListen of normSpec \"%s\"", normSpec.c_str()));
            m_listening.erase(i);
        }
    }

    return m_listening.empty();
}

void WFDTransport::EnableDiscovery(const char* namePrefix)
{
    QCC_DbgPrintf(("WFDTransport::EnableDiscovery()"));

    /*
     * We only want to allow this call to proceed if we have a running server
     * accept thread that isn't in the process of shutting down.  We use the
     * thread response from IsRunning to give us an idea of what our server
     * accept (Run) thread is doing.  See the comment in Start() for details
     * about what IsRunning actually means, which might be subtly different from
     * your intuitition.
     *
     * If we see IsRunning(), the thread might actually have gotten a Stop(),
     * but has not yet exited its Run routine and become STOPPING.  To plug this
     * hole, we need to check IsRunning() and also m_stopping, which is set in
     * our Stop() method.
     */
    if (IsRunning() == false || m_stopping == true) {
        QCC_LogError(ER_BUS_TRANSPORT_NOT_STARTED, ("WFDTransport::EnableDiscovery(): Not running or stopping; exiting"));
        return;
    }

    QueueEnableDiscovery(namePrefix);
}

void WFDTransport::QueueEnableDiscovery(const char* namePrefix)
{
    QCC_DbgPrintf(("WFDTransport::QueueEnableDiscovery()"));

    ListenRequest listenRequest;
    listenRequest.m_requestOp = ENABLE_DISCOVERY_INSTANCE;
    listenRequest.m_requestParam = namePrefix;

    m_listenRequestsLock.Lock(MUTEX_CONTEXT);
    m_listenRequests.push(listenRequest);
    m_listenRequestsLock.Unlock(MUTEX_CONTEXT);

    /*
     * Wake the server accept loop thread up so it will process the request we
     * just queued.
     */
    Alert();
}

void WFDTransport::DisableDiscovery(const char* namePrefix)
{
    QCC_DbgPrintf(("WFDTransport::DisableDiscovery()"));

    /*
     * We only want to allow this call to proceed if we have a running server
     * accept thread that isn't in the process of shutting down.  We use the
     * thread response from IsRunning to give us an idea of what our server
     * accept (Run) thread is doing.  See the comment in Start() for details
     * about what IsRunning actually means, which might be subtly different from
     * your intuitition.
     *
     * If we see IsRunning(), the thread might actually have gotten a Stop(),
     * but has not yet exited its Run routine and become STOPPING.  To plug this
     * hole, we need to check IsRunning() and also m_stopping, which is set in
     * our Stop() method.
     */
    if (IsRunning() == false || m_stopping == true) {
        QCC_LogError(ER_BUS_TRANSPORT_NOT_STARTED, ("WFDTransport::DisbleDiscovery(): Not running or stopping; exiting"));
        return;
    }

    QueueDisableDiscovery(namePrefix);
}

void WFDTransport::QueueDisableDiscovery(const char* namePrefix)
{
    QCC_DbgPrintf(("WFDTransport::QueueDisableDiscovery()"));

    ListenRequest listenRequest;
    listenRequest.m_requestOp = DISABLE_DISCOVERY_INSTANCE;
    listenRequest.m_requestParam = namePrefix;

    m_listenRequestsLock.Lock(MUTEX_CONTEXT);
    m_listenRequests.push(listenRequest);
    m_listenRequestsLock.Unlock(MUTEX_CONTEXT);

    /*
     * Wake the server accept loop thread up so it will process the request we
     * just queued.
     */
    Alert();
}

QStatus WFDTransport::EnableAdvertisement(const qcc::String& advertiseName)
{
    QCC_DbgPrintf(("WFDTransport::EnableAdvertisement()"));

    /*
     * We only want to allow this call to proceed if we have a running server
     * accept thread that isn't in the process of shutting down.  We use the
     * thread response from IsRunning to give us an idea of what our server
     * accept (Run) thread is doing.  See the comment in Start() for details
     * about what IsRunning actually means, which might be subtly different from
     * your intuitition.
     *
     * If we see IsRunning(), the thread might actually have gotten a Stop(),
     * but has not yet exited its Run routine and become STOPPING.  To plug this
     * hole, we need to check IsRunning() and also m_stopping, which is set in
     * our Stop() method.
     */
    if (IsRunning() == false || m_stopping == true) {
        QCC_LogError(ER_BUS_TRANSPORT_NOT_STARTED, ("WFDTransport::EnableAdvertisement(): Not running or stopping; exiting"));
        return ER_BUS_TRANSPORT_NOT_STARTED;
    }

    QueueEnableAdvertisement(advertiseName);
    return ER_OK;
}

void WFDTransport::QueueEnableAdvertisement(const qcc::String& advertiseName)
{
    QCC_DbgPrintf(("WFDTransport::QueueEnableAdvertisement()"));

    ListenRequest listenRequest;
    listenRequest.m_requestOp = ENABLE_ADVERTISEMENT_INSTANCE;
    listenRequest.m_requestParam = advertiseName;

    m_listenRequestsLock.Lock(MUTEX_CONTEXT);
    m_listenRequests.push(listenRequest);
    m_listenRequestsLock.Unlock(MUTEX_CONTEXT);

    /*
     * Wake the server accept loop thread up so it will process the request we
     * just queued.
     */
    Alert();
}

void WFDTransport::DisableAdvertisement(const qcc::String& advertiseName, bool nameListEmpty)
{
    QCC_DbgPrintf(("WFDTransport::DisableAdvertisement()"));

    /*
     * We only want to allow this call to proceed if we have a running server
     * accept thread that isn't in the process of shutting down.  We use the
     * thread response from IsRunning to give us an idea of what our server
     * accept (Run) thread is doing.  See the comment in Start() for details
     * about what IsRunning actually means, which might be subtly different from
     * your intuitition.
     *
     * If we see IsRunning(), the thread might actually have gotten a Stop(),
     * but has not yet exited its Run routine and become STOPPING.  To plug this
     * hole, we need to check IsRunning() and also m_stopping, which is set in
     * our Stop() method.
     */
    if (IsRunning() == false || m_stopping == true) {
        QCC_LogError(ER_BUS_TRANSPORT_NOT_STARTED, ("WFDTransport::DisableAdvertisement(): Not running or stopping; exiting"));
        return;
    }

    QueueDisableAdvertisement(advertiseName);
}

void WFDTransport::QueueDisableAdvertisement(const qcc::String& advertiseName)
{
    QCC_DbgPrintf(("WFDTransport::QueueDisableAdvertisement()"));

    ListenRequest listenRequest;
    listenRequest.m_requestOp = DISABLE_ADVERTISEMENT_INSTANCE;
    listenRequest.m_requestParam = advertiseName;

    m_listenRequestsLock.Lock(MUTEX_CONTEXT);
    m_listenRequests.push(listenRequest);
    m_listenRequestsLock.Unlock(MUTEX_CONTEXT);

    /*
     * Wake the server accept loop thread up so it will process the request we
     * just queued.
     */
    Alert();
}

/*
 * This is the destination of the found name callback for the Wi-Fi P2P helper.
 * If we have successfully enabled pre-association service discovery and done
 * a FindAdvertisedName there, as soon as the system finds a matching service
 * it will call us back here.
 *
 * The main difference between a normal multicast name service callback and a
 * Wi-Fi P2P callback is that in the P2P case, there is no wireless connection
 * and therefore no IP address or port to connect to.  Instead of using the IP
 * address and port to key to the desired service, we use the GUID of the daemon
 * that hosts that service.
 *
 * This means that we need to change the definition of a bus address to admit
 * the possibility that there is no IP address or port.  In the "normal" case
 * a bus address will look like, "r4addr=192.168.1.100,port=9955" but in the P2P
 * case, a bus address will look like, "guid=167a3d1d18c9404a846a0110e287d751"
 * This bus address fill filter up through the AllJoyn system into any client
 * app searching for the name and if it decides to join a corresponding session
 * will filter back down here and reappear as the connect spec in a call to
 * WFDTransport::Connect().
 *
 * In order to do the actual connect, we need to keep a mapping from the GUID
 * which AllJoyn will use to the device which is meaningful to the P2P code.
 * We keep these mappings in the p2pDeviceList.
 */
void WFDTransport::OnFoundAdvertisedName(const char* name, const char* namePrefix, const char* guid, const char* device)
{
    QCC_DbgPrintf(("WFDTransport::OnFoundAdvertisedName(\"%s\", \"%s\", \"%s\", \"%s\")", name, namePrefix, guid, device));

    /*
     * If there is no listener, there is nobody to listen to our ravings so it
     * wouild be pointless do do anything.  This is really an error somewhere
     * else, but we'll just accept it instead of asserting here.
     */
    if (m_listener) {
        qcc::String foundName(name);
        qcc::String foundGuid(guid);
        qcc::String foundDevice(device);
        qcc::String busAddr = "wfd:guid=" + foundGuid;

        /*
         * Save the mapping between the guid and the device.  Since it is
         * possible to have more than one daemon running on a P2P device, and
         * more than one P2P device associated with a single daemon, we just
         * keep a list of these mappings.
         */
        m_deviceListLock.Lock(MUTEX_CONTEXT);
        m_deviceList.push_back(std::pair<qcc::String, qcc::String>(guid, device));
        m_deviceListLock.Unlock(MUTEX_CONTEXT);

        /*
         * The transport list listeners expect to get a vector of names, so
         * we need to make a temporary one before we call back.
         */
        std::vector<qcc::String> nameList;
        nameList.push_back(foundName);

        /*
         * This is the call to tell the AllJoyn world that we've found something
         * possibly interesting.  We report back that the transport will be a
         * wireless LAN even though the connection is not made yet -- it will
         * be.  the value 255 means that the advertisement never expires.  It
         * will remain until we are explicity told it has gone away by the P2P
         * framework.
         */
        m_listener->FoundNames(busAddr, foundGuid, TRANSPORT_WLAN, &nameList, 255);
    }
}

/*
 * This is the destination of the lost name callback for the Wi-Fi P2P helper.
 * The difference between found and lost to the AllJoyn world above is only the
 * value of the timer we give it.  So we have to translate this callback into a
 * FoundNames with the provided timer value set to zero, which indicates a lost
 * name.
 */
void WFDTransport::OnLostAdvertisedName(const char* name, const char* namePrefix, const char* guid, const char* device)
{
    QCC_DbgPrintf(("WFDTransport::OnLostAdvertisedName(\"%s\", \"%s\", \"%s\", \"%s\")", name, namePrefix, guid, device));
    /*
     * If there is no listener, there is nobody to listen to our ravings so it
     * wouild be pointless do do anything.  This is really an error somewhere
     * else, but we'll just accept it instead of asserting here.
     */
    if (m_listener) {
        qcc::String foundName(name);
        qcc::String foundGuid(guid);
        qcc::String foundDevice(device);
        qcc::String busAddr = "guid=" + foundGuid;

        /*
         * Remove a mapping between the guid and the device.
         */
        m_deviceListLock.Lock(MUTEX_CONTEXT);
        for (DeviceList::iterator i = m_deviceList.begin(); i != m_deviceList.end(); ++i) {
            if (i->first == guid && i->second == device) {
                m_deviceList.erase(i);
                break;
            }
        }
        m_deviceListLock.Unlock(MUTEX_CONTEXT);

        /*
         * The transport list listeners expect to get a vector of names, so
         * we need to make a temporary one before we call back.
         */
        std::vector<qcc::String> nameList;
        nameList.push_back(foundName);

        /*
         * This is the call to tell the AllJoyn world that we've lost something
         * that was previously interesting.  The value 0 means that any prior
         * advertisement of this name and guid is no longer valid.
         */
        m_listener->FoundNames(busAddr, foundGuid, TRANSPORT_WLAN, &nameList, 0);
    }
}

void WFDTransport::FoundCallback::Found(const qcc::String& busAddr, const qcc::String& guid,
                                        std::vector<qcc::String>& nameList, uint8_t timer)
{
    QCC_DbgPrintf(("WFDTransport::FoundCallback::Found(): busAddr = \"%s\"", busAddr.c_str()));

    /*
     * Whenever the name service receives a message indicating that a bus-name
     * is out on the network somewhere, it sends a message back to us via this
     * callback.  In order to avoid duplication of effort, the name service does
     * not manage a cache of names, but delegates that to the daemon having this
     * transport.  If the timer parameter is non-zero, it indicates that the
     * nameList (actually a vector of bus-name Strings) can be expected to be
     * valid for the value of timer in seconds.  If timer is zero, it means that
     * the bus names in the nameList are no longer available and should be
     * flushed out of the daemon name cache.
     *
     * The name service does not have a cache and therefore cannot time out
     * entries, but also delegates that task to the daemon.  It is expected that
     * remote daemons will send keepalive messages that the local daemon will
     * recieve, also via this callback.  Since we are just a go-between, we
     * pretty much just pass what we find on back to the daemon, modulo some
     * filtering to avoid situations we don't yet support:
     *
     * 1. Currently this transport has no clue how to handle anything but
     *    reliable IPv4 endpoints (r4addr, r4port), so we filter everything else
     *    out (by removing the unsupported endpoints from the bus address)
     */
    qcc::String r4addr("r4addr=");
    qcc::String r4port("r4port=");
    qcc::String comma(",");

    /*
     * Find where the r4addr name starts.
     */
    size_t i = busAddr.find(r4addr);
    if (i == String::npos) {
        QCC_DbgPrintf(("WFDTransport::FoundCallback::Found(): No r4addr in busaddr."));
        return;
    }
    i += r4addr.size();

    /*
     * We assume that the address is always followed by the port so there must
     * be a comma following the address.
     */
    size_t j = busAddr.find(comma, i);
    if (j == String::npos) {
        QCC_DbgPrintf(("WFDTransport::FoundCallback::Found(): No comma after r4addr in busaddr."));
        return;
    }

    size_t k = busAddr.find(r4port);
    if (k == String::npos) {
        QCC_DbgPrintf(("WFDTransport::FoundCallback::Found(): No r4port in busaddr."));
        return;
    }
    k += r4port.size();

    size_t l = busAddr.find(comma, k);
    if (l == String::npos) {
        l = busAddr.size();
    }

    /*
     * We have the following situation now.  Either:
     *
     *     "r4addr=192.168.1.1,r4port=9955,u4addr=192.168.1.1,u4port=9955"
     *             ^          ^       ^   ^
     *             i          j       k   l = 30
     *
     * or
     *
     *     "r4addr=192.168.1.1,r4port=9955"
     *             ^          ^       ^   ^
     *             i          j       k   l = 30
     *
     * So construct a new bus address with only the reliable IPv4 part pulled
     * out.
     */
    qcc::String newBusAddr = String("wfd:") + r4addr + busAddr.substr(i, j - i) + "," + r4port + busAddr.substr(k, l - k);

    QCC_DbgPrintf(("WFDTransport::FoundCallback::Found(): newBusAddr = \"%s\".", newBusAddr.c_str()));

    /*
     * Let the Wi-Fi P2P code know that we've found an address and port
     * corresponding to some GUID.
     */
    if (m_transport) {
        QCC_DbgPrintf(("WFDTransport::FoundCallback::Found(): OnFound() %s", newBusAddr.c_str()));
        m_transport->OnFound(newBusAddr, guid);
    }

    /*
     * Let AllJoyn know that we've found a service(s).
     */
    if (m_listener) {
        QCC_DbgPrintf(("WFDTransport::FoundCallback::Found(): FoundNames(): %s", newBusAddr.c_str()));
        m_listener->FoundNames(newBusAddr, guid, TRANSPORT_WFD, &nameList, timer);
    }
}

} // namespace ajn
