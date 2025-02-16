#include "MqttManager.h"

MqttManager::MqttManager(const char* mqttBroker, int mqttPort)
    : mqttClient(wifiClient), broker(mqttBroker), port(mqttPort) {
}

bool MqttManager::connect() {
    if (!mqttClient.connect(broker, port)) {
        Serial.print("MQTT connection failed! Error code = ");
        Serial.println(mqttClient.connectError());
        return false;
    }
    Serial.println("Connected to MQTT broker");
    return true;
}

void MqttManager::sendAction(const char* topic, const char* message) {
    mqttClient.beginMessage(topic);
    mqttClient.print(message);
    mqttClient.endMessage();
}

void MqttManager::poll() {
    mqttClient.poll();
} 