/*
 * Copyright 2012, Qualcomm Innovation Center, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

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
