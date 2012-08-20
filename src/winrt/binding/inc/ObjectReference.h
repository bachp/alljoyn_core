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

#include <alljoyn/Session.h>
#include <qcc/Mutex.h>
#include <map>

namespace AllJoyn {

inline void AddObjectReference(Platform::Object ^ obj)
{
    __abi_IUnknown* pUnk = reinterpret_cast<__abi_IUnknown*>(obj);
    pUnk->__abi_AddRef();
}

inline void RemoveObjectReference(Platform::Object ^ obj)
{
    __abi_IUnknown* pUnk = reinterpret_cast<__abi_IUnknown*>(obj);
    pUnk->__abi_Release();
}

void AddObjectReference(qcc::Mutex * mtx, Platform::Object ^ key, std::map<void*, void*>* map);

void RemoveObjectReference(qcc::Mutex * mtx, Platform::Object ^ key, std::map<void*, void*>* map);

void AddObjectReference2(qcc::Mutex * mtx, void* key, Platform::Object ^ val, std::map<void*, void*>* map);

void RemoveObjectReference2(qcc::Mutex* mtx, void* key, std::map<void*, void*>* map);

void ClearObjectMap(qcc::Mutex* mtx, std::map<void*, void*>* m);

void AddIdReference(qcc::Mutex * mtx, ajn::SessionPort key, Platform::Object ^ val, std::map<ajn::SessionId, std::map<void*, void*>*>* m);

void RemoveIdReference(qcc::Mutex* mtx, ajn::SessionPort key, std::map<ajn::SessionId, std::map<void*, void*>*>* m);

void ClearIdMap(qcc::Mutex* mtx, std::map<ajn::SessionId, std::map<void*, void*>*>* m);

void AddPortReference(qcc::Mutex * mtx, ajn::SessionPort key, Platform::Object ^ val, std::map<ajn::SessionPort, std::map<void*, void*>*>* m);

void RemovePortReference(qcc::Mutex* mtx, ajn::SessionPort key, std::map<ajn::SessionPort, std::map<void*, void*>*>* m);

void ClearPortMap(qcc::Mutex* mtx, std::map<ajn::SessionPort, std::map<void*, void*>*>* m);

uint32_t QueryReferenceCount(Platform::Object ^ obj);

}
// ObjectReference.h
