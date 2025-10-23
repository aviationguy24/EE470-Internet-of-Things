/*
 * ----------------------------------------------
 * Project/Program Name : ESP8266 IoT Node (Switch-triggered)
 * File Name            : main.cpp
 * Author               : Aaron John Estrada
 * Date                 : 23/10/2025
 * Version              : v1.1
 *
 * Purpose:
 *   Main orchestrator. Only calls high-level functions per rubric.
 *
 * Inputs:
 *   D6 pushbutton (node_1), D7 tilt sensor (node_2), USB Serial (TZ select).
 *
 * Outputs:
 *   HTTP POST with temperature, humidity, and timestamp.
 *
 * Example Application:
 *   IoT node that sends DHT11 data only on switch events; user selects
 *   time zone via Serial on startup.
 *
 * Dependencies:
 *   config.h, switches.h, sensors.h, time_sync.h, net.h, ui.h
 *
 * Usage Notes:
 *   Edit Wi-Fi/server in config.h. Open Serial Monitor at 115200 to pick TZ.
 * ----------------------------------------------
 */

#include <Arduino.h>
#include "config.h"
#include "switches.h"
#include "sensors.h"
#include "time_sync.h"
#include "net.h"
#include "ui.h"
#include <ESP8266WiFi.h>


static void check_error(){ /* placeholder for diagnostics */ }

static void read_sensor_1_and_transmit(){
  float tC,h;
  if (!read_sensor_1(tC,h)){
    Serial.println("Bad sensor data; not sending (button)");
    return;
  }
  transmit("node_1", tC, h);
}

static void read_sensor_2_and_transmit(){
  float tC,h;
  if (!read_sensor_2(tC,h)){
    Serial.println("Bad sensor data; not sending (tilt)");
#if ALLOW_DUMMY_SEND
    tC=24.5; h=46.0; Serial.println("Using dummy values for demo (tilt).");
    transmit("node_2", tC, h);
#endif
    return;
  }
  transmit("node_2", tC, h);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nESP8266 Switch-Triggered Node (modular + TZ menu)");
  Serial.print("ESP8266 MAC Address: ");
  Serial.println(WiFi.macAddress());
  init_switches();
  init_sensors();

  // --- user selects time zone over USB Serial (defaults to PT) ---
  select_time_zone_via_serial(15000);

  connect_wifi();
  read_time();
}

void loop() {
  read_time_if_due();

  SwitchEvent ev = check_switch();
  if (ev == SW_BUTTON)      read_sensor_1_and_transmit(); // node_1
  else if (ev == SW_TILT)   read_sensor_2_and_transmit(); // node_2

  check_error();
  delay(5);
}
