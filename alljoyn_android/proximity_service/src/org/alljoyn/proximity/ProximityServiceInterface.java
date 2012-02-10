package org.alljoyn.proximity;


import java.util.Map;

import org.alljoyn.bus.BusException;
import org.alljoyn.bus.annotation.BusInterface;
import org.alljoyn.bus.annotation.BusMethod;
import org.alljoyn.bus.annotation.Position;
import org.alljoyn.bus.annotation.Signature;

@BusInterface(name = "org.alljoyn.proximity.proximityservice")
public interface ProximityServiceInterface {
	
@BusMethod(replySignature = "a(ssb)")
public ScanResultMessage[] Scan(boolean request_scan);

}
