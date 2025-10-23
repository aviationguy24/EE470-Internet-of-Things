/*
 * ----------------------------------------------
 * Project/Program Name : ESP8266 IoT Node (Switch-triggered)
 * File Name            : time_sync.cpp
 * Author               : Aaron John Estrada
 * Date                 : 23/10/2025
 * Version              : v1.1
 *
 * Purpose:
 *   Wi-Fi connection and time sync using timeapi.io (HTTPS) with
 *   NTP fallback; supports user-selected IANA time zones.
 * ----------------------------------------------
 */
#include "time_sync.h"
#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <time.h>

static String   g_isoTime   = "";
static uint32_t g_lastFetch = 0;
static String   g_timeZone  = TIMEZONE; // starts from default in config.h

void set_time_zone(const String& tz_iana){ g_timeZone = tz_iana; }
String get_time_zone(){ return g_timeZone; }

static bool waitForIP(uint32_t ms = 20000){
  uint32_t t0 = millis();
  while (millis() - t0 < ms){
    if (WiFi.status()==WL_CONNECTED){
      IPAddress ip = WiFi.localIP();
      if (ip[0] | ip[1] | ip[2] | ip[3]) return true;
    }
    delay(200);
  }
  return false;
}

static bool httpBeginAuto(HTTPClient& http, const String& url) {
  if (url.startsWith("https://")) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();  // demo (no cert validation)
    return http.begin(*client, url);
  } else {
    WiFiClient client;
    return http.begin(client, url);
  }
}

void connect_wifi() {
  Serial.printf("Connecting Wi-Fi: %s\n", WIFI_SSID);
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(250);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  if (WiFi.waitForConnectResult(25000) != WL_CONNECTED) {
    Serial.println("Wi-Fi FAILED.");
    return;
  }
  if (!waitForIP(8000)) {
    Serial.println("Connected but IP unset; retrying DHCP…");
    WiFi.disconnect(); delay(400);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    if (WiFi.waitForConnectResult(15000) != WL_CONNECTED || !waitForIP(8000)) {
      Serial.println("Still no IP.");
      return;
    }
  }
  Serial.printf("Wi-Fi OK, IP=%s RSSI=%d dBm\n",
                WiFi.localIP().toString().c_str(), WiFi.RSSI());
}

void read_time(){
  if (WiFi.status()!=WL_CONNECTED) return;

  HTTPClient https;
  int code=-1; String payload;

  // HTTPS (timeapi.io) for selected IANA zone
  {
    String url = String("https://timeapi.io/api/Time/current/zone?timeZone=") + g_timeZone;
    if (httpBeginAuto(https, url)) {
      https.setTimeout(12000);
      code = https.GET();
      if (code==200) payload = https.getString();
      https.end();
    }
  }
  if (code==200 && payload.length()){
    int p = payload.indexOf("\"dateTime\":\"");
    if (p>0){
      int start = p + 12;
      int end   = payload.indexOf('"', start);
      if (end>start){
        g_isoTime   = payload.substring(start,end);
        g_lastFetch = millis();
        Serial.println("Time(HTTPS) = " + g_isoTime + "  TZ=" + g_timeZone);
        return;
      }
    }
    Serial.println("Time JSON parse error");
  }

  // NTP fallback (UTC)
  Serial.println("HTTPS time failed; using NTP (UTC) fallback…");
  configTime(0,0,"pool.ntp.org","time.nist.gov");
  uint32_t t0 = millis(); time_t now = 0;
  while ((now=time(nullptr)) < 1700000000 && millis()-t0 < 15000) delay(300);
  if (now >= 1700000000){
    struct tm* tm_utc = gmtime(&now);
    char buf[24]; strftime(buf,sizeof(buf),"%Y-%m-%dT%H:%M:%SZ", tm_utc);
    g_isoTime = String(buf);
    g_lastFetch = millis();
    Serial.println("Time(NTP)  = " + g_isoTime);
  } else {
    Serial.println("NTP failed too.");
  }
}

void read_time_if_due(){
  if (millis() - g_lastFetch > TIME_REFRESH_MS) read_time();
}

const String& get_time_str(){ return g_isoTime; }

String mysql_time_str(){
  String s = g_isoTime;
  if (s.length() == 0) return "";
  s.replace('T',' ');
  int dot = s.indexOf('.'); if (dot > 0) s = s.substring(0, dot);
  int z   = s.indexOf('Z'); if (z   > 0) s = s.substring(0, z);
  int plus  = s.indexOf('+');
  int minus = s.indexOf('-', 11);
  int cut = s.length();
  if (plus  > 0 && plus  < cut) cut = plus;
  if (minus > 0 && minus < cut) cut = minus;
  s = s.substring(0, cut);
  s.trim();
  return s; // "YYYY-MM-DD HH:MM:SS"
}
