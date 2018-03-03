#include <MIDINotes.h>
#include <FastLED.h>
#include <MIDI.h>


#define LED_TYPE APA102

#define NUM_LEDS 565 //Number of LEDs in use
#define NUM_LEDS_PORT 113 //Number of LEDs on Port 01
#define NUM_SABERS 5 //Number of Lightsabers connected
#define NUM_SABER_LEDS 113 //Number of LEDs per Lightsaber
#define NUM_SABER_SEGMENTS 5 //Number of Segments per Lightsaber

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

#define BRIGHTNESS 255 //Default Brightness
#define COLOR_TEMPERATURE HighNoonSun //Default Color Temperature
#define COLOR_CORRECTION TypicalSMD5050 //TypicalSMD5050 //Default Color Correction
#define DITHER_MODE 2

#define COLOR_ORDER BGR // APA102 requires BGR Color order!

long previousMillis = 0;
long interval = 40; //interval between frames (in milliseconds)
int fps = 0;

int seconds = 0;
byte counter_frames = 0;
byte counter_sabers = 0;

byte last_saber = 0;

struct CRGB leds[NUM_LEDS]; //THE LED ARRAY
struct CRGB leds_STM[NUM_LEDS]; //THE LED ARRAY


//
//MIDI GLOBAL VARIABLES
//

//Create Serial MIDI instance to accept incoming MIDI signals
MIDI_CREATE_DEFAULT_INSTANCE();

//TRIGGERS

byte trigger_randomPixels = 0;
byte trigger_flash = 0;
CRGB trigger_ambience = CRGB::Black;
bool trigger_fadeAmbience = false;
byte trigger_red = 0;
//array for random Segments
byte trigger_randomSegment = 0;
int activeRandomSegment = 0;
//array for random Sabers
byte randomSaberSequence[36] = {1,4,3,2,0,2,3,4,1,3,0,2,4,1,3,0,2,4,0,1,2,3,0,4,3,2,0,4,0,4,2,4,1,3,2,0};
byte randomSaberPos = 0;
byte activeRandomSaber = 0;
//array to store saber
CRGB trigger_sabersToFill[5];
//fade switches
byte trigger_sabersToFadeIn[5];

byte sineOffset = 0;
bool trigger_sineLine = false;
bool trigger_pingPongLine = false;

bool trigger_allBlack = false;

/*
 * 
 * ARDUINO
 * 
 */

void setup() {
  //Start receiving MIDI input
  MIDI.begin(16);
  // As of the MIDI Library v3.1, the lib uses C style function
  // pointers to create a callback system for handling input events.
  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandleNoteOff(HandleNoteOff);
//  MIDI.setHandleClock(HandleClock);
  MIDI.turnThruOff();
  
  //FastLED setup: Setup All 5 Ports
  FastLED.addLeds<LED_TYPE, PORT01_DATA_PIN, PORT01_CLOCK_PIN, COLOR_ORDER>(leds, 0, NUM_LEDS_PORT);
  FastLED.addLeds<LED_TYPE, PORT02_DATA_PIN, PORT02_CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS_PORT, NUM_LEDS_PORT);
  FastLED.addLeds<LED_TYPE, PORT03_DATA_PIN, PORT03_CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS_PORT * 2, NUM_LEDS_PORT);
  FastLED.addLeds<LED_TYPE, PORT04_DATA_PIN, PORT04_CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS_PORT * 3, NUM_LEDS_PORT);
  FastLED.addLeds<LED_TYPE, PORT05_DATA_PIN, PORT05_CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS_PORT * 4, NUM_LEDS_PORT);

  for(int i = 0; i<NUM_LEDS;i++) {
    leds_STM[i] = CRGB(0,255,0);
  }

  //FastLED color adjustments
  FastLED.setBrightness ( BRIGHTNESS );
  FastLED.setTemperature( COLOR_TEMPERATURE );
  FastLED.setCorrection ( COLOR_CORRECTION );
  FastLED.setDither     ( DITHER_MODE );

  set_max_power_in_volts_and_milliamps(5, 40000);

  Serial.println("Ready");

  fps = 1000 / interval;
}

/*
 * 
 * LOOP FUNCTION - RUNS INDEFINETLY
 * 
 * 
 */

void loop() {
  MIDI.read();
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) {
    // save the last time you blinked the LED 
    counter_frames++;
    sineOffset+=2;    
    
    previousMillis = currentMillis;
    output();
    if(counter_frames>=fps) {
      counter_frames = 0;
    }
 
  }
  
}

//
// MAIN OUTPUT FUNCTION
//

void output() {
  if(trigger_allBlack) {
    for(int i = 0; i<NUM_LEDS;i++) {
      leds[i] = CRGB(0,0,0);
      leds_STM[i] = CRGB(0,0,0);
    }
    trigger_ambience = 0;
    trigger_flash = 0;
    trigger_randomSegment = 0;
    trigger_sineLine = false;
    trigger_pingPongLine = false;
   for(int i = 0; i<NUM_SABERS;i++) {
      trigger_sabersToFill[i] = 0;
      //fade switches
      trigger_sabersToFadeIn[i] = 0;
   }
    
    FastLED.show();
    //don't repeat
    trigger_allBlack = false;
    return;
  }
  //Start by clearing image buffer
  clearAll();

  //AMBIENT LAYER
  fillAmbience(trigger_ambience);
  
  if(trigger_randomPixels > 0) {
    randomPixels(trigger_randomPixels);
  }

  if(trigger_flash > 0) {
    flash(trigger_flash);
  }

  if(trigger_randomSegment > 0) {
    fillSegment(activeRandomSegment, trigger_randomSegment);
  }

  if(trigger_pingPongLine == true) {
    drawLinePingPong();
  }
  if(trigger_sineLine == true) {
    drawLine();
  }
  updateSaberState();
  //SHORT TERM MEMORY
  addSTM();
  
  fadeSTM();
  if(trigger_ambience.getLuma() >= 0 && trigger_fadeAmbience == true) {
    trigger_ambience.nscale8(192);
  }

  
  
  //Output changes
  FastLED.show();
  
}

void clearAll() {
  for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
    }
}

//go through all the sabers and check if they need a change in color
void updateSaberState() {
  for(byte i=0; i < NUM_SABERS; i++) { //Loop to check for activated sabers and fill them accordingly
    if(trigger_sabersToFill[i].getLuma() > 0) { //if trigger_sabersToFill contains a value at this index larger than 0, we need to fill it with white...
      if(trigger_sabersToFadeIn[i] != 0 && trigger_sabersToFadeIn[i] < trigger_sabersToFill[i].getLuma()) { //...unless a fade is active here, in which case...
        byte val = trigger_sabersToFadeIn[i];
        fillSaber(i, CRGB(val,val,val)); //...fill it with the fade value instead...
        trigger_sabersToFadeIn[i] = constrain(trigger_sabersToFadeIn[i]+10, 0, 255); //...and increment it
      } else {
        fillSaber(i, trigger_sabersToFill[i]);
      }
    }
  }
}

// FILL ALL LEDS

template <class T> void fillAmbience(T color) {
  for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
  }
}

template <class T> void addAmbience(T color) {
  for(int i = 0; i < NUM_LEDS; i++) {
      leds_STM[i] += color;
  }
}

///

// FILL SABER

void fillSaber(byte saber, CRGB color) {
  int offset = saber * NUM_SABER_LEDS;
  int numOffset = (saber * NUM_SABER_LEDS) + NUM_SABER_LEDS;
  for (int i = offset; i < numOffset; i++) {
    leds[i] = color;
  }
}

void fillSaber(byte saber, byte velocity) {
  CRGB color = CRGB(velocity,velocity,velocity);
  fillSaber(saber, color);
}

// FILL SEGMENT

void fillSegment(int offset, byte v) {
  CRGB color = CRGB(v,v,v);
  for(int i=offset;i<offset+20;i++) {
    leds_STM[i] += color;
  }
}

// RANDOM PIXELS

void randomPixels(int count) {
  for(byte i = 0; i<count;i++) {
    int rndLEDS = random(NUM_LEDS);
    leds[rndLEDS] = CRGB(255,255,255);
  }
}

// FLASH ALL
void flash(byte velocity) {
  for( int i = 0; i < NUM_LEDS; i++) {
        leds_STM[i] += CRGB(velocity,velocity,velocity);
    }
}

// DRAW LINE

void drawLine() {
  byte offset = map(sin8(sineOffset),0,255,3,112);
  for(byte i = 0; i<5; i++) {
    leds_STM[i*NUM_SABER_LEDS + offset] = CRGB(255,255,255); 
  }
}

void drawLinePingPong() {
  byte offset1 = map(sin8(sineOffset),0,255,3,112);
  byte offset2 = map(sin8(sineOffset),0,255,122,3);
  int ping = 0;
  for(byte i = 0; i<5; i++) {
    ping = (1-ping);
    if(ping) {
      leds_STM[i*NUM_SABER_LEDS + offset1] = CRGB(255,255,255);
    } else {
      leds_STM[i*NUM_SABER_LEDS + offset2] = CRGB(255,255,255);
    }
     
  }
}

//SHORT TERM MEMORY

void addSTM() {
  for(int i = 0; i<NUM_LEDS;i++) {
    leds[i] += leds_STM[i];
  }
}

void fadeSTM() {
  for(int i = 0; i<NUM_LEDS;i++) {
    leds_STM[i].nscale8(192); //TODO: this can potentially be moved to addSTM()
  }
}


//MIDI FUNCTIONS

void HandleNoteOn(byte channel, byte pitch, byte velocity) {
  velocity = map(velocity,1,127,0,255);
  switch(pitch) {
    //ALL BLACK
    case NOTE_C0:
      trigger_allBlack = true;
//      break;
    //Ambient colors:
    case NOTE_C1: 
      trigger_fadeAmbience = false;
      trigger_ambience = CRGB::Black; //BLACK
//      break;
    case NOTE_CIS1: 
      trigger_fadeAmbience = true; //Fade to BLACK
//      break;
    case NOTE_D1:
      trigger_fadeAmbience = false;
      trigger_ambience = CRGB(velocity, 0, 0); //RED
//      break;
    case NOTE_DIS1:
      trigger_fadeAmbience = false;
      trigger_ambience = CRGB(0, velocity, 0); //GREEN
//      break;
    case NOTE_E1:
      trigger_fadeAmbience = false;
      trigger_ambience = CRGB(0, 0, velocity); //BLUE
//      break;
    case NOTE_F1:
      trigger_fadeAmbience = false;
      trigger_ambience = CRGB(255, 255, 0); //YELLOW
//      break;
    case NOTE_FIS1:
      trigger_fadeAmbience = false;
      trigger_ambience = CRGB(velocity, 0, velocity); //PURPLE
//      break;
    //Flash All
    case NOTE_C3:
      trigger_flash = velocity;
//      break;
    //Flash random
    case NOTE_CIS3:
      if(randomSaberPos >= 36) {
        randomSaberPos = 0;
      }
      activeRandomSaber = randomSaberSequence[randomSaberPos];
      trigger_sabersToFill[activeRandomSaber] = CRGB(velocity,velocity,velocity);
      randomSaberPos++;
//      break;
    //SINGLE SABERS STATIC FILL
    case NOTE_D3:
      activeRandomSegment = random16(NUM_LEDS-20);
      trigger_randomSegment = velocity;
//      break;
    case NOTE_DIS3:
      trigger_sabersToFill[0] = CRGB(velocity,velocity,velocity); //FILL SABER 0 WHITE
//      break;
    case NOTE_E3:
      trigger_sabersToFill[1] = CRGB(velocity,velocity,velocity); //FILL SABER 1 WHITE
//      break;
    case NOTE_F3:
      trigger_sabersToFill[2] = CRGB(velocity,velocity,velocity); //FILL SABER 2 WHITE
//      break;
    case NOTE_FIS3:
      trigger_sabersToFill[3] = CRGB(velocity,velocity,velocity); //FILL SABER 3 WHITE
//      break;
    case NOTE_G3:
      trigger_sabersToFill[4] = CRGB(velocity,velocity,velocity); //FILL SABER 4 WHITE
//      break;
    //SINGLE SABERS FADE IN
    case NOTE_D4:
      trigger_sabersToFadeIn[0] = 1; //FADE SABER 0 WHITE IN
      trigger_sabersToFill[0] = CRGB(velocity,velocity,velocity); //FADE SABER 0 WHITE
//      break;
    case NOTE_DIS4:
      trigger_sabersToFadeIn[1] = 1; //FADE SABER 0 WHITE IN
      trigger_sabersToFill[1] = CRGB(velocity,velocity,velocity); //FADE SABER 1 WHITE
//      break;
    case NOTE_E4:
      trigger_sabersToFadeIn[2] = 1; //FADE SABER 0 WHITE IN
      trigger_sabersToFill[2] = CRGB(velocity,velocity,velocity); //FADE SABER 2 WHITE
//      break;
    case NOTE_F4:
      trigger_sabersToFadeIn[3] = 1; //FADE SABER 0 WHITE IN
      trigger_sabersToFill[3] = CRGB(velocity,velocity,velocity); //FADE SABER 3 WHITE
//      break;
    case NOTE_FIS4:
      trigger_sabersToFadeIn[4] = 1; //FADE SABER 0 WHITE IN
      trigger_sabersToFill[4] = CRGB(velocity,velocity,velocity); //FADE SABER 4 WHITE
//      break;
    //SINGLE SABERS STATIC FILL COLOR
    case NOTE_C5:
      trigger_sabersToFill[0] = CRGB(velocity,0,0); //FILL SABER 0 RED
//      break;
    case NOTE_CIS5:
      trigger_sabersToFill[0] = CRGB(0,0,velocity); //FILL SABER 0 BLUE
//      break;
    case NOTE_D5:
      trigger_sabersToFill[1] = CRGB(velocity,0,0); //FILL SABER 1 RED
//      break;
    case NOTE_DIS5:
      trigger_sabersToFill[1] = CRGB(0,0,velocity); //FILL SABER 1 BLUE
//      break;
    case NOTE_E5:
      trigger_sabersToFill[2] = CRGB(velocity,0,0); //FILL SABER 2 RED
//      break;  
    case NOTE_F5:
      trigger_sabersToFill[2] = CRGB(0,0,velocity); //FILL SABER 2 BLUE
//      break;
    case NOTE_FIS5:
      trigger_sabersToFill[3] = CRGB(velocity,0,0); //FILL SABER 3 RED
//      break;
    case NOTE_G5:
      trigger_sabersToFill[3] = CRGB(0,0,velocity); //FILL SABER 3 BLUE
//      break;
    case NOTE_GIS5:
      trigger_sabersToFill[4] = CRGB(velocity,0,0); //FILL SABER 4 RED
//      break;
    case NOTE_A5:
      trigger_sabersToFill[4] = CRGB(0,0,velocity); //FILL SABER 4 BLUE
//      break;
    case NOTE_C6:
      trigger_randomPixels = map(velocity,0,255,0,20);
//      break;
    case NOTE_CIS6:
      sineOffset = 65;
      trigger_sineLine = true;
//      break;
    case NOTE_D6:
      sineOffset = 65;
      trigger_pingPongLine = true;
//      break;
    default:
      break;
  }
}

void HandleNoteOff(byte channel, byte pitch, byte velocity) {
  velocity = map(velocity, 1, 127, 0, 255);
  switch(pitch) {
    //Flash All
    case NOTE_C3:
      trigger_flash = 0;
//      break;
    //Flash random Saber
    case NOTE_CIS3:
      trigger_sabersToFill[activeRandomSaber] = 0;
//      break;
    //Flash random Segment
    case NOTE_D3:
      trigger_randomSegment = 0;
//      break;
    //SINGLE SABERS STATIC FILL
    case NOTE_DIS3:
      trigger_sabersToFill[0] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_E3:
      trigger_sabersToFill[1] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_F3:
      trigger_sabersToFill[2] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_FIS3:
      trigger_sabersToFill[3] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_G3:
      trigger_sabersToFill[4] = 0; //FILL SABER 0 WHITE
//      break;
    //SINGLE SABERS FADE IN
    case NOTE_D4:
      trigger_sabersToFadeIn[0] = 0; //FADE SABER 0 WHITE IN
      trigger_sabersToFill[0] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_DIS4:
      trigger_sabersToFadeIn[1] = 0; //FADE SABER 0 WHITE IN
      trigger_sabersToFill[1] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_E4:
      trigger_sabersToFadeIn[2] = 0; //FADE SABER 0 WHITE IN
      trigger_sabersToFill[2] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_F4:
      trigger_sabersToFadeIn[3] = 1; //FADE SABER 0 WHITE IN
      trigger_sabersToFill[3] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_FIS4:
      trigger_sabersToFadeIn[4] = 0; //FADE SABER 0 WHITE IN
      trigger_sabersToFill[4] = 0; //FILL SABER 0 WHITE
//      break;
    //SINGLE SABERS STATIC FILL
    case NOTE_C5:
      trigger_sabersToFill[0] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_CIS5:
      trigger_sabersToFill[0] = 0; //FILL SABER 0 WHITE
//      break;  
    case NOTE_D5:
      trigger_sabersToFill[1] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_DIS5:
      trigger_sabersToFill[1] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_E5:
      trigger_sabersToFill[2] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_F5:
      trigger_sabersToFill[2] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_FIS5:
      trigger_sabersToFill[3] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_G5:
      trigger_sabersToFill[3] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_GIS5:
      trigger_sabersToFill[4] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_A5:
      trigger_sabersToFill[4] = 0; //FILL SABER 0 WHITE
//      break;
    case NOTE_C6:
      trigger_randomPixels = 0;
//      break;
    case NOTE_CIS6:
      trigger_sineLine = false;
    case NOTE_D6:
      trigger_pingPongLine = false;
    default:
      break;
  }
}

