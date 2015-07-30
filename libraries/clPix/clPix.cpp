#include "Arduino.h"
#include "clCmd.h"
#include "../Adafruit_NeoPixel/Adafruit_NeoPixel.h"
// How to define these dynamically?
#define NEOPIN 5
#define NUMPIXELS 5

Adafruit_NeoPixel neo = Adafruit_NeoPixel(NUMPIXELS, NEOPIN, NEO_RGB + NEO_KHZ800);

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
  neo.begin();

  for(int i=0; i < _num; i++){
    neo.setPixelColor(i, neo.Color(0,0,0));
  }

  neo.show();

  Serial.println("clPix Setup Completed");
}



void ClPix::update(Command c) {
  // eat a command and update internal state based on it.
  Serial.println("ClPix Recieved: " + c.name + " Args: " + String(c.arg1) + ", " + String(c.arg2) + ", " + String(c.arg3));

  // switch on c.name
  if (c.name == "clpix-all") {
    allRand(uint16_t(c.arg1.toInt())); // POC!
  } else if (c.name == "clpix-clearall") {
    clearAll();
  } else if (c.name == "clpix-colorall") {
    rgb color = hexToColor(c.arg1);
    colorAll(color, uint16_t(c.arg2.toInt()));
  } else if (c.name == "clpix-color") {
    rgb color = hexToColor(c.arg2);
    setColor(c.arg1.toInt(), neo.Color(color.r, color.g, color.b), uint16_t(c.arg2.toInt()));
  } else if (c.name == "clpix-pulseall") {
    pulseAll();
  } else if (c.name == "clpix-pulse") {
    pulse(c.arg1.toInt());
  }

}

void ClPix::operate() {
  // look at internal state and do whatever changes needed.
  updatePulse();

  tween(); // wraps this for neo...
}

// ==== helper functions

rgb ClPix::getColors(uint32_t c) {

  uint8_t r = (uint8_t)(c >> 16);
  uint8_t g = (uint8_t)(c >> 8);
  uint8_t b = (uint8_t)(c >> 0);

  rgb a = {r, g, b};
  return a;
}

int ClPix::strToHex(char str[])
{
  Serial.println(str);
  int i = strtol(str, 0, 16);
  Serial.println(String(i));
  return (int) strtol(str, 0, 16);
}

rgb ClPix::hexToColor(String hex) {

  Serial.println("hexToColor: " + hex);

  char red[3];
  char green[3];
  char blue[3];

  hex.substring(0, 2).toCharArray(red, 3);
  hex.substring(2, 4).toCharArray(green, 3);
  hex.substring(4, 6).toCharArray(blue, 3);

  Serial.println("HEX COLORS ARE: " + String(red) + " " + String(green) + " " + String(blue));

  uint8_t r = strToHex(red);
  uint8_t g = strToHex(green);
  uint8_t b = strToHex(blue);

  rgb a = {r, g, b};
  return a;
}

// ==== internal functions ====

void ClPix::pulseAll() {
  for(int i=0; i<_num; i++){
    pulse(i);
  }
}

void ClPix::pulse(int led) {
  // random color first.
  int m = random(0,100);
  int n = random(0,100);
  int o = random(0,100);

  int x = random(100,200);
  int y = random(100,200);
  int z = random(100,200);

  setPulse(led, neo.Color(m, n, o), neo.Color(x, y, z), 500);
}

void ClPix::clearEffect(int led) {
  neoset[led].effect = "";
}

void ClPix::setPulse(int led, uint32_t color1, uint32_t color2, uint16_t inMillis)
{
  Serial.println("SetPulse called:" + String(led) +" " + String(color1) + " " + String(color2) + " : " + String(inMillis));

  neoset[led].effect = "pulse";
  neoset[led].color1 = color1;
  neoset[led].color2 = color2;
  neoset[led].timer = inMillis;
}


void ClPix::updatePulse() {
  // check the internal values - and update target and tts if needed - then tween takes care of the rest.
  for(int i=0; i<_num; i++){

    if (neoset[i].effect == "pulse" && neoset[i].ttl < 1) { // completed an oscillate so go again...
      Serial.println("Pulse complete for " + String(i));
      if (neoset[i].target == neoset[i].color1) {
        neoset[i].target = neoset[i].color2;
      } else {
        neoset[i].target = neoset[i].color1;
      }
      neoset[i].ttl = neoset[i].timer;
    }


  }
}



void ClPix::clearAll() {
  for(int i=0; i<_num; i++){
    clear(i);
  }
}

void ClPix::clear(int led) {
  setColor(led, neo.Color(0, 0, 0), 0);
  neoset[led].effect = ""; // stops effects..
}

void ClPix::allRand(uint16_t ttc) {
  Serial.println("Called allRand() randomizing " + String(_num) + " neopixels!" + " ttc set for " + String(ttc));
  for(int i=0; i<_num; i++){
    randColor(i, ttc);
  }
}

void ClPix::colorAll(rgb color, uint16_t ttc) {
  Serial.println("Called allColor() " + String(_num) + " neopixels!");
  for(int i=0; i<_num; i++){
    setColor(i,  neo.Color(color.r, color.g, color.b), ttc);
  }
}

void ClPix::randColor(int led, uint16_t ttc) {

    int m = random(0,200);
    int n = random(0,200);
    int o = random(0,200);

    setColor(led, neo.Color(m, n, o), ttc);
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


      uint32_t current = neo.getPixelColor(i);

      if (current != neoset[i].target) { // We are not at target - so no do the math.

        // Serial.println("neoset[i]" + String(i) + " target: " + String(neoset[i].target) + " ttl: " + String(neoset[i].ttl));
        if (neoset[i].ttl < 1 || delta > neoset[i].ttl) {
          Serial.println("Completed " + String(i));
          neo.setPixelColor(i, neoset[i].target); // just set the color and move on.
          neoset[i].ttl = 0;
        } else {

          // STEP TWEEN!

          rgb t = getColors(neoset[i].target);
          rgb c = getColors(neo.getPixelColor(i));

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

          neo.setPixelColor(i, neo.Color(r, g, b));
          neoset[i].ttl -= delta;
        }

      }

    }
  }


  neo.show();
}
