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
#include <qcc/winrt/exception.h>

namespace AllJoyn {

/// <summary>
/// A helper class to interpret a customized COMException thrown by AllJoyn component. When an AllJoyn
/// method/signal call fails, AllJoyn throws a customized COMException which contains the QStatus code.
/// This class provides static methods to retrieve the QStatus/Error code and the corresponding error message.
/// </summary>
public ref class AllJoynException sealed {
  public:
    /// <summary>
    /// Map a COMException HResult property to the corresponding AllJoyn QStatus code.
    /// </summary>
    /// <param name="hresult">The HResult property of a COMException </param>
    /// <returns>The corresponding AllJoyn QStatus code </returns>
    static QStatus GetErrorCode(int hresult)
    {
        return (QStatus)(hresult & 0x3FFFFFFF);
    }

    /// <summary>
    /// Get a text string that gives more information about a COMException thrown by AllJoyn component.
    /// </summary>
    /// <param name="hresult">The HResult property of a COMException </param>
    /// <returns>A text string that sheds light on the cause of the COMException </returns>
    static Platform::String ^ GetErrorMessage(int hresult)
    {
        return MultibyteToPlatformString(qcc::QCC_StatusMessage(hresult & 0x3FFFFFFF));
    }
};

}
// AllJoynException.h
