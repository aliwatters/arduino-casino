#include "Arduino.h"
#include "clCmd.h"
#include <Servo.h>

#include "clServo.h"

const int MAX_RUNTIME = 10000;
const int REST_LENGTH = 2000;

ClServo::ClServo(Servo servo, int pin, uint16_t zero) { // Constructor
    _servo = servo;
    _pin = pin;
    _zero = zero;
}

void ClServo::init() {
    _servo.attach(_pin);
    _servo.write(_zero);
    _speed = _zero;
    _nextSpeed = _zero;
    _lastrun = millis();
    _running = 0;
    _resting = REST_LENGTH + 1;
    Serial.println(F("clServo Setup Completed"));
}

void ClServo::update(Command c) {
  Serial.println("ClServo Recieved: " + c.name + " Args: " + String(c.arg1) + ", " + String(c.arg2) + ", " + String(c.arg3));


  if (c.name == "clservo-set") {
    setNextSpeed(c.arg1.toInt());
  } else if (c.name == "clservo-stop") {
    stop();
  }
}

void ClServo::stop() {
  _nextSpeed = _zero;
  _speed = _zero;
}

void ClServo::setNextSpeed(int nextSpeed) {
    Serial.println("ClServo-set speed to: " + String(nextSpeed));
    int constrained_speed = constrain(nextSpeed, 0, 180);

    _nextSpeed = constrained_speed;
    _lastCmd = millis(); //
}

int ClServo::getSpeed() {
    return _speed;
}

void ClServo::operate() {

/*
    if ((now - _lastCmd) > MAX_RUNTIME) {
      _speed = _zero;
      _nextSpeed = _zero;
    }  else {
      _speed = _nextSpeed;
    }
    */

    int now = millis();
    int delta = (now - _lastrun);
    if (_speed != _zero) {
      _running += delta;
      _resting = 0;
    } else {
      _resting += delta;
    }

    if (_resting > REST_LENGTH) {
      _running = 0;
    }

    if (_resting > 1000000) {
      _resting = REST_LENGTH + 1;
    }


    if (_running > MAX_RUNTIME && _resting < REST_LENGTH) {
      _speed = _zero;
      _nextSpeed = _zero;
    } else {
      _speed = _nextSpeed;
    }

    _servo.write(_speed);

    _lastrun = now;

/*
    unsigned long delta;
    unsigned long currentTime = millis();

    if ((_nextSpeed > _zero) && (millis() - _last_rest > MAX_RUNTIME)) {
        setNextSpeed(_zero); // Take a break
    } else {
        // Respect their wishes

    }
    if ((_speed == _zero) && (_nextSpeed > _zero)) { // resting -> running

    } else if ((_speed == _zero) && (_nextSpeed == _zero)) { // resting -> resting
        delta = millis() - _rest_begin;
        if (delta >= REST_LENGTH) {
            _last_rest = millis();
        } else {
            // Haven't been resting long enough for it to "count"
        }
    } else if ((_speed > _zero) && (_nextSpeed == _zero)) { // running -> resting
        _rest_begin = millis();
    } else { // running -> running

    }
    _speed = _nextSpeed;
    _servo.write(_speed);
    if (millis() % 1000 == 0) {
        Serial.print(F("[ARDUINO] speed: "));
        Serial.println(_speed);
    }
    */
}
