#include "homefans.h"

#define LOGGING_TOPIC BASE_TOPIC "/log"

void setup_wifi() {
  delay(10);

  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  randomSeed(micros());
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void transmitState(int fanId, char* attr, char* payload) {
  ELECHOUSE_cc1101.SetTx();               // set Transmit on
  mySwitch.disableReceive();              // Receiver off
  mySwitch.enableTransmit(TX_PIN);        // Transmit on
  mySwitch.setRepeatTransmit(RF_REPEATS); // set RF code repeat
  mySwitch.setProtocol(RF_PROTOCOL);      // send Received Protocol
  mySwitch.setPulseLength(320);           // modify this if required
  
  // Generate and send the RF payload to the fan
  int rfCommand = generateCommand(fanId, attr, payload);
  mySwitch.send(rfCommand, 24);
  
  #if DEBUG_MODE
    Serial.print("(RF) OUT [protocol: ");
    Serial.print(RF_PROTOCOL);
    Serial.print("] [repeats: ");
    Serial.print(RF_REPEATS);
    Serial.print("] [frequency: ");
    Serial.print(RF_FREQUENCY);
    Serial.print("] [fan: ");
    Serial.print(fanId);
    Serial.print("] [attr: ");
    Serial.print(attr);
    Serial.print("] [payload: ");
    Serial.print(payload);
    Serial.print("] ");
    Serial.print(rfCommand);
    Serial.println();
  #endif
  
  ELECHOUSE_cc1101.SetRx();               // set Receive on
  mySwitch.disableTransmit();             // set Transmit off
  mySwitch.enableReceive(RX_PIN);         // Receiver on
  
  postStateUpdate(fanId);
}

void callback(char* topic, byte* payload, unsigned int length) {
  #if DEBUG_MODE
    Serial.print("(MQTT) IN [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
  #endif
  
  char payloadChar[length + 1];
  sprintf(payloadChar, "%s", payload);
  payloadChar[length] = '\0';
  mqttLog("(MQTT) IN [" + String(topic) + "] " + String(payloadChar));
  
  // Get ID after the base topic + a slash
  char id[5];
  memcpy(id, &topic[sizeof(BASE_TOPIC)], 4);
  id[4] = '\0';
  if(strspn(id, idchars)) {
    uint8_t idint = strtol(id, (char**) NULL, 2);
    char *attr;
    char *action;
    // Split by slash after ID in topic to get attribute and action
    attr = strtok(topic+sizeof(BASE_TOPIC) + 5, "/");
    action = strtok(NULL, "/");
    // Convert payload to lowercase
    for(int i=0; payloadChar[i]; i++) {
      payloadChar[i] = tolower(payloadChar[i]);
    }
    
    // Sync tracked fan states based on the incomming MQTT message
    if(strcmp(attr, "on") == 0) { // Fan Power State (On/Off)
      if(strcmp(payloadChar, "on") == 0) {
        fans[idint].fanState = true;
      } else if(strcmp(payloadChar, "off") == 0) {
        fans[idint].fanState = false;
      }
    } else if(strcmp(attr, "speed") == 0) { // Fan Speed (low, med, high, off)
      if(strcmp(payloadChar, "low") == 0) {
        fans[idint].fanSpeed = FAN_LOW;
        fans[idint].fanState = true;
      } else if(strcmp(payloadChar, "medium") == 0) {
        fans[idint].fanSpeed = FAN_MED;
        fans[idint].fanState = true;
      } else if(strcmp(payloadChar, "high") == 0) {
        fans[idint].fanSpeed = FAN_HI;
        fans[idint].fanState = true;
      } else if(strcmp(payloadChar, "off") == 0) {
        fans[idint].fanSpeed = 0;
        fans[idint].fanState = false;
      }
    } else if(strcmp(attr, "light") == 0) { // Fan Light State (On/Off)
      if(strcmp(payloadChar, "on") == 0) {
        fans[idint].lightState = true;
      } else if(strcmp(payloadChar, "off") == 0) {
        fans[idint].lightState = false;
      }
    }
    
    if(strcmp(action, "set") == 0) {
      transmitState(idint, attr, payloadChar);
    }
  }
  else {
    // Invalid ID
    return;
  }
}

void postStateUpdate(int id) {
  char outTopic[100];
  
  // Publish "Fan Power" state
  sprintf(outTopic, "%s/%s/on/state", BASE_TOPIC, idStrings[id]);
  client.publish(outTopic, fans[id].fanState ? "on":"off", true);
  #if DEBUG_MODE
    Serial.print("(MQTT) OUT [");
    Serial.print(outTopic);
    Serial.print("] ");
    Serial.print(fans[id].fanState ? "on":"off");
    Serial.println();
  #endif
  
  // Publish "Fan Speed" state
  sprintf(outTopic, "%s/%s/speed/state", BASE_TOPIC, idStrings[id]);
  client.publish(outTopic, fanStateTable[fans[id].fanSpeed], true);
  #if DEBUG_MODE
    Serial.print("(MQTT) OUT [");
    Serial.print(outTopic);
    Serial.print("] ");
    Serial.print(fanStateTable[fans[id].fanSpeed]);
    Serial.println();
  #endif
  
  // Publish "Fan Light" state
  sprintf(outTopic, "%s/%s/light/state", BASE_TOPIC, idStrings[id]);
  client.publish(outTopic, fans[id].lightState ? "on":"off", true);
  #if DEBUG_MODE
    Serial.print("(MQTT) OUT [");
    Serial.print(outTopic);
    Serial.print("] ");
    Serial.print(fans[id].lightState ? "on":"off");
    Serial.println();
  #endif
}

void mqttLog(char* message) {
  client.publish(LOGGING_TOPIC, message, true);
}

void mqttLog(String message) {
  char charBuf[256];
  message.toCharArray(charBuf, 256);
  client.publish(LOGGING_TOPIC, charBuf);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_NAME, MQTT_USER, MQTT_PASS, STATUS_TOPIC, 0, true, "offline")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(STATUS_TOPIC, "online", true);
      // ... and resubscribe
      client.subscribe(SUBSCRIBE_TOPIC_ON_SET);
      client.subscribe(SUBSCRIBE_TOPIC_ON_STATE);
      client.subscribe(SUBSCRIBE_TOPIC_SPEED_SET);
      client.subscribe(SUBSCRIBE_TOPIC_SPEED_STATE);
      client.subscribe(SUBSCRIBE_TOPIC_LIGHT_SET);
      client.subscribe(SUBSCRIBE_TOPIC_LIGHT_STATE);
      mqttLog("Connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

int generateCommand(int fanId, char* attr, char* payload) {
  int baseCommand = 0b111111000110000000000000;
  int fanIdDips   = (fanId ^ 0b1111) << 8;
  int commandInt  = 0b01110000;
  int command     = 0b0000;

  if(strcmp(attr, "speed") == 0) { // Handle "Fan Speed" commands
    if(strcmp(payload, "low") == 0) {
      command = 0b0110;
    } else if(strcmp(payload, "medium") == 0) {
      command = 0b0101;
    } else if(strcmp(payload, "high") == 0) {
      command = 0b0100;
    } else if(strcmp(payload, "off") == 0) {
      command = 0b0111;
    }
  } else if(strcmp(attr, "light") == 0) { // Handle "Fan Light" commands
    if(strcmp(payload, "on") == 0) {
      command = 0b0010;
    } else if(strcmp(payload, "off") == 0) {
      command = 0b0001;
    }
  } else if(strcmp(attr, "on") == 0) { // Handle "Fan Power" commands
    if(strcmp(payload, "on") == 0) {
      command = 0b1110;
    } else if(strcmp(payload, "off") == 0) {
      command = 0b0111;
    }
  } else { // Handle all other commands (i.e. unknown commands)
    Serial.print("Unsupported command: (attr: ");
    Serial.print(attr);
    Serial.print(") ");
    Serial.println(payload);
  }

  // Combine all values together to create our final command
  int finalCommand = baseCommand + fanIdDips + commandInt + command;
  return finalCommand;
}

void setup() {
  Serial.begin(9600);
  
  // initialize fan struct
  for(int i=0; i<16; i++) {
    fans[i].lightState = false;
    fans[i].fanState = false;
    fans[i].fanSpeed = 0;
  }
  
  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setMHZ(RF_FREQUENCY);
  ELECHOUSE_cc1101.SetRx();
  mySwitch.enableReceive(RX_PIN);
  
  setup_wifi();

  // https://github.com/esp8266/Arduino/blob/master/libraries/ArduinoOTA/ArduinoOTA.h
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  ArduinoOTA.setPort(8266);
  ArduinoOTA.begin();
  
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(callback);
}

void loop() {
  // Allow for OTA updating
  ArduinoOTA.handle();
  
  // Ensure that the MQTT client is connected
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Handle received rf-transmissions
  if (mySwitch.available()) {
    value               = mySwitch.getReceivedValue();        // int value of rf command
    prot                = mySwitch.getReceivedProtocol();     // transmission protocol used (expect 11)
    bits                = mySwitch.getReceivedBitlength();    // transmission bit length (expect 24)
    int subtractedValue = value - 0b111111000110000000000000; // zero out the common "1111  1100  0110" from the command
    int truncatedValue  = subtractedValue >> 8;               // shift out the subtracted values (4 bits remaining)
    int id              = truncatedValue ^0b1111;             // invert "truncatedValue" to get remote ID (0-15)
    
    // Ensure that the protocol and bit-length are what we expect to see
    if( prot == 11 && bits == 24 ) {
      // Remove the first and last 4 bits to get the "cmdMode" which is either:
      //  (7) Normal Commands (On / Off / Speed / Light)
      //  (6) Light Temperature Commands - not covered at the moment
      // Use a modulus (% 6) to convert it to a 1 or 0 to use as a bool when required
      int cmdMode = ((subtractedValue & 0b000011110000) >> 4) % 6;

      // Strip the first 8 bits from the "subtractedValue" above to be left with the command
      //  - 1110 (14) On from OFF        | WHEN: cmdMode: 1
      //  - 0110 (6)  Fan Speed 1        | WHEN: cmdMode: 1 - DONE
      //  - 0101 (5)  Fan speed 2        | WHEN: cmdMode: 1 - DONE
      //  - 0100 (4)  Fan Speed 3        | WHEN: cmdMode: 1 - DONE
      //  - 0001 (1)  Light Off          | WHEN: cmdMode: 1 - DONE
      //  - 0010 (2)  Light On           | WHEN: cmdMode: 1 - DONE
      //  - 1101 (13) Change Light Mode  | WHEN: cmdMode: 0 - (TODO) Need to add support in the future
      //  - 1101 (13) All Off            | WHEN: cmdMode: 1 - DONE
      //  - 0111 (7)  Fan Speed Off      | WHEN: cmdMode: 1 - DONE
      int command = subtractedValue & 0b000000001111;

      // Ensure that the command if for one of our remote IDs (0-15)
      if(id < 16) {
        if(cmdMode == 1) {
          if(command == 7) { // Fan OFF
            fans[id].fanState = false;
          } else if(command == 6) { // Fan Speed 1
            fans[id].fanState = true;
            fans[id].fanSpeed = FAN_LOW;
          } else if(command == 5) { // Fan Speed 2
            fans[id].fanState = true;
            fans[id].fanSpeed = FAN_MED;
          } else if(command == 4) { // Fan Speed 3
            fans[id].fanState = true;
            fans[id].fanSpeed = FAN_HI;
          } else if(command == 1) { // Light OFF
            fans[id].lightState = false;
          } else if(command == 2) { // Light ON
            fans[id].lightState = true;
          } else if(command == 13) { // All Off
            fans[id].lightState = false;
            fans[id].fanState = false;
          } else if(command == 14) { // On From Off
            fans[id].fanState = true;
          }
        } else if(cmdMode == 0) {
          // Need to add handling logic here
        }
        
        postStateUpdate(id);
      }
    }
    
    mySwitch.resetAvailable();
  }
}

// Fin.
