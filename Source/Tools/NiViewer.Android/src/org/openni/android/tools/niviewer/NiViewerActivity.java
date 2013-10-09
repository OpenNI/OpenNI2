package org.openni.android.tools.niviewer;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.concurrent.TimeoutException;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.hardware.usb.UsbDeviceConnection;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.TextView;

import org.openni.Device;
import org.openni.DeviceInfo;
import org.openni.OpenNI;
import org.openni.Recorder;
import org.openni.SensorType;
import org.openni.VideoFrameRef;
import org.openni.VideoMode;
import org.openni.VideoStream;
import org.openni.android.OpenNIHelper;
import org.openni.android.OpenNIView;

public class NiViewerActivity 
		extends Activity 
		implements OpenNIHelper.DeviceOpenListener {
	
	private static final String TAG = "NiViewer";
	private OpenNIHelper mOpenNIHelper;
	private UsbDeviceConnection mDeviceConnection;
	private boolean mDeviceOpenPending = false;
	private Thread mMainLoopThread;
	private boolean mShouldRun = true;
	private Device mDevice;
	private VideoStream mStream;
	private Recorder mRecorder;
	private String mRecordingName;
	private Spinner mSensorSpinner;
	private List<SensorType> mDeviceSensors;
	private List<VideoMode> mStreamVideoModes;
	private Spinner mVideoModeSpinner;
	private OpenNIView mFrameView;
	private TextView mStatusLine;
	private String mRecording;
	private static SensorType[] SENSORS = { SensorType.DEPTH, SensorType.COLOR, SensorType.IR };
	private static CharSequence[] SENSOR_NAMES = { "Depth", "Color", "IR" };

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		mOpenNIHelper = new OpenNIHelper(this);
		OpenNI.setLogAndroidOutput(true);
		OpenNI.setLogMinSeverity(0);
		OpenNI.initialize();
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_simple_viewer);
		mSensorSpinner = (Spinner) findViewById(R.id.spinnerSensor);
		mVideoModeSpinner = (Spinner) findViewById(R.id.spinnerVideoMode);
		mFrameView = (OpenNIView) findViewById(R.id.frameView);
		mStatusLine = (TextView) findViewById(R.id.status_line);
		
		mSensorSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
			@Override
			public void onItemSelected(AdapterView<?> parent, View view,
					int position, long id) {
				onSensorSelected(position);
			}

			@Override
			public void onNothingSelected(AdapterView<?> parent) {
			}
		});

		mVideoModeSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {
			@Override
			public void onItemSelected(AdapterView<?> parent, View view,
					int position, long id) {
				onVideoModeSelected(position);
			}

			@Override
			public void onNothingSelected(AdapterView<?> parent) {
			}
		});
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.simple_viewer, menu);
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
			case R.id.record:
				toggleRecording(item);
				return true;
			default:
				return super.onOptionsItemSelected(item);
		}
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		
		mOpenNIHelper.shutdown();
		OpenNI.shutdown();
	}
	
	@Override 
	protected void onStart() {
		Log.d(TAG, "onStart");
		super.onStart();
		
		final android.content.Intent intent = getIntent ();

		if (intent != null) {
			final android.net.Uri data = intent.getData ();
			if (data != null) {
				mRecording = data.getEncodedPath ();
				Log.d(TAG, "Will open file " + mRecording);
			}
		}
	}

	@Override
	protected void onResume() {
		Log.d(TAG, "onResume");

		super.onResume();

		// onResume() is called after the USB permission dialog is closed, in which case, we don't want
		// to request device permissions again
		if (mDeviceOpenPending) {
			return;
		}

		// Request opening the first OpenNI-compliant device found
		String uri;
		
		if (mRecording != null) {
			uri = mRecording;
		} else {
			List<DeviceInfo> devices = OpenNI.enumerateDevices();
			if (devices.isEmpty()) {
				showAlertAndExit("No OpenNI-compliant device found.");
				return;
			}
			uri = devices.get(0).getUri();
		}
		
		mDeviceOpenPending = true;
		mOpenNIHelper.requestDeviceOpen(uri, this);
	}

	private void showAlert(String message) {
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setMessage(message);
		builder.show();
	}
	
	private void showAlertAndExit(String message) {
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setMessage(message);
		builder.setNeutralButton("OK", new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				finish();
			}
		});
		builder.show();
	}

	@Override
	public void onDeviceOpened(Device aDevice) {
		Log.d(TAG, "Permission granted for device " + aDevice.getDeviceInfo().getUri());
		
		mDeviceOpenPending = false;

		mDevice = aDevice;
		mDeviceSensors = new ArrayList<SensorType>();

		List<CharSequence> sensors = new ArrayList<CharSequence>();
		
		for (int i = 0; i < SENSORS.length; ++i) {
			if (mDevice.hasSensor(SENSORS[i])) {
				sensors.add(SENSOR_NAMES[i]);
				mDeviceSensors.add(SENSORS[i]);
			}
		}
		
		ArrayAdapter<CharSequence> sensorsAdapter = new ArrayAdapter<CharSequence>(this,
				android.R.layout.simple_spinner_item, sensors);
		sensorsAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
		mSensorSpinner.setAdapter(sensorsAdapter);
		
		mSensorSpinner.setSelection(0);
	}
	
	private void onSensorSelected(int pos) {
		try {
			stop();
			
			if (mStream != null) {
				mStream.destroy();
				mStream = null;
			}
		
			mStream = VideoStream.create(mDevice, mDeviceSensors.get(pos));
			List<CharSequence> videoModesNames = new ArrayList<CharSequence>();
	
			mStreamVideoModes = mStream.getSensorInfo().getSupportedVideoModes();
			for (int i = 0; i < mStreamVideoModes.size(); ++i) {
				VideoMode mode = mStreamVideoModes.get(i);
				
				videoModesNames.add(String.format("%d x %d @ %d FPS (%s)",
	                mode.getResolutionX(),
	                mode.getResolutionY(), 
	                mode.getFps(),
	                pixelFormatToName(mode.getPixelFormat())));
			}
	
			ArrayAdapter<CharSequence> videoModesAdapter = new ArrayAdapter<CharSequence>(this,
					android.R.layout.simple_spinner_item, videoModesNames);
			videoModesAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
			mVideoModeSpinner.setAdapter(videoModesAdapter);
	
			// use default mode
			VideoMode currentMode = mStream.getVideoMode();
			int selected = mStreamVideoModes.indexOf(currentMode);
			if (selected == -1) {
				selected = 0;
			}
			
			mVideoModeSpinner.setSelection(selected);
		} catch (RuntimeException e) {
			showAlert("Failed to switch to stream: " + e.getMessage());
		}
	}
	
    private CharSequence pixelFormatToName(org.openni.PixelFormat format) {
        switch (format) {
            case DEPTH_1_MM:    return "1 mm";
            case DEPTH_100_UM:  return "100 um";
            case SHIFT_9_2:     return "9.2";
            case SHIFT_9_3:     return "9.3";
            case RGB888:        return "RGB";
            case GRAY8:         return "Gray8";
            case GRAY16:        return "Gray16";
            case YUV422:		return "YUV422";
            case YUYV:			return "YUYV";
            default:            return "UNKNOWN";
        }
    }
	
	private void onVideoModeSelected(int pos) {
		try {
			stop();
			
			mStream.setVideoMode(mStreamVideoModes.get(pos));
			mStream.start();
		} catch (RuntimeException e) {
			showAlert("Failed to switch to video mode: " + e.getMessage());
		}

		mShouldRun = true;
		mMainLoopThread = new Thread() {
			@Override
			public void run() {
				List<VideoStream> streams = new ArrayList<VideoStream>();
				streams.add(mStream);
				
				while (mShouldRun) {
					VideoFrameRef frame = null;
					
					try {
						OpenNI.waitForAnyStream(streams, 100);
						frame = mStream.readFrame();
						
						// Request rendering of the current OpenNI frame
						mFrameView.update(frame);
						updateLabel(String.format("Frame Index: %,d | Timestamp: %,d", frame.getFrameIndex(), frame.getTimestamp()));
						
					} catch (TimeoutException e) {
					} catch (Exception e) {
						Log.e(TAG, "Failed reading frame: " + e);
					}
				}
			};
		};
		
		mMainLoopThread.setName("SimpleViewer MainLoop Thread");
		mMainLoopThread.start();
	}
	
	private void stop() {
		mShouldRun = false;
		
		while (mMainLoopThread != null) {
			try {
				mMainLoopThread.join();
				mMainLoopThread = null;
				break;
			} catch (InterruptedException e) {
			}
		}

		if (mStream != null) {
			mStream.stop();
		}
		
		mFrameView.clear();
		mStatusLine.setText(R.string.waiting_for_frames);
	}
	
	@SuppressLint("SimpleDateFormat")
	private void toggleRecording(MenuItem item) {
		if (mRecorder == null) {
			mRecordingName = Environment.getExternalStorageDirectory().getPath() +
					"/" + new SimpleDateFormat("yyyyMMddHHmmss").format(new Date()) + ".oni";
			
			try {
				mRecorder = Recorder.create(mRecordingName);
				mRecorder.addStream(mStream, true);
				mRecorder.start();
			} catch (RuntimeException ex) {
				mRecorder = null;
				showAlert("Failed to start recording: " + ex.getMessage());
				return;
			}
			
			item.setTitle(R.string.stop_record);
		} else {
			stopRecording();
			item.setTitle(R.string.start_record);
		}
	}
	
	private void stopRecording() {
		if (mRecorder != null) {
			mRecorder.stop();
			mRecorder.destroy();
			mRecorder = null;

			showAlert("Recording saved to: " + mRecordingName);
			mRecordingName = null;
		}
	}
	
	private void updateLabel(final String message) {
		runOnUiThread(new Runnable() {
			public void run() {
				mStatusLine.setText(message);								
			}
		});
	}

	@Override
	public void onDeviceOpenFailed(String uri) {
		Log.e(TAG, "Failed to open device " + uri);
		mDeviceOpenPending = false;
		showAlertAndExit("Failed to open device");
	}

	@Override
	protected void onPause() {
		Log.d(TAG, "onPause");

		super.onPause();
		
		// onPause() is called just before the USB permission dialog is opened, in which case, we don't
		// want to shutdown OpenNI
		if (mDeviceOpenPending)
			return;

		stop();
		
		stopRecording();
		
		if (mStream != null) {
			mStream.destroy();
			mStream = null;
		}
		
		if (mDevice != null) {
			mDevice.close();
			mDevice = null;
		}

		if (mDeviceConnection != null) {
			mDeviceConnection.close();
			mDeviceConnection = null;
		}
	}
}
