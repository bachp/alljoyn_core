/**
 * @file
 * The Android Wi-Fi Direct (Wi-Fi P2P) name service implementation
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include <qcc/Debug.h>

#include "P2PNameService.h"
#include "P2PNameServiceImpl.h"

#define QCC_MODULE "P2PNS"

namespace ajn {

P2PNameService::P2PNameService()
    : m_constructed(false), m_destroyed(false), m_refCount(0), m_pimpl(NULL)
{
    QCC_DbgPrintf(("P2PNameService::P2PNameService()"));

    //
    // AllJoyn is a multithreaded system.  Since the name service instance is
    // created on first use, the first use is in the Start() method of each
    // transport wanting to use the P2PNameService.  Since the starting of all
    // of the transports happens on a single thread, we assume we are
    // single-threaded here and don't do anything fancy to prevent interleaving
    // scenarios on the private implementation constructor.
    //
    m_pimpl = new P2PNameServiceImpl;
    m_constructed = true;
}

P2PNameService::~P2PNameService()
{
    QCC_DbgPrintf(("P2PNameService::~P2PNameService()"));

    //
    // We get here (on Linux) because when main() returns, the function
    // __run_exit_handlers() is called which, in turn, calls the destructors of
    // all of the static objects in whatever order the linker has decided will
    // be best.
    //
    // For us, the troublesome object is going to be the BundledDaemon.  It is
    // torn down by the call to its destructor which may happen before or after
    // the call to our destructor.  If we are destroyed first, the bundled
    // daemon may happily continue to call into the name service singleton since
    // it may have no idea that it is about to go away.
    //
    // Eventually, we want to explicitly control the construction and
    // destruction order, but for now we have to live with the insanity (and
    // complexity) of dealing with out-of-order destruction.
    //
    // The name service singleton is a static, so its underlying memory won't go
    // away.  So by marking the singleton as destroyed we will have a lasting
    // indication that it has become unusable in case some bozo (the bundled
    // daemon) accesses us during destruction time after we have been destroyed.
    //
    // The exit handlers are going to be called by the main thread (that
    // originally called the main function), so the destructors will be called
    // sequentially.  The interesting problem, though, is that the BundledDaemon
    // is going to have possibly more than one transport running, and typically
    // each of those transports has multiple threads that could conceivably be
    // making name service calls.  So, while our destructor is being called by
    // the main thread, it is possible that other transport threads will also
    // be calling.  Like in hunting rabbits, We've got to be very, very careful.
    //
    //
    // First, make sure no callbacks leak out of the private implementation
    // during this critical time by turning off ALL callbacks to all transports.
    //
    if (m_pimpl) {
        m_pimpl->SetCallback(TRANSPORT_ANY, NULL);
    }

    //
    // Now we slam shut an entry gate so that no new callers can get through and
    // try to do things while we are destroying the private implementation.
    //
    m_destroyed = true;

    //
    // No new callers will now be let in, but there may be existing callers
    // rummaging around in the object.  If the private implemetation is not
    // careful about multithreading, it can begin destroying itself with
    // existing calls in progress.  Thankfully, that's not our problem here.
    // We assume it will do the right thing.
    //
    if (m_pimpl) {

        //
        // Deleting the private implementation must accomplish an orderly shutdown
        // with an impled Stop() and Join().
        //
        delete m_pimpl;
        m_pimpl = NULL;
    }
}

#define ASSERT_STATE(function) \
    { \
        assert(m_constructed && "P2PNameService::" # function "(): Singleton not constructed"); \
        assert(!m_destroyed && "P2PNameService::" # function "(): Singleton destroyed"); \
        assert(m_pimpl && "P2PNameService::" # function "(): Private impl is NULL"); \
    }

void P2PNameService::Acquire(BusAttachment* bus, const qcc::String& guid)
{
    QCC_DbgPrintf(("P2PNameService::Acquire()"));

    //
    // If the entry gate has been closed, we do not allow an Acquire to actually
    // acquire a reference.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return;
    }

    //
    // The only way someone can get to us is if they call Instance() which will
    // cause the object to be constructed.
    //
    assert(m_constructed && "P2PNameService::Acquire(): Singleton not constructed");

    ASSERT_STATE("Acquire");
    int refs = qcc::IncrementAndFetch(&m_refCount);
    if (refs == 1) {
        //
        // The first transport in gets to set the GUID.  There should be only
        // one GUID associated with a daemon process, so this should never
        // change.
        //
        Init(bus, guid);
        Start();
    }
}

void P2PNameService::Release()
{
    QCC_DbgPrintf(("P2PNameService::Release()"));

    //
    // If the entry gate has been closed, we do not allow a Release to actually
    // release a reference.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return;
    }

    ASSERT_STATE("Release");
    int refs = qcc::DecrementAndFetch(&m_refCount);
    if (refs == 0) {
        QCC_DbgPrintf(("P2PNameService::Release(): refs == 0"));

        //
        // The last transport to release its interest in the name service gets
        // pay the price for waiting for the service to exit.  Since we do a
        // Join(), this method is expected to be called out of a transport's
        // Join() so the price is expected.
        //
        Stop();
        Join();
    }
}

QStatus P2PNameService::Start()
{
    QCC_DbgPrintf(("P2PNameService::Start()"));

    //
    // If the entry gate has been closed, we do not allow a Start to actually
    // start anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("Start");
    return m_pimpl->Start();
}

bool P2PNameService::Started()
{
    QCC_DbgPrintf(("P2PNameService::Started()"));

    //
    // If the entry gate has been closed, we do not allow a Started to actually
    // test anything.  We just return false.  The singleton is going away and so
    // we assume we are running __run_exit_handlers() so main() has returned.
    // We are definitely shutting down, and the process is going to exit, so
    // tricking callers who may be temporarily running is okay.
    //
    if (m_destroyed) {
        return false;
    }

    ASSERT_STATE("Started");
    return m_pimpl->Started();
}

QStatus P2PNameService::Stop()
{
    QCC_DbgPrintf(("P2PNameService::Stop()"));

    //
    // If the entry gate has been closed, we do not allow a Stop to actually
    // stop anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.  The destructor is going to handle
    // the actual Stop() and Join().
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("Stop");
    return m_pimpl->Stop();
}

QStatus P2PNameService::Join()
{
    QCC_DbgPrintf(("P2PNameService::Join()"));

    //
    // If the entry gate has been closed, we do not allow a Join to actually
    // join anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.  The destructor is going to handle
    // the actual Stop() and Join().
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("Join");
    return m_pimpl->Join();
}

QStatus P2PNameService::Init(BusAttachment* bus, const qcc::String& guid)
{
    QCC_DbgPrintf(("P2PNameService::Init()"));

    //
    // If the entry gate has been closed, we do not allow an Init to actually
    // init anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("Init");
    return m_pimpl->Init(bus, guid);
}

void P2PNameService::SetCallback(TransportMask transportMask,
                                 Callback<void, const qcc::String&, qcc::String&, uint8_t>* cb)
{
    QCC_DbgPrintf(("P2PNameService::SetCallback()"));

    //
    // If the entry gate has been closed, we do not allow a SetCallback to
    // actually set anything.  The singleton is going away and so we assume we
    // are running __run_exit_handlers() so main() has returned.  We are
    // definitely shutting down, and the process is going to exit, so tricking
    // callers who may be temporarily running is okay.
    //
    // The gotcha is that if there is a valid callback set, and the caller is
    // now setting the callback to NULL to prevent any new callbacks, the caller
    // will expect that no callbacks will follow this call.  This is taken care
    // of by calling SetCallback(NULL) on the private implemtation BEFORE
    // setting m_destroyed in our destructor.  In other words, the possible set
    // to NULL has already been done.
    //
    if (m_destroyed) {
        return;
    }

    ASSERT_STATE("SetCallback");
    m_pimpl->SetCallback(transportMask, cb);
}

QStatus P2PNameService::Enable(TransportMask transportMask)
{
    QCC_DbgPrintf(("P2PNameService::Enable()"));

    //
    // If the entry gate has been closed, we do not allow an Enable to actually
    // enable anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("Enable");
    m_pimpl->Enable(transportMask);
    return ER_OK;
}

QStatus P2PNameService::Enabled(TransportMask transportMask, bool& enabled)
{
    QCC_DbgPrintf(("P2PNameService::Enabled()"));

    //
    // If the entry gate has been closed, we do not allow an Enabled to actually
    // do anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("Enabled");
    return m_pimpl->Enabled(transportMask, enabled);
}

QStatus P2PNameService::Disable(TransportMask transportMask)
{
    QCC_DbgPrintf(("P2PNameService::Disable()"));

    //
    // If the entry gate has been closed, we do not allow an Enable to actually
    // enable anything.  The singleton is going away and so we assume we are
    // running __run_exit_handlers() so main() has returned.  We are definitely
    // shutting down, and the process is going to exit, so tricking callers who
    // may be temporarily running is okay.
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("Disable");
    m_pimpl->Disable(transportMask);
    return ER_OK;
}

QStatus P2PNameService::FindAdvertisedName(TransportMask transportMask, const qcc::String& prefix)
{
    QCC_DbgPrintf(("P2PNameService::FindAdvertisedName()"));

    //
    // If the entry gate has been closed, we do not allow a FindAdvertisedName
    // to actually find anything.  The singleton is going away and so we assume
    // we are running __run_exit_handlers() so main() has returned.  We are
    // definitely shutting down, and the process is going to exit, so tricking
    // callers who may be temporarily running is okay.
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("FindAdvertisedName");
    return m_pimpl->FindAdvertisedName(transportMask, prefix);
}

QStatus P2PNameService::CancelFindAdvertisedName(TransportMask transportMask, const qcc::String& prefix)
{
    QCC_DbgPrintf(("P2PNameService::CancelFindAdvertisedName()"));

    //
    // If the entry gate has been closed, we do not allow a FindAdvertisedName
    // to actually find anything.  The singleton is going away and so we assume
    // we are running __run_exit_handlers() so main() has returned.  We are
    // definitely shutting down, and the process is going to exit, so tricking
    // callers who may be temporarily running is okay.
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("CancelFindAdvertisedName");
    return m_pimpl->CancelFindAdvertisedName(transportMask, prefix);
}

QStatus P2PNameService::AdvertiseName(TransportMask transportMask, const qcc::String& wkn)
{
    //
    // If the entry gate has been closed, we do not allow an AdvertiseName to
    // actually advertise anything.  The singleton is going away and so we
    // assume we are running __run_exit_handlers() so main() has returned.  We
    // are definitely shutting down, and the process is going to exit, so
    // tricking callers who may be temporarily running is okay.
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("AdvertiseName");
    return m_pimpl->AdvertiseName(transportMask, wkn);
}

QStatus P2PNameService::CancelAdvertiseName(TransportMask transportMask, const qcc::String& wkn)
{
    //
    // If the entry gate has been closed, we do not allow a CancelAdvertiseName
    // to actually cancel anything.  The singleton is going away and so we
    // assume we are running __run_exit_handlers() so main() has returned.  We
    // are definitely shutting down, and the process is going to exit, so
    // tricking callers who may be temporarily running is okay.
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("CancelAdvertiseName");
    return m_pimpl->CancelAdvertiseName(transportMask, wkn);
}

QStatus P2PNameService::GetDeviceForGuid(const qcc::String& guid, qcc::String& device)
{
    //
    // If the entry gate has been closed, we do not allow a CancelAdvertiseName
    // to actually cancel anything.  The singleton is going away and so we
    // assume we are running __run_exit_handlers() so main() has returned.  We
    // are definitely shutting down, and the process is going to exit, so
    // tricking callers who may be temporarily running is okay.
    //
    if (m_destroyed) {
        return ER_OK;
    }

    ASSERT_STATE("GetDeviceForGuid");
    return m_pimpl->GetDeviceForGuid(guid, device);
}

} // namespace ajn
