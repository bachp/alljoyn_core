#ifndef _ALLJOYN_PROTECTED_AUTH_LISTENER_H
#define _ALLJOYN_PROTECTED_AUTH_LISTENER_H
/**
 * @file
 * This file defines a wrapper class for ajn::AuthListener that protects against asynchronous
 * deregistration of the listener instance.
 */

/******************************************************************************
 * Copyright 2011, Qualcomm Innovation Center, Inc.
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
#error Only include ProtectedAuthListener.h in C++ code.
#endif

#include <qcc/platform.h>
#include <qcc/Mutex.h>
#include <qcc/String.h>
#include <qcc/Thread.h>

#include <alljoyn/AuthListener.h>

#include <Status.h>


namespace ajn {

/**
 * This class adds a level of indirection to an AuthListener so the actual AuthListener can
 * asynchronously be set or removed safely. If the
 */
class ProtectedAuthListener : public AuthListener {

  public:

    ProtectedAuthListener() : listener(NULL), refCount(0) { }

    /**
     * Virtual destructor for derivable class.
     */
    virtual ~ProtectedAuthListener()
    {
        Set(NULL);
    }

    /**
     * Set the listener. If one of internal listener callouts is currently being called this
     * function will block until the callout returns.
     */
    void Set(AuthListener* listener);

    /**
     * Simply wraps the call of the same name to the inner AuthListener
     */
    bool RequestCredentials(const char* authMechanism, const char* peerName, uint16_t authCount, const char* userName, uint16_t credMask, Credentials& credentials);

    /**
     * Simply wraps the call of the same name to the inner AuthListener
     */
    bool VerifyCredentials(const char* authMechanism, const char* peerName, const Credentials& credentials);

    /**
     * Simply wraps the call of the same name to the inner AuthListener
     */
    void SecurityViolation(QStatus status, const Message& msg);

    /**
     * Simply wraps the call of the same name to the inner AuthListener
     */
    void AuthenticationComplete(const char* authMechanism, const char* peerName, bool success);

  private:

    /*
     * The inner listener that is being protected.
     */
    AuthListener* listener;

    qcc::Mutex lock;

    /*
     * Reference count so we know when the inner listener is no longer in use.
     */
    volatile int32_t refCount;
};

}

#endif
