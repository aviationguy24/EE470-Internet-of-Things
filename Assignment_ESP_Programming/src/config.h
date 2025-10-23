/*
 * ----------------------------------------------
 * Project/Program Name : ESP8266 IoT Node (Switch-triggered)
 * File Name            : config.h
 * Author               : Aaron John Estrada
 * Date                 : 23/10/2025
 * Version              : v1.1
 *
 * Purpose:
 *   Global pins, credentials, URLs, and constants.
 *
 * Inputs : None
 * Outputs: Constants for all modules.
 *
 * Example Application:
 *   Central place to edit Wi-Fi creds and default timezone.
 *
 * Dependencies: None
 * Usage Notes : Change values here—no need to touch logic files.
 * ----------------------------------------------
 */
#pragma once

// Pins / sensors
#define PIN_DHT    D1
#define DHTTYPE    DHT11
#define PIN_BTN    D6
#define PIN_TILT   D7
#define BTN_ACTIVE_HIGH   0
#define TILT_ACTIVE_HIGH  1
#define LDR_PIN    A0

// Wi-Fi
#define WIFI_SSID  "esp8266test"
#define WIFI_PASS  "12345678"

// Server
#define SERVER_URL "https://aaronjohnestrada.com/insert.php"

// Time
#define TIMEZONE   "America/Los_Angeles"   // default; user can change via Serial
#define TIME_REFRESH_MS  120000UL

// Behavior
#define ALLOW_DUMMY_SEND 0
#define DEBOUNCE_MS      250UL
