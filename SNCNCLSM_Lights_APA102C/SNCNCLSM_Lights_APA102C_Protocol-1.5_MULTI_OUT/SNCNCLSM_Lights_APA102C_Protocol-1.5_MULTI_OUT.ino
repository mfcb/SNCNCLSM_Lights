//Includes
#include <FastLED.h>

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

#define PORT03_DATA_PIN 9 //Pin to transmit Light Data
#define PORT03_CLOCK_PIN 8 //Pin to sync data

#define PORT04_DATA_PIN 7 //Pin to transmit Light Data
#define PORT04_CLOCK_PIN 6 //Pin to sync data

#define PORT05_DATA_PIN 5 //Pin to transmit Light Data
#define PORT05_CLOCK_PIN 4 //Pin to sync data

#define CLOCK_RATE 16 //Clock Rate for APA102 in MHZ


#define BRIGHTNESS 255 //Default Brightness
#define COLOR_TEMPERATURE HighNoonSun //Default Color Temperature
#define COLOR_CORRECTION TypicalSMD5050 //TypicalSMD5050 //Default Color Correction
#define DITHER_MODE 1

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

typedef struct LEDSetting {
  byte saturation;
  byte value;
  boolean fading;
} LEDSetting;

struct CRGB leds[NUM_LEDS]; //This struct stores all the used LEDs

struct LEDSetting layer1[NUM_LEDS];
struct LEDSetting layer2[NUM_LEDS];

byte numSegmentLEDS[] =  {25, 22, 22, 22, 22}; //Number of LEDs per Segment

byte clockTicker = 0; //Used to keep time for Fade calculations.

CHSV globalColor = CHSV(0, 255, 255);

byte randomSabers[5]; //variable used to temporarily store active random saber
byte randomSegments[5]; //variable used to temporarily store active random segment

byte randomSaber = 0;
byte randomSegment = 0;



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
  NOTE_B4,
  //Octave C5
  NOTE_C5,
  NOTE_CIS5,
  NOTE_D5,
  NOTE_DIS5,
  NOTE_E5,
  NOTE_F5,
  NOTE_FIS5,
  NOTE_G5,
  NOTE_GIS5,
  NOTE_A5,
  NOTE_AIS5,
  NOTE_B5,
  //Octave C6
  NOTE_C6,
  NOTE_CIS6,
  NOTE_D6,
  NOTE_DIS6,
  NOTE_E6,
  NOTE_F6,
  NOTE_FIS6,
  NOTE_G6,
  NOTE_GIS6,
  NOTE_A6,
  NOTE_AIS6,
  NOTE_B6,
  //Octave C7
  NOTE_C7,
  NOTE_CIS7,
  NOTE_D7,
  NOTE_DIS7,
  NOTE_E7,
  NOTE_F7,
  NOTE_FIS7,
  NOTE_G7,
  NOTE_GIS7,
  NOTE_A7,
  NOTE_AIS7,
  NOTE_B7
};

int l = 0;
//
//SETUP function. Used at startup to set global variables.
//
void setup() {
  //Set MIDI Baud rate
  Serial.begin(57600); //57600

  //Wait 1 Second to protect LEDs from current onslaught
  delay(1000);

  //FastLED setup: Setup All 5 Ports
  FastLED.addLeds<LED_TYPE, PORT01_DATA_PIN, PORT01_CLOCK_PIN, COLOR_ORDER, DATA_RATE_MHZ(CLOCK_RATE)>(leds, 0, NUM_LEDS_PORT);
  FastLED.addLeds<LED_TYPE, PORT02_DATA_PIN, PORT02_CLOCK_PIN, COLOR_ORDER, DATA_RATE_MHZ(CLOCK_RATE)>(leds, NUM_LEDS_PORT, NUM_LEDS_PORT);
  FastLED.addLeds<LED_TYPE, PORT03_DATA_PIN, PORT03_CLOCK_PIN, COLOR_ORDER, DATA_RATE_MHZ(CLOCK_RATE)>(leds, NUM_LEDS_PORT * 2, NUM_LEDS_PORT);
  FastLED.addLeds<LED_TYPE, PORT04_DATA_PIN, PORT04_CLOCK_PIN, COLOR_ORDER, DATA_RATE_MHZ(CLOCK_RATE)>(leds, NUM_LEDS_PORT * 3, NUM_LEDS_PORT);
  FastLED.addLeds<LED_TYPE, PORT05_DATA_PIN, PORT05_CLOCK_PIN, COLOR_ORDER, DATA_RATE_MHZ(CLOCK_RATE)>(leds, NUM_LEDS_PORT * 4, NUM_LEDS_PORT);

  FillAllSabers(layer1, 0, 0, false);
  FillAllSabers(layer2, 255, 0, false);
  

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
  //Set Power unit to use correct voltage and current
  set_max_power_in_volts_and_milliamps(5, 6000); //36000 = 36 Amperes = 7,2 Amperes per Saber

  //Setup is complete, blink through Lights to show we're ready
  BlinkLedToShowThatWeAreReady();

  globalColor = CHSV(0, 255, 0);

  ApplyLayers();
  FastLED.show();



}

//
//LOOP Function. Runs indefinitely.
//

void loop() {
  MIDI.read();
  /*if(layer1_fade > 0) {
    layer1_fade -= 5;
    FillAllSabers(layer1_fade);
    FastLED.show();
  }*/
  /*if(l == 0) {
    FillAllSabers(layer1, 255, 255, false);
    l=1;
  } else {
    FillAllSabers(layer1, 255, 0, false);
    l=0;
  }*/
  ApplyLayers();
  FastLED.show();
  
  
  

}

void ApplyLayers() {
  //if the led at this index is set to fading, subtract from its value
  for(int i = 0; i<NUM_LEDS;i++) {
    int sat = layer2[i].saturation;
    int val = layer2[i].value;
    //flash layer
    if(layer2[i].fading == true) {
      sat +=5;
      if(sat < 255) {
        layer2[i].saturation = sat;
      } else {
        layer2[i].saturation = 255;
      }
      val -= 5;
      if(val > 0) {
        layer2[i].value = val;
      } else {
        layer2[i].value = 0;
        layer2[i].fading = false;
      }
      
    }
    
    sat = layer1[i].saturation;
    val = layer1[i].value;
    //base layer
    if(layer1[i].fading == true) {
      val -= 5;
      if(val > 0) {
        layer1[i].value = val;
      } else {
        layer1[i].value = 0;
        layer1[i].fading = false;
      }
    }

    leds[i].setHSV(globalColor.hue, min(layer1[i].saturation,layer2[i].saturation), max(layer1[i].value,layer2[i].value));
  }
  
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
    //LAYER 1
    //

    case NOTE_C2: //C2 Set baseLayer of all lights
      FillAllSabers(layer1, 255, velocity, false);
      break;
    case NOTE_CIS2: //D2 Same as C2, but fades in NoteOff
      FillAllSabers(layer1, 255, velocity, false);
      break;
    //STATIC ON, STATIC OFF
    //Note ON
    case NOTE_D2: //Saber 01 Static ON
      FillSaber(layer1, 0, globalColor.saturation, velocity, false);
      break;
    case NOTE_DIS2: //Saber 02 Static ON
      FillSaber(layer1, 1, globalColor.saturation, velocity, false);
      break;
    case NOTE_E2: //Saber 03 Static ON
      FillSaber(layer1, 2, globalColor.saturation, velocity, false);
      break;
    case NOTE_F2: //Saber 04 Static ON
      FillSaber(layer1, 3, globalColor.saturation, velocity, false);
      break;
    case NOTE_FIS2: //Saber 05 Static ON
      FillSaber(layer1, 4, globalColor.saturation, velocity, false);
      break;
    //STATIC ON, FADE OUT
    case NOTE_G2: //Saber 01 Note ON
      FillSaber(layer1, 0, globalColor.saturation, velocity, false);
      break;
    case NOTE_GIS2: //Saber 02 Note ON
      FillSaber(layer1, 1, globalColor.saturation, velocity, false);
      break;
    case NOTE_A2: //Saber 03 Note ON
      FillSaber(layer1, 2, globalColor.saturation, velocity, false);
      break;
    case NOTE_AIS2: //Saber 04 Note ON
      FillSaber(layer1, 3, globalColor.saturation, velocity, false);
      break;
    case NOTE_B2: //Saber 05 Note ON
      FillSaber(layer1, 4, globalColor.saturation, velocity, false);
      break;  

    //
    //LAYER 2
    //

    case NOTE_C3: //C3 sets all lights to white
      FillAllSabers(layer2, 0, velocity, false);
      break;
    case NOTE_CIS3: //CIS3 same as C3 but with fade in NoteOFF
      FillAllSabers(layer2, 0, velocity, false);
      break;
    case NOTE_D3: //C3 Random Saber ON
      randomSaber = random8(NUM_SABERS);
      FillSaber(layer2, randomSaber, 0, velocity, false);
      break;
    case NOTE_DIS3: //C3 sets all lights
      FillAllSabers(layer2, 0, velocity, false);
      break;
    case NOTE_E3: //E3 Random saber strobe
      FillSaber(layer2, randomSabers[0], 0, velocity, false);
      break;
      
    //SINGLE SABERS STATIC
    case NOTE_F3: //C3 sets all lights
      FillSaber(layer2, 0, 0, velocity, false);
      break;
    case NOTE_FIS3: //C3 sets all lights
      FillSaber(layer2, 1, 0, velocity, false);
      break;
    case NOTE_G3: //C3 sets all lights
      FillSaber(layer2, 2, 0, velocity, false);
      break;
    case NOTE_GIS3: //C3 sets all lights
      FillSaber(layer2, 3, 0, velocity, false);
      break;
    case NOTE_A3: //C3 sets all lights
      FillSaber(layer2, 4, 0, velocity, false);
      break;
      
    //SINGLE SABERS FADE NOTEOFF
    case NOTE_AIS3: //C3 sets all lights
      FillSaber(layer2, 0, 0, velocity, false);
      break;
    case NOTE_B3: //C3 sets all lights
      FillSaber(layer2, 1, 0, velocity, false);
      break;
    case NOTE_C4: //C3 sets all lights
      FillSaber(layer2, 2, 0, velocity, false);
      break;
    case NOTE_CIS4: //C3 sets all lights
      FillSaber(layer2, 3, 0, velocity, false);
      break;
    case NOTE_D4: //C3 sets all lights
      FillSaber(layer2, 4, 0, velocity, false);
      break;

    //SEGMENTS

    case NOTE_C5: //Random Segment
      randomSaber = random8(NUM_SABERS);
      randomSegment = random8(NUM_SABER_SEGMENTS);
      FillSaberSegment(layer2, randomSaber, randomSegment, 0, velocity, false);
      break;
    case NOTE_CIS5: //Random Segments
      RandomizeSegments(layer2, 5, 0, velocity);
      break;  
    case NOTE_D5: //Segment Rush UP
      break;
    case NOTE_DIS5: //Segment Rush DOWN
      break;
    case NOTE_F5: //Segment Row 1
      FillSaberSegmentRow(layer2, 0, 0, velocity, false);
      break;
    case NOTE_FIS5: //Segment Row 2
      FillSaberSegmentRow(layer2, 1, 0, velocity, false);
      break;
    case NOTE_G5: //Segment Row 3
      FillSaberSegmentRow(layer2, 2, 0, velocity, false);
      break;
    case NOTE_GIS5: //Segment Row 4
      FillSaberSegmentRow(layer2, 3, 0, velocity, false);
      break;
    case NOTE_A5: //Segment Row 5
      FillSaberSegmentRow(layer2, 4, 0, velocity, false);
      break;        
      
    //IF NOTE DOES NOT MATCH ANY DEFINED NOTES, CANCEL
    default:
      return;
  }

  //Output changes
  //ApplyLayers();
  //FastLED.show();
}

//
//Function to handle MIDI NOTE OFF signals
//

void HandleNoteOff(byte channel, byte pitch, byte velocity)
{
  //Remap velocity to match light requirements
  velocity = remapBrightness(velocity);
  
  switch (pitch) {
    //
    //PANIC
    //
    case NOTE_C0: //C0 is our Panic Switch
      AllLightsOff();
      break;

    //
    //LAYER 1
    //
    //STATIC ON, STATIC OFF
    //Note OFF
    case NOTE_C2: //C2 Set baseLayer of all lights
      FillAllSabers(layer1, globalColor.saturation, 0, false);
      break;
    case NOTE_CIS2: //D2 Same as C2, but fades in NoteOff
      FillAllSabers(layer1, 255, layer1[0].value, true);
      break;
    //STATIC ON, STATIC OFF
    case NOTE_D2: //Saber 01 Note OFF: Static
      FillSaber(layer1, 0, globalColor.saturation, _OFF, false);
      break;
    case NOTE_DIS2: //Saber 02 Note OFF: Static
      FillSaber(layer1, 1, globalColor.saturation, _OFF, false);
      break;
    case NOTE_E2: //Saber 03 Note OFF: Static
      FillSaber(layer1, 2, globalColor.saturation, _OFF, false);
      break;
    case NOTE_F2: //Saber 04 Note OFF: Static
      FillSaber(layer1, 3, globalColor.saturation, _OFF, false);
      break;
    case NOTE_FIS2: //Saber 05 Note OFF: Static
      FillSaber(layer1, 4, globalColor.saturation, _OFF, false);
      break;
    //STATIC ON, FADE OUT
    //Note OFF
    case NOTE_G2: //Saber 01 Note OFF: Fade
      FillSaber(layer1, 0, globalColor.saturation, layer1[0].value, true);
      break;
    case NOTE_GIS2: //Saber 02 Note OFF: Fade
      FillSaber(layer1, 1, globalColor.saturation, layer1[113].value, true);
      break;
    case NOTE_A2: //Saber 03 Note OFF: Fade
      FillSaber(layer1, 2, globalColor.saturation, layer1[226].value, true);
      break;
    case NOTE_AIS2: //Saber 04 Note OFF: Fade
      FillSaber(layer1, 3, globalColor.saturation, layer1[339].value, true);
      break;
    case NOTE_B2: //Saber 05 Note OFF: Fade
      FillSaber(layer1, 4, globalColor.saturation, layer1[452].value, true);
      break;

    //
    //LAYER 2
    //

    case NOTE_C3: //All lights off
      FillAllSabers(layer2, 0, 0, false);
      break;
    case NOTE_CIS3: //All lights fade
      FillAllSabers(layer2, 0, layer2[0].value, true);
      break;
    case NOTE_D3: //Random Saber OFF
      FillSaber(layer2, randomSaber, 0, 0, false);
      break;
    case NOTE_DIS3: //All Lights off
      FillAllSabers(layer2, 0, 0, false);
      break;
    case NOTE_E3: //Random Saber OFF
      FillSaber(layer2, randomSabers[0], 0, 0, false);
      break;
    //SINGLE SABERS STATIC
    case NOTE_F3: //Saber 1 OFF
      FillSaber(layer2, 0, 0, _OFF, false);
      break;
    case NOTE_FIS3: //Saber 2 OFF
      FillSaber(layer2, 1, 0, _OFF, false);
      break;
    case NOTE_G3: //Saber 3 OFF
      FillSaber(layer2, 2, 0, _OFF, false);
      break;
    case NOTE_GIS3: //Saber 4 OFF
      FillSaber(layer2, 3, 0, _OFF, false);
      break;
    case NOTE_A3: //Saber 5 OFF
      FillSaber(layer2, 4, 0, _OFF, false);
      break;
    //SINGLE SABERS FADE IN NOTEOFF
    case NOTE_AIS3: //Saber 1 Fade
      FillSaber(layer2, 0, 0, layer2[0].value, true);
      break;
    case NOTE_B3: //Saber 2 Fade
      FillSaber(layer2, 1, 0, layer2[113].value, true);
      break;
    case NOTE_C4: //Saber 3 Fade
      FillSaber(layer2, 2, 0, layer2[226].value, true);
      break;
    case NOTE_CIS4: //Saber 4 Fade
      FillSaber(layer2, 3, 0, layer2[339].value, true);
      break;
    case NOTE_D4: //Saber 5 Fade
      FillSaber(layer2, 4, 0, layer2[452].value, true);
      break;

    //SEGMENTS

    case NOTE_C5: //Random Segment
      FillSaberSegment(layer2, randomSaber, randomSegment, 0, _OFF, false);
      break;
    case NOTE_CIS5: //Random Segments
      FillRandomSegments(layer2, 0, _OFF);
      break;  
    case NOTE_D5: //Segment Rush UP
      break;
    case NOTE_DIS5: //Segment Rush DOWN
      break;
    case NOTE_F5: //Segment Row 1
      FillSaberSegmentRow(layer2, 0, 0, _OFF, false);
      break;
    case NOTE_FIS5: //Segment Row 2
      FillSaberSegmentRow(layer2, 1, 0, _OFF, false);
      break;
    case NOTE_G5: //Segment Row 3
      FillSaberSegmentRow(layer2, 2, 0, _OFF, false);
      break;
    case NOTE_GIS5: //Segment Row 4
      FillSaberSegmentRow(layer2, 3, 0, _OFF, false);
      break;
    case NOTE_A5: //Segment Row 5
      FillSaberSegmentRow(layer2, 4, 0, _OFF, false);
      break;

    
    //IF NOTE DOES NOT MATCH ANY DEFINED NOTES, CANCEL
    default:
      return;
  }

  //Output Changes
  //ApplyLayers();
  //FastLED.show();

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
    FillSaber(layer1, i, 0, 255, false); //Fill white
    ApplyLayers();
    FastLED.show(); //Display changes
    delay(200);

    FillSaber(layer1, i, 0, _OFF, false); //Fill black
    ApplyLayers();
    delay(200);
    FastLED.show(); //Display changes
  }

}

//Function to set single saber to color and brightness
void FillSaber(struct LEDSetting *targetLayer, byte saber, byte saturation, byte value, bool fading) {
  //iterate through LEDs of selected saber and set to new value
  for (int i = saber * NUM_SABER_LEDS; i < (saber + 1) * NUM_SABER_LEDS; i++) {
    targetLayer[i].saturation = saturation;
    targetLayer[i].value = value;
    targetLayer[i].fading = fading;
  }
  //FastLED.show(); //Display changes
}

//Function to fill a segment of a given saber
void FillSaberSegment(struct LEDSetting *targetLayer, byte saber, byte segment, byte saturation, byte value, byte fading) {

  //iterate through LEDs of selected saber-segment and set to new value
  for (int i = saber * NUM_SABER_LEDS + segment * numSegmentLEDS[segment]; i < saber * NUM_SABER_LEDS + (segment + 1) * numSegmentLEDS[segment]; i++) {
    targetLayer[i].saturation = saturation;
    targetLayer[i].value = value;
    targetLayer[i].fading = fading;
  }
}

//Function to fill a row of segments
void FillSaberSegmentRow(struct LEDSetting *targetLayer, byte row, byte saturation, byte value, byte fading) {
  //iterate through Sabers and fill each segment accordingly to form a row
  for (byte i = 0; i < NUM_SABERS; i++) {
    FillSaberSegment(targetLayer, i, row, saturation, value, fading);
  }
}

//Function to switch off all lights
void AllLightsOff() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

//Function to set all sabers to color and brightness
void FillAllSabers(struct LEDSetting *targetArray, byte saturation, byte value, boolean fading) {
  for(int i=0; i<NUM_LEDS; i++) {
    targetArray[i].saturation = saturation;
    targetArray[i].value = value;
    targetArray[i].fading = fading;
  }
  
}

void RandomizeSabers(struct LEDSetting *targetLayer, byte count, byte saturation, byte value) {
  for(byte i=0;i<count;i++) {
    randomSabers[i] = random8(NUM_SABERS);
    FillSaber(targetLayer, randomSabers[i], saturation, value, false);
    
  }
}

void FillRandomSabers(struct LEDSetting *targetLayer, byte saturation, byte value) {
  for(byte i=0;i<5;i++) {
    FillSaber(targetLayer, randomSabers[i], saturation, value, false);
  }
}

void RandomizeSegments(struct LEDSetting *targetLayer, byte count, byte saturation, byte value) {
  
  for(byte i=0;i<count;i++) {
    randomSabers[i] = random8(NUM_SABERS);
    randomSegments[i] = random8(NUM_SABER_SEGMENTS);
    FillSaberSegment(targetLayer, randomSabers[i], randomSegments[i], saturation, value, false);
  }
}

void FillRandomSegments(struct LEDSetting *targetLayer, byte saturation, byte value) {
  for(byte i=0;i<5;i++) {
    FillSaberSegment(targetLayer, randomSabers[i], randomSegments[i], saturation, value, false);
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


