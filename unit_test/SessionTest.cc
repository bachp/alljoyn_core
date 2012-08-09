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

using namespace std;
using namespace qcc;
using namespace ajn;

class SessionTest : public testing::Test {
  public:
    virtual void SetUp() { }
    virtual void TearDown() { }

};

class TwoMultipointSessionsSessionPortListener : public SessionPortListener {
  public:
    virtual ~TwoMultipointSessionsSessionPortListener() { }
    virtual bool AcceptSessionJoiner(SessionPort sessionPort, const char* joiner, const SessionOpts& opts) { return true; }
};

TEST_F(SessionTest, TwoMultipointSessions)
{
    SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, true, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
    SessionPort portA = 27;
    SessionPort portB = portA;

    BusAttachment busA("A");
    TwoMultipointSessionsSessionPortListener listenerA;
    ASSERT_EQ(ER_OK, busA.Start());
    ASSERT_EQ(ER_OK, busA.Connect(getConnectArg().c_str()));
    ASSERT_EQ(ER_OK, busA.BindSessionPort(portA, opts, listenerA));
    ASSERT_EQ(ER_OK, busA.RequestName("bus.A", DBUS_NAME_FLAG_DO_NOT_QUEUE));
    ASSERT_EQ(ER_OK, busA.AdvertiseName("bus.A", TRANSPORT_ANY));

    BusAttachment busB("B");
    TwoMultipointSessionsSessionPortListener listenerB;
    ASSERT_EQ(ER_OK, busB.Start());
    ASSERT_EQ(ER_OK, busB.Connect(getConnectArg().c_str()));
    ASSERT_EQ(ER_OK, busB.BindSessionPort(portB, opts, listenerB));
    ASSERT_EQ(ER_OK, busB.RequestName("bus.B", DBUS_NAME_FLAG_DO_NOT_QUEUE));
    ASSERT_EQ(ER_OK, busB.AdvertiseName("bus.B", TRANSPORT_ANY));

    SessionId outIdA;
    SessionOpts outOptsA;
    ASSERT_EQ(ER_OK, busA.JoinSession("bus.B", portB, NULL, outIdA, opts));

    SessionId outIdB;
    SessionOpts outOptsB;
    ASSERT_EQ(ER_OK, busB.JoinSession("bus.A", portA, NULL, outIdB, opts));

    /*
     * The bug is that joining two multipoint sessions with the same port resulted in only one
     * session, not two.  This asserts that there are in fact two different sessions created above.
     */
    ASSERT_NE(outIdA, outIdB);
}
