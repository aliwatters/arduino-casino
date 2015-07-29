#include "Arduino.h"
#include "clCmd.h"

ClCmd::ClCmd() {
  // this is the constructor
  _complete = false;

}

void ClCmd::init(int baud) {
  Serial.begin(baud);
  while (! Serial); // wait until ready.
  Serial.println("clCmd Setup Completed");
}

Command ClCmd::getCmd() {
  // read what ever is on the serial buffer.

  Command cmd;
  char ch;
  cmd.name = "none";
  String text;

  while(Serial.available() && _complete == false) {
      ch = Serial.read();
      if (ch == ';') {
        _complete = true;
      } else {
        _buffer += ch;
      }
  }

  if (_complete) {
    Serial.println("Echo: " + _buffer);
    cmd = parse(_buffer);
    clear();
  }

  return cmd;
}

Command ClCmd::parse(String text) {
  Command cmd;
  cmd.name = "parse_error";

  // parse the string here... according to our protocol.
  // <name>:<arg1>:<arg2>:<arg3>

  int p1 = text.indexOf(':');
  int p2 = text.indexOf(':', p1 + 1);
  int p3 = text.indexOf(':', p2 + 1);

  if (p1 != -1 && p2 != -1 && p3 != -1) {
    cmd.name = _buffer.substring(0,p1);
    cmd.arg1 = _buffer.substring(p1, p2);
    cmd.arg2 = _buffer.substring(p2, p3);
    cmd.arg3 = _buffer.substring(p3);
  } else {
    Serial.println("Invalid command");
  }

  return cmd;
}

void ClCmd::clear() {
  _buffer = "";
  _complete = false;
}
