/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
package org.openni.android.tools.niviewer;

import java.util.ArrayList;
import java.util.List;

import org.openni.DeviceInfo;

import android.app.AlertDialog;
import android.content.DialogInterface;

public class DeviceSelectDialog implements DialogInterface.OnClickListener {
	protected List<String> mDeviceURIs;
	NiViewerActivity mViewerActivity;

	public void showDialog(List<DeviceInfo> devices, int activeDeviceID,
			NiViewerActivity viewerActivity) {

		mViewerActivity = viewerActivity;
		int devicesCount = devices.size();

		CharSequence[] deviceNames = new CharSequence[devicesCount];
		mDeviceURIs = new ArrayList<String>(devicesCount);
		for (int i = 0; i < devices.size(); i++) {
			deviceNames[i] = devices.get(i).getName() + "(" + devices.get(i).getUri() + ")";
			mDeviceURIs.add(devices.get(i).getUri());
		}

		AlertDialog.Builder builder = new AlertDialog.Builder(viewerActivity);
		builder.setTitle("Pick Device");
		builder.setNegativeButton("Cancel",
				new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int id) {
					}
				});

		builder.setSingleChoiceItems(deviceNames, activeDeviceID, this);

		builder.create().show();
	}

	@Override
	public void onClick(DialogInterface dialog, int which) {
		String uri;

		uri = mDeviceURIs.get(which);

		mViewerActivity.openDevice(uri);
		
		dialog.cancel();
	}
}
