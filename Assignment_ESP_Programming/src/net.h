/*
 * ----------------------------------------------
 * Project/Program Name : ESP8266 IoT Node (Switch-triggered)
 * File Name            : net.h
 * Author               : Aaron John Estrada
 * Date                 : 23/10/2025
 * Version              : v1.1
 *
 * Purpose:
 *   Declaration for HTTP POST transmit.
 * ----------------------------------------------
 */
#pragma once
#include <Arduino.h>
bool transmit(const String& nodeName, float tC, float h);
