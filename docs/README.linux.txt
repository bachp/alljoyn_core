 
##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##
##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##
##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##


AllJoyn SDK for Linux
---------------------

This subtree contains one complete copy of the AllJoyn SDK for Linux, built for
a single CPU (either x86 or x86_64) and VARIANT (either debug or release).
The CPU and VARIANT are normally incorporated into the name of the package or
folder containing this SDK.

Please see ReleaseNotes.txt for the applicable AllJoyn release version and
related information on new features and known issues.


Summary of file and directory structure:
----------------------------------------

The contents of this SDK are arranged into the following top level folders:

cpp/    core AllJoyn functionality, implemented in C++
          - built from Git projects alljoyn_core and common
          - required for all AllJoyn applications
java/   optional Java language binding          (built from alljoyn_java)
js/     optional Javascript binding             (built from alljoyn_js)
c/      optional ANSI C language binding        (built from alljoyn_c)


The contents of each top level folder are further arranged into sub-folders:

        ----------------------------------------------
cpp/    core AllJoyn functionality, implemented in C++
        ----------------------------------------------

    bin/                        executable binaries

        alljoyn-daemon                  installable AllJoyn daemon
        ???

    bin/samples/                pre-built sample programs

    docs/html/                  AllJoyn Core API documentation

    inc/alljoyn                 AllJoyn Core headers
    inc/qcc

    lib/                        AllJoyn Core client libraries

        liballjoyn.a                    implements core API
        libajdaemon.a                   ???
        BundledDaemon.o                 ???

    samples/                    C++ sample programs (see README)


        ---------------------
java/   Java language binding
        ---------------------

    docs/html/                  API documentation

    jar/                        client library, misc jar files

        alljoyn.jar                     implements Java API
        alljoyn-sources.jar             (for VARIANT=debug only)
        alljoyn_test.jar                ???
        ???

    lib/liballjoyn_java.so      client library

    samples/                    sample programs (see README)


        ---------------------------
js/     Javascript language binding
        ---------------------------

    docs/html/                  API documentation

    lib/libnpalljoyn.so         installable Netscape plugin ???

    samples/                    sample programs


        -----------------------
c/      ANSI C language binding
        -----------------------

    docs/html/                  API documentation

    inc/alljoyn_c               ANSI C headers
    inc/qcc

    lib/                        client libraries

## DELETE ME ## partial list of files in Linux SDK
cpp/bin/BTAccessorTester
cpp/bin/advtunnel
cpp/bin/aes_ccm
cpp/bin/alljoyn-daemon
cpp/bin/autochat
cpp/bin/bastress
cpp/bin/bastress2
cpp/bin/bbclient
cpp/bin/bbdaemon
cpp/bin/bbjitter
cpp/bin/bbjoin
cpp/bin/bbservice
cpp/bin/bbsig
cpp/bin/bignum
cpp/bin/bluetoothd-crasher
cpp/bin/bttimingclient
cpp/bin/compression
cpp/bin/keystore
cpp/bin/marshal
cpp/bin/mc-rcv
cpp/bin/mc-snd
cpp/bin/names
cpp/bin/ns
cpp/bin/packettest
cpp/bin/rawclient
cpp/bin/rawservice
cpp/bin/remarshal
cpp/bin/rsa
cpp/bin/samples/FileTransferClient
cpp/bin/samples/FileTransferService
cpp/bin/samples/basic_client
cpp/bin/samples/basic_service
cpp/bin/samples/chat
cpp/bin/samples/nameChange_client
cpp/bin/samples/signalConsumer_client
cpp/bin/samples/signal_service
cpp/bin/sessions
cpp/bin/socktest
cpp/bin/srp
cpp/bin/unpack
cpp/docs/html/
cpp/inc/alljoyn/AllJoynStd.h
cpp/inc/alljoyn/AuthListener.h
cpp/inc/alljoyn/BusAttachment.h
cpp/inc/alljoyn/BusListener.h
cpp/inc/alljoyn/BusObject.h
cpp/inc/alljoyn/DBusStd.h
cpp/inc/alljoyn/DBusStdDefines.h
cpp/inc/alljoyn/InterfaceDescription.h
cpp/inc/alljoyn/KeyStoreListener.h
cpp/inc/alljoyn/Message.h
cpp/inc/alljoyn/MessageReceiver.h
cpp/inc/alljoyn/MessageSink.h
cpp/inc/alljoyn/MsgArg.h
cpp/inc/alljoyn/ProxyBusObject.h
cpp/inc/alljoyn/Session.h
cpp/inc/alljoyn/SessionListener.h
cpp/inc/alljoyn/SessionPortListener.h
cpp/inc/alljoyn/SimpleBusListener.h
cpp/inc/alljoyn/Status.h
cpp/inc/alljoyn/TransportMask.h
cpp/inc/alljoyn/version.h
cpp/inc/qcc/Log.h
cpp/inc/qcc/ManagedObj.h
cpp/inc/qcc/String.h
cpp/inc/qcc/atomic.h
cpp/inc/qcc/platform.h
cpp/inc/qcc/posix/atomic.h
cpp/inc/qcc/posix/platform_types.h
cpp/lib/BundledDaemon.o
cpp/lib/libajdaemon.a
cpp/lib/liballjoyn.a
cpp/samples/FileTransfer/FileTransferClient.cc
cpp/samples/FileTransfer/FileTransferService.cc
cpp/samples/README.txt
cpp/samples/basic/Makefile
cpp/samples/basic/README.txt
cpp/samples/basic/basic_client.cc
cpp/samples/basic/basic_service.cc
cpp/samples/basic/nameChange_client.cc
cpp/samples/basic/signalConsumer_client.cc
cpp/samples/basic/signal_service.cc
cpp/samples/chat/Makefile
cpp/samples/chat/chat.cc
java/docs/html/
java/jar/JavaSDKDocMethodsClient.jar
java/jar/JavaSDKDocMethodsService.jar
java/jar/JavaSDKDocPropertiesClient.jar
java/jar/JavaSDKDocPropertiesService.jar
java/jar/JavaSDKDocSecurityLogonClient.jar
java/jar/JavaSDKDocSecurityLogonService.jar
java/jar/JavaSDKDocSecurityRsaClient.jar
java/jar/JavaSDKDocSecurityRsaClient2.jar
java/jar/JavaSDKDocSecurityRsaService.jar
java/jar/JavaSDKDocSecuritySPRService.jar
java/jar/JavaSDKDocSecuritySRPClient.jar
java/jar/JavaSDKDocSignalClient.jar
java/jar/JavaSDKDocSignalService.jar
java/jar/alljoyn-sources.jar
java/jar/alljoyn.jar
java/jar/alljoyn_test.jar
java/lib/liballjoyn_java.so
java/samples/JavaSDKDoc/JavaSDKDocMethodsClient/src/org/alljoyn/bus/samples/Client.java
java/samples/JavaSDKDoc/JavaSDKDocMethodsClient/src/org/alljoyn/bus/samples/SampleInterface.java
java/samples/JavaSDKDoc/JavaSDKDocMethodsService/src/org/alljoyn/bus/samples/SampleInterface.java
java/samples/JavaSDKDoc/JavaSDKDocMethodsService/src/org/alljoyn/bus/samples/Service.java
java/samples/JavaSDKDoc/JavaSDKDocPropertiesClient/src/org/alljoyn/bus/samples/Client.java
java/samples/JavaSDKDoc/JavaSDKDocPropertiesClient/src/org/alljoyn/bus/samples/PropertiesInterface.java
java/samples/JavaSDKDoc/JavaSDKDocPropertiesService/src/org/alljoyn/bus/samples/PropertiesInterface.java
java/samples/JavaSDKDoc/JavaSDKDocPropertiesService/src/org/alljoyn/bus/samples/Service.java
java/samples/JavaSDKDoc/JavaSDKDocSecurityLogonClient/src/org/alljoyn/bus/samples/Client.java
java/samples/JavaSDKDoc/JavaSDKDocSecurityLogonClient/src/org/alljoyn/bus/samples/SecureInterface.java
java/samples/JavaSDKDoc/JavaSDKDocSecurityLogonService/src/org/alljoyn/bus/samples/SecureInterface.java
java/samples/JavaSDKDoc/JavaSDKDocSecurityLogonService/src/org/alljoyn/bus/samples/Service.java
java/samples/JavaSDKDoc/JavaSDKDocSecurityRsaClient/src/org/alljoyn/bus/samples/Client.java
java/samples/JavaSDKDoc/JavaSDKDocSecurityRsaClient/src/org/alljoyn/bus/samples/SecureInterface.java
java/samples/JavaSDKDoc/JavaSDKDocSecurityRsaClient2/src/org/alljoyn/bus/samples/Client.java
java/samples/JavaSDKDoc/JavaSDKDocSecurityRsaClient2/src/org/alljoyn/bus/samples/SecureInterface.java
java/samples/JavaSDKDoc/JavaSDKDocSecurityRsaService/src/org/alljoyn/bus/samples/SecureInterface.java
java/samples/JavaSDKDoc/JavaSDKDocSecurityRsaService/src/org/alljoyn/bus/samples/Service.java
java/samples/JavaSDKDoc/JavaSDKDocSecuritySRPClient/src/org/alljoyn/bus/samples/Client.java
java/samples/JavaSDKDoc/JavaSDKDocSecuritySRPClient/src/org/alljoyn/bus/samples/SecureInterface.java
java/samples/JavaSDKDoc/JavaSDKDocSecuritySRPService/src/org/alljoyn/bus/samples/SecureInterface.java
java/samples/JavaSDKDoc/JavaSDKDocSecuritySRPService/src/org/alljoyn/bus/samples/Service.java
java/samples/JavaSDKDoc/JavaSDKDocSignalClient/src/org/alljoyn/bus/samples/Client.java
java/samples/JavaSDKDoc/JavaSDKDocSignalClient/src/org/alljoyn/bus/samples/SampleInterface.java
java/samples/JavaSDKDoc/JavaSDKDocSignalService/src/org/alljoyn/bus/samples/SampleInterface.java
java/samples/JavaSDKDoc/JavaSDKDocSignalService/src/org/alljoyn/bus/samples/Service.java
java/samples/README.txt
js/docs/icon.png
js/docs/index.html
js/docs/widlhtml.css
js/lib/libnpalljoyn.so
js/samples/addressbook/client/alljoyn_init.js
js/samples/addressbook/client/css/style.css
js/samples/addressbook/client/index.html
js/samples/addressbook/client/js/alljoyn.js
js/samples/addressbook/client/js/client.js
js/samples/addressbook/service/alljoyn_init.js
js/samples/addressbook/service/css/style.css
js/samples/addressbook/service/index.html
js/samples/addressbook/service/js/addressbook.js
js/samples/addressbook/service/js/alljoyn.js
js/samples/addressbook/service/js/service.js
js/samples/chat/alljoyn_init.js
js/samples/chat/css/style.css
js/samples/chat/img/tab_host.png
js/samples/chat/img/tab_use.png
js/samples/chat/index.html
js/samples/chat/js/alljoyn.js
js/samples/chat/js/chat.js
js/samples/props/client/alljoyn_init.js
js/samples/props/client/css/style.css
js/samples/props/client/index.html
js/samples/props/client/js/alljoyn.js
js/samples/props/client/js/client.js
js/samples/props/service/alljoyn_init.js
js/samples/props/service/css/style.css
js/samples/props/service/index.html
js/samples/props/service/js/alljoyn.js
js/samples/props/service/js/service.js
js/samples/simple/client/alljoyn_init.js
js/samples/simple/client/css/style.css
js/samples/simple/client/index.html
js/samples/simple/client/js/simple.js
js/samples/simple/service/alljoyn_init.js
js/samples/simple/service/css/style.css
js/samples/simple/service/index.html
js/samples/simple/service/js/simple.js
c/docs/html/
c/inc/alljoyn_c/AjAPI.h
c/inc/alljoyn_c/AuthListener.h
c/inc/alljoyn_c/BusAttachment.h
c/inc/alljoyn_c/BusListener.h
c/inc/alljoyn_c/BusObject.h
c/inc/alljoyn_c/DBusStdDefines.h
c/inc/alljoyn_c/InterfaceDescription.h
c/inc/alljoyn_c/KeyStoreListener.h
c/inc/alljoyn_c/Message.h
c/inc/alljoyn_c/MessageReceiver.h
c/inc/alljoyn_c/MsgArg.h
c/inc/alljoyn_c/ProxyBusObject.h
c/inc/alljoyn_c/Session.h
c/inc/alljoyn_c/SessionListener.h
c/inc/alljoyn_c/SessionPortListener.h
c/inc/alljoyn_c/Status.h
c/inc/alljoyn_c/TransportMask.h
c/inc/alljoyn_c/version.h
c/inc/qcc/platform.h
c/inc/qcc/posix/platform_types.h
c/lib/liballjoyn_c.a
c/lib/liballjoyn_c.so
## DELETE ME ##
