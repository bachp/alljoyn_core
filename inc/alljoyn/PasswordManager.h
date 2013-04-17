#ifndef _ALLJOYN_PASSWORDMANAGER_H
#define _ALLJOYN_PASSWORDMANAGER_H
/**
 * @file
 * This file defines the PasswordManager class that provides the interface to
 * set credentials used for the authentication of thin clients.
 */

/******************************************************************************
 * Copyright 2013, Qualcomm Innovation Center, Inc.
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

#ifndef __cplusplus
#error Only include PasswordManager.h in C++ code.
#endif

#include <qcc/platform.h>
#include <qcc/String.h>
#include <alljoyn/Status.h>
namespace ajn {

/**
 * Class to allow the user or application to set credentials used for the authentication
 * of thin clients.
 * Before invoking Connect() to BusAttachment, the application should call SetCredentials
 * if it expects to be able to communicate to/from thin clients.
 * The bundled daemon will start advertising the name as soon as it is started and MUST have
 * the credentials set to be able to authenticate any thin clients that may try to use the
 * bundled daemon to communicate with the app.
 */

class PasswordManager {
  public:
    /**
     * @brief Set credentials used for the authentication of thin clients.
     *
     * @param authMechanism  Mechanism to use for authentication.
     * @param password       Password to use for authentication.
     *
     * @return   Returns ER_OK if the credentials was successfully set.
     */
    static QStatus SetCredentials(qcc::String authMechanism, qcc::String password) {
        PasswordManager::authMechanism = authMechanism;
        PasswordManager::password = password;
        return ER_OK;
    }

    /// @cond ALLJOYN_DEV
    /* @internal Get the password set by the user/app
     *
     * @return Returns the password set by the user/app.
     */
    static qcc::String GetPassword() { return password; }

    /* @internal Get the authMechanism set by the user/app
     *
     * @return Returns the authMechanism set by the user/app.
     */
    static qcc::String GetAuthMechanism() { return authMechanism; }
    /// @endcond
  private:
    static qcc::String authMechanism;           /**< The auth mechanism selected by the user/app */
    static qcc::String password;                /**< The password selected by the user/app */
};

}
#endif
