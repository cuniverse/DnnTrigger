package com.diotek.stt.trgandroidtester;

import android.Manifest;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder.AudioSource;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.NotificationCompat;
import android.util.AndroidRuntimeException;
import android.util.Log;

import com.selvasai.stt.selvywakeup.WakeUpSolid;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.Serializable;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.text.SimpleDateFormat;
import java.util.Date;


/**
 * Created by mckeum on 2016-08-11.
 * Audio recorder for DNN Trigger
 */

/* Note: Using API>22 will require additional permission request on runtime
 * https://developer.android.com/training/permissions/requesting.html
 */

// Get audio using AudioRecord and detect keyword using WakeUpSolid
// AudioRecord source from http://stackoverflow.com/a/4827945/3720866
class TriggerDetector extends Thread {
    private final static String TAG = "TriggerDetector";
    private boolean stopped = false;
    private boolean saveSpotFlag = false;

    private final String logRootPath = Environment.getExternalStorageDirectory().getAbsolutePath()
            + File.separator + "diotek" + File.separator + "okselva";
    private final SimpleDateFormat df = new SimpleDateFormat("yyMMdd_HHmmss");
    private BufferedOutputStream pcmOut;
    private Object pcmOutLock = new Object();
    private WakeUpSolid trgInstance;

    private Context mContext;
    private Callback cb;

    boolean isAmp2x = false;
    boolean isComm = false;

    short[][] buffers = new short[256][160];
    private int ix = 0;

    interface Callback extends Serializable {
        void callback(String txt);

        void rms(short value, short min);
    }

    TriggerDetector(Context context, Callback cb) {
        this.cb = cb;
        mContext = context;

        try {
            trgInstance = WakeUpSolid.I(mContext);
        } catch (WakeUpSolid.CreateEngineErrorException e) {
            throw new AndroidRuntimeException(e);
        }

        File rootDir = new File(logRootPath);
        if (!rootDir.exists())
            rootDir.mkdirs();
    }

    boolean startPcmLog() {
        stopPcmLog();

        String timestamp = df.format(new Date());
        String pcmPath = logRootPath + File.separator + timestamp + ".pcm";

        synchronized (pcmOutLock) {
            try {
                pcmOut = new BufferedOutputStream(new FileOutputStream(pcmPath));
                this.cb.callback("Start saving PCM data to " + pcmPath);
            } catch (FileNotFoundException e) {
                e.printStackTrace();
                return false;
            }
        }
        return true;
    }

    void stopPcmLog() {
        synchronized (pcmOutLock) {
            if (null == pcmOut) return;
            try {
                pcmOut.close();
                this.cb.callback("Finished saving PCM file");
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                pcmOut = null;
            }
        }
    }

    // 검출시 오디오 버퍼 저장
    void setSpotLog(boolean flag) {
        saveSpotFlag = flag;
    }

    boolean saveBuffer() {
        String timestamp = df.format(new Date());
        String pcmPath = logRootPath + File.separator + timestamp + "-oksv.pcm";
        byte[] byteBuf = new byte[2 * buffers[0].length];

        try {
            BufferedOutputStream spotOut = new BufferedOutputStream(new FileOutputStream(pcmPath));

            for (int b = 0; b < buffers.length; b++) {
                short[] shortBuf = buffers[(ix + b) % buffers.length];
                ByteBuffer.wrap(byteBuf).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer().put(shortBuf);
                spotOut.write(byteBuf);
            }
            spotOut.close();
            this.cb.callback("Spot saved to " + pcmPath);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return false;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }

        return true;
    }

    @Override
    public void run() {
        trgInstance.reset();
        int _f_count = 0;
        android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);
        AudioRecord recorder = null;
        try {
            int N = AudioRecord.getMinBufferSize(16000, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT);
            Log.i(TAG, "min buffer size " + N);
            if (AudioRecord.ERROR_BAD_VALUE == N) {
                this.cb.callback("ERROR: Device does not supports 16kHz audio recording!");
                return;
            }

//	        recorder = new AudioRecord(AudioSource.MIC, 16000,
//			        AudioFormat.CHANNEL_IN_MONO,
//			        AudioFormat.ENCODING_PCM_16BIT, N * 16);
            if (ActivityCompat.checkSelfPermission(this.mContext, Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
                return;
            }
            recorder = new AudioRecord(isComm ? AudioSource.VOICE_COMMUNICATION : AudioSource.VOICE_RECOGNITION
                    , 16000,
                    AudioFormat.CHANNEL_IN_MONO,
                    AudioFormat.ENCODING_PCM_16BIT, N * 16);
            recorder.startRecording();
	        CreateNoti();
            this.cb.callback("Detecting...");
	        changeNoti("Detecting...");
            if (saveSpotFlag)
                this.cb.callback("Save spot enabled");
            while (!stopped) {
                short[] buffer = buffers[ix % buffers.length];
                N = recorder.read(buffer, 0, buffer.length);
                if (N <= 0) continue;
                ix++;

                // get max, min for visualization
                short maxAmplitude = Short.MIN_VALUE;
                short minAmplitude = Short.MAX_VALUE;
                for (int s = 0; s < N; s++)
                    if (maxAmplitude < buffer[s]) maxAmplitude = buffer[s];
                for (int s = 0; s < N; s++)
                    if (minAmplitude > buffer[s]) minAmplitude = buffer[s];
                this.cb.rms(maxAmplitude, minAmplitude);

                // log pcm
                synchronized (pcmOutLock) {
                    if (null != pcmOut) {
                        byte[] byteBuf = new byte[2 * N];
                        ByteBuffer.wrap(byteBuf).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer().put(buffer);
                        pcmOut.write(byteBuf);
                    }
                }

                // word detection
	            short[] buffer_input = buffer;
	            if(isAmp2x){
		            buffer_input = new short[buffer.length];
		            for (short sample_idx = 0; sample_idx < buffer.length; sample_idx++) {
			            buffer_input[sample_idx] = (short) (buffer[sample_idx] * 2);
		            }
	            }
	            int info[] = new int[2];
                int det = trgInstance.detect(buffer_input,info);
	            _f_count += (buffer_input.length/160);
	            String _log_str = String.format("[%08d] det: %d : %d / %d",_f_count,det,info[0],info[1]);
	            if(det!=-1) Log.d("cuniverse",_log_str);
                if (det <= 0)   // not detected
                    continue;
	            changeNoti("Detected Trigger!");
                // detected
                this.cb.callback("Detected at " + det + " frame!");
                if (saveSpotFlag)
                    saveBuffer();
            }

            recorder.stop();
	        changeNoti("Ready");
            this.cb.callback("Ready");
        } catch (Throwable x) {
            Log.w(TAG, "Error reading voice audio", x);
            this.cb.callback("Error!");
        } finally {
            Log.d(TAG, "finally");
            if (recorder != null) {
                recorder.release();
            }
	        NotificationManager nm = (NotificationManager)mContext.getSystemService(Context.NOTIFICATION_SERVICE);
            nm.cancel(R.string.app_name);
        }
        stopPcmLog();
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

	void changeNoti(String content){
		if(builder != null) {
			//if(title!=null && title.isEmpty()==false) builder.setContentTitle(title);
			if(content!=null && content.isEmpty()==false)builder.setContentText(content);

			NotificationManager nm = (NotificationManager)mContext.getSystemService(Context.NOTIFICATION_SERVICE);
			nm.notify(R.string.app_name, builder.build());
		}
	}
}
