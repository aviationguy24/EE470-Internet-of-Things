// ============================================================================
// File        : esp8266_mqtt_relay.ino
// Author      : Aaron John Estrada
// Date        : November 24, 2025
// Course/Proj : MQTT Relay Node + ESP8266 (Potentiometer + Switch + LED Control)
// Board       : ESP8266 NodeMCU
//
// Description :
//   Firmware for ESP8266 that connects to the public HiveMQ broker,
//   publishes potentiometer values every 15 seconds, listens to LED
//   control commands, and publishes switch events (1 on release, then
//   0 after 5 seconds). Designed for use with a remote relay node that
//   subscribes and stores data in a Hostinger database.
//
// ============================================================================

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WIFI SETTINGS 
const char* ssid     = "esp8266test";
const char* password = "12345678";

// MQTT SETTINGS
const char* mqtt_server = "broker.hivemq.com";
const int   mqtt_port   = 1883;
const char* potTopic    = "testtopic/temp/outTopic/estrada123";      // pot publish
const char* switchTopic = "testtopic/temp/outTopic/estrada123_sw";   // switch publish
const char* ledTopic    = "testtopic/temp/inTopic";                  // LED commands

// PIN ASSIGNMENTS
const int POT_PIN    = A0;   // Potentiometer wiper
const int LED_PIN    = D5;   // LED + resistor
const int SWITCH_PIN = D6;   // Push button to GND (INPUT_PULLUP)

// GLOBALS
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastPotPublish   = 0;
const unsigned long potPeriod  = 15000;  // 15 seconds

bool lastSwitchPressed   = false;
bool waitingToSendZero   = false;
unsigned long zeroSendAt = 0;


// MQTT CALLBACK to handle incoming messages
void callback(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (unsigned int i = 0; i < length; i++) {
        msg += (char)payload[i];
    }

    Serial.print("MQTT [");
    Serial.print(topic);
    Serial.print("] ");
    Serial.println(msg);

    // LED Control --------
    if (String(topic) == ledTopic) {
        if (msg == "1") {
            digitalWrite(LED_PIN, HIGH);
            Serial.println("LED -> ON (MQTT)");
        } else if (msg == "0") {
            digitalWrite(LED_PIN, LOW);
            Serial.println("LED -> OFF (MQTT)");
        }
    }
}


// WiFi SETUP
void setup_wifi() {
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}


// MQTT RECONNECT (also subscribes to LED topic)
void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection... ");

        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);

        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
            client.subscribe(ledTopic);  // Subscribe to LED control
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" -> retry in 5 seconds");
            delay(5000);
        }
    }
}


// SETUP
void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    pinMode(SWITCH_PIN, INPUT_PULLUP);  // button to GND

    setup_wifi();

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}


// LOOP
void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    unsigned long now = millis();

    // Section B: Publish potentiometer every 15s 
    if (now - lastPotPublish >= potPeriod) {
        lastPotPublish = now;

        int potVal = analogRead(POT_PIN);
        char buf[12];
        snprintf(buf, sizeof(buf), "%d", potVal);

        Serial.print("Publishing pot value: ");
        Serial.println(buf);

        client.publish(potTopic, buf);
    }

    // Section C-2: Switch press -> 1 now, 0 after 5s 
    bool pressedNow = (digitalRead(SWITCH_PIN) == LOW);  // active-low

    // Detect release edge: was pressed, now not pressed
    if (!pressedNow && lastSwitchPressed) {
        Serial.println("Switch released -> send 1, schedule 0 in 5s");

        client.publish(switchTopic, "1");

        waitingToSendZero = true;
        zeroSendAt = now + 5000;  // 5 seconds from now
    }

    // Time to send 0?
    if (waitingToSendZero && (long)(now - zeroSendAt) >= 0) {
        client.publish(switchTopic, "0");
        Serial.println("Switch timeout -> send 0");
        waitingToSendZero = false;
    }

    lastSwitchPressed = pressedNow;
}
