/**
 * @file
 * @brief  Sample implementation of two AllJoyn Clients that start discovering one after
 * another over the ICE transport and then stop discovering one after another.
 */

/******************************************************************************
 *
 *
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
#include <qcc/platform.h>

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <vector>

#include <qcc/String.h>
#include <qcc/Thread.h>

#include <alljoyn/BusAttachment.h>
#include <alljoyn/version.h>
#include <alljoyn/AllJoynStd.h>
#include <Status.h>

using namespace std;
using namespace qcc;
using namespace ajn;

/** Static top level message bus object */
static BusAttachment* g_msgBus_one = NULL;
static BusAttachment* g_msgBus_two = NULL;

/*constants*/
static const char* INTERFACE_NAME_ONE = "org.alljoyn.Bus.pmethod_sample";
static const char* SERVICE_NAME_ONE = "org.alljoyn.Bus.pmethod_sample";
static const SessionPort SERVICE_PORT_ONE = 25;

static const char* INTERFACE_NAME_TWO = "org.alljoyn.Bus.pice_sample";
static const char* SERVICE_NAME_TWO = "org.alljoyn.Bus.pice_sample";
static const SessionPort SERVICE_PORT_TWO = 26;

static bool s_joinComplete_one = false;
static SessionId s_sessionId_one = 0;

static bool s_joinComplete_two = false;
static SessionId s_sessionId_two = 0;

static volatile sig_atomic_t g_interrupt_one = false;
static volatile sig_atomic_t g_interrupt_two = false;

/** Signal handler */
static void SigIntHandler(int sig)
{
    if (NULL != g_msgBus_one) {
        g_interrupt_one = true;
    }

    if (NULL != g_msgBus_two) {
        g_interrupt_two = true;
    }
    exit(0);
}

/** AllJoynListener receives discovery events from AllJoyn */
class MyBusListenerOne : public BusListener, public SessionListener {
  public:
    void FoundAdvertisedName(const char* name, TransportMask transport, const char* namePrefix)
    {
        printf("FoundAdvertisedName(name=%s, prefix=%s)\n", name, namePrefix);
        if (0 == strcmp(name, SERVICE_NAME_ONE)) {
            /* We found a remote bus that is advertising basic service's  well-known name so connect to it */
            SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
            QStatus status = g_msgBus_one->JoinSession(name, SERVICE_PORT_ONE, this, s_sessionId_one, opts);
            if (ER_OK != status) {
                printf("JoinSession failed (status=%s) for org.alljoyn.Bus.method_sample\n", QCC_StatusText(status));
            } else {
                printf("JoinSession SUCCESS (Session id=%d) for org.alljoyn.Bus.method_sample\n", s_sessionId_one);
            }
        }
        s_joinComplete_one = true;
    }

    void NameOwnerChanged(const char* busName, const char* previousOwner, const char* newOwner)
    {
        if (newOwner && (0 == strcmp(busName, SERVICE_NAME_ONE))) {
            printf("NameOwnerChanged: name=%s, oldOwner=%s, newOwner=%s\n",
                   busName,
                   previousOwner ? previousOwner : "<none>",
                   newOwner ? newOwner : "<none>");
        }
    }
};

/** AllJoynListener receives discovery events from AllJoyn */
class MyBusListenerTwo : public BusListener, public SessionListener {
  public:
    void FoundAdvertisedName(const char* name, TransportMask transport, const char* namePrefix)
    {
        printf("FoundAdvertisedName(name=%s, prefix=%s)\n", name, namePrefix);
        if (0 == strcmp(name, SERVICE_NAME_TWO)) {
            /* We found a remote bus that is advertising basic service's  well-known name so connect to it */
            SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
            QStatus status = g_msgBus_two->JoinSession(name, SERVICE_PORT_TWO, this, s_sessionId_two, opts);
            if (ER_OK != status) {
                printf("JoinSession failed (status=%s) for org.alljoyn.Bus.ice_sample\n", QCC_StatusText(status));
            } else {
                printf("JoinSession SUCCESS (Session id=%d) for org.alljoyn.Bus.ice_sample\n", s_sessionId_two);
            }
        }
        s_joinComplete_two = true;
    }

    void NameOwnerChanged(const char* busName, const char* previousOwner, const char* newOwner)
    {
        if (newOwner && (0 == strcmp(busName, SERVICE_NAME_TWO))) {
            printf("NameOwnerChanged: name=%s, oldOwner=%s, newOwner=%s\n",
                   busName,
                   previousOwner ? previousOwner : "<none>",
                   newOwner ? newOwner : "<none>");
        }
    }
};


/** Static bus listener */
static MyBusListenerOne g_busListener_one;

/** Static bus listener */
static MyBusListenerTwo g_busListener_two;


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

    // Search for org.alljoyn.Bus.method_sample
    /* Create message bus */
    g_msgBus_one = new BusAttachment("myICEAppOne", true);

    /* Add org.alljoyn.Bus.method_sample interface */
    InterfaceDescription* testIntf = NULL;
    status = g_msgBus_one->CreateInterface(INTERFACE_NAME_ONE, testIntf);
    if (status == ER_OK) {
        printf("Interface Created.\n");
        testIntf->AddMethod("cat", "ss",  "s", "inStr1,inStr2,outStr", 0);
        testIntf->Activate();
    } else {
        printf("Failed to create interface 'org.alljoyn.Bus.method_sample'\n");
    }


    /* Start the msg bus */
    if (ER_OK == status) {
        status = g_msgBus_one->Start();
        if (ER_OK != status) {
            printf("BusAttachment::Start failed\n");
        } else {
            printf("BusAttachment started.\n");
        }
    }

    /* Connect to the bus */
    if (ER_OK == status) {
        status = g_msgBus_one->Connect(connectArgs);
        if (ER_OK != status) {
            printf("BusAttachment::Connect(\"%s\") failed\n", connectArgs);
        } else {
            printf("BusAttchement connected to %s\n", connectArgs);
        }
    }

    /* Register a bus listener in order to get discovery indications */
    if (ER_OK == status) {
        g_msgBus_one->RegisterBusListener(g_busListener_one);
        printf("BusListener Registered.\n");
    }

    /* Begin discovery on the well-known name of the service to be called */
    if (ER_OK == status) {
        status = g_msgBus_one->FindAdvertisedName(SERVICE_NAME_ONE);
        if (status != ER_OK) {
            printf("org.alljoyn.Bus.FindAdvertisedName failed (%s))\n", QCC_StatusText(status));
        }
    }

    if (ER_OK == status) {
        qcc::Sleep(5000);
    }

    // Search for org.alljoyn.Bus.ice_sample
    /* Create message bus */
    g_msgBus_two = new BusAttachment("myICEAppTwo", true);

    /* Add org.alljoyn.Bus.method_sample interface */
    InterfaceDescription* testIntftwo = NULL;
    status = g_msgBus_two->CreateInterface(INTERFACE_NAME_TWO, testIntftwo);
    if (status == ER_OK) {
        printf("Interface Created.\n");
        testIntf->AddMethod("cat", "ss",  "s", "inStr1,inStr2,outStr", 0);
        testIntf->Activate();
    } else {
        printf("Failed to create interface 'org.alljoyn.Bus.ice_sample'\n");
    }


    /* Start the msg bus */
    if (ER_OK == status) {
        status = g_msgBus_two->Start();
        if (ER_OK != status) {
            printf("BusAttachment::Start failed for org.alljoyn.Bus.ice_sample\n");
        } else {
            printf("BusAttachment started for org.alljoyn.Bus.ice_sample.\n");
        }
    }

    /* Connect to the bus */
    if (ER_OK == status) {
        status = g_msgBus_two->Connect(connectArgs);
        if (ER_OK != status) {
            printf("BusAttachment::Connect(\"%s\") failed for org.alljoyn.Bus.ice_sample\n", connectArgs);
        } else {
            printf("BusAttchement connected to %s for org.alljoyn.Bus.ice_sample\n", connectArgs);
        }
    }

    /* Register a bus listener in order to get discovery indications */
    if (ER_OK == status) {
        g_msgBus_two->RegisterBusListener(g_busListener_two);
        printf("BusListener Registered.\n");
    }

    /* Begin discovery on the well-known name of the service to be called */
    if (ER_OK == status) {
        status = g_msgBus_two->FindAdvertisedName(SERVICE_NAME_TWO);
        if (status != ER_OK) {
            printf("org.alljoyn.Bus.FindAdvertisedName failed (%s)) for org.alljoyn.Bus.ice_sample\n", QCC_StatusText(status));
        }
    }

    if (ER_OK == status) {
        qcc::Sleep(5000);
    }

    // Cancel the discovery for org.alljoyn.Bus.method_sample
    if (ER_OK == status) {
        status = g_msgBus_one->CancelFindAdvertisedName(SERVICE_NAME_ONE);
        if (status != ER_OK) {
            printf("org.alljoyn.Bus.CancelFindAdvertisedName failed (%s)) for org.alljoyn.Bus.method_sample\n", QCC_StatusText(status));
        }
    }

    if (ER_OK == status) {
        qcc::Sleep(5000);
    }

    // Cancel the discovery for org.alljoyn.Bus.ice_sample
    if (ER_OK == status) {
        status = g_msgBus_two->CancelFindAdvertisedName(SERVICE_NAME_TWO);
        if (status != ER_OK) {
            printf("org.alljoyn.Bus.CancelFindAdvertisedName failed (%s)) for org.alljoyn.Bus.ice_sample\n", QCC_StatusText(status));
        }
    }

    if (ER_OK == status) {
        qcc::Sleep(5000);
    }

    /* Deallocate bus */
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

    printf("ICE Search exiting with status %d (%s)\n", status, QCC_StatusText(status));

    return (int) status;
}
