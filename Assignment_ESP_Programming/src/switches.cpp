/*
 * ----------------------------------------------
 * Project/Program Name : ESP8266 IoT Node (Switch-triggered)
 * File Name            : switches.cpp
 * Author               : Aaron John Estrada
 * Date                 : 23/10/2025
 * Version              : v1.1
 *
 * Purpose:
 *   Implements debounced edge detection for pushbutton (D6) and tilt (D7).
 * ----------------------------------------------
 */
#include "switches.h"
#include "config.h"

static bool activeRead(uint8_t pin, bool activeHigh){
  int v = digitalRead(pin);
  return activeHigh ? (v == HIGH) : (v == LOW);
}

void init_switches(){
  pinMode(PIN_BTN,  INPUT_PULLUP); // active LOW
  pinMode(PIN_TILT, INPUT);        // active HIGH
}

SwitchEvent check_switch(){
  static bool lastBtn=false, lastTilt=false;
  static uint32_t dbBtn=0, dbTilt=0;

  bool btnNow  = activeRead(PIN_BTN,  BTN_ACTIVE_HIGH);
  bool tiltNow = activeRead(PIN_TILT, TILT_ACTIVE_HIGH);

  if (btnNow != lastBtn && millis()-dbBtn > DEBOUNCE_MS){
    dbBtn = millis(); lastBtn = btnNow;
    if (btnNow) return SW_BUTTON;
  }
  if (tiltNow != lastTilt && millis()-dbTilt > DEBOUNCE_MS){
    dbTilt = millis(); lastTilt = tiltNow;
    if (tiltNow) return SW_TILT;
  }
  return SW_NONE;
}
