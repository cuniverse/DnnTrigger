package com.selvasai.stt.selvywakeup;

import android.content.Context;
import android.content.res.AssetManager;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * Created by mckeum on 2017-01-04.
 */

class AssetCopier {
    Context mContext;

    AssetCopier(Context context) {
        mContext = context;
    }

    // for copy assets to internal storage
    void copyAssetAll(String srcPath) {
        AssetManager assetMgr = mContext.getAssets();
        try {
            String[] assets = assetMgr.list(srcPath);
            if (assets.length == 0) {
                copyFile(srcPath);
            } else {
                String destPath = mContext.getFilesDir().getAbsolutePath() + File.separator + srcPath;
                File dir = new File(destPath);
                if (!dir.exists()) dir.mkdir();
                for (String element : assets) {
                    copyAssetAll(srcPath + File.separator + element);
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    void copyFile(String srcFile) {
        AssetManager assetMgr = mContext.getAssets();
        String destFile = mContext.getFilesDir().getAbsolutePath() + File.separator + srcFile;
        try {
            InputStream is = assetMgr.open(srcFile);
            BufferedOutputStream os = new BufferedOutputStream(new FileOutputStream(destFile));

            byte[] buffer = new byte[8192];
            while (true) {
                int readBytes = is.read(buffer);
                if (-1 == readBytes) break;
                os.write(buffer, 0, readBytes);
            }
            is.close();
            os.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
