/*
  clCmd.h - read a string from the serial monitor parse and return struct
*/
#ifndef clCmd_h
#define clCmd_h

#include "Arduino.h"

typedef struct Command {
  String name;
  String arg1; // optional
  String arg2; // optional
  String arg3; // optional
};

class ClCmd
{
  public:
    ClCmd();
    void init(int baud);
    Command getCmd();
    Command parse(String text);
  private:
    boolean _complete;
    String _buffer;
    void clear();
};

#endif
