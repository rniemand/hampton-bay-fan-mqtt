#include "homefans.h"

/*
 * Modifications       : General code cleanup & seperation
 *                     : Modified to work with Home Decorations #1001 415 438 fans
 *
 * Known issues        : Somewhat spotty state tracking when codes are sent too quickly
*/

void setup_wifi() {
  delay(10);

  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
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
  // mySwitch.setPulseLength(320);        // modify this if required
  
  // Generate and send the RF payload to the fan
  mySwitch.send(generateCommand(fanId, attr, payload), 24);
  
  ELECHOUSE_cc1101.SetRx();               // set Receive on
  mySwitch.disableTransmit();             // set Transmit off
  mySwitch.enableReceive(RX_PIN);         // Receiver on
  
  postStateUpdate(fanId);
}

void callback(char* topic, byte* payload, unsigned int length) {
  #if MQTT_LOG_MESSAGES
    Serial.print("Message arrived [");
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
    for(int i=0; payloadChar[i]; i++) { payloadChar[i] = tolower(payloadChar[i]); }

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
      } else if(strcmp(payloadChar, "medium") == 0) {
        fans[idint].fanSpeed = FAN_MED;
      } else if(strcmp(payloadChar, "high") == 0) {
        fans[idint].fanSpeed = FAN_HI;
      } else if(strcmp(payloadChar, "off") == 0) {
        fans[idint].fanSpeed = 0;
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
  
  // Publish "Fan Speed" state
  sprintf(outTopic, "%s/%s/speed/state", BASE_TOPIC, idStrings[id]);
  client.publish(outTopic, fanStateTable[fans[id].fanSpeed], true);
  
  // Publish "Fan Light" state
  sprintf(outTopic, "%s/%s/light/state", BASE_TOPIC, idStrings[id]);
  client.publish(outTopic, fans[id].lightState ? "on":"off", true);
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

  if(strcmp(attr, "speed") == 0) {
    if(strcmp(payload, "low") == 0) { command = 0b0110; }
    else if(strcmp(payload, "medium") == 0) { command = 0b0101; }
    else if(strcmp(payload, "high") == 0) { command = 0b0100; }
    else if(strcmp(payload, "off") == 0) { command = 0b0111; }
  }
  else if(strcmp(attr, "light") == 0) {
    if(strcmp(payload, "on") == 0) { command = 0b0010; }
    else if(strcmp(payload, "off") == 0) { command = 0b0001; }
  }
  else {
    Serial.print("Unsupported command: (attr: ");
    Serial.print(attr);
    Serial.print(") ");
    Serial.println(payload);
  }

  int finalCommand = baseCommand + fanIdDips + commandInt + command;

#if LOG_OUTGOING_COMMANDS
  Serial.print("(INFO) Generated Command: ");
  Serial.println(finalCommand);
#endif

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
  ELECHOUSE_cc1101.setMHZ(FREQUENCY);
  ELECHOUSE_cc1101.SetRx();
  mySwitch.enableReceive(RX_PIN);

  setup_wifi();
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) { reconnect(); }
  client.loop();

  // Handle received transmissions
  if (mySwitch.available()) {
    value            =  mySwitch.getReceivedValue();       // save received Value
    prot             = mySwitch.getReceivedProtocol();     // save received Protocol
    bits             = mySwitch.getReceivedBitlength();    // save received Bitlength
    int subResult    = value - 0b111111000110000000000000;
    int shift8_1     = subResult >> 8;
    int id           = shift8_1 ^0b1111;

    if( prot == 11 && bits == 24 ) {
      int intMode = ((subResult & 0b000011110000) >> 4) % 6; // 1=Normal | 0=Light command
      int command = subResult & 0b000000001111;
      
      if(id < 16) {
        if(intMode == 1) {
          if(command == 7) { // Fan OFF
            fans[id].fanState = false;
            fans[id].fanSpeed = 0;
          }
          if(command == 6) { // Fan Speed 1
            fans[id].fanState = true;
            fans[id].fanSpeed = FAN_LOW;
          }
          if(command == 5) { // Fan Speed 2
            fans[id].fanState = true;
            fans[id].fanSpeed = FAN_MED;
          }
          if(command == 4) { // Fan Speed 3
            fans[id].fanState = true;
            fans[id].fanSpeed = FAN_HI;
          }
          if(command == 1) { // Light OFF
            fans[id].lightState = false;
          }
          if(command == 2) { // Light ON
            fans[id].lightState = true;
          }
          if(command == 13) { // All Off
            fans[id].lightState = false;
            fans[id].fanState = false;
            fans[id].fanSpeed = 0;
          }
        }
        
        postStateUpdate(id);
      }
    }
    
    mySwitch.resetAvailable();
  }
}
