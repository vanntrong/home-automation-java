package com.javaiot.server.models;

public class DeviceModel {
    private String id;
    private String name;
    private String status;
    private String type;
    private Long pin;

    public DeviceModel(String id, String name, String status, String type, Long pin) {
        this.id = id;
        this.name = name;
        this.status = status;
        this.type = type;
        this.pin = pin;
    }

    public DeviceModel(String name, String status, String type, Long pin) {
        this.name = name;
        this.status = status;
        this.type = type;
        this.pin = pin;
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getStatus() {
        return status;
    }

    public void setStatus(String status) {
        this.status = status;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public Long getPin() {
        return pin;
    }

    public void setPin(Long pin) {
        this.pin = pin;
    }
}
