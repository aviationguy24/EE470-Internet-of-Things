/*
 * ----------------------------------------------
 * Project/Program Name : ESP8266 IoT Node (Switch-triggered)
 * File Name            : ui.cpp
 * Author               : Aaron John Estrada
 * Date                 : 23/10/2025
 * Version              : v1.1
 *
 * Purpose:
 *   Implements a USB Serial menu to choose the IANA time zone.
 * ----------------------------------------------
 */
#include "ui.h"
#include "time_sync.h"

static const char* tzFor(int idx){
  switch(idx){
    case 1: return "America/New_York";       // ET
    case 2: return "America/Chicago";        // CT
    case 3: return "America/Denver";         // MT
    case 4: return "America/Los_Angeles";    // PT (default)
    case 5: return "America/Anchorage";      // AKT
    case 6: return "Pacific/Honolulu";       // HAT
    case 7: return "America/Puerto_Rico";    // AT
    default: return "America/Los_Angeles";
  }
}

void select_time_zone_via_serial(uint32_t timeout_ms){
  Serial.println();
  Serial.println(F("Select Your Time Zone (default is PT):"));
  Serial.println(F(" 1) Eastern (ET)   – New York, NY"));
  Serial.println(F(" 2) Central (CT)   – Chicago, IL"));
  Serial.println(F(" 3) Mountain (MT)  – Denver, CO"));
  Serial.println(F(" 4) Pacific (PT)   – Los Angeles, CA (DEFAULT)"));
  Serial.println(F(" 5) Alaska (AKT)   – Anchorage, AK"));
  Serial.println(F(" 6) Hawaii (HAT)   – Honolulu, HI"));
  Serial.println(F(" 7) Atlantic (AT)  – San Juan, Puerto Rico"));
  Serial.print  (F("-> Enter 1..7 then Enter (or just Enter): "));

  uint32_t t0 = millis();
  int choice = -1;
  while (millis() - t0 < timeout_ms){
    if (Serial.available()){
      int c = Serial.read();
      if (c == '\r' || c == '\n') { break; }
      if (c >= '1' && c <= '7'){ choice = c - '0'; break; }
    }
    delay(10);
  }

  if (choice == -1){
    Serial.println(F("\n(no input) -> Defaulting to PT (4)"));
    choice = 4;
  } else {
    Serial.printf("\nYou selected %d\n", choice);
  }

  const char* tz = tzFor(choice);
  set_time_zone(String(tz));
  Serial.print(F("Time zone set to: "));
  Serial.println(tz);
}
