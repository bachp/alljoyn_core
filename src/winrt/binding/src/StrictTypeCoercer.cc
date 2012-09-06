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

#include "StrictTypeCoercer.h"

#include <alljoyn/MsgArg.h>
#include <qcc/String.h>
#include <qcc/winrt/utility.h>
#include <Status.h>
#include <MsgArg.h>
#include <AllJoynException.h>

using namespace Windows::Foundation;
using namespace ajn;

namespace AllJoyn {

Platform::Object ^ StrictTypeCoercer::Coerce(Platform::Object ^ obj, ajn::AllJoynTypeId typeId, bool inParam)
{
    Platform::Object ^ retObj = nullptr;
    // Check obj for invalid parameters
    if (obj == nullptr) {
        return retObj;
    }

    // Get property value for object
    IPropertyValue ^ prop = dynamic_cast<IPropertyValue ^>(obj);
    PropertyType type = Windows::Foundation::PropertyType::Empty;
    // Get the property type
    if (nullptr != prop) {
        type = prop->Type;
    }

    switch (typeId) {
    case ALLJOYN_BOOLEAN:
    {
        // Property type must be boolean
        if (type == Windows::Foundation::PropertyType::Boolean) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_DOUBLE:
    {
        // Property type must be double
        if (type == Windows::Foundation::PropertyType::Double) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_VARIANT:
    case ALLJOYN_STRUCT:
    case ALLJOYN_DICT_ENTRY:
    {
        // Property type must be MsgArg
        MsgArg ^ msgArg = dynamic_cast<MsgArg ^>(obj);
        if (msgArg != nullptr) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_INT32:
    {
        // Property type must be int32
        if (type == Windows::Foundation::PropertyType::Int32) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_STRING:
    {
        // Property type must be string
        if (type == Windows::Foundation::PropertyType::String) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_INT64:
    {
        // Property type must be int64
        if (type == Windows::Foundation::PropertyType::Int64) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_BYTE:
    {
        // Property type must be uint8
        if (type == Windows::Foundation::PropertyType::UInt8) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_UINT32:
    {
        // Property type must be uint32
        if (type == Windows::Foundation::PropertyType::UInt32) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_UINT64:
    {
        // Property type must be uint64
        if (type == Windows::Foundation::PropertyType::UInt64) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_OBJECT_PATH:
    {
        // Property type must be string
        if (type == Windows::Foundation::PropertyType::String) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_SIGNATURE:
    {
        // Property type must be string
        if (type == Windows::Foundation::PropertyType::String) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_HANDLE:
    {
        // Property type must be uint64
        if (type == Windows::Foundation::PropertyType::UInt64) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_UINT16:
    {
        // Property type must be uint16
        if (type == Windows::Foundation::PropertyType::UInt16) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_INT16:
    {
        // Property type must be int16
        if (type == Windows::Foundation::PropertyType::Int16) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_ARRAY:
    {
        // Property type must be array
        if (type == Windows::Foundation::PropertyType::InspectableArray) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_BOOLEAN_ARRAY:
    {
        // Property type must be boolean array
        if (type == Windows::Foundation::PropertyType::BooleanArray) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_DOUBLE_ARRAY:
    {
        // Property type must be double array
        if (type == Windows::Foundation::PropertyType::DoubleArray) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_INT32_ARRAY:
    {
        // Property type must be int32 array
        if (type == Windows::Foundation::PropertyType::Int32Array) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_INT16_ARRAY:
    {
        // Property type must be int16 array
        if (type == Windows::Foundation::PropertyType::Int16Array) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_UINT16_ARRAY:
    {
        // Property type must be uint16 array
        if (type == Windows::Foundation::PropertyType::UInt16Array) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_UINT64_ARRAY:
    {
        // Property type must be uint64 array
        if (type == Windows::Foundation::PropertyType::UInt64Array) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_UINT32_ARRAY:
    {
        // Property type must be uint32 array
        if (type == Windows::Foundation::PropertyType::UInt32Array) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_INT64_ARRAY:
    {
        // Property type must be int64 array
        if (type == Windows::Foundation::PropertyType::Int64Array) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_BYTE_ARRAY:
    {
        // Property type must be byte array
        if (type == Windows::Foundation::PropertyType::UInt8Array) {
            retObj = obj;
        }
    }
    break;

    case 'sa':
    case 'oa':
    case 'ga':
    {
        // Property type must be string array
        if (type == Windows::Foundation::PropertyType::StringArray) {
            retObj = obj;
        }
    }
    break;

    default:
        break;
    }

    return retObj;
}

}
