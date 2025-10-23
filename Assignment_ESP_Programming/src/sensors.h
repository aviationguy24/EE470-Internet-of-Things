/*
 * ----------------------------------------------
 * Project/Program Name : ESP8266 IoT Node (Switch-triggered)
 * File Name            : sensors.h
 * Author               : Aaron John Estrada
 * Date                 : 23/10/2025
 * Version              : v1.1
 *
 * Purpose:
 *   DHT11 interface declarations.
 * ----------------------------------------------
 */
#pragma once
void init_sensors();
bool read_sensor_1(float& tC, float& h);
bool read_sensor_2(float& tC, float& h);
