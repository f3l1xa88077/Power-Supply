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
volatile float desired_voltage = 2.5;

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
    float set_voltage = desired_voltage; // Save current voltage so it isn't overwritten
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("INPUT: ");
    lcd.print(set_voltage);
    lcd.print(" V   "); // Clear trailing digits

    // -------- VOLTAGE SENSING --------
    vsense = vsense_voltage(vsense_pin);

    // -------- FEEDBACK --------

    // Calculate Error
    volatile float cur_error = vsense - set_voltage;

    while (fabs(cur_error) >= 0.05) {

      // Calculate how many steps to take
      int steps = cur_error / 0.1;
      if (steps == 0) {
        steps = (cur_error > 0) ? 1 : -1;
      }

      // Calculate New Rheostat Value
      mcp_val += steps;

      // Send new Rheo value
      mcp_val = constrain(mcp_val, 0, 255);
      slowMCPWrite(mcp_val);

      // Small Delay for Circuit Stabilisation
      delay(5);

      // Calculate Error
      vsense = vsense_voltage(vsense_pin);
      cur_error = vsense - set_voltage;

      // Print Error
      Serial.print("Not Converged... Error is: ");
      Serial.print(cur_error);
      Serial.print(" | ");
      Serial.print("MCP: ");
      Serial.print(mcp_val);
      Serial.print(" | ");
      Serial.print("Vsense: ");
      Serial.print(vsense);
      Serial.print(" | ");
      Serial.print("Vtarget: ");
      Serial.println(set_voltage);

    }

    Serial.print("Converged... Voltage: ");
    Serial.println(vsense);

    // -------- LCD --------
    lcd.setCursor(0, 1);
    lcd.print("OUTPUT: ");
    lcd.print(vsense);
    lcd.print(" V   "); // Clear trailing digits

    // Update State
    prev_voltage = set_voltage;

  }

}

// -------------------------- Analog ---------------------------

float vsense_voltage(int vsense_pin) {
  int vsense_adc = analogRead(vsense_pin);
  return (float)vsense_adc * 5.0 * 3.0 / 1023.0 + 0.05; // 0.05 from experimental testing
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

  if (desired_voltage < 2.4) { desired_voltage = 2.4; }
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