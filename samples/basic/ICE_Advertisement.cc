/**
 * @file
 * @brief Sample implementation to test the sending of ICE Advertisements to the Rendezvous Server.
 *
 * This sample will show how to set up an AllJoyn services that will registered with the
 * wellknown names 'org.alljoyn.Bus.method_sample' and 'org.alljoyn.Bus.org.alljoyn.Bus.ice_sample'.
 * It would first send both the advertisements and then revoke them one after the other so that the
 * interface support for these operations may be verified.
 *
 */

/******************************************************************************
 * Copyright 2012, Qualcomm Innovation Center, Inc.
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
#include <qcc/platform.h>

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <vector>

#include <qcc/String.h>
#include <qcc/Thread.h>

#include <alljoyn/BusAttachment.h>
#include <alljoyn/DBusStd.h>
#include <alljoyn/AllJoynStd.h>
#include <alljoyn/BusObject.h>
#include <alljoyn/MsgArg.h>
#include <alljoyn/version.h>

#include <Status.h>

using namespace std;
using namespace qcc;
using namespace ajn;

class MyBusListenerOne;
class MyBusListenerTwo;

/** Static top level message bus object */
static BusAttachment* g_msgBus_one = NULL;
static BusAttachment* g_msgBus_two = NULL;

static MyBusListenerOne* s_busListener_one = NULL;
static MyBusListenerTwo* s_busListener_two = NULL;

/*constants*/
static const char* INTERFACE_NAME_ONE = "org.alljoyn.Bus.method_sample";
static const char* SERVICE_NAME_ONE = "org.alljoyn.Bus.method_sample";
static const char* SERVICE_PATH_ONE = "/method_sample";
static const SessionPort SERVICE_PORT_ONE = 25;

static const char* INTERFACE_NAME_TWO = "org.alljoyn.Bus.ice_sample";
static const char* SERVICE_NAME_TWO = "org.alljoyn.Bus.ice_sample";
static const char* SERVICE_PATH_TWO = "/ice_sample";
static const SessionPort SERVICE_PORT_TWO = 26;

static volatile sig_atomic_t g_interrupt_one = false;
static volatile sig_atomic_t g_interrupt_two = false;

/** Signal handler
 * with out the signal handler the program will exit without stopping the bus
 * when kill signal is received.  (i.e. [Ctrl + c] is pressed) not using this
 * may result in a memory leak if [cont + c] is used to end this program.
 */
static void SigIntHandler(int sig)
{
    if (NULL != g_msgBus_one) {
        g_interrupt_one = true;
    }

    if (NULL != g_msgBus_two) {
        g_interrupt_two = true;
    }
}

class ICEAdvertisementOneObject : public BusObject {
  public:
    ICEAdvertisementOneObject(BusAttachment& bus, const char* path) :
        BusObject(bus, path)
    {
        /** Add the test interface to this object */
        const InterfaceDescription* exampleIntf = bus.GetInterface(INTERFACE_NAME_ONE);
        assert(exampleIntf);
        AddInterface(*exampleIntf);

        /** Register the method handlers with the object */
        const MethodEntry methodEntries[] = {
            { exampleIntf->GetMember("cat"), static_cast<MessageReceiver::MethodHandler>(&ICEAdvertisementOneObject::Cat) }
        };
        QStatus status = AddMethodHandlers(methodEntries, sizeof(methodEntries) / sizeof(methodEntries[0]));
        if (ER_OK != status) {
            printf("Failed to register method handlers for ICEAdvertisementOneObject");
        }
    }

    void ObjectRegistered()
    {
        BusObject::ObjectRegistered();
        printf("ObjectRegistered has been called\n");
    }


    void Cat(const InterfaceDescription::Member* member, Message& msg)
    {
        /* Concatenate the two input strings and reply with the result. */
        qcc::String inStr1 = msg->GetArg(0)->v_string.str;
        qcc::String inStr2 = msg->GetArg(1)->v_string.str;
        qcc::String outStr = inStr1 + inStr2;

        MsgArg outArg("s", outStr.c_str());
        QStatus status = MethodReply(msg, &outArg, 1);
        if (ER_OK != status) {
            printf("Ping: Error sending reply\n");
        }
    }
};

class ICEAdvertisementTwoObject : public BusObject {
  public:
    ICEAdvertisementTwoObject(BusAttachment& bus, const char* path) :
        BusObject(bus, path)
    {
        /** Add the test interface to this object */
        const InterfaceDescription* exampleIntf = bus.GetInterface(INTERFACE_NAME_TWO);
        assert(exampleIntf);
        AddInterface(*exampleIntf);

        /** Register the method handlers with the object */
        const MethodEntry methodEntries[] = {
            { exampleIntf->GetMember("cat"), static_cast<MessageReceiver::MethodHandler>(&ICEAdvertisementTwoObject::Cat) }
        };
        QStatus status = AddMethodHandlers(methodEntries, sizeof(methodEntries) / sizeof(methodEntries[0]));
        if (ER_OK != status) {
            printf("Failed to register method handlers for ICEAdvertisementTwoObject");
        }
    }

    void ObjectRegistered()
    {
        BusObject::ObjectRegistered();
        printf("ObjectRegistered has been called\n");
    }


    void Cat(const InterfaceDescription::Member* member, Message& msg)
    {
        /* Concatenate the two input strings and reply with the result. */
        qcc::String inStr1 = msg->GetArg(0)->v_string.str;
        qcc::String inStr2 = msg->GetArg(1)->v_string.str;
        qcc::String outStr = inStr1 + inStr2;

        MsgArg outArg("s", outStr.c_str());
        QStatus status = MethodReply(msg, &outArg, 1);
        if (ER_OK != status) {
            printf("Ping: Error sending reply\n");
        }
    }
};


class MyBusListenerOne : public BusListener, public SessionPortListener {
    void NameOwnerChanged(const char* busName, const char* previousOwner, const char* newOwner)
    {
        if (newOwner && (0 == strcmp(busName, SERVICE_NAME_ONE))) {
            printf("NameOwnerChanged: name=%s, oldOwner=%s, newOwner=%s\n",
                   busName,
                   previousOwner ? previousOwner : "<none>",
                   newOwner ? newOwner : "<none>");
        }
    }
    bool AcceptSessionJoiner(SessionPort sessionPort, const char* joiner, const SessionOpts& opts)
    {
        if (sessionPort != SERVICE_PORT_ONE) {
            printf("Rejecting join attempt on unexpected session port %d\n", sessionPort);
            return false;
        }
        printf("Accepting join session request from %s (opts.proximity=%x, opts.traffic=%x, opts.transports=%x)\n",
               joiner, opts.proximity, opts.traffic, opts.transports);
        return true;
    }
};

class MyBusListenerTwo : public BusListener, public SessionPortListener {
    void NameOwnerChanged(const char* busName, const char* previousOwner, const char* newOwner)
    {
        if (newOwner && (0 == strcmp(busName, SERVICE_NAME_TWO))) {
            printf("NameOwnerChanged: name=%s, oldOwner=%s, newOwner=%s\n",
                   busName,
                   previousOwner ? previousOwner : "<none>",
                   newOwner ? newOwner : "<none>");
        }
    }
    bool AcceptSessionJoiner(SessionPort sessionPort, const char* joiner, const SessionOpts& opts)
    {
        if (sessionPort != SERVICE_PORT_TWO) {
            printf("Rejecting join attempt on unexpected session port %d\n", sessionPort);
            return false;
        }
        printf("Accepting join session request from %s (opts.proximity=%x, opts.traffic=%x, opts.transports=%x)\n",
               joiner, opts.proximity, opts.traffic, opts.transports);
        return true;
    }
};

/** Main entry point */
int main(int argc, char** argv, char** envArg)
{
    QStatus status = ER_OK;

    printf("AllJoyn Library version: %s\n", ajn::GetVersion());
    printf("AllJoyn Library build info: %s\n", ajn::GetBuildInfo());

    /* Install SIGINT handler */
    signal(SIGINT, SigIntHandler);

    const char* connectArgs = getenv("BUS_ADDRESS");
    if (connectArgs == NULL) {
#ifdef _WIN32
        connectArgs = "tcp:addr=127.0.0.1,port=9955";
#else
        connectArgs = "unix:abstract=alljoyn";
#endif
    }

    // Advertise the first Service
    /* Create message bus */
    g_msgBus_one = new BusAttachment("myICEAdvertismentAppOne", true);


    /* Add org.alljoyn.Bus.method_sample interface */
    InterfaceDescription* testIntf = NULL;
    status = g_msgBus_one->CreateInterface(INTERFACE_NAME_ONE, testIntf);
    if (status == ER_OK) {
        printf("Interface Created for org.alljoyn.Bus.method_sample.\n");
        testIntf->AddMethod("cat", "ss",  "s", "inStr1,inStr2,outStr", 0);
        testIntf->Activate();
    } else {
        printf("Failed to create interface 'org.alljoyn.Bus.method_sample'\n");
    }

    /* Register a bus listener */
    if (ER_OK == status) {
        s_busListener_one = new MyBusListenerOne();
        g_msgBus_one->RegisterBusListener(*s_busListener_one);
    }

    ICEAdvertisementOneObject testObj(*g_msgBus_one, SERVICE_PATH_ONE);

    /* Start the msg bus */
    status = g_msgBus_one->Start();
    if (ER_OK == status) {
        printf("BusAttachement started for org.alljoyn.Bus.method_sample.\n");
        /* Register  local objects and connect to the daemon */
        g_msgBus_one->RegisterBusObject(testObj);

        /* Create the client-side endpoint */
        status = g_msgBus_one->Connect(connectArgs);
        if (ER_OK != status) {
            printf("Failed to connect to \"%s\" for org.alljoyn.Bus.method_sample\n", connectArgs);
            exit(1);
        } else {
            printf("Connected to '%s' for org.alljoyn.Bus.method_sample\n", connectArgs);
        }
    } else {
        printf("BusAttachment::Start failed for org.alljoyn.Bus.method_sample\n");
    }

    /*
     * Advertise this service on the bus
     * There are three steps to advertising this service on the bus
     * 1) Request a well-known name that will be used by the client to discover
     *    this service
     * 2) Create a session
     * 3) Advertise the well-known name
     */
    /* Request name */
    if (ER_OK == status) {
        uint32_t flags = DBUS_NAME_FLAG_REPLACE_EXISTING | DBUS_NAME_FLAG_DO_NOT_QUEUE;
        QStatus status = g_msgBus_one->RequestName(SERVICE_NAME_ONE, flags);
        if (ER_OK != status) {
            printf("RequestName(%s) failed (status=%s) for org.alljoyn.Bus.method_sample\n", SERVICE_NAME_ONE, QCC_StatusText(status));
        }
    }

    /* Create session */
    SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
    if (ER_OK == status) {
        SessionPort sp = SERVICE_PORT_ONE;
        status = g_msgBus_one->BindSessionPort(sp, opts, *s_busListener_one);
        if (ER_OK != status) {
            printf("BindSessionPort failed (%s) for org.alljoyn.Bus.method_sample\n", QCC_StatusText(status));
        }
    }

    /* Advertise name */
    if (ER_OK == status) {
        status = g_msgBus_one->AdvertiseName(SERVICE_NAME_ONE, opts.transports);
        if (status != ER_OK) {
            printf("Failed to advertise name %s (%s) for org.alljoyn.Bus.method_sample\n", SERVICE_NAME_ONE, QCC_StatusText(status));
        }
    }

    if (ER_OK == status) {
        qcc::Sleep(5000);
    }

    // Advertise the second Service
    /* Create message bus */
    g_msgBus_two = new BusAttachment("myICEAdvertismentAppTwo", true);


    /* Add org.alljoyn.Bus.ice_sample interface */
    InterfaceDescription* testIntftwo = NULL;
    status = g_msgBus_two->CreateInterface(INTERFACE_NAME_TWO, testIntftwo);
    if (status == ER_OK) {
        printf("Interface Created for org.alljoyn.Bus.ice_sample.\n");
        testIntftwo->AddMethod("cat", "ss",  "s", "inStr1,inStr2,outStr", 0);
        testIntftwo->Activate();
    } else {
        printf("Failed to create interface 'org.alljoyn.Bus.ice_sample'\n");
    }

    /* Register a bus listener */
    if (ER_OK == status) {
        s_busListener_two = new MyBusListenerTwo();
        g_msgBus_one->RegisterBusListener(*s_busListener_two);
    }

    ICEAdvertisementTwoObject testObjTwo(*g_msgBus_two, SERVICE_PATH_TWO);

    /* Start the msg bus */
    status = g_msgBus_two->Start();
    if (ER_OK == status) {
        printf("BusAttachement started for org.alljoyn.Bus.ice_sample.\n");
        /* Register  local objects and connect to the daemon */
        g_msgBus_two->RegisterBusObject(testObjTwo);

        /* Create the client-side endpoint */
        status = g_msgBus_two->Connect(connectArgs);
        if (ER_OK != status) {
            printf("Failed to connect to \"%s\" for org.alljoyn.Bus.ice_sample\n", connectArgs);
            exit(1);
        } else {
            printf("Connected to '%s' for org.alljoyn.Bus.ice_sample\n", connectArgs);
        }
    } else {
        printf("BusAttachment::Start failed for org.alljoyn.Bus.ice_sample\n");
    }

    /*
     * Advertise this service on the bus
     * There are three steps to advertising this service on the bus
     * 1) Request a well-known name that will be used by the client to discover
     *    this service
     * 2) Create a session
     * 3) Advertise the well-known name
     */
    /* Request name */
    if (ER_OK == status) {
        uint32_t flags = DBUS_NAME_FLAG_REPLACE_EXISTING | DBUS_NAME_FLAG_DO_NOT_QUEUE;
        QStatus status = g_msgBus_two->RequestName(SERVICE_NAME_TWO, flags);
        if (ER_OK != status) {
            printf("RequestName(%s) failed (status=%s) for org.alljoyn.Bus.ice_sample\n", SERVICE_NAME_TWO, QCC_StatusText(status));
        }
    }

    /* Create session */
    if (ER_OK == status) {
        SessionPort sp = SERVICE_PORT_TWO;
        status = g_msgBus_two->BindSessionPort(sp, opts, *s_busListener_two);
        if (ER_OK != status) {
            printf("BindSessionPort failed (%s) for org.alljoyn.Bus.ice_sample\n", QCC_StatusText(status));
        }
    }

    /* Advertise name */
    if (ER_OK == status) {
        status = g_msgBus_two->AdvertiseName(SERVICE_NAME_TWO, opts.transports);
        if (status != ER_OK) {
            printf("Failed to advertise name %s (%s) for org.alljoyn.Bus.ice_sample\n", SERVICE_NAME_TWO, QCC_StatusText(status));
        }
    }

    if (ER_OK == status) {
        qcc::Sleep(5000);
    }

    // Stop Advertising org.alljoyn.Bus.method_sample
    if (ER_OK == status) {
        status = g_msgBus_one->CancelAdvertiseName(SERVICE_NAME_ONE, opts.transports);
        if (status != ER_OK) {
            printf("Failed to cancel advertise name %s (%s) for org.alljoyn.Bus.method_sample\n", SERVICE_NAME_ONE, QCC_StatusText(status));
        }
    }

    if (ER_OK == status) {
        qcc::Sleep(5000);
    }

    // Stop Advertising org.alljoyn.Bus.ice_sample
    if (ER_OK == status) {
        status = g_msgBus_two->CancelAdvertiseName(SERVICE_NAME_TWO, opts.transports);
        if (status != ER_OK) {
            printf("Failed to cancel advertise name %s (%s) for org.alljoyn.Bus.ice_sample\n", SERVICE_NAME_TWO, QCC_StatusText(status));
        }
    }

    if (ER_OK == status) {
        qcc::Sleep(10000);
    }

    /* Clean up msg bus */
    if (g_msgBus_one) {
        BusAttachment* deleteMe = g_msgBus_one;
        g_msgBus_one = NULL;
        delete deleteMe;
    }
    if (g_msgBus_two) {
        BusAttachment* deleteMe = g_msgBus_two;
        g_msgBus_two = NULL;
        delete deleteMe;
    }
    return (int) status;
}
