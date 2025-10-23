/*
 * ----------------------------------------------
 * Project/Program Name : ESP8266 IoT Node (Switch-triggered)
 * File Name            : switches.h
 * Author               : Aaron John Estrada
 * Date                 : 23/10/2025
 * Version              : v1.1
 *
 * Purpose:
 *   Declarations for switch debounce and edge detection.
 * ----------------------------------------------
 */
#pragma once
#include <Arduino.h>

enum SwitchEvent { SW_NONE = 0, SW_BUTTON, SW_TILT };

void init_switches();
SwitchEvent check_switch();
