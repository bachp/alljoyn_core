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

#include "Debug.h"

#include <qcc/Log.h>
#include <qcc/String.h>
#include <qcc/winrt/utility.h>
#include <Status.h>
#include <AllJoynException.h>

namespace AllJoyn {

void Debug::UseOSLogging(bool useOSLog)
{
    QCC_UseOSLogging(useOSLog);
}

void Debug::SetDebugLevel(Platform::String ^ module, uint32_t level)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == module) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strModule = PlatformToMultibyteString(module);
        if (strModule.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        QCC_SetDebugLevel(strModule.c_str(), level);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

}
