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
package org.alljoyn.bus.p2p.service;

import android.os.IBinder;
import android.os.Binder;
import android.os.Handler;

import android.app.Service;

import android.content.Intent;

import android.util.Log;

public class P2pHelperAndroidService extends Service {

    static {
        System.loadLibrary("P2pHelperService");
    }

    private static final String TAG = "P2pHelperService";

    public class LocalBinder extends Binder {
        P2pHelperAndroidService getService() {
            return P2pHelperAndroidService.this;
        }
    }

    private final IBinder mBinder = new LocalBinder();

    public IBinder onBind(Intent intent) {
        Log.i(TAG, "onBind()");
        return mBinder;
    }

    private P2pHelperService mP2pHelperService = null;
    private Handler mHandler = null;
    private Runnable mConnect = null;

    private final long connectionRetryInterval = 5000;

    public void onCreate() {
        Log.i(TAG, "onCreate()");
        mHandler = new Handler();
        mConnect = new Runnable() {
            public void run() {
                if (null == mP2pHelperService) {
                    Log.d(TAG, "Attempting to connect to daemon");
                    mP2pHelperService = new P2pHelperService(getApplicationContext(), "unix:abstract=alljoyn");
                }
                if (!mP2pHelperService.isConnected()) {
                    Log.d(TAG, "Not connected to daemon");
                    mP2pHelperService = null;
                }
                mHandler.postDelayed(mConnect, connectionRetryInterval);
            }
        };
    }

    private void cleanup() {
        mHandler.removeCallbacks(mConnect);

        if (mP2pHelperService != null) {
            mP2pHelperService.shutdown();
            mP2pHelperService = null;
        }
    }

    public void onDestroy() {
        Log.i(TAG, "onDestroy()");
        cleanup();
    }

    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG, "onStartCommand()");

        cleanup();
        mHandler.post(mConnect);

        return START_STICKY;
    }

    public void exit() {
        Log.i(TAG, "exit");
        System.exit(0);
    }

}
