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

import android.os.AsyncTask;
import android.os.IBinder;
import android.os.Binder;

import android.app.Service;

import android.content.Intent;
import android.content.Context;

import android.util.Log;

public class P2pHelperAndroidService extends Service {
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

    P2pHelperService mP2pHelperService = null;
    private P2pManager mP2pManager = null;

    public void onCreate() {
        Log.i(TAG, "onCreate()");

        DoStartTask doStartTask = new DoStartTask();
        doStartTask.execute(this);
    }

    public void onDestroy() {
        Log.i(TAG, "onDestroy()");
        if (mP2pHelperService != null) {
            mP2pHelperService.shutdown();
        }
    }

    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG, "onStartCommand()");
        return START_STICKY;
    }

    private class DoStartTask extends AsyncTask<Context, Integer, String> {
        @Override
        protected String doInBackground(Context...params) {

            System.loadLibrary("P2pHelperService");
            mP2pHelperService = new P2pHelperService();

            if (!mP2pHelperService.isReady()) {
                Log.e(TAG, "P2pHelperService could not connect to daemon");
                return "FAILED";
            }

            mP2pManager = new P2pManager(getApplicationContext(), mP2pHelperService);

            return "SUCCESS";
        }
    }

    class P2pHelperService implements P2pInterface {
        private native boolean jniOnCreate();
        private native void jniOnDestroy();
        private native void jniOnFoundAdvertisedName(String name, String namePrefix, String guid, String device);
        private native void jniOnLostAdvertisedName(String name, String namePrefix, String guid, String device);
        private native void jniOnLinkEstablished(int handle);
        private native void jniOnLinkError(int handle, int error);
        private native void jniOnLinkLost(int handle);

        private boolean jniReady = false;

        public P2pHelperService() {
            jniReady = jniOnCreate();
        }

        public synchronized void shutdown() {
            jniReady = false;
            jniOnDestroy();
        }

        protected void finalize() throws Throwable {
            if (jniReady) {
                shutdown();
            }
        }

        public void busFailed() {
            jniReady = false;
            // Try to restart bus?
        }

        public int FindAdvertisedName(String namePrefix) {
            Log.i(TAG, "FindAdvertisedName(" + namePrefix + "): Received RPC call");
            return mP2pManager.findAdvertisedName(namePrefix);
        }

        public int CancelFindAdvertisedName(String namePrefix) {
            Log.i(TAG, "CancelFindAdvertisedName(" + namePrefix + "): Received RPC call");
            return mP2pManager.cancelFindAdvertisedName(namePrefix);
        }

        public int AdvertiseName(String name, String guid) {
            Log.i(TAG, "AdvertiseName(" + name + ", " + guid + "): Received RPC call");
            return mP2pManager.advertiseName(name, guid);
        }

        public int CancelAdvertiseName(String name, String guid) {
            Log.i(TAG, "CancelAdvertiseName(" + name + ", " + guid + "): Received RPC call");
            return mP2pManager.cancelAdvertiseName(name, guid);
        }

        public int EstablishLink(String device, int groupOwnerIntent) {
            Log.i(TAG, "EstablishLink(" + device + ", " + groupOwnerIntent + "): Received RPC call");
            return mP2pManager.establishLink(device, groupOwnerIntent);
        }

        public int ReleaseLink(int handle) {
            Log.i(TAG, "ReleaseLink(" + handle + "): Received RPC call");
            return mP2pManager.releaseLink(handle);
        }

        public String GetInterfaceNameFromHandle(int handle) {
            Log.i(TAG, "GetInterfaceNamefromHandle(" + handle + "): Received RPC call");
            return mP2pManager.getInterfaceNameFromHandle(handle);
        }

        public void OnFoundAdvertisedName(String name, String namePrefix, String guid, String device) {
            if (jniReady) {
                Log.i(TAG, "OnFoundAdvertisedName(" + name + ", " + namePrefix + ", " + guid + ", " + device + "): Sending signal");
                jniOnFoundAdvertisedName(name, namePrefix, guid, device);
            } else {
                Log.e(TAG, "OnFoundAdvertisedName() not sent, JNI not available");
            }
        }

        public void OnLostAdvertisedName(String name, String namePrefix, String guid, String device) {
            if (jniReady) {
                Log.i(TAG, "OnLostAdvertisedName(" + name + ", " + namePrefix + ", " + guid + ", " + device + "): Sending signal");
                jniOnLostAdvertisedName(name, namePrefix, guid, device);
            } else {
                Log.e(TAG, "OnLostAdvertisedName() not sent, JNI not available");
            }
        }

        public void OnLinkEstablished(int handle) {
            if (jniReady) {
                Log.i(TAG, "OnLinkEstablished(" + handle + "): Sending signal");
                jniOnLinkEstablished(handle);
            } else {
                Log.e(TAG, "OnLinkEstablished() not sent, JNI not available");
            }
        }

        public void OnLinkError(int handle, int error) {
            if (jniReady) {
                Log.i(TAG, "OnLinkError(" + handle + ", " + error + "): Sending signal");
                jniOnLinkError(handle, error);
            } else {
                Log.e(TAG, "OnLinkError() not sent, JNI not available");
            }
        }

        public void OnLinkLost(int handle) {
            if (jniReady) {
                Log.i(TAG, "OnLinkLost(" + handle + "): Sending signal");
                jniOnLinkLost(handle);
            } else {
                Log.e(TAG, "OnLinkLost() not sent, JNI not available");
            }
        }

        public boolean isReady() {
            return jniReady;
        }
    }

    public void exit() {
        Log.i(TAG, "exit");
        System.exit(0);
    }
}
