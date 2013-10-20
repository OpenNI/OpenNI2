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
