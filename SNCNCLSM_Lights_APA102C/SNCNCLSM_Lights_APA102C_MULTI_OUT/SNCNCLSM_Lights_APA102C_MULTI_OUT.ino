//Includes
#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <cpp_compat.h>
#include <dmx.h>
#include <FastLED.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <fastpin.h>
#include <fastspi.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixelset.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>

#include <MIDI.h>

//////////////////////////////////

//
//FASTLED DEFINITIONS
//

#define LED_TYPE APA102

#define NUM_LEDS 565 //Number of LEDs in use
#define NUM_LEDS_PORT 113 //Number of LEDs on Port 01
#define NUM_SABERS 5 //Number of Lightsabers connected
#define NUM_SABER_LEDS 113 //Number of LEDs per Lightsaber
#define NUM_SABER_SEGMENTS 5 //Number of Segments per Lightsaber

#define MAX_NOISE_LEDS 100 //Maximum number of LEDs that can be randomized for white noise mode

#define _OFF 0

//
//FASTLED DATA AND CLOCK PINS
//
#define PORT01_DATA_PIN 13 //Pin to transmit Light Data
#define PORT01_CLOCK_PIN 12 //Pin to sync data

#define PORT02_DATA_PIN 11 //Pin to transmit Light Data
#define PORT02_CLOCK_PIN 10 //Pin to sync data

#define PORT03_DATA_PIN 48 //Pin to transmit Light Data
#define PORT03_CLOCK_PIN 49 //Pin to sync data

#define PORT04_DATA_PIN 50 //Pin to transmit Light Data
#define PORT04_CLOCK_PIN 51 //Pin to sync data

#define PORT05_DATA_PIN 52 //Pin to transmit Light Data
#define PORT05_CLOCK_PIN 53 //Pin to sync data

#define CLOCK_RATE 16 //Clock Rate for APA102 in MHZ


#define BRIGHTNESS 255 //Default Brightness
#define COLOR_TEMPERATURE HighNoonSun //Default Color Temperature
#define COLOR_CORRECTION TypicalSMD5050 //TypicalSMD5050 //Default Color Correction
#define DITHER_MODE 2

#define COLOR_ORDER BGR // APA102 requires BGR Color order!

//
//MIDI DEFINITIONS
//

#define MIDI_CHANNEL 16 //Midi Channel used to receive Midi signals
#define CLOCK_DIVIDER 12 //Clock divider
#define MIDI_OFFSET 12 //Offset used to account for different Midi Numbering in modern sequencers

//
//FASTLED GLOBAL VARIABLES
//

struct CRGB leds[NUM_LEDS]; //This struct stores all the used LEDs

byte numSegmentLEDS[] =  {20, 20, 20, 20, 20}; //Number of LEDs per Segment

byte clockTicker = 0; //Used to keep time for Fade calculations.

CHSV globalColor = CHSV(0, 255, 255); //Default White. used for color overrides

CHSV saberColors[NUM_LEDS];

byte randomSaber = 0; //variable used to temporarily store active random saber
byte randomSegment = 0; //variable used to temporarily store active random segment

//This variable is used to store which sabers should be affected by PulseSabersFunction.
//Bit 0: All sabers - Bit 1: Saber 1 - Bit 2: Saber 2 - Bit 3: Saber 3 - Bit 4: Saber 4 - Bit 5: Saber 5
byte sabersRegisteredForPulsing = 0;
byte pulseVelocity = 0;

int l = 0;

bool whiteNoiseON = true;
//
//MIDI GLOBAL VARIABLES
//

//Create Serial MIDI instance to accept incoming MIDI signals
MIDI_CREATE_DEFAULT_INSTANCE();

//Enumeration to store all used Midi notes
enum midiNotes {
  //Octave C0
  NOTE_C0 = 24,
  NOTE_CIS0,
  NOTE_D0,
  NOTE_DIS0,
  NOTE_E0,
  NOTE_F0,
  NOTE_FIS0,
  NOTE_G0,
  NOTE_GIS0,
  NOTE_A0,
  NOTE_AIS0,
  NOTE_B0,
  //Octave C1
  NOTE_C1 = 36,
  NOTE_CIS1,
  NOTE_D1,
  NOTE_DIS1,
  NOTE_E1,
  NOTE_F1,
  NOTE_FIS1,
  NOTE_G1,
  NOTE_GIS1,
  NOTE_A1,
  NOTE_AIS1,
  NOTE_B1,
  //Octave C2
  NOTE_C2,
  NOTE_CIS2,
  NOTE_D2,
  NOTE_DIS2,
  NOTE_E2,
  NOTE_F2,
  NOTE_FIS2,
  NOTE_G2,
  NOTE_GIS2,
  NOTE_A2,
  NOTE_AIS2,
  NOTE_B2,
  //Octave C3
  NOTE_C3,
  NOTE_CIS3,
  NOTE_D3,
  NOTE_DIS3,
  NOTE_E3,
  NOTE_F3,
  NOTE_FIS3,
  NOTE_G3,
  NOTE_GIS3,
  NOTE_A3,
  NOTE_AIS3,
  NOTE_B3,
  //Octave C4
  NOTE_C4,
  NOTE_CIS4,
  NOTE_D4,
  NOTE_DIS4,
  NOTE_E4,
  NOTE_F4,
  NOTE_FIS4,
  NOTE_G4,
  NOTE_GIS4,
  NOTE_A4,
  NOTE_AIS4,
  NOTE_B4
};

//DEMO VARIABLES
int variableCounter = 0;

//
//SETUP function. Used at startup to set global variables.
//
void setup() {
  //Setup Fast PWM Mode
  /*pinMode(3, OUTPUT);
    pinMode(11, OUTPUT);
    TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM20);
    TCCR2B = _BV(CS22);
    OCR2A = 180;
    OCR2B = 50;*/
  //Set MIDI Baud rate
  Serial.begin(57600); //57600

  //Wait 1 Second to protect LEDs from current onslaught
  delay(1000);

  //FastLED setup
  //Port 01
  FastLED.addLeds<LED_TYPE, PORT01_DATA_PIN, PORT01_CLOCK_PIN, COLOR_ORDER, DATA_RATE_MHZ(CLOCK_RATE)>(leds, 0, NUM_LEDS_PORT);
  FastLED.addLeds<LED_TYPE, PORT02_DATA_PIN, PORT02_CLOCK_PIN, COLOR_ORDER, DATA_RATE_MHZ(CLOCK_RATE)>(leds, NUM_LEDS_PORT, NUM_LEDS_PORT);
  FastLED.addLeds<LED_TYPE, PORT03_DATA_PIN, PORT03_CLOCK_PIN, COLOR_ORDER, DATA_RATE_MHZ(CLOCK_RATE)>(leds, NUM_LEDS_PORT * 2, NUM_LEDS_PORT);
  FastLED.addLeds<LED_TYPE, PORT04_DATA_PIN, PORT04_CLOCK_PIN, COLOR_ORDER, DATA_RATE_MHZ(CLOCK_RATE)>(leds, NUM_LEDS_PORT * 3, NUM_LEDS_PORT);
  FastLED.addLeds<LED_TYPE, PORT05_DATA_PIN, PORT05_CLOCK_PIN, COLOR_ORDER, DATA_RATE_MHZ(CLOCK_RATE)>(leds, NUM_LEDS_PORT * 4, NUM_LEDS_PORT);
  //Set all Saber Colors to default white
  for (byte i = 0; i < NUM_SABERS; i++) {
    saberColors[i] = CHSV(0, 0, 255);
  }

  //FastLED color adjustments
  FastLED.setBrightness ( BRIGHTNESS );
  FastLED.setTemperature( COLOR_TEMPERATURE );
  FastLED.setCorrection ( COLOR_CORRECTION );
  FastLED.setDither     ( DITHER_MODE );

  //Start receiving MIDI input
  MIDI.begin(MIDI_CHANNEL);

  // As of the MIDI Library v3.1, the lib uses C style function
  // pointers to create a callback system for handling input events.
  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandleNoteOff(HandleNoteOff);
  MIDI.setHandleClock(HandleClock);

  //Set Power unit to use correct voltage and current
  set_max_power_in_volts_and_milliamps(5, 6000); //36000 = 36 Amperes = 7,2 Amperes per Saber

  //Setup is complete, blink through Lights to show we're ready
  BlinkLedToShowThatWeAreReady();

  globalColor = CHSV(0, 0, 255);



}

//
//LOOP Function. Runs indefinitely.
//

void loop() {
  MIDI.read();
 
  
  WhiteNoise();
  
  clockTicker += 1;

}

//
//MIDI Functions
//

//
//Function to handle MIDI NOTE ON signals
//

void HandleNoteOn(byte channel, byte pitch, byte velocity)
{
  //Remap velocity to match light requirements
  velocity = remapBrightness(velocity);

  switch (pitch) {
    //
    //PANIC
    //
    case NOTE_C0: //C0 is our PANIC button
      AllLightsOff();
      break;

    //
    //Colors
    //

    case NOTE_D1:
      globalColor = CHSV(0, 0, velocity); //White
      break;
    case NOTE_E1:
      globalColor = CHSV(0, 255, velocity); //Red
      break;
    case NOTE_F1:
      globalColor = CHSV(96, 255, velocity); //Green
      break;
    case NOTE_G1:
      globalColor = CHSV(160, 255, velocity); //Blue
      break;
    case NOTE_A1:
      globalColor = CHSV(64, 255, velocity); //Yellow
      break;
    case NOTE_B1:
      globalColor = CHSV(192, 255, velocity); //Purple
      break;

    //
    //Solid Fills: SABERS
    //

    case NOTE_C2: //C2 Switches all lights on
      FillAllSabers(velocity);
      break;
    case NOTE_D2: //D2 fills a random Saber
      randomSaber = random8(NUM_SABERS);
      FillSaber(randomSaber, velocity);
      break;
    case NOTE_DIS2: //TODO: DIS2 fills random saber with random color
      ////////
      break;
    //Single Sabers
    case NOTE_F2: //Saber 1
      FillSaber(0, velocity);
      break;
    case NOTE_FIS2: //Saber 2
      FillSaber(1, velocity);
      break;
    case NOTE_G2: //Saber 3
      FillSaber(2, velocity);
      break;
    case NOTE_GIS2: //Saber 4
      FillSaber(3, velocity);
      break;
    case NOTE_A2: //Saber 5
      FillSaber(4, velocity);
      break;

    //
    //Solid Fills: SEGMENTS
    //

    case NOTE_C3: //C3 fills random segment with global color
      randomSaber = random8(NUM_SABERS);
      randomSegment = random8(NUM_SABER_SEGMENTS);
      FillSaberSegment(randomSaber, randomSegment, velocity);
      break;
    case NOTE_CIS3: //TODO: CIS3 fills random segment with random color
      break;
    //Segment rows
    case NOTE_F3: //Segment Row 1
      FillSaberSegmentRow(0, velocity);
      break;
    case NOTE_FIS3: //Segment Row 2
      FillSaberSegmentRow(1, velocity);
      break;
    case NOTE_G3: //Segment Row 3
      FillSaberSegmentRow(2, velocity);
      break;
    case NOTE_GIS3: //Segment Row 4
      FillSaberSegmentRow(3, velocity);
      break;
    case NOTE_A3: //Segment Row 5
      FillSaberSegmentRow(4, velocity);
      break;

    //
    //PULSING
    //

    case NOTE_C4: //C4 has all lights pulse
      bitSet(sabersRegisteredForPulsing, 0);
      break;
    //Single sabers
    case NOTE_F4: //Saber 1
      bitSet(sabersRegisteredForPulsing, 1);
      break;
    case NOTE_FIS4: //Saber 2
      bitSet(sabersRegisteredForPulsing, 2);
      break;
    case NOTE_G4: //Saber 3
      bitSet(sabersRegisteredForPulsing, 3);
      break;
    case NOTE_GIS4: //Saber 4
      bitSet(sabersRegisteredForPulsing, 4);
      break;
    case NOTE_A4: //Saber 5
      bitSet(sabersRegisteredForPulsing, 5);
      break;
  }

  //Output changes
  FastLED.show();
}

//
//Function to handle MIDI NOTE OFF signals
//

void HandleNoteOff(byte channel, byte pitch, byte velocity)
{
  switch (pitch) {
    //
    //PANIC
    //
    case NOTE_C0: //C0 is our Panic Switch
      AllLightsOff();
      break;

    //
    //Solid Fills: SABERS
    //

    case NOTE_C2: //C2 Note Off Switches all lights OFF
      AllLightsOff();
      break;
    case NOTE_D2: //D2 switches random Saber OFF
      FillSaber(randomSaber, _OFF);
      break;
    case NOTE_DIS2: //TODO: DIS2 fills random saber with random color
      ///////
      break;
    //Single Sabers
    case NOTE_F2: //Saber 1
      FillSaber(0, _OFF);
      break;
    case NOTE_FIS2: //Saber 2
      FillSaber(1, _OFF);
      break;
    case NOTE_G2: //Saber 3
      FillSaber(2, _OFF);
      break;
    case NOTE_GIS2: //Saber 4
      FillSaber(3, _OFF);
      break;
    case NOTE_A2: //Saber 5
      FillSaber(4, _OFF);
      break;

    //
    //Solid Fills: SEGMENTS
    //

    case NOTE_C3: //C3 turns random segment OFF
      FillSaberSegment(randomSaber, randomSegment, _OFF);
      break;
    case NOTE_CIS3: //TODO: CIS3 fills random segment with random color
      /////
      break;
    //Segment rows
    case NOTE_F3: //Segment Row 1
      FillSaberSegmentRow(0, _OFF);
      break;
    case NOTE_FIS3: //Segment Row 2
      FillSaberSegmentRow(1, _OFF);
      break;
    case NOTE_G3: //Segment Row 3
      FillSaberSegmentRow(2, _OFF);
      break;
    case NOTE_GIS3: //Segment Row 4
      FillSaberSegmentRow(3, _OFF);
      break;
    case NOTE_A3: //Segment Row 5
      FillSaberSegmentRow(4, _OFF);
      break;

    //
    //Fades
    //

    case NOTE_C4: //C4 unregister all from pulsing
      bitClear(sabersRegisteredForPulsing, 0);
      break;
    //Single sabers
    case NOTE_F4: //Saber 1
      bitClear(sabersRegisteredForPulsing, 1);
      break;
    case NOTE_FIS4: //Saber 2
      bitClear(sabersRegisteredForPulsing, 2);
      break;
    case NOTE_G4: //Saber 3
      bitClear(sabersRegisteredForPulsing, 3);
      break;
    case NOTE_GIS4: //Saber 4
      bitClear(sabersRegisteredForPulsing, 4);
      break;
    case NOTE_A4: //Saber 5
      bitClear(sabersRegisteredForPulsing, 5);
      break;
  }

  //Output Changes
  FastLED.show();

}

//Function to handle MIDI CLOCK ticks (required for fading)
void HandleClock() {

  clockTicker++; //increment clock ticker.
  PulseSabers(); //Call function to Fade Sabers

}


//
//FASTLED functions
//

//Helper function to remap midi values (0-127) to light values (0-255)
byte remapBrightness(byte velocity) {
  return map(velocity, 0, 127, 0, 255);
}


//Startup Function to indicate proper functionality
void BlinkLedToShowThatWeAreReady() {

  for ( byte i = 0; i < NUM_SABERS; i++) {
    FillSaber(i, 255); //Fill white
    FastLED.show(); //Display changes
    delay(200);

    FillSaber(i, 0); //Fill black
    delay(200);
    FastLED.show(); //Display changes
  }

}

//Function to set single saber to color and brightness
void FillSaber(byte saber, byte brightness) {

  //saberColors[saber].value = brightness; //Set brightness of selected saber
  globalColor.value = brightness;
  //iterate through LEDs of selected saber and set to new value
  for (int i = saber * NUM_SABER_LEDS; i < (saber + 1) * NUM_SABER_LEDS; i++) {
    leds[i] = globalColor;
  }
  //FastLED.show(); //Display changes
}

//Function to fill a segment of a given saber
void FillSaberSegment(byte saber, byte segment, byte brightness) {

  globalColor.value = brightness;

  //iterate through LEDs of selected saber-segment and set to new value
  for (int i = saber * NUM_SABER_LEDS + segment * numSegmentLEDS[segment]; i < saber * NUM_SABER_LEDS + (segment + 1) * numSegmentLEDS[segment]; i++) {
    leds[i] = globalColor;
  }
}

//Function to fill a row of segments
void FillSaberSegmentRow(byte row, byte brightness) {
  //iterate through Sabers and fill each segment accordingly to form a row
  for (byte i = 0; i < NUM_SABERS; i++) {
    FillSaberSegment(i, row, brightness);
  }
}

//Function to switch off all lights
void AllLightsOff() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

//Function to set all sabers to color and brightness
void FillAllSabers(byte brightness) {
  globalColor.value = brightness;
  fill_solid(leds, NUM_LEDS, globalColor);
}

//Function to fade all registered sabers
void PulseSabers() {
  //Calculate sine function for gradual fade.
  //BUILTIN VERSION:
  //Speed is determind by MIDI clock signal.
  //Multiply by 127 and offset by 128 to create values from 1 (-127 + 128) to 255 (127+128).
  //pulseVelocity = sin(clockTicker) * 127 + 128;
  //FastLED VERSION:
  //FastLED.sin8 can only accept values <256, if clockTicker is larger it needs to be reset.
  if (clockTicker >= 256) {
    clockTicker = 0;
  }
  pulseVelocity = sin8(clockTicker);

  //If no sabers are registered, don't bother calculating anything. Saves resources ;)
  if (sabersRegisteredForPulsing == 0) {
    return;
  }

  //LOOP THROUGH REGISTERED SABERS AND SET VALUE ACCORDINGLY
  for (byte i = 0; i < NUM_SABERS; i++) {
    //if bit 0 is set to 1, fade all sabers and exit loop
    if (bitRead(sabersRegisteredForPulsing, 0) == 1) {
      FillAllSabers(pulseVelocity);
      break;
    } else { //else loop through remaining bits and set sabers accordingly
      if (bitRead(sabersRegisteredForPulsing, i) == 0) { //if saber bit is not set, go to next
        continue;
      } else { //otherwise set saber accordingly
        FillSaber(i - 1, pulseVelocity);
      }
    }

  }
}

void WhiteNoise() {
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  for (int i = 0; i < MAX_NOISE_LEDS; i++) {
    leds[random16(NUM_LEDS)] = CRGB::White;
  }
  FastLED.show();

}


