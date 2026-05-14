// -------- I2C --------

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// -------- SPI --------
#include <SPI.h>

// -------- Voltage Pin Configuration --------
int vsense_pin = A1;
volatile float vsense;

// -------- I2C Configuration --------
int sda = 18;
int scl = 19;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------- Rotary Encoder Configuration --------
const int RE_A = 3;
const int RE_B = 2;
const int RE_Btn = 7;

volatile float prev_voltage = -1;
volatile float desired_voltage = 2.0;

// -------- SPI Configuration --------
int sck = 13;
int miso = 12;
int ncs = 10;
uint8_t mcp_val = 128;

void setup() {

  // -------- PIN I/O --------
  pinMode(vsense_pin, INPUT);

  // -------- Serial --------
  Serial.begin(9600);

  // -------- SPI --------
  pinMode(ncs, OUTPUT);
  pinMode(sck, OUTPUT);
  pinMode(12, OUTPUT); // Use Pin 12 (MISO track) as your Data Out
  pinMode(11, INPUT);  // Set Pin 11 (MOSI track) to INPUT so it doesn't short
  digitalWrite(ncs, HIGH); // Disable Slave Select

  // -------- I2C --------
  lcd.init();
  lcd.backlight();
  lcd.print("Startup...");

  // -------- Rotary Encoder --------
  pinMode(RE_A, INPUT_PULLUP);
  pinMode(RE_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(RE_A), readEncoderISR, CHANGE);

}

void loop() {

  if (desired_voltage != prev_voltage) { // Execute code only if a change in input is detected

    // -------- LCD --------
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("INPUT: ");
    lcd.print(desired_voltage);
    lcd.print(" V   "); // Clear trailing digits

    // -------- VOLTAGE SENSING --------
    vsense = vsense_voltage(vsense_pin);

    // -------- FEEDBACK --------

    // Calculate Error
    volatile float prev_error = 99;
    volatile float cur_error = vsense - desired_voltage;

    while (fabs(cur_error) < fabs(prev_error)) {

      // Print Error
      Serial.print("Not Converged... Error is: ");
      Serial.println(cur_error);

      // Update values
      prev_error = cur_error;

      // Calculate New Rheostat Value
      if (prev_error < 0) { // If the error is negative i.e., output voltage is too low
        mcp_val -= 2; // Decrease Rheostat Resistance
      }
      else if (prev_error > 0) { // If the error is positive i.e., output voltage is too high
        mcp_val -= 2; // Increase Rheostat Resistance
      }

      // Send new Rheo value
      slowMCPWrite(mcp_val);

      // Calculate Error
      vsense = vsense_voltage(vsense_pin);
      volatile float cur_error = vsense - desired_voltage;

      Serial.print("VSENSE: ");
      Serial.println(vsense);

    }

    Serial.print("Converged... Voltage: ");
    Serial.println(vsense);

    // -------- LCD --------
    lcd.setCursor(0, 1);
    lcd.print("OUTPUT: ");
    lcd.print(vsense);
    lcd.print(" V   "); // Clear trailing digits

  }

  // if (desired_voltage != lastDisplayedPos) {

  //   // -------- DIGITAL POTENTIOMETER --------
  //   // uint8_t mcp_data = calcResistance(desired_voltage) * 255 / 10000;
  //   slowMCPWrite(mcp_data);
    


  //   // -------- FEEDBACK --------
  //   float best_err = 999;
  //   int best_vsense = vsense;
  //   int best_mcp_data = mcp_data;
  //   bool cont = 1;
  //   float cur_err = 0;

  //   while (1 == 1) {
  //     if (best_err < 0) { // If the actual voltage is too low
  //       mcp_data-=2;
  //     }
  //     else if (best_err > 0) { // If the actual voltage is too high
  //       mcp_data+=2;
  //     }
  //     // Send new value
  //     slowMCPWrite(mcp_data);

  //     delay(50); // Time for circuit to settle

  //     // Read new value
  //     vsense = analogRead(vsense_pin); // Adding for error
  //     cur_err = 3*(float)vsense*5/1023 - desired_voltage;

  //     Serial.print("Not Converged... Error: ");
  //     Serial.println(cur_err);

  //     // Calculate if previously converged
  //     if (fabs(cur_err) < fabs(best_err)) {
  //       best_mcp_data = mcp_data;
  //       best_vsense = vsense;
  //       best_err = cur_err;
  //     }
  //     else {
  //       mcp_data = best_mcp_data;
  //       vsense = best_vsense;
  //       cur_err = best_err;
  //       Serial.print("CONVERGED");
  //       slowMCPWrite(mcp_data);
  //       break;
  //     }
  //   }

  //   // -------- SERIAL MONITOR --------
  //   Serial.print(" | Voltage: ");
  //   Serial.print(desired_voltage);
  //   Serial.print(" | VSENSE: ");
  //   Serial.print(vsense);
  //   Serial.print(" | MCP: ");
  //   Serial.print(mcp_data);
  //   Serial.print(" | Error: ");
  //   Serial.println(cur_err);

  //   // -------- LCD --------
  //   lcd.setCursor(0, 0);
  //   lcd.print("Set: ");
  //   lcd.print(desired_voltage);
  //   lcd.print("    "); // Clear trailing digits
  //   lcd.setCursor(0, 1);
  //   lcd.print("Cur: ");
  //   lcd.print(3*(float)vsense*5/1023);
  //   lcd.print("    "); // Clear trailing digits

  //   lastDisplayedPos = desired_voltage;

  // }

  // Update State
  prev_voltage = desired_voltage;

}

// -------------------------- Analog ---------------------------

float vsense_voltage(int vsense_pin) {
  int vsense_adc = analogRead(vsense_pin);
  return (float)vsense_adc * 5.0 * 3.0 / 1023.0;
}

// -------------------------- ISR ---------------------------

void readEncoderISR() {
  // Triggered by Pin A changing
  int aState = digitalRead(RE_A);
  int bState = digitalRead(RE_B);

  if (aState != bState) {
    desired_voltage -= 0.1;
  } else {
    desired_voltage += 0.1;
  }

  if (desired_voltage < 2.0) { desired_voltage = 2.0; }
  else if (desired_voltage > 6.0) { desired_voltage = 6.0; }
}

// -------------------------- MCP Functions ---------------------------

void slowMCPWrite(byte data) {
  digitalWrite(ncs, LOW);
  delayMicroseconds(10); 

  // Address 0x00 (Wiper 0)
  shiftOutManual(miso, sck, MSBFIRST, 0x00); 
  // Data 0-255
  shiftOutManual(miso, sck, MSBFIRST, data);

  delayMicroseconds(10);
  digitalWrite(ncs, HIGH);
}

void shiftOutManual(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val) {
  for (uint8_t i = 0; i < 8; i++)  {
    if (bitOrder == LSBFIRST)
      digitalWrite(dataPin, !!(val & (1 << i)));
    else      
      digitalWrite(dataPin, !!(val & (1 << (7 - i))));
      
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(10); // Faster than 1Hz but still very safe
    digitalWrite(clockPin, LOW);
    delayMicroseconds(10);
  }
}

int calcResistance(float voltage) {
  // Base Cases
  if (voltage > 6.0) {return 0;}
  if (voltage < 2.0) {return 10000;}

  float num = 12.5*220;
  float den = voltage - 1.25;
  return num / den - 440;
}