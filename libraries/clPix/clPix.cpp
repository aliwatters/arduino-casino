#include "Arduino.h"
#include "clCmd.h"
#include "../Adafruit_NeoPixel/Adafruit_NeoPixel.h"
// How to define these dynamically?
#define NEOPIN 5
#define NUMPIXELS 5

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIN, NEO_RGB + NEO_KHZ800);

#include "clPix.h"

ClPix::ClPix(uint32_t * tick, int pin, int num) {
  // this is the constructor

// neoset is out internal state control
pixelType neoset[NUMPIXELS];


// TODO - construct Adafruit here...
_tick = tick;
_num = num;
_pin = pin;
}

void ClPix::init() {
  // ClCmd set up the serial monitor...

  // clear and show.
  pixels.begin();

  for(int i=0; i < _num; i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0));
  }

  pixels.show();

  Serial.println("clPix Setup Completed");
}

void ClPix::update(Command c) {
  // eat a command and update internal state based on it.
  Serial.println("ClPix Recieved: " + c.name + "Args: " + String(c.arg1) + ", " + String(c.arg2) + ", " + String(c.arg3));

  // switch on c.name
  if (c.name == "clpix-all") {
    allRand(uint16_t(c.arg1.toInt())); // POC!
  }
  if (c.name == "clpix-clearall") {
    clearAll();
  }

}

void ClPix::operate() {
  // look at internal state and do whatever changes needed.
  tween(); // wraps this for pixels...
}

// ==== helper functions

rgb ClPix::getColors(uint32_t c) {

  uint8_t r = (uint8_t)(c >> 16);
  uint8_t g = (uint8_t)(c >> 8);
  uint8_t b = (uint8_t)(c >> 0);

  rgb a = {r, g, b};
  return a;
}

// ==== internal functions ====

void ClPix::clearAll() {
  for(int i=0; i<_num; i++){
    clear(i);
  }
}

void ClPix::clear(int led) {
  setColor(led, pixels.Color(0, 0, 0), 0);
}

void ClPix::allRand(uint16_t ttc) {
  Serial.println("Called allRand() randomizing " + String(_num) + " neopixels!" + " ttc set for " + String(ttc));
  for(int i=0; i<_num; i++){
    randColor(i, ttc);
  }
}

void ClPix::randColor(int led, uint16_t ttc) {

    int m = random(0,200);
    int n = random(0,200);
    int o = random(0,200);

    setColor(led, pixels.Color(m, n, o), ttc);
}


void ClPix::setColor(int led, uint32_t color, uint16_t inMillis)
{
  Serial.println("SetColor called:" + String(led) +" " + String(inMillis));
  for(int i=0; i<_num; i++){

    if (led == i) {

      neoset[i].target = color;
      neoset[i].ttl = inMillis;
      // TODO - clear pulse values
    }

  }
}


void ClPix::tween() {
  // loop through all the pixels - work out the tween for each and do it...
  // use millis.

  int tock = millis();
  int delta = tock - int(* _tick); // note _tick is a pointer - loop needs to update.


  bool changed = false;

  if (delta > 0) {

    //Serial.println("++++++++ delta " + String(delta));

    for(int i=0;i<NUMPIXELS;i++){


      uint32_t current = pixels.getPixelColor(i);

      if (current != neoset[i].target) { // We are not at target - so no do the math.

        //Serial.println("neoset[i]" + String(i) + " target: " + String(neoset[i].target) + " ttl: " + String(neoset[i].ttl));
        if (neoset[i].ttl == 0 || delta > neoset[i].ttl) {
          Serial.println("Completed " + String(i));
          pixels.setPixelColor(i, neoset[i].target); // just set the color and move on.
          neoset[i].ttl = 0;
        } else {

          // STEP TWEEN!

          rgb t = getColors(neoset[i].target);
          rgb c = getColors(pixels.getPixelColor(i));

/*
            Serial.println(String(i) + " R currently " + String(c.r)  + " tweening to " + String(t.r));
            Serial.println(String(i) + " G currently " + String(c.g)  + " tweening to " + String(t.g));
            Serial.println(String(i) + " B currently " + String(c.b)  + " tweening to " + String(t.b));

            Serial.println("Last run " + String(delta) + " ms ago");
*/
          // calculate step color...

          boolean rUp = (t.r - c.r) > 0;
          boolean gUp = (t.g - c.g) > 0;
          boolean bUp = (t.b - c.b) > 0;

          int chunk = round(neoset[i].ttl / delta);
          if (chunk < 1) chunk = 1;

          //Serial.println("Chunk is " + String(chunk) + " TTL is " + String(neoset[i].ttl));
          // red step
          int rS = round((t.r - c.r) / chunk);
          if (rS == 0) {
            rUp ? rS += 1: rS -= 1;
          }
          if (t.r == c.r) rS = 0;

          //Serial.println("Red change " + String(rS));
          //Serial.println("Red diff " + String(t.r - c.r)  + " diff/chunk: " + String((t.r - c.r) / chunk));
          int gS = round((t.g - c.g) / chunk);
          if (gS == 0) {
            gUp ? gS += 1: gS -= 1;
          }
          if (t.g == c.g) gS = 0;

          //Serial.println("Green change " + String(gS));

          int bS = round((t.b - c.b) / chunk);
          if (bS == 0) {
            bUp ? bS += 1: bS -= 1;
          }
          if (t.b == c.b) bS = 0;

          //Serial.println("Blue change " + String(bS));

          int r = c.r + rS;
          int g = c.g + gS;
          int b = c.b + bS;

          pixels.setPixelColor(i, pixels.Color(r, g, b));
          neoset[i].ttl -= delta;
        }

      }

    }
  }


  pixels.show();
}
