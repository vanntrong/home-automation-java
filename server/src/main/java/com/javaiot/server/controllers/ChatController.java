package com.javaiot.server.controllers;

import org.springframework.messaging.handler.annotation.DestinationVariable;
import org.springframework.messaging.handler.annotation.MessageMapping;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class ChatController {
    private final SimpMessagingTemplate messagingTemplate;

    public ChatController(SimpMessagingTemplate messagingTemplate) {
        this.messagingTemplate = messagingTemplate;
    }

    @MessageMapping("/chat/{to}")
    public void sendMessage(@DestinationVariable String to, String message) {
        System.out.println(message);
        messagingTemplate.convertAndSend("/topic/messages/" + to, message);
    }

    @GetMapping("/leds")
    String sendLed(@RequestParam("status") int status) {
        String ledStatus = status == 0 ? "off" : "on";
        messagingTemplate.convertAndSend("/topic/messages/q", "led1 on");
        return "Send request to q led1 on";
    }
}
