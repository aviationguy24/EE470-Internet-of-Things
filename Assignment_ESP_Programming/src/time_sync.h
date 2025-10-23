/*
 * ----------------------------------------------
 * Project/Program Name : ESP8266 IoT Node (Switch-triggered)
 * File Name            : time_sync.h
 * Author               : Aaron John Estrada
 * Date                 : 23/10/2025
 * Version              : v1.1
 *
 * Purpose:
 *   Wi-Fi connect and time sync (HTTPS with NTP fallback) + mutable TZ.
 * ----------------------------------------------
 */
#pragma once
#include <Arduino.h>

void   connect_wifi();
void   read_time();
void   read_time_if_due();
const  String& get_time_str();
String mysql_time_str();

// dynamic time-zone control
void   set_time_zone(const String& tz_iana);
String get_time_zone();

// Back-compat aliases
inline void   connectWiFi()   { connect_wifi(); }
inline void   getTime()       { read_time(); }
inline void   getTimeIfDue()  { read_time_if_due(); }
