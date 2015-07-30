#include <Servo.h>
#include <ClServo.h>

int servoPin = 9;

Servo servo;

ClServo cl_servo = ClServo(servo, servoPin);

boolean handshakePerformed = false;

void setup() {
  Serial.begin(9600);
  cl_servo.init();
}

void loop() {
  int inSpeed;
  
  cl_servo.setNextSpeed(cl_servo.getSpeed());
  if (Serial.available()) {
    if (!handshakePerformed) {
      inSpeed = Serial.read();
      Serial.println("ack");
      handshakePerformed = true;
      return;
    }
    inSpeed = Serial.read();
    cl_servo.setNextSpeed(inSpeed); 
  }
  cl_servo.operate();
}
