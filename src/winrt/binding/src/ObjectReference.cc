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

#include "ObjectReference.h"
#include <AllJoynException.h>

namespace AllJoyn {

void AddObjectReference(qcc::Mutex* mtx, Platform::Object ^ key, std::map<void*, void*>* map)
{
    if (NULL != mtx) {
        mtx->Lock();
    }
    void* handle = (void*)key;
    if (map->find(handle) == map->end()) {
        Platform::Object ^ oHandle = key;
        __abi_IUnknown* pUnk = reinterpret_cast<__abi_IUnknown*>(oHandle);
        pUnk->__abi_AddRef();
        (*map)[handle] = handle;
    }
    if (NULL != mtx) {
        mtx->Unlock();
    }
}

void RemoveObjectReference(qcc::Mutex* mtx, Platform::Object ^ key, std::map<void*, void*>* map)
{
    if (NULL != mtx) {
        mtx->Lock();
    }
    void* handle = (void*)key;
    if (map->find(handle) != map->end()) {
        Platform::Object ^ oHandle = key;
        __abi_IUnknown* pUnk = reinterpret_cast<__abi_IUnknown*>(oHandle);
        pUnk->__abi_Release();
        map->erase(handle);
    }
    if (NULL != mtx) {
        mtx->Unlock();
    }
}

void AddObjectReference2(qcc::Mutex* mtx, void* key, Platform::Object ^ val, std::map<void*, void*>* map)
{
    if (NULL != mtx) {
        mtx->Lock();
    }
    void* handle = (void*)key;
    if (map->find(handle) == map->end()) {
        Platform::Object ^ oHandle = val;
        __abi_IUnknown* pUnk = reinterpret_cast<__abi_IUnknown*>(oHandle);
        pUnk->__abi_AddRef();
        (*map)[handle] = (void*)oHandle;
    }
    if (NULL != mtx) {
        mtx->Unlock();
    }
}

void RemoveObjectReference2(qcc::Mutex* mtx, void* key, std::map<void*, void*>* map)
{
    if (NULL != mtx) {
        mtx->Lock();
    }
    void* handle = (void*)key;
    if (map->find(handle) != map->end()) {
        __abi_IUnknown* pUnk = reinterpret_cast<__abi_IUnknown*>((*map)[handle]);
        pUnk->__abi_Release();
        map->erase(handle);
    }
    if (NULL != mtx) {
        mtx->Unlock();
    }
}

void ClearObjectMap(qcc::Mutex* mtx, std::map<void*, void*>* m)
{
    if (NULL != mtx) {
        mtx->Lock();
    }
    for (std::map<void*, void*, std::less<void*> >::const_iterator iter = m->begin();
         iter != m->end();
         ++iter) {
        __abi_IUnknown* pUnk = reinterpret_cast<__abi_IUnknown*>(iter->second);
        pUnk->__abi_Release();
    }
    m->clear();
    if (NULL != mtx) {
        mtx->Unlock();
    }
}

void AddIdReference(qcc::Mutex* mtx, ajn::SessionPort key, Platform::Object ^ val, std::map<ajn::SessionId, std::map<void*, void*>*>* m)
{
    if (val == nullptr) {
        return;
    }

    if (NULL != mtx) {
        mtx->Lock();
    }
    if (m->find(key) == m->end()) {
        std::map<void*, void*>* lMap = new std::map<void*, void*>();
        if (NULL == lMap) {
            QCC_THROW_EXCEPTION(ER_OUT_OF_MEMORY);
        }
        Platform::Object ^ oHandle = val;
        (*lMap)[(void*)oHandle] = (void*)oHandle;
        __abi_IUnknown* pUnk = reinterpret_cast<__abi_IUnknown*>(oHandle);
        pUnk->__abi_AddRef();
        (*m)[key] = lMap;
    } else {
        Platform::Object ^ oHandle = val;
        std::map<void*, void*>* lMap = (*m)[key];
        if (lMap->find((void*)oHandle) == lMap->end()) {
            (*lMap)[(void*)oHandle] = (void*)oHandle;
            __abi_IUnknown* pUnk = reinterpret_cast<__abi_IUnknown*>(oHandle);
            pUnk->__abi_AddRef();
            (*m)[key] = lMap;
        }
    }
    if (NULL != mtx) {
        mtx->Unlock();
    }
}

void RemoveIdReference(qcc::Mutex* mtx, ajn::SessionPort key, std::map<ajn::SessionId, std::map<void*, void*>*>* m)
{
    if (NULL != mtx) {
        mtx->Lock();
    }
    if (m->find(key) != m->end()) {
        std::map<void*, void*>* lMap = (*m)[key];
        for (std::map<void*, void*, std::less<void*> >::const_iterator iter = lMap->begin();
             iter != lMap->end();
             ++iter) {
            __abi_IUnknown* pUnk = reinterpret_cast<__abi_IUnknown*>(iter->second);
            pUnk->__abi_Release();
        }
        m->erase(key);
        lMap->clear();
        delete lMap;
        lMap = NULL;
    }
    if (NULL != mtx) {
        mtx->Unlock();
    }
}

void ClearIdMap(qcc::Mutex* mtx, std::map<ajn::SessionId, std::map<void*, void*>*>* m)
{
    if (NULL != mtx) {
        mtx->Lock();
    }
    for (std::map<ajn::SessionId, std::map<void*, void*>*, std::less<ajn::SessionId> >::const_iterator iter = m->begin();
         iter != m->end();
         ++iter) {
        std::map<void*, void*>* lMap = iter->second;
        for (std::map<void*, void*, std::less<void*> >::const_iterator iter = lMap->begin();
             iter != lMap->end();
             ++iter) {
            __abi_IUnknown* pUnk = reinterpret_cast<__abi_IUnknown*>(iter->second);
            pUnk->__abi_Release();
        }
        lMap->clear();
        delete lMap;
        lMap = NULL;
    }
    m->clear();
    if (NULL != mtx) {
        mtx->Unlock();
    }
}

void AddPortReference(qcc::Mutex* mtx, ajn::SessionPort key, Platform::Object ^ val, std::map<ajn::SessionPort, std::map<void*, void*>*>* m)
{
    if (NULL != mtx) {
        mtx->Lock();
    }
    if (m->find(key) == m->end()) {
        std::map<void*, void*>* lMap = new std::map<void*, void*>();
        if (NULL == lMap) {
            QCC_THROW_EXCEPTION(ER_OUT_OF_MEMORY);
        }
        Platform::Object ^ oHandle = val;
        (*lMap)[(void*)oHandle] = (void*)oHandle;
        __abi_IUnknown* pUnk = reinterpret_cast<__abi_IUnknown*>(oHandle);
        pUnk->__abi_AddRef();
        (*m)[key] = lMap;
    } else {
        Platform::Object ^ oHandle = val;
        std::map<void*, void*>* lMap = (*m)[key];
        if (lMap->find((void*)oHandle) == lMap->end()) {
            (*lMap)[(void*)oHandle] = (void*)oHandle;
            __abi_IUnknown* pUnk = reinterpret_cast<__abi_IUnknown*>(oHandle);
            pUnk->__abi_AddRef();
            (*m)[key] = lMap;
        }
    }
    if (NULL != mtx) {
        mtx->Unlock();
    }
}

void RemovePortReference(qcc::Mutex* mtx, ajn::SessionPort key, std::map<ajn::SessionPort, std::map<void*, void*>*>* m)
{
    if (NULL != mtx) {
        mtx->Lock();
    }
    if (m->find(key) != m->end()) {
        std::map<void*, void*>* lMap = (*m)[key];
        for (std::map<void*, void*, std::less<void*> >::const_iterator iter = lMap->begin();
             iter != lMap->end();
             ++iter) {
            __abi_IUnknown* pUnk = reinterpret_cast<__abi_IUnknown*>(iter->second);
            pUnk->__abi_Release();
        }
        m->erase(key);
        lMap->clear();
        delete lMap;
        lMap = NULL;
    }
    if (NULL != mtx) {
        mtx->Unlock();
    }
}

void ClearPortMap(qcc::Mutex* mtx, std::map<ajn::SessionPort, std::map<void*, void*>*>* m)
{
    if (NULL != mtx) {
        mtx->Lock();
    }
    for (std::map<ajn::SessionPort, std::map<void*, void*>*, std::less<ajn::SessionPort> >::const_iterator iter = m->begin();
         iter != m->end();
         ++iter) {
        std::map<void*, void*>* lMap = iter->second;
        for (std::map<void*, void*, std::less<void*> >::const_iterator iter = lMap->begin();
             iter != lMap->end();
             ++iter) {
            __abi_IUnknown* pUnk = reinterpret_cast<__abi_IUnknown*>(iter->second);
            pUnk->__abi_Release();
        }
        lMap->clear();
        delete lMap;
        lMap = NULL;
    }
    m->clear();
    if (NULL != mtx) {
        mtx->Unlock();
    }
}

uint32_t QueryReferenceCount(Platform::Object ^ obj)
{
    __abi_IUnknown* pUnk = reinterpret_cast<__abi_IUnknown*>(obj);
    pUnk->__abi_AddRef();
    uint32_t count = pUnk->__abi_Release();
    // Adjust for the assignment
    return count - 1;
}

}
