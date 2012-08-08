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

#pragma once

#include <qcc/platform.h>

namespace AllJoyn {

public ref class Debug sealed {
  public:
    /// <summary>Indicate whether AllJoyn logging goes to OS logger or stdout</summary>
    /// <param name="useOSLog">
    ///true iff OS specific logging should be used rather than print for AllJoyn debug messages.
    /// </param>
    static void UseOSLogging(bool useOSLog);

    /// <summary>Set AllJoyn debug levels.</summary>
    /// <param name="module">name of the module to generate debug output</param>
    /// <param name="level">debug level to set for the module</param>
    static void SetDebugLevel(Platform::String ^ module, uint32_t level);
};

}
// Debug.h
