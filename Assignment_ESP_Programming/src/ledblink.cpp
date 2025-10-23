/*
 * ----------------------------------------------
 * Project/Program Name : ESP8266 LED Blink Demo
 * File Name            : ledblink.cpp
 * Author               : Aaron John Estrada
 * Date                 : 10/22/2025
 *
 * Purpose:
 *   Implements the Blink class defined in ledblink.h. Provides
 *   methods for initializing an LED pin, setting blink rate, and
 *   toggling the LED at non-blocking intervals.
 *
 * Inputs:
 *   Rate (milliseconds) via blinkRate()
 *
 * Outputs:
 *   Digital output toggled on the specified GPIO pin.
 *
 * Example Application:
 *   Blink LED(2);
 *   LED.blinkRate(250);
 *   LED.tick();
 *
 * Dependencies:
 *   Arduino Core for ESP8266
 *
 * Usage Notes:
 *   Uses millis() instead of delay() for non-blocking operation.
 * ----------------------------------------------
 */

#include <Arduino.h>
#include "ledblink.h"

Blink::Blink(int pinNumber)
    : _pin(pinNumber), _rate(500), _lastToggle(0), _state(false) {
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
}

void Blink::blinkRate(int rate_ms) {
  _rate = max(1, rate_ms);
}

void Blink::tick() {
  unsigned long now = millis();
  if (now - _lastToggle >= (unsigned long)_rate) {
    _lastToggle = now;
    _state = !_state;
    digitalWrite(_pin, _state ? HIGH : LOW);
  }
}
