#include <PololuOLED.h>
#include <WiFiNINA.h>
#include <utility/wifi_drv.h>

#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>

#include "arduino_secrets.h"

uint32_t count = 0;

const int selectButtonPin = 0;
const int leftButtonPin = 1;
const int rightButtonPin = 2;
int selectButtonState = HIGH;
int leftButtonState = HIGH;
int rightButtonState = HIGH;
int selectButtonCounter = 0;

int previousSelectButtonState = HIGH;
int previousLeftButtonState = HIGH;
int previousRightButtonState = HIGH;

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
  { "Hue", true, 0, 100, 50, 0 },
  { "Sonos", true, 0, 100, 20, 1 }
};

int pagesLength = sizeof(pages) / sizeof(pages[0]);

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "6.tcp.eu.ngrok.io";
int port = 10020;
const char topic[] = "status";

const long interval = 8000;
unsigned long previousMillis = 0;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;

void setup() {
  Serial.begin(9600);
  
  pinMode(selectButtonPin, INPUT_PULLUP);
  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);

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
  if (!mqttClient.connect(broker, port)) {
    display.gotoXY(0, 0);
    printDisplay("Fail", "MQTT");
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  mqttClient.onMessage(onMqttMessage);
  char buffer[50];
  sprintf(buffer, "%s/server", topic);
  mqttClient.subscribe(buffer);

  delay(500);
}

void loop() {
  mqttClient.poll();
  selectButtonState = digitalRead(selectButtonPin);
  leftButtonState = digitalRead(leftButtonPin);
  rightButtonState = digitalRead(rightButtonPin);

  Page *currentPage = &pages[currentPageIndex];

  if (previousLeftButtonState != leftButtonState) {
    previousLeftButtonState = leftButtonState;
    if (leftButtonState == LOW) {
      setValue(currentPage, currentPage->currentValue - 1);
    }
    updatePageInMQTT(currentPage);
  }

  if(previousRightButtonState != rightButtonState) {
    previousRightButtonState = rightButtonState;
    if (rightButtonState == LOW) {
      setValue(currentPage, currentPage->currentValue + 1);
    }
    updatePageInMQTT(currentPage);
  }

  if (previousSelectButtonState != selectButtonState) {
    // an actual button press happened
    previousSelectButtonState = selectButtonState;
    if (selectButtonState == LOW) {
      // button is pressed
      selectButtonCounter++;
      Serial.print("Switching page to ");
      Serial.println(currentPage->name);

      currentPageIndex = selectButtonCounter%2;
    } 
  }

  String currentValueDisplayOutput = (String)currentPage->currentValue + "%";
  printDisplay(currentPage->name, currentValueDisplayOutput);
  count++;
}
