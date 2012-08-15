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

import android.app.Activity;

import android.os.Bundle;
import android.os.IBinder;

import android.content.Context;
import android.content.Intent;
import android.content.ComponentName;
import android.content.ServiceConnection;


import android.util.Log;
import android.view.View;
import android.widget.Button;

public class P2pHelperAndroidActivity extends Activity {
    private static final String TAG = "P2pHelperAndroidActivity";

    private Button mOnFoundAdvertisedNameButton = null;
    private Button mOnLostAdvertisedNameButton = null;
    private Button mOnLinkEstablishedButton = null;
    private Button mOnLinkErrorButton = null;
    private Button mOnLinkLostButton = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Log.i(TAG, "onCreate()");
        setContentView(R.layout.main);

        mOnFoundAdvertisedNameButton = (Button)findViewById(R.id.mainOnFoundAdvertisedName);
        mOnFoundAdvertisedNameButton.setOnClickListener(new View.OnClickListener() {
                                                            public void onClick(View v) {
                                                                if (mBoundService != null) {
                                                                    mBoundService.onFoundAdvertisedName("org.yadda.1234", "org.blah",
                                                                                                        "4506750e77b049fc94880d35d9ea6cdd", "08002e000123");
                                                                }
                                                            }
                                                        });

        mOnLostAdvertisedNameButton = (Button)findViewById(R.id.mainOnLostAdvertisedName);
        mOnLostAdvertisedNameButton.setOnClickListener(new View.OnClickListener() {
                                                           public void onClick(View v) {
                                                               if (mBoundService != null) {
                                                                   mBoundService.onLostAdvertisedName("org.yadda.1234", "org.blah",
                                                                                                      "4506750e77b049fc94880d35d9ea6cdd", "08002e000123");
                                                               }
                                                           }
                                                       });

        mOnLinkEstablishedButton = (Button)findViewById(R.id.mainOnLinkEstablished);
        mOnLinkEstablishedButton.setOnClickListener(new View.OnClickListener() {
                                                        public void onClick(View v) {
                                                            if (mBoundService != null) {
                                                                mBoundService.onLinkEstablished(1);
                                                            }
                                                        }
                                                    });

        mOnLinkErrorButton = (Button)findViewById(R.id.mainOnLinkError);
        mOnLinkErrorButton.setOnClickListener(new View.OnClickListener() {
                                                  public void onClick(View v) {
                                                      if (mBoundService != null) {
                                                          mBoundService.onLinkError(2, 3);
                                                      }
                                                  }
                                              });

        mOnLinkLostButton = (Button)findViewById(R.id.mainOnLinkLost);
        mOnLinkLostButton.setOnClickListener(new View.OnClickListener() {
                                                 public void onClick(View v) {
                                                     if (mBoundService != null) {
                                                         mBoundService.onLinkLost(4);
                                                     }
                                                 }
                                             });

        Intent intent = new Intent(this, P2pHelperAndroidService.class );
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
    }

    private P2pHelperAndroidService mBoundService = null;

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            mBoundService = ((P2pHelperAndroidService.LocalBinder)service).getService();
        }

        public void onServiceDisconnected(ComponentName className) {
            mBoundService = null;
        }
    };

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "onDestroy()");
    }
}
