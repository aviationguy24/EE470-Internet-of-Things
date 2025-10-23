/*
 * ----------------------------------------------
 * Project/Program Name : ESP8266 IoT Node (Switch-triggered)
 * File Name            : sensors.cpp
 * Author               : Aaron John Estrada
 * Date                 : 23/10/2025
 * Version              : v1.1
 *
 * Purpose:
 *   Implements DHT11 reads with small retry loop.
 * ----------------------------------------------
 */
#include "sensors.h"
#include "config.h"
#include <Arduino.h>
#include <DHT.h>

static DHT dht(PIN_DHT, DHTTYPE);

static bool readDHT(float& tC, float& h){
  for (int i=0;i<3;i++){
    tC = dht.readTemperature();  // °C
    h  = dht.readHumidity();     // %
    if (!isnan(tC) && !isnan(h)) return true;
    delay(700);
  }
  return false;
}

void init_sensors(){ dht.begin(); delay(1500); }
bool read_sensor_1(float& tC, float& h){ return readDHT(tC,h); }
bool read_sensor_2(float& tC, float& h){ return readDHT(tC,h); }
