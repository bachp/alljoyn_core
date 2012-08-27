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
#include <gtest/gtest.h>
#include "ajTestCommon.h"
#include <alljoyn/Message.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/BusListener.h>
#include <alljoyn/BusObject.h>
#include <alljoyn/ProxyBusObject.h>
#include <alljoyn/InterfaceDescription.h>
#include <alljoyn/DBusStd.h>
#include <qcc/Debug.h>
#include <qcc/Thread.h>

using namespace ajn;
using namespace qcc;

/*constants*/
static const char* OBJECT_PATH =   "/org/alljoyn/test/BusObjectTest";

class BusObjectTest : public testing::Test {
  public:
    BusObjectTest() : bus("BusObjectTest", false) { }

    class BusObjectTestBusObject : public BusObject {
      public:
        BusObjectTestBusObject(BusAttachment& bus, const char* path)
            : BusObject(bus, path), wasRegistered(false), wasUnregistered(false) { }
        virtual ~BusObjectTestBusObject() { }
        virtual void ObjectRegistered(void) {
            BusObject::ObjectRegistered();
            wasRegistered = true;
        }
        virtual void ObjectUnregistered(void) {
            BusObject::ObjectUnregistered();
            wasUnregistered = true;
        }

        bool wasRegistered, wasUnregistered;
    };

    QStatus status;
    BusAttachment bus;
};

/* ALLJOYN-1292 */
TEST_F(BusObjectTest, DISABLED_ObjectRegisteredUnregistered) {
    BusObjectTestBusObject testObj(bus, OBJECT_PATH);
    status = bus.RegisterBusObject(testObj);
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    status = bus.Start();
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);
    status = bus.Connect(ajn::getConnectArg().c_str());
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    status = bus.Disconnect(ajn::getConnectArg().c_str());
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);
    status = bus.Stop();
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);
    status = bus.Join();
    EXPECT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);

    for (int i = 0; i < 500; ++i) {
        qcc::Sleep(10);
        if (testObj.wasRegistered && testObj.wasUnregistered) {
            break;
        }
    }
    EXPECT_TRUE(testObj.wasRegistered);
    EXPECT_TRUE(testObj.wasUnregistered);
}
