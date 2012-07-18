 
##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##
##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##
##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##  DRAFT WORK-IN-PROGRESS  ##


AllJoyn SDK for OSX/iOS
------------------------

This subtree contains one complete copy of the AllJoyn SDK for OSX/iOS.  

This SDK includes all supported combinations of
    target type (OS X, iphone OS, iphone OS simulator)
    CPU         (x86, arm, armv7, armv7s)
    VARIANT     (Debug or Release)

Please see alljoyn_objc/README-INSTALLING.txt for instructions on how to
install this SDK and use it with Xcode.

Please see ReleaseNotes.txt for the applicable AllJoyn release version and
related information on new features and known issues.


Summary of file and directory structure:
----------------------------------------

alljoyn_core/       core AllJoyn functionality, implemented in C++
                      - built from Git projects alljoyn_core, common, alljoyn_objc
                      - required for all AllJoyn applications

    build/darwin/.../dist/cpp/inc/          headers and client libraries,
    build/darwin/.../dist/cpp/lib/            in sub-folders by target platform.

    docs/html/                              AllJoyn Core API documentation
    AllJoyn_API_Changes_cpp.txt


alljoyn_objc/       Objective-C language binding for OSX/iOS
                      - Git project alljoyn_objc, as source library

    AllJoynFramework/                       ???
    AllJoynFramework_iOS/

    AllJoynCodeGenerator/                   ???

    Unity/                                  ???

    alljoyn_darwin.xcodeproj/               ???

    samples/OSX/                            sample apps
    samples/iOS/

## DELETE ME ## partial list of files in OS-X/iOS SDK
alljoyn_core/AllJoyn_API_Changes_cpp.txt
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/AllJoynStd.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/AuthListener.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/BusAttachment.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/BusListener.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/BusObject.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/DBusStd.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/DBusStdDefines.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/InterfaceDescription.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/KeyStoreListener.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/Message.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/MessageReceiver.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/MessageSink.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/MsgArg.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/ProxyBusObject.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/Session.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/SessionListener.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/SessionPortListener.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/SimpleBusListener.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/Status.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/TransportMask.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/alljoyn/version.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/qcc/Log.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/qcc/ManagedObj.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/qcc/String.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/qcc/atomic.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/qcc/platform.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/qcc/posix/atomic.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/inc/qcc/posix/platform_types.h
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/lib/BundledDaemon.o
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/lib/libajdaemon.a
alljoyn_core/build/darwin/arm/iphoneos/debug/dist/cpp/lib/liballjoyn.a
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/AllJoynStd.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/AuthListener.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/BusAttachment.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/BusListener.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/BusObject.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/DBusStd.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/DBusStdDefines.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/InterfaceDescription.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/KeyStoreListener.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/Message.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/MessageReceiver.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/MessageSink.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/MsgArg.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/ProxyBusObject.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/Session.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/SessionListener.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/SessionPortListener.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/SimpleBusListener.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/Status.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/TransportMask.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/alljoyn/version.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/qcc/Log.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/qcc/ManagedObj.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/qcc/String.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/qcc/atomic.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/qcc/platform.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/qcc/posix/atomic.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/inc/qcc/posix/platform_types.h
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/lib/BundledDaemon.o
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/lib/libajdaemon.a
alljoyn_core/build/darwin/arm/iphoneos/release/dist/cpp/lib/liballjoyn.a
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/AllJoynStd.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/AuthListener.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/BusAttachment.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/BusListener.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/BusObject.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/DBusStd.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/DBusStdDefines.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/InterfaceDescription.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/KeyStoreListener.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/Message.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/MessageReceiver.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/MessageSink.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/MsgArg.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/ProxyBusObject.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/Session.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/SessionListener.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/SessionPortListener.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/SimpleBusListener.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/Status.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/TransportMask.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/alljoyn/version.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/qcc/Log.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/qcc/ManagedObj.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/qcc/String.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/qcc/atomic.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/qcc/platform.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/qcc/posix/atomic.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/inc/qcc/posix/platform_types.h
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/lib/BundledDaemon.o
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/lib/libajdaemon.a
alljoyn_core/build/darwin/arm/iphonesimulator/debug/dist/cpp/lib/liballjoyn.a
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/AllJoynStd.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/AuthListener.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/BusAttachment.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/BusListener.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/BusObject.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/DBusStd.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/DBusStdDefines.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/InterfaceDescription.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/KeyStoreListener.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/Message.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/MessageReceiver.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/MessageSink.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/MsgArg.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/ProxyBusObject.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/Session.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/SessionListener.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/SessionPortListener.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/SimpleBusListener.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/Status.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/TransportMask.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/alljoyn/version.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/qcc/Log.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/qcc/ManagedObj.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/qcc/String.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/qcc/atomic.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/qcc/platform.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/qcc/posix/atomic.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/inc/qcc/posix/platform_types.h
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/lib/BundledDaemon.o
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/lib/libajdaemon.a
alljoyn_core/build/darwin/arm/iphonesimulator/release/dist/cpp/lib/liballjoyn.a
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/AllJoynStd.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/AuthListener.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/BusAttachment.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/BusListener.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/BusObject.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/DBusStd.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/DBusStdDefines.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/InterfaceDescription.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/KeyStoreListener.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/Message.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/MessageReceiver.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/MessageSink.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/MsgArg.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/ProxyBusObject.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/Session.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/SessionListener.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/SessionPortListener.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/SimpleBusListener.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/Status.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/TransportMask.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/alljoyn/version.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/qcc/Log.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/qcc/ManagedObj.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/qcc/String.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/qcc/atomic.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/qcc/platform.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/qcc/posix/atomic.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/inc/qcc/posix/platform_types.h
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/lib/BundledDaemon.o
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/lib/libajdaemon.a
alljoyn_core/build/darwin/armv7/iphoneos/debug/dist/cpp/lib/liballjoyn.a
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/AllJoynStd.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/AuthListener.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/BusAttachment.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/BusListener.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/BusObject.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/DBusStd.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/DBusStdDefines.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/InterfaceDescription.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/KeyStoreListener.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/Message.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/MessageReceiver.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/MessageSink.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/MsgArg.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/ProxyBusObject.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/Session.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/SessionListener.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/SessionPortListener.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/SimpleBusListener.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/Status.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/TransportMask.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/alljoyn/version.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/qcc/Log.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/qcc/ManagedObj.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/qcc/String.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/qcc/atomic.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/qcc/platform.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/qcc/posix/atomic.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/inc/qcc/posix/platform_types.h
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/lib/BundledDaemon.o
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/lib/libajdaemon.a
alljoyn_core/build/darwin/armv7/iphoneos/release/dist/cpp/lib/liballjoyn.a
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/AllJoynStd.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/AuthListener.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/BusAttachment.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/BusListener.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/BusObject.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/DBusStd.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/DBusStdDefines.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/InterfaceDescription.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/KeyStoreListener.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/Message.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/MessageReceiver.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/MessageSink.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/MsgArg.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/ProxyBusObject.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/Session.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/SessionListener.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/SessionPortListener.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/SimpleBusListener.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/Status.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/TransportMask.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/alljoyn/version.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/qcc/Log.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/qcc/ManagedObj.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/qcc/String.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/qcc/atomic.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/qcc/platform.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/qcc/posix/atomic.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/inc/qcc/posix/platform_types.h
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/lib/BundledDaemon.o
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/lib/libajdaemon.a
alljoyn_core/build/darwin/armv7s/iphoneos/debug/dist/cpp/lib/liballjoyn.a
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/AllJoynStd.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/AuthListener.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/BusAttachment.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/BusListener.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/BusObject.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/DBusStd.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/DBusStdDefines.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/InterfaceDescription.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/KeyStoreListener.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/Message.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/MessageReceiver.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/MessageSink.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/MsgArg.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/ProxyBusObject.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/Session.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/SessionListener.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/SessionPortListener.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/SimpleBusListener.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/Status.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/TransportMask.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/alljoyn/version.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/qcc/Log.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/qcc/ManagedObj.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/qcc/String.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/qcc/atomic.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/qcc/platform.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/qcc/posix/atomic.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/inc/qcc/posix/platform_types.h
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/lib/BundledDaemon.o
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/lib/libajdaemon.a
alljoyn_core/build/darwin/armv7s/iphoneos/release/dist/cpp/lib/liballjoyn.a
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/advtunnel
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/aes_ccm
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/autochat
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/bastress
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/bastress2
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/bbclient
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/bbjitter
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/bbjoin
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/bbservice
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/bbsig
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/bignum
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/bttimingclient
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/compression
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/keystore
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/marshal
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/names
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/ns
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/packettest
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/rawclient
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/rawservice
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/remarshal
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/rsa
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/samples/FileTransferClient
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/samples/FileTransferService
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/samples/basic_client
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/samples/basic_service
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/samples/chat
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/samples/nameChange_client
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/samples/signalConsumer_client
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/samples/signal_service
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/sessions
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/socktest
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/srp
alljoyn_core/build/darwin/x86/debug/dist/cpp/bin/unpack
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/AllJoynStd.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/AuthListener.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/BusAttachment.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/BusListener.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/BusObject.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/DBusStd.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/DBusStdDefines.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/InterfaceDescription.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/KeyStoreListener.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/Message.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/MessageReceiver.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/MessageSink.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/MsgArg.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/ProxyBusObject.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/Session.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/SessionListener.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/SessionPortListener.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/SimpleBusListener.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/Status.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/TransportMask.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/alljoyn/version.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/qcc/Log.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/qcc/ManagedObj.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/qcc/String.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/qcc/atomic.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/qcc/platform.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/qcc/posix/atomic.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/inc/qcc/posix/platform_types.h
alljoyn_core/build/darwin/x86/debug/dist/cpp/lib/BundledDaemon.o
alljoyn_core/build/darwin/x86/debug/dist/cpp/lib/libajdaemon.a
alljoyn_core/build/darwin/x86/debug/dist/cpp/lib/liballjoyn.a
alljoyn_core/build/darwin/x86/debug/dist/cpp/samples/FileTransfer/FileTransferClient.cc
alljoyn_core/build/darwin/x86/debug/dist/cpp/samples/FileTransfer/FileTransferService.cc
alljoyn_core/build/darwin/x86/debug/dist/cpp/samples/basic/Makefile
alljoyn_core/build/darwin/x86/debug/dist/cpp/samples/basic/README.txt
alljoyn_core/build/darwin/x86/debug/dist/cpp/samples/basic/basic_client.cc
alljoyn_core/build/darwin/x86/debug/dist/cpp/samples/basic/basic_service.cc
alljoyn_core/build/darwin/x86/debug/dist/cpp/samples/basic/nameChange_client.cc
alljoyn_core/build/darwin/x86/debug/dist/cpp/samples/basic/signalConsumer_client.cc
alljoyn_core/build/darwin/x86/debug/dist/cpp/samples/basic/signal_service.cc
alljoyn_core/build/darwin/x86/debug/dist/cpp/samples/chat/Makefile
alljoyn_core/build/darwin/x86/debug/dist/cpp/samples/chat/chat.cc
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/advtunnel
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/aes_ccm
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/autochat
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/bastress
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/bastress2
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/bbclient
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/bbjitter
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/bbjoin
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/bbservice
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/bbsig
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/bignum
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/bttimingclient
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/compression
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/keystore
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/marshal
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/names
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/ns
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/packettest
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/rawclient
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/rawservice
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/remarshal
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/rsa
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/samples/FileTransferClient
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/samples/FileTransferService
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/samples/basic_client
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/samples/basic_service
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/samples/chat
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/samples/nameChange_client
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/samples/signalConsumer_client
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/samples/signal_service
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/sessions
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/socktest
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/srp
alljoyn_core/build/darwin/x86/release/dist/cpp/bin/unpack
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/AllJoynStd.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/AuthListener.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/BusAttachment.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/BusListener.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/BusObject.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/DBusStd.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/DBusStdDefines.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/InterfaceDescription.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/KeyStoreListener.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/Message.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/MessageReceiver.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/MessageSink.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/MsgArg.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/ProxyBusObject.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/Session.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/SessionListener.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/SessionPortListener.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/SimpleBusListener.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/Status.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/TransportMask.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/alljoyn/version.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/qcc/Log.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/qcc/ManagedObj.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/qcc/String.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/qcc/atomic.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/qcc/platform.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/qcc/posix/atomic.h
alljoyn_core/build/darwin/x86/release/dist/cpp/inc/qcc/posix/platform_types.h
alljoyn_core/build/darwin/x86/release/dist/cpp/lib/BundledDaemon.o
alljoyn_core/build/darwin/x86/release/dist/cpp/lib/libajdaemon.a
alljoyn_core/build/darwin/x86/release/dist/cpp/lib/liballjoyn.a
alljoyn_core/build/darwin/x86/release/dist/cpp/samples/FileTransfer/FileTransferClient.cc
alljoyn_core/build/darwin/x86/release/dist/cpp/samples/FileTransfer/FileTransferService.cc
alljoyn_core/build/darwin/x86/release/dist/cpp/samples/basic/Makefile
alljoyn_core/build/darwin/x86/release/dist/cpp/samples/basic/README.txt
alljoyn_core/build/darwin/x86/release/dist/cpp/samples/basic/basic_client.cc
alljoyn_core/build/darwin/x86/release/dist/cpp/samples/basic/basic_service.cc
alljoyn_core/build/darwin/x86/release/dist/cpp/samples/basic/nameChange_client.cc
alljoyn_core/build/darwin/x86/release/dist/cpp/samples/basic/signalConsumer_client.cc
alljoyn_core/build/darwin/x86/release/dist/cpp/samples/basic/signal_service.cc
alljoyn_core/build/darwin/x86/release/dist/cpp/samples/chat/Makefile
alljoyn_core/build/darwin/x86/release/dist/cpp/samples/chat/chat.cc
alljoyn_core/docs/html/
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator.xcodeproj/project.pbxproj
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator.xcodeproj/project.xcworkspace/contents.xcworkspacedata
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/AllJoynCodeGenerator.xcscheme
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/xcschememanagement.plist
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator/AJNBasicObject.h
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator/AJNBasicObject.mm
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator/AllJoynCodeGenerator-Prefix.pch
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator/AllJoynCodeGenerator.1
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator/BasicObject.h
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator/BasicObject.m
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator/BasicObjectModel.xml
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator/SConscript
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator/main.m
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator/objcExtensionHeader.xsl
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator/objcExtensionSource.xsl
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator/objcHeader.xsl
alljoyn_objc/AllJoynCodeGenerator/AllJoynCodeGenerator/objcSource.xsl
alljoyn_objc/AllJoynFramework/AllJoynFramework.xcodeproj/project.pbxproj
alljoyn_objc/AllJoynFramework/AllJoynFramework.xcodeproj/project.xcworkspace/contents.xcworkspacedata
alljoyn_objc/AllJoynFramework/AllJoynFramework.xcodeproj/xcshareddata/xcschemes/API Documentation.xcscheme
alljoyn_objc/AllJoynFramework/AllJoynFramework.xcodeproj/xcshareddata/xcschemes/AllJoynFramework.xcscheme
alljoyn_objc/AllJoynFramework/AllJoynFramework.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/xcschememanagement.plist
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNAuthenticationListener.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNAuthenticationListenerImpl.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNAuthenticationListenerImpl.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNBus.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNBus.m
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNBusAttachment.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNBusAttachment.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNBusController.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNBusInterface.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNBusListener.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNBusListenerImpl.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNBusListenerImpl.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNBusObject.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNBusObject.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNBusObjectImpl.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNBusObjectImpl.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNClientController.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNClientController.m
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNHandle.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNInterfaceDescription.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNInterfaceDescription.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNInterfaceMember.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNInterfaceMember.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNInterfaceProperty.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNInterfaceProperty.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNKeyStoreListener.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNKeyStoreListenerImpl.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNKeyStoreListenerImpl.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNMessage.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNMessage.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNMessageArgument.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNMessageArgument.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNMessageHeaderFields.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNMessageHeaderFields.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNObject.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNObject.m
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNProxyBusObject.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNProxyBusObject.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNSecurityCredentials.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNSecurityCredentials.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNServiceController.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNServiceController.m
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNSessionListener.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNSessionListenerImpl.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNSessionListenerImpl.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNSessionOptions.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNSessionOptions.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNSessionPortListener.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNSessionPortListenerImpl.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNSessionPortListenerImpl.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNSignalHandler.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNSignalHandlerImpl.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNSignalHandlerImpl.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNStatus.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNStatus.m
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNTransportMask.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNTransportMask.m
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNType.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNVersion.h
alljoyn_objc/AllJoynFramework/AllJoynFramework/AJNVersion.mm
alljoyn_objc/AllJoynFramework/AllJoynFramework/AllJoynFramework-Prefix.pch
alljoyn_objc/AllJoynFramework/AllJoynFramework/SConscript
alljoyn_objc/AllJoynFramework/AllJoynFrameworkTests/AJNBasicObject.h
alljoyn_objc/AllJoynFramework/AllJoynFrameworkTests/AJNBasicObject.mm
alljoyn_objc/AllJoynFramework/AllJoynFrameworkTests/AllJoynFrameworkTests-Info.plist
alljoyn_objc/AllJoynFramework/AllJoynFrameworkTests/AuthenticationTests.h
alljoyn_objc/AllJoynFramework/AllJoynFrameworkTests/AuthenticationTests.m
alljoyn_objc/AllJoynFramework/AllJoynFrameworkTests/BasicObject.h
alljoyn_objc/AllJoynFramework/AllJoynFrameworkTests/BasicObject.m
alljoyn_objc/AllJoynFramework/AllJoynFrameworkTests/BusAttachmentTests.h
alljoyn_objc/AllJoynFramework/AllJoynFrameworkTests/BusAttachmentTests.m
alljoyn_objc/AllJoynFramework/AllJoynFrameworkTests/BusObjectTests.h
alljoyn_objc/AllJoynFramework/AllJoynFrameworkTests/BusObjectTests.m
alljoyn_objc/AllJoynFramework/AllJoynFrameworkTests/InterfaceDescriptionTests.h
alljoyn_objc/AllJoynFramework/AllJoynFrameworkTests/InterfaceDescriptionTests.m
alljoyn_objc/AllJoynFramework/AllJoynFrameworkTests/TestAuthenticationListener.h
alljoyn_objc/AllJoynFramework/AllJoynFrameworkTests/TestAuthenticationListener.m
alljoyn_objc/AllJoynFramework/AllJoynFrameworkTests/en.lproj/InfoPlist.strings
alljoyn_objc/AllJoynFramework_iOS/AllJoynFramework_iOS.xcodeproj/project.pbxproj
alljoyn_objc/AllJoynFramework_iOS/AllJoynFramework_iOS.xcodeproj/project.xcworkspace/contents.xcworkspacedata
alljoyn_objc/AllJoynFramework_iOS/AllJoynFramework_iOS.xcodeproj/project.xcworkspace/xcuserdata/seabuild.xcuserdatad/UserInterfaceState.xcuserstate
alljoyn_objc/AllJoynFramework_iOS/AllJoynFramework_iOS.xcodeproj/xcshareddata/xcschemes/API Documentation.xcscheme
alljoyn_objc/AllJoynFramework_iOS/AllJoynFramework_iOS.xcodeproj/xcshareddata/xcschemes/AllJoynFramework_iOS.xcscheme
alljoyn_objc/AllJoynFramework_iOS/AllJoynFramework_iOS.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/xcschememanagement.plist
alljoyn_objc/AllJoynFramework_iOS/AllJoynFramework_iOS.xcodeproj/xcuserdata/seabuild.xcuserdatad/xcschemes/API Documentation.xcscheme
alljoyn_objc/AllJoynFramework_iOS/AllJoynFramework_iOS.xcodeproj/xcuserdata/seabuild.xcuserdatad/xcschemes/AllJoynFramework_iOS.xcscheme
alljoyn_objc/AllJoynFramework_iOS/AllJoynFramework_iOS.xcodeproj/xcuserdata/seabuild.xcuserdatad/xcschemes/xcschememanagement.plist
alljoyn_objc/AllJoynFramework_iOS/AllJoynFramework_iOS/AllJoynFramework_iOS-Prefix.pch
alljoyn_objc/AllJoynFramework_iOS/AllJoynFramework_iOSTests/AllJoynFramework_iOSTests-Info.plist
alljoyn_objc/AllJoynFramework_iOS/AllJoynFramework_iOSTests/en.lproj/InfoPlist.strings
alljoyn_objc/README-INSTALLING.txt
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient.xcodeproj/project.pbxproj
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient.xcodeproj/project.xcworkspace/contents.xcworkspacedata
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/AJNPingObject.h
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/AJNPingObject.mm
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/AppDelegate.h
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/AppDelegate.m
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/BasicBusClient-Info.plist
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/BasicBusClient-Prefix.pch
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/Constants.h
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/Constants.m
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/PingClient.h
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/PingClient.m
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/PingObject.h
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/PingObject.m
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/PingObjectModel.xml
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/ViewController.h
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/ViewController.m
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/en.lproj/InfoPlist.strings
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/en.lproj/MainMenu.xib
alljoyn_objc/Test/OSX/BasicBusClient/BasicBusClient/main.m
alljoyn_objc/Test/OSX/BasicBusService/BasicBusService.xcodeproj/project.pbxproj
alljoyn_objc/Test/OSX/BasicBusService/BasicBusService.xcodeproj/project.xcworkspace/contents.xcworkspacedata
alljoyn_objc/Test/OSX/BasicBusService/BasicBusService/AppDelegate.h
alljoyn_objc/Test/OSX/BasicBusService/BasicBusService/AppDelegate.m
alljoyn_objc/Test/OSX/BasicBusService/BasicBusService/BasicBusService-Info.plist
alljoyn_objc/Test/OSX/BasicBusService/BasicBusService/BasicBusService-Prefix.pch
alljoyn_objc/Test/OSX/BasicBusService/BasicBusService/PingService.h
alljoyn_objc/Test/OSX/BasicBusService/BasicBusService/PingService.m
alljoyn_objc/Test/OSX/BasicBusService/BasicBusService/ViewController.h
alljoyn_objc/Test/OSX/BasicBusService/BasicBusService/ViewController.m
alljoyn_objc/Test/OSX/BasicBusService/BasicBusService/en.lproj/InfoPlist.strings
alljoyn_objc/Test/OSX/BasicBusService/BasicBusService/en.lproj/MainMenu.xib
alljoyn_objc/Test/OSX/BasicBusService/BasicBusService/main.m
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest.xcodeproj/project.pbxproj
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/AJNPerformanceObject.h
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/AJNPerformanceObject.mm
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/AllJoynPerfTest-Info.plist
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/AllJoynPerfTest-Prefix.pch
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/AllJoynPerfTestObjectModel.xml
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/AppDelegate.h
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/AppDelegate.m
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/Constants.h
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/Constants.m
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/Default-568h@2x.png
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/Default.png
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/Default@2x.png
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/PerformanceObject.h
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/PerformanceObject.m
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/PerformanceStatistics.h
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/PerformanceStatistics.m
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/ViewController.h
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/ViewController.m
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/en.lproj/InfoPlist.strings
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/en.lproj/MainStoryboard_iPad.storyboard
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/en.lproj/MainStoryboard_iPhone.storyboard
alljoyn_objc/Test/iOS/AllJoynPerfTest/AllJoynPerfTest/main.m
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient.xcodeproj/project.pbxproj
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient.xcodeproj/project.xcworkspace/contents.xcworkspacedata
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient.xcodeproj/xcshareddata/xcschemes/BasicBusClient.xcscheme
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient.xcodeproj/xcshareddata/xcschemes/Generate Code.xcscheme
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/AJNPingObject.h
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/AJNPingObject.mm
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/AppDelegate.h
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/AppDelegate.m
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/BasicBusClient-Info.plist
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/BasicBusClient-Prefix.pch
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/Constants.h
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/Constants.m
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/Default-568h@2x.png
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/Default.png
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/Default@2x.png
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/PingClient.h
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/PingClient.m
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/PingObject.h
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/PingObject.m
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/PingObjectModel.xml
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/ViewController.h
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/ViewController.m
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/en.lproj/InfoPlist.strings
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/en.lproj/MainStoryboard_iPad.storyboard
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/en.lproj/MainStoryboard_iPhone.storyboard
alljoyn_objc/Test/iOS/BasicBusClient/BasicBusClient/main.m
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService.xcodeproj/project.pbxproj
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService.xcodeproj/project.xcworkspace/contents.xcworkspacedata
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService.xcodeproj/xcshareddata/xcschemes/BasicBusService.xcscheme
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService/AppDelegate.h
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService/AppDelegate.m
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService/BasicBusService-Info.plist
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService/BasicBusService-Prefix.pch
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService/Default-568h@2x.png
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService/Default.png
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService/Default@2x.png
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService/PingService.h
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService/PingService.m
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService/ViewController.h
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService/ViewController.m
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService/en.lproj/InfoPlist.strings
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService/en.lproj/MainStoryboard_iPad.storyboard
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService/en.lproj/MainStoryboard_iPhone.storyboard
alljoyn_objc/Test/iOS/BasicBusService/BasicBusService/main.m
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress.xcodeproj/project.pbxproj
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress.xcodeproj/project.xcworkspace/contents.xcworkspacedata
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress/AJNBasicObject.h
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress/AJNBasicObject.mm
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress/AppDelegate.h
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress/AppDelegate.m
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress/BasicObject.h
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress/BasicObject.m
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress/Bus Stress-Info.plist
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress/Bus Stress-Prefix.pch
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress/BusStressManager.h
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress/BusStressManager.mm
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress/BusStressViewController.h
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress/BusStressViewController.m
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress/en.lproj/InfoPlist.strings
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress/en.lproj/MainStoryboard_iPad.storyboard
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress/en.lproj/MainStoryboard_iPhone.storyboard
alljoyn_objc/Test/iOS/Bus Stress/Bus Stress/main.m
alljoyn_objc/Test/iOS/Bus Stress/Default-568h@2x.png
alljoyn_objc/Unity/AllJoynUnityBundle/AllJoynUnityBundle.xcodeproj/project.pbxproj
alljoyn_objc/Unity/AllJoynUnityBundle/AllJoynUnityBundle.xcodeproj/project.xcworkspace/contents.xcworkspacedata
alljoyn_objc/Unity/AllJoynUnityBundle/AllJoynUnityBundle/AllJoynUnityBundle-Info.plist
alljoyn_objc/Unity/AllJoynUnityBundle/AllJoynUnityBundle/AllJoynUnityBundle-Prefix.pch
alljoyn_objc/Unity/AllJoynUnityBundle/AllJoynUnityBundle/en.lproj/InfoPlist.strings
alljoyn_objc/alljoyn_darwin.xcodeproj/project.pbxproj
alljoyn_objc/alljoyn_darwin.xcodeproj/project.xcworkspace/contents.xcworkspacedata
alljoyn_objc/alljoyn_darwin.xcodeproj/xcshareddata/xcschemes/alljoyn_c_ios.xcscheme
alljoyn_objc/alljoyn_darwin.xcodeproj/xcshareddata/xcschemes/alljoyn_c_osx.xcscheme
alljoyn_objc/alljoyn_darwin.xcodeproj/xcshareddata/xcschemes/alljoyn_core_ios.xcscheme
alljoyn_objc/alljoyn_darwin.xcodeproj/xcshareddata/xcschemes/alljoyn_core_ios_armv7.xcscheme
alljoyn_objc/alljoyn_darwin.xcodeproj/xcshareddata/xcschemes/alljoyn_core_ios_armv7s.xcscheme
alljoyn_objc/alljoyn_darwin.xcodeproj/xcshareddata/xcschemes/alljoyn_core_osx.xcscheme
alljoyn_objc/alljoyn_darwin.xcodeproj/xcshareddata/xcschemes/alljoyn_streaming_ios.xcscheme
alljoyn_objc/alljoyn_darwin.xcodeproj/xcshareddata/xcschemes/alljoyn_streaming_osx.xcscheme
alljoyn_objc/alljoyn_darwin.xcodeproj/xcshareddata/xcschemes/alljoyn_unity_ios.xcscheme
alljoyn_objc/alljoyn_darwin.xcodeproj/xcshareddata/xcschemes/alljoyn_unity_osx.xcscheme
alljoyn_objc/alljoyn_darwin.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/bastress.xcscheme
alljoyn_objc/alljoyn_darwin.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/bastress2.xcscheme
alljoyn_objc/alljoyn_darwin.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/bbclient.xcscheme
alljoyn_objc/alljoyn_darwin.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/bbservice.xcscheme
alljoyn_objc/alljoyn_darwin.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/xcschememanagement.plist
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat.xcodeproj/project.pbxproj
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AJNCBusObject.h
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AJNCBusObject.mm
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AJNCBusObjectImpl.h
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AJNCBusObjectImpl.mm
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AJNCChatObjectSignalHandler.h
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AJNCChatObjectSignalHandler.mm
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AJNCChatObjectSignalHandlerImpl.h
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AJNCChatObjectSignalHandlerImpl.mm
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AJNCChatReceiver.h
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AJNCConstants.h
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AJNCConstants.m
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AJNCConversation.h
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AJNCConversation.m
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AJNCMessage.h
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AJNCMessage.m
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AllJoynChat-Info.plist
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AllJoynChat-Prefix.pch
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AppDelegate.h
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/AppDelegate.m
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/ChatViewController.h
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/ChatViewController.m
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/en.lproj/InfoPlist.strings
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/en.lproj/MainMenu.xib
alljoyn_objc/samples/OSX/AllJoynChat/AllJoynChat/main.m
alljoyn_objc/samples/OSX/basic_client/basic_client.xcodeproj/project.pbxproj
alljoyn_objc/samples/OSX/basic_client/basic_client.xcodeproj/project.xcworkspace/contents.xcworkspacedata
alljoyn_objc/samples/OSX/basic_client/basic_client.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/AllJoynLib.xcscheme
alljoyn_objc/samples/OSX/basic_client/basic_client.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/basic_client.xcscheme
alljoyn_objc/samples/OSX/basic_client/basic_client.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/xcschememanagement.plist
alljoyn_objc/samples/OSX/basic_client/basic_client/AppDelegate.h
alljoyn_objc/samples/OSX/basic_client/basic_client/AppDelegate.m
alljoyn_objc/samples/OSX/basic_client/basic_client/BasicClient.h
alljoyn_objc/samples/OSX/basic_client/basic_client/BasicClient.mm
alljoyn_objc/samples/OSX/basic_client/basic_client/BasicClientViewController.h
alljoyn_objc/samples/OSX/basic_client/basic_client/BasicClientViewController.m
alljoyn_objc/samples/OSX/basic_client/basic_client/SConscript
alljoyn_objc/samples/OSX/basic_client/basic_client/basic_client-Info.plist
alljoyn_objc/samples/OSX/basic_client/basic_client/basic_client-Prefix.pch
alljoyn_objc/samples/OSX/basic_client/basic_client/en.lproj/Credits.rtf
alljoyn_objc/samples/OSX/basic_client/basic_client/en.lproj/InfoPlist.strings
alljoyn_objc/samples/OSX/basic_client/basic_client/en.lproj/MainMenu.xib
alljoyn_objc/samples/OSX/basic_client/basic_client/main.m
alljoyn_objc/samples/OSX/basic_client/basic_clientTests/basic_clientTests-Info.plist
alljoyn_objc/samples/OSX/basic_client/basic_clientTests/basic_clientTests.h
alljoyn_objc/samples/OSX/basic_client/basic_clientTests/basic_clientTests.m
alljoyn_objc/samples/OSX/basic_client/basic_clientTests/en.lproj/InfoPlist.strings
alljoyn_objc/samples/OSX/basic_service/basic_service.xcodeproj/project.pbxproj
alljoyn_objc/samples/OSX/basic_service/basic_service.xcodeproj/project.xcworkspace/contents.xcworkspacedata
alljoyn_objc/samples/OSX/basic_service/basic_service.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/AllJoynLib.xcscheme
alljoyn_objc/samples/OSX/basic_service/basic_service.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/basic_service.xcscheme
alljoyn_objc/samples/OSX/basic_service/basic_service.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/xcschememanagement.plist
alljoyn_objc/samples/OSX/basic_service/basic_service/AppDelegate.h
alljoyn_objc/samples/OSX/basic_service/basic_service/AppDelegate.m
alljoyn_objc/samples/OSX/basic_service/basic_service/BasicSampleObject.h
alljoyn_objc/samples/OSX/basic_service/basic_service/BasicSampleObject.mm
alljoyn_objc/samples/OSX/basic_service/basic_service/BasicSampleObjectImpl.h
alljoyn_objc/samples/OSX/basic_service/basic_service/BasicSampleObjectImpl.mm
alljoyn_objc/samples/OSX/basic_service/basic_service/BasicService.h
alljoyn_objc/samples/OSX/basic_service/basic_service/BasicService.mm
alljoyn_objc/samples/OSX/basic_service/basic_service/BasicServiceViewController.h
alljoyn_objc/samples/OSX/basic_service/basic_service/BasicServiceViewController.m
alljoyn_objc/samples/OSX/basic_service/basic_service/MyBasicSampleObject+AllJoyn.h
alljoyn_objc/samples/OSX/basic_service/basic_service/MyBasicSampleObject+AllJoyn.m
alljoyn_objc/samples/OSX/basic_service/basic_service/SConscript
alljoyn_objc/samples/OSX/basic_service/basic_service/basic_service-Info.plist
alljoyn_objc/samples/OSX/basic_service/basic_service/basic_service-Prefix.pch
alljoyn_objc/samples/OSX/basic_service/basic_service/en.lproj/Credits.rtf
alljoyn_objc/samples/OSX/basic_service/basic_service/en.lproj/InfoPlist.strings
alljoyn_objc/samples/OSX/basic_service/basic_service/en.lproj/MainMenu.xib
alljoyn_objc/samples/OSX/basic_service/basic_service/main.m
alljoyn_objc/samples/OSX/basic_service/basic_serviceTests/basic_serviceTests-Info.plist
alljoyn_objc/samples/OSX/basic_service/basic_serviceTests/basic_serviceTests.h
alljoyn_objc/samples/OSX/basic_service/basic_serviceTests/basic_serviceTests.m
alljoyn_objc/samples/OSX/basic_service/basic_serviceTests/en.lproj/InfoPlist.strings
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template.xcodeproj/project.pbxproj
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template.xcodeproj/project.xcworkspace/contents.xcworkspacedata
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/AllJoyn iOS Project Template.xcscheme
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/xcschememanagement.plist
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/AJNSampleObject.h
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/AJNSampleObject.mm
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/AllJoyn iOS Project Template-Info.plist
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/AllJoyn iOS Project Template-Prefix.pch
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/AppDelegate.h
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/AppDelegate.m
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/Constants.h
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/Constants.m
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/README.txt
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/SampleAllJoynObjectModel.xml
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/SampleClient.h
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/SampleClient.m
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/SampleObject.h
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/SampleObject.m
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/SampleService.h
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/SampleService.m
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/en.lproj/InfoPlist.strings
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project Template/main.m
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project TemplateTests/AllJoyn iOS Project TemplateTests-Info.plist
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project TemplateTests/AllJoyn_iOS_Project_TemplateTests.h
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project TemplateTests/AllJoyn_iOS_Project_TemplateTests.m
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/AllJoyn iOS Project TemplateTests/en.lproj/InfoPlist.strings
alljoyn_objc/samples/iOS/AllJoyn iOS Project Template/Default-568h@2x.png
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat.xcodeproj/project.pbxproj
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat.xcodeproj/project.xcworkspace/contents.xcworkspacedata
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat.xcodeproj/xcshareddata/xcschemes/AllJoynChat.xcscheme
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat.xcodeproj/xcuserdata/guest.xcuserdatad/xcschemes/xcschememanagement.plist
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCAppDelegate.h
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCAppDelegate.m
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCBusObject.h
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCBusObject.mm
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCBusObjectImpl.h
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCBusObjectImpl.mm
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCChatObjectSignalHandler.h
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCChatObjectSignalHandler.mm
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCChatObjectSignalHandlerImpl.h
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCChatObjectSignalHandlerImpl.mm
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCChatReceiver.h
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCConstants.h
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCConstants.m
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCConversation.h
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCConversation.m
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCMessage.h
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCMessage.m
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCStartPageViewController.h
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCStartPageViewController.m
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCViewController.h
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AJNCViewController.m
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AllJoynChat-Info.plist
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/AllJoynChat-Prefix.pch
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/SConscript
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/en.lproj/InfoPlist.strings
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/en.lproj/MainStoryboard_iPhone.storyboard
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/icon-for-app-store-retina.png
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/icon-for-app-store.png
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/icon.png
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/launch-image.png
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/main.m
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChat/search-results.png
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChatTests/AllJoynChatTests-Info.plist
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChatTests/AllJoynChatTests.h
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChatTests/AllJoynChatTests.m
alljoyn_objc/samples/iOS/AllJoynChat/AllJoynChatTests/en.lproj/InfoPlist.strings
alljoyn_objc/samples/iOS/AllJoynChat/Default-568h@2x.png
alljoyn_objc/samples/iOS/AllJoynChat/Default.png
alljoyn_objc/samples/iOS/BasicClient/BasicClient.xcodeproj/project.pbxproj
alljoyn_objc/samples/iOS/BasicClient/BasicClient/AJNBasicObject.h
alljoyn_objc/samples/iOS/BasicClient/BasicClient/AJNBasicObject.mm
alljoyn_objc/samples/iOS/BasicClient/BasicClient/AppDelegate.h
alljoyn_objc/samples/iOS/BasicClient/BasicClient/AppDelegate.m
alljoyn_objc/samples/iOS/BasicClient/BasicClient/BasicClient-Info.plist
alljoyn_objc/samples/iOS/BasicClient/BasicClient/BasicClient-Prefix.pch
alljoyn_objc/samples/iOS/BasicClient/BasicClient/BasicClient.h
alljoyn_objc/samples/iOS/BasicClient/BasicClient/BasicClient.m
alljoyn_objc/samples/iOS/BasicClient/BasicClient/BasicObject.h
alljoyn_objc/samples/iOS/BasicClient/BasicClient/BasicObject.m
alljoyn_objc/samples/iOS/BasicClient/BasicClient/Default-568h@2x.png
alljoyn_objc/samples/iOS/BasicClient/BasicClient/Default.png
alljoyn_objc/samples/iOS/BasicClient/BasicClient/Default@2x.png
alljoyn_objc/samples/iOS/BasicClient/BasicClient/ViewController.h
alljoyn_objc/samples/iOS/BasicClient/BasicClient/ViewController.m
alljoyn_objc/samples/iOS/BasicClient/BasicClient/en.lproj/InfoPlist.strings
alljoyn_objc/samples/iOS/BasicClient/BasicClient/en.lproj/MainStoryboard_iPad.storyboard
alljoyn_objc/samples/iOS/BasicClient/BasicClient/en.lproj/MainStoryboard_iPhone.storyboard
alljoyn_objc/samples/iOS/BasicClient/BasicClient/main.m
alljoyn_objc/samples/iOS/BasicService/BasicService.xcodeproj/project.pbxproj
alljoyn_objc/samples/iOS/BasicService/BasicService/AppDelegate.h
alljoyn_objc/samples/iOS/BasicService/BasicService/AppDelegate.m
alljoyn_objc/samples/iOS/BasicService/BasicService/BasicService-Info.plist
alljoyn_objc/samples/iOS/BasicService/BasicService/BasicService-Prefix.pch
alljoyn_objc/samples/iOS/BasicService/BasicService/BasicService.h
alljoyn_objc/samples/iOS/BasicService/BasicService/BasicService.m
alljoyn_objc/samples/iOS/BasicService/BasicService/Default-568h@2x.png
alljoyn_objc/samples/iOS/BasicService/BasicService/Default.png
alljoyn_objc/samples/iOS/BasicService/BasicService/Default@2x.png
alljoyn_objc/samples/iOS/BasicService/BasicService/ViewController.h
alljoyn_objc/samples/iOS/BasicService/BasicService/ViewController.m
alljoyn_objc/samples/iOS/BasicService/BasicService/en.lproj/InfoPlist.strings
alljoyn_objc/samples/iOS/BasicService/BasicService/en.lproj/MainStoryboard_iPad.storyboard
alljoyn_objc/samples/iOS/BasicService/BasicService/en.lproj/MainStoryboard_iPhone.storyboard
alljoyn_objc/samples/iOS/BasicService/BasicService/main.m
alljoyn_objc/samples/iOS/FileTransferClient/FileTransferClient.xcodeproj/project.pbxproj
alljoyn_objc/samples/iOS/FileTransferClient/FileTransferClient/AppDelegate.h
alljoyn_objc/samples/iOS/FileTransferClient/FileTransferClient/AppDelegate.m
alljoyn_objc/samples/iOS/FileTransferClient/FileTransferClient/Default-568h@2x.png
alljoyn_objc/samples/iOS/FileTransferClient/FileTransferClient/Default.png
alljoyn_objc/samples/iOS/FileTransferClient/FileTransferClient/Default@2x.png
alljoyn_objc/samples/iOS/FileTransferClient/FileTransferClient/FileTransferClient-Info.plist
alljoyn_objc/samples/iOS/FileTransferClient/FileTransferClient/FileTransferClient-Prefix.pch
alljoyn_objc/samples/iOS/FileTransferClient/FileTransferClient/ViewController.h
alljoyn_objc/samples/iOS/FileTransferClient/FileTransferClient/ViewController.mm
alljoyn_objc/samples/iOS/FileTransferClient/FileTransferClient/en.lproj/InfoPlist.strings
alljoyn_objc/samples/iOS/FileTransferClient/FileTransferClient/en.lproj/MainStoryboard_iPad.storyboard
alljoyn_objc/samples/iOS/FileTransferClient/FileTransferClient/en.lproj/MainStoryboard_iPhone.storyboard
alljoyn_objc/samples/iOS/FileTransferClient/FileTransferClient/main.m
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer.xcodeproj/project.pbxproj
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/AJNFileTransferObject.h
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/AJNFileTransferObject.mm
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/AppDelegate.h
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/AppDelegate.m
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/Constants.h
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/Constants.m
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/Default-568h@2x.png
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/Default.png
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/Default@2x.png
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/FileTransferObject.h
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/FileTransferObject.m
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/FileTransferObjectModel.xml
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/FileTransferServer-Info.plist
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/FileTransferServer-Prefix.pch
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/ViewController.h
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/ViewController.mm
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/en.lproj/InfoPlist.strings
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/en.lproj/MainStoryboard_iPad.storyboard
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/en.lproj/MainStoryboard_iPhone.storyboard
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/image.png
alljoyn_objc/samples/iOS/FileTransferServer/FileTransferServer/main.m
alljoyn_objc/samples/iOS/SecureClient/SecureClient.xcodeproj/project.pbxproj
alljoyn_objc/samples/iOS/SecureClient/SecureClient/AJNSecureObject.h
alljoyn_objc/samples/iOS/SecureClient/SecureClient/AJNSecureObject.mm
alljoyn_objc/samples/iOS/SecureClient/SecureClient/AppDelegate.h
alljoyn_objc/samples/iOS/SecureClient/SecureClient/AppDelegate.m
alljoyn_objc/samples/iOS/SecureClient/SecureClient/AuthenticationTableViewController.h
alljoyn_objc/samples/iOS/SecureClient/SecureClient/AuthenticationTableViewController.m
alljoyn_objc/samples/iOS/SecureClient/SecureClient/Constants.h
alljoyn_objc/samples/iOS/SecureClient/SecureClient/Constants.m
alljoyn_objc/samples/iOS/SecureClient/SecureClient/Default-568h@2x.png
alljoyn_objc/samples/iOS/SecureClient/SecureClient/Default.png
alljoyn_objc/samples/iOS/SecureClient/SecureClient/Default@2x.png
alljoyn_objc/samples/iOS/SecureClient/SecureClient/SecureClient-Info.plist
alljoyn_objc/samples/iOS/SecureClient/SecureClient/SecureClient-Prefix.pch
alljoyn_objc/samples/iOS/SecureClient/SecureClient/SecureObject.h
alljoyn_objc/samples/iOS/SecureClient/SecureClient/SecureObject.m
alljoyn_objc/samples/iOS/SecureClient/SecureClient/SecureObjectModel.xml
alljoyn_objc/samples/iOS/SecureClient/SecureClient/ViewController.h
alljoyn_objc/samples/iOS/SecureClient/SecureClient/ViewController.m
alljoyn_objc/samples/iOS/SecureClient/SecureClient/en.lproj/InfoPlist.strings
alljoyn_objc/samples/iOS/SecureClient/SecureClient/en.lproj/MainStoryboard_iPad.storyboard
alljoyn_objc/samples/iOS/SecureClient/SecureClient/en.lproj/MainStoryboard_iPhone.storyboard
alljoyn_objc/samples/iOS/SecureClient/SecureClient/main.m
alljoyn_objc/samples/iOS/SecureService/SecureService.xcodeproj/project.pbxproj
alljoyn_objc/samples/iOS/SecureService/SecureService/AppDelegate.h
alljoyn_objc/samples/iOS/SecureService/SecureService/AppDelegate.m
alljoyn_objc/samples/iOS/SecureService/SecureService/AuthenticationTableViewController.h
alljoyn_objc/samples/iOS/SecureService/SecureService/AuthenticationTableViewController.m
alljoyn_objc/samples/iOS/SecureService/SecureService/Constants.h
alljoyn_objc/samples/iOS/SecureService/SecureService/Constants.m
alljoyn_objc/samples/iOS/SecureService/SecureService/Default-568h@2x.png
alljoyn_objc/samples/iOS/SecureService/SecureService/Default.png
alljoyn_objc/samples/iOS/SecureService/SecureService/Default@2x.png
alljoyn_objc/samples/iOS/SecureService/SecureService/SecureService-Info.plist
alljoyn_objc/samples/iOS/SecureService/SecureService/SecureService-Prefix.pch
alljoyn_objc/samples/iOS/SecureService/SecureService/ViewController.h
alljoyn_objc/samples/iOS/SecureService/SecureService/ViewController.m
alljoyn_objc/samples/iOS/SecureService/SecureService/en.lproj/InfoPlist.strings
alljoyn_objc/samples/iOS/SecureService/SecureService/en.lproj/MainStoryboard_iPad.storyboard
alljoyn_objc/samples/iOS/SecureService/SecureService/en.lproj/MainStoryboard_iPhone.storyboard
alljoyn_objc/samples/iOS/SecureService/SecureService/main.m
## DELETE ME ##
