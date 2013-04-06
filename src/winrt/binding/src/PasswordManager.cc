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

#include "PasswordManager.h"

#include <qcc/String.h>
#include <qcc/winrt/utility.h>

using namespace Windows::Foundation;

namespace AllJoyn {

QStatus PasswordManager::SetCredentials(Platform::String ^ authMechanism, Platform::String ^ password) {
    qcc::String strAuthMechanism = PlatformToMultibyteString(authMechanism);
    qcc::String strPassword = PlatformToMultibyteString(password);
    return (AllJoyn::QStatus)ajn::PasswordManager::SetCredentials(strAuthMechanism.c_str(), strPassword.c_str());

}
}
