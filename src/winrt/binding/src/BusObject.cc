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

#include "BusObject.h"

#include <alljoyn/BusObject.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/Session.h>
#include <BusAttachment.h>
#include <InterfaceDescription.h>
#include <MsgArg.h>
#include <qcc/String.h>
#include <qcc/winrt/utility.h>
#include <alljoyn/MsgArg.h>
#include <ObjectReference.h>
#include <AllJoynException.h>

namespace AllJoyn {

BusObject::BusObject(BusAttachment ^ bus, Platform::String ^ path, bool isPlaceholder)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == bus) {
            status = ER_BAD_ARG_1;
            break;
        }
        if (nullptr == path) {
            status = ER_BAD_ARG_2;
            break;
        }
        qcc::String strPath = PlatformToMultibyteString(path);
        if (strPath.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::BusAttachment* attachment = bus->_busAttachment;
        _BusObject* bo = new _BusObject(bus, *attachment, strPath.c_str(), isPlaceholder);
        if (NULL == bo) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mBusObject = new qcc::ManagedObj<_BusObject>(bo);
        if (NULL == _mBusObject) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _busObject = &(**_mBusObject);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

BusObject::BusObject(const qcc::ManagedObj<_BusObject>* busObject)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == busObject) {
            status = ER_BAD_ARG_1;
            break;
        }
        _mBusObject = new qcc::ManagedObj<_BusObject>(*busObject);
        if (NULL == _mBusObject) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _busObject = &(**_mBusObject);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

BusObject::~BusObject()
{
    if (NULL != _mBusObject) {
        delete _mBusObject;
        _mBusObject = NULL;
        _busObject = NULL;
    }
}

void BusObject::EmitPropChanged(Platform::String ^ ifcName, Platform::String ^ propName, MsgArg ^ val, ajn::SessionId id)
{
    ::QStatus status = ER_OK;

    while (true) {
        qcc::String strIfcName = PlatformToMultibyteString(ifcName);
        if (nullptr != ifcName && strIfcName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strPropName = PlatformToMultibyteString(propName);
        if (nullptr != propName && strPropName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::MsgArg* msgArg = val->_msgArg;
        _busObject->EmitPropChanged(strIfcName.c_str(), strPropName.c_str(), *msgArg, id);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusObject::MethodReply(Message ^ msg, const Platform::Array<MsgArg ^> ^ args)
{
    ::QStatus status = ER_OK;
    ajn::MsgArg* msgScratch = NULL;

    while (true) {
        if (nullptr == msg) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::Message* mMessage = *(msg->_message);
        if (nullptr == args || args->Length < 1) {
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
        status = _busObject->MethodReply(*mMessage, msgScratch, argsCount);
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

void BusObject::MethodReply(Message ^ msg, Platform::String ^ error, Platform::String ^ errorMessage)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == msg) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::Message* mMessage = *(msg->_message);
        qcc::String strError = PlatformToMultibyteString(error);
        if (nullptr != error && strError.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strErrorMessage = PlatformToMultibyteString(errorMessage);
        if (nullptr != errorMessage && strErrorMessage.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = _busObject->MethodReply(*mMessage, strError.c_str(), strErrorMessage.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusObject::MethodReplyWithQStatus(Message ^ msg, QStatus s)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == msg) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::Message* mMessage = *(msg->_message);
        status = _busObject->MethodReply(*mMessage, (::QStatus)(int)s);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusObject::Signal(Platform::String ^ destination,
                       ajn::SessionId sessionId,
                       InterfaceMember ^ signal,
                       const Platform::Array<MsgArg ^> ^ args,
                       uint16_t timeToLive,
                       uint8_t flags)
{
    ::QStatus status = ER_OK;
    ajn::MsgArg* msgArgs = NULL;

    while (true) {
        qcc::String strDestination = PlatformToMultibyteString(destination);
        if (nullptr != destination && strDestination.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == signal) {
            status = ER_BAD_ARG_3;
            break;
        }
        ajn::InterfaceDescription::Member* member = *(signal->_member);
        size_t numArgs = 0;
        if (nullptr != args && args->Length > 0) {
            numArgs = args->Length;
            msgArgs = new ajn::MsgArg[numArgs];
            if (NULL == msgArgs) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            for (int i = 0; i < numArgs; ++i) {
                if (nullptr == args[i]) {
                    status = ER_BUFFER_TOO_SMALL;
                    break;
                }
                msgArgs[i] = *(args[i]->_msgArg);
            }
            if (ER_OK != status) {
                break;
            }
        }
        status = _busObject->Signal(strDestination.c_str(), sessionId, *member, msgArgs, numArgs, timeToLive, flags);
        break;
    }

    if (NULL != msgArgs) {
        delete [] msgArgs;
        msgArgs = NULL;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusObject::AddInterface(InterfaceDescription ^ iface)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == iface) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::InterfaceDescription* id = *(iface->_interfaceDescr);
        status = _busObject->AddInterface(*id);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusObject::AddMethodHandler(InterfaceMember ^ member, MessageReceiver ^ receiver)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == member) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::InterfaceDescription::Member* imember = *(member->_member);
        if (nullptr == receiver) {
            status = ER_BAD_ARG_2;
            break;
        }
        if (!Bus->IsSameBusAttachment(receiver->_receiver->Bus)) {
            status = ER_BAD_ARG_2;
            break;
        }
        ajn::MessageReceiver::MethodHandler handler = receiver->_receiver->GetMethodHandler();
        status = _busObject->AddMethodHandler(imember, handler, (void*)receiver);
        if (ER_OK == status) {
            AddObjectReference(&(_busObject->_mutex), receiver, &(_busObject->_messageReceiverMap));
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Windows::Foundation::EventRegistrationToken BusObject::Get::add(BusObjectGetHandler ^ handler)
{
    return _busObject->_eventsAndProperties->Get::add(handler);
}

void BusObject::Get::remove(Windows::Foundation::EventRegistrationToken token)
{
    _busObject->_eventsAndProperties->Get::remove(token);
}

QStatus BusObject::Get::raise(Platform::String ^ ifcName, Platform::String ^ propName, Platform::WriteOnlyArray<MsgArg ^> ^ val)
{
    return _busObject->_eventsAndProperties->Get::raise(ifcName, propName, val);
}

Windows::Foundation::EventRegistrationToken BusObject::Set::add(BusObjectSetHandler ^ handler)
{
    return _busObject->_eventsAndProperties->Set::add(handler);
}

void BusObject::Set::remove(Windows::Foundation::EventRegistrationToken token)
{
    _busObject->_eventsAndProperties->Set::remove(token);
}

QStatus BusObject::Set::raise(Platform::String ^ ifcName, Platform::String ^ propName, MsgArg ^ val)
{
    return _busObject->_eventsAndProperties->Set::raise(ifcName, propName, val);
}

Windows::Foundation::EventRegistrationToken BusObject::GenerateIntrospection::add(BusObjectGenerateIntrospectionHandler ^ handler)
{
    return _busObject->_eventsAndProperties->GenerateIntrospection::add(handler);
}

void BusObject::GenerateIntrospection::remove(Windows::Foundation::EventRegistrationToken token)
{
    _busObject->_eventsAndProperties->GenerateIntrospection::remove(token);
}

Platform::String ^ BusObject::GenerateIntrospection::raise(bool deep, uint32_t indent)
{
    return _busObject->_eventsAndProperties->GenerateIntrospection::raise(deep, indent);
}

Windows::Foundation::EventRegistrationToken BusObject::ObjectRegistered::add(BusObjectObjectRegisteredHandler ^ handler)
{
    return _busObject->_eventsAndProperties->ObjectRegistered::add(handler);
}

void BusObject::ObjectRegistered::remove(Windows::Foundation::EventRegistrationToken token)
{
    _busObject->_eventsAndProperties->ObjectRegistered::remove(token);
}

void BusObject::ObjectRegistered::raise(void)
{
    _busObject->_eventsAndProperties->ObjectRegistered::raise();
}

Windows::Foundation::EventRegistrationToken BusObject::ObjectUnregistered::add(BusObjectObjectUnregisteredHandler ^ handler)
{
    return _busObject->_eventsAndProperties->ObjectUnregistered::add(handler);
}

void BusObject::ObjectUnregistered::remove(Windows::Foundation::EventRegistrationToken token)
{
    _busObject->_eventsAndProperties->ObjectUnregistered::remove(token);
}

void BusObject::ObjectUnregistered::raise(void)
{
    _busObject->_eventsAndProperties->ObjectUnregistered::raise();
}

Windows::Foundation::EventRegistrationToken BusObject::GetAllProps::add(BusObjectGetAllPropsHandler ^ handler)
{
    return _busObject->_eventsAndProperties->GetAllProps::add(handler);
}

void BusObject::GetAllProps::remove(Windows::Foundation::EventRegistrationToken token)
{
    _busObject->_eventsAndProperties->GetAllProps::remove(token);
}

void BusObject::GetAllProps::raise(InterfaceMember ^ member, Message ^ msg)
{
    _busObject->_eventsAndProperties->GetAllProps::raise(member, msg);
}

Windows::Foundation::EventRegistrationToken BusObject::Introspect::add(BusObjectIntrospectHandler ^ handler)
{
    return _busObject->_eventsAndProperties->Introspect::add(handler);
}

void BusObject::Introspect::remove(Windows::Foundation::EventRegistrationToken token)
{
    _busObject->_eventsAndProperties->Introspect::remove(token);
}

void BusObject::Introspect::raise(InterfaceMember ^ member, Message ^ msg)
{
    _busObject->_eventsAndProperties->Introspect::raise(member, msg);
}

BusAttachment ^ BusObject::Bus::get()
{
    return _busObject->_eventsAndProperties->Bus;
}

Platform::String ^ BusObject::Name::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _busObject->_eventsAndProperties->Name) {
            qcc::String oName = _busObject->GetName();
            result = MultibyteToPlatformString(oName.c_str());
            if (nullptr == result && !oName.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _busObject->_eventsAndProperties->Name = result;
        } else {
            result = _busObject->_eventsAndProperties->Name;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ BusObject::Path::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _busObject->_eventsAndProperties->Name) {
            qcc::String oPath = _busObject->GetPath();
            result = MultibyteToPlatformString(oPath.c_str());
            if (nullptr == result && !oPath.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _busObject->_eventsAndProperties->Path = result;
        } else {
            result = _busObject->_eventsAndProperties->Path;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

MessageReceiver ^ BusObject::Receiver::get()
{
    return _busObject->_eventsAndProperties->Receiver;
}

_BusObject::_BusObject(BusAttachment ^ b, ajn::BusAttachment& bus, const char* path, bool isPlaceholder)
    :  ajn::BusObject(bus, path, isPlaceholder)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __BusObject();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        AllJoyn::MessageReceiver ^ receiver =  ref new AllJoyn::MessageReceiver(b);
        if (nullptr == receiver) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Receiver = receiver;
        _eventsAndProperties->Get += ref new BusObjectGetHandler([&] (Platform::String ^ ifcName, Platform::String ^ propName, Platform::WriteOnlyArray<MsgArg ^> ^ val)->AllJoyn::QStatus {
                                                                     return DefaultBusObjectGetHandler(ifcName, propName, val);
                                                                 });
        _eventsAndProperties->Set += ref new BusObjectSetHandler([&] (Platform::String ^ ifcName, Platform::String ^ propName, MsgArg ^ val)->AllJoyn::QStatus {
                                                                     return DefaultBusObjectSetHandler(ifcName, propName, val);
                                                                 });
        _eventsAndProperties->GenerateIntrospection += ref new BusObjectGenerateIntrospectionHandler([&] (bool deep, size_t indent)->Platform::String ^ {
                                                                                                         return DefaultBusObjectGenerateIntrospectionHandler(deep, indent);
                                                                                                     });
        _eventsAndProperties->ObjectRegistered += ref new BusObjectObjectRegisteredHandler([&] (void) {
                                                                                               DefaultBusObjectObjectRegisteredHandler();
                                                                                           });
        _eventsAndProperties->ObjectUnregistered += ref new BusObjectObjectUnregisteredHandler([&] (void) {
                                                                                                   DefaultBusObjectObjectUnregisteredHandler();
                                                                                               });
        _eventsAndProperties->GetAllProps += ref new BusObjectGetAllPropsHandler([&] (InterfaceMember ^ member, Message ^ msg) {
                                                                                     DefaultBusObjectGetAllPropsHandler(member, msg);
                                                                                 });
        _eventsAndProperties->Introspect += ref new BusObjectIntrospectHandler([&] (InterfaceMember ^ member, Message ^ msg) {
                                                                                   DefaultBusObjectIntrospectHandler(member, msg);
                                                                               });
        _eventsAndProperties->Bus = b;
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_BusObject::~_BusObject()
{
    _eventsAndProperties = nullptr;
    _mReceiver = NULL;
    ClearObjectMap(&(this->_mutex), &(this->_messageReceiverMap));
}

QStatus _BusObject::DefaultBusObjectGetHandler(Platform::String ^ ifcName, Platform::String ^ propName, Platform::WriteOnlyArray<MsgArg ^> ^ val)
{
    ::QStatus status = ER_OK;

    while (true) {
        qcc::String strIfcName = PlatformToMultibyteString(ifcName);
        if (nullptr != ifcName && strIfcName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strPropName = PlatformToMultibyteString(propName);
        if (nullptr != propName && strPropName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::MsgArg msgArg;
        status = ajn::BusObject::Get(strIfcName.c_str(), strPropName.c_str(), msgArg);
        if (ER_OK == status) {
            MsgArg ^ newArg = ref new MsgArg(&msgArg);
            val[0] = newArg;
        }
        break;
    }

    return (AllJoyn::QStatus)(int)status;
}

QStatus _BusObject::DefaultBusObjectSetHandler(Platform::String ^ ifcName, Platform::String ^ propName, MsgArg ^ val)
{
    ::QStatus status = ER_OK;

    while (true) {
        qcc::String strIfcName = PlatformToMultibyteString(ifcName);
        if (nullptr != ifcName && strIfcName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::String strPropName = PlatformToMultibyteString(propName);
        if (nullptr != propName && strPropName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        ajn::MsgArg* msgArg = val->_msgArg;
        status = ajn::BusObject::Set(strIfcName.c_str(), strPropName.c_str(), *msgArg);
        break;
    }

    return (AllJoyn::QStatus)(int)status;
}

Platform::String ^ _BusObject::DefaultBusObjectGenerateIntrospectionHandler(bool deep, uint32_t indent)
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        qcc::String introspected = ajn::BusObject::GenerateIntrospection(deep, indent);
        result = MultibyteToPlatformString(introspected.c_str());
        if (nullptr == result && !introspected.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void _BusObject::DefaultBusObjectObjectRegisteredHandler(void)
{
    ajn::BusObject::ObjectRegistered();
}

void _BusObject::DefaultBusObjectObjectUnregisteredHandler(void)
{
    ajn::BusObject::ObjectUnregistered();
}

void _BusObject::DefaultBusObjectGetAllPropsHandler(InterfaceMember ^ member, Message ^ msg)
{
    ajn::InterfaceDescription::Member* imember = *(member->_member);
    ajn::Message* mMsg = *(msg->_message);
    ajn::Message m = *mMsg;
    ajn::BusObject::GetAllProps(imember, m);
}

void _BusObject::DefaultBusObjectIntrospectHandler(InterfaceMember ^ member, Message ^ msg)
{
    ajn::InterfaceDescription::Member* imember = *(member->_member);
    ajn::Message* mMsg = *(msg->_message);
    ajn::Message m = *mMsg;
    ajn::BusObject::Introspect(imember, m);
}

::QStatus _BusObject::AddInterface(const ajn::InterfaceDescription& iface)
{
    return ajn::BusObject::AddInterface(iface);
}

::QStatus _BusObject::Get(const char* ifcName, const char* propName, ajn::MsgArg& val)
{
    ::QStatus status = ER_FAIL;

    while (true) {
        Platform::String ^ strIfcName = MultibyteToPlatformString(ifcName);
        if (strIfcName == nullptr && NULL != ifcName && '\0' != ifcName[0]) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        Platform::String ^ strPropName = MultibyteToPlatformString(propName);
        if (strPropName == nullptr && NULL != propName && '\0' != propName[0]) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        Platform::Array<MsgArg ^> ^ msgArgArray = ref new Platform::Array<MsgArg ^>(1);
        if (nullptr == msgArgArray) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     status = (::QStatus)(int)_eventsAndProperties->Get(strIfcName, strPropName, msgArgArray);
                                                                                                                 }));
        if (ER_OK == status) {
            MsgArg ^ msgArgOut = msgArgArray[0];
            ajn::MsgArg* msgArg = msgArgOut->_msgArg;
            val = *msgArg;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return status;
}

::QStatus _BusObject::Set(const char* ifcName, const char* propName, ajn::MsgArg& val)
{
    ::QStatus status = ER_FAIL;

    while (true) {
        Platform::String ^ strIfcName = MultibyteToPlatformString(ifcName);
        if (strIfcName == nullptr && NULL != ifcName && '\0' != ifcName[0]) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        Platform::String ^ strPropName = MultibyteToPlatformString(propName);
        if (strPropName == nullptr && NULL != propName && '\0' != propName[0]) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        MsgArg ^ msgArg = ref new MsgArg(&val);
        if (nullptr == msgArg) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     status = (::QStatus)(int)_eventsAndProperties->Set(strIfcName, strPropName, msgArg);
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return status;
}

qcc::String _BusObject::GenerateIntrospection(bool deep, uint32_t indent)
{
    ::QStatus status = ER_OK;
    qcc::String result;

    while (true) {
        Platform::String ^ ret = nullptr;
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     ret = _eventsAndProperties->GenerateIntrospection(deep, indent);
                                                                                                                 }));
        result = PlatformToMultibyteString(ret);
        if (nullptr != ret && result.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void _BusObject::ObjectRegistered(void)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     _eventsAndProperties->ObjectRegistered();
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void _BusObject::ObjectUnregistered(void)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     _eventsAndProperties->ObjectUnregistered();
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void _BusObject::GetAllProps(const ajn::InterfaceDescription::Member* member, ajn::Message& msg)
{
    ::QStatus status = ER_OK;

    while (true) {
        InterfaceMember ^ imember = ref new InterfaceMember((void*)member);
        if (nullptr == imember) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        Message ^ m = ref new Message(&msg);
        if (nullptr == m) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     _eventsAndProperties->GetAllProps(imember, m);
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void _BusObject::Introspect(const ajn::InterfaceDescription::Member* member, ajn::Message& msg)
{
    ::QStatus status = ER_OK;

    while (true) {
        InterfaceMember ^ imember = ref new InterfaceMember((void*)member);
        if (nullptr == imember) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        Message ^ m = ref new Message(&msg);
        if (nullptr == m) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _eventsAndProperties->Bus->_busAttachment->DispatchCallback(ref new Windows::UI::Core::DispatchedHandler([&]() {
                                                                                                                     _eventsAndProperties->Introspect(imember, m);
                                                                                                                 }));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void _BusObject::CallMethodHandler(ajn::MessageReceiver::MethodHandler handler, const ajn::InterfaceDescription::Member* member, ajn::Message& message, void* context)
{
    AllJoyn::MessageReceiver ^ receiver = reinterpret_cast<AllJoyn::MessageReceiver ^>(context);
    if (nullptr != receiver) {
        receiver->_receiver->MethodHandler(member, message);
    } else if (NULL != handler) {
        // Signals, Get[Prop]/Set[Prop] have no way to specify the context/receiver
        (this->*handler)(member, message);
    }
}

__BusObject::__BusObject()
{
    Bus = nullptr;
    Name = nullptr;
    Path = nullptr;
    Receiver = nullptr;
}

__BusObject::~__BusObject()
{
    Bus = nullptr;
    Name = nullptr;
    Path = nullptr;
    Receiver = nullptr;
}

}
