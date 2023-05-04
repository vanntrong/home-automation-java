package com.javaiot.server.controllers;

import com.google.api.core.ApiFuture;
import com.google.cloud.firestore.Firestore;
import com.google.cloud.firestore.QueryDocumentSnapshot;
import com.google.cloud.firestore.QuerySnapshot;
import com.google.firebase.cloud.FirestoreClient;
import com.google.firebase.database.*;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.javaiot.server.models.DeviceModel;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.messaging.handler.annotation.DestinationVariable;
import org.springframework.messaging.handler.annotation.MessageMapping;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import javax.xml.crypto.Data;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;

@RestController
public class DeviceController {
    private final SimpMessagingTemplate messagingTemplate;

    public DeviceController(SimpMessagingTemplate messagingTemplate) {
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

    @MessageMapping("/room/{to}/setup-devices")
    @GetMapping("devices")
    CompletableFuture<ResponseEntity<Object>> getDevices(@DestinationVariable String to) {
        return CompletableFuture.supplyAsync(() -> {
            try {
                DatabaseReference databaseReference = FirebaseDatabase.getInstance().getReference();
                DatabaseReference devicesRef = databaseReference.child("device");
                CompletableFuture<Object> future = new CompletableFuture<>();
                devicesRef.addListenerForSingleValueEvent(new ValueEventListener() {
                    @Override
                    public void onDataChange(DataSnapshot dataSnapshot) {
                        if (dataSnapshot.exists()) {
                            Iterable<DataSnapshot> dataSnapshotIterable = dataSnapshot.getChildren();
                            Iterator<DataSnapshot> iterator = dataSnapshotIterable.iterator();
                            JSONArray jsonArray = new JSONArray();
                            while (iterator.hasNext()) {
                                Gson gson = new GsonBuilder().setPrettyPrinting().create();
                                String json = gson.toJson(iterator.next().getValue());
                                JSONObject jsonObject = new JSONObject(json);
                                jsonArray.put(jsonObject);
                            }

                            future.complete(jsonArray);
                        }
                    }
                    @Override
                    public void onCancelled(DatabaseError databaseError) {
                        future.completeExceptionally(databaseError.toException());
                    }
                });

                Object data = future.get();

                messagingTemplate.convertAndSend("/home-device/setup-devices/" + to,data.toString());

                return ResponseEntity.ok(data);
            }
            catch (JSONException e) {
                e.printStackTrace();
                return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body("Failed to fetch data from Firebase.");
            }
            catch (InterruptedException e) {
                e.printStackTrace();
                return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body("Failed to fetch data from Firebase.");
            }
            catch (ExecutionException e) {
                e.printStackTrace();
                return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body("Failed to fetch data from Firebase.");
            }
        });
    }
}
