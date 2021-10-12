#include <Arduino.h>

#include "Adafruit_TinyUSB.h"
//Include bitbang i2c
#include "BitBang_I2C.h"
// PyBase SDA4 SCL5
#define SDA_PIN 4
#define SCL_PIN 5
BBI2C bbi2c;
//Include bitbang TCS
#include "bitbangTCS34725.h"
//#include "bitbangTCS34725autogain.h" //NG
TCS34725 tcs;

#define TCS_LED 0

#include "Adafruit_NeoPixel.h"
//pyBase AD2:pin28, 8 pixels
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(8, 28, NEO_GRB + NEO_KHZ800);

void ledTCS(bool onoff) {
    pinMode(TCS_LED, OUTPUT);
    digitalWrite(TCS_LED, onoff);
}

void setupTCS(void) {
    //Serial.begin(115200);
    //Setup bitbang i2c
    memset(&bbi2c, 0, sizeof(bbi2c));
    bbi2c.bWire = 0;  // use bit bang, not wire library
    bbi2c.iSDA = SDA_PIN;
    bbi2c.iSCL = SCL_PIN;
    I2CInit(&bbi2c, 440000L);
    delay(100);  // allow devices to power up

    pinMode(TCS_LED, OUTPUT);
    digitalWrite(TCS_LED, LOW);  //Turn Off

    if (!tcs.attach(&bbi2c)) {
        //Serial.println("ERROR: TCS34725 NOT FOUND !!!");
        SerialTinyUSB.println("ERROR: TCS34725 NOT FOUND !!!");
    } else {
        //tcs.integrationTime(600);  // ms
        //tcs.gain(TCS34725::Gain::X01); //Try Gain4
        //SerialTinyUSB.println("TCS 500ms intergration started!");
        //if (tcs.autoGain()) {
        //    SerialTinyUSB.println("AutoGain:OK !!!");
        //} else {
        tcs.integrationTime(400);       //400ms intergration time
        tcs.gain(TCS34725::Gain::X04);  //Try Gain4X
        //    SerialTinyUSB.println("AutoGain:NG !!!");
        //}
        // don't generate an interrupt on every completed read, but only for reads
        // whose clear count was outside the threshold range
        //tcs.persistence(0x01);
    }
    // set LEDs...
    //digitalWrite(TCS_LED, HIGH);

    // This initializes the NeoPixel with low RED
    pixels.begin();
    pixels.setBrightness(16);
    pixels.fill(0x880000);
    pixels.show();
}

extern bool isHidReadingTCS;
void loopTCS(void) {
    static unsigned long lastMillis = 0;
    if (tcs.available()) {  //TCS cycle completed!
        //static unsigned long prev_ms = millis();
        char buf[255];

        TCS34725::Color color = tcs.color();
        /*
        Serial.print("Interval   : "); Serial.println(millis() - prev_ms);
        Serial.print("Color Temp : "); Serial.println(tcs.colorTemperature());
        Serial.print("Lux        : "); Serial.println(tcs.lux());
        Serial.print("R          : "); Serial.println(color.r);
        Serial.print("G          : "); Serial.println(color.g);
        Serial.print("B          : "); Serial.println(color.b);
        */
        //TCS34725::RawData raw = tcs.raw();
        /*
        Serial.print("Raw R      : "); Serial.println(raw.r);
        Serial.print("Raw G      : "); Serial.println(raw.g);
        Serial.print("Raw B      : "); Serial.println(raw.b);
        Serial.print("Raw C      : "); Serial.println(raw.c);
        */
        unsigned long int_time = tcs.getIntegrationTime();
        long gain = tcs.getGain();
        if ((millis() - lastMillis) > int_time) {  //keep speed w/ integration time
            lastMillis = millis();
            sprintf(buf, "Int:% 3ldms Gain:x%02ld Temp:% 5.0f° Lux:%4.0f RGB(%3.0f,%3.0f,%3.0f)  RGBC:%3u/%3u/%3u/%3u ",
                    int_time, gain, tcs.colorTemperature(), tcs.lux(), color.r, color.g, color.b,
                    tcs.raw().r, tcs.raw().g, tcs.raw().b, tcs.raw().c);
            //prev_ms = millis();

            //Serial.println(buf);
            SerialTinyUSB.println(buf);

            //Update NeoPixels when HID reading  -------------------------------
            if (isHidReadingTCS) {
                int l, r, g, b;
                l = map((int)tcs.lux(), 0, 1024, 0, 255);
                l = constrain(l, 0, 255);
                r = color.r;
                g = color.g;
                b = color.b;
                int c = (r << 16) | (g << 8) | b;

                pixels.setBrightness(l);
                pixels.fill(c);
                pixels.show();
            }
        }
    }
}
