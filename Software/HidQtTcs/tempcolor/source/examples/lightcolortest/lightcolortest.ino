#include "tempcolor.h"
#include "stdio.h"
//#include "RT.h"  // For unnecessarily fancy people
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

///////////////////////////////

// Which pin are you using?
const uint8_t pin = A3;

// How many LEDs do you have?
const uint8_t numpixels = 1;

// Are your LEDs RGB (0) or GRB (1)?
const bool format = 1;

uint8_t framerate = 62; // Hz

///////////////////////////////

float color = 0.0;
float brite = 0.0;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(numpixels, pin, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin();
  pixels.show();
  
  if(format == 0)
    temp.RGB();
  else
    temp.GRB();

//rt.init(framerate);  // Fancy
}

void loop() {
  for(int i = 0; i < numpixels; i++) {
    brite = (float)(i + 1) / numpixels;
    pixels.setPixelColor(i, temp.color(float(sin(color)), brite));
  }
  
  pixels.show();

  if(color >= 6.28)
    color = 0.0;
  else
    color += 0.02;

  delay(1000/framerate);  // If you're okay with not being fancy
//  rt.cycle();  // If you prefer to be fancy
}