/*
 * ----------------------------------------------
 * Project/Program Name : ESP8266 IoT Node (Switch-triggered)
 * File Name            : net.cpp
 * Author               : Aaron John Estrada
 * Date                 : 23/10/2025
 * Version              : v1.1
 *
 * Purpose:
 *   Implement x-www-form-urlencoded POST of DHT11 data + timestamp.
 * ----------------------------------------------
 */
#include <Arduino.h>
#include "net.h"
#include "config.h"
#include "time_sync.h"

#include <ESP8266WiFi.h>                 // for WiFi, WL_CONNECTED
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>

static bool httpBeginAuto(HTTPClient& http, const String& url) {
  if (url.startsWith("https://")) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    return http.begin(*client, url);
  } else {
    WiFiClient client;
    return http.begin(client, url);
  }
}

static String encPlus(String v){ v.replace(' ', '+'); return v; }

bool transmit(const String& nodeName, float tC, float h){
  if (WiFi.status()!=WL_CONNECTED){ Serial.println("Cannot send (no Wi-Fi)"); return false; }
  String t = mysql_time_str();
  if (t.length()==0){ Serial.println("No time available; not sending"); return false; }

  String body  = "node=" + nodeName;
  body += "&temperature=" + String(tC,2);
  body += "&humidity="    + String(h,2);
  body += "&time="        + encPlus(t);

  Serial.println("POST body: " + body);

  HTTPClient http;
  if (!httpBeginAuto(http, SERVER_URL)){ Serial.println("HTTP begin failed (tx)"); return false; }
  http.setTimeout(10000);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int code = http.POST(body);
  String resp = http.getString();
  Serial.printf("POST %d: %s\n", code, resp.c_str());
  http.end();
  return (code >= 200 && code < 300);
}
