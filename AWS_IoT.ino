#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>

#define THINGNAME "ESP32 OM"
//Compte, no pot començar per /
#define AWS_IOT_SUBSCRIBE_TOPIC "iticbcn/esp32om/sub"
#define AWS_IOT_PUBLISH_TOPIC "iticbcn/esp32om/pub"

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

extern bool WifiConnected;
extern bool AWSConnected;

void SetupAWS() {
  // només actualitza l'estat inicial
  AWSConnected = false;
}

void CheckAWS() {
  if (!client.connected()) {
    bool connectat = client.connect(THINGNAME);

    if (connectat) {
      Serial.println(" Connectat a AWS IoT Core!");
      AWSConnected = true;

      client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
      Serial.print("Subscrita al topic: ");
      Serial.println(AWS_IOT_SUBSCRIBE_TOPIC);
    } else {
      Serial.println("Error en la connexió amb AWS IoT!");
      AWSConnected = false;
    }
  } else {
    AWSConnected = true;
    client.loop();
  }
}

void PublicaTag(String tagID) {
  if (AWSConnected) {
    StaticJsonDocument<200> doc;
    doc["tag"] = tagID;
    doc["timestamp"] = millis();

    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);

    if (client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer)) {
      Serial.print(" Publicat a ");
      Serial.print(AWS_IOT_PUBLISH_TOPIC);
      Serial.print(": ");
      Serial.println(jsonBuffer);
    } else {
      Serial.println(" Error en publicar el missatge a AWS");
    }
  } else {
    Serial.println(" No connectat a AWS, no es pot publicar el tag.");
  }
}
