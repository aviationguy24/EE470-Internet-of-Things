/*
 * ----------------------------------------------
 * Project/Program Name : ESP8266 IoT Node (Switch-triggered)
 * File Name            : ui.h
 * Author               : Aaron John Estrada
 * Date                 : 23/10/2025
 * Version              : v1.1
 *
 * Purpose:
 *   Serial UI for user time-zone selection.
 * ----------------------------------------------
 */
#pragma once
#include <Arduino.h>
void select_time_zone_via_serial(uint32_t timeout_ms = 15000);
