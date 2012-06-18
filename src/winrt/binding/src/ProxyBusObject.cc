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
        ajn::BusAttachment* ba = bus->_busAttachment;
        if (nullptr == service) {
            status = ER_BAD_ARG_2;
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
        _ProxyBusObject* pbo = new _ProxyBusObject(bus, *ba, strService.c_str(), strPath.c_str(), sessionId);
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

ProxyBusObject::ProxyBusObject(BusAttachment ^ bus, void* proxybusobject)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == bus) {
            status = ER_BAD_ARG_1;
            break;
        }
        if (NULL == proxybusobject) {
            status = ER_BAD_ARG_2;
            break;
        }
        ajn::ProxyBusObject* proxyobj = reinterpret_cast<_ProxyBusObject*>(proxybusobject);
        _ProxyBusObject* pbo = new _ProxyBusObject(bus, proxyobj);
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

ProxyBusObject::ProxyBusObject(void* proxybusobject, bool isManaged)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == proxybusobject) {
            status = ER_BAD_ARG_1;
            break;
        }
        if (!isManaged) {
            status = ER_FAIL;
            break;
        }
        qcc::ManagedObj<_ProxyBusObject>* mpbo = reinterpret_cast<qcc::ManagedObj<_ProxyBusObject>*>(proxybusobject);
        _mProxyBusObject = new qcc::ManagedObj<_ProxyBusObject>(*mpbo);
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

void ProxyBusObject::IntrospectRemoteObjectAsync(Platform::Object ^ context)
{
    ::QStatus status = ER_OK;

    while (true) {
        ajn::ProxyBusObject::Listener* listener = _proxyBusObject->_proxyBusObjectListener;
        ajn::ProxyBusObject::Listener::IntrospectCB cb = _proxyBusObject->_proxyBusObjectListener->GetProxyListenerCBHandler();
        IntrospectCBCtx* introspectCBCtx = new IntrospectCBCtx(this, context);
        if (NULL == introspectCBCtx) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = _proxyBusObject->IntrospectRemoteObjectAsync(listener, cb, (void*)introspectCBCtx);
        if (ER_OK != status) {
            if (NULL != introspectCBCtx) {
                delete introspectCBCtx;
                introspectCBCtx = NULL;
            }
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void ProxyBusObject::GetProperty(Platform::String ^ iface, Platform::String ^ property, Platform::WriteOnlyArray<MsgArg ^> ^ value)
{
    ::QStatus status = ER_OK;

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
        if (nullptr == value || value->Length != 1) {
            status = ER_BAD_ARG_3;
            break;
        }
        ajn::MsgArg arg;
        status = _proxyBusObject->GetProperty(strIface.c_str(), strProperty.c_str(), arg);
        if (ER_OK == status) {
            MsgArg ^ msgArgOut = ref new MsgArg((void*)&arg, false);
            if (nullptr == msgArgOut) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            value[0] = msgArgOut;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void ProxyBusObject::GetAllProperties(Platform::String ^ iface, Platform::WriteOnlyArray<MsgArg ^> ^ values)
{
    ::QStatus status = ER_OK;

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
        if (nullptr == values || values->Length != 1) {
            status = ER_BAD_ARG_2;
            break;
        }
        ajn::MsgArg arg;
        status = _proxyBusObject->GetAllProperties(strIface.c_str(), arg);
        if (ER_OK == status) {
            MsgArg ^ msgArgOut = ref new MsgArg((void*)&arg, false);
            if (nullptr == msgArgOut) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            values[0] = msgArgOut;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void ProxyBusObject::SetProperty(Platform::String ^ iface, Platform::String ^ property, MsgArg ^ value)
{
    ::QStatus status = ER_OK;

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
        status = _proxyBusObject->SetProperty(strIface.c_str(), strProperty.c_str(), *arg);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
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
        result = _proxyBusObject->GetInterfaces((const ajn::InterfaceDescription**)idescArray, ifaces->Length);
        if (result > 0 && NULL != idescArray) {
            for (int i = 0; i < result; i++) {
                InterfaceDescription ^ id = ref new InterfaceDescription((void*)idescArray[i], false);
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
        const ajn::InterfaceDescription* ret = _proxyBusObject->GetInterface(strIface.c_str());
        if (NULL != ret) {
            result = ref new InterfaceDescription((void*)ret, false);
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
        result = _proxyBusObject->ImplementsInterface(strIface.c_str());
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
        status = _proxyBusObject->AddInterface(*id);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void ProxyBusObject::AddInterface(Platform::String ^ name)
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
        status = _proxyBusObject->AddInterface(strName.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

uint32_t ProxyBusObject::GetChildren(Platform::WriteOnlyArray<ProxyBusObject ^> ^ children)
{
    ::QStatus status = ER_OK;
    ajn::ProxyBusObject** pboArray = NULL;
    size_t result = -1;

    while (true) {
        if (nullptr != children && children->Length < 1) {
            status = ER_BAD_ARG_1;
            break;
        } else if (nullptr != children) {
            pboArray = new ajn::ProxyBusObject *[children->Length];
            if (NULL == pboArray) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
        }
        result = _proxyBusObject->GetChildren(pboArray, children->Length);
        if (result > 0 && NULL != pboArray) {
            for (int i = 0; i < result; i++) {
                ProxyBusObject ^ pbo = ref new ProxyBusObject(Bus, (void*)pboArray[i]);
                if (nullptr == pbo) {
                    status = ER_OUT_OF_MEMORY;
                    break;
                }
                children[i] = pbo;
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
        ajn::ProxyBusObject* ret = _proxyBusObject->GetChild(strPath.c_str());
        if (NULL != ret) {
            result = ref new ProxyBusObject(Bus, (void*)ret);
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

void ProxyBusObject::AddChild(ProxyBusObject ^ child)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == child) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::ProxyBusObject* pbo = child->_proxyBusObject;
        status = _proxyBusObject->AddChild(*pbo);
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
            ProxyBusObject ^ tempChild = child->GetChild(strItem);
            if (nullptr == tempChild) {
                status = ER_FAIL;
                break;
            }
            pbo = tempChild->_proxyBusObject;
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
        ajn::ProxyBusObject* pbo = child->_proxyBusObject;
        status = _proxyBusObject->RemoveChild(strPath.c_str());
        if (ER_OK == status) {
            RemoveObjectReference2(&(_proxyBusObject->_mutex), (void*)pbo, &(_proxyBusObject->_childObjectMap));
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void ProxyBusObject::MethodCallAsync(InterfaceMember ^ method,
                                     MessageReceiver ^ receiver,
                                     const Platform::Array<MsgArg ^> ^ args,
                                     Platform::Object ^ context,
                                     uint32_t timeout,
                                     uint8_t flags)
{
    ::QStatus status = ER_OK;
    ajn::MsgArg* msgScratch = NULL;

    while (true) {
        if (nullptr == method) {
            status = ER_BAD_ARG_1;
            break;
        }
        if (nullptr == receiver) {
            status = ER_BAD_ARG_2;
            break;
        }
        if (!Bus->IsSameBusAttachment(receiver->_receiver->Bus)) {
            status = ER_BAD_ARG_2;
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
                MsgArg ^ tempArg = args[i];
                ajn::MsgArg* arg = tempArg->_msgArg;
                msgScratch[i] = *arg;
            }
        }
        ProxyMessageReceiverCtx* proxyCtx = new ProxyMessageReceiverCtx(this, receiver, context);
        if (nullptr == proxyCtx) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::InterfaceDescription::Member* imethod = *(method->_member);
        ajn::MessageReceiver* mreceiver = Receiver->_receiver;
        ajn::MessageReceiver::ReplyHandler handler = static_cast<ajn::MessageReceiver::ReplyHandler>(&_ProxyBusObject::MessageReceiverProxyReplyHandler);
        status = _proxyBusObject->MethodCallAsync(*imethod, mreceiver, handler, msgScratch, argsCount, (void*)proxyCtx, timeout, flags);
        break;
    }

    if (NULL != msgScratch) {
        delete [] msgScratch;
        msgScratch = NULL;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void ProxyBusObject::MethodCallAsync(Platform::String ^ ifaceName,
                                     Platform::String ^ methodName,
                                     MessageReceiver ^ receiver,
                                     const Platform::Array<MsgArg ^> ^ args,
                                     Platform::Object ^ context,
                                     uint32_t timeout,
                                     uint8_t flags)
{
    ::QStatus status = ER_OK;
    ajn::MsgArg* msgScratch = NULL;

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
        if (nullptr == receiver) {
            status = ER_BAD_ARG_3;
            break;
        }
        if (!Bus->IsSameBusAttachment(receiver->_receiver->Bus)) {
            status = ER_BAD_ARG_3;
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
                MsgArg ^ tempArg = args[i];
                ajn::MsgArg* arg = tempArg->_msgArg;
                msgScratch[i] = *arg;
            }
        }
        ProxyMessageReceiverCtx* proxyCtx = new ProxyMessageReceiverCtx(this, receiver, context);
        if (nullptr == proxyCtx) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::MessageReceiver* mreceiver = Receiver->_receiver;
        ajn::MessageReceiver::ReplyHandler handler = static_cast<ajn::MessageReceiver::ReplyHandler>(&_ProxyBusObject::MessageReceiverProxyReplyHandler);
        status = _proxyBusObject->MethodCallAsync(strIfaceName.c_str(), strMethodName.c_str(), mreceiver,
                                                  handler, msgScratch, argsCount, (void*)proxyCtx, timeout, flags);
        break;
    }

    if (NULL != msgScratch) {
        delete [] msgScratch;
        msgScratch = NULL;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
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
        status = _proxyBusObject->ParseXml(strXml.c_str(), strIdentifier.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void ProxyBusObject::SecureConnectionAsync(bool forceAuth)
{
    ::QStatus status = _proxyBusObject->SecureConnectionAsync(forceAuth);

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

bool ProxyBusObject::IsValid()
{
    return _proxyBusObject->IsValid();
}

Windows::Foundation::EventRegistrationToken ProxyBusObject::IntrospectRemoteObject::add(IntrospectRemoteObjectHandler ^ handler)
{
    return _proxyBusObject->_eventsAndProperties->IntrospectRemoteObject::add(handler);
}

void ProxyBusObject::IntrospectRemoteObject::remove(Windows::Foundation::EventRegistrationToken token)
{
    _proxyBusObject->_eventsAndProperties->IntrospectRemoteObject::remove(token);
}

QStatus ProxyBusObject::IntrospectRemoteObject::raise(QStatus status, ProxyBusObject ^ obj, Platform::Object ^ context)
{
    return _proxyBusObject->_eventsAndProperties->IntrospectRemoteObject::raise(status, obj, context);
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
            qcc::String strName = _proxyBusObject->GetServiceName();
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
            qcc::String strPath = _proxyBusObject->GetPath();
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
            result = _proxyBusObject->GetSessionId();
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

_ProxyBusObject::_ProxyBusObject(BusAttachment ^ b, ajn::ProxyBusObject* proxybusobject)
    : ajn::ProxyBusObject(*proxybusobject)
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
        _eventsAndProperties->IntrospectRemoteObject += ref new IntrospectRemoteObjectHandler([&] (QStatus status, AllJoyn::ProxyBusObject ^ obj, Platform::Object ^ context)->QStatus {
                                                                                                  return DefaultIntrospectRemoteObjectHandler(status, obj, context);
                                                                                              });
        _eventsAndProperties->Bus = b;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_ProxyBusObject::_ProxyBusObject(BusAttachment ^ b, ajn::BusAttachment& bus, const char* service, const char* path, ajn::SessionId sessionId)
    : ajn::ProxyBusObject(bus, service, path, sessionId)
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
        _eventsAndProperties->Receiver = receiver;
        _eventsAndProperties->IntrospectRemoteObject += ref new IntrospectRemoteObjectHandler([&] (QStatus status, AllJoyn::ProxyBusObject ^ obj, Platform::Object ^ context)->QStatus {
                                                                                                  return DefaultIntrospectRemoteObjectHandler(status, obj, context);
                                                                                              });
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
    ClearObjectMap(&(this->_mutex), &(this->_childObjectMap));
}

QStatus _ProxyBusObject::DefaultIntrospectRemoteObjectHandler(QStatus status, AllJoyn::ProxyBusObject ^ obj, Platform::Object ^ context)
{
    return QStatus::ER_FAIL;
}

void _ProxyBusObject::MessageReceiverProxyReplyHandler(ajn::Message& msg, void* context)
{
    try {
        ::QStatus status = ER_OK;

        while (true) {
            ProxyMessageReceiverCtx* proxyCtx = reinterpret_cast<ProxyMessageReceiverCtx*>(context);
            if (NULL == proxyCtx) {
                status = ER_FAIL;
                break;
            }
            AllJoyn::ProxyBusObject ^ proxy = proxyCtx->Proxy;
            AllJoyn::MessageReceiver ^ receiver = proxyCtx->Receiver;
            Platform::Object ^ objContext = proxyCtx->Context;
            delete proxyCtx;
            proxyCtx = NULL;
            receiver->_receiver->ReplyHandler(msg, (void*)objContext);
            break;
        }

        if (ER_OK != status) {
            QCC_THROW_EXCEPTION(status);
        }
    } catch (...) {
        // Do nothing
    }
}

_ProxyBusObjectListener::_ProxyBusObjectListener(_ProxyBusObject* proxybusobject)
    : _proxyBusObject(proxybusobject)
{
}

_ProxyBusObjectListener::~_ProxyBusObjectListener()
{
    _proxyBusObject = nullptr;
}

ajn::ProxyBusObject::Listener::IntrospectCB _ProxyBusObjectListener::GetProxyListenerCBHandler()
{
    return static_cast<ajn::ProxyBusObject::Listener::IntrospectCB>(&AllJoyn::_ProxyBusObjectListener::IntrospectCB);
}

void _ProxyBusObjectListener::IntrospectCB(::QStatus s, ajn::ProxyBusObject* obj, void* context)
{
    ::QStatus status = ER_OK;

    try {
        while (true) {
            IntrospectCBCtx* introspectCBCtx = (IntrospectCBCtx*)context;
            ProxyBusObject ^ proxy = introspectCBCtx->Proxy;
            Platform::Object ^ objContext = introspectCBCtx->Context;
            delete introspectCBCtx;
            introspectCBCtx = NULL;
            _proxyBusObject->_eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                                          _proxyBusObject->_eventsAndProperties->IntrospectRemoteObject((QStatus)(int)s, proxy, objContext);
                                                                                                                                      }));
            break;
        }

        if (ER_OK != status) {
            QCC_THROW_EXCEPTION(status);
        }
    } catch (...) {
        // Do nothing
    }
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
