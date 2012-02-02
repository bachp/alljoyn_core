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
 * This class is to ensure that AllJoyn daemon is initialized to be ready for accepting connections from clients.
 * Before invoking Connect() to BusAttachment, the application should call PrepareDaemon()/PrepareDaemonAsync()
 * if it expects to use the bundled daemon no other daemons are available. PrepareDaemon() waits until the daemon
 * initialization is done. PrepareDaemonAsync() is the async version of PrepareDaemon().
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
    private static boolean sDaemonInited= false;
    public static String sConnectSpec = null;
    
    /**
     * Check whether daemon service is already initialized
     * @return true if already initialized
     */
    public static boolean IsDaemonInited() {
        return sDaemonInited;
    }
    /**
     * Asynchronously initialize daemon service if needed.
     * If BusAttachment.Connect() is invoked from the android UI thread, then PrepareDaemonAsync() must be used
     * for the initialization logic to work correctly. In addition there should be a time delay of 200ms ~ 500ms between
     * PrepareDaemonAsync() and BusAttachment.Connect().
     */
    public static void PrepareDaemonAsync(Context context) {
        if(sIniting) {
            return;
        }
        sInitThread = new InitThread();
        sContext = context.getApplicationContext();
        sInitThread.start();
        sIniting = true;
    }

    /**
     * Initialize daemon service if needed.
     * First it checks whether any daemon is running; if no daemon is running, then it starts the APK daemon if it is installed;
     * If no APK daemon is installed, then starts the bundled daemon. This is a blocking call; it blocks until the daemon is ready or
     * no daemon is found. Thus only non-UI thread is allowed to call PrepareDaemon().
     * @param context The application context
     * @return true  if the daemon is ready for connection
     *         false if no daemon is available
     */
    public static boolean PrepareDaemon(Context context) {
        int CONNECT_WAIT_PERIOD = 100;
        int CONNECT_TRY_MAX = 10;
        context = context.getApplicationContext();
        BusAttachment bus =  new BusAttachment(context.getPackageName(), BusAttachment.RemoteMessage.Receive);
        Status status = bus.connect();
        bus.release();
        if (Status.OK == status) {
            Log.d(TAG, "Daemon Service is already Running.");
            sDaemonInited = true;
            return true;
        }

        Intent apkDaemonIntent = new Intent();
        apkDaemonIntent.setAction("org.alljoyn.bus.START_DAEMON");
        boolean serviceExist = true;
        if(null == context.startService(apkDaemonIntent)) {
            serviceExist = false;
        }
        int count = CONNECT_TRY_MAX;

        try {
            while(serviceExist && count > 0) {
                Thread.sleep(CONNECT_WAIT_PERIOD);
                count--;
                bus =  new BusAttachment(context.getPackageName(), BusAttachment.RemoteMessage.Receive);
                status = bus.connect();
                bus.release();
                if(Status.OK == status) {
                    Log.d(TAG, "APK daemon service is running.");
                    sDaemonInited = true;
                    return true;
                }
            }
        } catch (InterruptedException e) {
            Log.e(TAG, "PrepareDaemon: " + e.getMessage());
        }

        Intent bundleDaemonIntent = new Intent(context, org.alljoyn.bus.alljoyn.BundleDaemonService.class);
        int pid = Process.myPid();
        String spec = "unix:abstract=alljoyn-" + pid;
        sConnectSpec = spec;
        bundleDaemonIntent.putExtra("ConnectSpec", spec);
        if (null == context.startService(bundleDaemonIntent)) {
            serviceExist = false;
        } else {
            serviceExist = true;
        }

        count = CONNECT_TRY_MAX;

        try {
            while(serviceExist && count > 0) {
                Thread.sleep(CONNECT_WAIT_PERIOD);
                count--;
                bus =  new BusAttachment(context.getPackageName(), BusAttachment.RemoteMessage.Receive);
                status = bus.connect();
                bus.release();
                if(Status.OK == status) {
                    Log.d(TAG, "Bundled daemon service is running.");
                    bus.disconnect();
                    sDaemonInited = true;
                    return true;
                }
            }
        } catch (InterruptedException e) {
            Log.e(TAG, "PrepareDaemon: " + e.getMessage());
        }
        Log.d(TAG, "No daemon service is available.");
        sDaemonInited = true;
        sConnectSpec = null;
        return false;
    }
    
}
