#include <PololuOLED.h>
#include <WiFiNINA.h>
#include <utility/wifi_drv.h>

#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>

#include <Encoder.h>

#include "arduino_secrets.h"
#include "config.h"

#include "Arduino.h"

uint32_t count = 0;

const int selectButtonPin = 17; // 0;
const int leftButtonPin = 1;
const int rightButtonPin = 2;
const int upButtonPin = 3;
const int downButtonPin = 5;

const int rotButton = 18;

Encoder myEncoder(16, 17); 

unsigned long previousMillis;
unsigned long currentMillis;

int selectButtonState = HIGH;
int leftButtonState = HIGH;
int rightButtonState = HIGH;
int upButtonState = HIGH;
int downButtonState = HIGH;
int selectButtonCounter = 0;

int previousSelectButtonState = HIGH;
int previousLeftButtonState = HIGH;
int previousRightButtonState = HIGH;
int previousUpButtonState = HIGH;
int previousDownButtonState = HIGH;

struct Page{
  String name;
  bool state;
  int minValue;
  int maxValue;
  int currentValue;
  int position;
};

int currentPageIndex = 0;

PololuSH1106 display(9, 8, 10, 11, 4);

Page pages[2] = {
  { "Lights", true, 0, 100, 50, 0 },
  { "Music", true, 0, 100, 20, 1 }
};

int pagesLength = sizeof(pages) / sizeof(pages[0]);

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = MQTT_BROKER;
int port = MQTT_PORT;
const char topic[] = "status";

const long interval = 8000;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;

long rotationPosition, lastRotationPosition, relativeDelta, lastRotationMillis, lastRotationMillisPosition, duplicationCheckDelta = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(selectButtonPin, INPUT_PULLUP);
  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);
  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);

  pinMode(rotButton, INPUT_PULLUP);

  while (status != WL_CONNECTED) {
    printDisplay("Init", "Wifi");
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    printDisplay("Wifi", ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    delay(1000);
  }

  display.gotoXY(0, 0);
  printDisplay("Init", "MQTT");
  // if (!mqttClient.connect(broker, port)) {
  //   display.gotoXY(0, 0);
  //   printDisplay("Fail", "MQTT");
  //   Serial.print("MQTT connection failed! Error code = ");
  //   Serial.println(mqttClient.connectError());

  //   while (1);
  // }

  // mqttClient.onMessage(onMqttMessage);
  char buffer[50];
  sprintf(buffer, "%s/server", topic);
  // mqttClient.subscribe(buffer);

  delay(500);

  previousMillis = millis();
}

void loop() {
  currentMillis = millis();
  mqttClient.poll();

  Page *currentPage = &pages[currentPageIndex];

  rotationPosition = myEncoder.read();

  // check for last rotation position for debounce

  if (lastRotationMillisPosition != rotationPosition) {
    lastRotationMillis = currentMillis;
  }

  lastRotationMillisPosition = rotationPosition;

  if (lastRotationPosition != rotationPosition) {
    // for some weird reason clockwise and counter clockwise are inverted
    bool direction = lastRotationPosition > rotationPosition;
    Serial.print("current position: ");
    Serial.println(rotationPosition);

    long rotationDelta = abs(lastRotationPosition - rotationPosition) / 4;
    relativeDelta = direction ? rotationDelta : -rotationDelta;

    if (currentMillis - lastRotationMillis > 500) {
      // send this value
      lastRotationPosition = rotationPosition;
      updatePageInMQTT(currentPage);
    }

    if (relativeDelta != duplicationCheckDelta) {
      // not a duplicate from the previous one, continue
      int newValue = currentPage->currentValue + int(relativeDelta);
      int normalizedNewValue = newValue < 0 ? 0 : newValue > 100 ? 100 : newValue;
      setValue(currentPage, normalizedNewValue);
      setState(currentPage, !!normalizedNewValue);

      if (newValue > 100 || newValue < 0) {
        relativeDelta, lastRotationPosition, rotationPosition = 0;
      }
    }
    duplicationCheckDelta = relativeDelta;
  }

  selectButtonState = digitalRead(rotButton);

  // if(previousDownButtonState != downButtonState) {
  //   previousDownButtonState = downButtonState;
  //   if (downButtonState == LOW) {
  //     // down
  //     setValue(currentPage, currentPage->currentValue - INCREASE_INTERVAL);
  //     setState(currentPage, true);
  //     updatePageInMQTT(currentPage);
  //   }
  // }

  // if (previousLeftButtonState != leftButtonState) {
  //   previousLeftButtonState = leftButtonState;
  //   if (leftButtonState == LOW) {
  //     // previous
  //     selectButtonCounter++;
  //     currentPageIndex = selectButtonCounter%2;
  //   }
  // }

  // if(previousRightButtonState != rightButtonState) {
  //   previousRightButtonState = rightButtonState;
  //   if (rightButtonState == LOW) {
  //     // next
  //     selectButtonCounter++;
  //     currentPageIndex = selectButtonCounter%2;
  //   }
  // }

  if (previousSelectButtonState != selectButtonState) {
    // TODO: long press (millis()) joins group of active players
    previousSelectButtonState = selectButtonState;
    if (selectButtonState == LOW) {
      // toggle
      setState(currentPage, !currentPage->state);
      updatePageInMQTT(currentPage);
    } 
  }

  String currentValueDisplayOutput = (String)currentPage->currentValue + "%";
  if (currentPage->state) {
    printDisplay(currentPage->name, currentValueDisplayOutput);
  } else {
    printDisplay(currentPage->name, "off");
  }

  if (currentMillis - previousMillis > DWELL * 1000) {
    // reset back to first page after DWELL (see config) seconds
    Page *currentPage = &pages[0];
    previousMillis = currentMillis;
  }
  count++;
  delay(50);
}
