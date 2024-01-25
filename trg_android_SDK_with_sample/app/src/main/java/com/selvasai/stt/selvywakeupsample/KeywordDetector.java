package com.selvasai.stt.selvywakeupsample;

import android.Manifest;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder.AudioSource;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.NotificationCompat;
import android.util.AndroidRuntimeException;
import android.util.Log;

import com.selvasai.stt.selvywakeup.WakeUpSolid;

import java.io.Serializable;


/**
 * Created by mckeum on 2016-08-11.
 * Get audio using AudioRecord and detect keyword using WakeUpSolid
 */

class KeywordDetector extends Thread {
    private final static String TAG = "TriggerDetector";
    private boolean stopped = false;

    private WakeUpSolid wakeupInstance;
    private Callback cb;

    private short[][] buffers = new short[256][160];
    private int ix = 0;

    interface Callback extends Serializable {
        void callback(String txt);
    }

    Context mContext;

    KeywordDetector(Context context, Callback callback) {
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
                cb.callback("ERROR: RECORD_AUDIO permission required!");
                return;
            }
            recorder = new AudioRecord(AudioSource.MIC, 16000,
                    AudioFormat.CHANNEL_IN_MONO,
                    AudioFormat.ENCODING_PCM_16BIT, N * 16);
            recorder.startRecording();
	        CreateNoti();
            cb.callback("검출중...");
	        changeNoti(null,"검출중...");
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
	            changeNoti(null,cbString);
            }

            recorder.stop();
	        changeNoti(null,"중지됨");
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

    void close() {
        Log.d(TAG, "close()");
        stopped = true;
    }
	NotificationCompat.Builder builder = null;
    void CreateNoti() {
    	if(builder == null) {
		    builder = new NotificationCompat.Builder(mContext)
				    .setContentText("트리거 실행중입니다.") //mContext.getString(R.string.notif_text)
				    .setContentTitle("SelvasAI KeyWordDetector")
				    .setSmallIcon(R.mipmap.ic_launcher)
				    .setAutoCancel(false)
				    .setOngoing(true) //ongoing
				    .setOnlyAlertOnce(true)
				    .setContentIntent(
						    PendingIntent.getActivity(mContext, 10,
								    new Intent(mContext, MainActivity.class)
										    .addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP),
								    0)
				    )
//				.addAction(running ? R.drawable.ic_action_pause
//								: R.drawable.ic_action_play,
//						running ? context.getString(R.string.pause)
//								: context.getString(R.string.start),
//						startPendingIntent)
		    //.addAction(R.drawable.ic_action_stop, context.getString(R.string.stop), stopPendingIntent)
		    ;
	    }
	    NotificationManager nm = (NotificationManager)mContext.getSystemService(Context.NOTIFICATION_SERVICE);
	    nm.notify(R.string.app_name, builder.build());

	}

	void changeNoti(String title,String content){
		if(builder != null) {
			if(title!=null && title.isEmpty()==false) builder.setContentTitle(title);
			if(content!=null && content.isEmpty()==false)builder.setContentText(content);

			NotificationManager nm = (NotificationManager)mContext.getSystemService(Context.NOTIFICATION_SERVICE);
			nm.notify(R.string.app_name, builder.build());
		}
	}


}
