// -------- I2C --------

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// -------- SPI --------
#include <SPI.h>

// -------- Voltage Pin Configuration --------
int vsense = A1;

// -------- I2C Configuration --------
int sda = 18;
int scl = 19;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------- Rotary Encoder Configuration --------
const int RE_A = 3;
const int RE_B = 2;
const int RE_Btn = 7;
volatile int encoderPos = 40;
volatile int lastState = 0;
int lastDisplayedPos = -1;
float desired_voltage = 2.0;
float desired_resistance = 10000;

// -------- SPI Configuration --------
int sck = 13;
// int mosi = 11;
int miso = 12;
int ncs = 10;

void setup() {

  // -------- PIN I/O --------
  pinMode(vsense, INPUT);

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
  lastState = (digitalRead(RE_A) << 1) | digitalRead(RE_B);

  attachInterrupt(digitalPinToInterrupt(RE_A), readEncoderISR, CHANGE);

  // -------- Setup --------
  lcd.clear();
  lcd.print("Ready");

}

void loop() {

  // -------- LCD DISPLAY --------
  desired_voltage = (float)encoderPos / 20;
  if (desired_voltage != lastDisplayedPos) {
    lcd.setCursor(0, 0);
    lcd.print("Pos: ");
    lcd.print(desired_voltage);
    lcd.print("    "); // Clear trailing digits

    desired_resistance = calcResistance(desired_voltage);
    lcd.setCursor(0, 1);
    lcd.print("Res: ");
    lcd.print(desired_resistance);
    lcd.print("    "); // Clear trailing digits
    
    lastDisplayedPos = desired_voltage;
  }

  // -------- DIGITAL POTENTIOMETER --------
  uint8_t mcp_data = desired_resistance * 255 / 10000;
  slowMCPWrite(mcp_data);
  Serial.print("MCP Value: ");
  Serial.print(mcp_data);
  
  // -------- VOLTAGE SENSING --------
  int tmp = analogRead(vsense);
  Serial.print(" | VSENSE: ");
  Serial.println(tmp);

}

// -------------------------- ISR ---------------------------

void readEncoderISR() {
  // Triggered by Pin A changing
  int aState = digitalRead(RE_A);
  int bState = digitalRead(RE_B);

  if (aState != bState) {
    encoderPos--;
  } else {
    encoderPos++;
  }
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