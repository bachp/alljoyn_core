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
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

import android.app.Service;
import android.app.Notification;
import android.app.PendingIntent;

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

    private static final int NOTIFICATION_ID = 0xca11ab1e;

    private Object mBus = null; // TODO: replace with JNI handle
    P2pHelperService mP2pHelperService = null;
    BackgroundHandler mBackgroundHandler = null;
    private P2pManager mP2pManager = null;

    public void onCreate() {
        Log.i(TAG, "onCreate()");

        CharSequence title = "AllJoyn";
        CharSequence message = "AllJoyn Wi-Fi Direct Helper Service";
        Intent intent = new Intent(this, P2pHelperAndroidActivity.class );
        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, intent, 0);
        Notification notification = new Notification(R.drawable.p2p_icon, null, System.currentTimeMillis());
        notification.setLatestEventInfo(this, title, message, pendingIntent);
        notification.flags |= Notification.DEFAULT_SOUND | Notification.FLAG_ONGOING_EVENT | Notification.FLAG_NO_CLEAR;

        Log.i(TAG, "onCreate(): startForeground()");
        startForeground(NOTIFICATION_ID, notification);

        HandlerThread handlerThread = new HandlerThread("BackgroundHandler");
        handlerThread.start();
        mBackgroundHandler = new BackgroundHandler(handlerThread.getLooper());

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

    private static final String SERVICE_NAME = "org.alljoyn.bus.p2p";

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

    public void onFoundAdvertisedName(String name, String namePrefix, String guid, String device) {
        Log.i(TAG, "onFoundAdvertisedName("
              + name + ", "
              + namePrefix + ", "
              + guid + ", "
              + device + "): Sending to BackgroundHandler");
        mBackgroundHandler.onFoundAdvertisedName(name, namePrefix, guid, device);
    }

    public void onLostAdvertisedName(String name, String namePrefix, String guid, String device) {
        Log.i(TAG, "onLostAdvertisedName("
              + name + ", "
              + namePrefix + ", "
              + guid + ", "
              + device + "): Sending to BackgroundHandler");
        mBackgroundHandler.onLostAdvertisedName(name, namePrefix, guid, device);
    }

    public void onLinkEstablished(int handle) {
        Log.i(TAG, "onLinkEstablished(" + handle + "): Sending to BackgroundHandler");
        mBackgroundHandler.onLinkEstablished(handle);
    }

    public void onLinkError(int handle, int error) {
        Log.i(TAG, "onLinkError(" + handle + ", " + error + "): Sending to BackgroundHandler");
        mBackgroundHandler.onLinkError(handle, error);
    }

    public void onLinkLost(int handle) {
        Log.i(TAG, "onLinkLost(" + handle + "): Sending to BackgroundHandler");
        mBackgroundHandler.onLinkLost(handle);
    }

    private class MessageArgs {
        public MessageArgs(String name, String namePrefix, String guid, String device) {
            mName = name;
            mNamePrefix = namePrefix;
            mGuid = guid;
            mDevice = device;
        }

        String mName;
        String mNamePrefix;
        String mGuid;
        String mDevice;
    }

    private final class BackgroundHandler extends Handler {
        public BackgroundHandler(Looper looper) {
            super(looper);
        }

        public void onFoundAdvertisedName(String name, String namePrefix, String guid, String device) {
            Log.i(TAG, "BackgroundHandler.onFoundAdvertisedName("
                  + name + ", "
                  + namePrefix + ", "
                  + guid + ", "
                  + device + "): Queueing message");
            Message msg = mBackgroundHandler.obtainMessage(DO_ON_FOUND_ADVERTISED_NAME,
                                                           new MessageArgs(name, namePrefix, guid, device));
            mBackgroundHandler.sendMessage(msg);
        }

        public void onLostAdvertisedName(String name, String namePrefix, String guid, String device) {
            Log.i(TAG, "BackgroundHandler.onLostAdvertisedName("
                  + name + ", "
                  + namePrefix + ", "
                  + guid + ", "
                  + device + "): Queueing message");
            Message msg = mBackgroundHandler.obtainMessage(DO_ON_LOST_ADVERTISED_NAME,
                                                           new MessageArgs(name, namePrefix, guid, device));
            mBackgroundHandler.sendMessage(msg);
        }

        public void onLinkEstablished(int handle) {
            Log.i(TAG, "BackgroundHandler.onLinkEstablished(" + handle + "): Queueing message");
            Message msg = mBackgroundHandler.obtainMessage(DO_ON_LINK_ESTABLISHED, handle, 0, null);
            mBackgroundHandler.sendMessage(msg);
        }

        public void onLinkError(int handle, int error) {
            Log.i(TAG, "BackgroundHandler.onLinkError(" + handle + ", " + error + "): Queueing message");
            Message msg = mBackgroundHandler.obtainMessage(DO_ON_LINK_ERROR, handle, error, null);
            mBackgroundHandler.sendMessage(msg);
        }

        public void onLinkLost(int handle) {
            Log.i(TAG, "BackgroundHandler.onLinkLost(" + handle + "): Queueing message");
            Message msg = mBackgroundHandler.obtainMessage(DO_ON_LINK_LOST, handle, 0, null);
            mBackgroundHandler.sendMessage(msg);
        }

        private static final int DO_ON_FOUND_ADVERTISED_NAME = 1;
        private static final int DO_ON_LOST_ADVERTISED_NAME = 2;
        private static final int DO_ON_LINK_ESTABLISHED = 3;
        private static final int DO_ON_LINK_ERROR = 4;
        private static final int DO_ON_LINK_LOST = 5;

        public void handleMessage(Message msg) {
            switch (msg.what) {
            case DO_ON_FOUND_ADVERTISED_NAME:
                doOnFoundAdvertisedName(((MessageArgs)msg.obj).mName, ((MessageArgs)msg.obj).mNamePrefix,
                                        ((MessageArgs)msg.obj).mGuid, ((MessageArgs)msg.obj).mDevice);
                break;

            case DO_ON_LOST_ADVERTISED_NAME:
                doOnLostAdvertisedName(((MessageArgs)msg.obj).mName, ((MessageArgs)msg.obj).mNamePrefix,
                                       ((MessageArgs)msg.obj).mGuid, ((MessageArgs)msg.obj).mDevice);
                break;

            case DO_ON_LINK_ESTABLISHED:
                doOnLinkEstablished(msg.arg1);
                break;

            case DO_ON_LINK_ERROR:
                doOnLinkError(msg.arg1, msg.arg2);
                break;

            case DO_ON_LINK_LOST:
                doOnLinkLost(msg.arg1);
                break;
            }
        }

        public void doOnFoundAdvertisedName(String name, String namePrefix, String guid, String device) {
            Log.i(TAG, "BackgroundHandler.doOnFoundAdvertisedName("
                  + name + ", "
                  + namePrefix + ", "
                  + guid + ", "
                  + device + "): Sending signal");
            mP2pHelperService.OnFoundAdvertisedName(name, namePrefix, guid, device);
        }

        public void doOnLostAdvertisedName(String name, String namePrefix, String guid, String device) {
            Log.i(TAG, "BackgroundHandler.doOnLostAdvertisedName("
                  + name + ", "
                  + namePrefix + ", "
                  + guid + ", "
                  + device + "): Sending signal");
            mP2pHelperService.OnLostAdvertisedName(name, namePrefix, guid, device);
        }

        public void doOnLinkEstablished(int handle) {
            Log.i(TAG, "BackgroundHandler.doOnLinkEstablished(" + handle + "): Sending signal");
            mP2pHelperService.OnLinkEstablished(handle);
        }

        public void doOnLinkError(int handle, int error) {
            Log.i(TAG, "BackgroundHandler.doOnLinkError(" + handle + ", " + error + "): Sending signal");
            mP2pHelperService.OnLinkError(handle, error);
        }

        public void doOnLinkLost(int handle) {
            Log.i(TAG, "BackgroundHandler.doOnLinkLost(" + handle + "): Sending signal");
            mP2pHelperService.OnLinkLost(handle);
        }
    }
}
