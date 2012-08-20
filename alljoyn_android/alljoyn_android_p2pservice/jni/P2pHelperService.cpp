/**
 * @file
 * This is the native code that handles the bus communication part of the
 * Android service used for controlling Wifi P2p via the Android framework
 */

/******************************************************************************
 * Copyright 2009-2012, Qualcomm Innovation Center, Inc.
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
 ******************************************************************************/

#include <alljoyn/BusAttachment.h>
#include <qcc/String.h>
#include <android/log.h>
#include <qcc/Log.h>
#include <assert.h>
#include <alljoyn/DBusStd.h>

#include <jni.h>

#define LOG_TAG  "P2pHelperService"
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))


/* Missing (from NDK) log macros (cutils/log.h) */
#ifndef LOGD
#define LOGD(...) (__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#endif

#ifndef LOGI
#define LOGI(...) (__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#endif

#ifndef LOGE
#define LOGE(...) (__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#endif

#define ER_GENERAL -1  // @@ TODO  lazy...

using namespace ajn;
using namespace qcc;

class P2pService;
static const char* P2P_SERVICE_INTERFACE_NAME = "org.alljoyn.bus.p2p.P2pInterface";
static const char* SERVICE_OBJECT_PATH = "/P2pService";
static const char* SERVICE_NAME = "org.alljoyn.bus.p2p";

static jfieldID bssidFID = NULL;
static jfieldID ssidFID = NULL;
static jfieldID attachedFID = NULL;

static BusAttachment* s_bus = NULL;
static P2pService* s_obj = NULL;


class P2pService : public BusObject {
  public:
    P2pService(BusAttachment& bus, const char* path, JavaVM* vm, jobject jobj) : BusObject(bus, path), vm(vm), jobj(jobj)
    {
        QStatus status;

        /* Add the P2p interface to this object */
        const InterfaceDescription* p2pIntf = bus.GetInterface(P2P_SERVICE_INTERFACE_NAME);
        assert(p2pIntf);
        AddInterface(*p2pIntf);

        advertiseNameMember = p2pIntf->GetMember("AdvertiseName");
        assert(advertiseNameMember);
        cancelAdvertiseNameMember = p2pIntf->GetMember("CancelAdvertiseName");
        assert(cancelAdvertiseNameMember);
        findAdvertisedNameMember = p2pIntf->GetMember("FindAdvertisedName");
        assert(findAdvertisedNameMember);
        cancelFindAdvertisedNameMember = p2pIntf->GetMember("CancelFindAdvertisedName");
        assert(cancelFindAdvertisedNameMember);
        establishLinkMember = p2pIntf->GetMember("EstablishLink");
        assert(establishLinkMember);
        releaseLinkMember = p2pIntf->GetMember("ReleaseLink");
        assert(releaseLinkMember);
        getInterfaceNameFromHandleMember = p2pIntf->GetMember("GetInterfaceNameFromHandle");
        assert(getInterfaceNameFromHandleMember);

        const MethodEntry methodEntries[] = {
            { p2pIntf->GetMember("AdvertiseName"), static_cast<MessageReceiver::MethodHandler>(&P2pService::handleAdvertiseName) },
            { p2pIntf->GetMember("CancelAdvertiseName"), static_cast<MessageReceiver::MethodHandler>(&P2pService::handleCancelAdvertiseName) },
            { p2pIntf->GetMember("FindAdvertisedName"), static_cast<MessageReceiver::MethodHandler>(&P2pService::handleFindAdvertisedName) },
            { p2pIntf->GetMember("CancelFindAdvertisedName"), static_cast<MessageReceiver::MethodHandler>(&P2pService::handleCancelFindAdvertisedName) },
            { p2pIntf->GetMember("EstablishLink"), static_cast<MessageReceiver::MethodHandler>(&P2pService::handleEstablishLink) },
            { p2pIntf->GetMember("ReleaseLink"), static_cast<MessageReceiver::MethodHandler>(&P2pService::handleReleaseLink) },
            { p2pIntf->GetMember("GetInterfaceNameFromHandle"), static_cast<MessageReceiver::MethodHandler>(&P2pService::handleGetInterfaceNameFromHandle) }
        };

        /* Add the AllJoyn method handlers to the BusObject */
        status = AddMethodHandlers(methodEntries, ARRAY_SIZE(methodEntries));
        if (ER_OK != status) {
            LOGE("P2pService(): Failed to register method handlers for P2pService (%s)", QCC_StatusText(status));
        }

        //
        // We need the JNI environment pointer to get access to the JNI functions
        // that we need to do the introspection.
        //
        JNIEnv* env = getEnv();

        //
        // Create a reference to the provided P2pHelperAndroidService Java object that remains
        // meaningful in other calls into our various functions.  We create a weak
        // reference so that we don't interfere with garbage collection.  Note well
        // that you can't use weak references directly, you must always create a
        // valid local referece from the weak reference before using it.
        //
        jhelper = env->NewWeakGlobalRef(jobj);
        if (!jhelper) {
            LOGD("P2pService(): Can't make NewWeakGlobalRef()\n");
            return;
        }

        //
        // Get a reference to the class from which the provided object was created.
        // We need the class in order to introspect its methods.
        //
        jclass clazz = env->GetObjectClass(jobj);
        if (!clazz) {
            LOGD("P2pService::P2pService(): Can't GetObjectClass()\n");
            return;
        }

        LOGI("P2pService::P2pService(): Mapping methods\n");

        //
        // Java associates method names with a method ID which we will need in order
        // to actually call the method.  We need to provide Java with a signature in
        // case the method is overloaded.  We do this once in the constructor so we
        // don't have to do possibly painfully expensive introspection calls when
        // our code is actually doing its main job.
        //
        MID_findAdvertisedName = env->GetMethodID(clazz, "FindAdvertisedName", "(Ljava/lang/String;)I");
        if (!MID_findAdvertisedName) {
            LOGD("P2pService(): Can't locate FindAdvertisedName()\n");
            return;
        }

        MID_cancelFindAdvertisedName = env->GetMethodID(clazz, "CancelFindAdvertisedName", "(Ljava/lang/String;)I");
        if (!MID_cancelFindAdvertisedName) {
            LOGD("P2pService(): Can't locate CancelFindAdvertisedName()\n");
            return;
        }

        MID_advertiseName = env->GetMethodID(clazz, "AdvertiseName", "(Ljava/lang/String;Ljava/lang/String;)I");
        if (!MID_advertiseName) {
            LOGD("P2pService(): Can't locate AdvertiseName()\n");
            return;
        }

        MID_cancelAdvertiseName = env->GetMethodID(clazz, "CancelAdvertiseName", "(Ljava/lang/String;Ljava/lang/String;)I");
        if (!MID_cancelAdvertiseName) {
            LOGD("P2pService(): Can't locate CancelAdvertiseName()\n");
            return;
        }

        MID_establishLink = env->GetMethodID(clazz, "EstablishLink", "(Ljava/lang/String;I)I");
        if (!MID_establishLink) {
            LOGD("P2pService(): Can't locate EstablishLink()\n");
            return;
        }

        MID_releaseLink = env->GetMethodID(clazz, "ReleaseLink", "(I)I");
        if (!MID_releaseLink) {
            LOGD("P2pService(): Can't locate ReleaseLink()\n");
            return;
        }

        MID_getInterfaceNameFromHandle = env->GetMethodID(clazz, "GetInterfaceNameFromHandle", "(I)Ljava/lang/String;");
        if (!MID_getInterfaceNameFromHandle) {
            LOGD("P2pService(): Can't locate GetInterfaceNameFromHandle()\n");
            return;
        }

        LOGI("P2pService::P2pService(): Mapping signals\n");

        onFoundAdvertisedNameMember = p2pIntf->GetMember("OnFoundAdvertisedName");
        onLostAdvertisedNameMember = p2pIntf->GetMember("OnLostAdvertisedName");
        onLinkEstablishedMember = p2pIntf->GetMember("OnLinkEstablished");
        onLinkErrorMember = p2pIntf->GetMember("OnLinkError");
        onLinkLostMember = p2pIntf->GetMember("OnLinkLost");

        sessionId = 0; // Where to get this?

        initialized = true;
    }

    //
    // Tell the P2P framework that the daemon wants to find names with the provided
    // prefix using pre-association service discovery.
    //
    int FindAdvertisedName(const char* namePrefix)
    {
        LOGD("FindAdvertisedName()\n");

        if (!initialized) {
            LOGD("FindAdvertisedName(): Not initialized\n");
            return ER_GENERAL;
        }

        //
        // Get a pointer we can use to talk to the JNI functions of the JVM we know
        // we must be part of.
        //
        JNIEnv* env = getEnv();

        //
        // Ask JNI to take the const char* we were given and make a new Java string
        // out of it.  This will be a normal Java (local) object and will be garbage
        // collected automagically when we are done here.
        //
        jstring jnamePrefix = env->NewStringUTF(namePrefix);
        if (env->ExceptionCheck()) {
            LOGD("FindAdvertisedName(): Exception converting parameter <namePrefix>\n");
            return ER_GENERAL;
        }

        //
        // The jhelper member is an object pointer to the Java object that will
        // actually make calls into the Android framework.  We stored it as a
        // weak object reference, though, so we didn't interfere with garbage
        // collecion.  These weak references cannot be used directly, so we need
        // to make a new local reference in order to do anything with it.
        //
        jobject jo = env->NewLocalRef(jhelper);
        if (!jo) {
            LOGD("FindAdvertisedName(): Can't get Java object\n");
            return ER_GENERAL;
        }

        //
        // Now we can call out into the Java object and method that will actually
        // do the FindAdvertisedName work.
        //
        int status = env->CallIntMethod(jo, MID_findAdvertisedName, jnamePrefix);
        if (env->ExceptionCheck()) {
            LOGD("FindAdvertisedName(): Exception calling Java\n");
            return ER_GENERAL;
        }

        //
        // Status is really only that the framework could start the operation.  The
        // only time you'll probably get an error back from the framework is if it
        // cannot possibly support Wi-Fi Direct -- for example, if it is running
        // on a Gingerbread device.
        //
        return status;
    }

    //
    // Tell the P2P framework that the daemon is no longer interested in services
    // with the provided prefix.
    //
    int CancelFindAdvertisedName(const char* namePrefix)
    {
        LOGD("CancelFindAdvertisedName()\n");

        if (!initialized) {
            LOGD("CancelFindAdvertisedName(): Not initialized\n");
            return ER_GENERAL;
        }

        JNIEnv* env = getEnv();

        jstring jnamePrefix = env->NewStringUTF(namePrefix);
        if (env->ExceptionCheck()) {
            LOGD("CancelFindAdvertisedName(): Exception converting parameter <namePrefix>\n");
            return ER_GENERAL;
        }

        jobject jo = env->NewLocalRef(jhelper);
        if (!jo) {
            LOGD("CancelFindAdvertisedName(): Can't get Java object\n");
            return ER_GENERAL;
        }

        int status = env->CallIntMethod(jo, MID_cancelFindAdvertisedName, jnamePrefix);
        if (env->ExceptionCheck()) {
            LOGD("CancelFindAdvertisedName(): Exception calling Java\n");
            return ER_GENERAL;
        }

        return status;
    }

    //
    // Call Java P2PHelperAndroidService.advertiseName
    //
    int AdvertiseName(const char* name, const char* guid)
    {
        LOGD("P2pService::AdvertiseName()\n");

        if (!initialized) {
            LOGD("P2pService::AdvertiseName(): Not initialized\n");
            return ER_GENERAL;
        }

        JNIEnv* env = getEnv();

        jstring jname = env->NewStringUTF(name);
        if (env->ExceptionCheck()) {
            LOGD("P2pService::AdvertiseName(): Exception converting parameter <name>\n");
            return ER_GENERAL;
        }

        jstring jguid = env->NewStringUTF(guid);
        if (env->ExceptionCheck()) {
            LOGD("P2pService::AdvertiseName(): Exception converting parameter <guid>\n");
            return ER_GENERAL;
        }

        jobject jo = env->NewLocalRef(jhelper);
        if (!jo) {
            LOGD("P2pService::AdvertiseName(): Can't get Java object\n");
            return ER_GENERAL;
        }

        int status = env->CallIntMethod(jo, MID_advertiseName, jname, jguid);
        if (env->ExceptionCheck()) {
            LOGD("P2pService::AdvertiseName(): Exception calling Java\n");
            return ER_GENERAL;
        }
        return status;
    }

    int CancelAdvertiseName(const char* name, const char* guid)
    {
        LOGD("CancelAdvertiseName()\n");

        if (!initialized) {
            LOGD("CancelAdvertiseName(): Not initialized\n");
            return ER_GENERAL;
        }

        JNIEnv* env = getEnv();

        jstring jname = env->NewStringUTF(name);
        if (env->ExceptionCheck()) {
            LOGD("CancelAdvertiseName(): Exception converting parameter <name>\n");
            return ER_GENERAL;
        }

        jstring jguid = env->NewStringUTF(guid);
        if (env->ExceptionCheck()) {
            LOGD("CancelAdvertiseName(): Exception converting parameter <guid>\n");
            return ER_GENERAL;
        }

        jobject jo = env->NewLocalRef(jhelper);
        if (!jo) {
            LOGD("CancelAdvertiseName(): Can't get Java object\n");
            return ER_GENERAL;
        }

        int status = env->CallIntMethod(jo, MID_cancelAdvertiseName, jname, jguid);
        if (env->ExceptionCheck()) {
            LOGD("CancelAdvertiseName(): Exception calling Java\n");
            return ER_GENERAL;
        }

        return status;
    }

    int EstablishLink(const char* device, int groupOwnerIntent)
    {
        LOGD("EstablishLink()\n");

        if (!initialized) {
            LOGD("EstablishLink(): Not initialized\n");
            return ER_GENERAL;
        }

        JNIEnv* env = getEnv();

        jstring jdevice = env->NewStringUTF(device);
        if (env->ExceptionCheck()) {
            LOGD("EstablishLink(): Exception converting parameter <device>\n");
            return ER_GENERAL;
        }

        jobject jo = env->NewLocalRef(jhelper);
        if (!jo) {
            LOGD("EstablishLink(): Can't get Java object\n");
            return ER_GENERAL;
        }

        int handle = env->CallIntMethod(jo, MID_establishLink, jdevice, groupOwnerIntent);
        if (env->ExceptionCheck()) {
            LOGD("EstablishLink(): Exception calling Java\n");
            return ER_GENERAL;
        }

        return handle;
    }

    //
    // Communicate that the daemon is done with the link identified by the provided
    // handle.
    //
    int ReleaseLink(int handle)
    {
        LOGD("ReleaseLink()\n");

        if (!initialized) {
            LOGD("ReleaseLink(): Not initialized\n");
            return ER_GENERAL;
        }

        JNIEnv* env = getEnv();

        jobject jo = env->NewLocalRef(jhelper);
        if (!jo) {
            LOGD("ReleaseLink(): Can't get Java object\n");
            return ER_GENERAL;
        }

        int status = env->CallIntMethod(jo, MID_releaseLink, handle);
        if (env->ExceptionCheck()) {
            LOGD("ReleaseLink(): Exception calling Java\n");
            return ER_GENERAL;
        }

        return status;
    }

    //
    // In order to route packets over particular links which may have link-local
    // addreses, the daemon may need to construct a zone ID to append to IP
    // addresses found during the multicast name service discovery phase.  To do
    // this, it needs the interface name of the network device used to send packets
    // to the destination.
    //
    // This function is used to get a netork interface name string corresponding to
    // the device associated with the P2P handle.
    //
    int GetInterfaceNameFromHandle(int handle, char* buffer, int buflen)
    {
        LOGD("GetInterfaceNameFromHandle()\n");

        if (!initialized) {
            LOGD("GetInterfaceNameFromHandle(): Not initialized\n");
            return ER_GENERAL;
        }

        //
        // Get a pointer we can use to talk to the JNI functions of the JVM we know
        // we must be part of.
        //
        JNIEnv* env = getEnv();

        //
        // The jhelper member is an object pointer to the Java object that will
        // actually make calls into the Android framework.  We stored it as a
        // weak object reference, though, so we didn't interfere with garbage
        // collecion.  These weak references cannot be used directly, so we need
        // to make a new local reference in order to do anything with it.
        //
        jobject jo = env->NewLocalRef(jhelper);
        if (!jo) {
            LOGD("GetInterfaceNameFromHandle(): Can't get Java object\n");
            return ER_GENERAL;
        }

        //
        // Now we can call out into the Java object and method that will actually
        // do the GetInterfaceNameFromHandle work.
        //
        jstring jname = (jstring)env->CallObjectMethod(jo, MID_getInterfaceNameFromHandle, handle);
        if (env->ExceptionCheck()) {
            LOGD("GetInterfaceNameFromHandle(): Exception calling Java\n");
            return ER_GENERAL;
        }

        if (jname) {
            const char* tmp = env->GetStringUTFChars(jname, NULL);
            strncpy(buffer, tmp, buflen);
            buffer[buflen] = '\0';
            env->ReleaseStringUTFChars(jname, tmp);
            return ER_OK;
        } else {
            return ER_GENERAL;
        }
    }

    //
    // AllJoyn Method handlers: Unmarshall the AllJoyn method parameters
    //
    void handleFindAdvertisedName(const InterfaceDescription::Member* member, Message& msg)
    {
        String namePrefix = msg->GetArg(0)->v_string.str;

        LOGD("handleFindAdvertisedName called from %s with: %s\n", msg->GetSender(), namePrefix.c_str());

        int result = FindAdvertisedName(namePrefix.c_str());

        MsgArg reply("i", 1, result);
        QStatus status = MethodReply(msg, &reply, 1);
        if (ER_OK != status) {
            LOGE("handleFindAdvertisedName: Error sending reply (%s)", QCC_StatusText(status));
        }
    }

    void handleCancelFindAdvertisedName(const InterfaceDescription::Member* member, Message& msg)
    {
        String namePrefix = msg->GetArg(0)->v_string.str;

        LOGD("handleCancelFindAdvertisedName called from %s with: %s\n", msg->GetSender(), namePrefix.c_str());

        int result = CancelFindAdvertisedName(namePrefix.c_str());

        MsgArg reply("i", 1, result);
        QStatus status = MethodReply(msg, &reply, 1);
        if (ER_OK != status) {
            LOGE("handleCancelAdvertisedName: Error sending reply (%s)", QCC_StatusText(status));
        }
    }

    void handleAdvertiseName(const InterfaceDescription::Member* member, Message& msg)
    {
        String name = msg->GetArg(0)->v_string.str;
        String guid = msg->GetArg(1)->v_string.str;

        LOGD("handleAdvertiseName called from %s with: %s, %s\n", msg->GetSender(), name.c_str(), guid.c_str());

        int result = AdvertiseName(name.c_str(), guid.c_str());

        MsgArg reply("i", 1, result);
        QStatus status = MethodReply(msg, &reply, 1);
        if (ER_OK != status) {
            LOGE("handleAdvertiseName: Error sending reply (%s)", QCC_StatusText(status));
        }
    }

    void handleCancelAdvertiseName(const InterfaceDescription::Member* member, Message& msg)
    {
        String name = msg->GetArg(0)->v_string.str;
        String guid = msg->GetArg(1)->v_string.str;

        LOGD("handleCancelAdvertiseName called from %s with: %s, %s\n", msg->GetSender(), name.c_str(), guid.c_str());

        int result = CancelAdvertiseName(name.c_str(), guid.c_str());

        MsgArg reply("i", 1, result);
        QStatus status = MethodReply(msg, &reply, 1);
        if (ER_OK != status) {
            LOGE("handleCancelAdvertiseName: Error sending reply (%s)", QCC_StatusText(status));
        }
    }

    void handleEstablishLink(const InterfaceDescription::Member* member, Message& msg)
    {
        String device = msg->GetArg(0)->v_string.str;
        int intent = msg->GetArg(1)->v_int32;

        LOGD("handleEstablishLink called from %s with: %s, %d\n", msg->GetSender(), device.c_str(), intent);

        int result = EstablishLink(device.c_str(), intent);

        MsgArg reply("i", 1, result);
        QStatus status = MethodReply(msg, &reply, 1);
        if (ER_OK != status) {
            LOGE("handleEstablishLink: Error sending reply (%s)", QCC_StatusText(status));
        }
    }

    void handleReleaseLink(const InterfaceDescription::Member* member, Message& msg)
    {
        int handle = msg->GetArg(1)->v_int32;

        LOGD("handleReleaseLink called from %s with: %d\n", msg->GetSender(), handle);

        int result = ReleaseLink(handle);

        MsgArg reply("i", 1, result);
        QStatus status = MethodReply(msg, &reply, 1);
        if (ER_OK != status) {
            LOGE("handleReleaseLink: Error sending reply (%s)", QCC_StatusText(status));
        }
    }

    void handleGetInterfaceNameFromHandle(const InterfaceDescription::Member* member, Message& msg)
    {
        int handle = msg->GetArg(1)->v_int32;
        char buf[64];

        LOGD("handleGetInterfaceNameFromHandle called from %s with: %d\n", msg->GetSender(), handle);

        int result = GetInterfaceNameFromHandle(handle, buf, 64);

        MsgArg reply("s", 1, buf);
        QStatus status = MethodReply(msg, &reply, 1);
        if (ER_OK != status) {
            LOGE("handleGetInterfaceNameFromHandle: Error sending reply (%s)", QCC_StatusText(status));
        }
    }

    void sendOnFoundAdvertisedName(const char* name, const char* namePrefix, const char* guid, const char* device) {
        MsgArg args[4];
        args[0].Set("s", name);
        args[1].Set("s", namePrefix);
        args[2].Set("s", guid);
        args[3].Set("s", device);

        LOGI("sendOnFoundAdvertisedName(%s, %s, %s, %s)", name, namePrefix, guid, device);
        QStatus status = Signal(NULL, sessionId, *onFoundAdvertisedNameMember, args, 4, 0);
        if (ER_OK != status) {
            LOGE("sendOnFoundAdvertisedName: Error sending signal (%s)", QCC_StatusText(status));
        }
    }

    void sendOnLostAdvertisedName(const char* name, const char* namePrefix, const char* guid, const char* device) {
        MsgArg args[4];
        args[0].Set("s", name);
        args[1].Set("s", namePrefix);
        args[2].Set("s", guid);
        args[3].Set("s", device);

        LOGI("sendOnLostAdvertisedName(%s, %s, %s, %s)", name, namePrefix, guid, device);
        QStatus status = Signal(NULL, sessionId, *onLostAdvertisedNameMember, args, 4, 0);
        if (ER_OK != status) {
            LOGE("sendOnLostAdvertisedName: Error sending signal (%s)", QCC_StatusText(status));
        }
    }

    void sendOnLinkEstablished(int handle) {
        MsgArg arg("i", handle);

        LOGI("sendOnLinkEstablished(%d)", handle);
        QStatus status = Signal(NULL, sessionId, *onLinkEstablishedMember, &arg, 1, 0);
        if (ER_OK != status) {
            LOGE("sendOnLinkEstablished: Error sending signal (%s)", QCC_StatusText(status));
        }
    }

    void sendOnLinkError(int handle, int error) {
        MsgArg args[2];
        args[0].Set("i", handle);
        args[1].Set("i", error);

        LOGI("sendOnLinkError(%d, %d)", handle, error);
        QStatus status = Signal(NULL, sessionId, *onLinkErrorMember, args, 2, 0);
        if (ER_OK != status) {
            LOGE("sendOnLinkError: Error sending signal (%s)", QCC_StatusText(status));
        }
    }

    void sendOnLinkLost(int handle) {
        MsgArg arg("i", handle);

        LOGI("sendOnLinkLost(%d)", handle);
        QStatus status = Signal(NULL, sessionId, *onLinkLostMember, &arg, 1, 0);
        if (ER_OK != status) {
            LOGE("sendOnLinkLost: Error sending signal (%s)", QCC_StatusText(status));
        }
    }

    jobject getJObject() {
        return jobj;
    }

    //
    // All JNI functions are accessed indirectly through a pointer provided by the
    // Java virtual machine.  When working with this pointer, we have to be careful
    // about arranging to get the current thread associated with the environment so
    // we provide a convenience function that does what we need.
    //
    // All JNI functions are accessed indirectly through a pointer provided by the
    // Java virtual machine.  When working with this pointer, we have to be careful
    // about arranging to get the current thread associated with the environment so
    // we provide a convenience function that does what we need.
    //
    JNIEnv* getEnv()
    {
        LOGD("getEnv()\n");
        JNIEnv* env;
        jint ret = vm->GetEnv((void**)&env, JNI_VERSION_1_2);
        if (ret == JNI_EDETACHED) {
            ret = vm->AttachCurrentThread(&env, NULL);
        }
        return env;
    }

  private:
    JavaVM* vm;
    jobject jobj;
    SessionId sessionId;
    const InterfaceDescription::Member* findAdvertisedNameMember;
    const InterfaceDescription::Member* cancelFindAdvertisedNameMember;
    const InterfaceDescription::Member* advertiseNameMember;
    const InterfaceDescription::Member* cancelAdvertiseNameMember;
    const InterfaceDescription::Member* establishLinkMember;
    const InterfaceDescription::Member* releaseLinkMember;
    const InterfaceDescription::Member* getInterfaceNameFromHandleMember;
    const InterfaceDescription::Member* onFoundAdvertisedNameMember;
    const InterfaceDescription::Member* onLostAdvertisedNameMember;
    const InterfaceDescription::Member* onLinkEstablishedMember;
    const InterfaceDescription::Member* onLinkErrorMember;
    const InterfaceDescription::Member* onLinkLostMember;
    bool initialized;
    jobject jhelper;
    jmethodID MID_findAdvertisedName;
    jmethodID MID_cancelFindAdvertisedName;
    jmethodID MID_advertiseName;
    jmethodID MID_cancelAdvertiseName;
    jmethodID MID_establishLink;
    jmethodID MID_releaseLink;
    jmethodID MID_getInterfaceNameFromHandle;

};



#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     org_alljoyn_bus_p2p_service_P2pHelperService
 * Method:    jniInit
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jboolean JNICALL Java_org_alljoyn_bus_p2p_service_P2pHelperAndroidService_00024P2pHelperService_jniOnCreate(JNIEnv*env, jobject jobj) {


    QStatus status = ER_OK;
    const char* daemonAddr = "unix:abstract=alljoyn";
    InterfaceDescription* p2pIntf = NULL;
    JavaVM* vm;
    jobject jglobalObj = NULL;

    jglobalObj = env->NewGlobalRef(jobj);

    env->GetJavaVM(&vm);

    /* Create message bus */
    s_bus = new BusAttachment("P2pHelperService", true);
    if (!s_bus) {
        LOGE("new BusAttachment failed");
    } else {
        /* Create org.alljoyn.bus.p2p.service interface */
        status = s_bus->CreateInterface(P2P_SERVICE_INTERFACE_NAME, p2pIntf);
        if (ER_OK != status) {
            LOGE("Failed to create interface \"%s\" (%s)", P2P_SERVICE_INTERFACE_NAME, QCC_StatusText(status));
        } else {

            p2pIntf->AddMethod("FindAdvertisedName",         "s",  "i",  "namePrefix,result");
            p2pIntf->AddMethod("CancelFindAdvertisedName",   "s",  "i",  "namePrefix,result");
            p2pIntf->AddMethod("AdvertiseName",              "ss", "i",  "name,guid,result");
            p2pIntf->AddMethod("CancelAdvertiseName",        "ss", "i",  "name,guid,result");
            p2pIntf->AddMethod("EstablishLink",              "si", "i",  "device,intent,result");
            p2pIntf->AddMethod("ReleaseLink",                "i",  "i",  "handle,result");
            p2pIntf->AddMethod("GetInterfaceNameFromHandle", "i",  "s",  "handle,interface");

            p2pIntf->AddSignal("OnFoundAdvertisedName",      "ssss", "name,namePrefix,guid,device");
            p2pIntf->AddSignal("OnLostAdvertisedName",       "ssss", "name,namePrefix,guid,device");
            p2pIntf->AddSignal("OnLinkEstablished",          "i",    "handle");
            p2pIntf->AddSignal("OnLinkError",                "ii",   "handle,error");
            p2pIntf->AddSignal("OnLinkLost",                 "i",    "handle");

            p2pIntf->Activate();

            /* Register service object */
            s_obj = new P2pService(*s_bus, SERVICE_OBJECT_PATH, vm, jglobalObj);
            status = s_bus->RegisterBusObject(*s_obj);
            if (ER_OK != status) {
                LOGE("BusAttachment::RegisterBusObject failed (%s)", QCC_StatusText(status));
            } else {
                /* Start the msg bus */
                status = s_bus->Start();
                if (ER_OK != status) {
                    LOGE("BusAttachment::Start failed (%s)", QCC_StatusText(status));
                } else {
                    /* Connect to the daemon */
                    status = s_bus->Connect(daemonAddr);
                    if (ER_OK != status) {
                        LOGE("BusAttachment::Connect(\"%s\") failed (%s)", daemonAddr, QCC_StatusText(status));
                        s_bus->Disconnect(daemonAddr);
                        s_bus->UnregisterBusObject(*s_obj);
                        delete s_obj;
                        s_obj = NULL;
                        return false;
                    } else {
                        LOGE("BusAttachment::Connect(\"%s\") SUCCEEDED (%s)", daemonAddr, QCC_StatusText(status));
                    }
                }
            }
        }
    }

    /* Request name */
    status = s_bus->RequestName(SERVICE_NAME, DBUS_NAME_FLAG_DO_NOT_QUEUE);
    if (ER_OK != status) {
        LOGE("RequestName(%s) failed (status=%s)\n", SERVICE_NAME, QCC_StatusText(status));
        status = (status == ER_OK) ? ER_FAIL : status;
    } else {
        LOGI("Request Name was successful");
    }

    return true;
}

/*
 * Class:     org_alljoyn_bus_p2p_service_P2pHelperService
 * Method:    jniShutdown
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_alljoyn_bus_p2p_service_P2pHelperAndroidService_00024P2pHelperService_jniOnDestroy(JNIEnv* env, jobject jobj) {
    LOGE("OnDestroy");
    delete s_obj;
    s_obj = NULL;
}

JNIEXPORT void JNICALL Java_org_alljoyn_bus_p2p_service_P2pHelperAndroidService_00024P2pHelperService_jniOnFoundAdvertisedName(JNIEnv* env, jobject jobj, jstring name, jstring namePrefix, jstring guid, jstring device) {

    if (s_obj) {
        const char* cName = env->GetStringUTFChars(name, NULL);
        const char* cNamePrefix = env->GetStringUTFChars(namePrefix, NULL);
        const char* cGuid = env->GetStringUTFChars(guid, NULL);
        const char* cDevice = env->GetStringUTFChars(device, NULL);

        s_obj->sendOnFoundAdvertisedName(cName, cNamePrefix, cGuid, cDevice);

        env->ReleaseStringUTFChars(name, cName);
        env->ReleaseStringUTFChars(namePrefix, cNamePrefix);
        env->ReleaseStringUTFChars(guid, cGuid);
        env->ReleaseStringUTFChars(device, cDevice);
    } else {
        LOGE("onFoundAdvertisedName: Bad P2pService pointer %p", s_obj);
    }

}

JNIEXPORT void JNICALL Java_org_alljoyn_bus_p2p_service_P2pHelperAndroidService_00024P2pHelperService_jniOnLostAdvertisedName(JNIEnv* env, jobject jobj, jstring name, jstring namePrefix, jstring guid, jstring device) {

    if (s_obj) {
        const char* cName = env->GetStringUTFChars(name, NULL);
        const char* cNamePrefix = env->GetStringUTFChars(namePrefix, NULL);
        const char* cGuid = env->GetStringUTFChars(guid, NULL);
        const char* cDevice = env->GetStringUTFChars(device, NULL);

        s_obj->sendOnLostAdvertisedName(cName, cNamePrefix, cGuid, cDevice);

        env->ReleaseStringUTFChars(name, cName);
        env->ReleaseStringUTFChars(namePrefix, cNamePrefix);
        env->ReleaseStringUTFChars(guid, cGuid);
        env->ReleaseStringUTFChars(device, cDevice);
    } else {
        LOGE("onLostAdvertisedName: Bad P2pService pointer %p", s_obj);
    }
}

JNIEXPORT void JNICALL Java_org_alljoyn_bus_p2p_service_P2pHelperAndroidService_00024P2pHelperService_jniOnLinkEstablished(JNIEnv* env, jobject jobj, jint handle) {

    if (s_obj) {
        s_obj->sendOnLinkEstablished(handle);
    } else {
        LOGE("onLinkEstablished: Bad P2pService pointer %p", s_obj);
    }
}

JNIEXPORT void JNICALL Java_org_alljoyn_bus_p2p_service_P2pHelperAndroidService_00024P2pHelperService_jniOnLinkError(JNIEnv* env, jobject jobj, jint handle, jint error) {

    if (s_obj) {
        s_obj->sendOnLinkError(handle, error);
    } else {
        LOGE("onLinkError: Bad P2pService pointer %p", s_obj);
    }
}

JNIEXPORT void JNICALL Java_org_alljoyn_bus_p2p_service_P2pHelperAndroidService_00024P2pHelperService_jniOnLinkLost(JNIEnv* env, jobject jobj, jint handle) {

    if (s_obj) {
        s_obj->sendOnLinkLost(handle);
    } else {
        LOGE("onLinkLost: Bad P2pService pointer %p", s_obj);
    }
}


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm,
                                  void* reserved)
{
    /* Set AllJoyn logging */
    //QCC_SetLogLevels("ALLJOYN=7;ALL=1");
    //QCC_UseOSLogging(true);

    JNIEnv* env;
    LOGD("Before getting environment\n");
    if (!vm) {
        LOGD("VM is NULL\n");
    }
    jint jret = vm->GetEnv((void**)&env, JNI_VERSION_1_2);
    if (JNI_EDETACHED == jret) {
        vm->AttachCurrentThread(&env, NULL);
        LOGD("Attached to VM thread \n");
    }
    LOGD("After getting environment\n");

    jclass clazz;
    clazz = env->FindClass("org/alljoyn/bus/p2p/service/P2pHelperAndroidService$P2pHelperService");
    if (!clazz) {
        LOGD("*********************** error while loading the class *******************");
        env->ExceptionDescribe();
        return JNI_ERR;
    } else {
        LOGD("org/alljoyn/jni/P2pHelperService loaded SUCCESSFULLY");
    }

    return JNI_VERSION_1_2;
}


#ifdef __cplusplus
}
#endif

