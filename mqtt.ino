#define MQTT_BUFFER_SIZE 200

void onMqttMessage(int messageSize) {
  StaticJsonDocument<MQTT_BUFFER_SIZE> doc;
  char mqttMessage[MQTT_BUFFER_SIZE];
  for (int i = 0; i < messageSize; i++) {
      mqttMessage[i] = (char)mqttClient.read();
  }
  mqttMessage[messageSize] = '\0'; // Null-terminate the C-string to properly format it
  
  DeserializationError error = deserializeJson(doc, mqttMessage);

  if (error) {
    Serial.print("deserializeJson() failed with code ");
    Serial.println(error.c_str());
    return;
  }

  // Get the name and value
  const char* name = doc["name"]; 
  int updatedCurrentValue = doc["currentValue"]; 

  Serial.print("Receiving request to update value of ");
  Serial.println(name);

  Page* pageToUpdate = getPageByName(pages, name, pagesLength);
  setValue(pageToUpdate, updatedCurrentValue);
}

void updatePageInMQTT(Page* currentPage) {
  StaticJsonDocument<MQTT_BUFFER_SIZE> doc;
  doc["name"] = currentPage->name;
  doc["currentValue"] = currentPage->currentValue;
  char updatedPage[MQTT_BUFFER_SIZE];
  serializeJson(doc, updatedPage, sizeof(updatedPage));

  char buffer[50];
  sprintf(buffer, "%s/button", topic);
  mqttClient.beginMessage(buffer);
  mqttClient.print(updatedPage);
  mqttClient.endMessage();
}