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

/* Header files included for Google Test Framework */
#include <gtest/gtest.h>
#include "ajTestCommon.h"
//#include <qcc/time.h>

using namespace std;
using namespace qcc;
using namespace ajn;
class BusAttachmentTest : public testing::Test {
  public:
    BusAttachment bus;

    BusAttachmentTest() : bus("BusAttachmentTest", false) { };

    virtual void SetUp() {
        QStatus status = ER_OK;
        status = bus.Start();
        ASSERT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);
        status = bus.Connect(getConnectArg().c_str());
        ASSERT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);
    }

    virtual void TearDown() {
        bus.Stop();
        bus.Join();
    }

};

TEST_F(BusAttachmentTest, FindName_Join_Self)
{
    SessionPortListener sp_listener;
    SessionOpts opts;
    SessionPort port = 52;

    QStatus status = ER_OK;

    const char* requestedName = "org.alljoyn.bus.BusAttachmentTest.JoinSelf";

    status = bus.BindSessionPort(port, opts, sp_listener);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    status = bus.RequestName(requestedName, DBUS_NAME_FLAG_DO_NOT_QUEUE);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    status = bus.AdvertiseName(requestedName, TRANSPORT_ANY);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    status = bus.FindAdvertisedName(requestedName);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);


    SessionId id;
    status = bus.JoinSession(requestedName, port, NULL, id, opts);
    EXPECT_EQ(ER_ALLJOYN_JOINSESSION_REPLY_ALREADY_JOINED, status) << "  Actual Status: " << QCC_StatusText(status);
}

TEST_F(BusAttachmentTest, FindName_Same_Name)
{
    QStatus status = ER_OK;

    const char* requestedName = "org.alljoyn.bus.BusAttachmentTest.advertise";

    /* flag indicates that Fail if name cannot be immediatly obtained */
    status = bus.FindAdvertisedName(requestedName);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    status = bus.FindAdvertisedName(requestedName);
    EXPECT_EQ(ER_ALLJOYN_FINDADVERTISEDNAME_REPLY_ALREADY_DISCOVERING, status) << "  Actual Status: " << QCC_StatusText(status);


    status = bus.CancelFindAdvertisedName(requestedName);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);
}

TEST_F(BusAttachmentTest, FindName_Null_Name)
{
    QStatus status = ER_OK;

    const char* requestedName = NULL;

    /* flag indicates that Fail if name cannot be immediatly obtained */
    status = bus.FindAdvertisedName(requestedName);
    EXPECT_EQ(ER_BAD_ARG_1, status) << "  Actual Status: " << QCC_StatusText(status);
}

bool foundNameA = false;
bool foundNameB = false;

class FindMulipleNamesBusListener : public BusListener {
    void FoundAdvertisedName(const char* name, TransportMask transport, const char* namePrefix) {
        printf("FoundAdvertisedName name=%s  prefix=%s\n", name, namePrefix);
        if (strcmp(name, "name.A") == 0) {
            foundNameA = true;
        }
        if (strcmp(name, "name.B") == 0) {
            foundNameB = true;
        }
    }
};

TEST_F(BusAttachmentTest, find_multiple_names)
{
    QStatus status = ER_FAIL;
    FindMulipleNamesBusListener testBusListener;
    bus.RegisterBusListener(testBusListener);

    foundNameA = false;
    foundNameB = false;

    status = bus.FindAdvertisedName("name.A");
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);
    status = bus.FindAdvertisedName("name.B");
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    BusAttachment otherBus("BusAttachmentTestOther", true);
    status = otherBus.Start();
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);
    status = otherBus.Connect(getConnectArg().c_str());
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    status = otherBus.AdvertiseName("name.A", TRANSPORT_ANY);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);
    status = otherBus.AdvertiseName("name.B", TRANSPORT_ANY);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    //Wait upto 8 seconds for the both found name signals to complete.
    for (int i = 0; i < 800; ++i) {
        qcc::Sleep(10);
        if (foundNameA && foundNameB) {
            break;
        }
    }


    EXPECT_TRUE(foundNameA);
    EXPECT_TRUE(foundNameB);

    status = otherBus.CancelAdvertiseName("name.A", TRANSPORT_ANY);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);
    status = otherBus.CancelAdvertiseName("name.B", TRANSPORT_ANY);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    status = bus.CancelFindAdvertisedName("name.B");
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    foundNameA = false;
    foundNameB = false;

    status = otherBus.AdvertiseName("name.A", TRANSPORT_ANY);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);
    status = otherBus.AdvertiseName("name.B", TRANSPORT_ANY);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    //Wait upto 2 seconds for the found name signal to complete.
    for (int i = 0; i < 200; ++i) {
        qcc::Sleep(10);
        if (foundNameA) {
            break;
        }
    }

    EXPECT_TRUE(foundNameA);
    EXPECT_FALSE(foundNameB);

    status = otherBus.CancelAdvertiseName("name.A", TRANSPORT_ANY);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);
    status = otherBus.CancelAdvertiseName("name.B", TRANSPORT_ANY);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    status = bus.CancelFindAdvertisedName("name.A");
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    //Must Unregister bus listener or the test will segfault
    bus.UnregisterBusListener(testBusListener);

    otherBus.Stop();
    otherBus.Join();
}

/*
 * listeners and variables used by the JoinSession Test.
 * This test is a mirror of the JUnit test that goes by the same name
 *
 */
bool found;
bool lost;

class FindNewNameBusListener : public BusListener, public BusAttachmentTest {
    virtual void FoundAdvertisedName(const char* name, TransportMask transport, const char* namePrefix) {
        found = true;
        bus.EnableConcurrentCallbacks();

    }
};

bool sessionAccepted;
bool sessionJoined;
bool onJoined;
QStatus joinSessionStatus;
int busSessionId;
int otherBusSessionId;

class JoinSession_SessionPortListener : public SessionPortListener {
  public:
    JoinSession_SessionPortListener(BusAttachment* bus) : bus(bus) { };

    bool AcceptSessionJoiner(SessionPort sessionPort, const char* joiner, const SessionOpts& opts) {
        if (sessionPort == 42) {
            sessionAccepted = true;
            bus->EnableConcurrentCallbacks();
            return true;
        } else {
            sessionAccepted = false;
            return false;
        }
    }

    void SessionJoined(SessionPort sessionPort, SessionId id, const char* joiner) {
        if (sessionPort == 42) {
            busSessionId = id;
            sessionJoined = true;
        } else {
            sessionJoined = false;
        }
    }
    BusAttachment* bus;
};

class JoinSession_BusListener : public BusListener {
  public:
    JoinSession_BusListener(BusAttachment* bus) : bus(bus) { }

    void FoundAdvertisedName(const char* name, TransportMask transport, const char* namePrefix) {
        SessionOpts sessionOpts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);

        SessionId sessionId = 0;
        // Since we are using blocking form of joinSession, we need to enable concurrency
        bus->EnableConcurrentCallbacks();
        // Join session once the AdvertisedName has been found
        joinSessionStatus = bus->JoinSession(name, 42, new SessionListener(), sessionId, sessionOpts);
        otherBusSessionId = sessionId;
        found = true;
    }
    BusAttachment* bus;
};

TEST_F(BusAttachmentTest, JoinSession) {
    QStatus status = ER_FAIL;
    // Set up SessionOpts
    SessionOpts sessionOpts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);

    // User defined sessionPort Number
    SessionPort sessionPort = 42;

    //bindSessionPort new SessionPortListener
    JoinSession_SessionPortListener sessionPortListener(&bus);
    status = bus.BindSessionPort(sessionPort, sessionOpts, sessionPortListener);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    // Request name from bus
    int flag = DBUS_NAME_FLAG_REPLACE_EXISTING | DBUS_NAME_FLAG_DO_NOT_QUEUE;
    status = bus.RequestName("org.alljoyn.bus.BusAttachmentTest.advertise", flag);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    // Advertise same bus name
    status = bus.AdvertiseName("org.alljoyn.bus.BusAttachmentTest.advertise", TRANSPORT_ANY);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    // Create Second BusAttachment
    BusAttachment otherBus("BusAttachemntTest.JoinSession", true);
    status = otherBus.Start();
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);
    status = otherBus.Connect(getConnectArg().c_str());
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    // Register BusListener for the foundAdvertisedName Listener
    JoinSession_BusListener busListener(&otherBus);
    otherBus.RegisterBusListener(busListener);

    // find the AdvertisedName
    status = otherBus.FindAdvertisedName("org.alljoyn.bus.BusAttachmentTest.advertise");
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    for (size_t i = 0; i < 200; ++i) {
        if (found) {
            break;
        }
        qcc::Sleep(5);
    }

    EXPECT_TRUE(found);

    for (size_t i = 0; i < 200; ++i) {
        if (!sessionAccepted || !sessionJoined) {
            break;
        }
        qcc::Sleep(5);
    }

    EXPECT_EQ(ER_OK, joinSessionStatus);
    EXPECT_TRUE(sessionAccepted);
    EXPECT_TRUE(sessionJoined);
    EXPECT_EQ(busSessionId, otherBusSessionId);
}
