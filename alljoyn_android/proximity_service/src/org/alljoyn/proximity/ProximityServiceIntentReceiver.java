package org.alljoyn.proximity;

import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.util.Log;

public class ProximityServiceIntentReceiver extends BroadcastReceiver{
	
	public static final String CUSTOM_INTENT = "org.alljoyn.proximity.ProximityServiceIntentReceiver.TEST";
	
	@Override
	public void onReceive(Context context, Intent intent) {
		
		Log.v("ALERTALERTALERT","What the hell is going on .............");
		// TODO Auto-generated method stub
		if(intent.getAction().equals(CUSTOM_INTENT)){
			Intent startServiceIntent = new Intent();
			startServiceIntent.setAction("org.alljoyn.proximity.ProximityService");
			context.startService(startServiceIntent);
		}
	}
}
