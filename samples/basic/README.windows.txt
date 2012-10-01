The following are samples of a basic service and client that will use the
already existing AllJoyn Service to transfer messages.
    +basic_service.cc
    +basic_client.cc

basic_service.cc:
    Will create a service that will join the bus and request the name
    'org.alljoyn.Bus.sample' it will create a single method called 'cat'
    that will concatenate two strings together and return the resulting
    string to the client.

    Example output:
    basic_service
        AllJoyn Library version: v2.5.0
        AllJoyn Library build info: Alljoyn Library v2.5.0 (Built Tue Aug 28 20:24:31 UTC 2012 on Win8-dev-pc by User)
        Interface Created.
        BusAttachement started.
        Initialized winsock
        Using BundledDaemon
        AllJoyn Daemon GUID = 3458b6ee5418a5d1d239b1113992f6ee (qOskKObH)
        ObjectRegistered has been called
        Connected to 'tcp:addr=127.0.0.1,port=9956'
        NameOwnerChanged: name=org.alljoyn.Bus.sample, oldOwner=<none>, newOwner=:qOskKObH.2
        Accepting join session request from :PlGGboNn.2 (opts.proximity=ff, opts.traffic=1, opts.transports=4)

basic_client.cc:
    Will connect to the bus discover the interface of 'org.alljoyn.Bus.sample'
    then it will make a call to the 'cat' method.

    Example output (there may be addition spurious error messages which are to
    be expected when the program terminates):
    basic_client
        AllJoyn Library version: v2.5.0
        AllJoyn Library build info: Alljoyn Library v2.5.0 (Built Wed Sep 26 20:12:23 UTC 2012 on Win8-dev-pc by User)
        Interface 'org.alljoyn.Bus.sample' created.
        BusAttachment started.
        Initialized winsock
        Using BundledDaemon
        AllJoyn Daemon GUID = c2545980a0129dd8910fd182256b1573 (2KP0WITO)
        BusAttachment connected to 'tcp:addr=127.0.0.1,port=9956'.
        BusListener Registered.
        org.alljoyn.Bus.FindAdvertisedName ('org.alljoyn.Bus.sample') succeeded.
        Waited 0 seconds for JoinSession completion.
        FoundAdvertisedName(name='org.alljoyn.Bus.sample', prefix='org.alljoyn.Bus.sample')
        NameOwnerChanged: name='org.alljoyn.Bus.sample', oldOwner='<none>', newOwner=':2yIp9ucd.2'.
        JoinSession SUCCESS (Session id=1292839962).
        'org.alljoyn.Bus.sample.cat' (path='/sample') returned 'Hello World!'.
        Basic client exiting with status 0x0000 (ER_OK).

The following show an example of how to emit and subscribe to an AllJoyn signal.
    +signal_service.cc
    +signalConsumer_client.cc
    +nameChange_client.cc
    
signal_service.cc:
    Will create a service that will join the bus and request the name
    'org.alljoyn.signal_sample'. This service will have a property 'name' the
    name can be changed using Get and Set methods. When the value for 'name'
    is changed a 'nameChanged' signal will be sent with the new name assigned
    to the 'name' property.
    
signalConsumer_client.cc:
    Will connect to the bus and subscribe to the 'nameChanged' signal. When the
    name has been changed it will print out a message indicating it has received
    the signal and indicate the new name.
     
nameChange_client.cc:
    Will accept a text string as one of its command line arguments. The string
    will be sent to the 'org.alljoyn.signal_sample' object and will be used as
    the new 'name' property using the 'set' method. 
    

    Example output (three different command prompts):
    signal_service
        AllJoyn Library version: v2.5.0.
        AllJoyn Library build info: Alljoyn Library v2.5.0 (Built Wed Sep 26 20:12:23 UTC 2012 on Win8-dev-pc by User).
        BusAttachment started.
        Registering the bus object.
        Initialized winsock
        Using BundledDaemon
        AllJoyn Daemon GUID = 77f710feaa91e3476326302f40a260c1 (ttG-gHZ7)
        Connected to 'tcp:addr=127.0.0.1,port=9956'.
        RequestName('org.alljoyn.Bus.signal_sample') succeeded.
        NameOwnerChanged: name=org.alljoyn.Bus.signal_sample, oldOwner=<none>, newOwner=:ttG-gHZ7.2
        BindSessionPort succeeded.
        Advertisement of the service name 'org.alljoyn.Bus.signal_sample' succeeded.
        Accepting join session request from :i2nHGQsU.2 (opts.proximity=ff, opts.traffic=1, opts.transports=4)
        Accepting join session request from :gjLyvJiQ.2 (opts.proximity=ff, opts.traffic=1, opts.transports=4)
        Set 'name' property was called changing name to Hello World
        Emiting Name Changed Signal.
        Sending NameChanged signal without a session id
        Accepting join session request from :oeiSmRty.2 (opts.proximity=ff, opts.traffic=1, opts.transports=4)
        Set 'name' property was called changing name to foo
        Emiting Name Changed Signal.
        Sending NameChanged signal without a session id
        Accepting join session request from :JDr9UZh-.2 (opts.proximity=ff, opts.traffic=1, opts.transports=4)
        Set 'name' property was called changing name to bar
        Emiting Name Changed Signal.
        Sending NameChanged signal without a session id
        Signal service exiting with status 0x0000 (ER_OK).

    signalConsumer_client
        AllJoyn Library version: v2.5.0.
        AllJoyn Library build info: Alljoyn Library v2.5.0 (Built Wed Sep 26 20:12:23 UTC 2012 on Win8-dev-pc by User).
        BusAttachment started.
        Interface created successfully.
        Interface successfully added to the bus.
        Registered signal handler for org.alljoyn.Bus.signal_sample.nameChanged.
        Registering the bus object.
        Initialized winsock
        Using BundledDaemon
        AllJoyn Daemon GUID = 2cc27111909af6de9aaa68232a2dce4c (i2nHGQsU)
        Connected to 'tcp:addr=127.0.0.1,port=9956'.
        BusListener registered.
        org.alljoyn.Bus.FindAdvertisedName ('org.alljoyn.Bus.signal_sample') succeeded.
        Waited 0 seconds for JoinSession completion.
        FoundAdvertisedName(name='org.alljoyn.Bus.signal_sample', prefix='org.alljoyn.Bus.signal_sample')
        NameOwnerChanged: name='org.alljoyn.Bus.signal_sample', oldOwner='<none>', newOwner=':ttG-gHZ7.2'.
        JoinSession SUCCESS (Session id=-1475224801).
        Successfully subscribed to the name changed signal.
        --==## signalConsumer: Name Changed signal Received ##==--
                New name: 'Hello World'.
        --==## signalConsumer: Name Changed signal Received ##==--
                New name: 'foo'.
        --==## signalConsumer: Name Changed signal Received ##==--
                New name: 'bar'.
        Signal consumer client exiting with status 0x0000 (ER_OK).

    nameChange_client "Hello World"
        AllJoyn Library version: v2.5.0.
        AllJoyn Library build info: Alljoyn Library v2.5.0 (Built Wed Sep 26 20:12:23 UTC 2012 on Win8-dev-pc by User).
        BusAttachment started.
        Initialized winsock
        Using BundledDaemon
        AllJoyn Daemon GUID = 6aedd53c3993ac1adab55ebfa613ded1 (gjLyvJiQ)
        BusAttachment connected to 'tcp:addr=127.0.0.1,port=9956'.
        BusListener registered.
        org.alljoyn.Bus.FindAdvertisedName ('org.alljoyn.Bus.signal_sample') succeeded.
        Waited 0 seconds for JoinSession completion.
        FoundAdvertisedName(name='org.alljoyn.Bus.signal_sample', prefix='org.alljoyn.Bus.signal_sample')
        NameOwnerChanged: name='org.alljoyn.Bus.signal_sample', oldOwner='<none>', newOwner=':ttG-gHZ7.2'.
        JoinSession SUCCESS (Session id=-798738377).
        SetProperty to change the 'name' property to 'Hello World' was successful.
        Name change client exiting with status 0x0000 (ER_OK).

    nameChange_client.exe foo
        AllJoyn Library version: v2.5.0.
        AllJoyn Library build info: Alljoyn Library v2.5.0 (Built Wed Sep 26 20:12:23 UTC 2012 on Win8-dev-pc by User).
        BusAttachment started.
        Initialized winsock
        Using BundledDaemon
        AllJoyn Daemon GUID = 7268acdcb05bf7fcb15f392373a14875 (oeiSmRty)
        BusAttachment connected to 'tcp:addr=127.0.0.1,port=9956'.
        BusListener registered.
        org.alljoyn.Bus.FindAdvertisedName ('org.alljoyn.Bus.signal_sample') succeeded.
        Waited 0 seconds for JoinSession completion.
        FoundAdvertisedName(name='org.alljoyn.Bus.signal_sample', prefix='org.alljoyn.Bus.signal_sample')
        NameOwnerChanged: name='org.alljoyn.Bus.signal_sample', oldOwner='<none>', newOwner=':ttG-gHZ7.2'.
        JoinSession SUCCESS (Session id=1283637305).
        SetProperty to change the 'name' property to 'foo' was successful.
        Name change client exiting with status 0x0000 (ER_OK).

    nameChange_client.exe bar
        AllJoyn Library version: v2.5.0.
        AllJoyn Library build info: Alljoyn Library v2.5.0 (Built Wed Sep 26 20:12:23 UTC 2012 on Win8-dev-pc by User).
        BusAttachment started.
        Initialized winsock
        Using BundledDaemon
        AllJoyn Daemon GUID = 13cdb5891ea3eb7e9d46f2bd0ebda6db (JDr9UZh-)
        BusAttachment connected to 'tcp:addr=127.0.0.1,port=9956'.
        BusListener registered.
        org.alljoyn.Bus.FindAdvertisedName ('org.alljoyn.Bus.signal_sample') succeeded.
        Waited 0 seconds for JoinSession completion.
        FoundAdvertisedName(name='org.alljoyn.Bus.signal_sample', prefix='org.alljoyn.Bus.signal_sample')
        NameOwnerChanged: name='org.alljoyn.Bus.signal_sample', oldOwner='<none>', newOwner=':ttG-gHZ7.2'.
        JoinSession SUCCESS (Session id=-2012919274).
        SetProperty to change the 'name' property to 'bar' was successful.
        Name change client exiting with status 0x0000 (ER_OK).
