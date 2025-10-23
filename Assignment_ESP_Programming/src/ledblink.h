/*
 * ----------------------------------------------
 * Project/Program Name : ESP8266 LED Blink Demo
 * File Name            : ledblink.h
 * Author               : Aaron John Estrada
 * Date                 : 10/22/2025
 *
 * Purpose:
 *   Header file declaring the Blink class for controlling an LED
 *   with adjustable blink rate.
 *
 * Inputs:
 *   GPIO pin number specified in constructor.
 *
 * Outputs:
 *   Digital signal toggling the specified pin.
 *
 * ----------------------------------------------
 */

#ifndef LEDBLINK_H
#define LEDBLINK_H

class Blink {
 public:
  explicit Blink(int pinNumber);
  void blinkRate(int rate_ms);
  void tick();

 private:
  int _pin;
  int _rate;
  unsigned long _lastToggle;
  bool _state;
};

#endif
