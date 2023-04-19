package com.javaiot.server.controllers;

import com.google.api.core.ApiFuture;
import com.google.auth.oauth2.GoogleCredentials;
import com.google.cloud.firestore.Firestore;
import com.google.cloud.firestore.QueryDocumentSnapshot;
import com.google.cloud.firestore.QuerySnapshot;
import com.google.firebase.FirebaseApp;
import com.google.firebase.FirebaseOptions;
import com.google.firebase.cloud.FirestoreClient;
import com.google.firebase.database.FirebaseDatabase;
import com.javaiot.server.models.DeviceModel;
import jakarta.websocket.server.PathParam;
import org.json.JSONObject;
import org.springframework.messaging.handler.annotation.DestinationVariable;
import org.springframework.messaging.handler.annotation.MessageMapping;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.io.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.ExecutionException;

@RestController
public class ChatController {
    private final SimpMessagingTemplate messagingTemplate;

    public ChatController(SimpMessagingTemplate messagingTemplate) {
        this.messagingTemplate = messagingTemplate;
    }

    @MessageMapping("/room/{to}")
    public void sendMessage(@DestinationVariable String to, String message) {
        System.out.println(message);
        messagingTemplate.convertAndSend("/home-device/messages/" + to, message);
    }

    @GetMapping("/leds/{ledNumber}")
    String sendLed(@RequestParam("status") int status, @PathVariable("ledNumber") String ledNumber) {
        String ledStatus = status == 0 ? "off" : "on";
        JSONObject jsonObject = new JSONObject();
        jsonObject.put("device", "led");
        jsonObject.put("number", ledNumber);
        jsonObject.put("status", ledStatus);
        messagingTemplate.convertAndSend("/home-device/messages/q",jsonObject.toString());
        return "Send request to q led" + ledNumber + " "  + ledStatus;
    }

    @GetMapping("/devices")
    ArrayList<DeviceModel> getDevices() {
        ArrayList<DeviceModel> devices = new ArrayList<DeviceModel>();
        try {
            Firestore db = FirestoreClient.getFirestore();
            // asynchronously retrieve all devices
            ApiFuture<QuerySnapshot> query = db.collection("devices").get();

            // query.get() blocks on response
            QuerySnapshot querySnapshot = query.get();
            List<QueryDocumentSnapshot> documents = querySnapshot.getDocuments();
            for (QueryDocumentSnapshot document : documents) {
                DeviceModel device = new DeviceModel(document.getId(), document.getString("name"),
                        document.getString("status"),
                        document.getString("type"), document.getLong("pin"));
                devices.add(device);
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        } catch (ExecutionException e) {
            e.printStackTrace();
        }

        return devices;
    }
}
