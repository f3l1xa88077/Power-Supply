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

// -------- SPI Configuration --------

int sck = 13;
// int mosi = 11;
int miso = 12;
int ncs = 10;

uint8_t mcp_data = 0;

// -------- Rotary Encoder Configuration --------

int scroll_button = 7;
int scroll_a = 3;
int scroll_b = 2;

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
  // SPI.begin();



  // -------- I2C --------

  lcd.init();
  lcd.backlight();
  lcd.print("Startup...");

}

void loop() {
  
  // -------- VOLTAGE SENSING --------

  int tmp = analogRead(vsense);
  Serial.print("VSENSE: ");
  Serial.println(tmp);

  // -------- DIGITAL POTENTIOMETER --------

  slowMCPWrite(mcp_data);
  mcp_data = (mcp_data + 64) % 256;
  Serial.print(" | MCP Value: ");
  Serial.println(mcp_data);

  delay(2000);

}

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