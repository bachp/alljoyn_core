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

#include "ProxyBusObject.h"

#include <BusAttachment.h>
#include <InterfaceDescription.h>
#include <MessageReceiver.h>
#include <MsgArg.h>
#include <qcc/String.h>
#include <qcc/winrt/utility.h>
#include <ObjectReference.h>
#include <AllJoynException.h>

namespace AllJoyn {

ProxyBusObject::ProxyBusObject(BusAttachment ^ bus, Platform::String ^ service, Platform::String ^ path, ajn::SessionId sessionId)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == bus) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strService = PlatformToMultibyteString(service);
        if (strService.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == path) {
            status = ER_BAD_ARG_3;
            break;
        }
        qcc::String strPath = PlatformToMultibyteString(path);
        if (strPath.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _ProxyBusObject* pbo = new _ProxyBusObject(bus, strService.c_str(), strPath.c_str(), sessionId);
        if (NULL == pbo) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mProxyBusObject = new qcc::ManagedObj<_ProxyBusObject>(pbo);
        if (NULL == _mProxyBusObject) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _proxyBusObject = &(**_mProxyBusObject);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

ProxyBusObject::ProxyBusObject(BusAttachment ^ bus, const ajn::ProxyBusObject* proxyBusObject)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == bus) {
            status = ER_BAD_ARG_1;
            break;
        }
        if (NULL == proxyBusObject) {
            status = ER_BAD_ARG_2;
            break;
        }
        _ProxyBusObject* pbo = new _ProxyBusObject(bus, proxyBusObject);
        if (NULL == pbo) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mProxyBusObject = new qcc::ManagedObj<_ProxyBusObject>(pbo);
        if (NULL == _mProxyBusObject) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _proxyBusObject = &(**_mProxyBusObject);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

ProxyBusObject::ProxyBusObject(BusAttachment ^ bus, const ajn::_ProxyBusObject* proxyBusObject)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == bus) {
            status = ER_BAD_ARG_1;
            break;
        }
        if (NULL == proxyBusObject) {
            status = ER_BAD_ARG_2;
            break;
        }
        _ProxyBusObject* pbo = new _ProxyBusObject(bus, proxyBusObject);
        if (NULL == pbo) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mProxyBusObject = new qcc::ManagedObj<_ProxyBusObject>(pbo);
        if (NULL == _mProxyBusObject) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _proxyBusObject = &(**_mProxyBusObject);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

}

ProxyBusObject::ProxyBusObject(BusAttachment ^ bus, const qcc::ManagedObj<_ProxyBusObject>* proxyBusObject)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == bus) {
            status = ER_BAD_ARG_1;
            break;
        }
        if (NULL == proxyBusObject) {
            status = ER_BAD_ARG_2;
            break;
        }
        _mProxyBusObject = new qcc::ManagedObj<_ProxyBusObject>(*proxyBusObject);
        if (NULL == _mProxyBusObject) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _proxyBusObject = &(**_mProxyBusObject);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

ProxyBusObject::~ProxyBusObject()
{
    if (NULL != _mProxyBusObject) {
        delete _mProxyBusObject;
        _mProxyBusObject = NULL;
        _proxyBusObject = NULL;
    }
}

Windows::Foundation::IAsyncOperation<IntrospectRemoteObjectResult ^> ^ ProxyBusObject::IntrospectRemoteObjectAsync(Platform::Object ^ context)
{
    ::QStatus status = ER_OK;
    Windows::Foundation::IAsyncOperation<IntrospectRemoteObjectResult ^> ^ result = nullptr;

    while (true) {
        ajn::ProxyBusObject::Listener* listener = _proxyBusObject->_proxyBusObjectListener;
        ajn::ProxyBusObject::Listener::IntrospectCB cb = _proxyBusObject->_proxyBusObjectListener->GetProxyListenerIntrospectCBHandler();
        IntrospectRemoteObjectResult ^ introspectRemoteObjectResult = ref new IntrospectRemoteObjectResult(this, context);
        if (nullptr == introspectRemoteObjectResult) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = ((ajn::ProxyBusObject*)*_proxyBusObject)->IntrospectRemoteObjectAsync(listener, cb, (void*)introspectRemoteObjectResult);
        if (ER_OK != status) {
            break;
        }
        result = concurrency::create_async([this, introspectRemoteObjectResult]()->IntrospectRemoteObjectResult ^
                                           {
                                               introspectRemoteObjectResult->Wait();
                                               return introspectRemoteObjectResult;
                                           });
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Windows::Foundation::IAsyncOperation<GetPropertyResult ^> ^ ProxyBusObject::GetPropertyAsync(
    Platform::String ^ iface,
    Platform::String ^ property,
    Platform::Object ^ context,
    uint32_t timeout)
{
    ::QStatus status = ER_OK;
    Windows::Foundation::IAsyncOperation<GetPropertyResult ^> ^ result = nullptr;

    while (true) {
        if (nullptr == iface) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strIface = PlatformToMultibyteString(iface);
        if (strIface.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == property) {
            status = ER_BAD_ARG_2;
            break;
        }
        qcc::String strProperty = PlatformToMultibyteString(property);
        if (strProperty.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::ProxyBusObject::Listener* listener = _proxyBusObject->_proxyBusObjectListener;
        ajn::ProxyBusObject::Listener::GetPropertyCB cb = _proxyBusObject->_proxyBusObjectListener->GetProxyListenerGetPropertyCBHandler();
        GetPropertyResult ^ getPropertyResult = ref new GetPropertyResult(this, context);
        if (nullptr == getPropertyResult) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = ((ajn::ProxyBusObject*)*_proxyBusObject)->GetPropertyAsync(
            strIface.c_str(),
            strProperty.c_str(),
            listener,
            cb,
            (void*)getPropertyResult,
            timeout);
        if (ER_OK != status) {
            break;
        }
        result = concurrency::create_async([this, getPropertyResult]()->GetPropertyResult ^
                                           {
                                               getPropertyResult->Wait();
                                               return getPropertyResult;
                                           });
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Windows::Foundation::IAsyncOperation<GetAllPropertiesResult ^> ^ ProxyBusObject::GetAllPropertiesAsync(
    Platform::String ^ iface,
    Platform::Object ^ context,
    uint32_t timeout)
{
    ::QStatus status = ER_OK;
    Windows::Foundation::IAsyncOperation<GetAllPropertiesResult ^> ^ result = nullptr;

    while (true) {
        if (nullptr == iface) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strIface = PlatformToMultibyteString(iface);
        if (strIface.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::ProxyBusObject::Listener* listener = _proxyBusObject->_proxyBusObjectListener;
        ajn::ProxyBusObject::Listener::GetPropertyCB cb = _proxyBusObject->_proxyBusObjectListener->GetProxyListenerGetAllPropertiesCBHandler();
        GetAllPropertiesResult ^ getAllPropertiesResult = ref new GetAllPropertiesResult(this, context);
        if (nullptr == getAllPropertiesResult) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = ((ajn::ProxyBusObject*)*_proxyBusObject)->GetAllPropertiesAsync(strIface.c_str(),
                                                                                 listener,
                                                                                 cb,
                                                                                 (void*)getAllPropertiesResult,
                                                                                 timeout);
        if (ER_OK != status) {
            break;
        }
        result = concurrency::create_async([this, getAllPropertiesResult]()->GetAllPropertiesResult ^
                                           {
                                               getAllPropertiesResult->Wait();
                                               return getAllPropertiesResult;
                                           });
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Windows::Foundation::IAsyncOperation<SetPropertyResult ^> ^ ProxyBusObject::SetPropertyAsync(
    Platform::String ^ iface,
    Platform::String ^ property,
    MsgArg ^ value,
    Platform::Object ^ context,
    uint32_t timeout)
{
    ::QStatus status = ER_OK;
    Windows::Foundation::IAsyncOperation<SetPropertyResult ^> ^ result = nullptr;

    while (true) {
        if (nullptr == iface) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strIface = PlatformToMultibyteString(iface);
        if (strIface.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == property) {
            status = ER_BAD_ARG_2;
            break;
        }
        qcc::String strProperty = PlatformToMultibyteString(property);
        if (strProperty.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == value) {
            status = ER_BAD_ARG_3;
            break;
        }
        ajn::MsgArg* arg = value->_msgArg;
        ajn::ProxyBusObject::Listener* listener = _proxyBusObject->_proxyBusObjectListener;
        ajn::ProxyBusObject::Listener::SetPropertyCB cb = _proxyBusObject->_proxyBusObjectListener->GetProxyListenerSetPropertyCBHandler();
        SetPropertyResult ^ setPropertyResult = ref new SetPropertyResult(this, context);
        if (nullptr == setPropertyResult) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = ((ajn::ProxyBusObject*)*_proxyBusObject)->SetPropertyAsync(strIface.c_str(),
                                                                            strProperty.c_str(),
                                                                            *arg,
                                                                            listener,
                                                                            cb,
                                                                            (void*)setPropertyResult,
                                                                            timeout);
        if (ER_OK != status) {
            break;
        }
        result = concurrency::create_async([this, setPropertyResult]()->SetPropertyResult ^
                                           {
                                               setPropertyResult->Wait();
                                               return setPropertyResult;
                                           });
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

uint32_t ProxyBusObject::GetInterfaces(Platform::WriteOnlyArray<InterfaceDescription ^> ^ ifaces)
{
    ::QStatus status = ER_OK;
    ajn::InterfaceDescription** idescArray = NULL;
    size_t result = -1;

    while (true) {
        if (nullptr != ifaces && ifaces->Length > 0) {
            idescArray = new ajn::InterfaceDescription *[ifaces->Length];
            if (NULL == idescArray) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
        }
        result = ((ajn::ProxyBusObject*)*_proxyBusObject)->GetInterfaces((const ajn::InterfaceDescription**)idescArray, ifaces->Length);
        if (result > 0 && NULL != idescArray) {
            for (int i = 0; i < result; i++) {
                InterfaceDescription ^ id = ref new InterfaceDescription(idescArray[i]);
                if (nullptr == id) {
                    status = ER_OUT_OF_MEMORY;
                    break;
                }
                ifaces[i] = id;
            }
        }
        break;
    }

    if (NULL != idescArray) {
        delete [] idescArray;
        idescArray = NULL;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

InterfaceDescription ^ ProxyBusObject::GetInterface(Platform::String ^ iface)
{
    ::QStatus status = ER_OK;
    InterfaceDescription ^ result = nullptr;

    while (true) {
        if (nullptr == iface) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strIface = PlatformToMultibyteString(iface);
        if (strIface.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        const ajn::InterfaceDescription* ret = ((ajn::ProxyBusObject*)*_proxyBusObject)->GetInterface(strIface.c_str());
        if (NULL != ret) {
            result = ref new InterfaceDescription(ret);
            if (nullptr == result) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

bool ProxyBusObject::ImplementsInterface(Platform::String ^ iface)
{
    ::QStatus status = ER_OK;
    bool result = false;

    while (true) {
        if (nullptr == iface) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strIface = PlatformToMultibyteString(iface);
        if (strIface.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        result = ((ajn::ProxyBusObject*)*_proxyBusObject)->ImplementsInterface(strIface.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void ProxyBusObject::AddInterface(InterfaceDescription ^ iface)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == iface) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::InterfaceDescription* id = *(iface->_interfaceDescr);
        status = ((ajn::ProxyBusObject*)*_proxyBusObject)->AddInterface(*id);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void ProxyBusObject::AddInterfaceWithString(Platform::String ^ name)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == name) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strName = PlatformToMultibyteString(name);
        if (strName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = ((ajn::ProxyBusObject*)*_proxyBusObject)->AddInterface(strName.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

uint32_t ProxyBusObject::GetChildren(Platform::WriteOnlyArray<ProxyBusObject ^> ^ children)
{
    ::QStatus status = ER_OK;
    ajn::_ProxyBusObject** pboArray = NULL;
    size_t result = -1;

    while (true) {
        if (nullptr != children && children->Length > 0) {
            pboArray = new ajn::_ProxyBusObject *[children->Length];
            if (NULL == pboArray) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
        }
        result = ((ajn::ProxyBusObject*)*_proxyBusObject)->GetManagedChildren(pboArray, children->Length);
        if (result > 0 && NULL != pboArray) {
            for (int i = 0; i < result; i++) {
                ProxyBusObject ^ pbo = ref new ProxyBusObject(Bus, pboArray[i]);
                if (nullptr == pbo) {
                    status = ER_OUT_OF_MEMORY;
                    break;
                }               else {
                    children[i] = pbo;
                    delete pboArray[i];
                    pboArray[i] = NULL;
                }
            }
        }
        break;
    }

    if (NULL != pboArray) {
        delete [] pboArray;
        pboArray = NULL;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

ProxyBusObject ^ ProxyBusObject::GetChild(Platform::String ^ path)
{
    ::QStatus status = ER_OK;
    ProxyBusObject ^ result = nullptr;

    while (true) {
        if (nullptr == path) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strPath = PlatformToMultibyteString(path);
        if (strPath.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::_ProxyBusObject* ret = (ajn::_ProxyBusObject*)((ajn::ProxyBusObject*)*_proxyBusObject)->GetManagedChild(strPath.c_str());
        if (NULL != ret) {
            result = ref new ProxyBusObject(Bus, ret);
            if (nullptr == result) {
                status = ER_OUT_OF_MEMORY;
                break;
            } else {
                delete ret;
                ret = NULL;
            }
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void ProxyBusObject::AddChild(ProxyBusObject ^ child)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == child) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::ProxyBusObject* pbo = (ajn::ProxyBusObject*)*(child->_proxyBusObject);
        status = ((ajn::ProxyBusObject*)*_proxyBusObject)->AddChild(*pbo);
        if (ER_OK == status) {
            // Often the path will be rewritten which will allocate a new ProxyBusObject
            qcc::String childPath = pbo->GetPath();
            size_t idx = childPath.size() + 1;
            size_t end = childPath.find_first_of('/', idx);
            qcc::String item = childPath.substr(0, (qcc::String::npos == end) ? end : end - 1);
            Platform::String ^ strItem = MultibyteToPlatformString(item.c_str());
            if (nullptr == strItem) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            ProxyBusObject ^ tempChild = this->GetChild(strItem);
            if (nullptr == tempChild) {
                status = ER_FAIL;
                break;
            }
            pbo = (ajn::ProxyBusObject*)*(tempChild->_proxyBusObject);
            AddObjectReference2(&(_proxyBusObject->_mutex), (void*)pbo, tempChild, &(_proxyBusObject->_childObjectMap));
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void ProxyBusObject::RemoveChild(Platform::String ^ path)
{
    ::QStatus status = ER_OK;

    while (true) {
        ProxyBusObject ^ child = GetChild(path);
        if (nullptr == child) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strPath = PlatformToMultibyteString(path);
        if (strPath.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::ProxyBusObject* pbo = (ajn::ProxyBusObject*)*(child->_proxyBusObject);
        status = ((ajn::ProxyBusObject*)*_proxyBusObject)->RemoveChild(strPath.c_str());
        if (ER_OK == status) {
            RemoveObjectReference2(&(_proxyBusObject->_mutex), (void*)pbo, &(_proxyBusObject->_childObjectMap));
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Windows::Foundation::IAsyncOperation<MethodCallResult ^> ^ ProxyBusObject::MethodCallAsync(InterfaceMember ^ method,
                                                                                           const Platform::Array<MsgArg ^> ^ args,
                                                                                           Platform::Object ^ context,
                                                                                           uint32_t timeout,
                                                                                           uint8_t flags)
{
    ::QStatus status = ER_OK;
    ajn::MsgArg* msgScratch = NULL;
    Windows::Foundation::IAsyncOperation<MethodCallResult ^> ^ result = nullptr;

    while (true) {
        if (nullptr == method) {
            status = ER_BAD_ARG_1;
            break;
        }
        size_t argsCount = 0;
        if (nullptr != args & args->Length > 0) {
            argsCount = args->Length;
            msgScratch = new ajn::MsgArg[argsCount];
            if (NULL == msgScratch) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            for (int i = 0; i < argsCount; i++) {
                if (nullptr == args[i]) {
                    status = ER_BUFFER_TOO_SMALL;
                    break;
                }
                ajn::MsgArg* arg = args[i]->_msgArg;
                msgScratch[i] = *arg;
            }
            if (ER_OK != status) {
                break;
            }
        }
        MethodCallResult ^ methodCallResult = ref new MethodCallResult(this, context);
        if (nullptr == methodCallResult) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::InterfaceDescription::Member* imethod = *(method->_member);
        ajn::MessageReceiver* mreceiver = Receiver->_receiver;
        ajn::MessageReceiver::ReplyHandler handler = static_cast<ajn::MessageReceiver::ReplyHandler>(&_ProxyBusObject::ReplyHandler);
        status = ((ajn::ProxyBusObject*)*_proxyBusObject)->MethodCallAsync(*imethod,
                                                                           mreceiver,
                                                                           handler,
                                                                           msgScratch,
                                                                           argsCount,
                                                                           (void*)methodCallResult,
                                                                           timeout,
                                                                           flags);
        if (ER_OK != status) {
            break;
        }
        result = concurrency::create_async([this, methodCallResult]()->MethodCallResult ^
                                           {
                                               methodCallResult->Wait();
                                               return methodCallResult;
                                           });
        break;
    }

    if (NULL != msgScratch) {
        delete [] msgScratch;
        msgScratch = NULL;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Windows::Foundation::IAsyncOperation<MethodCallResult ^> ^ ProxyBusObject::MethodCallAsync(Platform::String ^ ifaceName,
                                                                                           Platform::String ^ methodName,
                                                                                           const Platform::Array<MsgArg ^> ^ args,
                                                                                           Platform::Object ^ context,
                                                                                           uint32_t timeout,
                                                                                           uint8_t flags)
{
    ::QStatus status = ER_OK;
    ajn::MsgArg* msgScratch = NULL;
    Windows::Foundation::IAsyncOperation<MethodCallResult ^> ^ result = nullptr;

    while (true) {
        if (nullptr == ifaceName) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strIfaceName = PlatformToMultibyteString(ifaceName);
        if (strIfaceName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == methodName) {
            status = ER_BAD_ARG_2;
            break;
        }
        qcc::String strMethodName = PlatformToMultibyteString(methodName);
        if (strMethodName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        size_t argsCount = 0;
        if (nullptr != args & args->Length > 0) {
            argsCount = args->Length;
            msgScratch = new ajn::MsgArg[argsCount];
            if (NULL == msgScratch) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            for (int i = 0; i < argsCount; i++) {
                if (nullptr == args[i]) {
                    status = ER_BUFFER_TOO_SMALL;
                    break;
                }
                ajn::MsgArg* arg = args[i]->_msgArg;
                msgScratch[i] = *arg;
            }
            if (ER_OK != status) {
                break;
            }
        }
        MethodCallResult ^ methodCallResult = ref new MethodCallResult(this, context);
        if (nullptr == methodCallResult) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::MessageReceiver* mreceiver = Receiver->_receiver;
        ajn::MessageReceiver::ReplyHandler handler = static_cast<ajn::MessageReceiver::ReplyHandler>(&_ProxyBusObject::ReplyHandler);
        status = ((ajn::ProxyBusObject*)*_proxyBusObject)->MethodCallAsync(strIfaceName.c_str(),
                                                                           strMethodName.c_str(),
                                                                           mreceiver,
                                                                           handler,
                                                                           msgScratch,
                                                                           argsCount,
                                                                           (void*)methodCallResult,
                                                                           timeout,
                                                                           flags);
        if (ER_OK != status) {
            break;
        }
        result = concurrency::create_async([this, methodCallResult]()->MethodCallResult ^
                                           {
                                               methodCallResult->Wait();
                                               return methodCallResult;
                                           });
        break;
    }

    if (NULL != msgScratch) {
        delete [] msgScratch;
        msgScratch = NULL;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void ProxyBusObject::ParseXml(Platform::String ^ xml, Platform::String ^ identifier)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == xml) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strXml = PlatformToMultibyteString(xml);
        if (strXml.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == identifier) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strIdentifier = PlatformToMultibyteString(identifier);
        if (strIdentifier.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = ((ajn::ProxyBusObject*)*_proxyBusObject)->ParseXml(strXml.c_str(), strIdentifier.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void ProxyBusObject::SecureConnectionAsync(bool forceAuth)
{
    ::QStatus status = ((ajn::ProxyBusObject*)*_proxyBusObject)->SecureConnectionAsync(forceAuth);

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

bool ProxyBusObject::IsValid()
{
    return ((ajn::ProxyBusObject*)*_proxyBusObject)->IsValid();
}

BusAttachment ^ ProxyBusObject::Bus::get()
{
    return _proxyBusObject->_eventsAndProperties->Bus;
}

Platform::String ^ ProxyBusObject::Name::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _proxyBusObject->_eventsAndProperties->Name) {
            qcc::String strName = ((ajn::ProxyBusObject*)*_proxyBusObject)->GetServiceName();
            result = MultibyteToPlatformString(strName.c_str());
            if (nullptr == result && !strName.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _proxyBusObject->_eventsAndProperties->Name = result;
        } else {
            result = _proxyBusObject->_eventsAndProperties->Name;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ ProxyBusObject::Path::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _proxyBusObject->_eventsAndProperties->Path) {
            qcc::String strPath = ((ajn::ProxyBusObject*)*_proxyBusObject)->GetPath();
            result = MultibyteToPlatformString(strPath.c_str());
            if (nullptr == result && !strPath.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _proxyBusObject->_eventsAndProperties->Path = result;
        } else {
            result = _proxyBusObject->_eventsAndProperties->Path;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

MessageReceiver ^ ProxyBusObject::Receiver::get()
{
    return _proxyBusObject->_eventsAndProperties->Receiver;
}

ajn::SessionId ProxyBusObject::SessionId::get()
{
    ::QStatus status = ER_OK;
    ajn::SessionId result = (ajn::SessionId)-1;

    while (true) {
        if ((ajn::SessionId)-1 == _proxyBusObject->_eventsAndProperties->SessionId) {
            result = ((ajn::ProxyBusObject*)*_proxyBusObject)->GetSessionId();
            _proxyBusObject->_eventsAndProperties->SessionId = result;
        } else {
            result = _proxyBusObject->_eventsAndProperties->SessionId;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

_ProxyBusObject::_ProxyBusObject(BusAttachment ^ b, const ajn::ProxyBusObject* proxyBusObject)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __ProxyBusObject();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _proxyBusObjectListener = new _ProxyBusObjectListener(this);
        if (NULL == _proxyBusObjectListener) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        AllJoyn::MessageReceiver ^ receiver =  ref new AllJoyn::MessageReceiver(b);
        if (nullptr == receiver) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mProxyBusObject = new ajn::_ProxyBusObject(*proxyBusObject);
        if (NULL == _mProxyBusObject) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _proxyBusObject = &(**_mProxyBusObject);
        _eventsAndProperties->Receiver = receiver;
        _eventsAndProperties->Bus = b;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_ProxyBusObject::_ProxyBusObject(BusAttachment ^ b, const ajn::_ProxyBusObject* proxyBusObject)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __ProxyBusObject();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _proxyBusObjectListener = new _ProxyBusObjectListener(this);
        if (NULL == _proxyBusObjectListener) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        AllJoyn::MessageReceiver ^ receiver =  ref new AllJoyn::MessageReceiver(b);
        if (nullptr == receiver) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mProxyBusObject = new ajn::_ProxyBusObject(*proxyBusObject);
        if (NULL == _mProxyBusObject) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _proxyBusObject = &(**_mProxyBusObject);
        _eventsAndProperties->Receiver = receiver;
        _eventsAndProperties->Bus = b;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_ProxyBusObject::_ProxyBusObject(BusAttachment ^ b, const char* service, const char* path, ajn::SessionId sessionId)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __ProxyBusObject();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _proxyBusObjectListener = new _ProxyBusObjectListener(this);
        if (NULL == _proxyBusObjectListener) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        AllJoyn::MessageReceiver ^ receiver =  ref new AllJoyn::MessageReceiver(b);
        if (nullptr == receiver) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::ProxyBusObject pbo(*(b->_busAttachment), service, path, sessionId);
        _mProxyBusObject = new ajn::_ProxyBusObject(pbo);
        if (NULL == _mProxyBusObject) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _proxyBusObject = &(**_mProxyBusObject);
        _eventsAndProperties->Receiver = receiver;
        _eventsAndProperties->Bus = b;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_ProxyBusObject::~_ProxyBusObject()
{
    _eventsAndProperties = nullptr;
    _mReceiver = NULL;
    if (NULL != _proxyBusObjectListener) {
        delete _proxyBusObjectListener;
        _proxyBusObjectListener = NULL;
    }
    if (NULL != _mProxyBusObject) {
        delete _mProxyBusObject;
        _mProxyBusObject = NULL;
    }
    _proxyBusObject = NULL;
    ClearObjectMap(&(this->_mutex), &(this->_childObjectMap));
}

void _ProxyBusObject::ReplyHandler(ajn::Message& msg, void* context)
{
    ::QStatus status = ER_OK;
    MethodCallResult ^ methodCallResult = reinterpret_cast<MethodCallResult ^>(context);

    try {
        while (true) {
            Message ^ message = ref new Message(&msg);
            if (nullptr == message) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            methodCallResult->Message = message;
            break;
        }

        if (ER_OK != status) {
            QCC_THROW_EXCEPTION(status);
        }

        methodCallResult->Complete();
    } catch (Platform::Exception ^ pe) {
        // Forward Platform::Exception
        methodCallResult->_exception = pe;
        methodCallResult->Complete();
    } catch (std::exception& e) {
        // Forward std::exception
        methodCallResult->_stdException = new std::exception(e);
        methodCallResult->Complete();
    }
}

_ProxyBusObject::operator ajn::ProxyBusObject * ()
{
    return _proxyBusObject;
}

_ProxyBusObject::operator ajn::_ProxyBusObject * ()
{
    return _mProxyBusObject;
}

_ProxyBusObjectListener::_ProxyBusObjectListener(_ProxyBusObject* proxybusobject)
    : _proxyBusObject(proxybusobject)
{
}

_ProxyBusObjectListener::~_ProxyBusObjectListener()
{
    _proxyBusObject = nullptr;
}

ajn::ProxyBusObject::Listener::IntrospectCB _ProxyBusObjectListener::GetProxyListenerIntrospectCBHandler()
{
    return static_cast<ajn::ProxyBusObject::Listener::IntrospectCB>(&AllJoyn::_ProxyBusObjectListener::IntrospectCB);
}

ajn::ProxyBusObject::Listener::GetPropertyCB _ProxyBusObjectListener::GetProxyListenerGetPropertyCBHandler()
{
    return static_cast<ajn::ProxyBusObject::Listener::GetPropertyCB>(&AllJoyn::_ProxyBusObjectListener::GetPropertyCB);
}

ajn::ProxyBusObject::Listener::GetAllPropertiesCB _ProxyBusObjectListener::GetProxyListenerGetAllPropertiesCBHandler()
{
    return static_cast<ajn::ProxyBusObject::Listener::GetAllPropertiesCB>(&AllJoyn::_ProxyBusObjectListener::GetAllPropertiesCB);
}

ajn::ProxyBusObject::Listener::SetPropertyCB _ProxyBusObjectListener::GetProxyListenerSetPropertyCBHandler()
{
    return static_cast<ajn::ProxyBusObject::Listener::SetPropertyCB>(&AllJoyn::_ProxyBusObjectListener::SetPropertyCB);
}

void _ProxyBusObjectListener::IntrospectCB(::QStatus s, ajn::ProxyBusObject* obj, void* context)
{
    ::QStatus status = ER_OK;
    IntrospectRemoteObjectResult ^ introspectRemoteObjectResult = reinterpret_cast<IntrospectRemoteObjectResult ^>(context);
    introspectRemoteObjectResult->Status = (AllJoyn::QStatus)s;
    introspectRemoteObjectResult->Complete();
}

void _ProxyBusObjectListener::GetPropertyCB(::QStatus s, ajn::ProxyBusObject* obj, const ajn::MsgArg& value, void* context)
{
    ::QStatus status = ER_OK;
    GetPropertyResult ^ getPropertyResult = reinterpret_cast<GetPropertyResult ^>(context);

    try {
        while (true) {
            MsgArg ^ arg = ref new MsgArg(&value);
            if (nullptr == arg) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            getPropertyResult->Value = arg;
            getPropertyResult->Status = (AllJoyn::QStatus)s;
            break;
        }

        if (ER_OK != status) {
            QCC_THROW_EXCEPTION(status);
        }

        getPropertyResult->Complete();
    } catch (Platform::Exception ^ pe) {
        // Forward Platform::Exception
        getPropertyResult->_exception = pe;
        getPropertyResult->Complete();
    } catch (std::exception& e) {
        // Forward std::exception
        getPropertyResult->_stdException = new std::exception(e);
        getPropertyResult->Complete();
    }
}

void _ProxyBusObjectListener::GetAllPropertiesCB(::QStatus s, ajn::ProxyBusObject* obj, const ajn::MsgArg& value, void* context)
{
    ::QStatus status = ER_OK;
    GetAllPropertiesResult ^ getAllPropertiesResult = reinterpret_cast<GetAllPropertiesResult ^>(context);

    try {
        while (true) {
            MsgArg ^ arg = ref new MsgArg(&value);
            if (nullptr == arg) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            getAllPropertiesResult->Value = arg;
            getAllPropertiesResult->Status = (AllJoyn::QStatus)s;
            break;
        }

        if (ER_OK != status) {
            QCC_THROW_EXCEPTION(status);
        }

        getAllPropertiesResult->Complete();
    } catch (Platform::Exception ^ pe) {
        // Forward Platform::Exception
        getAllPropertiesResult->_exception = pe;
        getAllPropertiesResult->Complete();
    } catch (std::exception& e) {
        // Forward std::exception
        getAllPropertiesResult->_stdException = new std::exception(e);
        getAllPropertiesResult->Complete();
    }
}

void _ProxyBusObjectListener::SetPropertyCB(::QStatus s, ajn::ProxyBusObject* obj, void* context)
{
    ::QStatus status = ER_OK;
    SetPropertyResult ^ setPropertyResult = reinterpret_cast<SetPropertyResult ^>(context);
    setPropertyResult->Complete();
}

__ProxyBusObject::__ProxyBusObject()
{
    Bus = nullptr;
    Name = nullptr;
    Path = nullptr;
    Receiver = nullptr;
    SessionId = (ajn::SessionId)-1;
}

__ProxyBusObject::~__ProxyBusObject()
{
    Bus = nullptr;
    Name = nullptr;
    Path = nullptr;
    Receiver = nullptr;
    SessionId = (ajn::SessionId)-1;
}

}
