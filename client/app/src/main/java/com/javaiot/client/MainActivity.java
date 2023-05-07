package com.javaiot.client;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.javaiot.client.databinding.ActivityMainBinding;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import ua.naiksoftware.stomp.Stomp;
import ua.naiksoftware.stomp.StompClient;
import util.StompUtils;

public class MainActivity extends AppCompatActivity {

    ActivityMainBinding binding;
    private StompClient stompClient;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
//        setContentView(R.layout.activity_main);
        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        connectSocket();
        setupDevices();
        addEvents();
    }

    private void addEvents() {
        binding.swChangeLed1Status.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    String ledStatus = binding.swChangeLed1Status.isChecked() ? "on" :"off";
                    JSONObject jsonObject = new JSONObject();
                    jsonObject.put("device", "led");
                    jsonObject.put("number", 1);
                    jsonObject.put("status", ledStatus);
                    stompClient.send("/room/q", jsonObject.toString()).subscribe();
                    Log.i("IOT", "Change Status to: " + binding.swChangeLed1Status.isChecked());
                }
                catch (JSONException e) {
                    e.printStackTrace();
                }
            }
        });
    }

    public void connectSocket() {
        stompClient = Stomp.over(Stomp.ConnectionProvider.OKHTTP, "ws://192.168.152.244:8080/iot/websocket");
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
                        binding.imvLedStatus.setImageResource(status.equals("on") ? R.drawable.ic_baseline_light_mode_24 : R.drawable.ic_baseline_lightbulb_24);
                    }
                }
                catch (JSONException e) {
                    e.printStackTrace();
                }
            });
        });

        stompClient.topic("/home-device/setup-devices/q").subscribe(stompMessage -> {
            JSONArray jsonArray = new JSONArray(stompMessage.getPayload());
            runOnUiThread(() -> {
                try {
                    for (int i = 0; i < jsonArray.length(); i++) {
                        JSONObject jsonObject = jsonArray.getJSONObject(i);
                        String status = jsonObject.getString("status");
                        binding.swChangeLed1Status.setChecked(status.equals("on") ? true : false);
                        binding.imvLedStatus.setImageResource(status.equals("on") ? R.drawable.ic_baseline_light_mode_24 : R.drawable.ic_baseline_lightbulb_24);
                    }
                }
                catch (JSONException e) {
                    e.printStackTrace();;
                }
                catch (Exception e) {
                    e.printStackTrace();
                }
            });
        });
    }

    public void setupDevices() {
        stompClient.send("/room/q/setup-devices").subscribe();
    }
}