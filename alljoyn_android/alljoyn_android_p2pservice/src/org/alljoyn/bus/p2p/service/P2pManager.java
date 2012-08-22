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

import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.lang.Runnable;

import android.content.Context;
import android.content.IntentFilter;
import android.net.wifi.WpsInfo;
import android.net.wifi.p2p.WifiP2pConfig;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pDeviceList;
import android.net.wifi.p2p.WifiP2pInfo;
import android.net.wifi.p2p.WifiP2pManager;
import android.net.wifi.p2p.WifiP2pManager.ActionListener;
import android.net.wifi.p2p.WifiP2pManager.Channel;
import android.net.wifi.p2p.WifiP2pManager.ConnectionInfoListener;
import android.net.wifi.p2p.WifiP2pManager.PeerListListener;
import android.net.wifi.p2p.WifiP2pManager.DnsSdServiceResponseListener;
import android.net.wifi.p2p.WifiP2pManager.DnsSdTxtRecordListener;
import android.net.wifi.p2p.nsd.WifiP2pDnsSdServiceRequest;
import android.net.wifi.p2p.nsd.WifiP2pDnsSdServiceInfo;
import android.util.Log;
import android.os.Handler;
import android.os.Looper;

enum PeerState {
    DISCONNECTED,
    INITIATED,
    CONNECTING,
    CONNECTED,
    DISCONNECTING
}

enum FindState {
    IDLE,
    DISCOVERING
}

public class P2pManager implements ConnectionInfoListener, DnsSdServiceResponseListener, DnsSdTxtRecordListener, PeerListListener {
    public static final String TAG = "P2pManager";
    private WifiP2pManager manager;
    private P2pInterface busInterface = null;
    private boolean isEnabled = true;
    private Context context = null;
    private P2pReceiver receiver = null;

    private final IntentFilter intentFilter = new IntentFilter();
    private Channel channel;
    private WifiP2pDevice device = null;
    private WifiP2pDeviceList peerList = null;

    private WifiP2pConfig peerConfig = null;
    private PeerState peerState = PeerState.DISCONNECTED;
    private FindState mFindState = FindState.IDLE;

    private Handler mHandler = null;
    private Runnable mPeriodicDiscovery = null;
    private Runnable mPeriodicPeerFind = null;
    private Runnable mRequestConnectionInfo = null;

    private static final long periodicInterval = 40000;
    private static final long connectionTimeout = 150000;

    private ArrayList <String> mServiceRequestList;
    private ArrayList <String> mRequestedNames;

    private class FoundServiceInfo {
        public String name;
        public String prefix;
        public String guid;

        FoundServiceInfo(String name, String prefix, String guid) {
            this.name = name;
            this.prefix = prefix;
            this.guid = guid;
        }
    }
    private HashMap <String, ArrayList<FoundServiceInfo> > mDeviceServices;

    private static final int OK = 0;
    private static final int ERROR = -1;

    private static final int MAX_ADVERTISE_TIMEOUT = 256000;

    private static final String ServiceSuffix = "._alljoyn._tcp.local.";

    public P2pManager(Context context, P2pInterface busInterface) {
        this.context = context;
        this.busInterface = busInterface;

        intentFilter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
        intentFilter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
        intentFilter.addAction(WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION);
        intentFilter.addAction(WifiP2pManager.WIFI_P2P_DISCOVERY_CHANGED_ACTION);
        intentFilter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);

        manager = (WifiP2pManager) context.getSystemService(Context.WIFI_P2P_SERVICE);
        channel = manager.initialize(context, context.getMainLooper(), null);
        manager.setDnsSdResponseListeners(channel, this, this);

        receiver = new P2pReceiver(manager, channel, this);
        context.registerReceiver(receiver, intentFilter); // TODO: When to unregister?

        mServiceRequestList = new ArrayList<String>();
        mRequestedNames = new ArrayList<String>();
        mDeviceServices = new HashMap<String, ArrayList<FoundServiceInfo> >();

        // Can only connect while discovering peers or services
        //manager.discoverPeers(channel, null);

        //Object groupApprover = DialogListenerProxy.newDialogListener(manager, channel);
        //DialogListenerProxy.setDialogListener(manager, channel, groupApprover);

        Looper.prepare();
        mHandler = new Handler();

        mPeriodicPeerFind = new Runnable() {
            public void run() {
                if (!isEnabled /* || (mFindState == FindState.IDLE && not discovering)*/) {
                    return;
                }

                manager.discoverPeers(channel, null);

                Log.d(TAG, "Keep P2P find alive");
                mHandler.postDelayed(mPeriodicPeerFind, periodicInterval);
            }
        };

        mPeriodicDiscovery = new Runnable() {
            public void run() {
                if (!isEnabled /* || (mFindState == FindState.IDLE && not advertising)*/) {
                    return;
                }

                manager.discoverServices(channel,
                                         new WifiP2pManager.ActionListener()
                                         {
                                             public void onSuccess() {
                                                 Log.d(TAG, "Service discovery started");
                                                 mFindState = FindState.DISCOVERING;
                                             }

                                             public void onFailure(int reasonCode) {
                                                 Log.d(TAG, "Service discovery failed: " + reasonCode);
                                                 mFindState = FindState.IDLE;
                                                 // TODO: Any other error handling?
                                             }
                                         });
                mHandler.postDelayed(mPeriodicDiscovery, periodicInterval);
            }
        };

        final ConnectionInfoListener connInfoListener = this;
        mRequestConnectionInfo = new Runnable() {
            public void run() {
                if (!isEnabled) {
                    return;
                }

                Log.d(TAG, "Connection initiation timeout: Request connection info");
                manager.requestConnectionInfo(channel, connInfoListener);
            }
        };
    };

    private void doDiscoverServices(boolean start) {
        mHandler.removeCallbacks(mPeriodicDiscovery);
        if (start) {
            mPeriodicDiscovery.run();
        }
    }

    private void doFindPeers(boolean start) {
        mHandler.removeCallbacks(mPeriodicPeerFind);
        if (start) {
            mPeriodicPeerFind.run();
        }
    }

    public void setEnabled(boolean enabled) {
        // TODO: Cancel advertisements or automatically re-register them?
        isEnabled = enabled;

        // When WiFi is disabled, onConnectionInfoAvailable() handles sending
        // of OnLinkError or OnLinkLost.
    }

    private int getHandle(String addr) {
        if (addr == null || addr.isEmpty()) {
            return 0;
        } else {
            return addr.hashCode() >>> 1;
        }
    }

    public void onConnectionInfoAvailable(WifiP2pInfo info) {
        Log.d(TAG, "onConnectionInfoAvailable()");
        Log.d(TAG, "Group Formed: " + info.groupFormed);
        Log.d(TAG, "Is Group Owner: " + info.isGroupOwner);
        if (info.groupOwnerAddress != null) {
            Log.d(TAG, "Group Owner Address: " + info.groupOwnerAddress.getHostAddress());
        }

        mHandler.removeCallbacks(mRequestConnectionInfo);

        switch (peerState) {
        case INITIATED:
        case CONNECTING:
            if (info.groupFormed) {
                peerState = PeerState.CONNECTED;
                busInterface.OnLinkEstablished(getHandle(peerConfig.deviceAddress));
            } else {
                int handle = 0;
                if (peerConfig != null) {
                    handle = getHandle(peerConfig.deviceAddress);
                }
                peerState = PeerState.DISCONNECTED;
                peerConfig = null;
                if (handle != 0) {
                    busInterface.OnLinkError(handle, -1);
                }
            }
            break;

        case CONNECTED:
            if (!info.groupFormed) {
                int handle = 0;
                if (peerConfig != null) {
                    handle = getHandle(peerConfig.deviceAddress);
                }
                peerState = PeerState.DISCONNECTED;
                peerConfig = null;
                busInterface.OnLinkLost(handle);
            }
            break;

        case DISCONNECTING:
        case DISCONNECTED:
            if (info.groupFormed) {
                // Assume it's an incoming connection
                peerState = PeerState.CONNECTED;
                // Dummy, for now
                peerConfig = new WifiP2pConfig();
                // Looks like the P2P framework does not tell us who connected.  Confirm?
                peerConfig.deviceAddress = "";
                busInterface.OnLinkEstablished(0);
            } else {
                int handle = 0;
                if (peerConfig != null) {
                    handle = getHandle(peerConfig.deviceAddress);
                }
                peerState = PeerState.DISCONNECTED;
                peerConfig = null;
                busInterface.OnLinkLost(handle);
            }
            break;
        }
    }

    private void deviceLost(WifiP2pDevice lostPeer) {
        Log.d(TAG, "deviceLost(): " + lostPeer.deviceAddress);

        ArrayList<FoundServiceInfo> services = null;
        String address = lostPeer.deviceAddress;

        synchronized (mDeviceServices) {
            if (!mDeviceServices.isEmpty() && mDeviceServices.containsKey(address)) {
                services = mDeviceServices.get(address);
                mDeviceServices.remove(address);
            }
        }

        if (services == null || services.isEmpty())
            return;

        Iterator<FoundServiceInfo> itr = services.iterator();

        while (itr.hasNext()) {
            FoundServiceInfo serviceInfo = (FoundServiceInfo) itr.next();
            busInterface.OnLostAdvertisedName(serviceInfo.name, serviceInfo.prefix, serviceInfo.guid, address);
        }
    }

    public void onPeersAvailable(WifiP2pDeviceList newPeers) {
        Log.d(TAG, "onPeersAvailable");
        if (peerList != null) {
            // Naive iteration for now.  Could be more efficient by using a hash.
            for (WifiP2pDevice oldPeer : peerList.getDeviceList()) {
                boolean found = false;
                for (WifiP2pDevice newPeer : newPeers.getDeviceList()) {
                    if (oldPeer.deviceAddress.equals(newPeer.deviceAddress)) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    deviceLost(oldPeer);
                }
            }
        }

        peerList = newPeers;
    }

    /** AllJoyn API Implementation **/

    synchronized public void onDnsSdServiceAvailable(final String instanceName, String registrationType, WifiP2pDevice srcDevice) {
        Log.d(TAG, "onDnsSdServiceAvailable(): " + instanceName + ", " + registrationType);

        // There must be an pending service discovery request.
        // If the lrequest ist is empty , the logic is broken somewhere.
        if (mRequestedNames.isEmpty()) {
            Log.d(TAG, "Ignore found remote service, since there are no pending service requests");
            return;
        }

        // Cycle through the outstanding service requests in order to figure out
        // if we are interested in this particular AllJoyn service.
        for (int i = 0; i < mRequestedNames.size(); i++) {
            String prefix = mRequestedNames.get(i);
            Log.d(TAG, "Outstanding request for " + prefix);
            if (instanceName.startsWith(prefix)) {
                Log.d(TAG, "Matches");

                //Check if request for service info already exists
                if (!mServiceRequestList.isEmpty() && mServiceRequestList.contains(instanceName))
                    continue;

                Log.d(TAG, "Adding ServiceRequest for " + instanceName);
                WifiP2pDnsSdServiceRequest serviceRequest = WifiP2pDnsSdServiceRequest.newInstance(instanceName, "_alljoyn._tcp");

                manager.addServiceRequest(channel, serviceRequest,
                                          new WifiP2pManager.ActionListener()
                                          {
                                              public void onSuccess() {
                                                  Log.d(TAG, "addServiceRequest( " + instanceName +  " ) success");
                                                  mServiceRequestList.add(instanceName);
                                              }

                                              public void onFailure(int reasonCode) {
                                                  Log.d(TAG, "addServiceRequest (" + instanceName + ") failed: " + reasonCode);
                                              }
                                          });

            }
            continue;
        }
    }

    synchronized private void updateDeviceServiceList(String name, String prefix, String guid, int timer, String address) {
        ArrayList<FoundServiceInfo> services;
        FoundServiceInfo serviceInfo;
        boolean isNew = true;

        // TODO Change key to GUID rather than device address.
        // If this service haven't been seen before, add it to a discovered services list.
        // If timer is set to zero, this means that the service is no longer available:
        // remove from the discovered services list.

        if (mDeviceServices.containsKey(address)) {
            services = mDeviceServices.get(address);
            mDeviceServices.remove(address);
        } else {
            services = new ArrayList<FoundServiceInfo>();
        }

        Iterator<FoundServiceInfo> itr = services.iterator();

        while (itr.hasNext()) {
            serviceInfo = (FoundServiceInfo) itr.next();
            if (name.equals(serviceInfo.name)) {
                isNew = false;
                if (timer == 0)
                    services.remove(serviceInfo);
                break;
            }
        }

        if (isNew) {
            serviceInfo = new FoundServiceInfo(name, prefix, guid);
            services.add(serviceInfo);
        }

        //Check
        Log.d(TAG, "Device " + address + " has " + services.size() + " services");
        mDeviceServices.put(address, services);
    }

    public void onDnsSdTxtRecordAvailable(String fullDomainName, Map<String, String> txtRecordMap, WifiP2pDevice srcDevice) {
        Log.d(TAG, "onDnsSdTxtRecordAvailable(): " + fullDomainName);

        String guid = txtRecordMap.get("GUID");
        int timer =  255;
        String namePrefix = null;
        String name = fullDomainName;

        if (txtRecordMap.containsKey("TIMER"))
            timer = Integer.parseInt(txtRecordMap.get("TIMER"));

        //Strip off the service type (suffix) to get the service name.
        int index = name.lastIndexOf(ServiceSuffix);

        //TODO What do we do in the unlikely event of not finding the postfix?
        if (index > 0)
            name = fullDomainName.substring(0, index);

        // Cycle through the outstanding name discovery requests to find the name prefix.
        synchronized (mServiceRequestList) {
            for (int i = 0; i < mServiceRequestList.size(); i++) {
                String current = mServiceRequestList.get(i);
                Log.d(TAG, "Found outstanding request for " + current);
                //TODO This should not be case insensitive, but somewhere
                // in frameworks the case is being converted to lower...
                if (fullDomainName.regionMatches(true, 0, current, 0, current.length())) {
                    namePrefix = current;
                    break;
                }
            }
        }

        if (namePrefix == null) {
            Log.d(TAG, "No request found for" + name + ". Ignore");
            return;
        }

        Log.d(TAG, "Matched service info request prefix " + namePrefix);

        for (Map.Entry entry : txtRecordMap.entrySet())
            Log.d(TAG, (entry.getKey() + ", " + entry.getValue()));

        updateDeviceServiceList(name, namePrefix, guid, timer, srcDevice.deviceAddress);

        if (timer != 0) {
            busInterface.OnFoundAdvertisedName(name, namePrefix, guid, /*timer,*/ srcDevice.deviceAddress);
        } else {
            busInterface.OnLostAdvertisedName(name, namePrefix, guid, srcDevice.deviceAddress);
        }
    }

    /**
     * Register interest in a well-known name prefix for the purpose of discovery.
     *
     * @param namePrefix    Well-known name prefix.
     *
     * @return
     * <ul>
     * <li>O if the request to start discovery of the provided namePrefix was successful.</li>
     * <li>Other error status codes indicating a failure.</li>
     * </ul>
     */
    public int findAdvertisedName(final String name) {
        Log.d(TAG, "findAdvertisedName(): " + name);
        final String namePrefix;

        if (!isEnabled) {
            Log.d(TAG, "findAdvertisedName(): P2P is OFF");
            return ERROR;
        }

        // Strip the wildcard if it is present.
        int index = name.lastIndexOf("*");

        if (index > 0)
            namePrefix = name.substring(0, index);
        else
            namePrefix = name;

        //TODO Need to implement arg check for size. Deep in the bowels of JNI code is a hidden limitation:
        // command buffer size cannot exceed 256. Buffer includes string version of WPA supplicant command
        // plus actual payload...
        synchronized (mRequestedNames) {
            if (!mRequestedNames.isEmpty() && mRequestedNames.contains(namePrefix)) {
                Log.d(TAG, "Request for " + namePrefix + " already added");
                if (mFindState != FindState.DISCOVERING)
                    doDiscoverServices(true);
                return OK;
            }

            // Keep track of outstanding Service Discovery requests.
            mRequestedNames.add(namePrefix);
        }

        // Find all AllJoyn services first.
        // If we are in discovery mode, then we are already finding all AllJoyn services.
        // Do not create a new request, just keep it on a serviceRequestList.
        if (mFindState == FindState.DISCOVERING)
            return OK;

        WifiP2pDnsSdServiceRequest serviceRequest = WifiP2pDnsSdServiceRequest.newInstance("_alljoyn._tcp");

        manager.addServiceRequest(channel, serviceRequest,
                                  new WifiP2pManager.ActionListener()
                                  {
                                      public void onSuccess() {
                                          Log.d(TAG, "addServiceRequest (find all AJN) success");
                                          doDiscoverServices(true);
                                      }

                                      public void onFailure(int reasonCode) {
                                          Log.d(TAG, "addServiceRequest (find all AJN) failed: " + reasonCode);
                                          synchronized (mServiceRequestList) {
                                              mServiceRequestList.remove(namePrefix);
                                          }
                                      }
                                  });

        return OK;
    }

    /**
     * Cancel interest in a well-known name prefix for the purpose of discovery.
     *
     * @param namePrefix    Well-known name prefix.
     *
     * @return
     * <ul>
     * <li>O if the request to stop discovery of the provided namePrefix was successful.</li>
     * <li>Other error status codes indicating a failure.</li>
     * </ul>
     */
    synchronized public int cancelFindAdvertisedName(String namePrefix) {
        Log.d(TAG, "cancelFindAdvertisedName( "  + namePrefix + " )");

        if (!isEnabled) {
            Log.d(TAG, "cancelFindAdvertisedName(): P2P is OFF");
            return ERROR;
        }

        // Strip the wildcard if it is present.
        int index = namePrefix.lastIndexOf("*");

        if (index > 0)
            namePrefix = namePrefix.substring(0, index);

        // Remove all pending service requests associated with this prefix
        for (int i = 0; !mServiceRequestList.isEmpty() && i < mServiceRequestList.size(); i++) {
            String name = mServiceRequestList.get(i);
            Log.d(TAG, name);

            if (!name.startsWith(namePrefix))
                continue;

            WifiP2pDnsSdServiceRequest serviceRequest = WifiP2pDnsSdServiceRequest.newInstance(name, "_alljoyn._tcp");
            Log.d(TAG, "Remove service name request for" + name);
            manager.removeServiceRequest(channel, serviceRequest,
                                         new WifiP2pManager.ActionListener()
                                         {
                                             public void onSuccess() {
                                                 Log.d(TAG, "removeServiceRequest success");
                                             }

                                             public void onFailure(int reasonCode) {
                                                 Log.d(TAG, "removeServiceRequest failed: " + reasonCode);
                                             }
                                         });

            mServiceRequestList.remove(name);
            break;
        }

        if (!mRequestedNames.isEmpty() && mRequestedNames.contains(namePrefix))
            mRequestedNames.remove(namePrefix);

        // If no pending "Find Advertised Name" requests, cleanup and stop discovery.
        if (mRequestedNames.isEmpty()) {
            Log.d(TAG, "Clear all service requests");
            //TODO We probably don't want to clear ALL requests.
            // Investigate whether this clears the requests from oother entities.
            // Maybe just clear the one for "_alljoyn._tcp" type of services (this is also questionable).
            mServiceRequestList.clear();
            manager.clearServiceRequests(channel, null);
            doDiscoverServices(false);
            //TODO We might not want to stop peer discovery in case we are advertising services.
            manager.stopPeerDiscovery(channel, null);
            mFindState = FindState.IDLE;
        }

        return OK;
    }

    public void discoveryChanged(int state) {
        Log.d(TAG, "discoveryChanged: " + state);

        // TODO: Any reason to restart discovery here?
        //doDiscoverServices(true);
    }

    /**
     * Advertise the existence of a well-known name to other (possibly
     * disconnected) AllJoyn daemons.
     *
     * @param name The well-known name to advertise.
     *
     * @param guid The globally unique identifier of the AllJoyn daemon
     *             associated with the well-known name.
     *
     * @return
     * <ul>
     * <li>O if the request to advertise the provided name was successful.</li>
     * <li>Other error status codes indicating a failure.</li>
     * </ul>
     */
    public int advertiseName(final String name, final String guid) {
        Log.d(TAG, "advertiseName(): " + name + ", " + guid + " _alljoyn._tcp");

        if (!isEnabled) {
            Log.d(TAG, "advertisedName(): P2P is OFF");
            return ERROR;
        }

        Map<String, String> txt = new HashMap<String, String>();
        int timer = 255;
        txt.put("GUID", guid);
        txt.put("TIMER", Integer.toString(timer));

        //TODO Need to implement arg cjeck for size. Deep in the bowels of JNI code is a hidden limitation:
        // command buffer size cannot exceed 256. Buffer includes string version of WPA supplicant command
        // plus actual payload...

        WifiP2pDnsSdServiceInfo serviceInfo = WifiP2pDnsSdServiceInfo.newInstance(name, "_alljoyn._tcp", txt);

        manager.addLocalService(channel, serviceInfo,
                                new ActionListener() {
                                    public void onSuccess() {
                                        Log.d(TAG, "AdvertiseName ( " + name + " ) success");
                                        // This is for keeping P2P discovery active
                                        if (mFindState != FindState.DISCOVERING)
                                            doFindPeers(true);
                                    }

                                    public void onFailure(int reasonCode) {
                                        Log.d(TAG, "AdvertiseName ( " + name + " ) fail. Reason : " + reasonCode);
                                    }
                                });


        return OK;
    }

    /**
     * Stop advertising the existence of the provided well-known name.
     *
     * @param name The well-known name to advertise.
     *
     * @param guid The globally unique identifier of the AllJoyn daemon
     *             associated with the well-known name.
     *
     * @return
     * <ul>
     * <li>O if the request to stop advertising the provided name was successful.</li>
     * <li>Other error status codes indicating a failure.</li>
     * </ul>
     */
    public int cancelAdvertiseName(final String name, String guid) {
        Log.d(TAG, "cancelAdvertiseName(" + name + ")");

        if (!isEnabled) {
            Log.d(TAG, "cancelAdvertisedName(): P2P is OFF");
            return ERROR;
        }

        Map<String, String> txt = new HashMap<String, String>();
        txt.put("GUID", guid);
        txt.put("TIMER", Integer.toString(0));

        final WifiP2pDnsSdServiceInfo serviceInfo = WifiP2pDnsSdServiceInfo.newInstance(name, "_alljoyn._tcp", txt);
        manager.addLocalService(channel, serviceInfo,
                                new ActionListener()
                                {
                                    public void onSuccess() {
                                        Log.d(TAG, "addLocalService (timer 0) success");
                                    }

                                    public void onFailure(int reasonCode) {
                                        Log.d(TAG, "removeLocalService (timer 0) failed: " + reasonCode);
                                    }
                                });

        Runnable removeService = new Runnable() {
            public void run() {
                if (!isEnabled /* || (mFindState == FindState.IDLE && not advertising)*/) {
                    return;
                }

                manager.removeLocalService(channel, serviceInfo,
                                           new ActionListener()
                                           {
                                               public void onSuccess() {
                                                   Log.d(TAG, "removeLocalService success");
                                               }

                                               public void onFailure(int reasonCode) {
                                                   Log.d(TAG, "removeLocalService failed: " + reasonCode);
                                               }
                                           });

                //TODO add housekeeping for advertised names.
                // Stop peer discovery if we are not advertising or looking for remote services.
                //Check that we are not in service discovery mode (as  a client)
                if (mFindState != FindState.DISCOVERING) {
                    Log.d(TAG, "Cancel peerdiscovery");
                    doFindPeers(false);
                }
            }
        };
        mHandler.postDelayed(removeService, MAX_ADVERTISE_TIMEOUT);

        return OK;
    }

    /**
     * Establish a Wi-Fi P2P connection to the device described by the
     * corresponding parameter.  The strength of desire to act as the group
     * owner is communicated via the groupOwnerIntent variable.
     *
     * It is assumed that if a call to this method returns success, there will
     * be a corresponding network interface available corresponding to the
     * resulting Wi-Fi Direct device, and the name of this interface will be
     * discoverable via a call to getInterfaceNameFromHandle().
     *
     * @param device The well-known name to advertise.
     *
     * @param groupOwnerIntent The strength of desire for the device to assume
     *                         the role of group owner.  Similar to GO Intent
     *                         as described in the Wi-Fi P2P spec, but a value
     *                         of fifteen here indicates an absolute requirement
     *                         to act as group owner, and a zero indicates an
     *                         absolute requirement to act as STA.
     *
     * @return
     * <ul>
     * <li>A positive handle describing the resulting connection if successful.</li>
     * <li>Other negative error status codes indicating a failure.</li>
     * </ul>
     */
    public int establishLink(String deviceAddress, int groupOwnerIntent) {
        Log.d(TAG, "establishLink(): " + deviceAddress);

        if (!isEnabled) {
            Log.d(TAG, "establishLink(): P2P is OFF");
            return ERROR;
        }

        if (peerState != PeerState.DISCONNECTED) {
            Log.e(TAG, "Already connected or in progress: " + peerState);
            return ERROR;
        }

        if (deviceAddress.isEmpty()) {

            if (device == null) {
                Log.e(TAG, "No local device known");
                return ERROR;
            }

            // Remove pending  calls
            mHandler.removeCallbacks(mRequestConnectionInfo);

            final int handle = getHandle(this.device.deviceAddress);

// This appears to be unnecessary. Would be needed for legacy WiFi compatibility (to appear as AP)
//            manager.createGroup(channel,
//                                new ActionListener()
//                                {
//                                    public void onSuccess() {
//                                        Log.d(TAG, "Created a P2P group.");
//                                    }
//
//                                    public void onFailure(int reasonCode) {
//                                        Log.d(TAG, "Creating a P2P group failed. Reason : " + reasonCode);
//                                        try {
//                                            busInterface.OnLinkError(handle, reasonCode);
//                                        } catch (BusException ex) {
//                                            Log.e(TAG, ex.getClass().getName());
//                                        }
//                                    }
//                                });
//
//            Log.d(TAG, "Group created");


            return handle;
        }

        peerConfig = new WifiP2pConfig();
        peerConfig.deviceAddress = deviceAddress;
        peerConfig.groupOwnerIntent = groupOwnerIntent;
        peerConfig.wps.setup = WpsInfo.PBC;
        peerState = PeerState.INITIATED;

        manager.connect(channel, peerConfig,
                        new ActionListener()
                        {
                            public void onSuccess() {
                                Log.d(TAG, "Connect initiated");
                                if (peerState == PeerState.INITIATED) {
                                    peerState = PeerState.CONNECTING;
                                }
                                Log.d(TAG, "Post delayed connection info request");
                                mHandler.postDelayed(mRequestConnectionInfo, connectionTimeout);
                            }

                            public void onFailure(int reasonCode) {
                                Log.d(TAG, "Connect failed. Reason : " + reasonCode);
                                int handle = getHandle(peerConfig.deviceAddress);
                                peerState = PeerState.DISCONNECTED;
                                peerConfig = null;
                                busInterface.OnLinkError(handle, reasonCode);
                            }
                        });
        Log.d(TAG, "establishLink(): Returning: " + getHandle(peerConfig.deviceAddress));
        return getHandle(peerConfig.deviceAddress);
    }

    /**
     * Tear down a no longer-required Wi-Fi P2P connection on the device
     * described by the handle parameter.
     *
     * @param handle A device handle previously returned by establishLink().
     *
     * @return
     * <ul>
     * <li>0 indicates that the connection has been torn down.</li>
     * <li>Other error status codes indicating a failure.</li>
     * </ul>
     */
    public int releaseLink(int handle) {
        Log.d(TAG, "releaseLink()");

        if (!isEnabled) {
            Log.d(TAG, "establishLink(): P2P is OFF");
            return ERROR;
        }

        switch (peerState) {
        case INITIATED:
        case CONNECTING:
            peerState = PeerState.DISCONNECTING;
            manager.cancelConnect(channel,
                                  new ActionListener()
                                  {
                                      public void onSuccess() {
                                          Log.d(TAG, "Canceling connect");
                                          peerState = PeerState.DISCONNECTED;
                                          peerConfig = null;
                                      }

                                      public void onFailure(int reasonCode) {
                                          Log.d(TAG, "Remove failed. Reason : " + reasonCode);

                                          /* Most likely failure is if the connection completed already,
                                           * but in that case onConnectionInfoAvailable would have called
                                           * removeGroup() and marked the peer as disconnected.  No harm
                                           * in doing it again.
                                           */
                                          peerState = PeerState.DISCONNECTED;
                                          peerConfig = null;
                                      }
                                  });

            break;

        case CONNECTED:
            // OnLinkLost will be sent via onConnectionInfoAvailable()
            peerState = PeerState.DISCONNECTING;
            manager.removeGroup(channel,
                                new ActionListener()
                                {
                                    public void onSuccess() {
                                        Log.d(TAG, "Remove initiated");
                                    }

                                    public void onFailure(int reasonCode) {
                                        Log.d(TAG, "Remove failed. Reason : " + reasonCode);
                                        int handle = 0;
                                        if (peerConfig != null) {
                                            handle = getHandle(peerConfig.deviceAddress);
                                        }
                                        peerState = PeerState.DISCONNECTED;
                                        peerConfig = null;
                                        if (handle != 0) {
                                            busInterface.OnLinkError(handle, reasonCode);
                                        }
                                    }
                                });
            break;

        default:
            Log.e(TAG, "No link to release");
            break;
        }

        return OK;
    }

    /**
     * Discover which network interface corresponds to a handle value returned
     * from establishlink().
     *
     * Useful for determining which interfaces to add in order to listen for
     * AllJoyn multicast name service packets.
     *
     * @param handle A device handle previously returned by establishLink().
     *
     * @return
     * <ul>
     * <li>A String containing the interface name (cf. wlan0)
     * <li>null indicating an interface name is not available for the given handle.</li>
     * </ul>
     */
    public String getInterfaceNameFromHandle(int handle) {
        Log.d(TAG, "getInterfaceNameFromHandle()");

        try {
            Enumeration<NetworkInterface> list = NetworkInterface.getNetworkInterfaces();

            while (list != null&& list.hasMoreElements()) {
                NetworkInterface nIface = list.nextElement();
                byte[] macAddr = nIface.getHardwareAddress();

                if (macAddr == null)
                    continue;

                StringBuilder macStr = new StringBuilder();
                for (int i = 0; i < macAddr.length; i++) {
                    macStr.append(String.format("%02x%s", macAddr[i], (i < macAddr.length - 1) ? ":" : ""));
                }

                if (this.device != null&& this.device.deviceAddress.equals(macStr.toString())) {
                    Log.d(TAG, "getInterfaceNameFromHandle(): Returning: " + nIface.getDisplayName());
                    return nIface.getDisplayName();
                }
            }
        } catch (SocketException e) {
            Log.e(TAG, "Could not get network interface name");
            return null;
        }

        return null;
    }

    public void setDevice(WifiP2pDevice device) {
        this.device = device;
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            //TODO We probably don't want to clear ALL requests.
            // Investigate whether this clears the requests from oother entities.
            // Maybe just clear the outstanding requests based on mServiceRequestList
            // plus the general one for "_alljoyn._tcp" type of services (this is also questionable).
            Log.d(TAG, "Finalize");
            manager.clearServiceRequests(channel, null);
            doDiscoverServices(false);
            manager.stopPeerDiscovery(channel, null);
        } finally {
            super.finalize();
        }
    }
}
