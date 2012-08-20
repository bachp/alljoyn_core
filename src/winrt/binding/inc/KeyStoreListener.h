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

#include <alljoyn/KeyStoreListener.h>
#include <qcc/ManagedObj.h>

namespace AllJoyn {

ref class BusAttachment;

public delegate Platform::String ^ KeyStoreListenerGetKeysHandler();
public delegate Platform::String ^ KeyStoreListenerGetPasswordHandler();
public delegate void KeyStoreListenerPutKeysHandler(Platform::String ^ keys);

ref class __KeyStoreListener {
  private:
    friend ref class KeyStoreListener;
    friend class _KeyStoreListener;
    __KeyStoreListener();
    ~__KeyStoreListener();

    event KeyStoreListenerGetKeysHandler ^ GetKeys;
    event KeyStoreListenerGetPasswordHandler ^ GetPassword;
    event KeyStoreListenerPutKeysHandler ^ PutKeys;
    property BusAttachment ^ Bus;
};

class _KeyStoreListener : protected ajn::KeyStoreListener {
  protected:
    friend class qcc::ManagedObj<_KeyStoreListener>;
    friend ref class KeyStoreListener;
    friend ref class BusAttachment;
    _KeyStoreListener(BusAttachment ^ bus);
    ~_KeyStoreListener();

    Platform::String ^ DefaultKeyStoreListenerGetKeysHandler();
    Platform::String ^ DefaultKeyStoreListenerGetPasswordHandler();
    void DefaultKeyStoreListenerPutKeysHandler(Platform::String ^ keys);
    ::QStatus LoadRequest(ajn::KeyStore& keyStore);
    ::QStatus StoreRequest(ajn::KeyStore& keyStore);

    __KeyStoreListener ^ _eventsAndProperties;
};

public ref class KeyStoreListener sealed {
  public:
    KeyStoreListener(BusAttachment ^ bus);

    event KeyStoreListenerGetKeysHandler ^ GetKeys
    {
        Windows::Foundation::EventRegistrationToken add(KeyStoreListenerGetKeysHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        Platform::String ^ raise();
    }

    event KeyStoreListenerGetPasswordHandler ^ GetPassword
    {
        Windows::Foundation::EventRegistrationToken add(KeyStoreListenerGetPasswordHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        Platform::String ^ raise();
    }

    event KeyStoreListenerPutKeysHandler ^ PutKeys
    {
        Windows::Foundation::EventRegistrationToken add(KeyStoreListenerPutKeysHandler ^ handler);
        void remove(Windows::Foundation::EventRegistrationToken token);
        void raise(Platform::String ^ keys);
    }

    property BusAttachment ^ Bus
    {
        BusAttachment ^ get();
    }

  private:
    friend ref class BusAttachment;
    KeyStoreListener(const qcc::ManagedObj<_KeyStoreListener>* listener);
    ~KeyStoreListener();

    qcc::ManagedObj<_KeyStoreListener>* _mListener;
    _KeyStoreListener* _listener;
};

}
// KeyStoreListener.h
