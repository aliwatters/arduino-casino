/*
  clPix.h - simple wrapper for neopixels - offering state based behavior
*/
#ifndef clPixMin_h
#define clPixMin_h
#define NUMPIXELS 5

#include "Arduino.h"
#include "../Adafruit_NeoPixel/Adafruit_NeoPixel.h"
#include "clCmd.h"


/*
Protocol:

clpix-clearall:::;

clpix-all:0::; // random color on each pixel

clpix-colorall:hexrgb:millis:; // eg. red ff0000 sets all pixels to color.

eg. clpix-colorall:aa0000:200:;

clpix-color:led:hexrgb:millis;

eg. clpix-color:1:bb00bb:100;

The CL rainbow:

clpix-color:0:000033:100;clpix-color:1:994444:100;
clpix-color:2:993300:100;clpix-color:3:003311:100;
clpix-color:4:005500:100;
*/

typedef struct rgb {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

typedef struct pixelType {
  // Primary attributes - used by tween.
  uint32_t target; // target color.
  uint16_t ttl; // milisecs remaining
  uint32_t last; // last change on this pixel.
};


class ClPixMin
{
  public:
    ClPixMin(uint32_t * tick, int pin, int num);
    void init();
    void update(Command cmd);
    void operate();
    void allRand(uint16_t ttc);
    void clearAll();
    void clear(int led);
    void colorAll(rgb color, uint16_t tcc); // change ALL leds to color in tcc millis
    rgb getColors(uint32_t c);
    rgb hexToColor(String hex);
    int strToHex(char str[]);
  private:
    uint32_t * _tick; // a "pointer"* to global tick. (passed currently.)
    uint8_t _pin; // output pin from arduino
    uint8_t _num; // number of neopixels chained
    pixelType neoset[NUMPIXELS];
    void tween();
    void randColor(int led, uint16_t ttc);
    void setColor(int led, uint32_t color, uint16_t inMillis);
};

#endif
