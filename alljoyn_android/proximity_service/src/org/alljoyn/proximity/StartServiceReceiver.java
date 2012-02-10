package org.alljoyn.proximity;

import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.Context;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;
import android.util.Log;


import java.util.ArrayList;
import java.util.List;




// This guys needs access to the Map which the original ProximityService is going to send back to the native code


public class StartServiceReceiver extends BroadcastReceiver{

	@Override
	public void onReceive(Context context, Intent intent) {
		if(intent.getAction().equals(Intent.ACTION_BOOT_COMPLETED)){
			Intent startServiceIntent = new Intent();
			startServiceIntent.setAction("org.alljoyn.proximity.ProximityService");
			context.startService(startServiceIntent);
		}
	}
}
