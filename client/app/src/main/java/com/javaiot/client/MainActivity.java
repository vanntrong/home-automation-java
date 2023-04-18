package com.javaiot.client;

import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.javaiot.client.databinding.ActivityMainBinding;

import org.json.JSONException;
import org.json.JSONObject;

import ua.naiksoftware.stomp.Stomp;
import ua.naiksoftware.stomp.StompClient;
import util.StompUtils;

public class MainActivity extends AppCompatActivity {

    ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
//        setContentView(R.layout.activity_main);
        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        connectSocket();
    }

    public void connectSocket() {
        StompClient stompClient = Stomp.over(Stomp.ConnectionProvider.OKHTTP, "ws://192.168.1.2:8080/iot/websocket");
        Toast.makeText(this, "Start connecting to server", Toast.LENGTH_SHORT).show();
        stompClient.connect();
        StompUtils.lifecycle(stompClient);

        stompClient.topic("/home-device/messages/q").subscribe(stompMessage -> {
            JSONObject jsonObject = new JSONObject(stompMessage.getPayload());
            Log.i("IOT", "Recieve: " + stompMessage.getPayload());
            runOnUiThread(() -> {
                try {
                    String device = jsonObject.getString("device");
                    String number = jsonObject.getString("number");
                    String status = jsonObject.getString("status");

                    if (device.equals("led")) {
                        binding.swChangeLed1Status.setChecked(status.equals("on") ? true : false);
                    }
                }
                catch (JSONException e) {
                    e.printStackTrace();
                }
            });
        });
    }
}