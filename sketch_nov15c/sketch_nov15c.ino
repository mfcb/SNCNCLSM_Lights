
/*
  DMX_Master.ino - Example code for using the Conceptinetics DMX library
  Copyright (c) 2013 W.A. van der Meeren <danny@illogic.nl>.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

//MIDI INCLUDES
#include <MIDI.h>


//DMX Includes
#include <Conceptinetics.h>
#include <Rdm_Defines.h>
#include <Rdm_Uid.h>



//MIDI defines

#define MIDI_CHANNEL 16
#define CLOCK_DIVIDER 12
#define MIDI_OFFSET 12

//MIDI globals

MIDI_CREATE_DEFAULT_INSTANCE();

enum midiNotes {
        NOTE_C1 = 36,
        NOTE_CIS1 = 37,
        NOTE_D1 = 38,
        NOTE_DIS1 = 39,
        NOTE_E1 = 40,
        NOTE_F1 = 41,
        NOTE_FIS1 = 42,
        NOTE_G1 = 43,
        NOTE_GIS1 = 44,
        NOTE_A1 = 45,
        NOTE_AIS1 = 46,
        NOTE_B1 = 47
};

//
// CTC-DRA-13-1 ISOLATED DMX-RDM SHIELD JUMPER INSTRUCTIONS
//
// If you are using the above mentioned shield you should 
// place the RXEN jumper towards pin number 2, this allows the
// master controller to put to iso shield into transmit 
// (DMX Master) mode 
//
//
// The !EN Jumper should be either placed in the G (GROUND) 
// position to enable the shield circuitry 
//   OR
// if one of the pins is selected the selected pin should be
// set to OUTPUT mode and set to LOGIC LOW in order for the 
// shield to work
//


//
// The master will control 100 Channels (1-100)
// 
// depending on the ammount of memory you have free you can choose
// to enlarge or schrink the ammount of channels (minimum is 1)
//
#define DMX_MASTER_CHANNELS   100 

//
// Pin number to change read or write mode on the shield
//
#define RXEN_PIN                2

//Light Addresses
//Pro Par56 CWWW: Set to 3-Channel Mode (2XXX)
//Reserve 3 Channels per light!
int chan_lamp01 = 1;
int chan_lamp02 = 4;


// Configure a DMX master controller, the master controller
// will use the RXEN_PIN to control its write operation 
// on the bus
DMX_Master        dmx_master ( DMX_MASTER_CHANNELS, RXEN_PIN );

// the setup routine runs once when you press reset:
void setup() {             
  //MIDI setup
  MIDI.begin(MIDI_CHANNEL);
  
  // Enable DMX master interface and start transmitting
  dmx_master.enable (); 
  
  

  // As of the MIDI Library v3.1, the lib uses C style function 
  // pointers to create a callback system for handling input events. 
  MIDI.setHandleNoteOn(HandleNoteOn); 
  MIDI.setHandleControlChange(HandleCC);
  MIDI.setHandleNoteOff(HandleNoteOff);

  MIDI.setHandleClock(HandleClock);
}

// the loop routine runs over and over again forever:
void loop() 
{
  MIDI.read();
}

//MIDI Functions

void HandleNoteOn(byte channel, byte pitch, byte velocity) 
{ 
  switch(pitch) {
    case NOTE_C1:
      allLightsOn();
      break;
    case NOTE_D1:
      setLightTo(chan_lamp01, velocity);
      break;
    case NOTE_E1:
      setLightTo(chan_lamp02, velocity);
      break;
  }
}

void HandleNoteOff(byte channel, byte pitch, byte velocity) 
{
  switch(pitch) {
    case NOTE_C1:
      allLightsOff();
      break;
    case NOTE_D1:
      setLightOff(chan_lamp01);
      break;
    case NOTE_E1:
      setLightOff(chan_lamp02);
      break;
  }
}

void HandleCC(byte channel, byte number, byte value) 
{
  
}

void HandleClock() {
  
}



void allLightsOn() {
  dmx_master.setChannelRange(1, DMX_MASTER_CHANNELS, 255);
}

void allLightsOff() {
  dmx_master.setChannelRange(1, DMX_MASTER_CHANNELS, 0);
}

void setLightTo(int channel, byte value) {
  dmx_master.setChannelValue(channel,value);
}

void setLightOff(int channel) {
  dmx_master.setChannelValue(channel,0);
}

