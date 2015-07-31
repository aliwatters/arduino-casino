#include "Arduino.h"
#include "clCmd.h"



ClCmd::ClCmd() {
  // this is the constructor
  _complete = false;
}

void ClCmd::init(int baud) {
  Serial.begin(baud);

  while (! Serial); // wait until ready.



  Serial.println(F("clCmd Setup Completed"));
}

Command ClCmd::getCmd(char ch) {
  // read what ever is on the serial buffer.

  Command cmd;
  cmd.name = "none";

  if (_complete) {
    Serial.println(_buffer);
    cmd = parse(_buffer);
    _buffer = "";
    _complete = false;
  }

  return cmd;
}

Command ClCmd::parse(String text) {
  Command cmd;
  cmd.name = "none";

  // parse the string here... according to our protocol.
  // <name>:<arg1>:<arg2>:<arg3>

  int p1 = text.indexOf(':');
  int p2 = text.indexOf(':', p1 + 1);
  int p3 = text.indexOf(':', p2 + 1);

  if (p1 != -1 && p2 != -1 && p3 != -1) {
    cmd.name = text.substring(0,p1);
    cmd.arg1 = text.substring(p1+1, p2);
    cmd.arg2 = text.substring(p2+1, p3);
    cmd.arg3 = text.substring(p3+1);
  } else {
    Serial.println(F("Invalid command"));
  }

  Serial.println("We have a " + cmd.name);
  return cmd;
}
