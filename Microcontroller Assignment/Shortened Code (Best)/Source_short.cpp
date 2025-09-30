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

#include <Arduino.h>                       //This is required for Arduino Core library

// Pinouts
const int buzzerPin = D5;                  // Digital pin driving the buzzer 
const int redPin    = D6;                  // PWM pin for RED channel of RGB LED
const int greenPin  = D7;                  // PWM pin for GREEN channel of RGB LED
const int bluePin   = D8;                  // PWM pin for BLUE channel of RGB LED
const int sensorPin = A0;                  // Analog input pin connected to photoresistor module AO

// Config
const bool  COMMON_ANODE = false;          // this is because we are using a common-cathode
const float L1_LUX = 120.0f,               // Reference lux #1 used for 2-point calibration (dimmer point)
            L2_LUX = 3200.0f;              // Reference lux #2 used for 2-point calibration (brighter point)
const float DARK_TO_ZERO_LUX = 50.1f;      // lux that is less than 50 is considered zero
const float LUX_MAX_DISPLAY  = 1000.0f;    // Lux value mapped to full RED (color scale ceiling)
const float LUX_BUZZER_THRESHOLD = 50.0f;  // lux that is less than 50 will activate the buzzer
const int   FIXED_BRIGHTNESS = 200;        // Max PWM value used

// Coefficient Models 
float C_coeff = 1.0f,                      // C in L ≈ C * ADC^p (power-law model)
      p_exp   = -1.0f,                     // Exponent p in the same model
      K_room  = 1.0f;                      // room scaling factor (kept at 1 here)

// RGB helper: set the LED color using 0–255 inputs for each channel
void setRGB(uint16_t r, uint16_t g, uint16_t b){
  // Small lambda that writes a PWM value to one pin, accounting for board + LED type
  auto put = [&](int pin, uint16_t v){
#if defined(ESP8266) || defined(ESP32)
    // On ESP boards, PWM resolution is 10-bit (0–1023). Map 0–255, 0–1023.
    uint16_t pwm = map(v, 0, 255, 0, 1023);
    // If LED is common-anode, invert PWM so higher means darker
    if (COMMON_ANODE) pwm = 1023 - pwm;
#else
    uint8_t pwm = v;
    if (COMMON_ANODE) pwm = 255 - pwm;     // Inverted for common-anode
#endif
    analogWrite(pin, pwm);                 // Output the PWM value
  };
  // Apply the per-channel writes
  put(redPin,   r);
  put(greenPin, g);
  put(bluePin,  b);
}

// Read the photoresistor multiple times and average to reduce noise
int readADC(int samples = 16, int delayMs = 2){
  long acc = 0;                             // Accumulator for averaging
  for (int i = 0; i < samples; i++){        // Take samples readings
    acc += analogRead(sensorPin);           // Add each ADC reading
    delay(delayMs);                         // Small delay between reads to stabilize
  }
  // Average and clamp to a safe range
  return constrain(acc / samples, 1, 1022);
}

// Compute the power-law calibration (L ≈ C * ADC^p) from two reference points
void solvePowerLaw(int ADC1, float L1, int ADC2, float L2){
  // Work in logs so we can solve for p = (ln L2 - ln L1) / (ln ADC2 - ln ADC1)
  float lnA = log((float)ADC2) - log((float)ADC1);
  float lnL = log(max(L2, 0.1f)) - log(max(L1, 0.1f)); // Guard tiny/zero lux in log()

  // If the two points are too close (small ln delta), fall back to a default p and compute C
  if (fabs(lnA) < 0.15f || fabs(lnL) < 0.15f){
    p_exp  = -1.0f;                          // Reasonable default slope for LDRs
    C_coeff = L1 / pow((float)ADC1, p_exp);  // Compute C so (ADC1, L1) still lies on the curve
  } else {
    // Otherwise compute p and clamp it to a sensible range to avoid wild curves
    p_exp   = constrain(lnL / lnA, -2.0f, 2.0f);
    C_coeff = L1 / pow((float)ADC1, p_exp);  // Back-solve C using (ADC1, L1)
  }
}

// Convert an ADC reading to “raw” lux using the current power-law model
float lux_raw_from_adc(int adc){
  // Ensure output never goes fully to zero (helps divisions/logs elsewhere if added)
  return max(C_coeff * pow((float)adc, p_exp), 0.0001f);
}

// Convert ADC → lux, then apply room scale and clamp to a practical range
float adcToLux(int adc){
  return constrain(K_room * lux_raw_from_adc(adc), 0.0f, 3000.0f);
}

// Buzzer state (for 5-second manual buzz command over Serial)
bool           buzzManualActive = false;   // True while manual buzz is active
unsigned long  buzzOffAtMs      = 0;       // Time (millis) after which manual buzz stops

// setup: runs once at boot/reset
void setup(){
  Serial.begin(115200);                     // Start serial port for printing lux and commands
  pinMode(buzzerPin, OUTPUT);               // Prepare buzzer pin
  pinMode(redPin, OUTPUT);                  // Prepare RGB pins
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Take two spaced readings to represent dim and then bright calibration points
  delay(1500);                              // Give user/time to set first (dim) condition
  int ADC1 = readADC(32, 10);               // Average more samples for a stable calibration point
  delay(1500);                              // Give user/time to set second (bright) condition
  int ADC2 = readADC(32, 10);               // Second calibration ADC

  // Fit power law using the two ADC readings and the provided lux references
  solvePowerLaw(ADC1, L1_LUX, ADC2, L2_LUX);
}

// loop: runs repeatedly after setup()
void loop(){
  // Handle serial commands (type 'b' or 'B' to force buzzer for 5 seconds)
  while (Serial.available()){
    char c = Serial.read();                 // Read one incoming byte
    if (c == 'B' || c == 'b'){              // If user requests manual buzz
      buzzManualActive = true;              // Start manual buzzing
      buzzOffAtMs = millis() + 5000UL;      // Set end time 5 seconds in the future
    }
  }

  // Read the sensor and convert to lux
  int   adc  = readADC();                   // Get a smoothed ADC value
  float lux  = adcToLux(adc);               // Convert ADC to lux using calibration
  // For display/logic: treat very dark values as 0.0 lux to avoid flicker/noise
  float lux_print = (lux <= DARK_TO_ZERO_LUX) ? 0.0f : lux;

  Serial.println(lux_print, 1);             // Print lux with 0.1 precision to Serial

  // Map lux to a blue→red gradient using a smoothstep curve for nicer transitions
  float t = constrain(lux / LUX_MAX_DISPLAY, 0.0f, 1.0f); // Normalize 0..LUX_MAX_DISPLAY → 0..1
  t = t * t * (3.0f - 2.0f * t);            // Smoothstep easing (less jumpy PWM changes)

  // Compute RED/BLUE intensities at a fixed overall brightness
  int r = (int)(FIXED_BRIGHTNESS * t);      // More light → more red
  int b = (int)(FIXED_BRIGHTNESS * (1.0f - t)); // Less light → more blue

  // Update the LED (no green channel here; could be added for full spectrum)
  setRGB(constrain(r, 0, 255), 0, constrain(b, 0, 255));

  // Auto-buzz when dark OR keep buzzing while a manual buzz is active
  bool autoBuzz = (lux_print <= LUX_BUZZER_THRESHOLD);  // Darkness triggers buzzer
  // If manual buzz time has expired, turn off the manual flag
  if (buzzManualActive && (long)(millis() - buzzOffAtMs) >= 0) buzzManualActive = false;

  // Drive the buzzer: on if either autoBuzz or manual buzz is true
  digitalWrite(buzzerPin, (autoBuzz || buzzManualActive) ? HIGH : LOW);

  delay(150);                               // Small loop delay to limit update rate (~6–7 Hz)
}

