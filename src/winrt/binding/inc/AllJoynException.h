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
#include <Status_CPP0x.h>
#include <qcc/winrt/utility.h>

namespace AllJoyn {

#ifndef NDEBUG
extern const char* QCC_StatusComment(uint32_t status);
#endif

/// <summary>
/// A help class to interpret a COM exception throwed by AllJoyn.
/// </summary>
public ref class AllJoynException sealed {
  public:
    /// <summary>
    /// Tranlate the COMException HResult value to a QStatus code.
    /// </summary>
    /// <param name="hresult">The HResult value of a COMException </param>
    /// <returns>A corresponding QStatus code </returns>
    static QStatus FromCOMException(int hresult)
    {
        return (QStatus)(hresult & 0x7FFFFFFF);
    }
#ifndef NDEBUG
    /// <summary>
    /// Get a string that gives more detail explanation of the COMException.
    /// </summary>
    /// <param name="hresult">The HResult value of a COMException </param>
    /// <returns>A string explaining the reason of the exception </returns>
    static Platform::String ^ GetExceptionMessage(int hresult)
    {
        return MultibyteToPlatformString(QCC_StatusComment(hresult & 0x7FFFFFFF));
    }
#endif
};

}
// AllJoynException.h
