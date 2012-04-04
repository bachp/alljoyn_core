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

#include <vector>

#include <qcc/String.h>
#include <qcc/StringSource.h>
#include <qcc/StringUtil.h>

#include <alljoyn/BusAttachment.h>

#include <Status.h>

#include "Bus.h"
#include "BusController.h"
#include "ConfigDB.h"
#include "Transport.h"
#include "TCPTransport.h"
#include "NullTransport.h"

#define QCC_MODULE "ALLJOYN_DAEMON"

using namespace qcc;
using namespace std;
using namespace ajn;

static const char bundledConfig[] =
    "<busconfig>"
    "  <type>alljoyn</type>"
    "  <listen>tcp:addr=0.0.0.0,port=9955,family=ipv4</listen>"
    "  <policy context=\"default\">"
    "    <allow send_interface=\"*\"/>"
    "    <allow receive_interface=\"*\"/>"
    "    <allow own=\"*\"/>"
    "    <allow user=\"*\"/>"
    "    <allow send_requested_reply=\"true\"/>"
    "    <allow receive_requested_reply=\"true\"/>"
    "  </policy>"
    "  <limit name=\"auth_timeout\">32768</limit>"
    "  <limit name=\"max_incomplete_connections_tcp\">16</limit>"
    "  <limit name=\"max_completed_connections_tcp\">64</limit>"
    "  <alljoyn module=\"ipns\">"
    "    <property interfaces=\"*\"/>"
    "  </alljoyn>"
    "</busconfig>";

class BundledDaemon : public DaemonLauncher {

  public:

    BundledDaemon();

    /**
     * Launch the bundled daemon
     */
    QStatus StartDaemon(BusAttachment*& busAttachment);

    /**
     * Terminate the bundled daemon
     */
    QStatus StopDaemon();

  private:

    Bus* ajBus;
    BusController* ajBusController;

};

/*
 * Create the singleton bundled daemon instance.
 */
static BundledDaemon bundledDaemon;

BundledDaemon::BundledDaemon() : ajBus(NULL), ajBusController(NULL)
{
    printf("Registering bundled daemon\n");
    NullTransport::RegisterDaemonLauncher(this);
}

QStatus BundledDaemon::StartDaemon(BusAttachment*& busAttachment)
{
    QStatus status;
    ConfigDB* config(ConfigDB::GetConfigDB());
    /*
     * Set the configuration
     */
    StringSource src(bundledConfig);
    config->LoadSource(src);
    /*
     * Extract the listen specs
     */
    const ConfigDB::ListenList& listenList = config->GetListen();
    ConfigDB::ListenList::const_iterator it = listenList.begin();
    String listenSpecs;
    while (it != listenList.end()) {
        qcc::String addrStr(*it);
        if (!listenSpecs.empty()) {
            listenSpecs.append(';');
        }
        listenSpecs.append(addrStr);
        ++it;
    }
    /*
     * Add the transports
     */
    TransportFactoryContainer cntr;
    cntr.Add(new TransportFactory<TCPTransport>(TCPTransport::TransportName, false));

    ajBus = new Bus("bundled-daemon", cntr, listenSpecs.c_str());
    ajBusController = new BusController(*ajBus, status);
    if (ER_OK != status) {
        goto ErrorExit;
    }
    status = ajBus->Start();
    if (status != ER_OK) {
        goto ErrorExit;
    }
    status = ajBus->StartListen(listenSpecs.c_str());
    if (ER_OK != status) {
        goto ErrorExit;
    }

    busAttachment = ajBus;
    return ER_OK;

ErrorExit:

    delete ajBusController;
    ajBusController = NULL;
    delete ajBus;
    ajBusController = NULL;
    return status;
}


QStatus BundledDaemon::StopDaemon()
{
    if (ajBus) {
        ajBus->Stop();
        ajBus->Join();
    }
    delete ajBusController;
    ajBusController = NULL;
    delete ajBus;
    ajBusController = NULL;
    return ER_OK;
}
