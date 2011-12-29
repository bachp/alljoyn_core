/*
 * Copyright 2011, Qualcomm Innovation Center, Inc.
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

package org.alljoyn.bus.alljoyn;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.os.Process;

import org.alljoyn.bus.BusAttachment;
import org.alljoyn.bus.Status;

/**
 * The purpose of this class to ensure that AllJoyn Daemon is initialized to be ready for accepting connections from clients.
 * Before invoking Connect() to BusAttachment, an application should call PrepareDaemon/PrepareDaemonAsync if it needs to 
 * use the bundled daemon when the pre-installed daemon is not available. PrepareDaemon() waits until the Daemon initialization
 * finishes. PrepareDaemonAsync() is the async version of PrepareDaemon(). If it is called from the Android UI thread (e.g. 
 * in C++ binding), then PrepareDaemonAsync() is required be used for the logic to work correctly, and a time period between 
 * PrepareDaemonAsync() and BusAttachment.Connect() is expected.
 *
 */
public class DaemonInit {
    static class InitThread extends Thread {
        @Override
        public void run() {
            PrepareDaemon(sContext);
            sIniting = false;
        }
    }

    private final static String TAG = "DaemonInit";
    private static InitThread sInitThread;
    private static Context sContext;
    private static boolean sIniting= false;
    
    public static void PrepareDaemonAsync(Context context) {
        if(sIniting) {
            return;
        }
        sInitThread = new InitThread();
        sContext = context.getApplicationContext();
        sInitThread.start();
        sIniting = true;
    }

    public static boolean PrepareDaemon(Context context) {
        int CONNECT_WAIT_PERIOD = 100;
        context = context.getApplicationContext();
        BusAttachment bus =  new BusAttachment(context.getPackageName(), BusAttachment.RemoteMessage.Receive);
        Status status = bus.connect();
        Log.d(TAG, "BusAttachment.connect() status = " + status);
        bus.release();
        if (Status.OK == status) {
            return true;
        }

        Intent apkDaemonIntent = new Intent();
        apkDaemonIntent.setAction("org.alljoyn.bus.START_DAEMON");
        boolean serviceExist = true;
        if(null == context.startService(apkDaemonIntent)) {
            serviceExist = false;
        }
        int count = 3;

        try {
            while(serviceExist && count > 0) {
                Thread.sleep(CONNECT_WAIT_PERIOD);
                count--;
                bus =  new BusAttachment(context.getPackageName(), BusAttachment.RemoteMessage.Receive);
                status = bus.connect();
                Log.d(TAG, "BusAttachment APK Daemon connect() status = " + status);
                bus.release();
                if(Status.OK == status) {
                    return true;
                }
            }
        } catch (InterruptedException e) {
            Log.e(TAG, "PrepareDaemon: " + e.getMessage());
        }

        Intent bundleDaemonIntent = new Intent(context, org.alljoyn.bus.alljoyn.BundleDaemonService.class);
        int pid = Process.myPid();
        String spec = "unix:abstract=alljoyn-" + pid;
        bundleDaemonIntent.putExtra("ConnectSpec", spec);
        if (null == context.startService(bundleDaemonIntent)) {
            serviceExist = false;
        } else {
            serviceExist = true;
        }

        count = 3;

        try {
            while(serviceExist && count > 0) {
                Thread.sleep(CONNECT_WAIT_PERIOD);
                count--;
                bus =  new BusAttachment(context.getPackageName(), BusAttachment.RemoteMessage.Receive);
                status = bus.connect();
                Log.d(TAG, "BusAttachment Bundle Daemon connect() status = " + status); 
                bus.release();
                if(Status.OK == status) {
                    bus.disconnect();
                    return true;
                }
            }
        } catch (InterruptedException e) {
            Log.e(TAG, "PrepareDaemon: " + e.getMessage());
        }
        return false;
    }
    
}
