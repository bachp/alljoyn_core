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

    Example output (there may be addition superious error messages which are to
    be expected when the program terminates):
    basic_client
        AllJoyn Library version: v2.5.0
        AllJoyn Library build info: Alljoyn Library v2.5.0 (Built Tue Aug 28 20:24:31 UTC 2012 on Win8-dev-pc by User)
        Interface Created.
        BusAttachment started.
        Initialized winsock
        Using BundledDaemon
        AllJoyn Daemon GUID = 192fd0d025f297b163285532dfcbdc31 (PlGGboNn)
        BusAttchement connected to tcp:addr=127.0.0.1,port=9956
        BusListener Registered.
        FoundAdvertisedName(name=org.alljoyn.Bus.sample, prefix=org.alljoyn.Bus.sample)
        NameOwnerChanged: name=org.alljoyn.Bus.sample, oldOwner=<none>, newOwner=:qOskKObH.2
        JoinSession SUCCESS (Session id=-766312187)
        org.alljoyn.Bus.sample.cat ( path=/sample) returned "Hello World!"

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
        AllJoyn Library version: v2.5.0
        Initialized winsock
        Using BundledDaemon
        AllJoyn Daemon GUID = 95f5619bf73838b7d36764a3611aff75 (LrXRtuut)
        Connected to 'tcp:addr=127.0.0.1,port=9956'
        NameOwnerChanged: name=org.alljoyn.Bus.signal_sample, oldOwner=<none>, newOwner=:LrXRtuut.2
        Accepting join session request from :iQl4xDWC.2 (opts.proximity=ff, opts.traffic=1, opts.transports=4)
        Accepting join session request from :v05mYILE.2 (opts.proximity=ff, opts.traffic=1, opts.transports=4)
        Set 'name' property was called changing name to Hello World
        Emiting Name Changed Signal.
        Sending NameChanged signal without a session id
        Accepting join session request from :jnO3VMWS.2 (opts.proximity=ff, opts.traffic=1, opts.transports=4)
        Set 'name' property was called changing name to foo
        Emiting Name Changed Signal.
        Sending NameChanged signal without a session id
        Accepting join session request from :Ajm4FcUp.2 (opts.proximity=ff, opts.traffic=1, opts.transports=4)
        Set 'name' property was called changing name to bar
        Emiting Name Changed Signal.
        Sending NameChanged signal without a session id

    signalConsumer_client
        AllJoyn Library version: v2.5.0
        BusAttachment started.
        Initialized winsock
        Using BundledDaemon
        AllJoyn Daemon GUID = ac1a6f443b0d60cc18762ea0c4193213 (iQl4xDWC)
        BusAttchement connected to tcp:addr=127.0.0.1,port=9956
        Interface created successfully.
        Interface successfully added to the bus.
        Registered signal handler for org.alljoyn.Bus.signal_sample.nameChanged
        BusListener Registered.
        FoundAdvertisedName(name=org.alljoyn.Bus.signal_sample, prefix=org.alljoyn.Bus.signal_sample)
        NameOwnerChanged: name=org.alljoyn.Bus.signal_sample, oldOwner=<none>, newOwner=:LrXRtuut.2
        JoinSession SUCCESS (Session id=20511552)
        Successfully Subscribed to the Name Changed Signal.
        --==## signalConsumer: Name Changed signal Received ##==--
                New name: Hello World
        --==## signalConsumer: Name Changed signal Received ##==--
                New name: foo
        --==## signalConsumer: Name Changed signal Received ##==--
                New name: bar

    nameChange_client "Hello World"
        AllJoyn Library version: v2.5.0
        BusAttachment started.
        Initialized winsock
        Using BundledDaemon
        AllJoyn Daemon GUID = b98005302292154e729f6e970bad28af (v05mYILE)
        BusAttchement connected to tcp:addr=127.0.0.1,port=9956
        BusListener Registered.
        FoundAdvertisedName(name=org.alljoyn.Bus.signal_sample, prefix=org.alljoyn.Bus.signal_sample)
        NameOwnerChanged: name=org.alljoyn.Bus.signal_sample, oldOwner=<none>, newOwner=:LrXRtuut.2
        JoinSession SUCCESS (Session id=-112718684)
        name Change client exiting with status 0 (ER_OK)
        
    nameChange_client.exe foo
        AllJoyn Library version: v2.5.0
        BusAttachment started.
        Initialized winsock
        Using BundledDaemon
        AllJoyn Daemon GUID = adb1d8c31fd6205c43e161d84a587ccc (jnO3VMWS)
        BusAttchement connected to tcp:addr=127.0.0.1,port=9956
        BusListener Registered.
        FoundAdvertisedName(name=org.alljoyn.Bus.signal_sample, prefix=org.alljoyn.Bus.signal_sample)
        NameOwnerChanged: name=org.alljoyn.Bus.signal_sample, oldOwner=<none>, newOwner=:LrXRtuut.2
        JoinSession SUCCESS (Session id=-246843947)
        name Change client exiting with status 0 (ER_OK)
