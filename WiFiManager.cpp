#include "WiFiManager.h"

WiFiManager::WiFiManager(const char* wifiSsid, const char* pass)
    : ssid(wifiSsid), password(pass) {
}

bool WiFiManager::connect() {
    // Check if the WiFi module is present
    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("Communication with WiFi module failed!");
        return false;
    }

    // Check firmware version
    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
        Serial.println("Please upgrade the firmware");
    }
    
    WiFi.disconnect();  // Disconnect from any previous connections
    delay(100);  // Short delay to ensure disconnection is complete
    
    Serial.print("Connecting to network: ");
    Serial.println(ssid);
    
    int attempts = 0;
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        Serial.print(".");
        delay(500);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    
    Serial.println("\nFailed to connect to WiFi");
    return false;
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
} 