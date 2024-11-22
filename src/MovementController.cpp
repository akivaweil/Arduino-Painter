#include "MovementController.h"

#include <Arduino.h>

#include "config.h"

MovementController::MovementController()
    : stepperX(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN),
      stepperY(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN),
      stepperRotation(AccelStepper::DRIVER, ROTATION_STEP_PIN,
                      ROTATION_DIR_PIN),
      motorsRunning(false)
{
}

void MovementController::setup()
{
    configureMotors();

    // Ensure spray is off initially
    pinMode(PAINT_RELAY_PIN, OUTPUT);
    digitalWrite(PAINT_RELAY_PIN, HIGH);
}

void MovementController::configureMotors()
{
    // X-axis configuration
    stepperX.setMaxSpeed(X_SPEED);
    stepperX.setAcceleration(X_ACCEL);
    stepperX.setPinsInverted(true);

    // Y-axis configuration
    stepperY.setMaxSpeed(Y_SPEED);
    stepperY.setAcceleration(Y_ACCEL);
    stepperY.setPinsInverted(false);

    // Rotation configuration
    stepperRotation.setMaxSpeed(ROTATION_SPEED);
    stepperRotation.setAcceleration(ROTATION_ACCEL);
    stepperRotation.setPinsInverted(false);  // Adjust based on your setup
}

void MovementController::updatePositionCache() const
{
    // Const cast is safe here because we're only reading values
    lastXPos = const_cast<AccelStepper&>(stepperX).currentPosition();
    lastYPos = const_cast<AccelStepper&>(stepperY).currentPosition();
    lastRotationPos =
        const_cast<AccelStepper&>(stepperRotation).currentPosition();
}

void MovementController::setRotationPosition(long position)
{
    stepperRotation.setCurrentPosition(position);
    updatePositionCache();
}

void MovementController::setRotationSpeed(float speed)
{
    stepperRotation.setMaxSpeed(speed);
}

long MovementController::getCurrentRotationSteps() const
{
    return lastRotationPos;
}

float MovementController::getCurrentRotationAngle() const
{
    return stepsToAngle(lastRotationPos);
}

float MovementController::stepsToAngle(long steps) const
{
    return (static_cast<float>(steps) * 360.0) / STEPS_PER_ROTATION;
}

bool MovementController::executeCommand(const Command& cmd)
{
    updateSprayControl(cmd);

    long targetSteps = 0;

    switch (cmd.type)
    {
        case 'X':  // Relative X movement
            stepperX.move(cmd.value * X_STEPS_PER_INCH);
            break;

        case 'Y':  // Relative Y movement
            stepperY.move(cmd.value * Y_STEPS_PER_INCH);
            break;

        case 'M':  // Absolute X movement
            stepperX.moveTo(cmd.value * X_STEPS_PER_INCH);
            break;

        case 'N':  // Absolute Y movement
            stepperY.moveTo(cmd.value * Y_STEPS_PER_INCH);
            break;

        case 'R':  // Rotation movement (in degrees)
        {
            // Convert degrees to steps
            targetSteps = (cmd.value * STEPS_PER_ROTATION) / 360;
            stepperRotation.move(targetSteps);
            break;
        }

        case 'S':  // Spray control only
            // Already handled by updateSprayControl
            break;

        default:
            Serial.println(F("ERROR: Invalid movement command type"));
            return false;
    }

    return true;
}

void MovementController::updateSprayControl(const Command& cmd)
{
    if (cmd.type == 'S')
    {
        digitalWrite(PAINT_RELAY_PIN, cmd.sprayOn ? LOW : HIGH);
    }
    else if (cmd.sprayOn && (cmd.type == 'X' || cmd.type == 'Y' ||
                             cmd.type == 'M' || cmd.type == 'N'))
    {
        digitalWrite(PAINT_RELAY_PIN, LOW);
    }
}

bool MovementController::isMoving() const { return motorsRunning; }

void MovementController::stop()
{
    stepperX.stop();
    stepperY.stop();
    stepperRotation.stop();
    digitalWrite(PAINT_RELAY_PIN, HIGH);  // Ensure spray is off
    motorsRunning = false;
}

long MovementController::getCurrentXSteps() const { return lastXPos; }

long MovementController::getCurrentYSteps() const { return lastYPos; }

float MovementController::stepsToInches(long steps, long stepsPerInch) const
{
    return static_cast<float>(steps) / stepsPerInch;
}

void MovementController::stopMovement()
{
    // Immediately stop all motors
    stepperX.stop();
    stepperY.stop();
    stepperRotation.stop();

    // Force an immediate update of the motors to ensure they stop
    stepperX.run();
    stepperY.run();
    stepperRotation.run();

    motorsRunning = false;
    digitalWrite(PAINT_RELAY_PIN, HIGH);  // Ensure spray is off
}

void MovementController::update()
{
    // Update all stepper positions
    stepperX.run();
    stepperY.run();
    stepperRotation.run();

    // Update position cache
    updatePositionCache();

    // Update motors running status
    bool previouslyRunning = motorsRunning;
    motorsRunning = stepperX.isRunning() || stepperY.isRunning() ||
                    stepperRotation.isRunning();

    // If motors just stopped running, turn off spray
    if (previouslyRunning && !motorsRunning)
    {
        digitalWrite(PAINT_RELAY_PIN, HIGH);
    }
}

void MovementController::setXPosition(long position)
{
    stepperX.setCurrentPosition(position);
    updatePositionCache();
}

void MovementController::setYPosition(long position)
{
    stepperY.setCurrentPosition(position);
    updatePositionCache();
}

void MovementController::setXSpeed(float speed) { stepperX.setMaxSpeed(speed); }

void MovementController::setYSpeed(float speed) { stepperY.setMaxSpeed(speed); }