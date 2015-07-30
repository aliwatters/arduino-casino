#ifndef _CLSERVO_H_
#define _CLSERVO_H_

#include <Servo.h>

#include "Arduino.h"


class ClServo {
 public:
    ClServo(Servo servo, int pin);
    void init();
    void setNextSpeed(int nextSpeed);
    int getSpeed();
    void operate();
 private:
    Servo servo;
    int _pin;
    int _speed;
    int _nextSpeed;
    unsigned long _rest_begin;
    unsigned long _last_rest;
    Servo _servo;
};

#endif
