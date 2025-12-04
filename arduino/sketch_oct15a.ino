#include <SPI.h>
#include <MFRC522.h>
#include "secrets.h"
#include "AWS-IoT.h"
#include <WiFi.h>  

// RFID pins
#define SS_PIN  5
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);

// State flags
bool WifiConnected = false;

void SetupWifi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 60) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  WifiConnected = (WiFi.status() == WL_CONNECTED);
  Serial.println(WifiConnected ? "\n✅ WiFi connected!" : "\n❌ WiFi failed");
  if (WifiConnected) {
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }
}













void CheckWifi() {
  WifiConnected = (WiFi.status() == WL_CONNECTED);
}

void SetupRFID() {
  // Explicit SPI pins for ESP32 (VSPI)
  SPI.begin(18, 19, 23, SS_PIN);   // SCK, MISO, MOSI, SS
  SPI.setFrequency(1000000);       // 1 MHz – RC522 likes slower SPI

  rfid.PCD_Init();
  delay(100);

  Serial.println("RC522 init done, checking version...");

  byte v = rfid.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.print("MFRC522 VersionReg: 0x");
  Serial.println(v, HEX);

  if (v == 0x91 || v == 0x92) {
    Serial.println("✅ MFRC522 detected OK");
  } else if (v == 0x00 || v == 0xFF) {
    Serial.println("❌ ERROR: MFRC522 not detected (check wiring, power, or dead module)");
  }

  Serial.println("RC522 ready — scan a card...");
}


bool ReadUID(String &uidOut) {
  if (!rfid.PICC_IsNewCardPresent()) return false;
  if (!rfid.PICC_ReadCardSerial())   return false;

  uidOut = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uidOut += "0";
    uidOut += String(rfid.uid.uidByte[i], HEX);
  }
  uidOut.toUpperCase();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(50);

  SetupWifi();
  SetupRFID();

  if (WifiConnected) {
    SetupAWS();
  }
}

void loop() {
  CheckWifi();
  if (WifiConnected) {
    CheckAWS();
  }

  String uid;
  if (ReadUID(uid)) {
    Serial.print("💳 Card UID: ");
    Serial.println(uid);

    if (WifiConnected && AWSConnected) {
      PublicaTag(uid);
    } else {
      Serial.println("⚠️ Card read but AWS not connected, not publishing");
    }
  }

  delay(50);
}
