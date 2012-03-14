/*
 * Copyright 2010-2012, Qualcomm Innovation Center, Inc.
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
package org.alljoyn.bus.bundle.tests.clientbundle;

import android.app.Activity;
import android.app.Dialog;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class DialogBuilder {

	private static final String TAG = "client.Dialogs";

	private TextView mFailReason;

    public Dialog createDebugDialog(final Activity activity, final String reason)
    {

    	Log.i(TAG, "createDebugDialog()");
    	final Dialog dialog = new Dialog(activity);
    	dialog.requestWindowFeature(dialog.getWindow().FEATURE_NO_TITLE);

    	dialog.setContentView(R.layout.debug);

    	mFailReason = (TextView) dialog.findViewById(R.id.failReason);
    	mFailReason.setText(reason);

    	Button cancel = (Button)dialog.findViewById(R.id.clientCancel);
    	cancel.setOnClickListener(new View.OnClickListener() {
    		public void onClick(View view) {
				/*
				 * Android likes to reuse dialogs for performance reasons.
				 * Tell the Android
				 * application framework to forget about this dialog completely.
				 */
    			activity.removeDialog(Client.DIALOG_DEBUG_ID);
    		}
    	});

    	return dialog;
    }
}
