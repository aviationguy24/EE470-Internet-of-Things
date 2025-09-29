//===============================
// Title: Photoresistor Lux Meter + RGB LED & Buzzer Indicator
//===============================
// Program Detail:
//-------------------------------
// Purpose: Read an ambient light with an HW-486 (LDR) on NodeMCU-ESP12,
//          convert ADC Lux and nd indicate level by RGB color 
//          it also buzz if photoresistor detects darkness.
// Inputs:  - A0  : Analog input from HW-486 (AO pin via voltage divider)
//         - Serial: Added a feature where typing 'b' to run a 5 s buzzer test 
// Outputs: - Serial: Prints LUX (float) once per loop
//         - D6/D7/D8: PWM to RGB LED (common-cathode): R=D6, G=D7, B=D8
//         - D5: Buzzer (HIGH=ON)
// Date:    2025-09-28
// Compiler/Platform: Arduino/PlatformIO (ESP8266 NodeMCU-ESP12)
// Authors:  Osvaldo Ramirez & Aaron John Estrada
// Versions:
//   V1 – Initial Version with
//   V2 - 
//
// File Dependencies:
//   N/A
//
// Hardware Notes:
//   - Sensor: HW-486 photoresistor module (AO → A0). Ensure proper 3V3 scaling.
//   - LED: Common-cathode RGB on D6/D7/D8 (use COMMON_ANODE=false).
//   - Buzzer: Active/passive on D5 (driven as digital HIGH/LOW).
//
// Usage Notes:
//   - We set two reference lux points (L1_LUX and L2_LUX) before calibration.
//===============================

#include <Arduino.h>


/*** === USER PINS === ***/
const int buzzerPin = D5;
const int redPin    = D6;
const int greenPin  = D7;
const int bluePin   = D8;
const int sensorPin = A0;   // AO from HW-486


// LED type
const bool COMMON_ANODE = false;  // false = common-cathode


//
const float L1_LUX = 120.0f;      // your dim phone reading
const float L2_LUX = 3200.0f;     // your bright phone reading
const float DARK_TO_ZERO_LUX = 50.1f;     // ≤ this prints as 0.0 lux
const bool  USE_ROOM_ANCHOR   = false;    // set true to scale room to ROOM_TARGET_LUX
const float ROOM_TARGET_LUX   = 150.0f;   // only used if USE_ROOM_ANCHOR=true


// LED color transition range (blue->red)
const float LUX_MAX_DISPLAY   = 1000.0f;  // red at ~1000 lux


// Auto buzzer threshold
const float LUX_BUZZER_THRESHOLD = 50.0f;


//COLOR/BRIGHTNESS
// Overall constant-ish LED intensity (0..255)
const int FIXED_BRIGHTNESS = 200;


// MODEL: Lux = K_room * (C * ADC^p)
float C_coeff = 1.0f;
float p_exp   = -1.0f;
float K_room  = 1.0f;  // room anchor scaling


// --- helpers ---
static inline int imap(long x,long in_min,long in_max,long out_min,long out_max){
  if(in_max==in_min) return (int)out_min;
  if(x<in_min) x=in_min;
  if(x>in_max) x=in_max;
  return (int)((x-in_min)*(out_max-out_min)/(in_max-in_min)+out_min);
}


void setRGB(uint16_t r,uint16_t g,uint16_t b){
#if defined(ESP8266) || defined(ESP32)
  auto put=[&](int pin,uint16_t v){
    uint16_t pwm = map(v,0,255,0,1023);
    if(COMMON_ANODE) pwm = 1023 - pwm;
    analogWrite(pin,pwm);
  };
#else
  auto put=[&](int pin,uint16_t v){
    uint8_t pwm = (uint8_t)v;
    if(COMMON_ANODE) pwm = 255 - pwm;
    analogWrite(pin,pwm);
  };
#endif
  put(redPin,r); put(greenPin,g); put(bluePin,b);
}
int readADC(int samples=16,int delayMs=2){
  long acc=0;
  for(int i=0;i<samples;i++){ acc += analogRead(sensorPin); delay(delayMs); }
  int adc = acc / samples;
  return constrain(adc,1,1022);
}


// Solve Lux = C * ADC^p from two points: (ADC1,L1), (ADC2,L2)
void solvePowerLaw(int ADC1,float L1,int ADC2,float L2){
  float lnA = log((float)ADC2) - log((float)ADC1);
  float lnL = log(max(L2,0.1f)) - log(max(L1,0.1f));
  if(fabs(lnA)<0.15f || fabs(lnL)<0.15f){
    p_exp   = -1.0f;
    C_coeff = L1 / pow((float)ADC1, p_exp);
  }else{
    p_exp = lnL/lnA;
    if(p_exp> 2.0f) p_exp= 2.0f;
    if(p_exp<-2.0f) p_exp=-2.0f;
    C_coeff = L1 / pow((float)ADC1, p_exp);
  }
}

float lux_raw_from_adc(int adc){
  float lux = C_coeff * pow((float)adc, p_exp);
  if(!isfinite(lux)) lux = 0.02f;
  return max(lux, 0.0001f);
}

float adcToLux(int adc){
  float lux = K_room * lux_raw_from_adc(adc);
  if(!isfinite(lux)) lux = 0.02f;
  // ---- clamp to 0..3000 per your request ----
  lux = constrain(lux, 0.0f, 3000.0f);
  return lux;
}

//Manual buzzer
bool          buzzManualActive = false;
unsigned long buzzOffAtMs      = 0;

void setup(){
  Serial.begin(115200);
  pinMode(buzzerPin,OUTPUT);
  pinMode(redPin,OUTPUT); pinMode(greenPin,OUTPUT); pinMode(bluePin,OUTPUT);

  //  2-point calibration 
  delay(1500); int ADC1 = readADC(32,10);     // place sensor at your ~120 lux
  delay(1500); int ADC2 = readADC(32,10);     // place sensor at your ~3200 lux
  solvePowerLaw(ADC1, L1_LUX, ADC2, L2_LUX);

void loop(){
  // ---- Manual buzzer trigger: 'B' or 'b' = ~5 seconds (non-blocking) ----
  while (Serial.available()){
    char c = Serial.read();
    if (c == 'B' || c == 'b'){
      buzzManualActive = true;
      buzzOffAtMs      = millis() + 5000UL; // 5 seconds
    }
  }
  // ---- Sensor & Lux ----
  int   adc           = readADC();
  float lux_unclamped = adcToLux(adc);

  // Print lux only 
  float lux_print = (lux_unclamped <= DARK_TO_ZERO_LUX) ? 0.0f : lux_unclamped;
  Serial.println(lux_print, 1);

  // ---- RGB: Blue -> Red gradient----
  float t = lux_unclamped / LUX_MAX_DISPLAY; // 0 .. 1 over chosen visual range
  if (t < 0.0f) t = 0.0f;
  if (t > 1.0f) t = 1.0f;
 
  t = t * t * (3.0f - 2.0f * t);  // smoothstep
  int r = (int)(FIXED_BRIGHTNESS * t);            // red grows with light
  int g = 0;
  int b = (int)(FIXED_BRIGHTNESS * (1.0f - t));   // blue fades with light

  r = constrain(r, 0, 255);
  b = constrain(b, 0, 255);
  setRGB(r, g, b);

  // Buzzer control
  // Auto-buzz when very dark (based on printed/clamped lux)
  bool autoBuzz = (lux_print <= LUX_BUZZER_THRESHOLD);
  
  if (buzzManualActive && (long)(millis() - buzzOffAtMs) >= 0){
    buzzManualActive = false;
  }

  digitalWrite(buzzerPin, (autoBuzz || buzzManualActive) ? HIGH : LOW);
  delay(150);
}





