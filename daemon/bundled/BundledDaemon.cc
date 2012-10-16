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
#include <qcc/Log.h>
#include <qcc/String.h>
#include <qcc/StringSource.h>
#include <qcc/StringUtil.h>
#include <qcc/Mutex.h>
#include <qcc/Thread.h>
#include <qcc/FileStream.h>

#include <alljoyn/BusAttachment.h>

#include <Status.h>

#include "Bus.h"
#include "BusController.h"
#include "DaemonConfig.h"
#include "Transport.h"
#include "TCPTransport.h"
#include "NullTransport.h"

#if defined(QCC_OS_ANDROID) || defined(QCC_OS_LINUX) || defined(QCC_OS_DARWIN) || defined(QCC_OS_WINRT)
#include "DaemonICETransport.h"
#endif

#define QCC_MODULE "ALLJOYN_DAEMON"

using namespace qcc;
using namespace std;
using namespace ajn;

static const char bundledConfig[] =
    "<busconfig>"
    "  <type>alljoyn_bundled</type>"
    "  <listen>tcp:r4addr=0.0.0.0,r4port=0</listen>"
    "  <limit auth_timeout=\"5000\"/>"
    "  <limit max_incomplete_connections=\"4\"/>"
    "  <limit max_completed_connections=\"16\"/>"
    "  <ip_name_service>"
    "    <property interfaces=\"*\"/>"
    "    <property disable_directed_broadcast=\"false\"/>"
    "    <property enable_ipv4=\"true\"/>"
    "    <property enable_ipv6=\"true\"/>"
    "  </ip_name_service>"
#if defined(QCC_OS_ANDROID) || defined(QCC_OS_LINUX) || defined(QCC_OS_DARWIN) || defined(QCC_OS_WINRT)
    "  <listen>ice:</listen>"
    "  <ice>"
    "    <limit max_incomplete_connections=\"16\"/>"
    "    <limit max_completed_connections=\"64\"/>"
    "  </ice>"
    "  <ice_discovery_manager>"
    "    <property interfaces=\"*\"/>"
    "    <property server=\"rdvs.alljoyn.org\"/>"
    "    <property protocol=\"HTTPS\"/>"
    "    <property enable_ipv6=\"false\"/>"
    "  </ice_discovery_manager>"
#endif
    "</busconfig>";

class BundledDaemon : public DaemonLauncher, public TransportFactoryContainer {

  public:

    BundledDaemon();

    ~BundledDaemon();

    /**
     * Launch the bundled daemon
     */
    QStatus Start(NullTransport* nullTransport);

    /**
     * Terminate the bundled daemon
     */
    QStatus Stop(NullTransport* nullTransport);

    /**
     * Wait for bundled daemon to exit
     */
    void Join();

  private:

    bool transportsInitialized;
    bool stopping;
    Bus* ajBus;
    BusController* ajBusController;
    Mutex lock;
    std::set<NullTransport*> transports;
};

bool ExistFile(const char* fileName) {
    FILE* file = NULL;
    if (fileName && (file = fopen(fileName, "r"))) {
        fclose(file);
        return true;
    }
    return false;
}

/*
 * Create the singleton bundled daemon instance.
 *
 * Sidebar on starting a bundled daemon
 * ====================================
 *
 * How this works is via a fairly non-obvious mechanism, so we describe the
 * process here.  If it is desired to use the bundled daemon, the user (for
 * example bbclient or bbservice) includes this compilation unit.  Since the
 * following defines a C++ static initializer, an instance of the BundledDaemon
 * object will be created before any call into a function in this file.  In
 * Linux, for example, this happens as a result of _init() being called before
 * the main() function of the program using the bundled daemon.  _init() loops
 * through the list of compilation units in link order and will eventually call
 * out to BundledDaemon.cc:__static_initialization_and_destruction_0().  This is
 * the initializer function for this file which will then calls the constructor
 * for the BundledDaemon object.  The constructor calls into a static method
 * (RegisterDaemonLauncher) of the NullTransport to register itself as the
 * daemon to be launched.  This sets the stage for the use of the bundled
 * daemon.
 *
 * When the program using the bundled daemon tries to connect to a bus
 * attachment it calls BusAttachment::Connect().  This tries to connect to an
 * existing daemon first and if that connect does not succeed, it tries to
 * connect over the NullTransport to the bundled daemon.
 *
 * The NullTransport::Connect() method looks to see if it (the null transport)
 * is running, and if it is not it looks to see if it has a daemonLauncher.
 * Recall that the constructor for the BundledDaemon object registered itself as
 * a daemon launcher, so the null transport will find the launcher since it
 * included the object file corresponding to this source.  The null transport
 * then does a daemonLauncher->Start() which calls back into the bundled daemon
 * object BundledDaemon::Start() method below, providing the daemon with the
 * NullTransport pointer.  The Start() method brings up the bundled daemon and
 * links the daemon to the bus attachment using the provided null transport.
 *
 * So to summarize, one uses the bundled daemon simply by linking to the object
 * file corresponding to this source file.  This automagically creates a bundled
 * daemon static object and registers it with the null transport.  When trying
 * to connect to a daemon using a bus attachment in the usual way, if there is
 * no currently running native daemon process, the bus attachment will
 * automagically try to connect to a registered bundled daemon using the null
 * transport.  This will start the bundled daemon and then connect to it.
 *
 * The client uses the bundled daemon transparently -- it only has to link to it.
 *
 * Stopping the bundled daemon happens in the destructor for the C++ static
 * global object, again transparently to the client.
 *
 * It's pretty magical.
 */
static BundledDaemon bundledDaemon;

BundledDaemon::BundledDaemon() : transportsInitialized(false), stopping(false), ajBus(NULL), ajBusController(NULL)
{
    NullTransport::RegisterDaemonLauncher(this);
}

BundledDaemon::~BundledDaemon()
{
    QCC_DbgPrintf(("BundledDaemon::~BundledDaemon"));
    lock.Lock(MUTEX_CONTEXT);
    while (!transports.empty()) {
        set<NullTransport*>::iterator iter = transports.begin();
        NullTransport* trans = *iter;
        transports.erase(iter);
        lock.Unlock(MUTEX_CONTEXT);
        trans->Disconnect("null:");
        lock.Lock(MUTEX_CONTEXT);
    }
    lock.Unlock(MUTEX_CONTEXT);
    Join();
}

QStatus BundledDaemon::Start(NullTransport* nullTransport)
{
    QStatus status = ER_OK;

    printf("Using BundledDaemon\n");

    /*
     * If the bundled daemon is in the process of stopping we need to wait until the operation is
     * complete (BundledDaemon::Join has exited) before we attempt to start up again.
     */
    lock.Lock(MUTEX_CONTEXT);
    while (stopping) {
        if (!transports.empty()) {
            assert(transports.empty());
        }
        lock.Unlock(MUTEX_CONTEXT);
        qcc::Sleep(5);
        lock.Lock(MUTEX_CONTEXT);
    }
    if (transports.empty()) {
#if defined(QCC_OS_ANDROID)
        LoggerSetting::GetLoggerSetting("bundled-daemon", LOG_DEBUG, true, NULL);
#else
        LoggerSetting::GetLoggerSetting("bundled-daemon", LOG_DEBUG, false, stdout);
#endif

        /*
         * Load the configuration
         */
        DaemonConfig* config = NULL;
#ifndef NDEBUG
        qcc::String configFile = qcc::String::Empty;
    #if defined(QCC_OS_ANDROID)
        configFile = "/mnt/sdcard/.alljoyn/config.xml";
    #endif

    #if defined(QCC_OS_LINUX) || defined(QCC_OS_GROUP_WINDOWS) || defined(QCC_OS_GROUP_WINRT)
        configFile = "./config.xml";
    #endif

        if (!configFile.empty() && ExistFile(configFile.c_str())) {
            FileSource fs(configFile);
            if (fs.IsValid()) {
                config = DaemonConfig::Load(fs);
                if (!config) {
                    status = ER_BUS_BAD_XML;
                    QCC_LogError(status, ("Error parsing configuration from %s", configFile.c_str()));
                    goto ErrorExit;
                }
            }
        }
#endif
        if (!config) {
            config = DaemonConfig::Load(bundledConfig);
        }
        if (!config) {
            status = ER_BUS_BAD_XML;
            QCC_LogError(status, ("Error parsing configuration"));
            goto ErrorExit;
        }
        /*
         * Extract the listen specs
         */
        vector<String> listenList = config->GetList("listen");
        String listenSpecs = StringVectorToString(&listenList, ";");
        /*
         * Register the transport factories - this is a one time operation
         */
        if (!transportsInitialized) {
            Add(new TransportFactory<TCPTransport>(TCPTransport::TransportName, false));
#if defined(QCC_OS_ANDROID) || defined(QCC_OS_LINUX) || defined(QCC_OS_DARWIN) || defined(QCC_OS_WINRT)
            Add(new TransportFactory<DaemonICETransport>(DaemonICETransport::TransportName, false));
#endif
            transportsInitialized = true;
        }
        QCC_DbgPrintf(("Starting bundled daemon bus attachment"));
        /*
         * Create and start the daemon
         */
        ajBus = new Bus("bundled-daemon", *this, listenSpecs.c_str());
        ajBusController = new BusController(*ajBus);
        status = ajBusController->Init(listenSpecs);
        if (ER_OK != status) {
            goto ErrorExit;
        }
    }
    /*
     * Use the null transport to link the daemon and client bus together
     */
    status = nullTransport->LinkBus(ajBus);
    if (status != ER_OK) {
        goto ErrorExit;
    }

    transports.insert(nullTransport);

    lock.Unlock(MUTEX_CONTEXT);
    return ER_OK;

ErrorExit:

    if (transports.empty()) {
        delete ajBusController;
        ajBusController = NULL;
        delete ajBus;
        ajBus = NULL;
    }
    lock.Unlock(MUTEX_CONTEXT);
    return status;
}

void BundledDaemon::Join()
{
    QCC_DbgPrintf(("BundledDaemon::Join"));
    lock.Lock(MUTEX_CONTEXT);
    if (transports.empty() && ajBus) {
        QCC_DbgPrintf(("Joining bundled daemon bus attachment"));
        delete ajBusController;
        ajBusController = NULL;
        delete ajBus;
        ajBus = NULL;
        /*
         * Clear the stopping state
         */
        stopping = false;
    }
    lock.Unlock(MUTEX_CONTEXT);
}

QStatus BundledDaemon::Stop(NullTransport* nullTransport)
{
    QCC_DbgPrintf(("BundledDaemon::Stop"));
    lock.Lock(MUTEX_CONTEXT);
    transports.erase(nullTransport);
    QStatus status = ER_OK;
    if (transports.empty()) {
        /*
         * Set the stopping state to block any calls to Start until
         * after Join() has been called.
         */
        stopping = true;
        if (ajBus) {
            status = ajBus->Stop();
        }
    }
    lock.Unlock(MUTEX_CONTEXT);
    return status;
}

