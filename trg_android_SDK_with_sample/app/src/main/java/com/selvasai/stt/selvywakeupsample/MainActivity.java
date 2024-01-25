package com.selvasai.stt.selvywakeupsample;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.ActivityManager;
import android.app.KeyguardManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.PowerManager;
import android.support.annotation.NonNull;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.widget.TextView;
import android.widget.Toast;

import com.selvasai.stt.selvywakeup.WakeUpSolid;

public class MainActivity extends AppCompatActivity implements AudioManager.OnAudioFocusChangeListener{
    private final static String TAG = "WakeUp Sample MainActivity";

    private KeywordDetector detectingThread;

    TextView tvSampleText;

    KeywordDetector.Callback kdcb = new KeywordDetector.Callback() {
        @Override
        public void callback(final String txt) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    String new_text = txt + "\n" + tvSampleText.getText().toString();
                    tvSampleText.setText(new_text);
	                if (txt.startsWith("Detected")) {
		                disableKeyguard();
		                SendAudioFocus();
	                }
                }
            });
        }
    };

    boolean checkFrontFlag = false;
    private Runnable mforegroundChanger = new Runnable() {
	    @Override
	    public void run() {
	    	Log.d("cuniverse","mforgroundChanger");
		    mHandler.removeCallbacks(mforegroundChanger);
		    if(checkFrontFlag == false) {
			    ActivityManager am = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
			    am.moveTaskToFront(getTaskId(), 0);
			    mHandler.postDelayed(mforegroundChanger, 100);
		    }
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

	Handler mHandler = new Handler();


	int mNotificationID = -1;

	@Override
	protected void onResume() {
		super.onResume();

	}

	@Override
	protected void onStart() {
		super.onStart();
		checkFrontFlag = true;

		//NotificationManager nm = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
	}

	@Override
	protected void onStop() {
		super.onStop();
		checkFrontFlag = false;
	}

	@Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

		getWindow().addFlags(WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD |
				WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED |
				WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON |
				WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);



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










		tvSampleText = (TextView) findViewById(R.id.tv_log);

        // 미리 초기화 작업을 수행 (버튼 탭 시 반응성 향상)
	    try {
		    WakeUpSolid.I(this);
	    } catch (WakeUpSolid.CreateEngineErrorException e) {
		    e.printStackTrace();
		    Toast.makeText(this,"Engine Create Error Check Android LOG and Error CODE!!",Toast.LENGTH_LONG).show();
		    finish();

	    }

	    FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                // Start detection
                if (detectingThread != null) {
	                stopDetection();
                	return;
                }
                try {
	                detectingThread = new KeywordDetector(getApplicationContext(), kdcb);
	                detectingThread.start();

	                Snackbar.make(findViewById(R.id.content_main), "Detection started!", Snackbar.LENGTH_LONG)
			                .setAction("Action", null).show();
                }catch (Exception e)
                {
	                Toast.makeText(MainActivity.this,"Engine Create Error Check Android LOG and Error CODE!!",Toast.LENGTH_LONG).show();
	                finish();
                }
            }
        });




    }

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.menu_main,menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {

		int id = item.getItemId();
		switch (id){
			case R.id.action_quit:
				stopDetection();
				moveTaskToBack(true);
				finish();
				android.os.Process.killProcess(android.os.Process.myPid());
				return true;
		}
		return super.onOptionsItemSelected(item);
	}

	@Override
    public void onPause() {
        super.onPause();
		//stopDetection();
    }

    void stopDetection(){

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
	public void onAudioFocusChange(int focusChange) {
		//후처리 필요 없으므로....

		Log.d("Trigger","onAudioFocusChange: " + focusChange);
	}


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
