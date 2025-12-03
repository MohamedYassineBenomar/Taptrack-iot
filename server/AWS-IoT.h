#pragma once
#include <Arduino.h>

extern bool AWSConnected;

void SetupAWS();
void CheckAWS();
void PublicaTag(const String& tagID);

// Topics (don’t start with /)
#define THINGNAME "espnode01"
#define AWS_IOT_SUBSCRIBE_TOPIC "iticbcn/espnode01/sub"
#define AWS_IOT_PUBLISH_TOPIC   "iticbcn/espnode01/pub"

