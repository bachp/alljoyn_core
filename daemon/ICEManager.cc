/**
 * @file IceManager.cpp
 *
 * IceManager is responsible for executing and coordinating ICE related network operations.
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
#include <qcc/Debug.h>
#include <ICESession.h>
#include <ICESessionListener.h>
#include <ICEManager.h>
#include "RendezvousServerInterface.h"

using namespace qcc;

/** @internal */
#define QCC_MODULE "ICEMANAGER"

namespace ajn {

ICEManager* ICEManager::GetInstance()
{
    static Mutex singletonLock;
    static ICEManager* singleton = 0;

    singletonLock.Lock();
    if (0 == singleton) {
        singleton = new ICEManager();

        // Feature: With current implementation, STUN request pacing is
        // handled within each Session.  It would be more consistent with
        // the spirit of ICE if pacing were a 'machine-wide' behavior instead.
        // To make such a change, here would be a good place to start the
        // thread which will perform keepalives, and periodic ICE-checks.
    }
    singletonLock.Unlock();

    return singleton;
}

ICEManager::ICEManager() :
    ethernetInterfaceName(),
    wifiInterfaceName(),
    mobileNwInterfaceName()
{
}

ICEManager::~ICEManager(void)
{
    // Reclaim memory consumed by the sessions.
    lock.Lock();
    while (!sessions.empty()) {
        ICESession* session = sessions.front();
        delete session;
        sessions.pop_front();
    }
    lock.Unlock();
}



QStatus ICEManager::AllocateSession(bool addHostCandidates,
                                    bool addRelayedCandidates,
                                    ICESessionListener* listener,
                                    ICESession*& session,
                                    STUNServerInfo stunInfo)
{
    QStatus status = ER_OK;

    session = new ICESession(addHostCandidates, addRelayedCandidates, listener, stunInfo,
                             ethernetInterfaceName, wifiInterfaceName, mobileNwInterfaceName);

    status = session->Init();

    if (ER_OK == status) {
        lock.Lock();                // Synch with another thread potentially calling destructor.
                                    // Not likely because this is a singleton, but...
        sessions.push_back(session);
        lock.Unlock();
    } else {
        QCC_LogError(status, ("session->Init"));
        delete session;
        session = NULL;
    }

    return status;
}



QStatus ICEManager::DeallocateSession(ICESession*& session)
{
    QStatus status = ER_OK;

    assert(session != NULL);

    // remove from list
    lock.Lock();                // Synch with another thread potentially calling destructor.
                                // Not likely because this is a singleton, but...
    sessions.remove(session);
    lock.Unlock();

    delete session;

    return status;
}

void ICEManager::SetInterfaceNamePrefixes(String ethPrefix, String wifiPrefix, String mobileNwPrefix)
{
    QCC_DbgPrintf(("ICEManager::SetInterfaceNamePrefixes()\n"));
    ethernetInterfaceName = ethPrefix;
    wifiInterfaceName = wifiPrefix;
    mobileNwInterfaceName = mobileNwPrefix;
}

} //namespace ajn
