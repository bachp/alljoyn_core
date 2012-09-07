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

#include <qcc/Debug.h>

#include "IpNameService.h"
#include "IpNameServiceImpl.h"

#define QCC_MODULE "IPNS"

namespace ajn {

IpNameService::IpNameService()
    : m_constructed(false), m_destroyed(false), m_pimpl(NULL)
{
    m_pimpl = new IpNameServiceImpl;
    m_constructed = true;
}

IpNameService::~IpNameService()
{
    assert(m_pimpl && "IpNameService::~IpNameService(): Private impl is NULL");
    assert(m_pimpl->Started() == false && "IpNameService::~IpNameService(): Private impl is started");

    delete m_pimpl;
    m_pimpl = NULL;
    m_destroyed = true;
}

#define ASSERT_STATE(function) \
    { \
        assert(m_constructed && "IpNameService::" #function "(): Singleton not constructed"); \
        assert(!m_destroyed && "IpNameService::" #function "(): Singleton destroyed"); \
        assert(m_pimpl && "IpNameService::" #function "(): Private impl is NULL"); \
    }

QStatus IpNameService::Start()
{
    ASSERT_STATE("Start");
    return m_pimpl->Start();
}

bool IpNameService::Started()
{
    ASSERT_STATE("Started");
    return m_pimpl->Started();
}

QStatus IpNameService::Stop()
{
    ASSERT_STATE("Stop");
    return m_pimpl->Stop();
}

QStatus IpNameService::Join()
{
    ASSERT_STATE("Join");
    return m_pimpl->Start();
}

QStatus IpNameService::Init(
    const qcc::String& guid,
    bool enableIPv4,
    bool enableIPv6,
    bool disableBroadcast,
    bool loopback)
{
    ASSERT_STATE("Init");
    return m_pimpl->Init(guid, enableIPv4, enableIPv6, disableBroadcast, loopback);
}

void IpNameService::SetCallback(Callback<void, const qcc::String&, const qcc::String&, std::vector<qcc::String>&, uint8_t>* cb)
{
    ASSERT_STATE("SetCallback");
    m_pimpl->SetCallback(cb);
}

QStatus IpNameService::OpenInterface(const qcc::String& name)
{
    ASSERT_STATE("OpenInterface");
    return m_pimpl->OpenInterface(name);
}

QStatus IpNameService::OpenInterface(const qcc::IPAddress& address)
{
    ASSERT_STATE("OpenInterface");
    return m_pimpl->OpenInterface(address);
}

QStatus IpNameService::CloseInterface(const qcc::String& name)
{
    ASSERT_STATE("CloseInterface");
    return m_pimpl->CloseInterface(name);
}

QStatus IpNameService::CloseInterface(const qcc::IPAddress& address)
{
    ASSERT_STATE("CloseInterface");
    return m_pimpl->CloseInterface(address);
}

QStatus IpNameService::Enable()
{
    ASSERT_STATE("Enable");
    m_pimpl->Enable();
    return ER_OK;
}

QStatus IpNameService::Disable()
{
    ASSERT_STATE("Disable");
    m_pimpl->Disable();
    return ER_OK;
}

bool IpNameService::Enabled()
{
    ASSERT_STATE("Enabled");
    return m_pimpl->Enabled();
}

QStatus IpNameService::FindAdvertisedName(const qcc::String& wkn)
{
    ASSERT_STATE("FindAdvertisedName");
    return m_pimpl->Locate(wkn);
}

QStatus IpNameService::CancelFindAdvertisedName(const qcc::String& wkn)
{
    return ER_OK;
}

QStatus IpNameService::AdvertiseName(const qcc::String& wkn)
{
    ASSERT_STATE("AdvertiseName");
    return m_pimpl->Advertise(wkn);
}

QStatus IpNameService::CancelAdvertiseName(const qcc::String& wkn)
{
    ASSERT_STATE("CancelAdvertiseName");
    return m_pimpl->Cancel(wkn);
}

QStatus IpNameService::SetEndpoints(
    const qcc::String& ipv4address,
    const qcc::String& ipv6address,
    uint16_t port)
{
    ASSERT_STATE("SetEndpoints");
    return m_pimpl->SetEndpoints(ipv4address, ipv6address, port);
}

QStatus IpNameService::GetEndpoints(
    qcc::String& ipv4address,
    qcc::String& ipv6address,
    uint16_t& port)
{
    ASSERT_STATE("GetEndpoints");
    return m_pimpl->GetEndpoints(ipv4address, ipv6address, port);
}

} // namespace ajn
