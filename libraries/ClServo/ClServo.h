#ifndef _CLSERVO_H_
#define _CLSERVO_H_

#include <Servo.h>
#include "clCmd.h"
#include "Arduino.h"


class ClServo {
 public:
    ClServo(Servo servo, int pin, uint16_t zero);
    void init();
    void setNextSpeed(int nextSpeed);
    void stop();
    int getSpeed();
    void operate();
    void update(Command c);
 private:
    int _pin;
    int _speed;
    int _nextSpeed;
    int _lastCmd;
    Servo _servo;
    int _zero;
    int _running;
    int _resting;
    int _lastrun;
};

#endif
