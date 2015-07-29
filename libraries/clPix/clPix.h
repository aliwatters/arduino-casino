/*
  clPix.h - simple wrapper for neopixels - offering state based behavior
*/
#ifndef clPix_h
#define clPix_h
#define NUMPIXELS 5

#include "Arduino.h"
#include "../Adafruit_NeoPixel/Adafruit_NeoPixel.h"
#include "clCmd.h"

typedef struct rgb {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

typedef struct pixelType {
  uint32_t target; // target color.
  uint16_t ttl; // milisecs remaining
  boolean pulse; // pulse up an down ~50 on each
  uint32_t pulsecolor;
};


class ClPix
{
  public:
    ClPix(uint32_t * tick, int pin, int num);
    void init();
    void update(Command cmd);
    void operate();
    void allRand(uint16_t ttc);
    void clearAll();
    void clear(int led);
    rgb getColors(uint32_t c);
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
