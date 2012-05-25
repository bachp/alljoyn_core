/**
 * @file
 */

/******************************************************************************
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
 ******************************************************************************/
package org.alljoyn.jni;

import android.app.Activity;
import android.os.Bundle;

import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;

import android.app.Application;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.util.Log;


public class AllJoynAndroidExt extends Service {
	 
	static {
	        System.loadLibrary("AllJoynAndroidExt");
	    }
   
	// Handler for all AllJoyn related things
	//private Handler mBusHandler;
	
	// Bus Object implementing the service
	//private ScanService mScanService;
	
	// For logging
	private static final String TAG = "AllJoynAndroidExt";
	
	// Moved here from bus handler
	
	private HandlerThread  busThread;
	
	BroadcastReceiver receiver;
	
	WifiManager wifiMgr;
	
	final int NOT_CONNECTED_TO_ANY_NW = -1;
	
//	boolean startScan = false;
	ScanResultMessage[] scanResultMessage;
	ScanResultMessage[] scanResultMessageToBeSent;
	boolean scanResultsObtained;
	boolean scanRequested = false;
	boolean stopScanRequested = false;

	// This needs to be accessible by the function which periodically populates results in the map
	boolean getScanResultsCallCompleted = false;
	
	public native int jniOnCreate(String packageName);

	public native void jniOnDestroy();
	
	
	
	@Override
	public IBinder onBind(Intent intent) {
		// TODO Auto-generated method stub
		return null;
	}
		
	// first this one time method is called. this in turn calls startService()
	
	@Override
	public void onCreate(){

		Log.v(TAG, "onCreate");
		/* We have to create a separate thread which will do all the AllJoyn stuff */
//		busThread = new HandlerThread("BusHandler");
//		busThread.start();
//		mBusHandler = new BusHandler(busThread.getLooper(),this);
//		mBusHandler.sendEmptyMessage(BusHandler.CONNECT);
		jniOnCreate(getPackageName());
		
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId){
		Log.v(TAG, "onStartCommand");
		return START_STICKY;
	}
	
	@Override
	public void onDestroy(){
		Log.v(TAG, "onDestroy");
		// jni on destroy
      //  mBusHandler.getLooper().quit();
 //	       busThread.stop();
		
	}
		
	void PrepareScanResults(){
			
		if(scanResultMessage == null || (scanResultMessage.length == 0)){
			scanResultMessageToBeSent = new ScanResultMessage[0];
			return;
		}
			
		scanResultMessageToBeSent = new ScanResultMessage[scanResultMessage.length];
			
		for(int currentBSSIDIndex=0 ; currentBSSIDIndex < scanResultMessage.length ; currentBSSIDIndex++){
				
			scanResultMessageToBeSent[currentBSSIDIndex] = new ScanResultMessage();
			scanResultMessageToBeSent[currentBSSIDIndex].bssid = scanResultMessage[currentBSSIDIndex].bssid;
			scanResultMessageToBeSent[currentBSSIDIndex].ssid = scanResultMessage[currentBSSIDIndex].ssid;
			scanResultMessageToBeSent[currentBSSIDIndex].attached = scanResultMessage[currentBSSIDIndex].attached;
		}
			
			
	}
		
	// GetScanResults
	public ScanResultMessage[] Scan(boolean request_scan){
			
			scanRequested = request_scan;
			
			
			// Remove -- for test ony ... always on scan
			//request_scan = true;
			
			Log.v(TAG,"---------------------- Scan results request called -------------------------------------------------------- !!!");
			// Else wifi is turned on and we can proceed with the scan
			if(request_scan){
				Log.v(TAG,"***************************Requested Scan Results**************************************");
				// Only a start scan or timeout scan can restart the scan processing so we set this boolean
//					stopScanRequested = false;
				
					Log.v(TAG," =-=-=-Scan-=-=-= Requested");

					wifiMgr = (WifiManager)getSystemService(Context.WIFI_SERVICE);
					
					boolean wifiEnabled = wifiMgr.isWifiEnabled(); 
					
					// This handles wifi not being enabled and hotspot being enabled
					
					if(!wifiEnabled){
						// return empty map since wifi is not enabled
						scanResultMessage = new ScanResultMessage[0];
						return scanResultMessage; 
					}
					
					scanResultsObtained = false;
	
					if(receiver == null){
						// Pass the map and the boolean scanResultsObtained here and use the same map to form the return message 
						receiver = new ScanResultsReceiver(this);
						registerReceiver(receiver, new IntentFilter(
								WifiManager.SCAN_RESULTS_AVAILABLE_ACTION));
					}
					
					if(!wifiMgr.startScan()){
						Log.v(TAG,"startScan() returned error");
					}
					
					// Check the boolean passed to the ScanResultsReceiver
					// If it was set then you can return the result 
					// Note : It can be the case that the scan did not return any results 
					while(true){
						Log.v(TAG,"Waiting for scanResultsObtained");
						if(scanResultsObtained){
							break;
						}
						else{
							try{	
								Thread.sleep(5000);
							}catch(InterruptedException ie){
								Log.v(TAG, "Thread was interrupted while it was sleeping");
							}
						}
					}
					PrepareScanResults();
			}
			else{
				Log.v(TAG,"*************************** NOT REQUESTED Scan Results**************************************");
				PrepareScanResults();
			}
		
			scanResultMessage = null;
			
			
			Log.v(TAG,"************************FINAL SCAN RESULTS ****************************************");
			for(int i=0 ; i < scanResultMessageToBeSent.length ; i++){
				ScanResultMessage result = scanResultMessageToBeSent[i];
				Log.v("Entry-->",result.bssid + " " + result.ssid + " " + result.attached);
			}
			Log.v(TAG,"*************************************************************************************");
			
			return scanResultMessageToBeSent;
		}
	
}
