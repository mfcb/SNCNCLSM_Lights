#include <FastLED.h>

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
long interval = 20;

int seconds = 0;
byte counter_frames = 0;
byte counter_sabers = 0;

byte last_saber = 0;

struct CRGB leds[NUM_LEDS]; //THE LED ARRAY
struct CRGB leds_STM[NUM_LEDS]; //THE LED ARRAY
struct CRGB leds2[NUM_LEDS]; //THE LED ARRAY

void setup() {
  Serial.begin(38400);
  //FastLED setup: Setup All 5 Ports
  FastLED.addLeds<LED_TYPE, PORT01_DATA_PIN, PORT01_CLOCK_PIN, COLOR_ORDER>(leds, 0, NUM_LEDS_PORT);
  FastLED.addLeds<LED_TYPE, PORT02_DATA_PIN, PORT02_CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS_PORT, NUM_LEDS_PORT);
  FastLED.addLeds<LED_TYPE, PORT03_DATA_PIN, PORT03_CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS_PORT * 2, NUM_LEDS_PORT);
  FastLED.addLeds<LED_TYPE, PORT04_DATA_PIN, PORT04_CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS_PORT * 3, NUM_LEDS_PORT);
  FastLED.addLeds<LED_TYPE, PORT05_DATA_PIN, PORT05_CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS_PORT * 4, NUM_LEDS_PORT);

  for(int i = 0; i<NUM_LEDS;i++) {
    leds_STM[i] = CRGB(0,255,0);
    leds2[i] = 0;
  }

  //FastLED color adjustments
  FastLED.setBrightness ( BRIGHTNESS );
  FastLED.setTemperature( COLOR_TEMPERATURE );
  FastLED.setCorrection ( COLOR_CORRECTION );
  FastLED.setDither     ( DITHER_MODE );

  set_max_power_in_volts_and_milliamps(5, 40000);

  Serial.println("Ready");
}

void loop() {
  unsigned long currentMillis = millis();
  
  if(currentMillis - previousMillis > interval) {
    // save the last time you blinked the LED 
    counter_frames++;
    
    previousMillis = currentMillis;
    output();
    if(counter_frames>=50) {
      counter_frames = 0;
    }
 
  }
  
}

void output() {
//  clearAll();
  fillAll(CRGB(0,0,0));
  if(counter_frames%5 == 0) {
    fillSaber(random8(NUM_SABERS),CRGB::White);
  }
//  randomPixels(5);
  if(counter_frames%49 == 0) {
//    flash();
  }
  addSTM();
  
  fadeSTM();
  //Output changes
  FastLED.show();
  
}

void clearAll() {
  for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
    }
}

// FILL ALL LEDS

void fillAll(CHSV color) {
  for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
    }
}

void fillAll(CRGB color) {
  for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
    }
}

///

// FILL SABER

void fillSaber(byte saber, CRGB color) {
  int offset = saber * NUM_SABER_LEDS;
  int numOffset = (saber * NUM_SABER_LEDS) + NUM_SABER_LEDS;
  for (int i = offset; i < numOffset; i++) {
    leds_STM[i] += color;
  }
}

void randomPixels(int count) {
  for(byte i = 0; i<count;i++) {
    int rndLEDS = random(NUM_LEDS);
    leds[rndLEDS] = CRGB(255,255,255);
  }
}


//STM

void addSTM() {
  for(int i = 0; i<NUM_LEDS;i++) {
    leds[i] += leds_STM[i];
  }
}

void fadeSTM() {
  for(int i = 0; i<NUM_LEDS;i++) {
    leds_STM[i].nscale8(192);
  }
}

void flash() {
  for( int i = 0; i < NUM_LEDS; i++) {
        leds_STM[i] += CRGB(0,0,255);
    }
}

