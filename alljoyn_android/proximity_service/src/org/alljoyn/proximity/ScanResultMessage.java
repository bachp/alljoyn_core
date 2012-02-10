package org.alljoyn.proximity;

import org.alljoyn.bus.annotation.Position;
import org.alljoyn.bus.annotation.Signature;

public class ScanResultMessage {

	@Position(0)
	@Signature("s")
	public String bssid;

	@Position(1)
	@Signature("s")
	public String ssid;
	
	@Position(2)
	@Signature("b")
	public boolean attached;
	
}
