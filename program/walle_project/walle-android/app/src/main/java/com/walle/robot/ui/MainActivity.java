package com.walle.robot.ui;

import android.Manifest;
import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.google.android.material.button.MaterialButton;
import com.walle.robot.R;
import com.walle.robot.bluetooth.BluetoothManager;

import java.util.ArrayList;
import java.util.Set;

/**
 * WALL-E 机器人遥控主界面
 * 功能：蓝牙连接、虚拟摇杆控制、蜂鸣器/眼灯开关、速度选择
 */
public class MainActivity extends AppCompatActivity implements
        JoystickView.OnDirectionChangeListener,
        BluetoothManager.ConnectionCallback {

    private BluetoothManager btManager;
    private JoystickView joystick;
    private MaterialButton btnConnect, btnBuzzer, btnEyeLeft, btnEyeRight;
    private View lcdContainer;
    private android.widget.ImageView ivFace;
    private android.widget.TextView tvStatus;

    // 状态
    private boolean buzzerOn = false;
    private boolean eyeLeftOn = false;
    private boolean eyeRightOn = false;
    private byte currentSpeed = BluetoothManager.CMD_SPEED_MID;

    // Activity Result
    private ActivityResultLauncher<Intent> bluetoothEnableLauncher;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initViews();
        initBluetooth();
        initListeners();
    }

    private void initViews() {
        joystick = findViewById(R.id.joystick);
        btnConnect = findViewById(R.id.btn_connect);
        btnBuzzer = findViewById(R.id.btn_buzzer);
        btnEyeLeft = findViewById(R.id.btn_eye_left);
        btnEyeRight = findViewById(R.id.btn_eye_right);
        lcdContainer = findViewById(R.id.lcd_container);
        ivFace = findViewById(R.id.iv_lcd_face);
        tvStatus = findViewById(R.id.tv_status);
    }

    private void initBluetooth() {
        btManager = new BluetoothManager();
        btManager.setCallback(this);

        bluetoothEnableLauncher = registerForActivityResult(
                new ActivityResultContracts.StartActivityForResult(),
                result -> {
                    if (btManager.isBluetoothEnabled()) {
                        showDeviceList();
                    } else {
                        showToast("蓝牙未启用");
                    }
                });
    }

    @SuppressLint("ClickableViewAccessibility")
    private void initListeners() {
        // 摇杆方向变化
        joystick.setOnDirectionChangeListener(this);

        // 连接按钮
        btnConnect.setOnClickListener(v -> {
            if (btManager.isConnected()) {
                btManager.disconnect();
                updateConnectionUI(false, null);
            } else {
                checkBluetoothAndConnect();
            }
        });

        // 蜂鸣器开关
        btnBuzzer.setOnClickListener(v -> {
            buzzerOn = !buzzerOn;
            btManager.sendCommand(buzzerOn ?
                    BluetoothManager.CMD_BUZZER_ON :
                    BluetoothManager.CMD_BUZZER_OFF);
            updateToggleButton(btnBuzzer, buzzerOn);
        });

        // 左眼灯
        btnEyeLeft.setOnClickListener(v -> {
            eyeLeftOn = !eyeLeftOn;
            btManager.sendCommand(eyeLeftOn ?
                    BluetoothManager.CMD_EYE_L_ON :
                    BluetoothManager.CMD_EYE_L_OFF);
            updateToggleButton(btnEyeLeft, eyeLeftOn);
        });

        // 右眼灯
        btnEyeRight.setOnClickListener(v -> {
            eyeRightOn = !eyeRightOn;
            btManager.sendCommand(eyeRightOn ?
                    BluetoothManager.CMD_EYE_R_ON :
                    BluetoothManager.CMD_EYE_R_OFF);
            updateToggleButton(btnEyeRight, eyeRightOn);
        });

        // 速度选择
        findViewById(R.id.chip_speed_low).setOnClickListener(v -> {
            currentSpeed = BluetoothManager.CMD_SPEED_LOW;
            btManager.sendCommand(currentSpeed);
        });
        findViewById(R.id.chip_speed_mid).setOnClickListener(v -> {
            currentSpeed = BluetoothManager.CMD_SPEED_MID;
            btManager.sendCommand(currentSpeed);
        });
        findViewById(R.id.chip_speed_high).setOnClickListener(v -> {
            currentSpeed = BluetoothManager.CMD_SPEED_HIGH;
            btManager.sendCommand(currentSpeed);
        });
    }

    // ==================== 蓝牙连接流程 ====================

    private void checkBluetoothAndConnect() {
        if (btManager.getBluetoothAdapter() == null) {
            showToast("设备不支持蓝牙");
            return;
        }
        if (!btManager.isBluetoothEnabled()) {
            Intent enableBt = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            bluetoothEnableLauncher.launch(enableBt);
            return;
        }
        // 检查权限
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT)
                    != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this,
                        new String[]{Manifest.permission.BLUETOOTH_CONNECT,
                                Manifest.permission.BLUETOOTH_SCAN}, 1001);
                return;
            }
        }
        showDeviceList();
    }

    @SuppressLint("MissingPermission")
    private void showDeviceList() {
        Set<BluetoothDevice> pairedDevices = btManager.getBluetoothAdapter().getBondedDevices();
        if (pairedDevices == null || pairedDevices.isEmpty()) {
            showToast("没有已配对的蓝牙设备，请先在系统设置中配对 HC-05");
            return;
        }

        ArrayList<BluetoothDevice> deviceList = new ArrayList<>(pairedDevices);
        String[] names = new String[deviceList.size()];
        for (int i = 0; i < deviceList.size(); i++) {
            names[i] = deviceList.get(i).getName() + "\n" + deviceList.get(i).getAddress();
        }

        new AlertDialog.Builder(this)
                .setTitle("选择蓝牙设备")
                .setItems(names, (dialog, which) -> {
                    BluetoothDevice device = deviceList.get(which);
                    tvStatus.setText("连接中...");
                    btManager.connect(device);
                })
                .setNegativeButton("取消", null)
                .show();
    }

    // ==================== 摇杆回调 ====================

    @Override
    public void onDirectionChanged(int direction) {
        if (!btManager.isConnected()) return;

        byte cmd;
        switch (direction) {
            case 1: cmd = BluetoothManager.CMD_FORWARD; break;
            case 2: cmd = BluetoothManager.CMD_BACKWARD; break;
            case 3: cmd = BluetoothManager.CMD_RIGHT; break;
            case 4: cmd = BluetoothManager.CMD_LEFT; break;
            default:
                btManager.sendCommand(BluetoothManager.CMD_STOP_BRAKE);
                updateFace(Face.NORMAL);
                return;
        }
        btManager.sendCommand(cmd);

        // 更新 LCD 表情
        switch (direction) {
            case 1: updateFace(Face.FORWARD); break;
            case 2: updateFace(Face.BACKWARD); break;
            case 3: updateFace(Face.RIGHT); break;
            case 4: updateFace(Face.LEFT); break;
        }
    }

    // ==================== 蓝牙回调 ====================

    @Override
    public void onConnected(String deviceName) {
        updateConnectionUI(true, deviceName);
    }

    @Override
    public void onDisconnected() {
        updateConnectionUI(false, null);
        showToast("蓝牙已断开");
    }

    @Override
    public void onConnectionFailed(String error) {
        updateConnectionUI(false, null);
        showToast("连接失败: " + error);
    }

    @Override
    public void onDataReceived(byte[] data, int length) {
        // 可以在这里处理从机器人返回的数据（如电量信息）
    }

    // ==================== UI 更新 ====================

    private void updateConnectionUI(boolean connected, String deviceName) {
        if (connected) {
            tvStatus.setText("已连接: " + deviceName);
            tvStatus.setTextColor(getColor(R.color.connected_green));
            btnConnect.setImageResource(R.drawable.ic_bluetooth_connected);
        } else {
            tvStatus.setText(R.string.disconnected);
            tvStatus.setTextColor(getColor(R.color.disconnected_red));
            btnConnect.setImageResource(R.drawable.ic_bluetooth);
            // 重置所有开关状态
            buzzerOn = false;
            eyeLeftOn = false;
            eyeRightOn = false;
            updateToggleButton(btnBuzzer, false);
            updateToggleButton(btnEyeLeft, false);
            updateToggleButton(btnEyeRight, false);
        }
    }

    private void updateToggleButton(MaterialButton btn, boolean isOn) {
        if (isOn) {
            btn.setBackgroundTintList(
                    android.content.res.ColorStateList.valueOf(getColor(R.color.accent_green)));
        } else {
            btn.setBackgroundTintList(
                    android.content.res.ColorStateList.valueOf(getColor(R.color.btn_inactive)));
        }
    }

    // LCD 表情
    private enum Face { NORMAL, FORWARD, BACKWARD, LEFT, RIGHT }

    private void updateFace(Face face) {
        int resId;
        switch (face) {
            case FORWARD: resId = R.drawable.face_happy; break;
            case BACKWARD: resId = R.drawable.face_careful; break;
            case LEFT:
            case RIGHT: resId = R.drawable.face_curious; break;
            default: resId = R.drawable.face_normal; break;
        }
        ivFace.setImageResource(resId);
    }

    private void showToast(String msg) {
        Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (btManager != null) btManager.destroy();
    }
}
