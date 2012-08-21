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

#include "BusAttachment.h"

#include <alljoyn/BusObject.h>
#include <alljoyn/BusListener.h>
#include <BusInternal.h>
#include <SocketStream.h>
#include <InterfaceDescription.h>
#include <MessageReceiver.h>
#include <BusObject.h>
#include <ProxyBusObject.h>
#include <BusListener.h>
#include <SessionListener.h>
#include <SessionPortListener.h>
#include <SessionOpts.h>
#include <AuthListener.h>
#include <KeyStoreListener.h>
#include <qcc/String.h>
#include <qcc/winrt/utility.h>
#include <qcc/Event.h>
#include <qcc/winrt/SocketWrapper.h>
#include <ObjectReference.h>
#include <AllJoynException.h>
#include <ctxtcall.h>
#include <ppltasks.h>

using namespace Windows::Foundation;

namespace AllJoyn {

qcc::Mutex _mutex;
std::map<void*, void*> _nativeToManagedMap;

BusAttachment::BusAttachment(Platform::String ^ applicationName, bool allowRemoteMessages, uint32_t concurrency)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == applicationName) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strApplicationName = PlatformToMultibyteString(applicationName);
        if (strApplicationName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _BusAttachment* ba = new _BusAttachment(strApplicationName.c_str(), allowRemoteMessages, concurrency);
        if (NULL == ba) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _mBusAttachment = new qcc::ManagedObj<_BusAttachment>(ba);
        if (NULL == _mBusAttachment) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _busAttachment = &(**_mBusAttachment);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

BusAttachment::BusAttachment(const ajn::BusAttachment* busAttachment)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == busAttachment) {
            status = ER_BAD_ARG_1;
            break;
        }
        _mutex.Lock();
        void* ba = _nativeToManagedMap[(void*)busAttachment];
        _mutex.Unlock();
        if (NULL == ba) {
            status = ER_FAIL;
            break;
        }
        qcc::ManagedObj<_BusAttachment>* mba = reinterpret_cast<qcc::ManagedObj<_BusAttachment>*>(ba);
        _mBusAttachment = new qcc::ManagedObj<_BusAttachment>(*mba);
        if (NULL == _mBusAttachment) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _busAttachment = &(**_mBusAttachment);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

BusAttachment::BusAttachment(const qcc::ManagedObj<_BusAttachment>* busAttachment)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (NULL == busAttachment) {
            status = ER_BAD_ARG_1;
            break;
        }
        _mBusAttachment = new qcc::ManagedObj<_BusAttachment>(*busAttachment);
        if (NULL == _mBusAttachment) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        _busAttachment = &(**_mBusAttachment);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

BusAttachment::~BusAttachment()
{
    if (NULL != _mBusAttachment) {
        delete _mBusAttachment;
        _mBusAttachment = NULL;
        _busAttachment = NULL;
    }
}

uint32_t BusAttachment::GetConcurrency()
{
    return _busAttachment->GetConcurrency();
}

void BusAttachment::EnableConcurrentCallbacks()
{
    _busAttachment->EnableConcurrentCallbacks();
}

void BusAttachment::CreateInterface(Platform::String ^ name, Platform::WriteOnlyArray<AllJoyn::InterfaceDescription ^> ^ iface, bool secure)
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
        if (nullptr == iface || iface->Length != 1) {
            status = ER_BAD_ARG_2;
            break;
        }
        ajn::InterfaceDescription* intDescr;
        status = _busAttachment->CreateInterface(strName.c_str(), intDescr, secure);
        if (ER_OK == status) {
            InterfaceDescription ^ id = ref new InterfaceDescription(intDescr);
            iface[0] = id;
        }
        break;
    }

    if (ER_OK != status) {
        if (nullptr !=  iface) {
            iface[0] = nullptr;
        }
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::CreateInterfacesFromXml(Platform::String ^ xml)
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
        status = _busAttachment->CreateInterfacesFromXml(strXml.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

uint32_t BusAttachment::GetInterfaces(Platform::WriteOnlyArray<InterfaceDescription ^> ^ iface, uint32_t numIfaces)
{
    const ajn::InterfaceDescription* id = NULL;
    size_t result = _busAttachment->GetInterfaces(nullptr == iface ? NULL : &id, numIfaces);
    if (result > 0 && NULL != id) {
        InterfaceDescription ^ refId = ref new InterfaceDescription(id);
        iface[0] = refId;
    }
    return result;
}

InterfaceDescription ^ BusAttachment::GetInterface(Platform::String ^ name)
{
    ::QStatus status = ER_OK;
    InterfaceDescription ^ idRef = nullptr;

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
        const ajn::InterfaceDescription* id = _busAttachment->GetInterface(strName.c_str());
        if (NULL == id) {
            status = ER_FAIL;
            break;
        }
        idRef = ref new InterfaceDescription(id);
        if (nullptr == idRef) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return idRef;
}

void BusAttachment::DeleteInterface(InterfaceDescription ^ iface)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == iface) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::InterfaceDescription* id = *(iface->_interfaceDescr);
        status = _busAttachment->DeleteInterface(*id);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::Start()
{
    ::QStatus status = _busAttachment->Start();
    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::Stop()
{
    ::QStatus status = _busAttachment->Stop();

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::Join()
{
    ::QStatus status = _busAttachment->Join();

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

bool BusAttachment::IsStarted()
{
    return _busAttachment->IsStarted();
}

bool BusAttachment::IsStopping()
{
    return _busAttachment->IsStopping();
}

IAsyncAction ^ BusAttachment::ConnectAsync(Platform::String ^ connectSpec)
{
    ::QStatus status = ER_OK;

    if (nullptr == connectSpec) {
        status = ER_BAD_ARG_1;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    qcc::String strConnectSpec = PlatformToMultibyteString(connectSpec);
    if (strConnectSpec.empty()) {
        status = ER_OUT_OF_MEMORY;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    IAsyncAction ^ action = concurrency::create_async([this, strConnectSpec](concurrency::cancellation_token ct) {
                                                          ::QStatus status = this->_busAttachment->Connect(strConnectSpec.c_str());
                                                          if (ER_OK != status) {
                                                              QCC_THROW_EXCEPTION(status);
                                                          }
                                                      });
    return action;
}

void BusAttachment::Disconnect(Platform::String ^ connectSpec)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == connectSpec) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strConnectSpec = PlatformToMultibyteString(connectSpec);
        if (strConnectSpec.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = _busAttachment->Disconnect(strConnectSpec.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

bool BusAttachment::IsConnected()
{
    return _busAttachment->IsConnected();
}

void BusAttachment::RegisterBusObject(BusObject ^ obj)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == obj) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::BusObject* bo = obj->_busObject;
        status = _busAttachment->RegisterBusObject(*bo);
        if (ER_OK == status) {
            AddObjectReference(&(_busAttachment->_mutex), obj, &(_busAttachment->_busObjectMap));
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::UnregisterBusObject(BusObject ^ object)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == object) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::BusObject* bo = object->_busObject;
        _busAttachment->UnregisterBusObject(*bo);
        RemoveObjectReference(&(_busAttachment->_mutex), object, &(_busAttachment->_busObjectMap));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::RegisterSignalHandler(Platform::Object ^ receiver,
                                          InterfaceMember ^ member,
                                          Platform::String ^ srcPath)
{
    ::QStatus status = ER_OK;
    MessageReceiver ^ receiverObj = dynamic_cast<MessageReceiver ^>(receiver);

    while (true) {
        if (nullptr == receiver) {
            status = ER_BAD_ARG_1;
            break;
        }
        if (nullptr == member) {
            status = ER_BAD_ARG_2;
            break;
        }
        _MessageReceiver* mreceiver = receiverObj->_receiver;
        ajn::MessageReceiver::SignalHandler handler = mreceiver->GetSignalHandler();
        ajn::MessageReceiver* ajnreceiver = mreceiver;
        ajn::InterfaceDescription::Member* imember = *(member->_member);
        qcc::String strSrcPath = PlatformToMultibyteString(srcPath);
        if (nullptr != srcPath && strSrcPath.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = _busAttachment->RegisterSignalHandler(ajnreceiver, handler, imember, strSrcPath.c_str());
        if (ER_OK == status) {
            AddObjectReference(&(_busAttachment->_mutex), receiver, &(_busAttachment->_signalHandlerMap));
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::UnregisterSignalHandler(Platform::Object ^ receiver,
                                            InterfaceMember ^ member,
                                            Platform::String ^ srcPath)
{
    ::QStatus status = ER_OK;
    MessageReceiver ^ receiverObj = dynamic_cast<MessageReceiver ^>(receiver);

    while (true) {
        if (nullptr == receiver) {
            status = ER_BAD_ARG_1;
            break;
        }
        if (nullptr == member) {
            status = ER_BAD_ARG_2;
            break;
        }
        _MessageReceiver* mreceiver = receiverObj->_receiver;
        ajn::MessageReceiver::SignalHandler handler = mreceiver->GetSignalHandler();
        ajn::MessageReceiver* ajnreceiver = mreceiver;
        ajn::InterfaceDescription::Member* imember = *(member->_member);
        qcc::String strSrcPath = PlatformToMultibyteString(srcPath);
        if (nullptr != srcPath && strSrcPath.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = _busAttachment->UnregisterSignalHandler(ajnreceiver, handler, imember, strSrcPath.c_str());
        if (ER_OK == status) {
            RemoveObjectReference(&(_busAttachment->_mutex), receiver, &(_busAttachment->_signalHandlerMap));
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::EnablePeerSecurity(Platform::String ^ authMechanisms,
                                       AuthListener ^ listener,
                                       Platform::String ^ keyStoreFileName,
                                       bool isShared)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == authMechanisms) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strAuthMechanisms = PlatformToMultibyteString(authMechanisms);
        if (strAuthMechanisms.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == listener) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::AuthListener* al = listener->_listener;
        if (nullptr == keyStoreFileName) {
            status = ER_BAD_ARG_3;
            break;
        }
        qcc::String strKeyStoreFileName = PlatformToMultibyteString(keyStoreFileName);
        if (strKeyStoreFileName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = _busAttachment->EnablePeerSecurity(strAuthMechanisms.c_str(), al, strKeyStoreFileName.c_str(), isShared);
        if (ER_OK == status) {
            _busAttachment->_authListener = listener;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::DisablePeerSecurity(AuthListener ^ listener)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == listener) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::AuthListener* al = listener->_listener;
        status = _busAttachment->EnablePeerSecurity(NULL, al, NULL, false);
        if (ER_OK == status) {
            _busAttachment->_authListener = nullptr;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

bool BusAttachment::IsPeerSecurityEnabled()
{
    return _busAttachment->IsPeerSecurityEnabled();
}

void BusAttachment::RegisterBusListener(BusListener ^ listener)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == listener) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::BusListener* bl = listener->_listener;
        // Walk around the native ptr in the callback
        _mutex.Lock();
        _nativeToManagedMap[_busAttachment] = _mBusAttachment;
        _busAttachment->RegisterBusListener(*bl);
        _nativeToManagedMap.erase(_busAttachment);
        _mutex.Unlock();
        AddObjectReference(&(_busAttachment->_mutex), listener, &(_busAttachment->_busListenerMap));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::UnregisterBusListener(BusListener ^ listener)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == listener) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::BusListener* bl = listener->_listener;
        _busAttachment->UnregisterBusListener(*bl);
        RemoveObjectReference(&(_busAttachment->_mutex), listener, &(_busAttachment->_busListenerMap));
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::RegisterKeyStoreListener(KeyStoreListener ^ listener)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == listener) {
            status = ER_BAD_ARG_1;
            break;
        }
        ajn::KeyStoreListener* ksl = listener->_listener;
        status = _busAttachment->RegisterKeyStoreListener(*ksl);
        if (ER_OK == status) {
            _busAttachment->_keyStoreListener = listener;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::UnregisterKeyStoreListener()
{
    ::QStatus status = _busAttachment->UnregisterKeyStoreListener();

    if (ER_OK == status) {
        _busAttachment->_keyStoreListener = nullptr;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::ReloadKeyStore()
{
    ::QStatus status = ER_OK;

    while (true) {
        status = _busAttachment->ReloadKeyStore();
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::ClearKeyStore()
{
    _busAttachment->ClearKeyStore();
}

void BusAttachment::ClearKeys(Platform::String ^ guid)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == guid) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strGuid = PlatformToMultibyteString(guid);
        if (strGuid.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = _busAttachment->ClearKeys(strGuid.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::SetKeyExpiration(Platform::String ^ guid, uint32_t timeout)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == guid) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strGuid = PlatformToMultibyteString(guid);
        if (strGuid.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = _busAttachment->SetKeyExpiration(strGuid.c_str(), timeout);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::GetKeyExpiration(Platform::String ^ guid, Platform::WriteOnlyArray<uint32> ^ timeout)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == guid) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strGuid = PlatformToMultibyteString(guid);
        if (strGuid.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (timeout == nullptr || timeout->Length != 1) {
            status = ER_BAD_ARG_2;
            break;
        }
        uint32_t expiration;
        status = _busAttachment->GetKeyExpiration(strGuid.c_str(), expiration);
        if (ER_OK == status) {
            timeout[0] = expiration;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::AddLogonEntry(Platform::String ^ authMechanism, Platform::String ^ userName, Platform::String ^ password)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == authMechanism) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strAuthMechanism = PlatformToMultibyteString(authMechanism);
        if (strAuthMechanism.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == userName) {
            status = ER_BAD_ARG_2;
            break;
        }
        qcc::String strUserName = PlatformToMultibyteString(userName);
        if (strUserName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        if (nullptr == password) {
            status = ER_BAD_ARG_3;
            break;
        }
        qcc::String strPassword = PlatformToMultibyteString(password);
        if (strPassword.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = _busAttachment->AddLogonEntry(strAuthMechanism.c_str(), strUserName.c_str(), strPassword.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::RequestName(Platform::String ^ requestedName, uint32_t flags)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == requestedName) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strRequestedName = PlatformToMultibyteString(requestedName);
        if (strRequestedName.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = _busAttachment->RequestName(strRequestedName.c_str(), flags);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::ReleaseName(Platform::String ^ name)
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
        status = _busAttachment->ReleaseName(strName.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::AddMatch(Platform::String ^ rule)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == rule) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strRule = PlatformToMultibyteString(rule);
        if (strRule.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = _busAttachment->AddMatch(strRule.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::RemoveMatch(Platform::String ^ rule)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == rule) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strRule = PlatformToMultibyteString(rule);
        if (strRule.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = _busAttachment->RemoveMatch(strRule.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::AdvertiseName(Platform::String ^ name, TransportMaskType transports)
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
        status = _busAttachment->AdvertiseName(strName.c_str(), (int)transports);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::CancelAdvertiseName(Platform::String ^ name, TransportMaskType transports)
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
        status = _busAttachment->CancelAdvertiseName(strName.c_str(), (int)transports);
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::FindAdvertisedName(Platform::String ^ namePrefix)
{
    ::QStatus status = ER_OK;

    while (true) {
        qcc::String strNamePrefix = PlatformToMultibyteString(namePrefix);
        if (nullptr != namePrefix && strNamePrefix.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = _busAttachment->FindAdvertisedName(strNamePrefix.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::CancelFindAdvertisedName(Platform::String ^ namePrefix)
{
    ::QStatus status = ER_OK;

    while (true) {
        qcc::String strNamePrefix = PlatformToMultibyteString(namePrefix);
        if (nullptr != namePrefix && strNamePrefix.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = _busAttachment->CancelFindAdvertisedName(strNamePrefix.c_str());
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::BindSessionPort(ajn::SessionPort sessionPort_in, Platform::WriteOnlyArray<ajn::SessionPort> ^ sessionPort_out, SessionOpts ^ opts, SessionPortListener ^ listener)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == sessionPort_out || sessionPort_out->Length != 1) {
            status = ER_BAD_ARG_2;
            break;
        }
        if (nullptr == opts) {
            status = ER_BAD_ARG_3;
            break;
        }
        ajn::SessionOpts* options = opts->_sessionOpts;
        if (nullptr == listener) {
            status = ER_BAD_ARG_4;
            break;
        }
        ajn::SessionPortListener* spl = listener->_listener;
        ajn::SessionPort port = sessionPort_in;
        status = _busAttachment->BindSessionPort(port, *options, *spl);
        if (ER_OK == status) {
            AddPortReference(&(_busAttachment->_mutex), port, listener, &(_busAttachment->_sessionPortListenerMap));
            sessionPort_out[0] = port;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::UnbindSessionPort(ajn::SessionPort sessionPort)
{
    ::QStatus status = _busAttachment->UnbindSessionPort(sessionPort);

    if (ER_OK == status) {
        RemovePortReference(&(_busAttachment->_mutex), sessionPort, &(_busAttachment->_sessionPortListenerMap));
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Windows::Foundation::IAsyncOperation<JoinSessionResult ^> ^ BusAttachment::JoinSessionAsync(Platform::String ^ sessionHost,
                                                                                            ajn::SessionPort sessionPort,
                                                                                            SessionListener ^ listener,
                                                                                            SessionOpts ^ opts_in,
                                                                                            Platform::WriteOnlyArray<SessionOpts ^> ^ opts_out,
                                                                                            Platform::Object ^ context)
{
    ::QStatus status = ER_OK;
    Windows::Foundation::IAsyncOperation<JoinSessionResult ^> ^ result = nullptr;

    while (true) {
        if (nullptr == sessionHost) {
            status = ER_BAD_ARG_1;
            break;
        }
        qcc::String strSessionHost = PlatformToMultibyteString(sessionHost);
        if (strSessionHost.empty()) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        qcc::ManagedObj<_SessionListener>* msl = NULL;
        ajn::SessionListener* ajnlistener = NULL;
        if (nullptr != listener) {
            ajnlistener = listener->_listener;
        }
        if (nullptr == opts_in) {
            status = ER_BAD_ARG_5;
            break;
        }
        ajn::SessionOpts* opts = opts_in->_sessionOpts;
        if (nullptr == opts_out || opts_out->Length != 1) {
            status = ER_BAD_ARG_6;
            break;
        }
        JoinSessionResult ^ joinSessionResult = ref new JoinSessionResult(this, listener, context);
        if (nullptr == joinSessionResult) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = _busAttachment->JoinSessionAsync(strSessionHost.c_str(),
                                                  sessionPort,
                                                  ajnlistener,
                                                  *opts,
                                                  _busAttachment,
                                                  (void*)joinSessionResult);
        if (ER_OK == status) {
            SessionOpts ^ newOpts = ref new SessionOpts(opts);
            if (nullptr == newOpts) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            opts_out[0] = newOpts;
        } else {
            break;
        }
        result = concurrency::create_async([this, joinSessionResult]()->JoinSessionResult ^
                                           {
                                               joinSessionResult->Wait();
                                               return joinSessionResult;
                                           });
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void BusAttachment::SetSessionListener(ajn::SessionId sessionId, SessionListener ^ listener)
{
    ::QStatus status = ER_OK;

    while (true) {
        ajn::SessionListener* sl = NULL;
        if (nullptr != listener) {
            sl = listener->_listener;
        }
        status = _busAttachment->SetSessionListener(sessionId, sl);
        if (ER_OK == status) {
            if (sl != NULL) {
                AddIdReference(&(_busAttachment->_mutex), sessionId, listener, &(_busAttachment->_sessionListenerMap));
            } else {
                RemoveIdReference(&(_busAttachment->_mutex), sessionId, &(_busAttachment->_sessionListenerMap));
            }
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::LeaveSession(ajn::SessionId sessionId)
{
    ::QStatus status = _busAttachment->LeaveSession(sessionId);

    if (ER_OK == status) {
        RemoveIdReference(&(_busAttachment->_mutex), sessionId, &(_busAttachment->_sessionListenerMap));
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::GetSessionSocketStream(ajn::SessionId sessionId, Platform::WriteOnlyArray<AllJoyn::SocketStream ^> ^ socketStream)
{
    ::QStatus status = ER_OK;

    while (true) {
        if (nullptr == socketStream || socketStream->Length != 1) {
            status = ER_BAD_ARG_2;
            break;
        }
        qcc::SocketFd sock;
        status = _busAttachment->GetSessionFd(sessionId, sock);
        if (ER_OK == status) {
            qcc::winrt::SocketWrapper ^ s = reinterpret_cast<qcc::winrt::SocketWrapper ^>((void*)sock);
            AllJoyn::SocketStream ^ ss = ref new AllJoyn::SocketStream(s);
            if (nullptr == ss) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            socketStream[0] = ss;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

Windows::Foundation::IAsyncOperation<SetLinkTimeoutResult ^> ^ BusAttachment::SetLinkTimeoutAsync(ajn::SessionId sessionid,
                                                                                                  uint32 linkTimeout,
                                                                                                  Platform::Object ^ context)
{
    ::QStatus status = ER_OK;
    Windows::Foundation::IAsyncOperation<SetLinkTimeoutResult ^> ^ result = nullptr;

    while (true) {
        SetLinkTimeoutResult ^ setLinkTimeoutResult = ref new SetLinkTimeoutResult(this, context);
        if (nullptr == setLinkTimeoutResult) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        status = _busAttachment->SetLinkTimeoutAsync(sessionid,
                                                     linkTimeout,
                                                     _busAttachment,
                                                     (void*)setLinkTimeoutResult);
        if (ER_OK != status) {
            break;
        }
        result = concurrency::create_async([this, setLinkTimeoutResult]()->SetLinkTimeoutResult ^
                                           {
                                               setLinkTimeoutResult->Wait();
                                               return setLinkTimeoutResult;
                                           });
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

void BusAttachment::NameHasOwner(Platform::String ^ name, Platform::WriteOnlyArray<bool> ^ hasOwner)
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
        if (hasOwner == nullptr || hasOwner->Length != 1) {
            status = ER_BAD_ARG_2;
            break;
        }
        bool owned;
        status = _busAttachment->NameHasOwner(strName.c_str(), owned);
        if (ER_OK == status) {
            hasOwner[0] = owned;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

void BusAttachment::GetPeerGUID(Platform::String ^ name, Platform::WriteOnlyArray<Platform::String ^> ^ guid)
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
        if (nullptr == guid || guid->Length != 1) {
            status = ER_BAD_ARG_2;
            break;
        }
        qcc::String peerGuid;
        status = _busAttachment->GetPeerGUID(strName.c_str(), peerGuid);
        if (ER_OK == status) {
            Platform::String ^ tempGuid = MultibyteToPlatformString(peerGuid.c_str());
            if (nullptr == tempGuid) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            guid[0] = tempGuid;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

bool BusAttachment::IsSameBusAttachment(BusAttachment ^ other)
{
    if (nullptr == other) {
        return false;
    }
    return &(**_mBusAttachment) == &(**other->_mBusAttachment);
}

ProxyBusObject ^ BusAttachment::DBusProxyBusObject::get()
{
    ::QStatus status = ER_OK;
    ProxyBusObject ^ result = nullptr;

    while (true) {
        if (nullptr == _busAttachment->_eventsAndProperties->DBusProxyBusObject) {
            const ajn::ProxyBusObject* proxyObj = &_busAttachment->GetDBusProxyObj();
            if (NULL == proxyObj) {
                // Do nothing. just return
                break;
            }
            result = ref new ProxyBusObject(this, proxyObj);
            if (nullptr == result) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _busAttachment->_eventsAndProperties->DBusProxyBusObject = result;
        } else {
            result = _busAttachment->_eventsAndProperties->DBusProxyBusObject;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

ProxyBusObject ^ BusAttachment::AllJoynProxyBusObject::get()
{
    ::QStatus status = ER_OK;
    ProxyBusObject ^ result = nullptr;

    while (true) {
        if (nullptr == _busAttachment->_eventsAndProperties->AllJoynProxyBusObject) {
            const ajn::ProxyBusObject* proxyObj = &_busAttachment->GetAllJoynProxyObj();
            if (NULL == proxyObj) {
                // Do nothing. just return
                break;
            }
            result = ref new ProxyBusObject(this, proxyObj);
            if (nullptr == result) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _busAttachment->_eventsAndProperties->AllJoynProxyBusObject = result;
        } else {
            result = _busAttachment->_eventsAndProperties->AllJoynProxyBusObject;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

ProxyBusObject ^ BusAttachment::AllJoynDebugProxyBusObject::get()
{
    ::QStatus status = ER_OK;
    ProxyBusObject ^ result = nullptr;

    while (true) {
        if (nullptr == _busAttachment->_eventsAndProperties->AllJoynDebugProxyBusObject) {
            const ajn::ProxyBusObject* proxyObj = &_busAttachment->GetAllJoynDebugObj();
            if (NULL == proxyObj) {
                // Do nothing. just return
                break;
            }
            result = ref new ProxyBusObject(this, proxyObj);
            if (nullptr == result) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _busAttachment->_eventsAndProperties->AllJoynDebugProxyBusObject = result;
        } else {
            result = _busAttachment->_eventsAndProperties->AllJoynDebugProxyBusObject;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ BusAttachment::UniqueName::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _busAttachment->_eventsAndProperties->UniqueName) {
            qcc::String uniqueName = _busAttachment->GetUniqueName();
            result = MultibyteToPlatformString(uniqueName.c_str());
            if (nullptr == result && !uniqueName.empty()) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _busAttachment->_eventsAndProperties->UniqueName = result;
        } else {
            result = _busAttachment->_eventsAndProperties->UniqueName;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

Platform::String ^ BusAttachment::GlobalGUIDString::get()
{
    ::QStatus status = ER_OK;
    Platform::String ^ result = nullptr;

    while (true) {
        if (nullptr == _busAttachment->_eventsAndProperties->GlobalGUIDString) {
            result = MultibyteToPlatformString(_busAttachment->GetGlobalGUIDString().c_str());
            if (nullptr == result) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            _busAttachment->_eventsAndProperties->GlobalGUIDString = result;
        } else {
            result = _busAttachment->_eventsAndProperties->GlobalGUIDString;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

uint32_t BusAttachment::Timestamp::get()
{
    ::QStatus status = ER_OK;
    uint32_t result = (uint32_t)-1;

    while (true) {
        if ((uint32_t)-1 == _busAttachment->_eventsAndProperties->Timestamp) {
            result = _busAttachment->GetTimestamp();
            _busAttachment->_eventsAndProperties->Timestamp = result;
        } else {
            result = _busAttachment->_eventsAndProperties->Timestamp;
        }
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }

    return result;
}

_BusAttachment::_BusAttachment(const char* applicationName, bool allowRemoteMessages, uint32_t concurrency)
    : BusAttachment(applicationName, allowRemoteMessages, concurrency), ajn::BusAttachment::JoinSessionAsyncCB(),
    _keyStoreListener(nullptr), _authListener(nullptr), _dispatcher(nullptr), _originSTA(false)
{
    ::QStatus status = ER_OK;

    while (true) {
        _eventsAndProperties = ref new __BusAttachment();
        if (nullptr == _eventsAndProperties) {
            status = ER_OUT_OF_MEMORY;
            break;
        }
        Windows::UI::Core::CoreWindow ^ window = Windows::UI::Core::CoreWindow::GetForCurrentThread();
        if (nullptr != window) {
            _dispatcher = window->Dispatcher;
        }
        _originSTA = IsOriginSTA();
        break;
    }

    if (ER_OK != status) {
        QCC_THROW_EXCEPTION(status);
    }
}

_BusAttachment::~_BusAttachment()
{
    _eventsAndProperties = nullptr;
    _keyStoreListener = nullptr;
    _authListener = nullptr;
    ClearObjectMap(&(this->_mutex), &(this->_busObjectMap));
    ClearObjectMap(&(this->_mutex), &(this->_signalHandlerMap));
    ClearObjectMap(&(this->_mutex), &(this->_busListenerMap));
    ClearPortMap(&(this->_mutex), &(this->_sessionPortListenerMap));
    ClearIdMap(&(this->_mutex), &(this->_sessionListenerMap));
}

void _BusAttachment::JoinSessionCB(::QStatus s, ajn::SessionId sessionId, const ajn::SessionOpts& opts, void* context)
{
    ::QStatus status = ER_OK;
    JoinSessionResult ^ joinSessionResult = reinterpret_cast<JoinSessionResult ^>(context);

    try {
        while (true) {
            SessionOpts ^ options = ref new SessionOpts(&opts);
            if (nullptr == options) {
                status = ER_OUT_OF_MEMORY;
                break;
            }
            AddIdReference(&(this->_mutex), sessionId, joinSessionResult->Listener, &(this->_sessionListenerMap));
            joinSessionResult->Status = (AllJoyn::QStatus)s;
            joinSessionResult->SessionId = sessionId;
            joinSessionResult->Opts = options;
            break;
        }

        if (ER_OK != status) {
            QCC_THROW_EXCEPTION(status);
        }

        joinSessionResult->Complete();
    } catch (Platform::Exception ^ pe) {
        // Forward Platform::Exception
        joinSessionResult->_exception = pe;
        joinSessionResult->Complete();
    } catch (std::exception& e) {
        // Forward std::exception
        joinSessionResult->_stdException = new std::exception(e);
        joinSessionResult->Complete();
    }
}

void _BusAttachment::SetLinkTimeoutCB(::QStatus s, uint32_t timeout, void* context)
{
    ::QStatus status = ER_OK;
    SetLinkTimeoutResult ^ setLinkTimeoutResult = reinterpret_cast<SetLinkTimeoutResult ^>(context);
    setLinkTimeoutResult->Status = (AllJoyn::QStatus)s;
    setLinkTimeoutResult->Timeout = timeout;
    setLinkTimeoutResult->Complete();
}

void _BusAttachment::DispatchCallback(Windows::UI::Core::DispatchedHandler ^ callback)
{
    Windows::UI::Core::CoreWindow ^ window = Windows::UI::Core::CoreWindow::GetForCurrentThread();
    Windows::UI::Core::CoreDispatcher ^ dispatcher = nullptr;
    if (nullptr != window) {
        dispatcher = window->Dispatcher;
    }
    if (_originSTA && nullptr != _dispatcher && _dispatcher != dispatcher) {
        // Our origin was STA and the thread dispatcher doesn't match up. Move execution to the origin dispatcher thread.
        Windows::Foundation::IAsyncAction ^ op = _dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
                                                                       ref new Windows::UI::Core::DispatchedHandler([this, callback] () {
                                                                                                                        callback();
                                                                                                                    }));
        // Since we are now queued up, enable concurrency to prevent any unnecessary blocking (this turns whatever callback does into a no op)
        EnableConcurrentCallbacks();
        // Exceptions are caught by specific handlers. If wait() below throws, that is a bug in the handler wrapper implementation.
        concurrency::task<void> dispatcherOp(op);
        dispatcherOp.wait();
    } else {
        // In this case, our source origin is MTA or we are STA with either no dispatcher (no UI threads involved) or we are already in the dispatcher thread
        // for the STA compartment.
        callback();
    }
}

bool _BusAttachment::IsOriginSTA()
{
    APTTYPE aptType;
    APTTYPEQUALIFIER aptTypeQualifier;
    HRESULT hr = ::CoGetApartmentType(&aptType, &aptTypeQualifier);
    if (SUCCEEDED(hr)) {
        if (aptType == APTTYPE_MAINSTA || aptType == APTTYPE_STA) {
            return true;
        }
    }
    return false;
}

__BusAttachment::__BusAttachment()
{
    DBusProxyBusObject = nullptr;
    AllJoynProxyBusObject = nullptr;
    AllJoynDebugProxyBusObject = nullptr;
    UniqueName = nullptr;
    GlobalGUIDString = nullptr;
    Timestamp = (uint32_t)-1;
}

__BusAttachment::~__BusAttachment()
{
    DBusProxyBusObject = nullptr;
    AllJoynProxyBusObject = nullptr;
    AllJoynDebugProxyBusObject = nullptr;
    UniqueName = nullptr;
    GlobalGUIDString = nullptr;
    Timestamp = (uint32_t)-1;
}

}
