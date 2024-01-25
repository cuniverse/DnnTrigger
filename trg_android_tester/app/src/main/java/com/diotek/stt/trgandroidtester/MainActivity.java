package com.diotek.stt.trgandroidtester;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.ActivityManager;
import android.app.KeyguardManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.PowerManager;
import android.support.annotation.NonNull;
import android.support.design.widget.Snackbar;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.text.Html;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import com.selvasai.stt.selvywakeup.WakeUpSolid;

import java.text.SimpleDateFormat;
import java.util.Date;


public class MainActivity extends AppCompatActivity implements AudioManager.OnAudioFocusChangeListener{
    private final static String TAG = "Trg Tester Main";
    private final static String PREFS_NAME = "TrgPrefs";

    private TriggerDetector td;

    private Menu menu;
    ScrollView svLog;
    TextView tvRecording;
    TextView tvSampleText;
	AutoResizeTextView tv_trig;
	AutoResizeTextView tv_Listen;

    //ShowRecWave waveCircle;
    ShowWaveform waveForm;

    boolean saveSpotFlag = false;
    boolean keepOnFlag = false;
    boolean showLogFlag = false;
    boolean pcmRecordingFlag = false;

    private final SimpleDateFormat timeFormat = new SimpleDateFormat("HH:mm:ss");
    private final SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
    int trgCount = 0;

    Handler mHandler = new Handler();

	Runnable m_auto_hide_Runnable=new Runnable() {
		@Override
		public void run() {
			tv_Listen.setVisibility(View.VISIBLE);
			tv_trig.setVisibility(View.INVISIBLE);
		}
	};


	Runnable m_auto_AudioFocus_hide_Runnable=new Runnable() {
		@Override
		public void run() {
			AudioManager am = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
			if(am !=null) {
				am.abandonAudioFocus(MainActivity.this);
				Toast.makeText(MainActivity.this,"AUDIOFOCUS 를 반환 하였습니다.(ABANDON)",Toast.LENGTH_SHORT).show();
			}
		}
	};







	boolean checkFrontFlag = false;
	private Runnable mforegroundChanger = new Runnable() {
		@Override
		public void run() {
			Log.d("cuniverse","mforgroundChanger");
			mHandler.removeCallbacks(mforegroundChanger);
			ActivityManager am = (ActivityManager)getSystemService(Context.ACTIVITY_SERVICE);
			am.moveTaskToFront(getTaskId(), 0);
			if(checkFrontFlag == false) mHandler.postDelayed(mforegroundChanger,100);
		}
	};

	private static PowerManager.WakeLock sCpuWakeLock;
	@SuppressLint("InvalidWakeLockTag")
	private void disableKeyguard() {



		if (sCpuWakeLock != null) {
			return;
		}
		PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
		sCpuWakeLock = pm.newWakeLock(
				PowerManager.SCREEN_BRIGHT_WAKE_LOCK |
						PowerManager.ACQUIRE_CAUSES_WAKEUP |
						PowerManager.ON_AFTER_RELEASE, "hi");

		sCpuWakeLock.acquire();


		if (sCpuWakeLock != null) {
			sCpuWakeLock.release();
			sCpuWakeLock = null;
		}





		KeyguardManager keyguardManager
				= (KeyguardManager)getSystemService(KEYGUARD_SERVICE);
		KeyguardManager.KeyguardLock lock = keyguardManager.newKeyguardLock(KEYGUARD_SERVICE);
		lock.disableKeyguard();

		if(checkFrontFlag == false) {
			try {
				PendingIntent.getActivity(this, 10,
						new Intent(this, this.getClass())
								.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP),
						0).send();
			} catch (PendingIntent.CanceledException e) {
				e.printStackTrace();
			}

			//startActivity(new Intent(this, MainActivity.class).addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP));
			//mHandler.postDelayed(mforegroundChanger, 100);
		}

	}

	@Override
	public void onAudioFocusChange(int focusChange) {
		//후처리 필요 없으므로....

		Log.d("Trigger","onAudioFocusChange: " + focusChange);
	}


	void SendAudioFocus(){
		AudioManager am = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
		if(am !=null) {
			int focusResult = am.requestAudioFocus(this,
					AudioManager.STREAM_MUSIC,
					AudioManager.AUDIOFOCUS_GAIN_TRANSIENT); // 이건 focusChangeListener를 보면 알 수 있다.

			if (focusResult == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
				mHandler.removeCallbacks(m_auto_AudioFocus_hide_Runnable);
				mHandler.postDelayed(m_auto_AudioFocus_hide_Runnable,10000);
				//Log.d("Trigger","SendAudioFocus: " + focusResult);
				Toast.makeText(this,"트리거가 검출되어 재생중인 음원을 중지합니다.\n(AUDIOFOCUS_REQUEST_GRANTED)\n10초 후 반환합니다.",Toast.LENGTH_SHORT).show();
			} else {
				Toast.makeText(this,"트리거가 검출되었으나, AUDIOFOCUS 획득에 실패 하여, 재생중인 음원 중지에 실패 하였습니다.",Toast.LENGTH_SHORT).show();
			}
		}
	}

    // 인식결과를 받아 화면에 표시하는 콜백함수
    TriggerDetector.Callback cb = new TriggerDetector.Callback() {
        @Override
        public void callback(final String txt) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    String timestamp = timeFormat.format(new Date());
                    String new_text = timestamp + " " + txt + "\n" + tvSampleText.getText().toString();
                    tvSampleText.setText(new_text);

                    if (txt.startsWith("Detected")) {
                        trgCount++;
	                    mHandler.removeCallbacks(m_auto_hide_Runnable);
	                    tv_Listen.setVisibility(View.INVISIBLE);
	                    tv_trig.setVisibility(View.VISIBLE);
	                    mHandler.postDelayed(m_auto_hide_Runnable, 1000);
	                    //tv_trig.runDrawHit();
	                    //tv_trig.setText(String.format("%d", trgCount));
//                        waveCircle.runDrawHit();
	                    disableKeyguard();
	                    SendAudioFocus();
                    }

                }
            });

        }

        @Override
        public void rms(final short value, final short min) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
//                    waveCircle.runDrawValue(value);
                    waveForm.push(value, min);
                }
            });
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

//	    Calendar date = Calendar.getInstance();
//	    long _currTS = date.getTimeInMillis();
//	    date.set(2018, 6-1, 1);
//	    long _lTS = date.getTimeInMillis();
//	    if(_currTS >=_lTS) {
//		    Toast.makeText(this,"License expired!!",Toast.LENGTH_LONG).show();
//		    finish();
//	    }

		setContentView(R.layout.activity_main);
		Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
		setSupportActionBar(toolbar);


		View layout = findViewById(R.id.content_main);



		for (int x = 0 ; x < REQUIRED_PERMISSIONS.length; x++) {
			String permission = REQUIRED_PERMISSIONS[x];
			String feature = REQUIRED_Feature[x];

			Log.d("MainActivity", "Check permission: " + permission);
			if (feature.length() == 0 || getPackageManager().hasSystemFeature(feature))
			{
				boolean hasPermission = ActivityCompat.checkSelfPermission(this, permission) == PackageManager.PERMISSION_GRANTED;
				Log.d("MainActivity", permission + " is " + hasPermission);
				if ( hasPermission) {
					Toast.makeText(this,permission+" is GRANTED!!",Toast.LENGTH_LONG).show();
				}
				else
				{
					ActivityCompat.requestPermissions( MainActivity.this, REQUIRED_PERMISSIONS, PERMISSIONS_REQUEST_CODE);
				}
			}
			else
			{
				Toast.makeText(this,"실행 할 수 없는 기기 입니다("+permission+" 미지원). 앱을 종료 합니다.",Toast.LENGTH_LONG).show();
				MainActivity.this.finish();
			}
		}




        svLog = (ScrollView) findViewById(R.id.sv_log);
        //waveCircle = (ShowRecWave) findViewById(R.id.srw_srw);
        waveForm = (ShowWaveform) findViewById(R.id.swf_swf);
        tvRecording = (TextView) findViewById(R.id.tv_recording);
        tvSampleText = (TextView)  findViewById(R.id.sample_text);
	    tv_trig = (AutoResizeTextView) findViewById(R.id.tv_trig);
	    tv_Listen= (AutoResizeTextView) findViewById(R.id.tv_Listen);
        SharedPreferences settings = getSharedPreferences(PREFS_NAME, 0);
        saveSpotFlag = settings.getBoolean("saveSpot", false);
        setShowLog(settings.getBoolean("showLog", false));
        setKeepOn(settings.getBoolean("keepOn", true));
	    setTR_Amp2X(settings.getBoolean("isAmp2x_on", isAmp2x_on));
	    setTR_Comm(settings.getBoolean("isComm_on", isComm_on));



	    WakeUpSolid.I(); // make instance

        // Display build time
        String buildDate = dateFormat.format(BuildConfig.TIMESTAMP);
        String new_text = "App build date: " + buildDate + "\n" + tvSampleText.getText().toString();
        tvSampleText.setText(new_text);

	    tv_Listen.setText(
			    Html.fromHtml("<font color='#ebef00'>"+ getString(R.string.trigger_word_string)+"</font>이라고 불러보세요!"));
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
//        tvCount.setTextSize(waveCircle.getHeight() / 16);
    }

	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		menu.findItem(R.id.action_savespot).setChecked(saveSpotFlag);
		menu.findItem(R.id.action_screenon).setChecked(keepOnFlag);
		menu.findItem(R.id.action_log).setChecked(showLogFlag);
		menu.findItem(R.id.action_am2x).setChecked(isAmp2x_on);
		menu.findItem(R.id.action_use_comm).setChecked(isComm_on);
		this.menu = menu;

		return super.onPrepareOptionsMenu(menu);
	}

	@Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);

        getMenuInflater().inflate(R.menu.menu_main, menu);
        MenuItem miSaveSpot = menu.findItem(R.id.action_savespot);
        miSaveSpot.setChecked(saveSpotFlag);
        menu.findItem(R.id.action_screenon).setChecked(keepOnFlag);
        menu.findItem(R.id.action_log).setChecked(showLogFlag);
	    menu.findItem(R.id.action_am2x).setChecked(isAmp2x_on);
	    menu.findItem(R.id.action_use_comm).setChecked(isComm_on);
        this.menu = menu;
        return true;
    }

    void setKeepOn(boolean flag) {
        keepOnFlag = flag;
        if (flag) {
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        } else {
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        }
    }

    void setShowLog(boolean flag) {
        showLogFlag = flag;
        if (flag) {
            svLog.setVisibility(View.VISIBLE);
        } else {
            svLog.setVisibility(View.INVISIBLE);
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        if (id == R.id.action_screenon) {
            item.setChecked(!item.isChecked());
            setKeepOn(item.isChecked());
            return true;
        } else if (id == R.id.action_pcmsave) {
            if (null == td) return true;
            if (pcmRecordingFlag) {
                td.stopPcmLog();
                tvRecording.setVisibility(View.INVISIBLE);
            } else {
                td.startPcmLog();
                tvRecording.setVisibility(View.VISIBLE);
            }
            pcmRecordingFlag = !pcmRecordingFlag;

            // update menu text
            MenuItem miPcmSave = menu.findItem(R.id.action_pcmsave);
            if (pcmRecordingFlag) {
                miPcmSave.setTitle("Stop PCM recording");
            } else {
                miPcmSave.setTitle("Record PCM file");
            }
            return true;
        } else if (id == R.id.action_savespot) {
            item.setChecked(!item.isChecked());
            saveSpotFlag = item.isChecked();
            if (null != td)
                td.setSpotLog(saveSpotFlag);
            return true;
        } else if (id == R.id.action_log) {
            item.setChecked(!item.isChecked());
            setShowLog(item.isChecked());
            return true;
        } else if (id == R.id.action_am2x) {
	        item.setChecked(!item.isChecked());
	        setTR_Amp2X(item.isChecked());
	        return true;
        } else if (id == R.id.action_use_comm) {
	        item.setChecked(!item.isChecked());
	        setTR_Comm(item.isChecked());
	        return true;
        } else if (id == R.id.hide) {
			ignoreOnceStopTrigger = true;
	        moveTaskToBack(true);
	        return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onResume() {
        super.onResume();

        // Start detection on activity shown
        if (td != null) return;
        try {
	        td = new TriggerDetector(this, cb);
	        td.isAmp2x = isAmp2x_on;
	        td.isComm = isComm_on;
	        td.setSpotLog(saveSpotFlag);
	        td.start();
	        Snackbar.make(findViewById(R.id.content_main), "Detection started!", Snackbar.LENGTH_LONG)
			        .setAction("Action", null).show();

        } catch (Exception e){
	        Toast.makeText(this,"Engine Create Error Check Android LOG and Error CODE!!",Toast.LENGTH_LONG).show();
			finish();
        }

    }
	boolean isAmp2x_on = false;
    public void setTR_Amp2X(boolean on){
	    isAmp2x_on = on;
    	if(td!=null) td.isAmp2x = isAmp2x_on;

	    SharedPreferences settings = getSharedPreferences(PREFS_NAME, 0);
	    SharedPreferences.Editor editor = settings.edit();
	    editor.putBoolean("isAmp2x_on", isAmp2x_on);
	    editor.apply();
    }
	boolean isComm_on = true;
	public void setTR_Comm(boolean on){
		isComm_on = on;
		_Stop_Trigger();
		SharedPreferences settings = getSharedPreferences(PREFS_NAME, 0);
		SharedPreferences.Editor editor = settings.edit();
		editor.putBoolean("isComm_on", isComm_on);
		editor.apply();
		try {
			td = new TriggerDetector(this, cb);
			td.setSpotLog(saveSpotFlag);
			td.isAmp2x = isAmp2x_on;
			td.isComm = isComm_on;
			td.start();
			Snackbar.make(findViewById(R.id.content_main), "Detection started!", Snackbar.LENGTH_LONG)
					.setAction("Action", null).show();

		} catch (Exception e){
			Toast.makeText(this,"Engine Create Error Check Android LOG and Error CODE!!",Toast.LENGTH_LONG).show();
			finish();
		}

	}

	void _Stop_Trigger(){

		if (null == td) return;
		td.close();
		try {
			td.join();
		} catch (Exception e) {
			e.printStackTrace();
		}
		pcmRecordingFlag = false;
		tvRecording.setVisibility(View.INVISIBLE);
		td = null;
	}

	boolean ignoreOnceStopTrigger = false;
	@Override
    public void onPause() {
        super.onPause();
		if(ignoreOnceStopTrigger==true){
			Toast.makeText(this,"백그라운드 실행",Toast.LENGTH_SHORT).show();
		}
		else _Stop_Trigger();
		ignoreOnceStopTrigger = false;
    }

	@Override
	protected void onStart() {
		super.onStart();
		checkFrontFlag = true;
	}

	@Override
    protected void onStop() {
        super.onStop();

        // We need an Editor object to make preference changes.
        SharedPreferences settings = getSharedPreferences(PREFS_NAME, 0);
        SharedPreferences.Editor editor = settings.edit();
        editor.putBoolean("saveSpot", saveSpotFlag);
        editor.putBoolean("showLog", showLogFlag);
        editor.putBoolean("keepOn", keepOnFlag);

        // Commit the edits!
        editor.apply();
	    checkFrontFlag = false;
//		if(td==null) { //
//			NotificationManager nm = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
//			nm.cancel(R.string.app_name);
//		}
    }

	private static final int PERMISSIONS_REQUEST_CODE = 100;

	String[] REQUIRED_PERMISSIONS  = {
			Manifest.permission.RECORD_AUDIO,
			Manifest.permission.WRITE_EXTERNAL_STORAGE
	};

	String[] REQUIRED_Feature  = {
			"",
			""
	};

	@Override
	public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grandResults) {

		for (int x = 0 ; x < grandResults.length; x++) {
			int result = grandResults[x];
			String permission = permissions[x];
			if (result != PackageManager.PERMISSION_GRANTED) {
				String msg = "권한 요청("+permission+")이 거부되었습니다. 설정(앱 정보)에서 퍼미션을 허용해야 합니다.";
				if( ActivityCompat.shouldShowRequestPermissionRationale(this, permission) )
				{
					msg = "권한 요청("+permission+")이 거부되었습니다. 앱을 사용 할 수 없습니다. 다시 실행하여 퍼미션을 허용해주세요.";
				}
				Snackbar.make(findViewById(R.id.content_main), msg,
						Snackbar.LENGTH_INDEFINITE).setAction("확인", new View.OnClickListener() {

					@Override
					public void onClick(View view) {
						MainActivity.this.finish();
					}
				}).show();
			}
		}
	}
}
