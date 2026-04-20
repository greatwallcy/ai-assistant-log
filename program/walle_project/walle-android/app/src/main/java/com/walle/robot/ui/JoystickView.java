package com.walle.robot.ui;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RadialGradient;
import android.graphics.Shader;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

/**
 * 虚拟摇杆控件
 * 支持8方向控制：前/后/左/右 + 4个对角方向
 */
public class JoystickView extends View {

    public interface OnDirectionChangeListener {
        /** direction: 0=无, 1=前, 2=后, 3=右, 4=左 */
        void onDirectionChanged(int direction);
    }

    private Paint basePaint;
    private Paint stickPaint;
    private Paint glowPaint;
    private float centerX, centerY;
    private float baseRadius;
    private float stickRadius;
    private float stickX, stickY;
    private int currentDirection = 0;
    private OnDirectionChangeListener listener;

    // 颜色
    private static final int BASE_COLOR = 0xFF2C2C2C;
    private static final int STICK_COLOR = 0xFF4CAF50;
    private static final int STICK_PRESSED_COLOR = 0xFF66BB6A;
    private static final int GLOW_COLOR = 0x334CAF50;
    private static final int ARROW_COLOR = 0x66FFFFFF;

    private boolean isPressed = false;

    public JoystickView(Context context) {
        super(context);
        init();
    }

    public JoystickView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    private void init() {
        basePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        basePaint.setColor(BASE_COLOR);
        basePaint.setStyle(Paint.Style.FILL);

        stickPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        stickPaint.setColor(STICK_COLOR);
        stickPaint.setStyle(Paint.Style.FILL);

        glowPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        glowPaint.setColor(GLOW_COLOR);
        glowPaint.setStyle(Paint.Style.FILL);

        // 允许硬件加速
        setLayerType(LAYER_TYPE_SOFTWARE, null);
    }

    public void setOnDirectionChangeListener(OnDirectionChangeListener listener) {
        this.listener = listener;
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        centerX = w / 2f;
        centerY = h / 2f;
        baseRadius = Math.min(w, h) / 2f * 0.85f;
        stickRadius = baseRadius * 0.32f;
        stickX = centerX;
        stickY = centerY;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        // 画底座（深色圆盘）
        canvas.drawCircle(centerX, centerY, baseRadius, basePaint);

        // 画方向指示（十字）
        Paint arrowPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        arrowPaint.setColor(ARROW_COLOR);
        arrowPaint.setTextSize(baseRadius * 0.18f);
        arrowPaint.setTextAlign(Paint.Align.CENTER);
        float arrowOffset = baseRadius * 0.7f;

        // 上下左右文字
        canvas.drawText("▲", centerX, centerY - arrowOffset + arrowPaint.getTextSize() / 3, arrowPaint);
        canvas.drawText("▼", centerX, centerY + arrowOffset - arrowPaint.getTextSize() / 6, arrowPaint);
        canvas.drawText("◀", centerX - arrowOffset, centerY + arrowPaint.getTextSize() / 3, arrowPaint);
        canvas.drawText("▶", centerX + arrowOffset, centerY + arrowPaint.getTextSize() / 3, arrowPaint);

        // 画摇杆球体渐变
        int stickColor = isPressed ? STICK_PRESSED_COLOR : STICK_COLOR;
        RadialGradient gradient = new RadialGradient(
                stickX - stickRadius * 0.2f, stickY - stickRadius * 0.2f,
                stickRadius * 1.5f,
                new int[]{Color.WHITE, stickColor, darkenColor(stickColor)},
                new float[]{0f, 0.4f, 1f},
                Shader.TileMode.CLAMP);
        stickPaint.setShader(gradient);
        canvas.drawCircle(stickX, stickY, stickRadius, stickPaint);

        // 按下时的光晕
        if (isPressed) {
            canvas.drawCircle(stickX, stickY, stickRadius * 1.3f, glowPaint);
        }
    }

    private int darkenColor(int color) {
        float[] hsv = new float[3];
        Color.colorToHSV(color, hsv);
        hsv[2] *= 0.7f;
        return Color.HSVToColor(hsv);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                isPressed = true;
                updateStickPosition(event.getX(), event.getY());
                return true;

            case MotionEvent.ACTION_MOVE:
                updateStickPosition(event.getX(), event.getY());
                return true;

            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                isPressed = false;
                stickX = centerX;
                stickY = centerY;
                updateDirection(0);
                invalidate();
                return true;
        }
        return super.onTouchEvent(event);
    }

    private void updateStickPosition(float touchX, float touchY) {
        float dx = touchX - centerX;
        float dy = touchY - centerY;
        float distance = (float) Math.sqrt(dx * dx + dy * dy);

        // 限制在圆形区域内
        if (distance > baseRadius - stickRadius) {
            float angle = (float) Math.atan2(dy, dx);
            stickX = centerX + (baseRadius - stickRadius) * (float) Math.cos(angle);
            stickY = centerY + (baseRadius - stickRadius) * (float) Math.sin(angle);
        } else {
            stickX = touchX;
            stickY = touchY;
        }

        // 计算方向（死区半径 15%）
        float deadZone = baseRadius * 0.15f;
        if (distance < deadZone) {
            updateDirection(0);
        } else {
            // 计算角度
            float angleDeg = (float) Math.toDegrees(Math.atan2(centerY - stickY, stickX - centerX));
            // 归一化到 0-360
            if (angleDeg < 0) angleDeg += 360;

            int dir;
            if (angleDeg >= 315 || angleDeg < 45) {
                dir = 3; // 右
            } else if (angleDeg >= 45 && angleDeg < 135) {
                dir = 1; // 前
            } else if (angleDeg >= 135 && angleDeg < 225) {
                dir = 4; // 左
            } else {
                dir = 2; // 后
            }
            updateDirection(dir);
        }
        invalidate();
    }

    private void updateDirection(int newDirection) {
        if (newDirection != currentDirection) {
            currentDirection = newDirection;
            if (listener != null) {
                listener.onDirectionChanged(currentDirection);
            }
        }
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        // 强制正方形
        int size = Math.min(
                MeasureSpec.getSize(widthMeasureSpec),
                MeasureSpec.getSize(heightMeasureSpec));
        setMeasuredDimension(size, size);
    }
}
