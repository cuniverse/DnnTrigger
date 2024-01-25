package com.diotek.stt.trgandroidtester;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.view.View;

/**
 * Created by mckeum on 2016-08-31.
 */

public class ShowWaveform extends View {
    public static int ptx = 480 - 10;
    public static int pty = 100 - 8;
    public static int pxX = 100 - 8;

    public int ibuf = 0;

    public short[] sbuf1;
    public short[] sbuf2;

    private long lastInvalidateTime = 0;
    private Paint mPaint;

    public ShowWaveform(Context context, AttributeSet attrs) {
        super(context, attrs);

        lastInvalidateTime = SystemClock.uptimeMillis();

        mPaint = new Paint();
        mPaint.setStyle(Paint.Style.STROKE);
        mPaint.setColor(0xff455a64);
        mPaint.setStrokeWidth(16);
        init();
    }

    public void init() {
        ibuf = 200;
        int len = 200;

        sbuf1 = new short[len];
        sbuf2 = new short[len];

        invalidate();
    }

    public void push(short max, short min) {
        System.arraycopy(sbuf1, 1, sbuf1, 0, sbuf1.length-1);
        sbuf1[sbuf1.length-1] = max;

        System.arraycopy(sbuf2, 1, sbuf2, 0, sbuf1.length-1);
        sbuf2[sbuf2.length-1] = min;

        long currentTime = SystemClock.uptimeMillis();
        if (30 < currentTime - lastInvalidateTime) {
            lastInvalidateTime = currentTime;
            invalidate();
        }
    }

    @Override
    protected void onDraw(Canvas canvas) {
        ptx = sbuf1.length;

        float stepX = (float) (pxX) / ptx;
        float stepY = (float) (pty) / 65535;

        for (int i = 0; i < ibuf; i++) {
            if ((sbuf1[i] * stepY) < 3 && (sbuf2[i] * stepY) > -3) {
                canvas.drawLine(i * stepX, (pty / 2) + (float) 0.5, i * stepX, (pty / 2) - (float) 0.5, mPaint);
            } else {
                canvas.drawLine(i * stepX, (pty / 2) + (sbuf1[i] * stepY), i * stepX, (pty / 2) + (sbuf2[i] * stepY), mPaint);
            }
        }

        super.onDraw(canvas);
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        pxX = this.getWidth();
        pty = this.getHeight();
        super.onSizeChanged(w, h, oldw, oldh);
    }
}
