#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include "config.h"

// Set receive and transmit pin numbers (GDO0 and GDO2)
#ifdef ESP32 // for esp32! Receiver on GPIO pin 4. Transmit on GPIO pin 2.
  #define RX_PIN 4 
  #define TX_PIN 2
#elif ESP8266  // for esp8266! Receiver on pin 4 = D2. Transmit on pin 5 = D1.
  #define RX_PIN 4
  #define TX_PIN 5
#else // for Arduino! Receiver on interrupt 0 => that is pin #2. Transmit on pin 6.
  #define RX_PIN 0
  #define TX_PIN 6
#endif

// Define fan states
#define FAN_HI  3
#define FAN_MED 2
#define FAN_LOW 1

const char *fanStateTable[4] = {
  "off",
  "low",
  "medium",
  "high"
};

int long value;      // int to save value
int bits;            // int to save bit number
int prot;            // int to save Protocol number

struct fan {
  bool lightState;
  bool fanState;
  uint8_t fanSpeed;
};

fan fans[16];

const char *idStrings[16] = {
  [ 0] = "0000",
  [ 1] = "0001",
  [ 2] = "0010",
  [ 3] = "0011",
  [ 4] = "0100",
  [ 5] = "0101",
  [ 6] = "0110",
  [ 7] = "0111",
  [ 8] = "1000",
  [ 9] = "1001",
  [10] = "1010",
  [11] = "1011",
  [12] = "1100",
  [13] = "1101",
  [14] = "1110",
  [15] = "1111",
};

char idchars[] = "01";

RCSwitch mySwitch = RCSwitch();
WiFiClient espClient;
PubSubClient client(espClient);
