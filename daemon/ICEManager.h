#ifndef _ICEMANAGER_H
#define _ICEMANAGER_H

/**
 * @file ICEManager.h
 *
 */

/******************************************************************************
 * Copyright 2009,2012 Qualcomm Innovation Center, Inc.
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

#include <list>
#include <qcc/Mutex.h>
#include "ICESession.h"
#include "ICESessionListener.h"
#include "Status.h"
#include "RendezvousServerInterface.h"

using namespace qcc;

namespace ajn {

// Forward Declaration
class ICESessionListener;

/**
 * ICEManager is an active singleton class that provides the external interface to ICE.
 * It is responsible for executing and coordinating ICE related network operations.
 */
class ICEManager {
  public:

    /** ICEManager accessor. */
    static ICEManager* GetInstance();

    /**
     * Destructor.  This will deallocate sessions and terminate
     * the keep-alive thread.
     *
     * @sa ICEManager:DeallocateSession
     */
    ~ICEManager(void);

    /**
     * Allocate a Session.
     *
     * Perform the following sequence:
     *  1) If addHostCandidates is true, add host candidates for ALL known local network
     *     interfaces.
     *  2) Allocate local network resources.
     *  3) If addRelayedCandidates is true, reserve TURN resource(s) from TURN server
     *  4) Determine server reflexive ICE candidates via STUN.
     *
     * Local network resources and TURN resource reservation(s) will remain in effect until
     * the session is deallocate.
     *
     * @param addHostCandidates     If true, all known local network adapters will be added for
     *                              allocation.
     *
     * @param addRelayedCandidates  If true, TURN server resources will be allocated .
     *
     * @param session               Handle for session.
     *
     * @param stunInfo              STUN server information
     */
    QStatus AllocateSession(bool addHostCandidates,
                            bool addRelayedCandidates,
                            ICESessionListener* listener,
                            ICESession*& session,
                            STUNServerInfo stunInfo);


    /**
     * Deallocate an ICESession.
     * Deallocate all local network resources and TURN reservations associated with a given ICESession.
     *
     * @param session   ICESession to deallocate.
     */
    QStatus DeallocateSession(ICESession*& session);

    void SetInterfaceNamePrefixes(String ethPrefix, String wifiPrefix, String mobileNwPrefix);

  private:

    list<ICESession*> sessions;     ///< List of allocated ICESessions.

    Mutex lock;                    ///< Synchronizes multiple threads

    /** Private constructor */
    ICEManager();

    /** Private copy constructor */
    ICEManager(const ICEManager&);

    /** Private assignment operator */
    ICEManager& operator=(const ICEManager&);

    /*Ethernet interface name prefix. For eg. eth */
    String ethernetInterfaceName;

    /* Wi-Fi interface name prefix. For eg. wlan */
    String wifiInterfaceName;

    /* Mobile Network interface name prefix. For eg. ppp */
    String mobileNwInterfaceName;

};

} //namespace ajn

#endif
