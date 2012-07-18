 
##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##
##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##
##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##


AllJoyn SDK for Windows 7
-------------------------

This subtree contains one complete copy of the AllJoyn SDK for Windows 7, built
for a single CPU (either x86 or x86_64), VARIANT (either debug or release), and
version of MS Visual Studio (either 2008 or 2010).  The CPU, VARIANT, and MSVS
version is normally incorporated into the name of the package or folder
containing this SDK.

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
          - required by Unity binding
unity/  optional Unity language binding         (built from alljoyn_unity)


The contents of each top level folder are further arranged into sub-folders:

        ----------------------------------------------
cpp/    core AllJoyn functionality, implemented in C++
        ----------------------------------------------

    bin/                        executable binaries

        ???

    bin/samples/                pre-built sample programs

    docs/html/                  AllJoyn Core API documentation

    inc/alljoyn                 AllJoyn Core headers
    inc/qcc

    lib/                        AllJoyn Core client libraries

        alljoyn.lib                     implements core API
        ajdaemon.lib                    ???
        daemonlib.lib                   ???
        BundledDaemon.obj               ???

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

    lib/alljoyn_java.dll        implements Java API
    lib/alljoyn_java.exp
    lib/alljoyn_java.lib

    samples/                    sample programs (see README)


        ---------------------------
js/     Javascript language binding
        ---------------------------

    docs/html/                  API documentation

    lib/npalljoyn.dll           implements
    lib/npalljoyn.exp
    lib/npalljoyn.lib

    plugin/npalljoyn.dll        Installable Netscape plugin ???

    samples/                    sample programs


        -----------------------
c/      ANSI C language binding
        -----------------------

    docs/html/                  API documentation

    inc/alljoyn_c               ANSI C headers
    inc/qcc

    lib/                        client libraries


        -------------
unity/  Unity binding
        -------------

    docs/html/                  API documentation

    AllJoyn.unitypackage        installable Unity package ???

    lib/alljoyn_unity.dll       client library

    package_support/            ???

    samples/                    sample apps for Android

## DELETE ME ## partial list of files in Windows 7 SDK
cpp/bin/advtunnel.exe
cpp/bin/advtunnel.pdb
cpp/bin/aes_ccm.exe
cpp/bin/aes_ccm.pdb
cpp/bin/autochat.exe
cpp/bin/autochat.pdb
cpp/bin/bastress.exe
cpp/bin/bastress.pdb
cpp/bin/bastress2.exe
cpp/bin/bastress2.pdb
cpp/bin/bbclient.exe
cpp/bin/bbclient.pdb
cpp/bin/bbjitter.exe
cpp/bin/bbjitter.pdb
cpp/bin/bbjoin.exe
cpp/bin/bbjoin.pdb
cpp/bin/bbservice.exe
cpp/bin/bbservice.pdb
cpp/bin/bbsig.exe
cpp/bin/bbsig.pdb
cpp/bin/bignum.exe
cpp/bin/bignum.pdb
cpp/bin/bttimingclient.exe
cpp/bin/bttimingclient.pdb
cpp/bin/compression.exe
cpp/bin/compression.pdb
cpp/bin/daemonlib.dll
cpp/bin/keystore.exe
cpp/bin/keystore.pdb
cpp/bin/litegen.exe
cpp/bin/litegen.pdb
cpp/bin/marshal.exe
cpp/bin/marshal.pdb
cpp/bin/mouseclient.exe
cpp/bin/mouseclient.pdb
cpp/bin/names.exe
cpp/bin/names.pdb
cpp/bin/ns.exe
cpp/bin/ns.pdb
cpp/bin/rawclient.exe
cpp/bin/rawclient.pdb
cpp/bin/rawservice.exe
cpp/bin/rawservice.pdb
cpp/bin/remarshal.exe
cpp/bin/remarshal.pdb
cpp/bin/rsa.exe
cpp/bin/rsa.pdb
cpp/bin/samples/FileTransferClient.exe
cpp/bin/samples/FileTransferClient.pdb
cpp/bin/samples/FileTransferService.exe
cpp/bin/samples/FileTransferService.pdb
cpp/bin/samples/basic_client.exe
cpp/bin/samples/basic_client.pdb
cpp/bin/samples/basic_service.exe
cpp/bin/samples/basic_service.pdb
cpp/bin/samples/chat.exe
cpp/bin/samples/nameChange_client.exe
cpp/bin/samples/nameChange_client.pdb
cpp/bin/samples/signalConsumer_client.exe
cpp/bin/samples/signalConsumer_client.pdb
cpp/bin/samples/signal_service.exe
cpp/bin/samples/signal_service.pdb
cpp/bin/sessions.exe
cpp/bin/sessions.pdb
cpp/bin/socktest.exe
cpp/bin/socktest.pdb
cpp/bin/srp.exe
cpp/bin/srp.pdb
cpp/bin/unpack.exe
cpp/bin/unpack.pdb
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
cpp/inc/qcc/windows/atomic.h
cpp/inc/qcc/windows/mapping.h
cpp/inc/qcc/windows/platform_types.h
cpp/lib/BundledDaemon.obj
cpp/lib/ajdaemon.lib
cpp/lib/alljoyn.lib
cpp/lib/daemonlib.lib
cpp/samples/FileTransfer/FileTransferClient.cc
cpp/samples/FileTransfer/FileTransferService.cc
cpp/samples/README.txt
cpp/samples/basic/README.txt
cpp/samples/basic/VC2008Win7/MSVC2008-Win7.sln
cpp/samples/basic/VC2008Win7/basic_client/basic_client.vcproj
cpp/samples/basic/VC2008Win7/basic_service/basic_service.vcproj
cpp/samples/basic/VC2008Win7/nameChange_client/nameChange_client.vcproj
cpp/samples/basic/VC2008Win7/signalConsumer_client/signalConsumer_client.vcproj
cpp/samples/basic/VC2008Win7/signal_service/signal_service.vcproj
cpp/samples/basic/basic_client.cc
cpp/samples/basic/basic_service.cc
cpp/samples/basic/nameChange_client.cc
cpp/samples/basic/signalConsumer_client.cc
cpp/samples/basic/signal_service.cc
cpp/samples/chat/VC2008Win7/VC2008Win7.sln
cpp/samples/chat/VC2008Win7/chatsample/chatsample.vcproj
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
java/lib/alljoyn_java.dll
java/lib/alljoyn_java.exp
java/lib/alljoyn_java.lib
java/lib/alljoyn_java.pdb
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
js/lib/npalljoyn.dll
js/lib/npalljoyn.exp
js/lib/npalljoyn.lib
js/lib/npalljoyn.pdb
js/plugin/alljoyn.reg
js/plugin/alljoyn64.reg
js/plugin/npalljoyn.dll
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
js/samples/simple/client/js/alljoyn.js
js/samples/simple/client/js/client.js
js/samples/simple/client/js/simple.js
js/samples/simple/service/alljoyn_init.js
js/samples/simple/service/css/style.css
js/samples/simple/service/index.html
js/samples/simple/service/js/alljoyn.js
js/samples/simple/service/js/service.js
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
c/inc/qcc/windows/mapping.h
c/inc/qcc/windows/platform_types.h
c/lib/alljoyn_c.dll
c/lib/alljoyn_c.lib
c/lib/alljoyn_c.pdb
unity/AllJoyn.unitypackage
unity/docs/html/
unity/lib/alljoyn_unity.dll
unity/package_support/UnityPackage/Assets/Plugins.meta
unity/package_support/UnityPackage/Assets/Plugins/Android.meta
unity/package_support/UnityPackage/Assets/Plugins/Android/AndroidManifest.xml
unity/package_support/UnityPackage/Assets/Plugins/Android/AndroidManifest.xml.meta
unity/package_support/UnityPackage/Assets/Plugins/alljoyn_c.dll
unity/package_support/UnityPackage/Assets/Plugins/alljoyn_unity.dll
unity/package_support/UnityPackage/Assets/Prefabs.meta
unity/package_support/UnityPackage/Assets/Prefabs/AllJoynAgent.prefab
unity/package_support/UnityPackage/Assets/Prefabs/AllJoynAgent.prefab.meta
unity/package_support/UnityPackage/Assets/Scripts.meta
unity/package_support/UnityPackage/Assets/Scripts/AllJoynAgent.cs
unity/package_support/UnityPackage/Assets/Scripts/AllJoynAgent.cs.meta
unity/package_support/UnityPackage/Library/AudioManager.asset
unity/package_support/UnityPackage/Library/BuildSettings.asset
unity/package_support/UnityPackage/Library/DynamicsManager.asset
unity/package_support/UnityPackage/Library/EditorBuildSettings.asset
unity/package_support/UnityPackage/Library/EditorUserBuildSettings.asset
unity/package_support/UnityPackage/Library/EditorUserSettings.asset
unity/package_support/UnityPackage/Library/InputManager.asset
unity/package_support/UnityPackage/Library/InspectorExpandedItems.asset
unity/package_support/UnityPackage/Library/MonoManager.asset
unity/package_support/UnityPackage/Library/NetworkManager.asset
unity/package_support/UnityPackage/Library/ProjectSettings.asset
unity/package_support/UnityPackage/Library/QualitySettings.asset
unity/samples/BasicClient/Assets/Plugins.meta
unity/samples/BasicClient/Assets/Plugins/Android.meta
unity/samples/BasicClient/Assets/Plugins/Android/AndroidManifest.xml
unity/samples/BasicClient/Assets/Plugins/alljoyn_c.dll
unity/samples/BasicClient/Assets/Plugins/alljoyn_unity.dll
unity/samples/BasicClient/Assets/Prefabs.meta
unity/samples/BasicClient/Assets/Prefabs/AllJoynAgent.prefab
unity/samples/BasicClient/Assets/Prefabs/AllJoynAgent.prefab.meta
unity/samples/BasicClient/Assets/Scenes.meta
unity/samples/BasicClient/Assets/Scenes/MainScene.unity
unity/samples/BasicClient/Assets/Scenes/MainScene.unity.meta
unity/samples/BasicClient/Assets/Scripts.meta
unity/samples/BasicClient/Assets/Scripts/AllJoynAgent.cs
unity/samples/BasicClient/Assets/Scripts/AllJoynAgent.cs.meta
unity/samples/BasicClient/Assets/Scripts/AllJoynClient.cs
unity/samples/BasicClient/Assets/Scripts/AllJoynClient.cs.meta
unity/samples/BasicClient/Assets/Scripts/BasicClient.cs
unity/samples/BasicClient/Assets/Scripts/BasicClient.cs.meta
unity/samples/BasicClient/Library/AudioManager.asset
unity/samples/BasicClient/Library/BuildSettings.asset
unity/samples/BasicClient/Library/DynamicsManager.asset
unity/samples/BasicClient/Library/EditorBuildSettings.asset
unity/samples/BasicClient/Library/EditorUserBuildSettings.asset
unity/samples/BasicClient/Library/EditorUserSettings.asset
unity/samples/BasicClient/Library/InputManager.asset
unity/samples/BasicClient/Library/InspectorExpandedItems.asset
unity/samples/BasicClient/Library/MonoManager.asset
unity/samples/BasicClient/Library/NetworkManager.asset
unity/samples/BasicClient/Library/ProjectSettings.asset
unity/samples/BasicClient/Library/QualitySettings.asset
unity/samples/BasicServer/Assets/Plugins.meta
unity/samples/BasicServer/Assets/Plugins/Android.meta
unity/samples/BasicServer/Assets/Plugins/Android/AndroidManifest.xml
unity/samples/BasicServer/Assets/Plugins/alljoyn_c.dll
unity/samples/BasicServer/Assets/Plugins/alljoyn_unity.dll
unity/samples/BasicServer/Assets/Prefabs.meta
unity/samples/BasicServer/Assets/Prefabs/AllJoynAgent.prefab
unity/samples/BasicServer/Assets/Prefabs/AllJoynAgent.prefab.meta
unity/samples/BasicServer/Assets/Scenes.meta
unity/samples/BasicServer/Assets/Scenes/MainScene.unity
unity/samples/BasicServer/Assets/Scenes/MainScene.unity.meta
unity/samples/BasicServer/Assets/Scripts.meta
unity/samples/BasicServer/Assets/Scripts/AllJoynAgent.cs
unity/samples/BasicServer/Assets/Scripts/AllJoynAgent.cs.meta
unity/samples/BasicServer/Assets/Scripts/AllJoynServer.cs
unity/samples/BasicServer/Assets/Scripts/AllJoynServer.cs.meta
unity/samples/BasicServer/Assets/Scripts/BasicServer.cs
unity/samples/BasicServer/Assets/Scripts/BasicServer.cs.meta
unity/samples/BasicServer/Library/AudioManager.asset
unity/samples/BasicServer/Library/BuildSettings.asset
unity/samples/BasicServer/Library/DynamicsManager.asset
unity/samples/BasicServer/Library/EditorBuildSettings.asset
unity/samples/BasicServer/Library/EditorUserBuildSettings.asset
unity/samples/BasicServer/Library/EditorUserSettings.asset
unity/samples/BasicServer/Library/InputManager.asset
unity/samples/BasicServer/Library/InspectorExpandedItems.asset
unity/samples/BasicServer/Library/MonoManager.asset
unity/samples/BasicServer/Library/NetworkManager.asset
unity/samples/BasicServer/Library/ProjectSettings.asset
unity/samples/BasicServer/Library/QualitySettings.asset
unity/samples/Chat/Assets/Plugins.meta
unity/samples/Chat/Assets/Plugins/Android/AndroidManifest.xml
unity/samples/Chat/Assets/Plugins/alljoyn_c.dll
unity/samples/Chat/Assets/Plugins/alljoyn_unity.dll
unity/samples/Chat/Assets/Prefabs.meta
unity/samples/Chat/Assets/Prefabs/AllJoynAgent.prefab
unity/samples/Chat/Assets/Prefabs/AllJoynAgent.prefab.meta
unity/samples/Chat/Assets/Scenes.meta
unity/samples/Chat/Assets/Scenes/MainScene.unity
unity/samples/Chat/Assets/Scenes/MainScene.unity.meta
unity/samples/Chat/Assets/Scripts.meta
unity/samples/Chat/Assets/Scripts/AllJoynAgent.cs
unity/samples/Chat/Assets/Scripts/AllJoynAgent.cs.meta
unity/samples/Chat/Assets/Scripts/AllJoynClientServer.cs
unity/samples/Chat/Assets/Scripts/AllJoynClientServer.cs.meta
unity/samples/Chat/Assets/Scripts/BasicChat.cs
unity/samples/Chat/Assets/Scripts/BasicChat.cs.meta
unity/samples/Chat/Library/AudioManager.asset
unity/samples/Chat/Library/BuildSettings.asset
unity/samples/Chat/Library/DynamicsManager.asset
unity/samples/Chat/Library/EditorBuildSettings.asset
unity/samples/Chat/Library/EditorUserBuildSettings.asset
unity/samples/Chat/Library/EditorUserSettings.asset
unity/samples/Chat/Library/InputManager.asset
unity/samples/Chat/Library/InspectorExpandedItems.asset
unity/samples/Chat/Library/MonoManager.asset
unity/samples/Chat/Library/NetworkManager.asset
unity/samples/Chat/Library/ProjectSettings.asset
unity/samples/Chat/Library/QualitySettings.asset
## DELETE ME ##
