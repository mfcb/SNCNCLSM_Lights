//FASTLED INCLUDES
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

//MIDI INCLUDES
#include <MIDI.h>

//DMX INCLUDES
#include <Conceptinetics.h>
#include <Rdm_Defines.h>
#include <Rdm_Uid.h>


//MIDI defines

#define MIDI_CHANNEL 16
#define CLOCK_DIVIDER 12
#define MIDI_OFFSET 12

//FASTLED defines

#define LED_TYPE WS2812B

#define NUM_LEDS 240
#define NUM_SABERS 12
#define NUM_SABER_LEDS 20
#define DATA_PIN 5

#define BRIGHTNESS 255
#define COLOR_TEMPERATURE CRGB(255,255,255)
#define COLOR_CORRECTION TypicalLEDStrip
#define DITHER_MODE 0

#define COLOR_ORDER GRB

//FASTLED globals

struct CRGB leds[NUM_LEDS];

int currentLED = 0;
int currentSaber = 0;

int clockCounter = 0;

CHSV mainColor = CHSV(255,0,255);


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

void setup() {
  //Set MIDI Baud rate
  Serial.begin(57600); //57600

  //Wait 1 Second to protect LEDs from current onslaught
  delay(1000);

  //FastLED setup
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);

  //FastLED color adjustments
  FastLED.setBrightness ( BRIGHTNESS );
  FastLED.setTemperature( COLOR_TEMPERATURE );
  FastLED.setCorrection ( COLOR_CORRECTION );
  FastLED.setDither     ( DITHER_MODE );
  
  //MIDI setup
  MIDI.begin(MIDI_CHANNEL);

  // As of the MIDI Library v3.1, the lib uses C style function 
  // pointers to create a callback system for handling input events. 
  MIDI.setHandleNoteOn(HandleNoteOn); 
  MIDI.setHandleControlChange(HandleCC);
  MIDI.setHandleNoteOff(HandleNoteOff);

  MIDI.setHandleClock(HandleClock);
  
  set_max_power_in_volts_and_milliamps(5, 6000);

  blinkLedToShowThatWeAreReady();

  

}

void loop() {
    MIDI.read();    
}

//MIDI Functions

void HandleNoteOn(byte channel, byte pitch, byte velocity) 
{ 
  if(pitch == NOTE_C1) {
    
    fillAll(CRGB::White,velocity);
    FastLED.show();
    
  } else {
      int saber = pitch - NOTE_C1 - 1;
      if(saber>=0 && saber < NUM_SABERS) {
        fillSaber(saber,CRGB::White, velocity);
      }
   }
}

void HandleCC(byte channel, byte number, byte value) 
{
     mainColor = CHSV(255,0,value*2);
}

void HandleNoteOff(byte channel, byte pitch, byte velocity) 
{
  // Do something here with your data!
  if(pitch == NOTE_C1) {
    lightsOut();
  } else {
    int saber = pitch - NOTE_C1 - 1;
    if(saber>=0 && saber < NUM_SABERS) {
      
      fillSaber(saber, CRGB(0,0,0), 0);
      
    } else {
      //lightsOut();
    }  
  }
  
  
  
}

void HandleClock() {
  currentLED++;
}

void  blinkLedToShowThatWeAreReady(){
  for ( int i = 0; i<3; i++) {
    fill_solid(leds, NUM_LEDS, CRGB::White);
    FastLED.show();
    delay(200);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(200);
  }
}

void lightsOut() {
  fill_solid(leds,NUM_LEDS,CRGB::Black);
  FastLED.show();
}

void fillSaber(int saber, CRGB Color, int brightness) {
  for (int i=saber*NUM_SABER_LEDS;i<(saber+1) * NUM_SABER_LEDS;i++) {    
    leds[i] = mainColor;
    //FastLED.setBrightness ( brightness*2 );
  }
  FastLED.show();
}

void fillAll(CRGB Color, int brightness) {
  fill_solid(leds,NUM_LEDS,mainColor);
  //FastLED.setBrightness ( brightness*2 );
  FastLED.show();
}

