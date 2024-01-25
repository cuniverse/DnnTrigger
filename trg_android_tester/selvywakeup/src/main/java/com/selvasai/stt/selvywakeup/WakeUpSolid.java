package com.selvasai.stt.selvywakeup;

import android.content.Context;
import android.util.AndroidException;
import android.util.Log;

import java.io.File;

/**
 * Created by mckeum on 2016-08-11.
 * JNI Java class wrapper for WakeUpSolid
 */

public class WakeUpSolid {
    private final static String TAG = "WakeUpSolid";
    private static WakeUpSolid instance;

    public static class CreateEngineErrorException extends AndroidException
    {
	    public CreateEngineErrorException() {
	    }

	    public CreateEngineErrorException(String msg){
	    	super(msg);
	    }
    }

    private WakeUpSolid(String bin_path) throws CreateEngineErrorException{
        int ret = solidInit(bin_path);
        if (ret < 0) {
	        Log.e(TAG, "triggerInit() " + ret);
	        throw new CreateEngineErrorException("Trigger Init Error!! code:" + ret);
        }
    }

    public static WakeUpSolid I(Context context) throws CreateEngineErrorException {
        if (instance != null)   return instance;

        // copy assets to internal storage
        AssetCopier as = new AssetCopier(context);
        as.copyAssetAll("conf");
        as.copyAssetAll("res");

        String resPath = context.getFilesDir().getAbsolutePath() + File.separator + "res" + File.separator;
        instance = new WakeUpSolid(resPath);
        return  instance;
    }

    public static WakeUpSolid I() {
        return  instance;
    }

    protected void finalize() throws Throwable {
        super.finalize();
        solidDestroy();
    }

    public void reset() {
        solidReset();
    }

    public int detect(short[] in_pcm) {
	    int[] info = new int[2];
        return detect(in_pcm,info);
    }
	public int detect(short[] in_pcm,int[] info) {
		int det = solidDetect(in_pcm.length, in_pcm,info);
		return det;
	}

    static {
        System.loadLibrary("SelvyWakeupJni");
    }

    private native int solidInit(String bin_path);
    private native int solidDetect(int len_sample, short[] in_pcm,int[] info);
    private native void solidDestroy();
    private native void solidReset();
}
