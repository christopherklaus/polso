#include "WiFiManager.h"
#include "MqttManager.h"
#include "arduino_secrets.h"
#include "config.h"
#include <ArduinoJson.h>
#include <Encoder.h>

// Button setup - using digital pin 2 for the MKR 1010
const int rotaryButtonPin = 2;
int rotaryButtonState = HIGH;
int previousRotaryButtonState = HIGH;

const int togglePin = 5;
int toggleState = LOW;

// Rotary encoder setup - using pins 3 and 4 for the MKR 1010
Encoder encoder(6, 7);
long oldPosition = -999;

// Network managers
WiFiManager wifi(SECRET_SSID, SECRET_PASS);
MqttManager mqtt(MQTT_BROKER, MQTT_PORT);

// Volume control
int currentVolume = 50;  // Start at 50%
const int VOLUME_STEP = 5;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\nStarting up...");
    
    // Setup button pin
    pinMode(rotaryButtonPin, INPUT_PULLUP);
    
    // Connect to WiFi
    if (!wifi.connect()) {
        Serial.println("Failed to connect to WiFi!");
        while (1) delay(1000); // Stop here
    }
    
    // Connect to MQTT
    if (!mqtt.connect()) {
        Serial.println("Failed to connect to MQTT!");
        while (1) delay(1000); // Stop here
    }
    
    Serial.println("Setup complete!");
}

void sendVolumeUpdate(int volume) {
    StaticJsonDocument<200> doc;
    doc["target"] = "music";
    doc["action"] = "volume";
    doc["room"] = "Space";
    doc["value"] = volume;

    char jsonBuffer[200];
    serializeJson(doc, jsonBuffer);
    mqtt.sendAction("control", jsonBuffer);
}

void sendMusicCommand(char* action) {
  StaticJsonDocument<200> doc;
  doc["target"] = "music";
  doc["action"] = action;
  doc["room"] = "Bathroom";

  char jsonBuffer[200];
  serializeJson(doc, jsonBuffer);
  mqtt.sendAction("control", jsonBuffer);
}

void handleRotaryEncoder() {
    long newPosition = encoder.read();
    if (newPosition != oldPosition) {
        // Calculate volume change (4 steps per detent is common)
        int volumeChange = ((newPosition - oldPosition) / 4) * VOLUME_STEP;
        currentVolume = constrain(currentVolume + volumeChange, 0, 100);
        
        sendVolumeUpdate(currentVolume);
        oldPosition = newPosition;
        
        Serial.print("Volume: ");
        Serial.println(currentVolume);
    }
}

void handleButton() {
    rotaryButtonState = digitalRead(rotaryButtonPin);
    
    if (rotaryButtonState != previousRotaryButtonState) {
      Serial.println("Button pressed");
        if (rotaryButtonState == LOW) {  // Button pressed
            // Toggle mute or other action
            currentVolume = (currentVolume > 0) ? 0 : 50;  // Toggle between mute and 50%
            sendVolumeUpdate(currentVolume);
        }
        delay(50);  // Simple debounce
    }
    previousRotaryButtonState = rotaryButtonState;
}

int previousToggleState = LOW;

void handleSwitch() {
  toggleState = digitalRead(togglePin);

  if (toggleState != previousToggleState) {
    Serial.print("Switch switched:");
    Serial.print(previousToggleState);
    Serial.println(toggleState);
    if (toggleState == HIGH) {
      sendMusicCommand("join");
    } else {
      sendMusicCommand("leave");
    }
  }

  previousToggleState = toggleState;
}

void loop() {
    handleRotaryEncoder();
    handleButton();
    handleSwitch();
    mqtt.poll();
    delay(10);  // Small delay to prevent overwhelming the system
}
