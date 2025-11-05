#include "AWS-IoT.h"
#include "secrets.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <time.h>

bool AWSConnected = false;

static WiFiClientSecure net;
static MQTTClient client(512);

static void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("⏱  Syncing time");
  time_t now = time(nullptr);
  int retries = 0;
  while (now < 8 * 3600 * 2 && retries < 30) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    retries++;
  }
  Serial.println();
  delay(2000); // <-- Add this extra wait
}


static void messageHandler(String &topic, String &payload) {
  Serial.println("📩 Incoming from AWS:");
  Serial.print("  Topic: ");   Serial.println(topic);
  Serial.print("  Payload: "); Serial.println(payload);
}

void SetupAWS() {
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  setClock();

  client.begin(AWS_IOT_ENDPOINT, 8883, net);
  client.onMessage(messageHandler);

  Serial.println("AWS IoT initialized.");
}

static void mqttConnect() {
  Serial.print("🔐 Connecting TLS+MQTT to AWS...");
  if (client.connect(THINGNAME)) {
    Serial.println(" connected ✅");
    client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
    AWSConnected = true;
  } else {
    Serial.print(" failed (state=");
    Serial.print(client.lastError());
    Serial.println(")");
    AWSConnected = false;
  }
}

void CheckAWS() {
  if (!client.connected()) {
    AWSConnected = false;
    mqttConnect();
  } else {
    AWSConnected = true;
    client.loop();
  }
}

void PublicaTag(const String& tagID) {
  StaticJsonDocument<256> doc;
  doc["device"] = THINGNAME;
  doc["uid"]    = tagID;
  doc["ts"]     = (uint32_t)millis();

  char payload[256];
  size_t n = serializeJson(doc, payload, sizeof(payload));

  Serial.print("📤 Publish → ");
  Serial.print(AWS_IOT_PUBLISH_TOPIC);
  Serial.print(": ");
  Serial.println(payload);

  if (!client.publish(AWS_IOT_PUBLISH_TOPIC, payload, n)) {
    Serial.println("⚠️  Publish failed");
  }
}

