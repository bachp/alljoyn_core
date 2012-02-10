package org.alljoyn.ice;


import java.util.Map;

import org.alljoyn.bus.BusException;
import org.alljoyn.bus.annotation.BusInterface;
import org.alljoyn.bus.annotation.BusMethod;
import org.alljoyn.bus.annotation.Position;
import org.alljoyn.bus.annotation.Signature;

@BusInterface(name = "org.alljoyn.ice.clientloginservice")
public interface ClientLoginServiceInterface {

@BusMethod(replySignature = "s")
public String GetClientAccountName();

@BusMethod(replySignature = "s")
public String GetClientAccountPassword();

}
