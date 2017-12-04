
// Wire Slave Receiver
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Receives data as an I2C/TWI slave device
// Refer to the "Wire Master Writer" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>
#include <Conceptinetics.h>
#include <Rdm_Defines.h>
#include <Rdm_Uid.h>

#define DMX_MASTER_CHANNELS   100 
#define RXEN_PIN                2

DMX_Master dmx_master ( DMX_MASTER_CHANNELS, RXEN_PIN );

byte lightInfoFromSerial[2];

void setup() {
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event

  
  // Enable DMX master interface and start transmitting
  dmx_master.enable ();  
  
}

void loop() {
  //delay(100);
  //dmx_master.setChannelValue ( 1, 255 );
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  byte index = 0;
  while(Wire.available() > 0 && index < 2) {
    lightInfoFromSerial[index] = Wire.read();
    index++;
  }
  dmx_master.setChannelValue ( lightInfoFromSerial[0], lightInfoFromSerial[1] );
  
}
