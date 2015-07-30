#include "Arduino.h"

#include <Servo.h>

#include "clServo.h"

const int SERVOZERO = 98;
const int REST_LENGTH = 2000;
const int MAX_RUNTIME = 10000;

ClServo::ClServo(Servo servo, int pin) { // Constructor
    _servo = servo;
    _pin = pin;
}

void ClServo::init() {
    _servo.attach(_pin);
    _servo.write(SERVOZERO);
    Serial.println("Just wrote speed:\n");
    Serial.println(SERVOZERO);
    _speed = SERVOZERO;
    _rest_begin = millis();
    _last_rest = millis();
}

void ClServo::setNextSpeed(int nextSpeed) {
    int constrained_speed = constrain(nextSpeed, 0, 180);
    
    _nextSpeed = constrained_speed;
}

int ClServo::getSpeed() {
    return _speed;
}

void ClServo::operate() {
    unsigned long delta;
    unsigned long currentTime = millis();

    if ((_nextSpeed > SERVOZERO) && (millis() - _last_rest > MAX_RUNTIME)) {
        setNextSpeed(SERVOZERO); // Take a break
    } else {
        // Respect their wishes
    }
    if ((_speed == SERVOZERO) && (_nextSpeed > SERVOZERO)) { // resting -> running

    } else if ((_speed == SERVOZERO) && (_nextSpeed == SERVOZERO)) { // resting -> resting
        delta = millis() - _rest_begin;
        if (delta >= REST_LENGTH) {
            _last_rest = millis();
        } else {
            // Haven't been resting long enough for it to "count"
        }
    } else if ((_speed > SERVOZERO) && (_nextSpeed == SERVOZERO)) { // running -> resting
        _rest_begin = millis();
    } else { // running -> running
        
    }
    _speed = _nextSpeed;
    _servo.write(_speed);
    if (millis() % 1000 == 0) {
        Serial.print("[ARDUINO] speed: ");
        Serial.println(_speed);
    }
}
