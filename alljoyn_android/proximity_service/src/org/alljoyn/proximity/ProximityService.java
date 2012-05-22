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

import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;

import org.alljoyn.bus.BusAttachment;
import org.alljoyn.bus.BusListener;
import org.alljoyn.bus.BusObject;
import org.alljoyn.bus.Mutable;
import org.alljoyn.bus.SessionOpts;
import org.alljoyn.bus.SessionPortListener;
import org.alljoyn.bus.Status;


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




public class ProximityService extends Service {

	static {
        System.loadLibrary("alljoyn_java");
    }
    
	// Handler for all AllJoyn related things
	private Handler mBusHandler;
	
	// Bus Object implementing the service
	private ScanService mScanService;
	
	// For logging
	private static final String TAG = "ProximityService";
	
	// Moved here from bus handler
	private BusAttachment mBus;
	
	private HandlerThread  busThread;
	
	BroadcastReceiver receiver;
	
	
	
	@Override
	public IBinder onBind(Intent intent) {
		// TODO Auto-generated method stub
		return null;
	}
	
	// first this one time method is called. this in turn calls startService()
	
	@Override
	public void onCreate(){

		Log.v("ProximityService", "onCreate");
//		busThread = new HandlerThread("BusHandler");
//		busThread.start();
//		mBusHandler = new BusHandler(busThread.getLooper());
//		mScanService = new ScanService();
//		mBusHandler.sendEmptyMessage(BusHandler.CONNECT);
		
		
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId){
		Log.v("ProximityService", "onStartCommand");
		
		/* We have to create a separte thread which will do all the AllJoyn stuff */
		HandlerThread  busThread = new HandlerThread("BusHandler");
		busThread.start();
		mBusHandler = new BusHandler(busThread.getLooper());
		mScanService = new ScanService();
		mBusHandler.sendEmptyMessage(BusHandler.CONNECT);
		Log.v("MIN_MAIN_THREAD","=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
		return START_STICKY;
	}
	
	@Override
	public void onDestroy(){
		Log.v("ProximityService", "onDestroy");
		busThread.stop();
		
	}
	
	class ScanService implements ProximityServiceInterface, BusObject {
		
		private static final String TAG = "ProximityService";
		final int NOT_CONNECTED_TO_ANY_NW = -1;
//		BroadcastReceiver receiver;
		WifiManager wifiMgr;
//		boolean startScan = false;
		ScanResultMessage[] scanResultMessage;
		ScanResultMessage[] scanResultMessageToBeSent;
		boolean scanResultsObtained;
		boolean scanRequested = false;
		boolean stopScanRequested = false;
		// This needs to be accessible by the function which periodically populates results in the map
		boolean getScanResultsCallCompleted = false;
	
//		public void StartScan() {
//			startScan = true;
//		}
		
		public void StopScan() {
			stopScanRequested = true;
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
			
			Log.v(TAG,"---------------------- SCAN called Wooooohoooooooo -------------------------------------------------------- !!!");
			

			// Else wifi is turned on and we can proceed with the scan
			if(request_scan){
				Log.v("ScanResults","***************************Requested Scan Results**************************************");
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
								Log.v("ProximityService", "Thread was interrupted while it was sleeping");
							}
						}
					}
					PrepareScanResults();
			}
			else{
				Log.v("ScanResults","*************************** NOT REQUESTED Scan Results**************************************");
				PrepareScanResults();
			}
		
			scanResultMessage = null;
			
			
			Log.v("ScanResults","************************FINAL SCAN RESULTS ****************************************");
			for(int i=0 ; i < scanResultMessageToBeSent.length ; i++){
				ScanResultMessage result = scanResultMessageToBeSent[i];
				Log.v("Entry-->",result.bssid + " " + result.ssid + " " + result.attached);
			}
			Log.v("ScanResults","*************************************************************************************");
			
			
			
			
			return scanResultMessageToBeSent;
		}
		
	}
	
	
	public class BusHandler extends Handler {
		
		
		public static final int CONNECT = 1;
		public static final int DISCONNECT = 2;
		public static final int SERVICE_UP = 3;
		private static final short SESSION_PORT=42;
		private static final String SERVICE_NAME = "org.alljoyn.proximity.proximityservice";
		
		public BusHandler(Looper looper){
			super(looper);
		}
		
		@Override
		public void handleMessage(Message msg){
	
				switch(msg.what){
				
			
				case CONNECT:{
					
					while(true){
								Log.v("ProximityService","Service INVOKED");
								// Create a new BusAttachment
								mBus = new BusAttachment(getPackageName(), BusAttachment.RemoteMessage.Receive);
								
								// Register Bus Listener
								mBus.registerBusListener(new BusListener());
								
								// Register Bus Object
								Status status = mBus.registerBusObject(mScanService, "/ProximityService");					
								Log.v("BusAttachment.registerBusObject()", "Done=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
								
								if (status != Status.OK) {
									Log.v("BusAttachment.registerBusObject()", "Failed");
								}
								
								// Connect to the Bus
								// If the status of the connect is an error we sleep for sometime and then retry
								// This is because the service is useful only the daemon is up and running so we 
								// wait for a confirmation for the daemon to be up and running
								
									status = mBus.connect();
									Log.v("BusAttachment.connect()", "Called");
									if (status == Status.BUS_ALREADY_CONNECTED || status == Status.OK) {
										Log.v("BusAttachment.Conect()", "Done=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
										break;
									}
									else {
										try{
											mBus.disconnect();
											mBus.release();
											Thread.sleep(10000);
											Log.v("BusAttachment.Disconnecting .... ", "Done=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
										}catch(InterruptedException ie){
											Log.v("ProximityService", "Thread was interrupted while it was sleeping");
										}
										
									}
					}
								/*
				                 * request a well-known name from the bus
				                 */
				                int flag = BusAttachment.ALLJOYN_REQUESTNAME_FLAG_REPLACE_EXISTING | BusAttachment.ALLJOYN_REQUESTNAME_FLAG_DO_NOT_QUEUE;
				                
				                Status status = mBus.requestName(SERVICE_NAME, flag);
				                logStatus(String.format("BusAttachment.requestName(%s, 0x%08x)", SERVICE_NAME, flag), status);
				                
				                mScanService.wifiMgr = (WifiManager)getSystemService(Context.WIFI_SERVICE);
				                
				                if(receiver == null){
									// Pass the map and the boolean scanResultsObtained here and use the same map to form the return message 
									receiver = new ScanResultsReceiver(mScanService);
									registerReceiver(receiver, new IntentFilter(
											WifiManager.SCAN_RESULTS_AVAILABLE_ACTION));
								}
				                
				                while(true){
					                try{
										Thread.sleep(60000);
										// Try to see if the daemon is still up
										// We may come across a case where the daemon has shut down
									}catch(InterruptedException ie){
										Log.v("ProximityService", "Thread was interrupted while it was sleeping");
									}
				                }
				                
					 
				}
				
			    /* Release all resources acquired in connect. */
	            case DISCONNECT: {
	                /* 
	                 * It is important to unregister the BusObject before disconnecting from the bus.
	                 * Failing to do so could result in a resource leak.
	                 */
	                mBus.unregisterBusObject(mScanService);
	                mBus.disconnect();
	                mBusHandler.getLooper().quit();
	                break;   
	            }

	            default:
	                break;
					
			}
			
		}
		
	}
	
    private void logStatus(String msg, Status status) {
        String log = String.format("%s: %s", msg, status);
        if (status == Status.OK) {
            Log.i(TAG, log);
        } 
    }
	
	
	
	
	
}
