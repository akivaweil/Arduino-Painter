#include "ServoController.h"

#include "config.h"

ServoController::ServoController() : currentAngle(SERVO_DEFAULT_ANGLE) {}

void ServoController::setup()
{
    // Attach servo and ensure it's properly initialized
    if (servo.attach(SERVO_PIN))
    {
        Serial.println(F("Servo OK"));
    }
    else
    {
        Serial.println(F("Servo Error"));
    }

    // Move to default position
    setAngle(SERVO_DEFAULT_ANGLE);
}

bool ServoController::setAngle(int angle)
{
    Serial.print(F("Servo - Angle: "));
    Serial.println(angle);

    if (!isAngleValid(angle))
    {
        Serial.print(F("Invalid angle: "));
        Serial.println(angle);
        return false;
    }

    currentAngle = angle;
    servo.write(angle);
    Serial.println(F("Servo angle updated successfully"));
    delay(15);  // Allow servo to move

    return true;
}

int ServoController::getCurrentAngle() const { return currentAngle; }

void ServoController::update()
{
    // For future use if needed
}

bool ServoController::isAngleValid(int angle) const
{
    return angle >= SERVO_MIN_ANGLE && angle <= SERVO_MAX_ANGLE;
}