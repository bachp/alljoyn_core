/**
 * @file
 * The lightweight name service implementation
 */

/******************************************************************************
 * Copyright 2010-2011, Qualcomm Innovation Center, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <algorithm>

#if defined(QCC_OS_GROUP_WINDOWS)
#define close closesocket
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#endif

#include <qcc/Debug.h>
#include <qcc/Event.h>
#include <qcc/Socket.h>
#include <qcc/SocketTypes.h>
#include <qcc/IfConfig.h>
#include <qcc/time.h>

#include <DaemonConfig.h>

#include "IpNameServiceImpl.h"

#define QCC_MODULE "IPNS"

using namespace std;

namespace ajn {

// ============================================================================
// Long sidebar on why this looks so complicated:
//
// In order to understand all of the trouble we are going to go through below,
// it is helpful to thoroughly understand what is done on our platforms in the
// presence of multicast.  This is long reading, but worthwhile reading if you
// are trying to understand what is going on.  I don't know of anywhere you
// can find all of this written in one place.
//
// The first thing to grok is that all platforms are implemented differently.
// Windows and Linux use IGMP to enable and disable multicast, and use other
// multicast-related socket calls to do the fine-grained control.  Android
// doesn't bother to compile its kernel with CONFIG_IP_MULTICAST set.  This
// doesn't mean that there is no multicast code in the Android kernel, it means
// there is no IGMP code in the kernel.  Since IGMP isn't implemented, Android
// can't use it to enable and disable multicast at the driver level, so it uses
// wpa_supplicant driver-private commands instead.  This means that you will
// probably get three different answers if you ask how some piece of the
// multicast puzzle works.
//
// On the send side, multicast is controlled by the IP_MULTICAST_IF (or for
// IPv6 IPV6_MULTICAST_IF) socket.  In IPv4 you provide an IP address and in
// IPv6 you provide an interface index.  These differences are abstracted in
// the qcc code and there you are asked to provide an interface name, which the
// abstraction function uses to figure out the appropriate address or index
// depending on the address family.  Unfortunately, you can't abstract away
// the operating system differences in how they interpret the calls; so you
// really need to understand what is happening at a low level in order to get
// the high level multicast operations to do what you really want.
//
// If you do nothing (leave the sockets as you find them), or set the interface
// address to 0.0.0.0 for IPv4 or the interface index to 0 for IPv6 the
// multicast output interface is essentially selected by the system routing
// code.
//
// In Linux (and Android), multicast packets are sent out the interface that is
// used for the default route (the default interface).  You can see this if you
// type "ip ro sh".  In Windows, however, the system chooses its default
// interface by looking for the lowest value for the routing metric for a
// destination IP address of 224.0.0.0 in its routing table.  You can see this
// in the output of "route print".
//
// We want all of our multicast code to work in the presence of IP addresses
// changing when phones move from one Wifi access point to another, or when our
// desktop access point changes when someone with a mobile access point walks
// by; so it is also important to know what will happen when these addresses
// change (or come up or go down).
//
// On Linux, if you set the IP_MULTICAST_IF to 0.0.0.0 (or index 0 in IPv6) and
// bring down the default interface or change the IP address on the default
// interface, you will begin to fail the multicast sends with "network
// unreachable" errors since the default route goes away when you change the IP
// address (e.g, just do somthing like "sudo ifconfig eth1 10.4.108.237 netmask
// 255.255.255.0 up to change the address).  Until you provide a new default
// route (e.g., "route add default gw 10.4.108.1") the multicast packets will be
// dropped, but as soon as a new default route is set, they will begin flowing
// again.
//
// In Windows, if you set the IP_MULTICAST_IF address to 0.0.0.0 and release the
// ip address (e.g., "ipconfig /release") the sends may still appear to work at
// the level of the program but nothing goes out the original interface.  The
// sends fail silently.  This is because Windows will dynamically change the
// default multicast route according to its internal multicast routing table.
// It selects another interface based on a routing metric, and it could, for
// example, just switch to a VMware virtual interface silently.  The name
// service would never know it just "broke" and is no longer sending packets out
// the interface it thinks it is.
//
// When we set up multicast advertisements in our system, we most likely do not
// want to route our advertisements only to the default adapter.  For example,
// on a desktop system, the default interface is probably one of the wired
// Ethernets.  We may or many not want to advertise on that interface, but we
// may also want to advertise on other wired interfaces and other wireless
// interfaces as well.
//
// We do not want the system to start changing multicast destinations out from
// under us, EVER.  Because of this, the only time using INADDR_ANY would be
// appropriate in the IP_MULTICAST_IF socket option is in the simplest, static
// network situations.  For the general case, we really need to keep multiple
// sockets that are each talking to an INTERFACE of interest (not an IP address
// of interest, since they can change at any time because of normal access point
// dis-associations, for example).
//
// Since we determined that we needed to use IP_MULTICAST_IF to control which
// interfaces are used for discovery, we needed to understand exactly what
// changing an IP address out from under a corresponding interface would do.
//
// The first thing we observed is that IP_MULTICAST_IF takes an IP address in
// the case of IPv4, but we wanted to specify an interface index as in IPv6 or
// for mere mortal human beings, a name (e.g., "wlan0").  It may be the case
// that the interface does not have an IP address assigned (is not up or
// connected to an access point) at the time we want to start our name service,
// so a call to set the IP_MULTICAST_IF (via the appropriate abstract qcc call)
// would not be possible until an address is available, perhaps an arbitrary
// unknowable time later.  If sendto() operations are attempted and the IP
// address is not valid one will see "network unreachable" errors.  As we will
// discuss shortly, joining a multicast group also requires an IP address in the
// case of IPv4 (need to send IGMP Join messages), so it is not possible to
// express interest in receiving multicast packets until an IP address is
// available.
//
// So we needed to provide an API that allows a user to specify a network
// interface over which she is interested in advertising.  This explains the
// method OpenInterface(qcc::String interface) defined below.  The client is
// expected to figure out which interfaces it wants to do discovery over (e.g.,
// "wlan0", "eth0") and explicitly tell the name service which interfaces it is
// interested in.  We clearly need a lazy evaluation mechanism in the name
// service to look at the interfaces which the client expresses interest in, and
// when IP addresses are available, or change, we begin using those interfaces.
// If the interfaces go down, or change out from under the name service, we need
// to deal with that fact and make things right.
//
// We can either hook system "IP address changed" or "interface state changed"
// events to drive the re-evaluation process as described above, or we can poll
// for those changes.  Since the event systems in our various target platforms
// are wildly different, creating an abstract event system is non-trivial (for
// example, a DBus-based network manager exists on Linux, but even though
// Android is basically Linux and has DBus, it doesn't use it.  You'd need to
// use Netlink sockets on most Posix systems, but Darwin doesn't have Netlink.
// Windows is from another planet.
//
// Because of all of these complications, we just choose the better part of
// valor and poll for changes using a maintenance thread that fires off every
// second and looks for changes in the networking environment and adjusts
// accordingly.
//
// We could check for IP address changes on the interfaces and re-evaluate and
// open new sockets bound to the correct interfaces whenever an address change
// happens.  It is possible, however, that we could miss the fact that we have
// switched access points if DHCP gives us the same IP address.  Windows, for
// example, could happily begin rerouting packets to other interfaces if one
// goes down.  If the interface comes back up on a different access point, which
// gives out the same IP address, Windows could bring us back up but leave the
// multicast route pointing somewhere else and we would never notice.  Because
// of these byzantine kinds of errors, we chose the better part of valor and
// decided to close all of our multicast sockets down and restart them in a
// known state periodically.
//
// The receive side has similar kinds of issues.
//
// In order to receive multicast datagrams sent to a particular port, it is
// necessary to bind that local port leaving the local address unspecified
// (i.e., INADDR_ANY or in6addr_any).  What you might think of as binding is
// then actually handled by the Internet Group Management Protocol (IGMP) or its
// ICMPv6 equivalent.  Recall that Android does not implement IGMP, so we have
// yet another complication.
//
// Using IGMP, we join the socket to the multicast group instead of binding the
// socket to a specific interface (address) and port.  Binding the socket to
// INADDR_ANY or in6addr_any may look strange, but it is actually the right
// thing to do.  Since joining a multicast group requires sending packets over
// the IGMP protocol, we need a valid IP address in order to do the join.  As
// mentioned above, an interface must be IFF_UP with an assigned IP address in
// order to join a multicast group.
//
// The socket option for joining a multicast group, of course, works differently
// for IPv4 and IPv6.  IP_ADD_MEMBERSHIP (for IPv4) has a provided IP address
// that can be either INADDR_ANY or a specific address.  If INADDR_ANY is
// provided, the interface of the default route is added to the group, and the
// IGMP join is sent out that interface.  IPV6_ADD_MEMBERSHIP (for IPv6) has a
// provided interface index that can be either 0 or a specific interface.  If 0
// is provided, the interface of the default route is added to the group, and
// the IGMP Join (actually an ICMPv6 equivalent) is sent out that interface.  If
// a specific interface index is that interface is added to the group and the
// IGMP join is sent out that interface.  Note that since an ICMP packet is sent,
// the interface must be IFF_UP with an assigned IP address even though the
// interface is specified by an index.
//
// A side effect of the IGMP join deep down in the kernel is to enable reception
// of multicast MAC addresses in the device driver.  Since there is no IGMP in
// Android, we must rely on a multicast (Java) lock being taken by some external
// code on phones that do not leave multicast always enabled (HTC Desire, for
// example).  When the Java multicast lock is taken, a private driver command is
// sent to the wpa_supplicant which, in turn, calls into the appropriate network
// device driver(s) to enable reception of multicast MAC packets.  This is
// completely out of our control here.
//
// Similar to the situation on the send side, we most likely do not want to rely
// on the system routing tables to configure which network interfaces our name
// service receives over; so we really need to provide a specific address.
//
// If a specific IP address is provided, then that address must be an address
// assigned to a currently-UP interface.  This is the same catch-22 as we have
// on the send side.  We need to lazily evaluate the interface in order to find
// if an IP address has appeared on that interface and then join the multicast
// group to enable multicast on the underlying network device.
//
// It turns out that in Linux, the IP address passed to the join multicast group
// socket option call is actually not significant after the initial call.  It is
// used to look up an interface and its associated net device and to then set
// the PACKET_MULTICAST filter on the net device to receive packets destined for
// the specified multicast address.  If the IP address associated with the
// interface changes, multicast messages will continue to be received.
//
// Of course, Windows does it differently.  They look at the IP address passed
// to the socket option as being significant, and so if the underlying IP
// address changes on a Windows system, multicast packets will no longer be
// delivered.  Because of this, the receive side of the multicast name service
// has also got to look for changes to IP address configuration and re-set
// itself whenever it finds a change.
//
// So the code you find below may look overly complicated, but (hopefully most
// of it, anyway) needs to be that way.
//
// As an aside, the daemon that owns us can be happy as a clam by simply binding
// to INADDR_ANY since the semantics of this action, as interpreted by both
// Windows and Linux, are to listen for connections on all current and future
// interfaces and their IP addresses.  The complexity is fairly well contained
// here.
// ============================================================================

//
// There are configurable attributes of the name service which are determined
// by the configuration database.  A module name is required and is defined
// here.  An example of how to use this is in setting the interfaces the name
// service will use for discovery.
//
//   <busconfig>
//     <ip_name_service>
//       <property interfaces="*"/>
//       <property disable_directed_broadcast="false"/>
//       <property enable_ipv4="true"/>
//       <property enable_ipv6="true"/>
//     </ip_name_service>
//   </busconfig>
//
//
// The value of the interfaces property used to configure the name service
// to run discovery over all interfaces in the system.
//
const char* IpNameServiceImpl::INTERFACES_WILDCARD = "*";

#if 1
//
// This is the IANA assigned IPv4 multicast group for AllJoyn.  This is
// a Local Network Control Block address.
//
// See www.iana.org/assignments/multicast-addresses
//
const char* IpNameServiceImpl::IPV4_ALLJOYN_MULTICAST_GROUP = "224.0.0.113";
#endif

//
// This is the IANA assigned UDP port for the AllJoyn Name Service.  See
// www.iana.org/assignments/service-names-port-numbers
//
const uint16_t IpNameServiceImpl::MULTICAST_PORT = 9956;
const uint16_t IpNameServiceImpl::BROADCAST_PORT = IpNameServiceImpl::MULTICAST_PORT;

//
// This is the IANA assigned IPv6 multicast group for AllJoyn.  The assigned
// address is a variable scope address (ff0x) but we always use the link local
// scope (ff02).  See www.iana.org/assignments/multicast-addresses
//
const char* IpNameServiceImpl::IPV6_ALLJOYN_MULTICAST_GROUP = "ff02::13a";

//
// Simple pattern matching function that supports '*' and '?' only.  Returns a
// bool in the sense of "a difference between the string and pattern exists."
// This is so it works like fnmatch or strcmp, which return a 0 if a match is
// found.
//
// We require an actual character match and do not consider an empty string
// something that can match or be matched.
//
bool IpNameServiceImplWildcardMatch(qcc::String str, qcc::String pat)
{
    size_t patsize = pat.size();
    size_t strsize = str.size();
    const char* p = pat.c_str();
    const char* s = str.c_str();
    uint32_t pi, si;

    //
    // Zero length strings are unmatchable.
    //
    if (patsize == 0 || strsize == 0) {
        return true;
    }

    for (pi = 0, si = 0; pi < patsize && si < strsize; ++pi, ++si) {
        switch (p[pi]) {
        case '*':
            //
            // Point to the character after the wildcard.
            //
            ++pi;

            //
            // If the wildcard is at the end of the pattern, we match
            //
            if (pi == patsize) {
                return false;
            }

            //
            // If the next character is another wildcard, we could go through
            // a bunch of special case work to figure it all out, but in the
            // spirit of simplicity we don't deal with it and return "different".
            //
            if (p[pi] == '*' || p[pi] == '?') {
                return true;
            }

            //
            // Scan forward in the string looking for the character after the
            // wildcard.
            //
            for (; si < strsize; ++si) {
                if (s[si] == p[pi]) {
                    break;
                }
            }
            break;

        case '?':
            //
            // A question mark matches any character in the string.
            //
            break;

        default:
            //
            // If no wildcard, we just compare character for character.
            //
            if (p[pi] != s[si]) {
                return true;
            }
            break;
        }
    }

    //
    // If we fall through to here, we have matched all the way through one or
    // both of the strings.  If pi == patsize and si == strsize then we matched
    // all the way to the end of both strings and we have a match.
    //
    if (pi == patsize && si == strsize) {
        return false;
    }

    //
    // If pi < patsize and si == strsize there are characters in the pattern
    // that haven't been matched.  The only way this can be a match is if
    // that last character is a '*' meaning zero or more characters match.
    //
    if (pi < patsize && si == strsize) {
        if (p[pi] == '*') {
            return false;
        } else {
            return true;
        }
    }

    //
    // The remaining chase is pi == patsize and si < strsize which means
    // that we've got characters in the string that haven't been matched
    // by the pattern.  There's no way this can be a match.
    //
    return true;
}

IpNameServiceImpl::IpNameServiceImpl()
    : Thread("IpNameServiceImpl"), m_state(IMPL_SHUTDOWN), m_terminal(false),
    m_callback(0), m_port(0),
    m_reliableIPv4Port(0), m_unreliableIPv4Port(0), m_reliableIPv6Port(0), m_unreliableIPv6Port(0),
    m_timer(0), m_tDuration(DEFAULT_DURATION),
    m_tRetransmit(RETRANSMIT_TIME), m_tQuestion(QUESTION_TIME),
    m_modulus(QUESTION_MODULUS), m_retries(NUMBER_RETRIES),
    m_loopback(false), m_enableIPv4(false), m_enableIPv6(false),
    m_any(false), m_wakeEvent(), m_forceLazyUpdate(false),
    m_enabled(false), m_doEnable(false), m_doDisable(false)
{
    QCC_DbgPrintf(("IpNameServiceImpl::IpNameServiceImpl()"));
}

QStatus IpNameServiceImpl::Init(const qcc::String& guid, bool loopback)
{
    QCC_DbgPrintf(("IpNameServiceImpl::Init()"));

    //
    // Can only call Init() if the object is not running or in the process
    // of initializing
    //
    if (m_state != IMPL_SHUTDOWN) {
        return ER_FAIL;
    }

    m_state = IMPL_INITIALIZING;

    DaemonConfig* config = DaemonConfig::Access();

    //
    // We enable outbound traffic on a per-interface basis.  Whether or not we
    // will consider using a network interface address to send name service
    // packets depends on the configuration.
    //
    m_enableIPv4 = config->Get("ip_name_service/property@enable_ipv4", "true") == "true";
    m_enableIPv6 = config->Get("ip_name_service/property@enable_ipv6", "true") == "true";
    m_broadcast = config->Get("ip_name_service/property@disable_directed_broadcast", "false") == "false";

    m_guid = guid;
    m_loopback = loopback;
    m_terminal = false;

    return ER_OK;
}

//
// When we moved the name service out of the TCP transport and promoted it to a
// singleton, we opened a bit of a can of worms because of the C++ static
// destruction order fiasco and our interaction with the bundled daemon.
//
// Since the bundled daemon may be destroyed after the IP name service singleton
// it is possible that multiple threads (transports) may be still accessing the
// name service as it is being destroyed.  This horrific situation will be
// resolved when we accomplish strict destructor ordering, but for now, we have
// the possibility.
//
// This object was never intended to provide multithread safe destruction and so
// we are exposed in the case where the object destroys itself around a thread
// that is executing in one of its methods.  The chances are small that this
// happens, but the chance is non-zero; and the result might be a crash after
// the process main() function exits!
//
IpNameServiceImpl::~IpNameServiceImpl()
{
    QCC_DbgPrintf(("IpNameServiceImpl::~IpNameServiceImpl()"));

    //
    // Stop the worker thread to get things calmed down.
    //
    if (IsRunning()) {
        Stop();
        Join();
    }

    //
    // We may have some open sockets.  Windows boxes may have Winsock shut down
    // by the time we get to this destructor so we are out of luck trying to
    // make the necessary calls.
    //
#if !defined(QCC_OS_GROUP_WINDOWS)
    ClearLiveInterfaces();
#endif

    //
    // We can just blow away the requested interfaces without a care.
    //
    m_requestedInterfaces.clear();

    //
    // Delete any callbacks that a user of this class may have set.
    //
    delete m_callback;
    m_callback = 0;

    //
    // All shut down and ready for bed.
    //
    m_state = IMPL_SHUTDOWN;
}

QStatus IpNameServiceImpl::OpenInterface(const qcc::String& name)
{
    QCC_DbgPrintf(("IpNameServiceImpl::OpenInterface(%s)", name.c_str()));

    //
    // Can only call OpenInterface() if the object is running.
    //
    if (m_state != IMPL_RUNNING) {
        QCC_DbgPrintf(("IpNameServiceImpl::OpenInterface(): Not running"));
        return ER_FAIL;
    }

    //
    // If the user specifies the wildcard interface name, this trumps everything
    // else.
    //
    if (name == INTERFACES_WILDCARD) {
        qcc::IPAddress wildcard("0.0.0.0");
        return OpenInterface(wildcard);
    }
    //

    // There are at least two threads that can wander through the vector below
    // so we need to protect access to the list with a convenient mutex.
    //
    m_mutex.Lock();

    for (uint32_t i = 0; i < m_requestedInterfaces.size(); ++i) {
        if (m_requestedInterfaces[i].m_interfaceName == name) {
            QCC_DbgPrintf(("IpNameServiceImpl::OpenInterface(): Already opened."));
            m_mutex.Unlock();
            return ER_OK;
        }
    }

    InterfaceSpecifier specifier;
    specifier.m_interfaceName = name;
    specifier.m_interfaceAddr = qcc::IPAddress("0.0.0.0");

    m_requestedInterfaces.push_back(specifier);
    m_forceLazyUpdate = true;
    m_wakeEvent.SetEvent();
    m_mutex.Unlock();
    return ER_OK;
}

QStatus IpNameServiceImpl::OpenInterface(const qcc::IPAddress& addr)
{
    QCC_DbgPrintf(("IpNameServiceImpl::OpenInterface(%s)", addr.ToString().c_str()));

    //
    // Can only call OpenInterface() if the object is running.
    //
    if (m_state != IMPL_RUNNING) {
        QCC_DbgPrintf(("IpNameServiceImpl::OpenInterface(): Not running"));
        return ER_FAIL;
    }

    //
    // There are at least two threads that can wander through the vector below
    // so we need to protect access to the list with a convenient mutex.
    //
    m_mutex.Lock();

    //
    // We treat the INADDR_ANY address (and the equivalent IPv6 address as a
    // wildcard.  To have the same semantics as using INADDR_ANY in the TCP
    // transport listen spec, and avoid resulting user confusion, we need to
    // interpret this as "use any interfaces that are currently up, or may come
    // up in the future to send and receive name service messages over."  This
    // trumps anything else the user might throw at us.  We set a global flag to
    // indicate this mode of operation and clear it if we see a CloseInterface()
    // on INADDR_ANY.  These calls are not reference counted.
    //
    if (addr == qcc::IPAddress("0.0.0.0") ||
        addr == qcc::IPAddress("0::0") ||
        addr == qcc::IPAddress("::")) {
        QCC_DbgPrintf(("IpNameServiceImpl::OpenInterface(): Wildcard address"));
        m_any = true;
        m_mutex.Unlock();
        return ER_OK;
    }

    for (uint32_t i = 0; i < m_requestedInterfaces.size(); ++i) {
        if (m_requestedInterfaces[i].m_interfaceAddr == addr) {
            QCC_DbgPrintf(("IpNameServiceImpl::OpenInterface(): Already opened."));
            m_mutex.Unlock();
            return ER_OK;
        }
    }

    InterfaceSpecifier specifier;
    specifier.m_interfaceName = "";
    specifier.m_interfaceAddr = addr;

    m_requestedInterfaces.push_back(specifier);
    m_forceLazyUpdate = true;
    m_wakeEvent.SetEvent();
    m_mutex.Unlock();
    return ER_OK;
}

QStatus IpNameServiceImpl::CloseInterface(const qcc::String& name)
{
    QCC_DbgPrintf(("IpNameServiceImpl::CloseInterface(%s)", name.c_str()));

    //
    // Can only call CloseInterface() if the object is running.
    //
    if (m_state != IMPL_RUNNING) {
        QCC_DbgPrintf(("IpNameServiceImpl::CloseInterface(): Not running"));
        return ER_FAIL;
    }

    //
    // There are at least two threads that can wander through the vector below
    // so we need to protect access to the list with a convenient mutex.
    //
    m_mutex.Lock();

    //
    // use Meyers' idiom to keep iterators sane.  Note that we don't close the
    // socket in this call, we just remove the request and the lazy updator will
    // just not use it when it re-evaluates what to do (called immediately below).
    //
    for (vector<InterfaceSpecifier>::iterator i = m_requestedInterfaces.begin(); i != m_requestedInterfaces.end();) {
        if ((*i).m_interfaceName == name) {
            m_requestedInterfaces.erase(i++);
        } else {
            ++i;
        }
    }

    m_forceLazyUpdate = true;
    m_wakeEvent.SetEvent();
    m_mutex.Unlock();
    return ER_OK;
}

QStatus IpNameServiceImpl::CloseInterface(const qcc::IPAddress& addr)
{
    QCC_DbgPrintf(("IpNameServiceImpl::CloseInterface(%s)", addr.ToString().c_str()));

    //
    // Can only call CloseInterface() if the object is running.
    //
    if (m_state != IMPL_RUNNING) {
        QCC_DbgPrintf(("IpNameServiceImpl::CloseInterface(): Not running"));
        return ER_FAIL;
    }

    //
    // There are at least two threads that can wander through the vector below
    // so we need to protect access to the list with a convenient mutex.
    //
    m_mutex.Lock();

    //
    // We treat the INADDR_ANY address (and the equivalent IPv6 address as a
    // wildcard.  We set a global flag in OpenInterface() to indicate this mode
    // of operation and clear it here.  These calls are not reference counted
    // so one call to CloseInterface(INADDR_ANY) will stop this mode
    // irrespective of how many opens are done.
    //
    if (addr == qcc::IPAddress("0.0.0.0") ||
        addr == qcc::IPAddress("0::0") ||
        addr == qcc::IPAddress("::")) {
        QCC_DbgPrintf(("IpNameServiceImpl::CloseInterface(): Wildcard address"));
        m_any = false;
        m_mutex.Unlock();
        return ER_OK;
    }

    //
    // use Meyers' idiom to keep iterators sane.  Note that we don't close the
    // socket in this call, we just remove the request and the lazy updator will
    // just not use it when it re-evaluates what to do (called immediately below).
    //
    for (vector<InterfaceSpecifier>::iterator i = m_requestedInterfaces.begin(); i != m_requestedInterfaces.end();) {
        if ((*i).m_interfaceAddr == addr) {
            m_requestedInterfaces.erase(i++);
        } else {
            ++i;
        }
    }

    m_forceLazyUpdate = true;
    m_wakeEvent.SetEvent();
    m_mutex.Unlock();
    return ER_OK;
}

void IpNameServiceImpl::ClearLiveInterfaces(void)
{
    QCC_DbgPrintf(("IpNameServiceImpl::ClearLiveInterfaces()"));

    for (uint32_t i = 0; i < m_liveInterfaces.size(); ++i) {
        if (m_liveInterfaces[i].m_sockFd == -1) {
            continue;
        }

        QCC_DbgPrintf(("IpNameServiceImpl::ClearLiveInterfaces(): clear interface %d", i));

        //
        // If the multicast bit is set, we have done an IGMP join.  In this
        // case, we must arrange an IGMP drop via the appropriate socket option
        // (via the qcc absraction layer). Android doesn't bother to compile its
        // kernel with CONFIG_IP_MULTICAST set.  This doesn't mean that there is
        // no multicast code in the Android kernel, it means there is no IGMP
        // code in the kernel.  What this means to us is that even through we
        // are doing an IP_DROP_MEMBERSHIP request, which is ultimately an IGMP
        // operation, the request will filter through the IP code before being
        // ignored and will do useful things in the kernel even though
        // CONFIG_IP_MULTICAST was not set for the Android build -- i.e., we
        // have to do it anyway.
        //
        if (m_liveInterfaces[i].m_flags & qcc::IfConfigEntry::MULTICAST) {
            if (m_liveInterfaces[i].m_address.IsIPv4()) {
#if 1
                qcc::LeaveMulticastGroup(m_liveInterfaces[i].m_sockFd, qcc::QCC_AF_INET, IPV4_ALLJOYN_MULTICAST_GROUP, m_liveInterfaces[i].m_interfaceName);
#endif
            } else if (m_liveInterfaces[i].m_address.IsIPv6()) {
                qcc::LeaveMulticastGroup(m_liveInterfaces[i].m_sockFd, qcc::QCC_AF_INET6, IPV6_ALLJOYN_MULTICAST_GROUP, m_liveInterfaces[i].m_interfaceName);
            }
        }

        //
        // Always delete the event before closing the socket because the event
        // is monitoring the socket state and therefore has a reference to the
        // socket.  One the socket is closed the FD can be reused and our event
        // can end up monitoring the wrong socket and interfere with the correct
        // operation of other unrelated event/socket pairs.
        //
        delete m_liveInterfaces[i].m_event;
        m_liveInterfaces[i].m_event = NULL;

        qcc::Close(m_liveInterfaces[i].m_sockFd);
        m_liveInterfaces[i].m_sockFd = -1;
    }

    QCC_DbgPrintf(("IpNameServiceImpl::ClearLiveInterfaces(): Clear interfaces"));
    m_liveInterfaces.clear();

    QCC_DbgPrintf(("IpNameServiceImpl::ClearLiveInterfaces(): Done"));
}

//
// N.B. This function must be called with m_mutex locked since we wander through
// the list of requested interfaces that can also be modified by the user in the
// context of her thread(s).
//
void IpNameServiceImpl::LazyUpdateInterfaces(void)
{
    QCC_DbgPrintf(("IpNameServiceImpl::LazyUpdateInterfaces()"));

    //
    // However desirable it may be, the decision to simply use an existing
    // open socket exposes us to system-dependent behavior.  For example,
    // In Linux and Windows, an IGMP join must be done on an interface that
    // is currently IFF_UP and IFF_MULTICAST with an assigned IP address.
    // On Linux, that join remains in effect (net devices will continue to
    // recieve multicast packets destined for our group) even if the net
    // device goes down and comes back up with a different IP address.  On
    // Windows, however, if the interface goes down, an IGMP drop is done
    // and multicast receives will stop.  Since the socket never returns
    // any status unless we actually send data, it is very possible that
    // the state of the system can change out from underneath us without
    // our knowledge, and we would simply stop receiving multicasts. This
    // behavior is not specified anywhere that I am aware of, so Windows
    // cannot really be said to be broken.  It is just different, like it
    // is in so many other ways.  In Android, IGMP isn't even compiled into
    // the kernel, and so an out-of-band mechanism is used (wpa_supplicant
    // private driver commands called by the Java multicast lock).
    //
    // It can be argued that since we are using Android phones (sort-of Linux)
    // when mobility is a concern, and Windows boxes would be relatively static,
    // we could get away with ignoring the possibility of missing interface
    // state changes.  Since we are really talking an average of a couple of
    // IGMP packets every 30 seconds we take the conservative approach and tear
    // down all of our sockets and restart them every time through.
    //
    ClearLiveInterfaces();

    //
    // If m_enable is false, we need to make sure that no packets are sent
    // and no sockets are listening for connections.  This is for Android
    // Compatibility Test Suite (CTS) conformance.  The only way we can talk
    // to the outside world is via one of the live interfaces, so if we don't
    // make any new ones, this will accomplish the requirement.
    //
    if (m_enabled == false) {
        QCC_DbgPrintf(("IpNameServiceImpl::LazyUpdateInterfaces(): Communication with the outside world is forbidden"));
        return;
    }

    //
    // Call IfConfig to get the list of interfaces currently configured in the
    // system.  This also pulls out interface flags, addresses and MTU.  If we
    // can't get the system interfaces, we give up for now and hope the error
    // is transient.
    //
    QCC_DbgPrintf(("IpNameServiceImpl::LazyUpdateInterfaces(): IfConfig()"));
    std::vector<qcc::IfConfigEntry> entries;
    QStatus status = qcc::IfConfig(entries);
    if (status != ER_OK) {
        QCC_LogError(status, ("LazyUpdateInterfaces: IfConfig() failed"));
        return;
    }

    //
    // There are two fundamental ways we can look for interfaces to use.  We
    // can either walk the list of IfConfig entries (real interfaces on the
    // system) looking for any that match our list of user-requested
    // interfaces; or we can walk the list of user-requested interfaces looking
    // for any that match the list of real IfConfig entries.  Since we have an
    // m_any mode that means match all real IfConfig entries, we need to walk
    // the real IfConfig entries.
    //
    for (uint32_t i = 0; (m_state == IMPL_RUNNING || m_terminal) && (i < entries.size()); ++i) {
        //
        // We expect that every device in the system must have a name.
        // It might be some crazy random GUID in Windows, but it will have
        // a name.
        //
        assert(entries[i].m_name.size());
        QCC_DbgPrintf(("IpNameServiceImpl::LazyUpdateInterfaces(): Checking out interface %s", entries[i].m_name.c_str()));

        //
        // We are never interested in interfaces that are not UP or are LOOPBACK
        // interfaces.  We don't allow loopbacks since sending messages to the
        // local host is handled by the MULTICAST_LOOP socket option which is
        // enabled by default.
        //
        if ((entries[i].m_flags & qcc::IfConfigEntry::UP) == 0 ||
            (entries[i].m_flags & qcc::IfConfigEntry::LOOPBACK) != 0) {
            QCC_DbgPrintf(("IpNameServiceImpl::LazyUpdateInterfaces(): not UP or LOOPBACK"));
            continue;
        }

        //
        // When initializing the name service, the user can decide whether or
        // not she wants to advertise and listen over IPv4 or IPv6.  We need
        // to check for that configuration here.  Since the rest of the code
        // just works with the live interfaces irrespective of address family,
        // this is the only place we need to do this check.
        //
        if ((m_enableIPv4 == false && entries[i].m_family == qcc::QCC_AF_INET) ||
            (m_enableIPv6 == false && entries[i].m_family == qcc::QCC_AF_INET6)) {
            QCC_DbgPrintf(("IpNameServiceImpl::LazyUpdateInterfaces(): family %d not enabled", entries[i].m_family));
            continue;
        }

        //
        // The current real interface entry is a candidate for use.  We need to
        // decide if we are actually going to use it either based on the
        // wildcard mode or the list of requestedInterfaces provided by our
        // user.
        //
        bool useEntry = false;

        if (m_any) {
            QCC_DbgPrintf(("IpNameServiceImpl::LazyUpdateInterfaces(): Use because wildcard mode"));
            useEntry = true;
        } else {
            for (uint32_t j = 0; j < m_requestedInterfaces.size(); ++j) {
                //
                // If the current real interface name matches the name in the
                // requestedInterface list, we will try to use it.
                //
                if (m_requestedInterfaces[j].m_interfaceName.size() != 0 &&
                    m_requestedInterfaces[j].m_interfaceName == entries[i].m_name) {
                    QCC_DbgPrintf(("IpNameServiceImpl::LazyUpdateInterfaces(): Found matching requestedInterface name"));
                    useEntry = true;
                    break;
                }

                //
                // If the current real interface IP Address matches the name in
                // the requestedInterface list, we will try to use it.
                //
                if (m_requestedInterfaces[j].m_interfaceName.size() == 0 &&
                    m_requestedInterfaces[j].m_interfaceAddr == qcc::IPAddress(entries[i].m_addr)) {
                    QCC_DbgPrintf(("IpNameServiceImpl::LazyUpdateInterfaces(): Found matching requestedInterface address"));
                    useEntry = true;
                    break;
                }
            }
        }

        //
        // If we aren't configured to use this entry, or have no idea how to use
        // this entry (not AF_INET or AF_INET6), try the next one.
        //
        if (useEntry == false || (entries[i].m_family != qcc::QCC_AF_INET && entries[i].m_family != qcc::QCC_AF_INET6)) {
            QCC_DbgPrintf(("IpNameServiceImpl::LazyUpdateInterfaces(): Won't use this IfConfig entry"));
            continue;
        }

        //
        // If we fall through to here, we have decided that the host configured
        // entries[i] interface describes an interface we want to use to send
        // and receive our name service messages over.  We keep a list of "live"
        // interfaces that reflect the interfaces we've previously made the
        // decision to use, so we'll set up a socket and move it there.  We have
        // to be careful about what kind of socket we are going to use for each
        // entry (IPv4 or IPv6) and whether or not multicast is actually supported
        // on the interface.
        //
        // This next condition may be a bit confusing, so we break it out a bit
        // for clarity.  We can posibly use an interface if it supports either
        // multicast or broadcast.  What we want to do is to detect the
        // condition when we cannot use it, so we invert the logic.  That means
        // !multicast && !broadcast.  Not being able to support broadcast is
        // also true if we don't want to (i.e., m_broadcast is false).  This
        // expression then looks like  !multicast && (!broadcast || !m_broadcast).
        // broadcast really implies AF_INET since there is no broadcast in IPv6
        // but we double-check this condition and come up with:
        //
        //   !multicast && (!broadcast || !m_broadcast || !AF_INET).
        //
        // To avoid a horribly complicated if statement, we make it look like
        // the above explanation.  The resulting debug print is intimidating,
        // but it says exactly the right thing for those in the know.
        //
        bool multicast = (entries[i].m_flags & qcc::IfConfigEntry::MULTICAST) != 0;
        bool broadcast = (entries[i].m_flags & qcc::IfConfigEntry::BROADCAST) != 0;
        bool af_inet = entries[i].m_family == qcc::QCC_AF_INET;

        if (!multicast && (!broadcast || !m_broadcast || !af_inet)) {
            QCC_DbgPrintf(("LazyUpdateInterfaces: !multicast && (!broadcast || !m_broadcast || !af_inet).  Ignoring"));
            continue;
        }

        //
        // We've decided the interface in question is interesting and we want to
        // use it to send and receive name service messages.  Now we need to
        // start the long process of convincing the network to do what we want.
        // This is going to mostly be done by setting a series of socket
        // options.  The small number of the ones we need are absracted in the
        // qcc package.
        //
        qcc::SocketFd sockFd;

        if (entries[i].m_family == qcc::QCC_AF_INET) {
            QStatus status = qcc::Socket(qcc::QCC_AF_INET, qcc::QCC_SOCK_DGRAM, sockFd);
            if (status != ER_OK) {
                QCC_LogError(status, ("LazyUpdateInterfaces: qcc::Socket(AF_INET) failed: %d - %s",
                                      qcc::GetLastError(), qcc::GetLastErrorString().c_str()));
                continue;
            }

            //
            // If we're going to send broadcasts, we have to ask for
            // permission.
            //
            if (m_broadcast && entries[i].m_flags & qcc::IfConfigEntry::BROADCAST) {
                status = qcc::SetBroadcast(sockFd, true);
                if (status != ER_OK) {
                    QCC_LogError(status, ("LazyUpdateInterfaces: enable broadcast failed"));
                    continue;
                }
            }
        } else if (entries[i].m_family == qcc::QCC_AF_INET6) {
            QStatus status = qcc::Socket(qcc::QCC_AF_INET6, qcc::QCC_SOCK_DGRAM, sockFd);
            if (status != ER_OK) {
                QCC_LogError(status, ("LazyUpdateInterfaces: qcc::Socket(AF_INET6) failed: %d - %s",
                                      qcc::GetLastError(), qcc::GetLastErrorString().c_str()));
                continue;
            }
        } else {
            assert(!"IpNameServiceImpl::LazyUpdateInterfaces(): Unexpected value in m_family (not AF_INET or AF_INET6");
            continue;
        }

        //
        // We must be able to reuse the address/port combination so other
        // AllJoyn daemon instances on the same host can listen in if desired.
        // This will set the SO_REUSEPORT socket option if available or fall
        // back onto SO_REUSEADDR if not.
        //
        status = qcc::SetReusePort(sockFd, true);
        if (status != ER_OK && status != ER_NOT_IMPLEMENTED) {
            QCC_LogError(status, ("IpNameServiceImpl::LazyUpdateInterfaces(): SetReusePort() failed"));
            qcc::Close(sockFd);
            continue;
        }

        //
        // If the MULTICAST flag is set, we are going to try and multicast out
        // over the interface in question.  If the MULTICAST flag is not set,
        // then we want to fall back to IPv4 subnet directed broadcast, so we
        // optionally do all of the multicast games and take the interface live
        // even if it doesn't support multicast.
        //
        if (entries[i].m_flags & qcc::IfConfigEntry::MULTICAST) {
            //
            // Restrict the scope of the sent muticast packets to the local subnet.
            //
            status = qcc::SetMulticastHops(sockFd, entries[i].m_family, 1);
            if (status != ER_OK && status != ER_NOT_IMPLEMENTED) {
                QCC_LogError(status, ("IpNameServiceImpl::LazyUpdateInterfaces(): SetMulticastHops() failed"));
                qcc::Close(sockFd);
            }

            //
            // In order to control which interfaces get our multicast datagrams, it
            // is necessary to do so via a socket option.  See the Long Sidebar above.
            // Yes, you have to do it differently depending on whether or not you're
            // using IPv4 or IPv6.
            //
            status = qcc::SetMulticastInterface(sockFd, entries[i].m_family, entries[i].m_name);
            if (status != ER_OK && status != ER_NOT_IMPLEMENTED) {
                QCC_LogError(status, ("IpNameServiceImpl::LazyUpdateInterfaces(): SetMulticastInterface() failed"));
                qcc::Close(sockFd);
                continue;
            }
        }

        //
        // We are going to end up binding the socket to the interface specified
        // by the IP address in the interface list, but with multicast, things
        // are a little different.  Binding to INADDR_ANY is the correct thing
        // to do.  The See the Long Sidebar above.
        //
        if (entries[i].m_family == qcc::QCC_AF_INET) {
            status = qcc::Bind(sockFd, qcc::IPAddress("0.0.0.0"), MULTICAST_PORT);
            if (status != ER_OK) {
                QCC_LogError(status, ("IpNameServiceImpl::LazyUpdateInterfaces(): bind(0.0.0.0) failed"));
                qcc::Close(sockFd);
                continue;
            }
        } else if (entries[i].m_family == qcc::QCC_AF_INET6) {
            status = qcc::Bind(sockFd, qcc::IPAddress("::"), MULTICAST_PORT);
            if (status != ER_OK) {
                QCC_LogError(status, ("IpNameServiceImpl::LazyUpdateInterfaces(): bind(::) failed"));
                qcc::Close(sockFd);
                continue;
            }
        }

        //
        // The IGMP join must be done after the bind for Windows XP.  Other
        // OSes are fine with it, but XP balks.
        //
        if (entries[i].m_flags & qcc::IfConfigEntry::MULTICAST) {
            //
            // Arrange an IGMP join via the appropriate socket option (via the
            // qcc abstraction layer). Android doesn't bother to compile its
            // kernel with CONFIG_IP_MULTICAST set.  This doesn't mean that
            // there is no multicast code in the Android kernel, it means there
            // is no IGMP code in the kernel.  What this means to us is that
            // even through we are doing an IP_ADD_MEMBERSHIP request, which is
            // ultimately an IGMP operation, the request will filter through the
            // IP code before being ignored and will do useful things in the
            // kernel even though CONFIG_IP_MULTICAST was not set for the
            // Android build -- i.e., we have to do it anyway.
            //
            if (entries[i].m_family == qcc::QCC_AF_INET) {
#if 1
                status = qcc::JoinMulticastGroup(sockFd, qcc::QCC_AF_INET, IPV4_ALLJOYN_MULTICAST_GROUP, entries[i].m_name);
#endif
            } else if (entries[i].m_family == qcc::QCC_AF_INET6) {
                status = qcc::JoinMulticastGroup(sockFd, qcc::QCC_AF_INET6, IPV6_ALLJOYN_MULTICAST_GROUP, entries[i].m_name);
            }
            if (status != ER_OK) {
                QCC_LogError(status, ("IpNameServiceImpl::LazyUpdateInterfaces(): unable to join multicast group"));
                qcc::Close(sockFd);
                continue;
            }
        }

        //
        // Now take the interface "live."
        //
        LiveInterface live;
        live.m_interfaceName = entries[i].m_name;
        live.m_interfaceAddr = entries[i].m_addr;
        live.m_prefixlen = entries[i].m_prefixlen;
        live.m_address = qcc::IPAddress(entries[i].m_addr);
        live.m_flags = entries[i].m_flags;
        live.m_mtu = entries[i].m_mtu;
        live.m_index = entries[i].m_index;
        live.m_sockFd = sockFd;
        live.m_event = new qcc::Event(sockFd, qcc::Event::IO_READ, false);
        m_liveInterfaces.push_back(live);
    }
}

QStatus IpNameServiceImpl::Enable(TransportMask transportMask,
                                  uint16_t reliableIPv4Port, uint16_t reliableIPv6Port,
                                  uint16_t unreliableIPv4Port, uint16_t unreliableIPv6Port)
{
    //
    // Version zero of the name service uses a single port, m_port, that
    // corresponds directly to the reliable IPv4 port of version one.  Version
    // one admits the possibility of four ports.
    //
    // XXX These must be set on a per-transport basis.
    //
    m_port = m_reliableIPv4Port = reliableIPv4Port;
    m_unreliableIPv4Port = unreliableIPv4Port;
    m_reliableIPv6Port = reliableIPv6Port;
    m_unreliableIPv6Port = reliableIPv6Port;

    //
    // XXX This doesn't make sense now.  doDisable and doEnable are global across
    // all transports, not for each transport.
    //
    if (m_reliableIPv4Port || m_unreliableIPv4Port || m_reliableIPv6Port || m_unreliableIPv6Port) {
        //
        // If a previous disable request has not yet been serviced, remove the
        // request. Only the latest request must be serviced and that is this
        // enable (since the port is non-zero).
        //
        m_doDisable = false;
        m_doEnable = true;
    } else {
        //
        // If the previous enable request has not yet been serviced, remove the
        // request. Only the latest request must be serviced and that is this
        // disable (since the port is zero).
        //
        m_doEnable = false;
        m_doDisable = true;
    }

    m_forceLazyUpdate = true;
    m_wakeEvent.SetEvent();

    return ER_OK;
}

QStatus IpNameServiceImpl::Enabled(TransportMask transportMask,
                                   uint16_t& reliableIPv4Port, uint16_t& reliableIPv6Port,
                                   uint16_t& unreliableIPv4Port, uint16_t& unreliableIPv6Port)
{
    reliableIPv4Port = m_reliableIPv4Port;
    reliableIPv6Port = 0;
    unreliableIPv4Port = m_unreliableIPv4Port;
    unreliableIPv4Port = 0;

    return ER_OK;
}

QStatus IpNameServiceImpl::Locate(const qcc::String& wkn, LocatePolicy policy)
{
    QCC_DbgHLPrintf(("IpNameServiceImpl::Locate(): %s with policy %d", wkn.c_str(), policy));

    //
    // Send a request to the network over our multicast channel, asking for
    // anyone who supports the specified well-known name.
    //
    // We are now at version one of the protocol.  There is no significant
    // difference between version zero and version one messages, but down-version
    // (version zero) clients don't know that, so they will ignore version one
    // messages.  This means that if we want to have clients running older daemons
    // be able to hear our discovery requests, we need to send both flavors of
    // message.  Since the version is located in the message header, this means
    // two messages.
    //
    // Do it once for version zero.
    //
    {
        WhoHas whoHas;

        //
        // We understand all messages from version zero to version one, but we
        // are sending a version zero message.  The whole point of sending a
        // version zero message is that can be understood by down-level code
        // so we can't use the new versioning scheme.  We have to use some
        // sneaky way to tell an in-the know version one client that the
        // packet is from a version one client and that is through the setting
        // of the UDP flag.
        //
        whoHas.SetVersion(0, 0);
        whoHas.SetTcpFlag(true);
        whoHas.SetUdpFlag(true);
        whoHas.SetIPv4Flag(true);
        whoHas.AddName(wkn);

        Header header;
        whoHas.SetVersion(0, 0);
        header.SetTimer(m_tDuration);
        header.AddQuestion(whoHas);

        //
        // We may want to retransmit this request a few times depending on our
        // retry policy, so add it to the list of messages to retry.
        //
        m_mutex.Lock();
        m_retry.push_back(header);
        m_mutex.Unlock();

        //
        // Queue this message for transmission out on the various live interfaces.
        //
        QueueProtocolMessage(header);
    }

    //
    // Do it again for version one.
    //
    {
        WhoHas whoHas;

        //
        // We understand all messages from version zero to version one, and we
        // are sending a version one message;
        //
        whoHas.SetVersion(1, 1);

        whoHas.AddName(wkn);

        Header header;
        header.SetVersion(1, 1);
        header.SetTimer(m_tDuration);
        header.AddQuestion(whoHas);

        //
        // We may want to retransmit this request a few times depending on our
        // retry policy, so add it to the list of messages to retry.
        //
        m_mutex.Lock();
        m_retry.push_back(header);
        m_mutex.Unlock();

        //
        // Queue this message for transmission out on the various live interfaces.
        //
        QueueProtocolMessage(header);
    }

    return ER_OK;
}

void IpNameServiceImpl::SetCriticalParameters(
    uint32_t tDuration,
    uint32_t tRetransmit,
    uint32_t tQuestion,
    uint32_t modulus,
    uint32_t retries)
{
    m_tDuration = tDuration;
    m_tRetransmit = tRetransmit;
    m_tQuestion = tQuestion;
    m_modulus = modulus;
    m_retries = retries;
}

void IpNameServiceImpl::SetCallback(Callback<void, const qcc::String&, const qcc::String&, vector<qcc::String>&, uint8_t>* cb)
{
    Callback<void, const qcc::String&, const qcc::String&, vector<qcc::String>&, uint8_t>*  goner = m_callback;

    QCC_DbgPrintf(("IpNameServiceImpl::SetCallback()"));

    m_callback = NULL;
    delete goner;
    m_callback = cb;
}

QStatus IpNameServiceImpl::AdvertiseName(TransportMask transportMask, const qcc::String& wkn)
{
    QCC_DbgHLPrintf(("IpNameServiceImpl::AdvertiseName(): %s", wkn.c_str()));

    vector<qcc::String> wknVector;
    wknVector.push_back(wkn);

    return AdvertiseName(transportMask, wknVector);
}

QStatus IpNameServiceImpl::AdvertiseName(TransportMask transportMask, vector<qcc::String>& wkn)
{
    QCC_DbgHLPrintf(("IpNameServiceImpl::AdvertiseName()"));

    if (m_state != IMPL_RUNNING) {
        QCC_DbgPrintf(("IpNameServiceImpl::AdvertiseName(): Not IMPL_RUNNING"));
        return ER_FAIL;
    }

    //
    // There are at least two threads wandering through the advertised list.
    // We are running short on toes, so don't shoot any more off by not being
    // thread-unaware.
    //
    m_mutex.Lock();

    //
    // Make a note to ourselves which services we are advertising so we can
    // respond to protocol questions in the future.  Only allow one entry per
    // name.
    //
    for (uint32_t i = 0; i < wkn.size(); ++i) {
        list<qcc::String>::iterator j = find(m_advertised.begin(), m_advertised.end(), wkn[i]);
        if (j == m_advertised.end()) {
            m_advertised.push_back(wkn[i]);
        } else {
            //
            // Nothing has changed, so don't bother.
            //
            QCC_DbgPrintf(("IpNameServiceImpl::AdvertiseName(): Duplicate advertisement"));
            m_mutex.Unlock();
            return ER_OK;
        }
    }

    //
    // Keep the list sorted so we can easily distinguish a change in
    // the content of the advertised names versus a change in the order of the
    // names.
    //
    m_advertised.sort();

    //
    // If the advertisement retransmission timer is cleared, then set us
    // up to retransmit.  This has to be done with the mutex locked since
    // the main thread is playing with this value as well.
    //
    if (m_timer == 0) {
        m_timer = m_tDuration;
    }

    m_mutex.Unlock();

    //
    // We are now at version one of the protocol.  There is a significant
    // difference between version zero and version one messages, so down-version
    // (version zero) clients will not know what to do with version one
    // messages.  This means that if we want to have clients running older
    // daemons be able to hear our advertisements, we need to send both flavors
    // of message.  Since the version is located in the message header, this
    // means two messages.
    //
    // Do it once for version zero.
    //
    {
        //
        // The underlying protocol is capable of identifying both TCP and UDP
        // services.  Right now, the only possibility is TCP, so this is not
        // exposed to the user unneccesarily.
        //
        IsAt isAt;

        //
        // We understand all messages from version zero to version one, and we
        // are sending a version zero message.  The whole point of sending a
        // version zero message is that can be understood by down-level code
        // so we can't use the new versioning scheme.  We have to use some
        // sneaky way to tell an in-the know version one client that the
        // packet is from a version one client and that is through the setting
        // of the UDP flag.
        //
        isAt.SetVersion(0, 0);
        isAt.SetTcpFlag(true);
        isAt.SetUdpFlag(true);

        //
        // Always send the provided daemon GUID out with the reponse.
        //
        isAt.SetGuid(m_guid);

        //
        // Send a protocol message describing the entire list of names we have
        // for the provided protocol.
        //
        isAt.SetCompleteFlag(true);

        //
        // Set the port here.  When the message goes out a selected interface, the
        // protocol handler will write out the addresses according to its rules.
        //
        isAt.SetPort(m_port);

        //
        // Add the provided names to the is-at message that will be sent out on the
        // network.
        //
        for (uint32_t i = 0; i < wkn.size(); ++i) {
            isAt.AddName(wkn[i]);
        }

        //
        // The header ties the whole protocol message together.  By setting the
        // timer, we are asking for everyone who hears the message to remember
        // the advertisements for that number of seconds.
        //
        Header header;
        header.SetVersion(0, 0);
        header.SetTimer(m_tDuration);
        header.AddAnswer(isAt);

        //
        // We don't want allow the caller to advertise an unlimited number of names
        // and consume all available network resources.  We expect Advertise() to
        // typically be called once per advertised name, but since we allow a vector
        // of names we need to limit that size somehow.  The easy way is to assume
        // that all of the names are the maximum size and just limit based on the
        // maximum NS packet size and the maximum name size of 256 bytes.  This,
        // however, leaves just five names which seems too restrictive.  So, we do
        // it the more time-consuming way and put together the message and then see
        // if it's "too big."
        //
        // This isn't terribly elegant, but we don't know the IP address(es) over
        // which the message will be sent.  These are added in the loop that
        // actually does the packet sends, with the interface addresses dynamically
        // added onto the message.  We have no clue here if an IPv4 or IPv6 or both
        // flavors of address will exist on a given interface, nor how many
        // interfaces there are.  All we can do here is to assume the worst case for
        // the size (both exist) and add the 20 bytes (four for IPv4, sixteen for
        // IPv6) that the addresses may consume in the final packet.
        //
        if (header.GetSerializedSize() + 20 <= NS_MESSAGE_MAX) {
            //
            // Queue this message for transmission out on the various live interfaces.
            //
            QueueProtocolMessage(header);
        } else {
            QCC_LogError(ER_PACKET_TOO_LARGE, ("IpNameServiceImpl::Advertise(): Resulting NS message too large"));
            return ER_PACKET_TOO_LARGE;
        }
    }

    //
    // Do it once for version one.
    //
    {
        IsAt isAt;

        //
        // We understand all messages from version zero to version one, and we
        // are sending a version one message;
        //
        isAt.SetVersion(1, 1);

        isAt.SetTransportMask(transportMask);

        //
        // Version one allows us to provide four possible endpoints.
        // XXX FIXME These should be stored per-transport mask (16 possible).
        //
        if (m_reliableIPv4Port) {
            isAt.SetReliableIPv4(m_reliableIPv4Address, m_reliableIPv4Port);
        }
        if (m_unreliableIPv4Port) {
            isAt.SetUnreliableIPv4(m_unreliableIPv4Address, m_unreliableIPv4Port);
        }
        if (m_reliableIPv6Port) {
            isAt.SetReliableIPv6(m_reliableIPv6Address, m_reliableIPv6Port);
        }
        if (m_unreliableIPv6Port) {
            isAt.SetUnreliableIPv6(m_unreliableIPv6Address, m_unreliableIPv6Port);
        }

        //
        // Always send the provided daemon GUID out with the reponse.
        //
        isAt.SetGuid(m_guid);

        //
        // Send a protocol message describing the entire list of names we have
        // for the provided protocol.
        //
        isAt.SetCompleteFlag(true);

        //
        // Add the provided names to the is-at message that will be sent out on the
        // network.
        //
        for (uint32_t i = 0; i < wkn.size(); ++i) {
            isAt.AddName(wkn[i]);
        }

        //
        // The header ties the whole protocol message together.  By setting the
        // timer, we are asking for everyone who hears the message to remember
        // the advertisements for that number of seconds.
        //
        Header header;
        header.SetVersion(1, 1);
        header.SetTimer(m_tDuration);
        header.AddAnswer(isAt);

        //
        // We don't want allow the caller to advertise an unlimited number of names
        // and consume all available network resources.  We expect Advertise() to
        // typically be called once per advertised name, but since we allow a vector
        // of names we need to limit that size somehow.  The easy way is to assume
        // that all of the names are the maximum size and just limit based on the
        // maximum NS packet size and the maximum name size of 256 bytes.  This,
        // however, leaves just five names which seems too restrictive.  So, we do
        // it the more time-consuming way and put together the message and then see
        // if it's "too big."
        //
        // This isn't terribly elegant, but we don't know the IP address(es) over
        // which the message will be sent.  These are added in the loop that
        // actually does the packet sends, with the interface addresses dynamically
        // added onto the message.  We have no clue here if an IPv4 or IPv6 or both
        // flavors of address will exist on a given interface, nor how many
        // interfaces there are.  All we can do here is to assume the worst case for
        // the size (both exist) and add the 20 bytes (four for IPv4, sixteen for
        // IPv6) that the addresses may consume in the final packet.
        //
        if (header.GetSerializedSize() + 20 <= NS_MESSAGE_MAX) {
            //
            // Queue this message for transmission out on the various live interfaces.
            //
            QueueProtocolMessage(header);
        } else {
            QCC_LogError(ER_PACKET_TOO_LARGE, ("IpNameServiceImpl::AdvertiseName(): Resulting NS message too large"));
            return ER_PACKET_TOO_LARGE;
        }
    }

    return ER_OK;
}

QStatus IpNameServiceImpl::CancelAdvertiseName(TransportMask transportMask, const qcc::String& wkn)
{
    QCC_DbgPrintf(("IpNameServiceImpl::CancelAdvertiseName(): %s", wkn.c_str()));

    vector<qcc::String> wknVector;
    wknVector.push_back(wkn);

    return CancelAdvertiseName(transportMask, wknVector);
}

QStatus IpNameServiceImpl::CancelAdvertiseName(TransportMask transportMask, vector<qcc::String>& wkn)
{
    QCC_DbgPrintf(("IpNameServiceImpl::CancelAdvertiseName()"));

    if (m_state != IMPL_RUNNING) {
        QCC_DbgPrintf(("IpNameServiceImpl::CancelAdvertiseName(): Not IMPL_RUNNING"));
        return ER_FAIL;
    }

    //
    // There are at least two threads wandering through the advertised list.
    // We are running short on toes, so don't shoot any more off by not being
    // thread-unaware.
    //
    m_mutex.Lock();

    //
    // Remove the given services from our list of services we are advertising.
    //
    bool changed = false;

    for (uint32_t i = 0; i < wkn.size(); ++i) {
        list<qcc::String>::iterator j = find(m_advertised.begin(), m_advertised.end(), wkn[i]);
        if (j != m_advertised.end()) {
            m_advertised.erase(j);
            changed = true;
        }
    }

    //
    // If we have no more advertisements, there is no need to repeatedly state
    // this so turn off the retransmit timer.  The main thread is playing with
    // this number too, so this must be done with the mutex locked.
    //
    if (m_advertised.size() == 0) {
        m_timer = 0;
    }

    m_mutex.Unlock();

    //
    // If we didn't actually make a change, just return.
    //
    if (changed == false) {
        return ER_OK;
    }

    //
    // We are now at version one of the protocol.  There is a significant
    // difference between version zero and version one messages, so down-version
    // (version zero) clients will not know what to do with versino one
    // messages.  This means that if we want to have clients running older
    // daemons be able to hear our advertisements, we need to send both flavors
    // of message.  Since the version is located in the message header, this
    // means two messages.
    //
    // Do it once for version zero.
    //
    {
        //
        // Send a protocol answer message describing the list of names we have just
        // been asked to withdraw.
        //
        // This code assumes that the daemon talks over TCP.  True for now.
        //
        IsAt isAt;

        //
        // We understand all messages from version zero to version one, and we
        // are sending a version zero message.  The whole point of sending a
        // version zero message is that can be understood by down-level code
        // so we can't use the new versioning scheme.  We have to use some
        // sneaky way to tell an in-the know version one client that the
        // packet is from a version one client and that is through the setting
        // of the UDP flag.
        //
        isAt.SetVersion(0, 0);
        isAt.SetTcpFlag(true);
        isAt.SetUdpFlag(true);

        //
        // Always send the provided daemon GUID out with the reponse.
        //
        isAt.SetGuid(m_guid);

        //
        // Set the port here.  When the message goes out a selected interface, the
        // protocol handler will write out the addresses according to its rules.
        //
        isAt.SetPort(m_port);

        //
        // Copy the names we are withdrawing the advertisement for into the
        // protocol message object.
        //
        for (uint32_t i = 0; i < wkn.size(); ++i) {
            isAt.AddName(wkn[i]);
        }

        //
        // When withdrawing advertisements, a complete flag means that we are
        // withdrawing all of the advertisements.  If the complete flag is
        // not set, we have some advertisements remaining.
        //
        if (m_advertised.size() == 0) {
            isAt.SetCompleteFlag(true);
        }

        //
        // The header ties the whole protocol message together.  We're at version
        // zero of the protocol.
        //
        Header header;
        header.SetVersion(0, 0);

        //
        // We want to signal that everyone can forget about these names
        // so we set the timer value to 0.
        //
        header.SetTimer(0);
        header.AddAnswer(isAt);

        //
        // Queue this message for transmission out on the various live interfaces.
        //
        QueueProtocolMessage(header);
    }

    //
    // Do it once for version one.
    //
    {
        //
        // Send a protocol answer message describing the list of names we have just
        // been asked to withdraw.
        //
        IsAt isAt;

        //
        // We understand all messages from version zero to version one, and we
        // are sending a version one message;
        //
        isAt.SetVersion(1, 1);

        //
        // Version one allows us to provide four possible endpoints.
        // XXX FIXME These should be stored per-transport mask (16 possible).
        //
        if (m_reliableIPv4Port) {
            isAt.SetReliableIPv4(m_reliableIPv4Address, m_reliableIPv4Port);
        }
        if (m_unreliableIPv4Port) {
            isAt.SetUnreliableIPv4(m_unreliableIPv4Address, m_unreliableIPv4Port);
        }
        if (m_reliableIPv6Port) {
            isAt.SetReliableIPv6(m_reliableIPv6Address, m_reliableIPv6Port);
        }
        if (m_unreliableIPv6Port) {
            isAt.SetUnreliableIPv6(m_unreliableIPv6Address, m_unreliableIPv6Port);
        }

        //
        // Always send the provided daemon GUID out with the reponse.
        //
        isAt.SetGuid(m_guid);

        //
        // Copy the names we are withdrawing the advertisement for into the
        // protocol message object.
        //
        for (uint32_t i = 0; i < wkn.size(); ++i) {
            isAt.AddName(wkn[i]);
        }

        //
        // When withdrawing advertisements, a complete flag means that we are
        // withdrawing all of the advertisements.  If the complete flag is
        // not set, we have some advertisements remaining.
        //
        if (m_advertised.size() == 0) {
            isAt.SetCompleteFlag(true);
        }

        //
        // The header ties the whole protocol message together.  We're at version
        // one of the protocol.
        //
        Header header;
        header.SetVersion(1, 1);

        //
        // We want to signal that everyone can forget about these names
        // so we set the timer value to 0.
        //
        header.SetTimer(0);
        header.AddAnswer(isAt);

        //
        // Queue this message for transmission out on the various live interfaces.
        //
        QueueProtocolMessage(header);
    }

    return ER_OK;
}

void IpNameServiceImpl::QueueProtocolMessage(Header& header)
{
    QCC_DbgPrintf(("IpNameServiceImpl::QueueProtocolMessage()"));

    m_mutex.Lock();
    m_outbound.push_back(header);
    m_wakeEvent.SetEvent();
    m_mutex.Unlock();
}

//
// If you set HAPPY_WANDERER to 1, it will enable a test behavior that
// simulates the daemon happily wandering in and out of range of an
// imaginary access point.
//
// It is essentially a trivial one-dimentional random walk across a fixed
// domain.  When Wander() is called, der froliche wandering daemon moves
// in a random direction for one meter.  When the daemon "walks" out of
// range, Wander() returns false and the test will arrange that name
// service messages are discarded.  When the daemon "walks" back into
// range, messages are delivered again.  We generally call Wander() out
// DoPeriodicMaintenance() which ticks every second, but also out of
// HandleProtocolAnswer() so the random walk is at a non-constant rate
// driven by network activity.  Very nasty.
//
// The environment is 100 meters long, and the range of the access point
// is 50 meters.  The daemon starts right at the edge of the range and is
// expected to hover around that point, but wander random distances in and
// out.
//
//   (*)                       X                         |
//    |                     <- D ->                      |
//    ---------------------------------------------------
//    0                        50                       100
//
// Since this is a very dangerous setting, turning it on is a two-step
// process (set the #define and enable the bool); and we log every action
// as an error.  It will be hard to ignore this and accidentally leave it
// turned on.
//
#define HAPPY_WANDERER 0

#if HAPPY_WANDERER

static const uint32_t WANDER_LIMIT = 100;
static const uint32_t WANDER_RANGE = WANDER_LIMIT / 2;
static const uint32_t WANDER_START = WANDER_RANGE;

bool g_enableWander = false;

void WanderInit(void)
{
    srand(time(NULL));
}

bool Wander(void)
{
    //
    // If you don't explicitly enable this behavior, Wander() always returns
    // "in-range".
    //
    if (g_enableWander == false) {
        return true;
    }

    static uint32_t x = WANDER_START;
    static bool xyzzy = false;

    if (xyzzy == false) {
        WanderInit();
        xyzzy = true;
    }

    switch (x) {
    case 0:
        // Valderi
        ++x;
        break;

    case WANDER_LIMIT:
        // Valdera
        --x;
        break;

    default:
        // Valderahahahahahaha
        x += rand() & 1 ? 1 : -1;
        break;
    }

    QCC_LogError(ER_FAIL, ("Wander(): Wandered to %d which %s in-range", x, x < WANDER_RANGE ? "is" : "is NOT"));

    return x < WANDER_RANGE;
}

#endif

void IpNameServiceImpl::SendProtocolMessage(
    qcc::SocketFd sockFd,
    qcc::IPAddress interfaceAddress,
    uint32_t interfaceAddressPrefixLen,
    uint32_t flags,
    bool sockFdIsIPv4,
    Header& header)
{
    QCC_DbgHLPrintf(("IpNameServiceImpl::SendProtocolMessage()"));

    //
    // Legacy 802.11 MACs do not do backoff and retransmission of packets
    // destined for multicast addresses.  Therefore if there is a collision
    // on the air, a multicast packet will be silently dropped.  We get
    // no indication of this at all up at the Socket level.  When a remote
    // daemon makes a WhoHas request, we have to be very careful about
    // ensuring that all other daemons on the net don't try to respond at
    // the same time.  That would mean that all responses could result in
    // collisions and all responses would be lost.  We delay a short
    // random time before sending anything to avoid the thundering herd.
    //
    // This also works to limit the number of name service messages that
    // can be sent out on the network for any given amount of time; which
    // is good for preventing our users from trying to advertise zillions
    // of names and tie up the net.
    //
    qcc::Sleep(rand() % 128);

#if HAPPY_WANDERER
    if (Wander() == false) {
        QCC_LogError(ER_FAIL, ("IpNameServiceImpl::SendProtocolMessage(): Wander(): out of range"));
        return;
    } else {
        QCC_LogError(ER_FAIL, ("IpNameServiceImpl::SendProtocolMessage(): Wander(): in range"));
    }
#endif

    size_t size = header.GetSerializedSize();

    if (size > NS_MESSAGE_MAX) {
        QCC_LogError(ER_FAIL, ("SendProtocolMessage: Message (%d bytes) is longer than NS_MESSAGE_MAX (%d bytes)",
                               size, NS_MESSAGE_MAX));
        return;
    }

    uint8_t* buffer = new uint8_t[size];
    header.Serialize(buffer);

    //
    // Now it's time to send the packets.  Packets is plural since we will try
    // to get our name service information across to peers in as many ways as is
    // reasonably possible since it turns out that discovery is a weak link in
    // the system.  This means we will try broadcast and IPv6 multicast whenever
    // possible.
    //
    size_t sent;
    if (sockFdIsIPv4) {
        //
        // If the underlying interface told us that it suported multicast, send
        // the packet out on our IPv4 multicast groups (IANA registered and
        // legacy).
        //
#if 1
        if (flags & qcc::IfConfigEntry::MULTICAST) {
            QCC_DbgPrintf(("IpNameServiceImpl::SendProtocolMessage():  Sending to IPv4 Local Network Control Block multicast group"));
            qcc::IPAddress ipv4LocalMulticast(IPV4_ALLJOYN_MULTICAST_GROUP);
            QStatus status = qcc::SendTo(sockFd, ipv4LocalMulticast, MULTICAST_PORT, buffer, size, sent);
            if (status != ER_OK) {
                QCC_LogError(ER_FAIL, ("IpNameServiceImpl::SendProtocolMessage():  Error sending to IPv4 Local Network Control Block multicast group"));
            }
        }
#endif

        //
        // If the interface is broadcast-capable, We want to send out a subnet
        // directed broadcast over IPv4.
        //
        if (flags & qcc::IfConfigEntry::BROADCAST) {
            //
            // If there was a problem getting the IP address prefix
            // length, it will come in as -1.  In this case, we can't form
            // a proper subnet directed broadcast and so we don't try.  An
            // error will have been logged when we did the IfConfig, so
            // don't flood out any more, just silenty ignore the problem.
            //
            if (m_broadcast && interfaceAddressPrefixLen != static_cast<uint32_t>(-1)) {
                //
                // In order to ensure that our broadcast goes to the correct
                // interface and is not just sent out some default way, we
                // have to form a subnet directed broadcast.  To do this we need
                // the IP address and netmask.
                //
                QCC_DbgPrintf(("IpNameServiceImpl::SendProtocolMessage():  InterfaceAddress %s, prefix %d",
                               interfaceAddress.ToString().c_str(), interfaceAddressPrefixLen));

                //
                // Create a netmask with a one in the leading bits for each position
                // implied by the prefix length.
                //
                uint32_t mask = 0;
                for (uint32_t i = 0; i < interfaceAddressPrefixLen; ++i) {
                    mask >>= 1;
                    mask |= 0x80000000;
                }

                //
                // The subnet directed broadcast address is the address part of the
                // interface address (defined by the mask) with the rest of the bits
                // set to one.
                //
                uint32_t addr = (interfaceAddress.GetIPv4AddressCPUOrder() & mask) | ~mask;
                qcc::IPAddress ipv4Broadcast(addr);
                QCC_DbgPrintf(("IpNameServiceImpl::SendProtocolMessage():  Sending to subnet directed broadcast address %s",
                               ipv4Broadcast.ToString().c_str()));

                QStatus status = qcc::SendTo(sockFd, ipv4Broadcast, BROADCAST_PORT, buffer, size, sent);
                if (status != ER_OK) {
                    QCC_LogError(ER_FAIL, ("IpNameServiceImpl::SendProtocolMessage():  Error sending to IPv4 (broadcast)"));
                }
            } else {
                QCC_DbgPrintf(("IpNameServiceImpl::SendProtocolMessage():  Subnet directed broadcasts are disabled"));
            }
        } else {
            QCC_DbgPrintf(("IpNameServiceImpl::SendProtocolMessage():  Interface does not support broadcast"));
        }
    } else {
        if (flags & qcc::IfConfigEntry::MULTICAST) {
            QCC_DbgPrintf(("IpNameServiceImpl::SendProtocolMessage():  Sending to IPv6 Link-Local Scope multicast group"));
            qcc::IPAddress ipv6AllJoyn(IPV6_ALLJOYN_MULTICAST_GROUP);
            QStatus status = qcc::SendTo(sockFd, ipv6AllJoyn, MULTICAST_PORT, buffer, size, sent);
            if (status != ER_OK) {
                QCC_LogError(ER_FAIL, ("IpNameServiceImpl::SendProtocolMessage():  Error sending to IPv6 Link-Local Scope multicast group "));
            }
        }
    }

    delete [] buffer;
}

void IpNameServiceImpl::SendOutboundMessages(void)
{
    //
    // We know what interfaces can be currently used to send messages
    // over, so now send any messages we have queued for transmission.
    //
    while (m_outbound.size() && (m_state == IMPL_RUNNING || m_terminal)) {

        QCC_DbgPrintf(("IpNameServiceImpl::SendOutboundMessages(): m_outbound.size() == %d.", m_outbound.size()));

        //
        // Pull a message off of the outbound queue.  What we get is a
        // header object that will tie together a number of "question"
        // (who-has) objects and a number of "answer" (is-at) objects.
        //
        Header header = m_outbound.front();

        //
        // When higher level code queues up messages, it doesn't know to what
        // interfaces and therefore over what source IP addresses it will be
        // using.  We expect the transport listeners to be listening to the
        // appropriate INADDR_ANY address, and relying on us to get the IP
        // addressing information of the various networks we are talking to
        // correct.  What this means is that we are going to rewrite any IP
        // addresses into is-at messages on the fly as we prepare to send them
        // out our sundry interfaces.  who-has messages don't include any
        // source addresses, so we leave them as-is.
        //
        // IP-based transports can implement four basic mechanisms for moving
        // bits (reliable IPv4, unreliable IPv4, reliable IPv6 and unreliable
        // IPv4), so we need to communicate addresses in the contents of the
        // name service packets and can not rely on packet source addresses at
        // all.
        //
        // So, we walk the list of live interfaces looking for those with IPv4
        // or IPv6 addresses, rewrite the messages as required for those
        // interfaces and send them out.
        //
        QCC_DbgPrintf(("IpNameServiceImpl::SendOutboundMessages(): Walk interfaces"));
        for (uint32_t i = 0; (m_state == IMPL_RUNNING || m_terminal) && (i < m_liveInterfaces.size()); ++i) {
            QCC_DbgPrintf(("IpNameServiceImpl::SendOutboundMessages(): Checking out live interface %d.", i));

            //
            // Don't bother if the socket FD isn't initialized, since we
            // wouldn't be able to send anything anyway.
            //
            if (m_liveInterfaces[i].m_sockFd != -1) {

                QCC_DbgPrintf(("IpNameServiceImpl::SendOutboundMessages(): Interface %d. is live and IPv6", i));

                uint32_t interfaceAddressPrefixLen = m_liveInterfaces[i].m_prefixlen;
                uint32_t flags = m_liveInterfaces[i].m_flags;

                qcc::IPAddress ipv4Address;
                bool haveIPv4Address = m_liveInterfaces[i].m_address.IsIPv4();
                if (haveIPv4Address) {
                    ipv4Address = m_liveInterfaces[i].m_address;
                }
                bool interfaceIsIPv4 = haveIPv4Address;

                qcc::IPAddress ipv6Address;
                bool haveIPv6Address = m_liveInterfaces[i].m_address.IsIPv6();
                if (haveIPv6Address) {
                    ipv6Address = m_liveInterfaces[i].m_address;
                }

                //
                // In order to get all of the information we need, if the
                // current address is IPv4, we need to scan for an IPv6 address.
                // If the current address is IPv6, we need to scan for an IPv4
                // address.
                //
                for (uint32_t j = 0; j < m_liveInterfaces.size(); ++j) {
                    if (m_liveInterfaces[i].m_sockFd == -1 ||
                        m_liveInterfaces[j].m_interfaceName != m_liveInterfaces[i].m_interfaceName) {
                        continue;
                    }
                    if (haveIPv4Address == false && m_liveInterfaces[j].m_address.IsIPv4()) {
                        QCC_DbgPrintf(("IpNameServiceImpl::SendOutboundMessages(): Interface %d. has IPv4 counterpart %d.", i, j));
                        haveIPv4Address = true;
                        ipv4Address = m_liveInterfaces[j].m_address;
                        break;
                    }

                    if (haveIPv6Address == false && m_liveInterfaces[j].m_address.IsIPv6()) {
                        QCC_DbgPrintf(("IpNameServiceImpl::SendOutboundMessages(): Interface %d. has IPv6 counterpart %d.", i, j));
                        haveIPv6Address = true;
                        ipv6Address = m_liveInterfaces[j].m_address;
                        break;
                    }
                }

                //
                // At this point, we are ready to multicast out an interface and
                // we know our IPv4 and IPv6 addresses if they exist.  Now, we
                // have to walk the list of answer messages and rewrite the
                // provided addresses.
                //
                for (uint8_t j = 0; j < header.GetNumberAnswers(); ++j) {
                    QCC_DbgPrintf(("IpNameServiceImpl::SendOutboundMessages(): Rewrite answer %d.", j));

                    IsAt* isAt;
                    header.GetAnswer(j, &isAt);

                    //
                    // Exactly what we need to set depends on the version of the
                    // message.  First, check for version zero in the header and
                    // if we have one of those, do the version-zero specific
                    // changes.
                    //
                    uint32_t nsVersion, msgVersion;
                    header.GetVersion(nsVersion, msgVersion);
                    if (msgVersion == 0) {
                        QCC_DbgPrintf(("IpNameServiceImpl::SendOutboundMessages(): Answer %d. gets version zero", j));

                        //
                        // We're modifying the answsers in-place so clear any
                        // state we might have added on the last iteration.
                        //
                        isAt->SetVersion(0, 0);
                        isAt->SetTcpFlag(true);
                        isAt->SetUdpFlag(true);
                        isAt->ClearIPv4();
                        isAt->ClearIPv6();

                        //
                        // For version zero, the name service was an integral
                        // part of the TCP transport.  Because of this, we
                        // know implicitly that the only kind of address
                        // supported is the reliable IPv4 address.  This means
                        // we just need to set the IPv4 address of the packet
                        // and we're done.
                        //
                        if (haveIPv4Address) {
                            isAt->SetIPv4(ipv4Address.ToString());
                        }
                    }

                    //
                    // Check for version one in the header and if we have one of
                    // those, do the version-one specific changes.
                    //
                    header.GetVersion(nsVersion, msgVersion);
                    if (msgVersion == 1) {
                        QCC_DbgPrintf(("IpNameServiceImpl::SendOutboundMessages(): Answer %d. gets version one", j));

                        //
                        // We're modifying the answsers in-place so clear any
                        // state we might have added on the last iteration.
                        //
                        isAt->SetVersion(1, 1);

                        //
                        // XXX This is bogus.
                        // How do we really keep track of this?
                        //
                        isAt->SetTransportMask(TRANSPORT_TCP);
                        isAt->ClearReliableIPv4();
                        isAt->ClearUnreliableIPv4();
                        isAt->ClearReliableIPv6();
                        isAt->ClearUnreliableIPv6();

                        //
                        // Now we can write the various addresses into the
                        // packet if they are called for.
                        //
                        if (haveIPv4Address && m_reliableIPv4Port) {
                            isAt->SetReliableIPv4(ipv4Address.ToString(), m_reliableIPv4Port);
                        }
                        if (haveIPv4Address && m_unreliableIPv4Port) {
                            isAt->SetUnreliableIPv4(ipv4Address.ToString(), m_unreliableIPv4Port);
                        }
                        if (m_reliableIPv6Port) {
                            isAt->SetReliableIPv6(ipv6Address.ToString(), m_reliableIPv6Port);
                        }
                        if (m_unreliableIPv6Port) {
                            isAt->SetUnreliableIPv6(ipv6Address.ToString(), m_unreliableIPv6Port);
                        }
                    }
                }

                //
                // At this point, we have ignored the questions (who-has) on the
                // header since they stay the same, and we have rewritten the
                // answers (is-at) on the header according to the version.  Now
                // we can the modified message on out the current interface.
                //
                QCC_DbgPrintf(("IpNameServiceImpl::SendOutboundMessages(): SendProtocolMessageg()"));
                SendProtocolMessage(m_liveInterfaces[i].m_sockFd, ipv6Address, interfaceAddressPrefixLen, flags,
                                    interfaceIsIPv4, header);
            }
        }

        //
        // The current message has been sent to all of the live interfaces, so
        // we can discard it and loop back for another.
        //
        m_outbound.pop_front();
    }
}

void* IpNameServiceImpl::Run(void* arg)
{
    QCC_DbgPrintf(("IpNameServiceImpl::Run()"));

    //
    // This method is executed by the name service main thread and becomes the
    // center of the name service universe.  All incoming and outgoing messages
    // percolate through this thread because of the way we have to deal with
    // interfaces coming up and going down underneath us in a mobile
    // environment.  See the "Long Sidebar" comment above for some details on
    // the pain this has caused.
    //
    // Ultimately, this means we have a number of sockets open that correspond
    // to the "live" interfaces we are listening to.  We have to listen to all
    // of these sockets in what amounts to a select() below.  That means we
    // have live FDs waiting in the select.  On the other hand, we want to be
    // responsive in the case of a user turning on wireless and immediately
    // doing a Locate().  This requirement implies that we need to update the
    // interface state whenever we do a Locate.  This Locate() will be done in
    // the context of a user thread.  So we have a requirement that we avoid
    // changing the state of the FDs in another thread and the requirement
    // that we change the state of the FDs when the user wants to Locate().
    // Either we play synchronization games and distribute our logic or do
    // everything here.  Because it is easier to manage the process in one
    // place, we have all messages gonig through this thread.
    //
    size_t bufsize = NS_MESSAGE_MAX;
    uint8_t* buffer = new uint8_t[bufsize];

    //
    // Instantiate an event that fires after one second, and once per second
    // thereafter.  Used to drive protocol maintenance functions, especially
    // dealing with interface state changes.
    //
    const uint32_t MS_PER_SEC = 1000;
    qcc::Event timerEvent(MS_PER_SEC, MS_PER_SEC);

    qcc::Timespec tNow, tLastLazyUpdate;
    GetTimeNow(&tLastLazyUpdate);

    while (m_state == IMPL_RUNNING || m_terminal) {
        //
        // If we are shutting down, we need to make sure that we send out the
        // terminal is-at messages that correspond to a CancelAdvertiseName for
        // any of the names we are advertising.  These messages are queued while
        // handling the thread stop event (below) and m_terminal is set to true.
        // The first time through the loop in which we find the m_outbound list
        // empty it means that all of the terminal messages have been sent and
        // we can exit.  So if we find m_terminal true and m_outbound.empty()
        // true, we break out of the loop and exit.
        //
        if (m_terminal && m_outbound.empty()) {
            QCC_DbgPrintf(("IpNameServiceImpl::Run(): m_terminal && m_outbound.empty() -> m_terminal = false"));
            m_terminal = false;
            break;
        }

        GetTimeNow(&tNow);
        m_mutex.Lock();

        //
        // In order to pass the Android Compatibility Test, we need to be able
        // to enable and disable communication with the outside world.  Enabling
        // is straightforward enough, but when we disable, we need to be careful
        // about turning things off before we've sent out all possibly queued
        // packets.
        //
        if (m_doEnable) {
            m_enabled = true;
            m_doEnable = false;
        }

        if (m_doDisable && m_outbound.empty()) {
            QCC_DbgPrintf(("IpNameServiceImpl::Run(): m_doDisable && m_outbound.empty() -> m_enabled = false"));
            m_enabled = false;
            m_doDisable = false;
        }

        //
        // We need to figure out which interfaces we can send and receive
        // protocol messages over.  On one hand, we don't want to get carried
        // away with multicast group joins and leaves since we could get tangled
        // up in IGMP rate limits.  On the other hand we want to do this often
        // enough to appear responsive to the user when she moves into proximity
        // with another device.
        //
        // Some quick measurements indicate that a Linux box can take about 15
        // seconds to associate, Windows and Android about 5 seconds.  Based on
        // the lower limits, it won't do much good to lazy update faster than
        // about once every five seconds; so we take that as an upper limit on
        // how often we allow a lazy update.  On the other hand, we want to
        // make sure we do a lazy update at least every 15 seconds.  We define
        // a couple of constants, LAZY_UPDATE_{MAX,MIN}_INTERVAL to allow this
        // range.
        //
        // What drives the middle ground between MAX and MIN timing?  The
        // presence or absence of Locate() and Advertise() calls.  If the
        // application is poked by an impatient user who "knows" she should be
        // able to connect, she may arrange to send out a Locate() or
        // Advertise().  This is indicated to us by a message on the m_outbound
        // queue.
        //
        // So there are three basic cases which cause us to rn the lazy updater:
        //
        //     1) If m_forceLazyUpdate is true, some major configuration change
        //        has happened and we need to update no matter what.
        //
        //     2) If a message is found on the outbound queue, we need to do a
        //        lazy update if LAZY_UPDATE_MIN_INTERVAL has passed since the
        //        last update.
        //
        //     3) If LAZY_UPDATE_MAX_INTERVAL has elapsed since the last lazy
        //        update, we need to update.
        //
        if (m_forceLazyUpdate ||
            (m_outbound.size() && tLastLazyUpdate + qcc::Timespec(LAZY_UPDATE_MIN_INTERVAL * MS_PER_SEC) < tNow) ||
            (tLastLazyUpdate + qcc::Timespec(LAZY_UPDATE_MAX_INTERVAL * MS_PER_SEC) < tNow)) {

            QCC_DbgPrintf(("IpNameServiceImpl::Run(): LazyUpdateInterfaces()"));
            LazyUpdateInterfaces();
            tLastLazyUpdate = tNow;
            m_forceLazyUpdate = false;
        }

        SendOutboundMessages();

        //
        // We've emptied the outbound messages, so we're done if we are shutting
        // down.  The thread stop event was set in IpNameServiceImpl::Stop(),
        // and we discovered the event was set in the last iteration of the loop
        // (below).  We also reset that event in the last iteration so we must
        // not wait again, or we will end up waiting for another stop event that
        // will likely never come.  We took the time to send out a final
        // advertisement(s) above, indicating that we are going away so we can
        // just loop back to the start (where we exit the Run() loop in one
        // convenient place).
        //
        if (IsStopping()) {
            QCC_DbgPrintf(("IpNameServiceImpl::Run(): Stopping.  ClearLiveInterfaces() and break"));
            QCC_DbgPrintf(("IpNameServiceImpl::Run(): Giving mutex"));
            m_mutex.Unlock();
            ClearLiveInterfaces();
            break;
        }

        //
        // Now, worry about what to do next.  Create a set of events to wait on.
        // We always wait on the stop event, the timer event and the event used
        // to signal us when an outging message is queued or a forced wakeup for
        // a lazy update is done.
        //
        vector<qcc::Event*> checkEvents, signaledEvents;
        checkEvents.push_back(&stopEvent);
        checkEvents.push_back(&timerEvent);
        checkEvents.push_back(&m_wakeEvent);

        //
        // We also need to wait on events from all of the sockets that
        // correspond to the "live" interfaces we need to listen for inbound
        // multicast messages on.
        //
        for (uint32_t i = 0; i < m_liveInterfaces.size(); ++i) {
            if (m_liveInterfaces[i].m_sockFd != -1) {
                checkEvents.push_back(m_liveInterfaces[i].m_event);
            }
        }

        //
        // We are going to go to sleep for possibly as long as a second, so
        // we definitely need to release other (user) threads that might
        // be waiting to talk to us.
        //
        m_mutex.Unlock();

        //
        // Wait for something to happen.  if we get an error, there's not
        // much we can do about it but bail.
        //
        QStatus status = qcc::Event::Wait(checkEvents, signaledEvents);
        if (status != ER_OK && status != ER_TIMEOUT) {
            QCC_LogError(status, ("IpNameServiceImpl::Run(): Event::Wait(): Failed"));
            break;
        }

        //
        // Loop over the events for which we expect something has happened
        //
        for (vector<qcc::Event*>::iterator i = signaledEvents.begin(); i != signaledEvents.end(); ++i) {
            if (*i == &stopEvent) {
                QCC_DbgPrintf(("IpNameServiceImpl::Run(): Stop event fired"));

                //
                // We heard the stop event, so reset it.  Our contract is that once
                // we've heard this event, we have to exit the run routine fairly
                // quickly.  We can take some time to clean up, but there will be
                // someone else eventually blocked waiting for us to exit, so we
                // can't get carried away.
                //
                stopEvent.ResetEvent();

                //
                // What we need to do is to send out is-at messages telling
                // anyone interested in our names that they are no longer valid.
                // This is a fairly complicated process that can involve sending
                // multiple packets out multiple interfaces, so we clearly don't
                // want to duplicate code here to make it all happen.  We use a
                // special case of normal operation to prevent new requests from
                // being queued, issue our own terminal requests corresponding to
                // the is-at messages metioned above, and then we run until they
                // are all processed and then we exit.
                //
                // Calling Retransmit(true) will queue the desired terminal
                // is-at messages on the m_outbound list.  To ensure that they
                // are sent before we exit, we set m_termianl to true.  We will
                // have set m_state to IMPL_STOPPING in IpNameServiceImpl::Stop.  This
                // stops new external requests from being acted upon.  We then
                // continue in our loop until the outbound queue is empty and
                // then exit the run routine (above).
                //
                Retransmit(true);
                m_terminal = true;
                break;
            } else if (*i == &timerEvent) {
                // QCC_DbgPrintf(("IpNameServiceImpl::Run(): Timer event fired"));
                //
                // This is an event that fires every second to give us a chance
                // to do any protocol maintenance, like retransmitting queued
                // advertisements.
                //
                DoPeriodicMaintenance();
            } else if (*i == &m_wakeEvent) {
                QCC_DbgPrintf(("IpNameServiceImpl::Run(): Wake event fired"));
                //
                // This is an event that fires whenever a message has been
                // queued on the outbound name service message queue.  We
                // always check the queue whenever we run through the loop,
                // (it'll happen before we sleep again) but we do have to reset
                // it.
                //
                m_wakeEvent.ResetEvent();
            } else {
                QCC_DbgPrintf(("IpNameServiceImpl::Run(): Socket event fired"));
                //
                // This must be activity on one of our multicast listener sockets.
                //
                qcc::SocketFd sockFd = (*i)->GetFD();

                QCC_DbgPrintf(("IpNameServiceImpl::Run(): Call qcc::RecvFrom()"));

                qcc::IPAddress address;
                uint16_t port;
                size_t nbytes;

                QStatus status = qcc::RecvFrom(sockFd, address, port, buffer, bufsize, nbytes);
                if (status != ER_OK) {
                    //
                    // We have a RecvFrom error.  We want to avoid states where
                    // we get repeated read errors and just end up in an
                    // infinite loop getting errors sucking up all available
                    // CPU, so we make sure we sleep for at least a short time
                    // after detecting the error.
                    //
                    // Our basic strategy is to hope that this is a transient
                    // error, or one that will be recovered at the next lazy
                    // update.  We don't want to blindly force a lazy update
                    // or we may get into an infinite lazy update loop, so
                    // the worst that can happen is that we introduce a short
                    // delay here in our handler whenever we detect an error.
                    //
                    // On Windows ER_WOULBLOCK can be expected because it takes
                    // an initial call to recv to determine if the socket is readable.
                    //
                    if (status != ER_WOULDBLOCK) {
                        QCC_LogError(status, ("IpNameServiceImpl::Run(): qcc::RecvFrom(%d, ...): Failed", sockFd));
                        qcc::Sleep(1);
                    }
                    continue;
                }

                //
                // We got a message over the multicast channel.  Deal with it.
                //
                HandleProtocolMessage(buffer, nbytes, address);
            }
        }
    }

    delete [] buffer;
    return 0;
}

void IpNameServiceImpl::Retry(void)
{
    static uint32_t tick = 0;

    //
    // tick holds 136 years of ticks at one per second, so we don't worry about
    // rolling over.
    //
    ++tick;

    //
    // use Meyers' idiom to keep iterators sane.
    //
    for (list<Header>::iterator i = m_retry.begin(); (m_state == IMPL_RUNNING) && (i != m_retry.end());) {
        uint32_t retryTick = (*i).GetRetryTick();

        //
        // If this is the first time we've seen this entry, set the first
        // retry time.
        //
        if (retryTick == 0) {
            (*i).SetRetryTick(tick + RETRY_INTERVAL);
            ++i;
            continue;
        }

        if (tick >= retryTick) {
            //
            // Send the message out over the multicast link (again).
            //
            QueueProtocolMessage(*i);

            uint32_t count = (*i).GetRetries();
            ++count;

            if (count == m_retries) {
                m_retry.erase(i++);
            } else {
                (*i).SetRetries(count);
                (*i).SetRetryTick(tick + RETRY_INTERVAL);
                ++i;
            }
        } else {
            ++i;
        }
    }
}

void IpNameServiceImpl::Retransmit(bool exiting)
{
    QCC_DbgPrintf(("IpNameServiceImpl::Retransmit()"));

    //
    // There are at least two threads wandering through the advertised list.
    // We are running short on toes, so don't shoot any more off by not being
    // thread-unaware.
    //
    QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Taking lock"));
    m_mutex.Lock();

    //
    // We are now at version one of the protocol.  There is a significant
    // difference between version zero and version one messages, so down-version
    // (version zero) clients will not know what to do with versino one
    // messages.  This means that if we want to have clients running older
    // daemons be able to hear our advertisements, we need to send both flavors
    // of message.  Since the version is located in the message header, this
    // means two messages.
    //
    // Do it once for version zero.
    //
    {
        //
        // Keep track of how many messages we actually send in order to get all of
        // the advertisements out.
        //
        uint32_t nSent = 0;

        //
        // The header will tie the whole protocol message together.  By setting the
        // timer, we are asking for everyone who hears the message to remember the
        // advertisements for that number of seconds.  If we are exiting, then we
        // set the timer to zero, which means that the name is no longer valid.
        //
        Header header;

        //
        // We understand all messages from version zero to version one, and we
        // are sending a version zero message.  The whole point of sending a
        // version zero message is that can be understood by down-level code
        // so we can't use the new versioning scheme.  We have to use some
        // sneaky way to tell an in-the know version one client that the
        // packet is from a version one client and that is through the setting
        // of the UDP flag.
        //
        header.SetVersion(0, 0);

        header.SetTimer(exiting ? 0 : m_tDuration);

        //
        // The underlying protocol is capable of identifying both TCP and UDP
        // services.  Right now, the only possibility is TCP.  See the comment in
        // AdvertiseName for why we set the UDP flag.
        //
        IsAt isAt;
        isAt.SetCompleteFlag(false);
        isAt.SetTcpFlag(true);
        isAt.SetUdpFlag(true);
        isAt.SetGuid(m_guid);
        isAt.SetPort(m_port);

        QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Loop through advertised names"));

        //
        // Loop through the list of names we are advertising, constructing as many
        // protocol messages as it takes to get our list of advertisements out.
        //
        // Note that the number of packets that can go out in any given amount of
        // time is effectively throttled in SendProtocolMessage() by a random delay.
        // A user can consume all available resources here by flooding us with
        // advertisements but she will only be shooting herself in the foot.
        //
        for (list<qcc::String>::iterator i = m_advertised.begin(); i != m_advertised.end(); ++i) {
            QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Accumulating \"%s\"", (*i).c_str()));

            //
            // It is possible that we have accumulated more advertisements than will
            // fit in a UDP IpNameServiceImpl packet.  A name service is-at message is going
            // to consist of a header and its answer section, which is made from an
            // IsAt object.  We first ask both of these objects to return their size
            // so we know how much space is committed already.  Note that we ask the
            // header for its max possible size since the header may be modified to
            // add actual IPv4 and IPv6 addresses when it is sent.
            //
            size_t currentSize = header.GetSerializedSize() + isAt.GetSerializedSize();

            //
            // This isn't terribly elegant, but we don't know the IP address(es)
            // over which the message will be sent.  These are added in the loop
            // that actually does the packet sends, with the interface addresses
            // dynamically added onto the message.  We have no clue here if an IPv4
            // or IPv6 or both flavors of address will exist on a given interface,
            // nor how many interfaces there are.  All we can do here is to assume
            // the worst case for the size (both exist) and add the 20 bytes (four
            // for IPv4, sixteen for IPv6) that the addresses may consume in the
            // final packet.
            //
            currentSize += 20;

            //
            // We cheat a little in order to avoid a string copy and use our
            // knowledge that names are stored as a byte count followed by the
            // string bytes.  If the current name won't fit into the currently
            // assembled message, we need to flush the current message and start
            // again.
            //
            if (currentSize + 1 + (*i).size() > NS_MESSAGE_MAX) {
                QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Message is full"));
                //
                // The current message cannot hold another name.  We need to send it
                // out before continuing.
                //
                QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Sending partial list"));
                header.AddAnswer(isAt);
                QueueProtocolMessage(header);
                ++nSent;

                //
                // The full message is now on the way out.  Now, we remove all of
                // the entries in the IsAt object, reset the header, which clears
                // out the existing is-at, and start accumulating new names again.
                //
                QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Resetting current list"));
                header.Reset();
                isAt.Reset();
                isAt.AddName(*i);
            } else {
                QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Message has room.  Adding \"%s\"", (*i).c_str()));
                isAt.AddName(*i);
            }
        }

        //
        // We most likely have a partially full message waiting to go out.  If we
        // haven't sent a message, then the one message holds all of the names that
        // are being advertised.  In this case, we set the complete flag to indicate
        // that this packet describes the full extent of advertised well known
        // names.
        //
        if (nSent == 0) {
            QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Single complete message "));
            isAt.SetCompleteFlag(true);
        }

        QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Sending final message "));
        header.AddAnswer(isAt);
        QueueProtocolMessage(header);
    }

    //
    // Do it once for version one.
    //
    {
        //
        // Keep track of how many messages we actually send in order to get all of
        // the advertisements out.
        //
        uint32_t nSent = 0;

        //
        // The header will tie the whole protocol message together.  By setting the
        // timer, we are asking for everyone who hears the message to remember the
        // advertisements for that number of seconds.  If we are exiting, then we
        // set the timer to zero, which means that the name is no longer valid.
        //
        Header header;

        //
        // We understand all messages from version zero to version one, and we
        // are sending a version one message;
        //
        header.SetVersion(1, 1);

        header.SetTimer(exiting ? 0 : m_tDuration);

        //
        // The underlying protocol is capable of identifying both TCP and UDP
        // services.  Right now, the only possibility is TCP.
        //
        IsAt isAt;

        //
        // We understand all messages from version zero to version one, and we
        // are sending a version one message;
        //
        isAt.SetVersion(1, 1);

        isAt.SetCompleteFlag(false);

        //
        // Version one allows us to provide four possible endpoints.
        // XXX FIXME These should be stored per-transport mask (16 possible).
        //
        if (m_reliableIPv4Port) {
            isAt.SetReliableIPv4(m_reliableIPv4Address, m_reliableIPv4Port);
        }
        if (m_unreliableIPv4Port) {
            isAt.SetUnreliableIPv4(m_unreliableIPv4Address, m_unreliableIPv4Port);
        }
        if (m_reliableIPv6Port) {
            isAt.SetReliableIPv6(m_reliableIPv6Address, m_reliableIPv6Port);
        }
        if (m_unreliableIPv6Port) {
            isAt.SetUnreliableIPv6(m_unreliableIPv6Address, m_unreliableIPv6Port);
        }

        isAt.SetGuid(m_guid);

        QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Loop through advertised names"));

        //
        // Loop through the list of names we are advertising, constructing as many
        // protocol messages as it takes to get our list of advertisements out.
        //
        // Note that the number of packets that can go out in any given amount of
        // time is effectively throttled in SendProtocolMessage() by a random delay.
        // A user can consume all available resources here by flooding us with
        // advertisements but she will only be shooting herself in the foot.
        //
        for (list<qcc::String>::iterator i = m_advertised.begin(); i != m_advertised.end(); ++i) {
            QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Accumulating \"%s\"", (*i).c_str()));

            //
            // It is possible that we have accumulated more advertisements than will
            // fit in a UDP IpNameServiceImpl packet.  A name service is-at message is going
            // to consist of a header and its answer section, which is made from an
            // IsAt object.  We first ask both of these objects to return their size
            // so we know how much space is committed already.  Note that we ask the
            // header for its max possible size since the header may be modified to
            // add actual IPv4 and IPv6 addresses when it is sent.
            //
            size_t currentSize = header.GetSerializedSize() + isAt.GetSerializedSize();

            //
            // This isn't terribly elegant, but we don't know the IP address(es)
            // over which the message will be sent.  These are added in the loop
            // that actually does the packet sends, with the interface addresses
            // dynamically added onto the message.  We have no clue here if an IPv4
            // or IPv6 or both flavors of address will exist on a given interface,
            // nor how many interfaces there are.  All we can do here is to assume
            // the worst case for the size (both exist) and add the 20 bytes (four
            // for IPv4, sixteen for IPv6) that the addresses may consume in the
            // final packet.
            //
            currentSize += 20;

            //
            // We cheat a little in order to avoid a string copy and use our
            // knowledge that names are stored as a byte count followed by the
            // string bytes.  If the current name won't fit into the currently
            // assembled message, we need to flush the current message and start
            // again.
            //
            if (currentSize + 1 + (*i).size() > NS_MESSAGE_MAX) {
                QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Message is full"));
                //
                // The current message cannot hold another name.  We need to send it
                // out before continuing.
                //
                QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Sending partial list"));
                header.AddAnswer(isAt);
                QueueProtocolMessage(header);
                ++nSent;

                //
                // The full message is now on the way out.  Now, we remove all of
                // the entries in the IsAt object, reset the header, which clears
                // out the existing is-at, and start accumulating new names again.
                //
                QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Resetting current list"));
                header.Reset();
                isAt.Reset();
                isAt.AddName(*i);
            } else {
                QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Message has room.  Adding \"%s\"", (*i).c_str()));
                isAt.AddName(*i);
            }
        }

        //
        // We most likely have a partially full message waiting to go out.  If we
        // haven't sent a message, then the one message holds all of the names that
        // are being advertised.  In this case, we set the complete flag to indicate
        // that this packet describes the full extent of advertised well known
        // names.
        //
        if (nSent == 0) {
            QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Single complete message "));
            isAt.SetCompleteFlag(true);
        }

        QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Sending final message "));
        header.AddAnswer(isAt);
        QueueProtocolMessage(header);
    }

    QCC_DbgPrintf(("IpNameServiceImpl::Retransmit(): Giving lock"));
    m_mutex.Unlock();
}

void IpNameServiceImpl::DoPeriodicMaintenance(void)
{
#if HAPPY_WANDERER
    Wander();
#endif
    m_mutex.Lock();

    //
    // Retry all Locate requests to ensure that those requests actually make
    // it out on the wire.
    //
    Retry();

    //
    // If we have something exported, we will have a retransmit timer value
    // set.  If not, this value will be zero and there's nothing to be done.
    //
    if (m_timer) {
        --m_timer;
        if (m_timer == m_tRetransmit) {
            QCC_DbgPrintf(("IpNameServiceImpl::DoPeriodicMaintenance(): Retransmit()"));
            Retransmit(false);
            m_timer = m_tDuration;
        }
    }

    m_mutex.Unlock();
}

void IpNameServiceImpl::HandleProtocolQuestion(WhoHas whoHas, qcc::IPAddress address)
{
    QCC_DbgHLPrintf(("IpNameServiceImpl::HandleProtocolQuestion()"));

    //
    // There are at least two threads wandering through the advertised list.
    // We are running short on toes, so don't shoot any more off by not being
    // thread-unaware.
    //
    QCC_DbgHLPrintf(("IpNameServiceImpl::HandleProtocolQuestion(): Taking lock"));
    m_mutex.Lock();

    //
    // Loop through the names we are being asked about, and if we have
    // advertised any of them, we are going to need to respond to this
    // question.
    //
    bool respond = false;
    for (uint32_t i = 0; i < whoHas.GetNumberNames(); ++i) {
        qcc::String wkn = whoHas.GetName(i);

        //
        // Zero length strings are unmatchable.  If you want to do a wildcard
        // match, you've got to send a wildcard character.
        //
        if (wkn.size() == 0) {
            continue;
        }

        //
        // check to see if this name on the list of names we advertise.
        //
        for (list<qcc::String>::iterator j = m_advertised.begin(); j != m_advertised.end(); ++j) {

            //
            // The requested name comes in from the WhoHas message and we
            // allow wildcards there.
            //
            if (IpNameServiceImplWildcardMatch((*j), wkn)) {
                QCC_DbgHLPrintf(("IpNameServiceImpl::HandleProtocolQuestion(): request for %s does not match my %s",
                                 wkn.c_str(), (*j).c_str()));
                continue;
            } else {
                respond = true;
                break;
            }
        }

        //
        // If we find a match, don't bother going any further since we need
        // to respond in any case.
        //
        if (respond) {
            break;
        }
    }

    QCC_DbgHLPrintf(("IpNameServiceImpl::HandleProtocolQuestion(): Giving lock"));
    m_mutex.Unlock();

    //
    // Since any response we send must include all of the advertisements we
    // are exporting; this just means to retransmit all of our advertisements.
    //
    if (respond) {
        Retransmit(false);
    }
}

void IpNameServiceImpl::HandleProtocolAnswer(IsAt isAt, uint32_t timer, qcc::IPAddress address)
{
    QCC_DbgHLPrintf(("IpNameServiceImpl::HandleProtocolAnswer()"));

    //
    // If there are no callbacks we can't tell the user anything about what is
    // going on the net, so it's pointless to go any further.
    //
    if (m_callback == 0) {
        QCC_DbgPrintf(("IpNameServiceImpl::HandleProtocolAnswer(): No callback, so nothing to do"));
        return;
    }

    //
    // For version zero messages from version one transports, we need to
    // disregard the name service messages sent out in compatibility mode
    // (version zero messages).  We know that a version one name service will be
    // following up with a version one packet, so a version zero compatibility
    // message provides incomplete information -- we drop such messages here.
    //
    uint32_t nsVersion, msgVersion;
    isAt.GetVersion(nsVersion, msgVersion);
    if (nsVersion == 0 && msgVersion == 0) {
        if (isAt.GetUdpFlag()) {
            QCC_DbgPrintf(("IpNameServiceImpl::HandleProtocolAnswer(): Ignoring version zero message from version one peer"));
            return;
        }
    }

    vector<qcc::String> wkn;

    for (uint8_t i = 0; i < isAt.GetNumberNames(); ++i) {
        QCC_DbgPrintf(("IpNameServiceImpl::HandleProtocolAnswer(): Got well-known name %s", isAt.GetName(i).c_str()));
        wkn.push_back(isAt.GetName(i));
    }

    //
    // Life is easier if we keep these things sorted.  Don't rely on the source
    // (even though it is really us) to do so.
    //
    sort(wkn.begin(), wkn.end());

    qcc::String guid = isAt.GetGuid();
    QCC_DbgPrintf(("IpNameServiceImpl::HandleProtocolAnswer(): Got GUID %s", guid.c_str()));

    //
    // How we infer addresses is different between version zero of the protocol
    // and version one.  In version zero, if there are no IP addresses present
    // in the received message, we take the IP address found in the received
    // packet.  This allowed us to optimize out the address in some cases.  We
    // do not do this in version one messages.  The advertised addresses must
    // always be present in the message.
    //
    isAt.GetVersion(nsVersion, msgVersion);
    if (msgVersion == 0) {
        //
        // We always get an address from the system since we got the message
        // over a call to recvfrom().  This will either be an IPv4 or an IPv6
        // address in the case of legacy daemons or only from IPv6 in new
        // daemons.  We can also get an IPv4 or an IPv6 address in the protocol.
        // So we have from one to three addresses of possibly different flavors
        // that we need to communicate back to the daemon.  We have to be very
        // careful to play by the old rules when appropriate to make sure we
        // have backward compatibility.
        //
        // Note that there is no such thing as a TCP transport that is capable
        // of listening on an IPv6 address, so we filter those out here.
        //
        // It is convenient for the daemon to get these addresses in the form of
        // a "listen-spec".  This is a string starting with the transport name,
        // followed by private (to the transport) name=value pairs.  In version
        // zero of the protocol, there was only one possible transport that used
        // the IP name service, and that was the TCP transport.  We used to be
        // integrated into the TCP transport, so, for us here and now these
        // listen specs look like, "tcp:r4addr=x,r4port=y".  The daemon is going
        // to keep track of unique instances of these and must be able to handle
        // multiple identical reports since we will be getting keepalives.  What
        // we need to do then is to send a callback with a listen-spec for every
        // address we find.  If we get all three addresses, we'll do three
        // callbacks with different listen-specs.  This completely changes in
        // version one, BTW.
        //
        qcc::String recvfromAddress, ipv4address, ipv6address;

        recvfromAddress = address.ToString();
        QCC_DbgPrintf(("IpNameServiceImpl::HandleProtocolAnswer(): Got IP %s from recvfrom", recvfromAddress.c_str()));

        if (isAt.GetIPv4Flag()) {
            ipv4address = isAt.GetIPv4();
            QCC_DbgPrintf(("IpNameServiceImpl::HandleProtocolAnswer(): Got IPv4 %s from message", ipv4address.c_str()));
        }

        if (isAt.GetIPv6Flag()) {
            ipv6address = isAt.GetIPv6();
            QCC_DbgPrintf(("IpNameServiceImpl::HandleProtocolAnswer(): Got IPv6 %s from message", ipv6address.c_str()));
        }

        uint16_t port = isAt.GetPort();
        QCC_DbgPrintf(("IpNameServiceImpl::HandleProtocolAnswer(): Got port %d from message", port));

        //
        //
        // Since version zero had no transport mask, the only transport that can
        // provide a version zero message is tcp.  So, the longest bus address
        // we can generate is going to be the larger of an IPv4 or IPv6 address:
        //
        // "addr=255.255.255.255,port=65535"
        // "addr=ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff,port=65535"
        //
        // or 56 characters long including the trailing '\0'
        //
        char addrbuf[64];

        //
        // Call back with the address we got via recvfrom unless it is
        // overridden by the address in the message. An ipv4 address in the
        // message overrides an ipv4 recvfrom address, an ipv6 address in the
        // message overrides an ipv6 recvfrom address.
        //
        // Note that we no longer prepend the transport name ("tcp:") since we
        // got broken out of the TCP transport.  We expect the transport to do
        // that now.
        //
        if ((address.IsIPv4() && !ipv4address.size())) {
            snprintf(addrbuf, sizeof(addrbuf), "r4addr=%s,r4port=%d", recvfromAddress.c_str(), port);
            QCC_DbgPrintf(("IpNameServiceImpl::HandleProtocolAnswer(): Calling back with %s", addrbuf));
            qcc::String busAddress(addrbuf);

            if (m_callback) {
                (*m_callback)(busAddress, guid, wkn, timer);
            }
        }

        //
        // If we received an IPv4 address in the message, call back with that
        // one.
        //
        if (ipv4address.size()) {
            snprintf(addrbuf, sizeof(addrbuf), "r4addr=%s,r4port=%d", ipv4address.c_str(), port);
            QCC_DbgPrintf(("IpNameServiceImpl::HandleProtocolAnswer(): Calling back with %s", addrbuf));
            qcc::String busAddress(addrbuf);

            if (m_callback) {
                (*m_callback)(busAddress, guid, wkn, timer);
            }
        }

        //
        // If we received an IPv6 address in the message, call back with that
        // one.
        //
        if (ipv6address.size()) {
            snprintf(addrbuf, sizeof(addrbuf), "r6addr=%s,r6port=%d", ipv6address.c_str(), port);
            QCC_DbgHLPrintf(("IpNameServiceImpl::HandleProtocolAnswer(): Calling back with %s", addrbuf));
            qcc::String busAddress(addrbuf);

            if (m_callback) {
                (*m_callback)(busAddress, guid, wkn, timer);
            }
        }
    } else if (msgVersion == 1) {
        //
        // In the version one protocol, the maximum size static buffer for the
        // longest bus address we can generate corresponds to two fully occupied
        // IPv4 addresses and two fully occupied IPV6 addresses.  So, we figure
        // that we need 2 X 35 == 70 bytes for the IPv4 endpoint information,
        // 2 X 59 == 118 bytes for the IPv6 endpoint information and three extra
        // commas:
        //
        //     " r4addr=192.168.100.101,r4port=65535,"
        //     "u4ddr=192.168.100.101,u4port=65535,"
        //     "r6addr=ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff,r6port=65535,"
        //     "u6addr=ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff,u6port=65535"
        //
        // Adding a byte for the trailing '\0' we come up with 192 bytes of bus
        // address. C++ purists will object to using the C stdio routines but
        // they are simpler and faster since there are no memory allocations or
        // reallocations.
        //
        // Note that we do not prepend the bus address with the transport name,
        // i.e. "tcp:" since we assume that the transport knows its own name.
        //
        char addrbuf[192];
        addrbuf[0] = '\0';

        char addr4buf[36];
        char addr6buf[60];

        bool needComma = false;

        if (isAt.GetReliableIPv4Flag()) {
            snprintf(addrbuf, sizeof(addrbuf), "r4addr=%s,r4port=%d",
                     isAt.GetReliableIPv4Address().c_str(), isAt.GetReliableIPv4Port());

            needComma = true;
        }

        if (isAt.GetUnreliableIPv4Flag()) {
            snprintf(addr4buf, sizeof(addr4buf), ",u4addr=%s,u4port=%d",
                     isAt.GetUnreliableIPv4Address().c_str(), isAt.GetUnreliableIPv4Port());
            //
            // Okay, we carefully calculated all of our buffer sizes so we can
            // never blow the buffer.  Why are we using strncat?  Because
            // Klocwork will complain if we don't.
            //
            if (needComma) {
                strncat(addrbuf, &addr4buf[0], sizeof(addr4buf));
            } else {
                strncat(addrbuf, &addr4buf[1], sizeof(addr4buf));
            }

            needComma = true;
        }

        if (isAt.GetReliableIPv6Flag()) {
            snprintf(addr6buf, sizeof(addr6buf), ",r6addr=%s,r6port=%d",
                     isAt.GetReliableIPv6Address().c_str(), isAt.GetReliableIPv6Port());
            if (needComma) {
                strncat(addrbuf, &addr6buf[0], sizeof(addr6buf));
            } else {
                strncat(addrbuf, &addr6buf[1], sizeof(addr6buf));
            }

            needComma = true;
        }

        if (isAt.GetUnreliableIPv6Flag()) {
            snprintf(addr6buf, sizeof(addr6buf), ",u6addr=%s,u6port=%d",
                     isAt.GetUnreliableIPv6Address().c_str(), isAt.GetUnreliableIPv6Port());
            if (needComma) {
                strncat(addrbuf, &addr6buf[0], sizeof(addr6buf));
            } else {
                strncat(addrbuf, &addr6buf[1], sizeof(addr6buf));
            }

            needComma = true;
        }

        //
        // In version one of the protocol, we always call back with the
        // addresses we find in the message.  We don't bother with the address
        // we got in recvfrom.
        //
        qcc::String busAddress(addrbuf);

        QCC_DbgHLPrintf(("IpNameServiceImpl::HandleProtocolAnswer(): Calling back with %s", busAddress.c_str()));

        if (m_callback) {
            (*m_callback)(busAddress, guid, wkn, timer);
        }
    }
}

void IpNameServiceImpl::HandleProtocolMessage(uint8_t const* buffer, uint32_t nbytes, qcc::IPAddress address)
{
    QCC_DbgHLPrintf(("IpNameServiceImpl::HandleProtocolMessage(0x%x, %d, %s)", buffer, nbytes, address.ToString().c_str()));

#if HAPPY_WANDERER
    if (Wander() == false) {
        QCC_LogError(ER_FAIL, ("IpNameServiceImpl::HandleProtocolMessage(): Wander(): out of range"));
        return;
    } else {
        QCC_LogError(ER_FAIL, ("IpNameServiceImpl::HandleProtocolMessage(): Wander(): in range"));
    }
#endif

    Header header;
    size_t bytesRead = header.Deserialize(buffer, nbytes);
    if (bytesRead != nbytes) {
        QCC_DbgPrintf(("IpNameServiceImpl::HandleProtocolMessage(): Deserialize(): Error"));
        return;
    }

    //
    // We only understand version zero and one messages.
    //
    uint32_t nsVersion, msgVersion;
    header.GetVersion(nsVersion, msgVersion);
    if (msgVersion != 0 && msgVersion != 1) {
        QCC_DbgPrintf(("IpNameServiceImpl::HandleProtocolMessage(): Unknown version: Error"));
        return;
    }

    //
    // If the received packet contains questions, see if we can answer them.
    // We have the underlying device in loopback mode so we can get receive
    // our own questions.  We usually don't have an answer and so we don't
    // reply, but if we do have the requested names, we answer ourselves
    // to pass on this information to other interested bystanders.
    //
    for (uint8_t i = 0; i < header.GetNumberQuestions(); ++i) {
        HandleProtocolQuestion(header.GetQuestion(i), address);
    }

    //
    // If the received packet contains answers, see if they are answers to
    // questions we think are interesting.  Make sure we are not talking to
    // ourselves unless we are told to for debugging purposes
    //
    for (uint8_t i = 0; i < header.GetNumberAnswers(); ++i) {
        IsAt isAt = header.GetAnswer(i);
        //
        // The version isn't actually carried in the is-at message since that
        // would be redundant, so we have to set it from the header version
        // before passing it off.
        //
        uint32_t nsVersion, msgVersion;
        header.GetVersion(nsVersion, msgVersion);
        isAt.SetVersion(nsVersion, msgVersion);
        if (m_loopback || (isAt.GetGuid() != m_guid)) {
            HandleProtocolAnswer(isAt, header.GetTimer(), address);
        }
    }
}

QStatus IpNameServiceImpl::Start()
{
    QCC_DbgPrintf(("IpNameServiceImpl::Start()"));
    m_mutex.Lock();
    assert(IsRunning() == false);
    QCC_DbgPrintf(("IpNameServiceImpl::Start(): Starting thread"));
    QStatus status = Thread::Start(this);
    QCC_DbgPrintf(("IpNameServiceImpl::Start(): Started"));
    m_state = IMPL_RUNNING;
    m_mutex.Unlock();
    return status;
}

bool IpNameServiceImpl::Started()
{
    return IsRunning();
}

QStatus IpNameServiceImpl::Stop()
{
    QCC_DbgPrintf(("IpNameServiceImpl::Stop()"));
    m_mutex.Lock();
    if (m_state != IMPL_SHUTDOWN) {
        m_state = IMPL_STOPPING;
    }
    QCC_DbgPrintf(("IpNameServiceImpl::Stop(): Stopping thread"));
    QStatus status = Thread::Stop();
    QCC_DbgPrintf(("IpNameServiceImpl::Stop(): Stopped"));
    m_mutex.Unlock();
    return status;
}

QStatus IpNameServiceImpl::Join()
{
    QCC_DbgPrintf(("IpNameServiceImpl::Join()"));
    assert(m_state == IMPL_STOPPING || m_state == IMPL_SHUTDOWN);
    QCC_DbgPrintf(("IpNameServiceImpl::Join(): Joining thread"));
    QStatus status = Thread::Join();
    QCC_DbgPrintf(("IpNameServiceImpl::Join(): Joined"));
    m_state = IMPL_SHUTDOWN;
    return status;
}

} // namespace ajn
