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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pInfo;
import android.net.wifi.p2p.WifiP2pManager;
import android.net.wifi.p2p.WifiP2pManager.Channel;
import android.net.wifi.p2p.WifiP2pManager.ConnectionInfoListener;
import android.net.wifi.p2p.WifiP2pManager.PeerListListener;
import android.util.Log;

public class P2pReceiver extends BroadcastReceiver {
    private WifiP2pManager manager;
    private P2pManager p2pManager;
    private Channel channel;

    public P2pReceiver(WifiP2pManager manager, Channel channel,
                       P2pManager p2pManager) {
        this.manager = manager;
        this.channel = channel;
        this.p2pManager = p2pManager;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();

        Log.d(P2pManager.TAG, "onReceive(): " + action);

        if (manager == null) {
            Log.d(P2pManager.TAG, "onReceive(): manager is NULL");
            return;
        }

        if (p2pManager == null) {
            Log.d(P2pManager.TAG, "onReceive(): caller is NULL");
            return;
        }

        if (WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION.equals(action)) {
            int state = intent.getIntExtra(WifiP2pManager.EXTRA_WIFI_STATE, -1);
            Log.d(P2pManager.TAG, "P2P state changed - " + state);
            p2pManager.setEnabled(state == WifiP2pManager.WIFI_P2P_STATE_ENABLED);
        } else if (WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION.equals(action)) {
            Log.d(P2pManager.TAG, "P2P connection changed");
            p2pManager.onConnectionInfoAvailable((WifiP2pInfo) intent.getParcelableExtra(WifiP2pManager.EXTRA_WIFI_P2P_INFO));
        } else if (WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION.equals(action)) {
            Log.d(P2pManager.TAG, "This device has changed");
            p2pManager.setDevice((WifiP2pDevice) intent.getParcelableExtra(WifiP2pManager.EXTRA_WIFI_P2P_DEVICE));
        } else if (WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION.equals(action)) {
            Log.d(P2pManager.TAG, "P2P discovery changed");
            p2pManager.discoveryChanged(intent.getIntExtra(WifiP2pManager.EXTRA_DISCOVERY_STATE, -1));
        } else if (WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION.equals(action)) {
            manager.requestPeers(channel, (PeerListListener) p2pManager);
        }
    }
}
