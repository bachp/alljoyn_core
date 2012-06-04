/**
 * @file
 * Implementation of class for launching a bundled daemon
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

#include <stdio.h>

#include <qcc/platform.h>
#include <qcc/Debug.h>
#include <qcc/Logger.h>

#include <qcc/String.h>
#include <qcc/StringSource.h>
#include <qcc/StringUtil.h>

#include <alljoyn/BusAttachment.h>

#include <Status.h>

#include "Bus.h"
#include "BusController.h"
#include "DaemonConfig.h"
#include "Transport.h"
#include "TCPTransport.h"
#include "NullTransport.h"

#define QCC_MODULE "ALLJOYN_DAEMON"

using namespace qcc;
using namespace std;
using namespace ajn;

static const char bundledConfig[] =
    "<busconfig>"
    "  <type>alljoyn_bundled</type>"
    "  <listen>tcp:addr=0.0.0.0,port=0,family=ipv4</listen>"
    "  <limit name=\"auth_timeout\">5000</limit>"
    "  <limit name=\"max_incomplete_connections_tcp\">4</limit>"
    "  <limit name=\"max_completed_connections_tcp\">16</limit>"
    "  <ip_name_service>"
    "    <property interfaces=\"*\"/>"
    "    <property disable_directed_broadcast=\"false\"/>"
    "    <property enable_ipv4=\"true\"/>"
    "    <property enable_ipv6=\"true\"/>"
    "  </ip_name_service>"
    "</busconfig>";

class BundledDaemon : public DaemonLauncher {

  public:

    BundledDaemon();

    /**
     * Launch the bundled daemon
     */
    QStatus Start(NullTransport* nullTransport);

    /**
     * Terminate the bundled daemon
     */
    QStatus Stop();

    /**
     * Wait for bundled daemon to exit
     */
    void Join();

  private:

    int32_t refCount;
    Bus* ajBus;
    BusController* ajBusController;

};

/*
 * Create the singleton bundled daemon instance.
 */
static BundledDaemon bundledDaemon;

BundledDaemon::BundledDaemon() : refCount(0), ajBus(NULL), ajBusController(NULL)
{
    printf("Registering bundled daemon\n");
    NullTransport::RegisterDaemonLauncher(this);
}

QStatus BundledDaemon::Start(NullTransport* nullTransport)
{
    QStatus status = ER_OK;

    if (IncrementAndFetch(&refCount) == 1) {
        LoggerSetting::GetLoggerSetting("bundled-daemon", LOG_DEBUG, false, stdout);

        /*
         * Load the configuration
         */
        DaemonConfig* config = DaemonConfig::Load(bundledConfig);
        /*
         * Extract the listen specs
         */
        vector<String> listenList = config->GetList("listen");
        String listenSpecs = StringVectorToString(&listenList, ";");
        /*
         * Add the transports
         */
        TransportFactoryContainer cntr;
        cntr.Add(new TransportFactory<TCPTransport>(TCPTransport::TransportName, false));

        ajBus = new Bus("bundled-daemon", cntr, listenSpecs.c_str());
        ajBusController = new BusController(*ajBus);
        status = ajBusController->Init(listenSpecs);
        if (ER_OK != status) {
            goto ErrorExit;
        }
        /*
         * TODO - until we figure out why the daemon doesn't cleanly restart bump the refCount once
         * more so the bundled daemon doesn't ever get released.
         */
        IncrementAndFetch(&refCount);
    }
    /*
     * Use the null transport to link the daemon and client bus together
     */
    status = nullTransport->LinkBus(ajBus);
    if (status != ER_OK) {
        goto ErrorExit;
    }

    return ER_OK;

ErrorExit:

    if (DecrementAndFetch(&refCount) == 0) {
        delete ajBusController;
        ajBusController = NULL;
        delete ajBus;
        ajBus = NULL;
    }
    return status;
}

void BundledDaemon::Join()
{
    if (refCount == 0) {
        if (ajBus) {
            ajBus->Join();
        }
        delete ajBusController;
        ajBusController = NULL;
        delete ajBus;
        ajBus = NULL;
    }
}

QStatus BundledDaemon::Stop()
{
    int32_t rc = DecrementAndFetch(&refCount);
    assert(rc >= 0);
    if (rc == 0) {
        if (ajBus) {
            return ajBus->Stop();
        } else {
            return ER_OK;
        }
    }
    return ER_OK;
}
