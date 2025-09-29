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
//   V1 – Initial Version with lenghty lines of code
//   V2 - Shortened Version 
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

// Pinouts
const int buzzerPin = D5;
const int redPin    = D6;
const int greenPin  = D7;
const int bluePin   = D8;
const int sensorPin = A0;

//Config
const bool COMMON_ANODE = false;
const float L1_LUX = 120.0f, L2_LUX = 3200.0f;
const float DARK_TO_ZERO_LUX = 50.1f;
const float LUX_MAX_DISPLAY = 1000.0f;
const float LUX_BUZZER_THRESHOLD = 50.0f;
const int   FIXED_BRIGHTNESS = 200;

// Coefficient Models
float C_coeff = 1.0f, p_exp = -1.0f, K_room = 1.0f;

//RGB
void setRGB(uint16_t r,uint16_t g,uint16_t b){
  auto put=[&](int pin,uint16_t v){
#if defined(ESP8266) || defined(ESP32)
    uint16_t pwm = map(v,0,255,0,1023);
    if(COMMON_ANODE) pwm = 1023 - pwm;
#else
    uint8_t pwm = v;
    if(COMMON_ANODE) pwm = 255 - pwm;
#endif
    analogWrite(pin,pwm);
  };
  put(redPin,r); put(greenPin,g); put(bluePin,b);
}

// Sensor Read
int readADC(int samples=16,int delayMs=2){
  long acc=0;
  for(int i=0;i<samples;i++){ acc += analogRead(sensorPin); delay(delayMs); }
  return constrain(acc/samples,1,1022);
}

// 2 pt power law calibration
void solvePowerLaw(int ADC1,float L1,int ADC2,float L2){
  float lnA = log((float)ADC2) - log((float)ADC1);
  float lnL = log(max(L2,0.1f)) - log(max(L1,0.1f));
  if(fabs(lnA)<0.15f || fabs(lnL)<0.15f){
    p_exp=-1.0f; C_coeff=L1/pow((float)ADC1,p_exp);
  }else{
    p_exp=constrain(lnL/lnA,-2.0f,2.0f);
    C_coeff=L1/pow((float)ADC1,p_exp);
  }
}

// ADC to Lux
float lux_raw_from_adc(int adc){ return max(C_coeff*pow((float)adc,p_exp),0.0001f); }
float adcToLux(int adc){ return constrain(K_room*lux_raw_from_adc(adc),0.0f,3000.0f); }

// Buzzer state
bool buzzManualActive=false; unsigned long buzzOffAtMs=0;

// setup
void setup(){
  Serial.begin(115200);
  pinMode(buzzerPin,OUTPUT);
  pinMode(redPin,OUTPUT); pinMode(greenPin,OUTPUT); pinMode(bluePin,OUTPUT);
  delay(1500); int ADC1=readADC(32,10);
  delay(1500); int ADC2=readADC(32,10);
  solvePowerLaw(ADC1,L1_LUX,ADC2,L2_LUX);
}

//loop
void loop(){
  while(Serial.available()){
    char c=Serial.read();
    if(c=='B'||c=='b'){ buzzManualActive=true; buzzOffAtMs=millis()+5000UL; }
  }

  int adc=readADC();
  float lux=adcToLux(adc);
  float lux_print=(lux<=DARK_TO_ZERO_LUX)?0.0f:lux;
  Serial.println(lux_print,1);

  float t=constrain(lux/LUX_MAX_DISPLAY,0.0f,1.0f);
  t=t*t*(3.0f-2.0f*t); // smoothstep
  int r=(int)(FIXED_BRIGHTNESS*t);
  int b=(int)(FIXED_BRIGHTNESS*(1.0f-t));
  setRGB(constrain(r,0,255),0,constrain(b,0,255));

  bool autoBuzz=(lux_print<=LUX_BUZZER_THRESHOLD);
  if(buzzManualActive && (long)(millis()-buzzOffAtMs)>=0) buzzManualActive=false;
  digitalWrite(buzzerPin,(autoBuzz||buzzManualActive)?HIGH:LOW);

  delay(150);
}

