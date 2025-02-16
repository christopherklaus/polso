#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFiNINA.h>

class WiFiManager {
private:
    const char* ssid;
    const char* password;

public:
    WiFiManager(const char* ssid, const char* pass);
    bool connect();
    bool isConnected();
};

#endif 