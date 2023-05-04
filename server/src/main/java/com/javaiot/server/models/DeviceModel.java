package com.javaiot.server.models;

public class DeviceModel {
    private int id;
    private String name;
    private String status;
    private String type;
    private int pin;

    public DeviceModel(int id, String name, String status, String type, int pin) {
        this.id = id;
        this.name = name;
        this.status = status;
        this.type = type;
        this.pin = pin;
    }

    public DeviceModel(String name, String status, String type, int pin) {
        this.name = name;
        this.status = status;
        this.type = type;
        this.pin = pin;
    }

    public int getId() {
        return id;
    }

    public void setId(int id) {
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

    public int getPin() {
        return pin;
    }

    public void setPin(int pin) {
        this.pin = pin;
    }
}
