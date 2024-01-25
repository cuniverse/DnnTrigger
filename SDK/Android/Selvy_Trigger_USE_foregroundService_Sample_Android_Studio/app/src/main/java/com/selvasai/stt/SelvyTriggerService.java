package com.selvasai.stt;

import android.Manifest;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.ResultReceiver;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.NotificationCompat;
import android.util.AndroidRuntimeException;
import android.util.Log;
import android.widget.Toast;

import com.selvasai.stt.selvywakeup.WakeUpSolid;
import com.selvasai.stt.selvywakeupsample.MainActivity;
import com.selvasai.stt.selvywakeupsample.R;

import java.io.Serializable;

public class SelvyTriggerService extends Service {

	public static final String ACTION_START_FOREGROUND_SERVICE = "ACTION_START_FOREGROUND_SERVICE";
	public static final String ACTION_STOP_FOREGROUND_SERVICE = "ACTION_STOP_FOREGROUND_SERVICE";


	Handler mHandler = new Handler();
//	Intent mIntent = null;
	ResultReceiver mResultReceiver = null;
	NotificationCompat.Builder builder = null;
//	Class<?> aClass;

	public SelvyTriggerService() {
	}

	@Override
	public IBinder onBind(Intent intent) {
		// TODO: Return the communication channel to the service.
		throw new UnsupportedOperationException("Not yet implemented");
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		if (intent != null) {
			String action = intent.getAction();

			switch (action) {
				case ACTION_START_FOREGROUND_SERVICE:
					Bundle bundle = new Bundle();
					mResultReceiver = intent.getParcelableExtra("RECEIVER"); // (1)
//					mIntent = intent.getParcelableExtra("TARGET"); // (1)

					startForegroundService();
					Toast.makeText(getApplicationContext(), "Foreground service is started.", Toast.LENGTH_LONG).show();
					break;
				case ACTION_STOP_FOREGROUND_SERVICE:
					stopForegroundService();
					Toast.makeText(getApplicationContext(), "Foreground service is stopped.", Toast.LENGTH_LONG).show();
					break;
			}
		}
		return super.onStartCommand(intent, flags, startId);
	}


	void startForegroundService() {
		if (StartDetection()) {
			if (builder == null) {
				builder = new NotificationCompat.Builder(this)
						.setContentText("트리거 실행중입니다.") //mContext.getString(R.string.notif_text)
						.setContentTitle("SelvasAI KeyWordDetector")
						.setSmallIcon(R.mipmap.ic_launcher)
						.setAutoCancel(true)
						.setOngoing(true); //ongoing

				Intent notificationIntent = new Intent(this, MainActivity.class)
						.setFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
				PendingIntent pendingIntent = PendingIntent.getActivity(this, 10, notificationIntent, 0);
				builder.setContentIntent(pendingIntent);
			}
			startForeground(R.string.app_name, builder.build());
		}
	}


	void stopForegroundService() {
		stopForeground(true);
		StopDetection();
		// Stop the foreground service.
		stopSelf();
	}


	KeywordDetectorThread.Callback kdcb = new KeywordDetectorThread.Callback() {
		@Override
		public void callback(final String txt) {
			Bundle bundle = new Bundle();
			bundle.putString("DetectMSG", txt);
			mResultReceiver.send(1, bundle);
			changeNoti(null, txt);
		}
	};


	private KeywordDetectorThread detectingThread;

	boolean StartDetection() {
		if (detectingThread != null) {
//			StopDetection();
			return false;
		}
		try {
			detectingThread = new KeywordDetectorThread(getApplicationContext(), kdcb);
			detectingThread.start();

			Toast.makeText(SelvyTriggerService.this, "Detection started!", Toast.LENGTH_LONG).show();
			return true;
		} catch (Exception e) {
			Toast.makeText(SelvyTriggerService.this, "Engine Create Error Check Android LOG and Error CODE!!", Toast.LENGTH_LONG).show();
			return false;
		}
	}


	void StopDetection() {

		// Stop detection
		if (null == detectingThread) return;
		detectingThread.close();
		try {
			detectingThread.join();
		} catch (Exception e) {
			e.printStackTrace();
		}
		detectingThread = null;
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		//stopForeground(true);
		StopDetection();
	}


	void changeNoti(String title, String content) {
		if (builder != null) {
			if (title != null && title.isEmpty() == false) builder.setContentTitle(title);
			if (content != null && content.isEmpty() == false) builder.setContentText(content);

			NotificationManager nm = (NotificationManager) SelvyTriggerService.this.getSystemService(Context.NOTIFICATION_SERVICE);
			nm.notify(R.string.app_name, builder.build());
		}
	}



	static public void startDetection(Context c, ResultReceiver resultReceiver) {
		Intent intent = new Intent(c, SelvyTriggerService.class);
		intent.putExtra("RECEIVER", resultReceiver);
		intent.setAction(SelvyTriggerService.ACTION_START_FOREGROUND_SERVICE);
		c.startService(intent);
	}


	static public boolean isServiceRunningCheck(Context c) {
		ActivityManager manager = (ActivityManager) c.getSystemService(MainActivity.ACTIVITY_SERVICE);
		for (ActivityManager.RunningServiceInfo service : manager.getRunningServices(Integer.MAX_VALUE)) {
			if ("com.selvasai.stt.SelvyTriggerService".equals(service.service.getClassName())) {
				return true;
			}
		}
		return false;
	}
	static public void stopDetection(Context c){
		Intent intent = new Intent(c, SelvyTriggerService.class);
		intent.setAction(SelvyTriggerService.ACTION_STOP_FOREGROUND_SERVICE);
		//stopService(intent)
		c.startService(intent);
	}

}

class KeywordDetectorThread extends Thread {
	private final static String TAG = "TriggerDetector";
	private boolean stopped = false;

	private WakeUpSolid wakeupInstance;
	private Callback cb;

	private short[][] buffers = new short[256][160];
	private int ix = 0;

	public interface Callback extends Serializable {
		void callback(String txt);
	}

	Context mContext;

	public KeywordDetectorThread(Context context, Callback callback) {
		cb = callback;
		mContext = context;
		// Get singleton instance of WakeUpSolid
		try {
			wakeupInstance = WakeUpSolid.I(context);
		} catch (WakeUpSolid.CreateEngineErrorException e) {
			e.printStackTrace();
			throw new AndroidRuntimeException(e);
		}
		setPriority(MAX_PRIORITY);
	}

	@Override
	public void run() {
		// Reset WakeUpSolid
		wakeupInstance.reset();

		android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);
		AudioRecord recorder = null;
		try {
			int N = AudioRecord.getMinBufferSize(16000, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT);
			if (AudioRecord.ERROR_BAD_VALUE == N) {
				cb.callback("ERROR: Device does not supports 16kHz audio recording!");
				return;
			}

			if (ActivityCompat.checkSelfPermission(mContext, Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
				// TODO: Consider calling
				//    ActivityCompat#requestPermissions
				// here to request the missing permissions, and then overriding
				//   public void onRequestPermissionsResult(int requestCode, String[] permissions,
				//                                          int[] grantResults)
				// to handle the case where the user grants the permission. See the documentation
				// for ActivityCompat#requestPermissions for more details.
				return;
			}
			recorder = new AudioRecord(MediaRecorder.AudioSource.MIC, 16000,
					AudioFormat.CHANNEL_IN_MONO,
					AudioFormat.ENCODING_PCM_16BIT, N * 16);
			recorder.startRecording();
			//CreateNoti();
			cb.callback("검출중...");
			//changeNoti(null,"검출중...");
			int _f_count = 0;
			while (!stopped) {
				short[] buffer = buffers[ix % buffers.length];
				N = recorder.read(buffer, 0, buffer.length);
				if (N <= 0) continue;
				ix++;

				// WakeUpSolid word detection
				//int det = wakeupInstance.detect(buffer);
				int _frame_info[] = new int[2];
				int det = wakeupInstance.detect(buffer,_frame_info);
				_f_count += (buffer.length/160);
				if(det>=0) Log.d("cuniverse",String.format(">> [%08d] det: %d : %d / %d",_f_count,det,_frame_info[0],_frame_info[1]));
				if (det <= 0)   // not detected
					continue;

				// detected
				String cbString = "Detected at " + det + " frame! Trigger Started at " + (_frame_info[0] - _frame_info[1]) + " frame ";
				cb.callback(cbString);
				//changeNoti(null,cbString);
			}

			recorder.stop();
			//changeNoti(null,"중지됨");
			cb.callback("중지됨");
		} catch (Throwable x) {
			Log.w(TAG, "Error reading voice audio", x);
			cb.callback("Error!");
		} finally {
			Log.d(TAG, "finally");
			if (recorder != null) {
				recorder.release();
			}
			NotificationManager nm = (NotificationManager)mContext.getSystemService(Context.NOTIFICATION_SERVICE);
			nm.cancel(R.string.app_name);
		}
	}

	public void close() {
		Log.d(TAG, "close()");
		stopped = true;
	}
}