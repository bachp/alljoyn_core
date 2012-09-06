/******************************************************************************
 *
 * Copyright 2011-2012, Qualcomm Innovation Center, Inc.
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
 *
 *****************************************************************************/

#include "SessionOpts.h"

#include <qcc/String.h>
#include <qcc/winrt/utility.h>
#include <alljoyn/Session.h>
#include <Status.h>
#include <ObjectReference.h>
#include <AllJoynException.h>

namespace AllJoyn {

SessionOpts::SessionOpts()
{
    ::QStatus status = ER_OK;

    while (true) {
        // Create _SessionOpts
        _SessionOpts* so = new _SessionOpts();
        // Check for allocation error
        if (NULL == so) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Attach so to managed _SessionOpts
        _mSessionOpts = new qcc::ManagedObj<_SessionOpts>(so);
        // Check for allocation error
        if (NULL == _mSessionOpts) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Store pointer to _SessionOpts for convenience
        _sessionOpts = &(**_mSessionOpts);
        break;
    }

    // Bubble up any QStatus errors as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

SessionOpts::SessionOpts(TrafficType traffic, bool isMultipoint, ProximityType proximity, TransportMaskType transports)
{
    ::QStatus status = ER_OK;

    while (true) {
        // Create _SessionOpts
        _SessionOpts* so = new _SessionOpts((ajn::SessionOpts::TrafficType)(int)traffic, isMultipoint, (ajn::SessionOpts::Proximity)(int)proximity, (ajn::TransportMask)(int)transports);
        // Check for allocation error
        if (NULL == so) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Attach so to managed _SessionOpts
        _mSessionOpts = new qcc::ManagedObj<_SessionOpts>(so);
        // Check for allocation error
        if (NULL == _mSessionOpts) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Store pointer to _SessionOpts for convenience
        _sessionOpts = &(**_mSessionOpts);
        break;
    }

    // Bubble up any QStatus errors as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

SessionOpts::SessionOpts(const ajn::SessionOpts* sessionOpts)
{
    ::QStatus status = ER_OK;

    while (true) {
        // Check sessionOpts for invalid values
        if (NULL == sessionOpts) {
            status = ER_BAD_ARG_1;
            break;
        }
        // Create _SessionOpts
        _SessionOpts* so = new _SessionOpts(sessionOpts->traffic,
                                            sessionOpts->isMultipoint,
                                            sessionOpts->proximity,
                                            sessionOpts->transports);
        // Check for allocation error
        if (NULL == so) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Attach so to managed _SessionOpts
        _mSessionOpts = new qcc::ManagedObj<_SessionOpts>(so);
        // Check for allocation error
        if (NULL == _mSessionOpts) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Store pointer to _SessionOpts for convenience
        _sessionOpts = &(**_mSessionOpts);
        break;
    }

    // Bubble up any QStatus errors as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

SessionOpts::SessionOpts(const qcc::ManagedObj<_SessionOpts>* sessionOpts)
{
    ::QStatus status = ER_OK;

    while (true) {
        // Check sessionOpts for invalid values
        if (NULL == sessionOpts) {
            status = ER_BAD_ARG_1;
            break;
        }
        // Attach sessionOpts to managed _SessionOpts
        _mSessionOpts = new qcc::ManagedObj<_SessionOpts>(*sessionOpts);
        // Check for allocation error
        if (NULL == _mSessionOpts) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        // Store pointer to _SessionOpts for convenience
        _sessionOpts = &(**_mSessionOpts);
        break;
    }

    // Bubble up any QStatus errors as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

SessionOpts::~SessionOpts()
{
    // Delete managed _SessionOpts to adjust ref count
    if (NULL != _mSessionOpts) {
        delete _mSessionOpts;
        _mSessionOpts = NULL;
        _sessionOpts = NULL;
    }
}

TrafficType SessionOpts::Traffic::get()
{
    TrafficType result = (TrafficType)(int)-1;

    // Check if wrapped value already exists
    if ((TrafficType)(int)-1 == _sessionOpts->_eventsAndProperties->Traffic) {
        result = (TrafficType)(int)_sessionOpts->traffic;
        // Store the result
        _sessionOpts->_eventsAndProperties->Traffic = result;
    } else {
        // Return Traffic
        result = _sessionOpts->_eventsAndProperties->Traffic;
    }

    return result;
}

void SessionOpts::Traffic::set(TrafficType value)
{
    // Store value
    _sessionOpts->traffic = (ajn::SessionOpts::TrafficType)(int)value;
    _sessionOpts->_eventsAndProperties->Traffic = value;
}

bool SessionOpts::IsMultipoint::get()
{
    bool result = (bool)-1;

    // Check if wrapped value already exists
    if ((bool)-1 == _sessionOpts->_eventsAndProperties->IsMultipoint) {
        result = _sessionOpts->isMultipoint;
        // Store the result
        _sessionOpts->_eventsAndProperties->IsMultipoint = result;
    } else {
        // Return IsMultipoint
        result = _sessionOpts->_eventsAndProperties->IsMultipoint;
    }

    return result;
}

void SessionOpts::IsMultipoint::set(bool value)
{
    // Store value
    _sessionOpts->isMultipoint = value;
    _sessionOpts->_eventsAndProperties->IsMultipoint = value;
}

ProximityType SessionOpts::Proximity::get()
{
    ProximityType result = (ProximityType)(int)-1;

    // Check if wrapped value already exits
    if ((ProximityType)(int)-1 == _sessionOpts->_eventsAndProperties->Proximity) {
        result = (ProximityType)(int)_sessionOpts->proximity;
        // Store the result
        _sessionOpts->_eventsAndProperties->Proximity = result;
    } else {
        // Return Proximity
        result = _sessionOpts->_eventsAndProperties->Proximity;
    }

    return result;
}

void SessionOpts::Proximity::set(ProximityType value)
{
    // Store value
    _sessionOpts->proximity = (ajn::SessionOpts::Proximity)(int)value;
    _sessionOpts->_eventsAndProperties->Proximity = value;
}

TransportMaskType SessionOpts::TransportMask::get()
{
    TransportMaskType result = (TransportMaskType)(int)-1;

    // Check if wrapped value already exists
    if ((TransportMaskType)(int)-1 == _sessionOpts->_eventsAndProperties->TransportMask) {
        result = (TransportMaskType)(int)_sessionOpts->transports;
        // Store the result
        _sessionOpts->_eventsAndProperties->TransportMask = result;
    } else {
        // Return TransportMask
        result = _sessionOpts->_eventsAndProperties->TransportMask;
    }

    return result;
}

void SessionOpts::TransportMask::set(TransportMaskType value)
{
    // Store value
    _sessionOpts->transports = (ajn::TransportMask)(int)value;
    _sessionOpts->_eventsAndProperties->TransportMask = value;
}

_SessionOpts::_SessionOpts()
    : ajn::SessionOpts()
{
    ::QStatus status = ER_OK;

    while (true) {
        // Create internal ref class
        _eventsAndProperties = ref new __SessionOpts();
        // Check for allocation error
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    // Bubble up any QStatus errors as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_SessionOpts::_SessionOpts(ajn::SessionOpts::TrafficType traffic, bool isMultipoint, ajn::SessionOpts::Proximity proximity, ajn::TransportMask transports)
    : ajn::SessionOpts(traffic, isMultipoint, proximity, transports)
{
    ::QStatus status = ER_OK;

    while (true) {
        // Create internal ref class
        _eventsAndProperties = ref new __SessionOpts();
        // Check for allocation error
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    // Bubble up any QStatus errors as an exception
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_SessionOpts::~_SessionOpts()
{
    _eventsAndProperties = nullptr;
}

__SessionOpts::__SessionOpts()
{
    Traffic = (TrafficType)(int)-1;
    IsMultipoint = (bool)-1;
    Proximity = (ProximityType)(int)-1;
    TransportMask = (TransportMaskType)(int)-1;
}

__SessionOpts::~__SessionOpts()
{
    Traffic = (TrafficType)(int)-1;
    IsMultipoint = (bool)-1;
    Proximity = (ProximityType)(int)-1;
    TransportMask = (TransportMaskType)(int)-1;
}

}
