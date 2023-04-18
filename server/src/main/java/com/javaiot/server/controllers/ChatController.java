package com.javaiot.server.controllers;

import jakarta.websocket.server.PathParam;
import org.springframework.messaging.handler.annotation.DestinationVariable;
import org.springframework.messaging.handler.annotation.MessageMapping;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class ChatController {
    private final SimpMessagingTemplate messagingTemplate;

    public ChatController(SimpMessagingTemplate messagingTemplate) {
        this.messagingTemplate = messagingTemplate;
    }

    @MessageMapping("/iot/{to}")
    public void sendMessage(@DestinationVariable String to, String message) {
        System.out.println(message);
        messagingTemplate.convertAndSend("/home-device/messages/" + to, message);
    }

    @GetMapping("/leds/{ledNumber}")
    String sendLed(@RequestParam("status") int status, @PathVariable("ledNumber") String ledNumber) {
        String ledStatus = status == 0 ? "off" : "on";
        messagingTemplate.convertAndSend("/home-device/messages/q", "led" + ledNumber + " " + ledStatus);
        return "Send request to q led" + ledNumber + " "  + ledStatus;
    }
}
