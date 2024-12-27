#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <Arduino.h>
#include <Servo.h>

class ServoController
{
   public:
    ServoController();
    void setup();
    bool setAngle(int angle);
    int getCurrentAngle() const;
    void update();  // For any continuous updates if needed

   private:
    Servo servo;
    int currentAngle;
    bool isAngleValid(int angle) const;
};

#endif