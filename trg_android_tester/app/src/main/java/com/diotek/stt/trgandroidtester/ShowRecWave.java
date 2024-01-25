package com.diotek.stt.trgandroidtester;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Point;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.view.View;

/**
 * Created by mckeum on 2016-08-24.
 */

public class ShowRecWave extends View {
    private final int palette[] = {
            0xfff44336, 0xffe91e63, 0xff9c27b0, 0xff673ab7, 0xff3f51b5, 0xff2196f3, 0xff03a9f4, 0xff00bcd4,
            0xff009688, 0xff4caf50, 0xff8bc34a, 0xffcddc39, 0xffffc107, 0xffff9800, 0xffff5722,
    };

    private float mStep = 0;
    private int mPtx = 0;
    private int mPty = 0;
    private int colorIdx = 0;
    private Paint mPaint;
    private Paint mPaint2;

    private long lastInvalidateTime = 0;

    private Point center = null;

    public ShowRecWave(Context context, AttributeSet attrs) {
        super(context, attrs);

        lastInvalidateTime = SystemClock.uptimeMillis();

        center = new Point();

        mPaint = new Paint();
        mPaint.setStyle(Paint.Style.FILL);
        mPaint.setColor(palette[colorIdx]);
        mPaint.setAlpha(128);

        mPaint2 = new Paint();
        mPaint2.setStyle(Paint.Style.FILL);
        mPaint2.setColor(palette[colorIdx]);
    }

    public void runDrawValue(float value) {
        mStep = Math.max(value, 0.9f*mStep + 0.1f*value);
        long currentTime = SystemClock.uptimeMillis();
        if (30 < currentTime - lastInvalidateTime) {
            lastInvalidateTime = currentTime;
            this.invalidate();
        }
    }

    public void runDrawHit() {
        colorIdx = (colorIdx + (palette.length/2+1)) % palette.length;
        mPaint.setColor(palette[colorIdx]);
        mPaint.setAlpha(128);
        mPaint2.setColor(palette[colorIdx]);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        int r = Math.min(mPtx, mPty) / 2 * 3/5;

        float amp = (float) Math.sqrt(mStep / Short.MAX_VALUE);

        canvas.drawCircle(center.x, center.y, (amp/3 + 1.f) * r, mPaint);
        canvas.drawCircle(center.x, center.y, r, mPaint2);

        super.onDraw(canvas);
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        mPtx = w;
        mPty = h;
        center.x = w / 2;
        center.y = h / 2;
        super.onSizeChanged(w, h, oldw, oldh);
    }
}
