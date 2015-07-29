# arduino-casino
Framework for delay-less state based arduino libraries.

Initially built for a www.creativelive.com Hack Week :) with no prior arduino experience and some very rusty C.

Open up the sketch cmd-lib - and things should become clearer.

Each libraries that have an constructor, init, update, operate interface.

- update - parse a command and set internal state.
- operate - based on internal state - change based on internal state - never block, be quick.

Then loop becomes a lot simpler.. 

```
void loop() {

  // get a command from Serial or Network etc.
  Command c = clCmd.getCmd();

  // set states below.
  if (c.name != "none") {
    // set new states
    Serial.println("Execute cmd: " + c.name);
    Serial.println("Arg1 " + c.arg1 + " Arg2 " + c.arg2 +" Arg3 " + c.arg3);

    clPix.update(c);
    // clServo.update(c);
  }

  // do stuff
  clPix.operate();
  // clServo.operate();

  // and loop again.
  tick = millis();
}
```

clCmd - is a simple serial monitor -> command parser.
clPix - a wrapper for NeoPixels - with a state based tween operation.

TODO: 
- Publish a wiring guide with each library.
- Fix many bugs - main one - the pointer to tick needs to update in the clPix lib
- Remove loads of Serial.println


Why casino? - it rhymes.
