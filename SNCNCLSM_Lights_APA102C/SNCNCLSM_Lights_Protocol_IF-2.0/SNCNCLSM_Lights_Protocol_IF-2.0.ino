#include <MIDINotes.h>
#include <FastLED.h>
#include <MIDI.h>


#define LED_TYPE APA102

#define NUM_LEDS 565 //Number of LEDs in use
#define NUM_LEDS_PORT 113 //Number of LEDs on Port 01
#define NUM_SABERS 5 //Number of Lightsabers connected
#define NUM_SABER_LEDS 113 //Number of LEDs per Lightsaber
//#define NUM_SABER_SEGMENTS 5 //Number of Segments per Lightsaber

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

#define DEFAULT_BRIGHTNESS 245 //Default Brightness
#define COLOR_TEMPERATURE DirectSunlight //Default Color Temperature
#define COLOR_CORRECTION UncorrectedColor //TypicalSMD5050 //Default Color Correction
#define DEFAULT_FADE_SPEED 192 //Default speed for fades
#define DITHER_MODE 1

#define COLOR_ORDER BGR // APA102 requires BGR Color order!

#define ON 2
#define STOP 1
#define OFF 0

#define INT_MIN -32767
#define INT_MAX 32767

#define DUCKERMODE_FAST 0
#define DUCKERMODE_SLOW 1
#define DUCKERMODE_WAVE 2

#define FADEIN_SPEED 5

long previousMillis = 0;
long interval = 40; //interval between frames (in milliseconds)
int fps = 0;

int seconds = 0;

//variable to count from 0 to 255
unsigned long longCounter = 0;
//variable to count frames
byte counter_frames = 0;

byte counter_sabers = 0;

byte last_saber = 0;

struct CRGB leds[NUM_LEDS]; //THE LED ARRAY
struct CRGB leds_STM[NUM_LEDS]; //THE LED ARRAY

//fading
byte globalFadeSpeed = DEFAULT_FADE_SPEED;


//
//MIDI GLOBAL VARIABLES
//

//Create Serial MIDI instance to accept incoming MIDI signals
MIDI_CREATE_DEFAULT_INSTANCE();



//TRIGGERS

//Panic
bool trigger_allBlack = false;
bool trigger_reset = false;

//Random Pixels
CRGB trigger_randomPixels = 0;
byte trigger_randomPixelCount = 0;
//Flash
byte trigger_flash = 0;
//Flash Color
CRGB trigger_flashColor = 0;
//Ambience
CRGB trigger_ambience = CRGB::Black;
bool trigger_fadeAmbience = false;

//Gradient
byte trigger_gradient = 0;
volatile byte gradientIntensity[NUM_LEDS];
void (*gradientFunction) ();
int gradientOffset = 0;

//Random Segments
byte numSegmentLEDS[] = {29, 28,28,28}; //first segment is a bit larger than the rest
const byte NUM_SABER_SEGMENTS = sizeof(numSegmentLEDS);
//byte segmentLEDValues[] = {1, 5, 10, 25, 50, 100, 150, 200, 255, 255, 255, 255, 255, 255, 200, 150, 100, 50, 25, 10, 5, 1, 0,0,0 };
byte trigger_randomSegment = 0;
byte activeRandomSegment = 0;
//Random Sabers
byte randomSaberSequence[36] = {1,4,3,2,0,2,3,4,1,3,0,2,4,1,3,0,2,4,0,1,2,3,0,4,3,2,0,4,0,4,2,4,1,3,2,0};
byte randomSaberPos = 0;
byte activeRandomSaber = 0;

//Framewise Random Saber
byte trigger_framewiseRandomSaber = 0;

//Pattern
CRGB trigger_pattern = 0;

//Framewise Pattern
byte trigger_framewisePattern = 0;
byte saberPattern = 0;
byte framewiseVelocity = 0;

//Single Sabers
byte trigger_saberStatus[NUM_SABERS];
CRGB trigger_sabersToFill[NUM_SABERS];
//Single Saber fade switches
byte trigger_sabersToFadeIn[NUM_SABERS];

//drawing a line
byte sineOffset_line = 0;
CRGB trigger_sineLine = 0;
CRGB trigger_pingPongLine = 0;
//drawing a point
unsigned int sineOffset_point = 10000;
CRGB trigger_sinePoint = 0;

//DUCKER
byte trigger_ducker = false;
byte trigger_duckerMode = 0;

CRGB trigger_duckValue = 0;

//RISER
 byte riserLevel = 0;
 CRGB trigger_riser = 0;

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

  //allocate memory for STM
  for(int i = 0; i<NUM_LEDS;i++) {
    leds_STM[i] = CRGB(0,255,0);
  }

  scaleGradient(0,255);

  //FastLED color adjustments
  FastLED.setBrightness ( DEFAULT_BRIGHTNESS );
  FastLED.setCorrection ( COLOR_CORRECTION );
  FastLED.setTemperature( COLOR_TEMPERATURE );  
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
    sineOffset_line+=2;    
    sineOffset_point+=200;
    previousMillis = currentMillis;
    
    output();
    counter_frames = longCounter%fps;

    longCounter++;    
 
  }
  
}

//
// MAIN OUTPUT FUNCTION
//

void output() {
  if(trigger_reset) {
    for(int i = 0; i<NUM_LEDS;i++) {
      leds[i] = CRGB(0,0,0);
      leds_STM[i] = CRGB(0,0,0);
    }
    trigger_ambience = 0;
    trigger_gradient = 0;
    trigger_flash = 0;
    trigger_flashColor = 0;
    trigger_randomSegment = 0;
    trigger_sineLine = false;
    trigger_pingPongLine = false;
   for(int i = 0; i<NUM_SABERS;i++) {
      trigger_sabersToFill[i] = 0;
      //fade switches
      trigger_sabersToFadeIn[i] = 0;
   }
    
    //don't repeat
    trigger_reset = false;
    return;
  }
  if(trigger_allBlack) {
    for(int i = 0; i<NUM_LEDS;i++) {
      leds[i] = CRGB(0,0,0);
      leds_STM[i] = CRGB(0,0,0);
    }
    trigger_ambience = 0;
    trigger_gradient = 0;
    trigger_flash = 0;
    trigger_flashColor = 0;
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

  if(trigger_gradient > 0) {
    gradientFunction();
  }

  if(riserLevel>0) {
    
    if(trigger_riser.getLuma() > 0 && riserLevel<113) {
      riserLevel =constrain(riserLevel +8 , 0, 113);
    } else if(trigger_riser.getLuma() == 0 && riserLevel > 0) {
      riserLevel = constrain(riserLevel - 8, 0, 113);
    }
    drawSquare(riserLevel, CRGB(20,20,20));
  }
  
  if(trigger_randomPixelCount > 0) {
    randomPixels(trigger_randomPixelCount, trigger_randomPixels);
  }

  if(trigger_flash > 0) {
    flash(trigger_flash);
  }

  if(trigger_flashColor.getLuma() > 0) {
    flashColor(trigger_flashColor);
    trigger_flashColor = 0;
  }

  if(trigger_randomSegment > 0) {
    fillSaberSegment(activeRandomSaber, activeRandomSegment, trigger_randomSegment);
  }

  if(trigger_framewiseRandomSaber == ON && counter_frames%2 == 0) {
    if(randomSaberPos >= 36) {
        randomSaberPos = 0;
    }
    fillSaber(activeRandomSaber, CRGB(0,0,0));
    activeRandomSaber = randomSaberSequence[randomSaberPos];
    fillSaber(activeRandomSaber, CRGB(framewiseVelocity,framewiseVelocity,framewiseVelocity));
    randomSaberPos++;
  } else if(trigger_framewiseRandomSaber == STOP) {
    fillSaber_STM(activeRandomSaber, CRGB(framewiseVelocity,framewiseVelocity,framewiseVelocity));
    trigger_framewiseRandomSaber = OFF;
  }

  if(trigger_framewisePattern == ON && counter_frames%2 == 0) {
    saberPattern++;
    if(saberPattern > 4) {
      saberPattern = 0;
    }
    fillSaber(saberPattern, CRGB(framewiseVelocity,framewiseVelocity,framewiseVelocity));
  }

  if(trigger_pattern.getLuma() > 0) {
    fillSaber(saberPattern, trigger_pattern);
    
  }

  if(trigger_pingPongLine.getLuma() > 0) {
    drawLinePingPong();
  }
  if(trigger_sineLine.getLuma() > 0) {
    drawLine();
  }

  if(trigger_sinePoint.getLuma() > 0) {
    drawPoints();
  }

  //Iterate through sabers and fill them accordingly
  updateSaberState();
  
  //SHORT TERM MEMORY
  addSTM();
  
//  fadeSTM();

  if(trigger_ambience.getLuma() >= 0 && trigger_fadeAmbience == true) {
    trigger_ambience.nscale8(192);
  }

  if(trigger_duckValue.getLuma() > 0) {
    if(trigger_ducker == true) {
      if(trigger_duckerMode == DUCKERMODE_WAVE) {
        byte v = sin8(longCounter*5)/3 + 5;
        trigger_duckValue = CRGB(v,v,v);
      }
    } else {
      if(trigger_duckerMode == DUCKERMODE_FAST) {
        trigger_duckValue.nscale8(192);
      } else if(trigger_duckerMode == DUCKERMODE_SLOW) {
        trigger_duckValue.nscale8(250);
      } else {
        trigger_duckValue.nscale8(230);
      }
      
    }
    duck(trigger_duckValue);
    
        
  }

  
  
  //Output changes
  FastLED.show();
  
}

void panic() {
  for(int i = 0; i<NUM_LEDS;i++) {
      leds[i] = CRGB(0,0,0);
      leds_STM[i] = CRGB(0,0,0);
    }
    trigger_ambience = 0;
    trigger_gradient = 0;
    trigger_flash = 0;
    trigger_flashColor = 0;
    trigger_randomSegment = 0;
    trigger_sineLine = false;
    trigger_pingPongLine = false;
   for(int i = 0; i<NUM_SABERS;i++) {
      trigger_sabersToFill[i] = 0;
      //fade switches
      trigger_sabersToFadeIn[i] = 0;
   }
    
    FastLED.show();
}

void clearAll() {
  /*for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
    }*/
  FastLED.clear();
}

//go through all the sabers and check if they need a change in color
void updateSaberState() {
  void (*fillFunction) (byte, CRGB);
  for(byte i=0; i < NUM_SABERS; i++) { //Loop to check for activated sabers and fill them accordingly
    if(checkIfWhite(trigger_sabersToFadeIn[i])) {
      fillFunction = fillSaber_STM;
    } else {
      fillFunction = fillSaber;
    }
    if(trigger_saberStatus[i] == ON) { //if trigger_sabersToFill contains a value at this index larger than 0, we need to fill it with white...
      if(trigger_sabersToFadeIn[i] != 0 && trigger_sabersToFadeIn[i] < trigger_sabersToFill[i].getLuma()) { //...unless a fade is active here, in which case...
        byte val = trigger_sabersToFadeIn[i];
        fillFunction(i, CRGB(val,val,val)); //...fill it with the fade value instead...
        trigger_sabersToFadeIn[i] = constrain(trigger_sabersToFadeIn[i]+FADEIN_SPEED, 0, 255); //...and increment it
      } else {
        fillFunction(i, trigger_sabersToFill[i]);
      }
    } else if(trigger_saberStatus[i] == STOP) {
      fillSaber_STM(i, trigger_sabersToFill[i]);
      trigger_saberStatus[i] = OFF;
    }
  }
}

bool checkIfWhite(CRGB color) {
  if(color.r == color.g == color.b) {
    return true;
  } else {
    return false;
  }
}

void updateSTMSaberState() {
  
}

// AMBIENCE

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

void fillGradientOrange() {
  if(gradientOffset>NUM_LEDS) {
    gradientOffset = 0;
  }
//  byte ping = 0;
  for(byte i = 0; i<NUM_SABERS;i++) {
//    ping = 1-ping;
    for(int j=i*NUM_SABER_LEDS;j<(i*NUM_SABER_LEDS)+NUM_SABER_LEDS;j++) {
      unsigned int k = (j+gradientOffset)%NUM_LEDS;
      leds[j] = CRGB(gradientIntensity[k],gradientIntensity[k]/2,gradientIntensity[k]/10);
    }
  }
    
  gradientOffset+=5;
}

void fillGradientBlue() {
  if(gradientOffset>NUM_LEDS) {
    gradientOffset = 0;
  }
//  byte ping = 0;
  for(byte i = 0; i<NUM_SABERS;i++) {
//    ping = 1-ping;
    for(int j=i*NUM_SABER_LEDS;j<(i*NUM_SABER_LEDS)+NUM_SABER_LEDS;j++) {
      unsigned int k = (j+gradientOffset)%NUM_LEDS;
//      if(ping == 1) {
//        k = NUM_LEDS - k;
//      } 
      
      leds[j] = CRGB(0,0,gradientIntensity[k]);
    }
  }
    
  gradientOffset+=2;
}

void fillGradientRed() {
  if(gradientOffset>NUM_LEDS) {
    gradientOffset = 0;
  }
//  byte ping = 0;
  for(byte i = 0; i<NUM_SABERS;i++) {
//    ping = 1-ping;
    for(int j=i*NUM_SABER_LEDS;j<(i*NUM_SABER_LEDS)+NUM_SABER_LEDS;j++) {
      unsigned int k = (j+gradientOffset)%NUM_LEDS;
//      if(ping == 1) {
//        k = NUM_LEDS - k;
//      } 
      
      leds[j] = CRGB(gradientIntensity[k],0,0);
    }
  }
    
  gradientOffset+=2;
}

CRGB mapColor(CRGB color, byte value) {
  value /= 255;
  return CRGB(value * color.r, value * color.g, value * color.b);
  
}

///

// FILL SABER

inline void fillSaber(byte saber, CRGB color) {
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

void fillSaber_STM(byte saber, CRGB color) {
  int offset = saber * NUM_SABER_LEDS;
  int numOffset = (saber * NUM_SABER_LEDS) + NUM_SABER_LEDS;
  for (int i = offset; i < numOffset; i++) {
    leds_STM[i] += color;
  }
}

// FILL SEGMENT


//Function to fill a segment of a given saber
void fillSaberSegment(byte saber, byte segment, byte v) {
  
  byte valPos = 0;
  //iterate through LEDs of selected saber-segment and set to new value
  for (int i = saber * NUM_SABER_LEDS + segment * numSegmentLEDS[segment]; i < saber * NUM_SABER_LEDS + (segment + 1) * numSegmentLEDS[segment]; i++) {
    CRGB color = CRGB(v,v,v);
    leds_STM[i] += color; //color.nscale8(segmentLEDValues[valPos]);
    valPos++;
  }
}

//Function to fill a row of segments
void fillSaberSegmentRow(byte row, byte v) {
  //iterate through Sabers and fill each segment accordingly to form a row
  for (byte i = 0; i < NUM_SABERS; i++) {
    fillSaberSegment(i, row, v);
  }
}

// RANDOM PIXELS

void randomPixels(int count, CRGB color) {
  for(byte i = 0; i<count;i++) {
    int rndLEDS = random16(NUM_LEDS);
    leds[rndLEDS] = color;
  }
}

// FLASH ALL
void flash(byte velocity) {
  for( int i = 0; i < NUM_LEDS; i++) {
        leds_STM[i] += CRGB(velocity,velocity,velocity);
    }
}

void flashColor(CRGB color) {
  for( int i = 0; i < NUM_LEDS; i++) {
        leds_STM[i] = color;
    }
}

// DRAW LINE

void drawLine() {
  byte offset = map(sin8(sineOffset_line),0,255,3,112);
  for(byte i = 0; i<5; i++) {
    leds_STM[i*NUM_SABER_LEDS + offset] += trigger_sineLine; 
  }
}

void drawLinePingPong() {
  byte offset1 = map(sin8(sineOffset_line),0,255,3,112);
  byte offset2 = map(sin8(sineOffset_line),0,255,122,3);
  int ping = 0;
  for(byte i = 0; i<5; i++) {
    ping = (1-ping);
    if(ping) {
      leds_STM[i*NUM_SABER_LEDS + offset1] += trigger_pingPongLine;
    } else {
      leds_STM[i*NUM_SABER_LEDS + offset2] += trigger_pingPongLine;
    }
     
  }
}

//DRAW POINTS

void drawPoints() {
  unsigned int offset1 = map(sin16(sineOffset_point), INT_MIN, INT_MAX, 0, NUM_LEDS);
  unsigned int offset2 = map(sin16(sineOffset_point), INT_MIN, INT_MAX, NUM_LEDS, 0);
  leds_STM[offset1] = trigger_sinePoint;
  leds_STM[offset2] = trigger_sinePoint;
  
}

//DRAW RAIN

void drawRain(byte saber, byte rainSpeed, int rainTime, CRGB color) {
  //fade previous drop
  int yPos = 0;
  if(rainTime>0) {
    yPos = ((rainTime-1) * rainSpeed)%NUM_SABER_LEDS;
    leds_STM[NUM_SABER_LEDS-1 - yPos + (NUM_SABER_LEDS*saber)].nscale8(20);  
  }
  
  //draw new drop
  yPos = (rainTime * rainSpeed)%NUM_SABER_LEDS;
  leds_STM[NUM_SABER_LEDS-1 - yPos + (NUM_SABER_LEDS*saber)] += color;
}

// DUCK

void duck(CRGB value) {
  for(int i=0;i<NUM_LEDS;i++) {
    leds[i] -= value;
  }
}

//draw square
void drawSquare(byte squareSize, CRGB color) {
  for(int i=0;i<squareSize;i++) {
    for(int j=0;j<NUM_SABERS;j++) {
      leds[j*NUM_SABER_LEDS + i] = color;
    }
  }
}

//SCALE GRADIENT
void scaleGradient(byte scaleMin, byte scaleMax) {
  byte j = 0;
  for(int i = 0; i<NUM_LEDS;i++) {
    gradientIntensity[i] = scale8(constrain(sin8(j)+5,0, 255), scaleMax);
    
    j+=4;
  }
}

//SHORT TERM MEMORY

void addSTM() {
  for(int i = 0; i<NUM_LEDS;i++) {
    leds[i] += leds_STM[i];
//    leds_STM[i].nscale8(globalFadeSpeed);
  }
}

void fadeSTM() {
  for(int i = 0; i<NUM_LEDS;i++) {
    leds_STM[i].nscale8(192); //TODO: this can potentially be moved to addSTM()
  }
}

//COLOR UTILITIES
CRGB getRed(byte velocity) {
  return CRGB(velocity,0,0);
}

CRGB getGreen(byte velocity) {
  return CRGB(0,velocity,0);
}

CRGB getBlue(byte velocity) {
  return CRGB(velocity/50, velocity/8, velocity);
}

CRGB getMagenta(byte velocity) {
  return CRGB(velocity, 0, velocity/10);
}

CRGB getOrange(byte velocity) {
  return CRGB(velocity, velocity/3.5, 0);
}

CRGB getBrightOrange(byte velocity) {
  return CRGB(velocity, velocity/2, velocity/10);
}

CRGB getWhite(byte velocity) {
  return CRGB(velocity,velocity,velocity);
}

//MIDI FUNCTIONS

void HandleNoteOn(byte channel, byte pitch, byte velocity) {
  velocity = map(velocity,1,127,2,255);
  //RESET
  if(pitch == NOTE_C_MINUS1) {
    trigger_reset = true;
  }
  //PANIC
  if(pitch == NOTE_C0) {
    trigger_allBlack = true;
  }
  //BRIGHTNESS OVERRIDE
  if(pitch == NOTE_D0) { 
    FastLED.setBrightness(velocity);
  }

  //FADE SPEED OVERRIDE
  if(pitch == NOTE_E0) {
//    globalFadeSpeed = map(velocity,2,255, 5,240);
  }
  //DUCKER
  if(pitch == NOTE_A0) {
    trigger_ducker = true;
    trigger_duckerMode = DUCKERMODE_FAST;
    trigger_duckValue = CRGB(velocity,velocity,velocity);
  }
  if(pitch == NOTE_AIS0) {
    trigger_ducker = true;
    trigger_duckerMode = DUCKERMODE_SLOW;
    trigger_duckValue = CRGB(velocity,velocity,velocity);
  }
  if(pitch == NOTE_B0) {
    trigger_ducker = true;
    trigger_duckerMode = DUCKERMODE_WAVE;
//    duckerCounter = 0;
    trigger_duckValue = CRGB::White;
  }
  //AMBIENCE LAYER
  if(pitch == NOTE_C1) {
    trigger_fadeAmbience = false;
    trigger_ambience = CRGB::Black; //BLACK
    trigger_gradient = false;
  }
  if(pitch == NOTE_CIS1) {
    trigger_fadeAmbience = true; //Fade to BLACK
    trigger_gradient = false;
    
  }
  if(pitch == NOTE_D1) {
    trigger_fadeAmbience = false;
    trigger_gradient = false;
    trigger_ambience = getRed(velocity); //RED
  }
  if(pitch == NOTE_DIS1) {
    trigger_fadeAmbience = false;
    trigger_gradient = false;
    trigger_ambience = getGreen(velocity); //GREEN
  }
  if(pitch == NOTE_E1) {
    trigger_fadeAmbience = false;
    trigger_gradient = false;
    trigger_ambience = getBlue(velocity); //BLUE
  }
  if(pitch == NOTE_F1) {
    trigger_fadeAmbience = false;
    trigger_gradient = false;
    trigger_ambience = getOrange(velocity); //ORANGE
  }
  if(pitch == NOTE_FIS1) {
    trigger_fadeAmbience = false;
    trigger_gradient = false;
    trigger_ambience = getMagenta(velocity); //MAGENTA
  }

  if(pitch == NOTE_G1) { //CIRCUS ORANGE
    trigger_fadeAmbience = false;
    trigger_gradient = velocity;
    gradientFunction = fillGradientOrange;
    scaleGradient(0,velocity);
  }

  if(pitch == NOTE_GIS1) { //FLASH ORANGE
    trigger_flashColor = getOrange(velocity);
  }

  if(pitch == NOTE_A1) { //GRADIENT BLUE
    trigger_fadeAmbience = false;
    trigger_gradient = velocity;
    gradientFunction = fillGradientBlue;
    scaleGradient(0,velocity);
  }

  if(pitch == NOTE_AIS1) { //GRADIENT RED
    trigger_fadeAmbience = false;
    trigger_gradient = velocity;
    gradientFunction = fillGradientRed;
    scaleGradient(0,velocity);
  }

  //STATIC SABER COLORS
  if(pitch == NOTE_C2) {
    trigger_sabersToFill[0] = getRed(velocity); //FILL SABER 0 RED
    trigger_saberStatus[0] = ON;
  }
  if(pitch == NOTE_CIS2) {
    trigger_sabersToFill[0] = getBlue(velocity); //FILL SABER 0 BLUE
    trigger_saberStatus[0] = ON;
  }
  if(pitch == NOTE_D2) {
    trigger_sabersToFill[1] = getRed(velocity); //FILL SABER 1 RED
    trigger_saberStatus[1] = ON;
  }
  if(pitch == NOTE_DIS2) {
    trigger_sabersToFill[1] = getBlue(velocity); //FILL SABER 1 BLUE 
    trigger_saberStatus[1] = ON;
  }
  if(pitch == NOTE_E2) {
    trigger_sabersToFill[2] = getRed(velocity); //FILL SABER 2 RED
    trigger_saberStatus[2] = ON;
  }
  if(pitch == NOTE_F2) {
    trigger_sabersToFill[2] = getBlue(velocity); //FILL SABER 2 BLUE
    trigger_saberStatus[2] = ON;    
  }
  if(pitch == NOTE_FIS2) {
    trigger_sabersToFill[3] = getRed(velocity); //FILL SABER 3 RED
    trigger_saberStatus[3] = ON;    
  }
  if(pitch == NOTE_G2) {
    trigger_sabersToFill[3] = getBlue(velocity); //FILL SABER 3 BLUE
    trigger_saberStatus[3] = ON;    
  }
  if(pitch == NOTE_GIS2) {
    trigger_sabersToFill[4] = getRed(velocity); //FILL SABER 4 RED
    trigger_saberStatus[4] = ON;    
  }
  if(pitch == NOTE_A2) {
    trigger_sabersToFill[4] = getBlue(velocity); //FILL SABER 4 BLUE
    trigger_saberStatus[4] = ON;    
  }

  if(pitch == NOTE_AIS2) {
    saberPattern++;
    if(saberPattern > 4) {
      saberPattern = 0;
    }
    trigger_pattern = getWhite(velocity);
  }

  if(pitch == NOTE_B2) {
    trigger_framewisePattern = ON;
    framewiseVelocity = velocity;
  }
  
  if(pitch == NOTE_C3) {
    trigger_flash = velocity; //FLASH WHITE
  }
  if(pitch == NOTE_CIS3) {
    if(randomSaberPos >= 36) {
        randomSaberPos = 0;
     }
     activeRandomSaber = randomSaberSequence[randomSaberPos];
     trigger_sabersToFill[activeRandomSaber] = getWhite(velocity);
     trigger_saberStatus[activeRandomSaber] = ON;
     randomSaberPos++;
  }
  if(pitch == NOTE_D3) {
    trigger_framewiseRandomSaber = ON; //Framewise Random
    framewiseVelocity = velocity;
  }
  if(pitch == NOTE_DIS3) {
     activeRandomSegment = random8(NUM_SABER_SEGMENTS);
     activeRandomSaber = random8(NUM_SABERS);
     trigger_randomSegment = velocity;
  }
  if(pitch == NOTE_E3) {
    trigger_sabersToFill[0] = getWhite(velocity); //FILL SABER 0 WHITE
    trigger_saberStatus[0] = ON;
  }
  if(pitch == NOTE_F3) {
    trigger_sabersToFill[1] = getWhite(velocity); //FILL SABER 1 WHITE
    trigger_saberStatus[1] = ON;    
  }
  if(pitch == NOTE_FIS3) {
    trigger_sabersToFill[2] = getWhite(velocity); //FILL SABER 2 WHITE
    trigger_saberStatus[2] = ON;    
  }
  if(pitch == NOTE_G3) {
    trigger_sabersToFill[3] = getWhite(velocity); //FILL SABER 3 WHITE
    trigger_saberStatus[3] = ON;    
  }
  if(pitch == NOTE_GIS3) {
    trigger_sabersToFill[4] = getWhite(velocity); //FILL SABER 4 WHITE
    trigger_saberStatus[4] = ON;    
  }

  if(pitch == NOTE_D4) {
    trigger_sabersToFadeIn[0] = 1; //FADE SABER 0 WHITE IN
    trigger_sabersToFill[0] = getWhite(velocity); //FADE SABER 0 WHITE
    trigger_saberStatus[0] = ON;
  }
  if(pitch == NOTE_DIS4) {
    trigger_sabersToFadeIn[1] = 1; //FADE SABER 0 WHITE IN
    trigger_sabersToFill[1] = getWhite(velocity); //FADE SABER 1 WHITE
    trigger_saberStatus[1] = ON;
  }
  if(pitch == NOTE_E4) {
    trigger_sabersToFadeIn[2] = 1; //FADE SABER 0 WHITE IN
    trigger_sabersToFill[2] = getWhite(velocity); //FADE SABER 2 WHITE
    trigger_saberStatus[2] = ON;
  }
  if(pitch == NOTE_F4) {
    trigger_sabersToFadeIn[3] = 1; //FADE SABER 0 WHITE IN
    trigger_sabersToFill[3] = getWhite(velocity); //FADE SABER 3 WHITE
    trigger_saberStatus[3] = ON;
  }
  if(pitch == NOTE_FIS4) {
    trigger_sabersToFadeIn[4] = 1; //FADE SABER 0 WHITE IN
    trigger_sabersToFill[4] = getWhite(velocity); //FADE SABER 4 WHITE
    trigger_saberStatus[4] = ON;
  }

  if(pitch == NOTE_C5) {
    trigger_sabersToFill[0] = getRed(velocity); //FILL SABER 0 RED
    trigger_saberStatus[0] = ON;
  }
  if(pitch == NOTE_CIS5) {
    trigger_sabersToFill[0] = getBlue(velocity); //FILL SABER 0 BLUE
    trigger_saberStatus[0] = ON;
  }
  if(pitch == NOTE_D5) {
    trigger_sabersToFill[1] = getRed(velocity); //FILL SABER 1 RED
    trigger_saberStatus[1] = ON;
  }
  if(pitch == NOTE_DIS5) {
    trigger_sabersToFill[1] = getBlue(velocity); //FILL SABER 1 BLUE 
    trigger_saberStatus[1] = ON;
  }
  if(pitch == NOTE_E5) {
    trigger_sabersToFill[2] = getRed(velocity); //FILL SABER 2 RED
    trigger_saberStatus[2] = ON;
  }
  if(pitch == NOTE_F5) {
    trigger_sabersToFill[2] = getBlue(velocity); //FILL SABER 2 BLUE
    trigger_saberStatus[2] = ON;    
  }
  if(pitch == NOTE_FIS5) {
    trigger_sabersToFill[3] = getRed(velocity); //FILL SABER 3 RED
    trigger_saberStatus[3] = ON;    
  }
  if(pitch == NOTE_G5) {
    trigger_sabersToFill[3] = getBlue(velocity); //FILL SABER 3 BLUE
    trigger_saberStatus[3] = ON;    
  }
  if(pitch == NOTE_GIS5) {
    trigger_sabersToFill[4] = getRed(velocity); //FILL SABER 4 RED
    trigger_saberStatus[4] = ON;    
  }
  if(pitch == NOTE_A5) {
    trigger_sabersToFill[4] = getBlue(velocity); //FILL SABER 4 BLUE
    trigger_saberStatus[4] = ON;    
  }

  if(pitch == NOTE_B5) {
    trigger_randomPixelCount = map(velocity,0,255,1,30);
    trigger_randomPixels = CRGB::Black;
  }
  if(pitch == NOTE_C6) {
    trigger_randomPixelCount = map(velocity,0,255,1,30);
    trigger_randomPixels = CRGB::White;
  }
  if(pitch == NOTE_CIS6) {
    sineOffset_line = 65;
    trigger_sineLine = CRGB::White; //WHITE
  }
  if(pitch == NOTE_D6) {
    sineOffset_line = 65;
    trigger_pingPongLine = CRGB::White; //WHITE
  }
  if(pitch == NOTE_DIS6) {
    sineOffset_line = 65;
    trigger_pingPongLine = getRed(velocity); //RED
  }
  if(pitch == NOTE_E6) {
    sineOffset_line = 65;
    trigger_pingPongLine = getBlue(velocity); //BLUE
  }
  if(pitch == NOTE_F6) {
    sineOffset_point = 0;
    trigger_sinePoint = getWhite(velocity); //WHITE
  }
  if(pitch == NOTE_FIS6) {
    sineOffset_point = 0;
    trigger_sinePoint = getRed(velocity); //RED
  }
  if(pitch == NOTE_G6) {
    sineOffset_point = 0;
    trigger_sinePoint = getBlue(velocity);
  }
  if(pitch == NOTE_GIS6) {
    sineOffset_point = 0;
    trigger_sinePoint = getOrange(velocity);
  }

  if(pitch == NOTE_B6) { //"VOLUME"-STYLE RISER
    riserLevel = 1;
    trigger_riser = getWhite(velocity); // WHITE
  }

  //White flashes single saber
  if(pitch == NOTE_C7) { 
    trigger_sabersToFill[0] = getWhite(velocity); //FILL SABER 0 WHITE
    trigger_saberStatus[0] = ON;
  }

  if(pitch == NOTE_CIS7) {
    trigger_sabersToFill[1] = getWhite(velocity); //FILL SABER 0 WHITE
    trigger_saberStatus[1] = ON;
  }

  if(pitch == NOTE_D7) {
    trigger_sabersToFill[2] = getWhite(velocity); //FILL SABER 0 WHITE
    trigger_saberStatus[2] = ON;
  }

  if(pitch == NOTE_DIS7) {
    trigger_sabersToFill[3] = getWhite(velocity); //FILL SABER 0 WHITE
    trigger_saberStatus[3] = ON;
  }
  if(pitch == NOTE_E7) {
    trigger_sabersToFill[4] = getWhite(velocity); //FILL SABER 0 WHITE
    trigger_saberStatus[4] = ON;
  }
  
}

//NOTE OFF
void HandleNoteOff(byte channel, byte pitch, byte velocity) {
  velocity = map(velocity, 1, 127, 0, 255);
    if(pitch == NOTE_A0) {
      trigger_ducker = false;
    }
    if(pitch == NOTE_AIS0) {
      trigger_ducker = false;
    }
    if(pitch == NOTE_B0) {
      trigger_ducker = false;
    }

    //SINGLE SABERS STATIC FILL
    if(pitch == NOTE_C2) {
      trigger_saberStatus[0] = OFF;
    }
    if(pitch == NOTE_CIS2) {
      trigger_saberStatus[0] = OFF;
    }
    if(pitch == NOTE_D2) {
      trigger_saberStatus[1] = OFF;
    }
    if(pitch == NOTE_DIS2) {
      trigger_saberStatus[1] = OFF;
    }
    if(pitch == NOTE_E2) {
      trigger_saberStatus[2] = OFF;
    }
    if(pitch == NOTE_F2) {
      trigger_saberStatus[2] = OFF;
    }
    if(pitch == NOTE_FIS2){
      trigger_saberStatus[3] = OFF;
    }
    if(pitch == NOTE_G2) {
      trigger_saberStatus[3] = OFF;
    }
    if(pitch == NOTE_GIS2) {
      trigger_saberStatus[4] = OFF;
    }
    if(pitch == NOTE_A2) {
      trigger_saberStatus[4] = OFF;
    }

    if(pitch == NOTE_AIS2) {
      trigger_pattern = 0;
    }
    
    if(pitch == NOTE_B2) {
      trigger_framewisePattern = OFF;
      framewiseVelocity = 0;
    }
    //Flash All
    if(pitch == NOTE_C3) {
      trigger_flash = 0;
    }
    //Flash random Saber
    if(pitch == NOTE_CIS3) {
      trigger_saberStatus[activeRandomSaber] = STOP;
      trigger_sabersToFill[activeRandomSaber] = 0;
    }
    if(pitch == NOTE_D3) {
      trigger_framewiseRandomSaber = STOP;
    }
    //Flash random Segment
    if(pitch == NOTE_DIS3) {
      trigger_randomSegment = 0;
    }
    //SINGLE SABERS STATIC FILL
    if(pitch == NOTE_E3) {
      trigger_saberStatus[0] = STOP;      
    }
    if(pitch == NOTE_F3) {
      trigger_saberStatus[1] = STOP;
    }
    if(pitch == NOTE_FIS3) {
      trigger_saberStatus[2] = STOP;
    }
    if(pitch == NOTE_G3) {
      trigger_saberStatus[3] = STOP;
    }
    if(pitch == NOTE_GIS3) {
      trigger_saberStatus[4] = STOP;
    }
    //SINGLE SABERS FADE IN
    if(pitch == NOTE_D4) {
      trigger_sabersToFadeIn[0] = 0; //FADE SABER 0 WHITE IN
      trigger_saberStatus[0] = STOP;
    }
    if(pitch == NOTE_DIS4) {
      trigger_sabersToFadeIn[1] = 0; //FADE SABER 0 WHITE IN
      trigger_saberStatus[1] = STOP;
    }
    if(pitch == NOTE_E4) {
      trigger_sabersToFadeIn[2] = 0; //FADE SABER 0 WHITE IN
      trigger_saberStatus[2] = STOP;
    }
    if(pitch == NOTE_F4) {
      trigger_sabersToFadeIn[3] = 1; //FADE SABER 0 WHITE IN
      trigger_saberStatus[3] = STOP;
    }
    if(pitch == NOTE_FIS4) {
      trigger_sabersToFadeIn[4] = 0; //FADE SABER 0 WHITE IN
      trigger_saberStatus[4] = STOP;
    }
    //SINGLE SABERS FADE FILL
    if(pitch == NOTE_C5) {
      trigger_saberStatus[0] = STOP;
    }
    if(pitch == NOTE_CIS5) {
      trigger_saberStatus[0] = STOP;
    }
    if(pitch == NOTE_D5) {
      trigger_saberStatus[1] = STOP;
    }
    if(pitch == NOTE_DIS5) {
      trigger_saberStatus[1] = STOP;
    }
    if(pitch == NOTE_E5) {
      trigger_saberStatus[2] = STOP;
    }
    if(pitch == NOTE_F5) {
      trigger_saberStatus[2] = STOP;
    }
    if(pitch == NOTE_FIS5){
      trigger_saberStatus[3] = STOP;
    }
    if(pitch == NOTE_G5) {
      trigger_saberStatus[3] = STOP;
    }
    if(pitch == NOTE_GIS5) {
      trigger_saberStatus[4] = STOP;
    }
    if(pitch == NOTE_A5) {
      trigger_saberStatus[4] = STOP;
    }
    if(pitch == NOTE_B5) {
      trigger_randomPixelCount = 0;
    }  
    if(pitch == NOTE_C6) {
      trigger_randomPixelCount = 0;
    }  
    if(pitch ==  NOTE_CIS6) {
      trigger_sineLine = 0;
    }
    if(pitch == NOTE_D6) {
      trigger_pingPongLine = 0;
    }
    if(pitch == NOTE_DIS6) {
      trigger_pingPongLine = 0;
    }
    if(pitch == NOTE_E6) {
      trigger_pingPongLine = 0;
    }
    if(pitch == NOTE_F6) {
      trigger_sinePoint = 0;
    }
    if(pitch == NOTE_FIS6) {
      trigger_sinePoint = 0;
    }
    if(pitch == NOTE_G6) {
      trigger_sinePoint = 0;
    }

    if(pitch == NOTE_GIS6) {
      trigger_sinePoint = 0;
    }

    if(pitch == NOTE_B6) {
      trigger_riser = 0;
    }

    if(pitch == NOTE_C7) {
      trigger_saberStatus[0] = OFF;
    }

    if(pitch == NOTE_CIS7) {
      trigger_saberStatus[1] = OFF;
    }

    if(pitch == NOTE_D7) {
      trigger_saberStatus[2] = OFF;
    }

    if(pitch == NOTE_DIS7) {
      trigger_saberStatus[3] = OFF;
    }
    if(pitch == NOTE_E7) {
      trigger_saberStatus[4] = OFF;
    }
    
}

