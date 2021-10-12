// tempcolor.cpp
// Travis Llado, 2016

// Temp.Color converts light temperatures from 1000K to
// 40,000K, as well as desired brightness, into light
// color codes for WS2812 LEDs.
// Conversion data is provided by Mitch Charity at Vendian.
// (http://www.vendian.org/mncharity/dir3/blackbody/)

#include "tempcolor.h"

tempClass temp;

// color
// Accepts two values (for temperature and brightness) and
// returns the corresponding neopixel color code (32b int)
uint32_t tempClass::color(double T, double brite) {
  if(brite >= 0.0 && brite <= 1.0) {
    uint8_t Red = R(T)*brite;
    uint8_t Grn = G(T)*brite;
    uint8_t Blu = B(T)*brite;

    if(format == rgb)
      return (uint32_t)Red << 16 | (uint32_t)Grn <<  8 | Blu;
    else
      return (uint32_t)Grn << 16 | (uint32_t)Red <<  8 | Blu;
  }
  return 0;
}

void tempClass::RGB() {
  format = rgb;
}

void tempClass::GRB() {
  format = grb;
}

uint8_t tempClass::R(double T) {
  if(T >= -1.0) {
    if(T <= 0.0)
      return 255*(1);
    if(T <= 1.0)
      return 255*(-0.990*T*T*T + 2.34*T*T - 1.99*T + 0.970);
  }
  return 0;
}

uint8_t tempClass::G(double T) {
  if(T >= -1.0) {
    if(T <= 0.0)
      return 255*(-0.402*T*T*T - 0.211*T*T + 1.09*T + 0.958);
    if(T <= 1.0)
      return 255*(-0.542*T*T*T + 1.37*T*T - 1.28*T + 0.941);
  }
  return 0;
}

uint8_t tempClass::B(double T) {
  if(T >= -1.0) {
    if(T <= -0.7)
      return 255*(0);
    if(T <= 0.0)
      return 255*(0.0117*T*T*T + 2.05*T*T + 2.85*T + 1.00);
    if(T <= 1.0)
      return 255*(1);
  }
  return 0;
}

// overloaded functions so user can give color and brightness
// as either double or int
uint32_t tempClass::color(double T, int brite) {
  double f_brite = (double)brite / 255.0;
  return color(T,f_brite);
}

uint32_t tempClass::color(int T, double brite) {
  double f_T = (log(T) - 8.79) / 1.8;
  return color(f_T,brite);
}

uint32_t tempClass::color(long int T, double brite) {
  double f_T = (log(T) - 8.79) / 1.8;
  return color(f_T,brite);
}

uint32_t tempClass::color(int T, int brite) {
  double f_T = (log(T) - 8.79) / 1.8;
  double f_brite = (double)brite / 255.0;
  return color(f_T,f_brite);
}

uint32_t tempClass::color(long int T, int brite) {
  double f_T = (log(T) - 8.79) / 1.8;
  double f_brite = (double)brite / 255.0;
  return color(f_T,f_brite);
}