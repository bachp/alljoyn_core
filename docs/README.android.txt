 
##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##
##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##
##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##


AllJoyn SDK for Android
-----------------------

This subtree contains one complete copy of the AllJoyn SDK for Android, built
for a single CPU (either x86 or arm) and VARIANT (either debug or release).
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
          - required for Android apps
c/      optional ANSI C language binding        (built from alljoyn_c)
          - required by Unity binding
unity/  optional Unity language binding         (built from alljoyn_unity)


The contents of each top level folder are further arranged into sub-folders:

        ----------------------------------------------
cpp/    core AllJoyn functionality, implemented in C++
        ----------------------------------------------

    alljoyn_android/            ???

        alljoyn/                        ???

        alljoyn_android_ext/            ???

        bundle_test/                    ???

        ice_client_login_service/       ???

        ???

    bin/                        executable binaries

        alljoyn-daemon                  installable AllJoyn daemon
        ???

    docs/html/                  AllJoyn Core API documentation

    inc/alljoyn                 AllJoyn Core headers
    inc/qcc

    lib/                        AllJoyn Core client libraries

        liballjoyn.a                    implements core API
        libajdaemon.a                   ???
        liballjoyn-daemon.a             ???
        libbbdaemon-lib.a               ???
        libdaemon-jni.so                ???
        BundledDaemon.o                 ???

    samples/                    C++ sample apps for Android (see README)


        ---------------------
java/   Java language binding
        ---------------------

    docs/html/                  API documentation

    jar/                        client library, misc jar files

        alljoyn.jar                     implements Java API
        alljoyn-sources.jar             (for VARIANT=debug only)
        alljoyn_test.jar                ???

    lib/liballjoyn_java.so      client library

    samples/                    sample apps for Android (see README)


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

## DELETE ME ## partial list of files in Android SDK
cpp/alljoyn_android/alljoyn/.settings/org.eclipse.jdt.core.prefs
cpp/alljoyn_android/alljoyn/AndroidManifest.xml
cpp/alljoyn_android/alljoyn/bin/AllJoyn.apk
cpp/alljoyn_android/alljoyn/default.properties
cpp/alljoyn_android/alljoyn/jni/Android.mk
cpp/alljoyn_android/alljoyn/jni/Application.mk
cpp/alljoyn_android/alljoyn/jni/daemon-jni.cpp
cpp/alljoyn_android/alljoyn/res/drawable/icon.png
cpp/alljoyn_android/alljoyn/res/layout/debugsettledialog.xml
cpp/alljoyn_android/alljoyn/res/layout/main.xml
cpp/alljoyn_android/alljoyn/res/layout/mainexitdialog.xml
cpp/alljoyn_android/alljoyn/res/values/strings.xml
cpp/alljoyn_android/alljoyn/src/org/alljoyn/bus/alljoyn/AllJoynActivity.java
cpp/alljoyn_android/alljoyn/src/org/alljoyn/bus/alljoyn/AllJoynApp.java
cpp/alljoyn_android/alljoyn/src/org/alljoyn/bus/alljoyn/AllJoynDaemon.java
cpp/alljoyn_android/alljoyn/src/org/alljoyn/bus/alljoyn/AllJoynService.java
cpp/alljoyn_android/alljoyn/src/org/alljoyn/bus/alljoyn/DialogBuilder.java
cpp/alljoyn_android/alljoyn_android_ext/AndroidManifest.xml
cpp/alljoyn_android/alljoyn_android_ext/bin/alljoyn_android_ext.apk
cpp/alljoyn_android/alljoyn_android_ext/default.properties
cpp/alljoyn_android/alljoyn_android_ext/jni/AllJoynAndroidExt.cpp
cpp/alljoyn_android/alljoyn_android_ext/jni/Android.mk
cpp/alljoyn_android/alljoyn_android_ext/jni/Application.mk
cpp/alljoyn_android/alljoyn_android_ext/jni/org_alljoyn_jni_AllJoynAndroidExt.h
cpp/alljoyn_android/alljoyn_android_ext/jni/org_alljoyn_jni_ScanResultMessage.h
cpp/alljoyn_android/alljoyn_android_ext/res/drawable-hdpi/icon.png
cpp/alljoyn_android/alljoyn_android_ext/res/drawable-ldpi/icon.png
cpp/alljoyn_android/alljoyn_android_ext/res/drawable-mdpi/icon.png
cpp/alljoyn_android/alljoyn_android_ext/res/layout/main.xml
cpp/alljoyn_android/alljoyn_android_ext/res/values/strings.xml
cpp/alljoyn_android/alljoyn_android_ext/src/org/alljoyn/jni/AllJoynAndroidExt.java
cpp/alljoyn_android/alljoyn_android_ext/src/org/alljoyn/jni/ScanResultMessage.java
cpp/alljoyn_android/alljoyn_android_ext/src/org/alljoyn/jni/ScanResultsReceiver.java
cpp/alljoyn_android/alljoyn_android_ext/src/org/alljoyn/jni/StartServiceReceiver.java
cpp/alljoyn_android/bundle_test/BundledClient/AndroidManifest.xml
cpp/alljoyn_android/bundle_test/BundledClient/bin/ClientBundle.apk
cpp/alljoyn_android/bundle_test/BundledClient/libs/alljoyn.jar
cpp/alljoyn_android/bundle_test/BundledClient/libs/armeabi/liballjoyn_java.so
cpp/alljoyn_android/bundle_test/BundledClient/libs/armeabi/libdaemon-jni.so
cpp/alljoyn_android/bundle_test/BundledClient/res/drawable-hdpi/icon.png
cpp/alljoyn_android/bundle_test/BundledClient/res/drawable-ldpi/icon.png
cpp/alljoyn_android/bundle_test/BundledClient/res/drawable-mdpi/icon.png
cpp/alljoyn_android/bundle_test/BundledClient/res/layout/debug.xml
cpp/alljoyn_android/bundle_test/BundledClient/res/layout/main.xml
cpp/alljoyn_android/bundle_test/BundledClient/res/menu/mainmenu.xml
cpp/alljoyn_android/bundle_test/BundledClient/res/values/strings.xml
cpp/alljoyn_android/bundle_test/BundledClient/src/org/alljoyn/bus/bundle/tests/clientbundle/Client.java
cpp/alljoyn_android/bundle_test/BundledClient/src/org/alljoyn/bus/bundle/tests/clientbundle/DialogBuilder.java
cpp/alljoyn_android/bundle_test/BundledClient/src/org/alljoyn/bus/bundle/tests/clientbundle/ServiceInterface.java
cpp/alljoyn_android/bundle_test/BundledService/AndroidManifest.xml
cpp/alljoyn_android/bundle_test/BundledService/bin/ServiceBundle.apk
cpp/alljoyn_android/bundle_test/BundledService/libs/alljoyn.jar
cpp/alljoyn_android/bundle_test/BundledService/libs/armeabi/liballjoyn_java.so
cpp/alljoyn_android/bundle_test/BundledService/libs/armeabi/libdaemon-jni.so
cpp/alljoyn_android/bundle_test/BundledService/res/drawable-hdpi/icon.png
cpp/alljoyn_android/bundle_test/BundledService/res/drawable-ldpi/icon.png
cpp/alljoyn_android/bundle_test/BundledService/res/drawable-mdpi/icon.png
cpp/alljoyn_android/bundle_test/BundledService/res/layout/main.xml
cpp/alljoyn_android/bundle_test/BundledService/res/menu/mainmenu.xml
cpp/alljoyn_android/bundle_test/BundledService/res/values/strings.xml
cpp/alljoyn_android/bundle_test/BundledService/src/org/alljoyn/bus/bundle/tests/bundledservice/Service.java
cpp/alljoyn_android/bundle_test/BundledService/src/org/alljoyn/bus/bundle/tests/bundledservice/ServiceInterface.java
cpp/alljoyn_android/ice_client_login_service/AndroidManifest.xml
cpp/alljoyn_android/ice_client_login_service/bin/ClientLoginService.apk
cpp/alljoyn_android/ice_client_login_service/default.properties
cpp/alljoyn_android/ice_client_login_service/libs/alljoyn.jar
cpp/alljoyn_android/ice_client_login_service/libs/armeabi/liballjoyn_java.so
cpp/alljoyn_android/ice_client_login_service/res/drawable-hdpi/icon.png
cpp/alljoyn_android/ice_client_login_service/res/drawable-ldpi/icon.png
cpp/alljoyn_android/ice_client_login_service/res/drawable-mdpi/icon.png
cpp/alljoyn_android/ice_client_login_service/res/layout/main.xml
cpp/alljoyn_android/ice_client_login_service/res/values/strings.xml
cpp/alljoyn_android/ice_client_login_service/src/org/alljoyn/ice/ClientLoginService.java
cpp/alljoyn_android/ice_client_login_service/src/org/alljoyn/ice/ClientLoginServiceInterface.java
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
cpp/lib/liballjoyn-daemon.a
cpp/lib/liballjoyn.a
cpp/lib/libbbdaemon-lib.a
cpp/lib/libdaemon-jni.so
cpp/samples/README.txt
cpp/samples/chat/AndroidManifest.xml
cpp/samples/chat/bin/Chat.apk
cpp/samples/chat/default.properties
cpp/samples/chat/jni/Android.mk
cpp/samples/chat/jni/Application.mk
cpp/samples/chat/jni/Chat_jni.cpp
cpp/samples/chat/jni/org_alljoyn_bus_samples_chat_Chat.h
cpp/samples/chat/res/drawable-hdpi/icon.png
cpp/samples/chat/res/drawable-ldpi/icon.png
cpp/samples/chat/res/drawable-mdpi/icon.png
cpp/samples/chat/res/layout/advertise.xml
cpp/samples/chat/res/layout/choice.xml
cpp/samples/chat/res/layout/joinsession.xml
cpp/samples/chat/res/layout/main.xml
cpp/samples/chat/res/layout/message.xml
cpp/samples/chat/res/menu/mainmenu.xml
cpp/samples/chat/res/values/strings.xml
cpp/samples/chat/src/org/alljoyn/bus/samples/chat/Chat.java
cpp/samples/simple/client/.settings/org.eclipse.jdt.core.prefs
cpp/samples/simple/client/AndroidManifest.xml
cpp/samples/simple/client/bin/SimpleClient.apk
cpp/samples/simple/client/default.properties
cpp/samples/simple/client/jni/Android.mk
cpp/samples/simple/client/jni/Application.mk
cpp/samples/simple/client/jni/Client_jni.cpp
cpp/samples/simple/client/jni/org_alljoyn_bus_samples_simpleclient_Client.h
cpp/samples/simple/client/res/drawable-hdpi/icon.png
cpp/samples/simple/client/res/drawable-ldpi/icon.png
cpp/samples/simple/client/res/drawable-mdpi/icon.png
cpp/samples/simple/client/res/layout/main.xml
cpp/samples/simple/client/res/layout/message.xml
cpp/samples/simple/client/res/layout/service.xml
cpp/samples/simple/client/res/menu/mainmenu.xml
cpp/samples/simple/client/res/values/id.xml
cpp/samples/simple/client/res/values/strings.xml
cpp/samples/simple/client/src/org/alljoyn/bus/samples/simpleclient/BusNameItem.java
cpp/samples/simple/client/src/org/alljoyn/bus/samples/simpleclient/BusNameItemAdapter.java
cpp/samples/simple/client/src/org/alljoyn/bus/samples/simpleclient/Client.java
cpp/samples/simple/service/.settings/org.eclipse.jdt.core.prefs
cpp/samples/simple/service/AndroidManifest.xml
cpp/samples/simple/service/bin/SimpleService.apk
cpp/samples/simple/service/default.properties
cpp/samples/simple/service/jni/Android.mk
cpp/samples/simple/service/jni/Application.mk
cpp/samples/simple/service/jni/Service_jni.cpp
cpp/samples/simple/service/jni/org_alljoyn_bus_samples_simpleservice_Service.h
cpp/samples/simple/service/res/drawable-hdpi/icon.png
cpp/samples/simple/service/res/drawable-ldpi/icon.png
cpp/samples/simple/service/res/drawable-mdpi/icon.png
cpp/samples/simple/service/res/layout/main.xml
cpp/samples/simple/service/res/layout/message.xml
cpp/samples/simple/service/res/menu/mainmenu.xml
cpp/samples/simple/service/res/values/strings.xml
cpp/samples/simple/service/src/org/alljoyn/bus/samples/simpleservice/Service.java
java/docs/html/
java/jar/alljoyn-sources.jar
java/jar/alljoyn.jar
java/jar/alljoyn_test.jar
java/lib/liballjoyn_java.so
java/samples/README.txt
java/samples/chat/AndroidManifest.xml
java/samples/chat/bin/Chat.apk
java/samples/chat/libs/alljoyn.jar
java/samples/chat/libs/armeabi/liballjoyn_java.so
java/samples/chat/res/drawable/ic_tab_host.xml
java/samples/chat/res/drawable/ic_tab_host_disabled.png
java/samples/chat/res/drawable/ic_tab_host_enabled.png
java/samples/chat/res/drawable/ic_tab_use.xml
java/samples/chat/res/drawable/ic_tab_use_disabled.png
java/samples/chat/res/drawable/ic_tab_use_enabled.png
java/samples/chat/res/drawable/icon.png
java/samples/chat/res/layout/alljoynerrordialog.xml
java/samples/chat/res/layout/host.xml
java/samples/chat/res/layout/hostnamedialog.xml
java/samples/chat/res/layout/hoststartdialog.xml
java/samples/chat/res/layout/hoststopdialog.xml
java/samples/chat/res/layout/main.xml
java/samples/chat/res/layout/use.xml
java/samples/chat/res/layout/usejoindialog.xml
java/samples/chat/res/layout/useleavedialog.xml
java/samples/chat/res/values/strings.xml
java/samples/chat/src/org/alljoyn/bus/sample/chat/AllJoynService.java
java/samples/chat/src/org/alljoyn/bus/sample/chat/ChatApplication.java
java/samples/chat/src/org/alljoyn/bus/sample/chat/ChatInterface.java
java/samples/chat/src/org/alljoyn/bus/sample/chat/DialogBuilder.java
java/samples/chat/src/org/alljoyn/bus/sample/chat/HostActivity.java
java/samples/chat/src/org/alljoyn/bus/sample/chat/Observable.java
java/samples/chat/src/org/alljoyn/bus/sample/chat/Observer.java
java/samples/chat/src/org/alljoyn/bus/sample/chat/TabWidget.java
java/samples/chat/src/org/alljoyn/bus/sample/chat/UseActivity.java
java/samples/contacts/ContactsClient/AndroidManifest.xml
java/samples/contacts/ContactsClient/bin/ContactsClient.apk
java/samples/contacts/ContactsClient/libs/alljoyn.jar
java/samples/contacts/ContactsClient/libs/armeabi/liballjoyn_java.so
java/samples/contacts/ContactsClient/res/drawable-hdpi/icon.png
java/samples/contacts/ContactsClient/res/drawable-ldpi/icon.png
java/samples/contacts/ContactsClient/res/drawable-mdpi/icon.png
java/samples/contacts/ContactsClient/res/layout/contact.xml
java/samples/contacts/ContactsClient/res/layout/main.xml
java/samples/contacts/ContactsClient/res/menu/mainmenu.xml
java/samples/contacts/ContactsClient/res/values/strings.xml
java/samples/contacts/ContactsClient/src/org/alljoyn/bus/samples/contacts_client/AddressBookInterface.java
java/samples/contacts/ContactsClient/src/org/alljoyn/bus/samples/contacts_client/Contact.java
java/samples/contacts/ContactsClient/src/org/alljoyn/bus/samples/contacts_client/ContactsClient.java
java/samples/contacts/ContactsClient/src/org/alljoyn/bus/samples/contacts_client/NameId.java
java/samples/contacts/ContactsService/AndroidManifest.xml
java/samples/contacts/ContactsService/bin/ContactsService.apk
java/samples/contacts/ContactsService/libs/alljoyn.jar
java/samples/contacts/ContactsService/libs/armeabi/liballjoyn_java.so
java/samples/contacts/ContactsService/res/drawable-hdpi/icon.png
java/samples/contacts/ContactsService/res/drawable-ldpi/icon.png
java/samples/contacts/ContactsService/res/drawable-mdpi/icon.png
java/samples/contacts/ContactsService/res/layout/main.xml
java/samples/contacts/ContactsService/res/menu/mainmenu.xml
java/samples/contacts/ContactsService/res/values/strings.xml
java/samples/contacts/ContactsService/src/org/alljoyn/bus/samples/contacts_service/AddressBookInterface.java
java/samples/contacts/ContactsService/src/org/alljoyn/bus/samples/contacts_service/Contact.java
java/samples/contacts/ContactsService/src/org/alljoyn/bus/samples/contacts_service/ContactsService.java
java/samples/contacts/ContactsService/src/org/alljoyn/bus/samples/contacts_service/NameId.java
java/samples/properties/PropertiesClient/AndroidManifest.xml
java/samples/properties/PropertiesClient/bin/PropertiesClient.apk
java/samples/properties/PropertiesClient/libs/alljoyn.jar
java/samples/properties/PropertiesClient/libs/armeabi/liballjoyn_java.so
java/samples/properties/PropertiesClient/res/drawable-hdpi/icon.png
java/samples/properties/PropertiesClient/res/drawable-ldpi/icon.png
java/samples/properties/PropertiesClient/res/drawable-mdpi/icon.png
java/samples/properties/PropertiesClient/res/layout/main.xml
java/samples/properties/PropertiesClient/res/menu/mainmenu.xml
java/samples/properties/PropertiesClient/res/values/strings.xml
java/samples/properties/PropertiesClient/src/org/alljoyn/bus/samples/properties_client/PropertiesClient.java
java/samples/properties/PropertiesClient/src/org/alljoyn/bus/samples/properties_client/PropertiesInterface.java
java/samples/properties/PropertiesService/AndroidManifest.xml
java/samples/properties/PropertiesService/bin/PropertiesService.apk
java/samples/properties/PropertiesService/libs/alljoyn.jar
java/samples/properties/PropertiesService/libs/armeabi/liballjoyn_java.so
java/samples/properties/PropertiesService/res/drawable-hdpi/icon.png
java/samples/properties/PropertiesService/res/drawable-ldpi/icon.png
java/samples/properties/PropertiesService/res/drawable-mdpi/icon.png
java/samples/properties/PropertiesService/res/layout/main.xml
java/samples/properties/PropertiesService/res/menu/mainmenu.xml
java/samples/properties/PropertiesService/res/values/strings.xml
java/samples/properties/PropertiesService/src/org/alljoyn/bus/samples/properties_service/PropertiesInterface.java
java/samples/properties/PropertiesService/src/org/alljoyn/bus/samples/properties_service/PropertiesService.java
java/samples/raw/client/AndroidManifest.xml
java/samples/raw/client/bin/RawClient.apk
java/samples/raw/client/libs/alljoyn.jar
java/samples/raw/client/libs/armeabi/liballjoyn_java.so
java/samples/raw/client/res/drawable-hdpi/icon.png
java/samples/raw/client/res/drawable-ldpi/icon.png
java/samples/raw/client/res/drawable-mdpi/icon.png
java/samples/raw/client/res/layout/main.xml
java/samples/raw/client/res/layout/message.xml
java/samples/raw/client/res/menu/mainmenu.xml
java/samples/raw/client/res/values/strings.xml
java/samples/raw/client/src/org/alljoyn/bus/samples/rawclient/Client.java
java/samples/raw/client/src/org/alljoyn/bus/samples/rawclient/RawInterface.java
java/samples/raw/service/AndroidManifest.xml
java/samples/raw/service/bin/RawService.apk
java/samples/raw/service/libs/alljoyn.jar
java/samples/raw/service/libs/armeabi/liballjoyn_java.so
java/samples/raw/service/res/drawable-hdpi/icon.png
java/samples/raw/service/res/drawable-ldpi/icon.png
java/samples/raw/service/res/drawable-mdpi/icon.png
java/samples/raw/service/res/layout/main.xml
java/samples/raw/service/res/layout/message.xml
java/samples/raw/service/res/menu/mainmenu.xml
java/samples/raw/service/res/values/strings.xml
java/samples/raw/service/src/org/alljoyn/bus/samples/rawservice/RawInterface.java
java/samples/raw/service/src/org/alljoyn/bus/samples/rawservice/Service.java
java/samples/secure/logonclient/AndroidManifest.xml
java/samples/secure/logonclient/bin/SecureLogonClient.apk
java/samples/secure/logonclient/libs/alljoyn.jar
java/samples/secure/logonclient/libs/armeabi/liballjoyn_java.so
java/samples/secure/logonclient/res/drawable-hdpi/icon.png
java/samples/secure/logonclient/res/drawable-ldpi/icon.png
java/samples/secure/logonclient/res/drawable-mdpi/icon.png
java/samples/secure/logonclient/res/layout/alert_dialog.xml
java/samples/secure/logonclient/res/layout/logon_dialog.xml
java/samples/secure/logonclient/res/layout/main.xml
java/samples/secure/logonclient/res/layout/message.xml
java/samples/secure/logonclient/res/menu/mainmenu.xml
java/samples/secure/logonclient/res/values/strings.xml
java/samples/secure/logonclient/src/org/alljoyn/bus/samples/logonclient/Client.java
java/samples/secure/logonclient/src/org/alljoyn/bus/samples/logonclient/SecureInterface.java
java/samples/secure/rsaclient/AndroidManifest.xml
java/samples/secure/rsaclient/bin/SecureRsaClient.apk
java/samples/secure/rsaclient/libs/alljoyn.jar
java/samples/secure/rsaclient/libs/armeabi/liballjoyn_java.so
java/samples/secure/rsaclient/res/drawable-hdpi/icon.png
java/samples/secure/rsaclient/res/drawable-ldpi/icon.png
java/samples/secure/rsaclient/res/drawable-mdpi/icon.png
java/samples/secure/rsaclient/res/layout/alert_dialog.xml
java/samples/secure/rsaclient/res/layout/main.xml
java/samples/secure/rsaclient/res/layout/message.xml
java/samples/secure/rsaclient/res/layout/passphrase_dialog.xml
java/samples/secure/rsaclient/res/menu/mainmenu.xml
java/samples/secure/rsaclient/res/values/strings.xml
java/samples/secure/rsaclient/src/org/alljoyn/bus/samples/rsaclient/Client.java
java/samples/secure/rsaclient/src/org/alljoyn/bus/samples/rsaclient/SecureInterface.java
java/samples/secure/service/AndroidManifest.xml
java/samples/secure/service/bin/SecureService.apk
java/samples/secure/service/libs/alljoyn.jar
java/samples/secure/service/libs/armeabi/liballjoyn_java.so
java/samples/secure/service/res/drawable-hdpi/icon.png
java/samples/secure/service/res/drawable-ldpi/icon.png
java/samples/secure/service/res/drawable-mdpi/icon.png
java/samples/secure/service/res/layout/alert_dialog.xml
java/samples/secure/service/res/layout/main.xml
java/samples/secure/service/res/layout/message.xml
java/samples/secure/service/res/layout/one_time_password_dialog.xml
java/samples/secure/service/res/layout/passphrase_dialog.xml
java/samples/secure/service/res/menu/mainmenu.xml
java/samples/secure/service/res/values/strings.xml
java/samples/secure/service/src/org/alljoyn/bus/samples/secureservice/SecureInterface.java
java/samples/secure/service/src/org/alljoyn/bus/samples/secureservice/Service.java
java/samples/secure/srpclient/AndroidManifest.xml
java/samples/secure/srpclient/bin/SecureSrpClient.apk
java/samples/secure/srpclient/libs/alljoyn.jar
java/samples/secure/srpclient/libs/armeabi/liballjoyn_java.so
java/samples/secure/srpclient/res/drawable-hdpi/icon.png
java/samples/secure/srpclient/res/drawable-ldpi/icon.png
java/samples/secure/srpclient/res/drawable-mdpi/icon.png
java/samples/secure/srpclient/res/layout/alert_dialog.xml
java/samples/secure/srpclient/res/layout/main.xml
java/samples/secure/srpclient/res/layout/message.xml
java/samples/secure/srpclient/res/menu/mainmenu.xml
java/samples/secure/srpclient/res/values/strings.xml
java/samples/secure/srpclient/src/org/alljoyn/bus/samples/srpclient/Client.java
java/samples/secure/srpclient/src/org/alljoyn/bus/samples/srpclient/SecureInterface.java
java/samples/sessionless/client/AndroidManifest.xml
java/samples/sessionless/client/bin/SessionlessClient.apk
java/samples/sessionless/client/libs/alljoyn.jar
java/samples/sessionless/client/libs/armeabi/liballjoyn_java.so
java/samples/sessionless/client/res/drawable-hdpi/icon.png
java/samples/sessionless/client/res/drawable-ldpi/icon.png
java/samples/sessionless/client/res/drawable-mdpi/icon.png
java/samples/sessionless/client/res/layout/main.xml
java/samples/sessionless/client/res/layout/message.xml
java/samples/sessionless/client/res/menu/mainmenu.xml
java/samples/sessionless/client/res/values/strings.xml
java/samples/sessionless/client/src/org/alljoyn/bus/samples/slclient/Client.java
java/samples/sessionless/client/src/org/alljoyn/bus/samples/slclient/SimpleInterface.java
java/samples/sessionless/service/AndroidManifest.xml
java/samples/sessionless/service/bin/SessionlessService.apk
java/samples/sessionless/service/libs/alljoyn.jar
java/samples/sessionless/service/libs/armeabi/liballjoyn_java.so
java/samples/sessionless/service/res/drawable-hdpi/icon.png
java/samples/sessionless/service/res/drawable-ldpi/icon.png
java/samples/sessionless/service/res/drawable-mdpi/icon.png
java/samples/sessionless/service/res/layout/main.xml
java/samples/sessionless/service/res/layout/message.xml
java/samples/sessionless/service/res/menu/mainmenu.xml
java/samples/sessionless/service/res/values/strings.xml
java/samples/sessionless/service/src/org/alljoyn/bus/samples/slservice/Service.java
java/samples/sessionless/service/src/org/alljoyn/bus/samples/slservice/SimpleInterface.java
java/samples/simple/client/AndroidManifest.xml
java/samples/simple/client/bin/SimpleClient.apk
java/samples/simple/client/libs/alljoyn.jar
java/samples/simple/client/libs/armeabi/liballjoyn_java.so
java/samples/simple/client/res/drawable-hdpi/icon.png
java/samples/simple/client/res/drawable-ldpi/icon.png
java/samples/simple/client/res/drawable-mdpi/icon.png
java/samples/simple/client/res/layout/main.xml
java/samples/simple/client/res/layout/message.xml
java/samples/simple/client/res/menu/mainmenu.xml
java/samples/simple/client/res/values/strings.xml
java/samples/simple/client/src/org/alljoyn/bus/samples/simpleclient/Client.java
java/samples/simple/client/src/org/alljoyn/bus/samples/simpleclient/SimpleInterface.java
java/samples/simple/service/AndroidManifest.xml
java/samples/simple/service/bin/SimpleService.apk
java/samples/simple/service/libs/alljoyn.jar
java/samples/simple/service/libs/armeabi/liballjoyn_java.so
java/samples/simple/service/res/drawable-hdpi/icon.png
java/samples/simple/service/res/drawable-ldpi/icon.png
java/samples/simple/service/res/drawable-mdpi/icon.png
java/samples/simple/service/res/layout/main.xml
java/samples/simple/service/res/layout/message.xml
java/samples/simple/service/res/menu/mainmenu.xml
java/samples/simple/service/res/values/strings.xml
java/samples/simple/service/src/org/alljoyn/bus/samples/simpleservice/Service.java
java/samples/simple/service/src/org/alljoyn/bus/samples/simpleservice/SimpleInterface.java
java/samples/simple/wfd_service/AndroidManifest.xml
java/samples/simple/wfd_service/bin/WFDSimpleService.apk
java/samples/simple/wfd_service/libs/alljoyn.jar
java/samples/simple/wfd_service/libs/armeabi/liballjoyn_java.so
java/samples/simple/wfd_service/res/drawable-hdpi/icon.png
java/samples/simple/wfd_service/res/drawable-ldpi/icon.png
java/samples/simple/wfd_service/res/drawable-mdpi/icon.png
java/samples/simple/wfd_service/res/layout/main.xml
java/samples/simple/wfd_service/res/layout/message.xml
java/samples/simple/wfd_service/res/menu/mainmenu.xml
java/samples/simple/wfd_service/res/values/strings.xml
java/samples/simple/wfd_service/src/org/alljoyn/bus/samples/wfdsimpleservice/Service.java
java/samples/simple/wfd_service/src/org/alljoyn/bus/samples/wfdsimpleservice/SimpleInterface.java
java/samples/simple/wfd_service/src/org/alljoyn/bus/samples/wfdsimpleservice/WifiDirectAutoAccept.java
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
unity/AllJoyn.unitypackage
unity/docs/html/
unity/lib/alljoyn_unity.dll
unity/package_support/UnityPackage/Assets/Plugins.meta
unity/package_support/UnityPackage/Assets/Plugins/Android.meta
unity/package_support/UnityPackage/Assets/Plugins/Android/AndroidManifest.xml
unity/package_support/UnityPackage/Assets/Plugins/Android/AndroidManifest.xml.meta
unity/package_support/UnityPackage/Assets/Plugins/Android/liballjoyn_c.so
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
unity/samples/BasicClient/Assets/Plugins/Android/liballjoyn_c.so
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
unity/samples/BasicServer/Assets/Plugins/Android/liballjoyn_c.so
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
unity/samples/Chat/Assets/Plugins/Android/liballjoyn_c.so
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
