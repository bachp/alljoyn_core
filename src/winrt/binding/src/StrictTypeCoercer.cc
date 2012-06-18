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
    if (obj == nullptr) {
        return retObj;
    }

    IPropertyValue ^ prop = dynamic_cast<IPropertyValue ^>(obj);
    PropertyType type = Windows::Foundation::PropertyType::Empty;
    if (nullptr != prop) {
        type = prop->Type;
    }

    switch (typeId) {
    case ALLJOYN_BOOLEAN:
    {
        if (type == Windows::Foundation::PropertyType::Boolean) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_DOUBLE:
    {
        if (type == Windows::Foundation::PropertyType::Double) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_VARIANT:
    case ALLJOYN_STRUCT:
    case ALLJOYN_DICT_ENTRY:
    {
        MsgArg ^ msgArg = dynamic_cast<MsgArg ^>(obj);
        if (msgArg != nullptr) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_INT32:
    {
        if (type == Windows::Foundation::PropertyType::Int32) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_STRING:
    {
        if (type == Windows::Foundation::PropertyType::String) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_INT64:
    {
        if (type == Windows::Foundation::PropertyType::Int64) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_BYTE:
    {
        if (type == Windows::Foundation::PropertyType::UInt8) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_UINT32:
    {
        if (type == Windows::Foundation::PropertyType::UInt32) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_UINT64:
    {
        if (type == Windows::Foundation::PropertyType::UInt64) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_OBJECT_PATH:
    {
        if (type == Windows::Foundation::PropertyType::String) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_SIGNATURE:
    {
        if (type == Windows::Foundation::PropertyType::String) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_HANDLE:
    {
        if (type == Windows::Foundation::PropertyType::UInt64) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_UINT16:
    {
        if (type == Windows::Foundation::PropertyType::UInt16) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_INT16:
    {
        if (type == Windows::Foundation::PropertyType::Int16) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_ARRAY:
    {
        if (type == Windows::Foundation::PropertyType::InspectableArray) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_BOOLEAN_ARRAY:
    {
        if (type == Windows::Foundation::PropertyType::BooleanArray) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_DOUBLE_ARRAY:
    {
        if (type == Windows::Foundation::PropertyType::DoubleArray) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_INT32_ARRAY:
    {
        if (type == Windows::Foundation::PropertyType::Int32Array) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_INT16_ARRAY:
    {
        if (type == Windows::Foundation::PropertyType::Int16Array) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_UINT16_ARRAY:
    {
        if (type == Windows::Foundation::PropertyType::UInt16Array) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_UINT64_ARRAY:
    {
        if (type == Windows::Foundation::PropertyType::UInt64Array) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_UINT32_ARRAY:
    {
        if (type == Windows::Foundation::PropertyType::UInt32Array) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_INT64_ARRAY:
    {
        if (type == Windows::Foundation::PropertyType::Int64Array) {
            retObj = obj;
        }
    }
    break;

    case ALLJOYN_BYTE_ARRAY:
    {
        if (type == Windows::Foundation::PropertyType::UInt8Array) {
            retObj = obj;
        }
    }
    break;

    case 'sa':
    case 'oa':
    case 'ga':
    {
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
