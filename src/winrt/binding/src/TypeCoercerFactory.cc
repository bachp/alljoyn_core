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

#include "TypeCoercerFactory.h"

#include <StrictTypeCoercer.h>
#include <WeakTypeCoercer.h>
#include <qcc/String.h>
#include <qcc/winrt/utility.h>
#include <Status.h>
#include <AllJoynException.h>
#include <qcc/Mutex.h>

namespace AllJoyn {

StrictTypeCoercer* _strictCoercer = NULL;
WeakTypeCoercer* _weakCoercer = NULL;
qcc::Mutex _typeFactoryMutex;

ITypeCoercer* TypeCoercerFactory::GetTypeCoercer(Platform::String ^ name)
{
    ITypeCoercer* result = NULL;

    if (wcscmp(L"strict", name->Data()) == 0) {
        _typeFactoryMutex.Lock();
        if (NULL == _strictCoercer) {
            _strictCoercer = new StrictTypeCoercer();
        }
        _typeFactoryMutex.Unlock();
        result =  _strictCoercer;
    } else if (wcscmp(L"weak", name->Data()) == 0) {
        _typeFactoryMutex.Lock();
        if (NULL == _weakCoercer) {
            _weakCoercer = new WeakTypeCoercer();
        }
        _typeFactoryMutex.Unlock();
        result = _weakCoercer;
    }

    return result;
}

}
