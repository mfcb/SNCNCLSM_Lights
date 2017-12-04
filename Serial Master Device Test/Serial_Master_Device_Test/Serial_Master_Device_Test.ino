// Wire Master Writer
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Writes data to an I2C/TWI slave device
// Refer to the "Wire Slave Receiver" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

byte lightInfoToSerial[2];

void setup() {
  Wire.begin(); // join i2c bus (address optional for master)
  lightInfoToSerial[0] = 1;
  lightInfoToSerial[1] = 255;
}



void loop() {
  Wire.beginTransmission(8); // transmit to device #8
  Wire.write(lightInfoToSerial, 2);
  Wire.endTransmission();    // stop transmitting

  delay(500);

  lightInfoToSerial[1] = 0;
  Wire.beginTransmission(8); // transmit to device #8
  Wire.write(lightInfoToSerial, 2);
  Wire.endTransmission();    // stop transmitting
  delay(500);
}
