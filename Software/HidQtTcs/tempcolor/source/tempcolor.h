// tempcolor.h
// Travis Llado, 2016

// Temp.Color converts light temperatures from 1000K to
// 40,000K, as well as desired brightness, into light
// color codes for WS2812 LEDs.
// Conversion data is provided by Mitch Charity at Vendian.
// (http://www.vendian.org/mncharity/dir3/blackbody/)

#ifndef TEMPCOLOR_H
#define TEMPCOLOR_H

//#include "Arduino.h"
#include <inttypes.h>
#include <math.h>

#define rgb 0
#define grb 1

class tempClass {
public:
	uint32_t color(double temp, double brightness);
	uint32_t color(double temp, int brightness);
	uint32_t color(int temp, double brightness);
	uint32_t color(long int temp, double brightness);
	uint32_t color(int temp, int brightness);
	uint32_t color(long int temp, int brightness);
	void RGB();
	void GRB();
private:
	uint8_t R(double);
	uint8_t G(double);
	uint8_t B(double);
	bool format = rgb;
};

extern tempClass temp;

#endif
