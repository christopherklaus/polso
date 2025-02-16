#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <WiFi.h>
#include <ArduinoMqttClient.h>

class MqttManager {
private:
    WiFiClient wifiClient;
    MqttClient mqttClient;
    const char* broker;
    int port;

public:
    MqttManager(const char* broker, int port);
    bool connect();
    void sendAction(const char* topic, const char* message);
    void poll();
};

#endif 