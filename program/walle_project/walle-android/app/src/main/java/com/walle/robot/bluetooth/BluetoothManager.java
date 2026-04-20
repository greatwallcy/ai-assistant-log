package com.walle.robot.bluetooth;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * 蓝牙连接管理器 - 蓝牙串口通信 (HC-05)
 * 波特率: 9600bps
 */
public class BluetoothManager {

    private static final String TAG = "BluetoothManager";
    // SPP (Serial Port Profile) UUID
    private static final UUID SPP_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    // ========== 蓝牙命令定义（与固件对应） ==========
    public static final byte CMD_FORWARD      = 0x01; // 前进
    public static final byte CMD_BACKWARD     = 0x02; // 后退
    public static final byte CMD_RIGHT        = 0x03; // 右转
    public static final byte CMD_LEFT         = 0x04; // 左转
    public static final byte CMD_STOP_BRAKE   = 0x05; // 刹车停
    public static final byte CMD_STOP_RELEASE = 0x06; // 释放停
    public static final byte CMD_BUZZER_ON    = 0x07; // 蜂鸣器开
    public static final byte CMD_EYE_L_ON     = 0x08; // 左眼灯开
    public static final byte CMD_EYE_R_ON     = 0x09; // 右眼灯开
    public static final byte CMD_BUZZER_OFF   = 0x0A; // 蜂鸣器关
    public static final byte CMD_EYE_L_OFF    = 0x0B; // 左眼灯关
    public static final byte CMD_EYE_R_OFF    = 0x0C; // 右眼灯关
    public static final byte CMD_SPEED_LOW    = 0x15; // 速度低(65)
    public static final byte CMD_SPEED_MID    = 0x16; // 速度中(80)
    public static final byte CMD_SPEED_HIGH   = 0x17; // 速度高(100)

    // 回调接口
    public interface ConnectionCallback {
        void onConnected(String deviceName);
        void onDisconnected();
        void onConnectionFailed(String error);
        void onDataReceived(byte[] data, int length);
    }

    private BluetoothAdapter bluetoothAdapter;
    private BluetoothSocket bluetoothSocket;
    private OutputStream outputStream;
    private InputStream inputStream;
    private ConnectionCallback callback;
    private final ExecutorService executor = Executors.newSingleThreadExecutor();
    private final Handler mainHandler = new Handler(Looper.getMainLooper());
    private volatile boolean isConnected = false;
    private volatile boolean isListening = false;

    @SuppressLint("MissingPermission")
    public BluetoothManager() {
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
    }

    public void setCallback(ConnectionCallback callback) {
        this.callback = callback;
    }

    public boolean isBluetoothEnabled() {
        return bluetoothAdapter != null && bluetoothAdapter.isEnabled();
    }

    public boolean isConnected() {
        return isConnected && bluetoothSocket != null && bluetoothSocket.isConnected();
    }

    /**
     * 连接蓝牙设备
     */
    @SuppressLint("MissingPermission")
    public void connect(BluetoothDevice device) {
        executor.execute(() -> {
            try {
                // 取消蓝牙发现（会占用连接）
                bluetoothAdapter.cancelDiscovery();

                // 创建 RFCOMM socket
                bluetoothSocket = device.createRfcommSocketToServiceRecord(SPP_UUID);
                bluetoothSocket.connect();

                outputStream = bluetoothSocket.getOutputStream();
                inputStream = bluetoothSocket.getInputStream();
                isConnected = true;

                mainHandler.post(() -> {
                    if (callback != null) callback.onConnected(device.getName());
                });

                // 启动接收线程
                startListening();

            } catch (IOException e) {
                Log.e(TAG, "Connection failed", e);
                isConnected = false;
                mainHandler.post(() -> {
                    if (callback != null) callback.onConnectionFailed(e.getMessage());
                });
                try {
                    if (bluetoothSocket != null) bluetoothSocket.close();
                } catch (IOException ignored) {}
            }
        });
    }

    /**
     * 监听接收数据
     */
    private void startListening() {
        isListening = true;
        executor.execute(() -> {
            byte[] buffer = new byte[256];
            while (isListening && isConnected) {
                try {
                    int bytes = inputStream.read(buffer);
                    if (bytes > 0 && callback != null) {
                        byte[] received = new byte[bytes];
                        System.arraycopy(buffer, 0, received, 0, bytes);
                        mainHandler.post(() -> callback.onDataReceived(received, bytes));
                    }
                } catch (IOException e) {
                    if (isConnected) {
                        isConnected = false;
                        isListening = false;
                        mainHandler.post(() -> {
                            if (callback != null) callback.onDisconnected();
                        });
                    }
                    break;
                }
            }
        });
    }

    /**
     * 发送单字节命令
     */
    public void sendCommand(byte cmd) {
        if (!isConnected() || outputStream == null) return;
        executor.execute(() -> {
            try {
                outputStream.write(cmd);
                outputStream.flush();
                Log.d(TAG, "Sent command: 0x" + String.format("%02X", cmd));
            } catch (IOException e) {
                Log.e(TAG, "Send failed", e);
                isConnected = false;
                mainHandler.post(() -> {
                    if (callback != null) callback.onDisconnected();
                });
            }
        });
    }

    /**
     * 断开连接
     */
    public void disconnect() {
        isListening = false;
        isConnected = false;
        executor.execute(() -> {
            try {
                if (outputStream != null) outputStream.close();
                if (inputStream != null) inputStream.close();
                if (bluetoothSocket != null) bluetoothSocket.close();
            } catch (IOException e) {
                Log.e(TAG, "Disconnect error", e);
            }
        });
    }

    public BluetoothAdapter getBluetoothAdapter() {
        return bluetoothAdapter;
    }

    /**
     * 清理资源
     */
    public void destroy() {
        disconnect();
        executor.shutdown();
    }
}
