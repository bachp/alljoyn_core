package org.alljoyn.proximity;

import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.Context;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;
import android.util.Log;


import java.util.ArrayList;
import java.util.List;

import org.alljoyn.proximity.ProximityService.ScanService;


// This guys needs access to the Map which the original ProximityService is going to send back to the native code


public class ScanResultsReceiver extends BroadcastReceiver{
	
	ScanService scanService;
	//
	// Need a constructor that will take the instance on Proximity Service
	//
	public ScanResultsReceiver(ScanService scanService){
		super();
		this.scanService = scanService;
	}
	
	@Override
	public void onReceive(Context c, Intent intent){
		
		if(intent.getAction().equals(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION)){
			
//			if(scanService.scanRequested){
//				Log.v("ScanResultsReceiver", "This is a result of USER REQUESTED SCAN");
//			}
//			
//			if(scanService.stopScanRequested){
//				return;
//			}
			
			Log.v("ScanResultsReceiver", "SCAN_RESULTS_AVAILABLE_ACTION received");
			
			List<ScanResult> scanResults = scanService.wifiMgr.getScanResults();
			//
			// If not access point were returned then we
			//
			if(scanResults.size() == 0){
				Log.v("ScanResultsReceiver", "Result size = 0");
				scanService.scanResultsObtained = true;
				return;
			}
			else{
				Log.v("ScanResultsReceiver", "Result size  = " + Integer.toString(scanResults.size()));
			}
			
			// If scan WAS requested we thrash all old results and just create and entirely new scan Results map
			// If scan WAS NOT requested we append the results obtained from this scan to our original result
			
			scanService.scanResultMessage = new ScanResultMessage[scanResults.size()];
						
			String currentBSSID = scanService.wifiMgr.getConnectionInfo().getBSSID();
			int currentBSSIDIndex = 0;
			
			for (ScanResult result : scanResults){

					scanService.scanResultMessage[currentBSSIDIndex] = new ScanResultMessage();
					scanService.scanResultMessage[currentBSSIDIndex].bssid = result.BSSID;
					scanService.scanResultMessage[currentBSSIDIndex].ssid = result.SSID;
					if(currentBSSID !=null && currentBSSID.equals(result.BSSID)){
						scanService.scanResultMessage[currentBSSIDIndex].attached = true;
					}
					currentBSSIDIndex++;
					
			}
			currentBSSIDIndex = 0;
			scanService.scanResultsObtained = true;
		}
	
	}

}
