#include "MovementController.h"

#include <Arduino.h>

#include "config.h"

MovementController::MovementController()
    : stepperX(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN),
      stepperY(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN),
      stepperRotation(AccelStepper::DRIVER, ROTATION_STEP_PIN,
                      ROTATION_DIR_PIN),
      motorsRunning(false),
      stateManager(nullptr),
      frontSpeed(X_SPEED),
      backSpeed(X_SPEED),
      leftSpeed(X_SPEED),
      rightSpeed(X_SPEED)
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
    stepperX.setMinPulseWidth(100);

    // Y-axis configuration
    stepperY.setMaxSpeed(Y_SPEED);
    stepperY.setAcceleration(Y_ACCEL);
    stepperY.setPinsInverted(false);
    stepperY.setMinPulseWidth(100);

    // Rotation configuration - keep current position as is
    stepperRotation.setMaxSpeed(ROTATION_SPEED);
    stepperRotation.setAcceleration(ROTATION_ACCEL);
    stepperRotation.setPinsInverted(false);
    stepperRotation.setMinPulseWidth(100);
}

void MovementController::setPatternSpeed(const String& pattern,
                                         float speedPercentage)
{
    // Log input parameters
    Serial.println(F("--- setPatternSpeed Debug Start ---"));
    Serial.print(F("Pattern: "));
    Serial.println(pattern);
    Serial.print(F("Speed Percentage: "));
    Serial.println(speedPercentage);

    // Validate input parameters
    if (speedPercentage < 0 || speedPercentage > 100)
    {
        Serial.println(F("ERROR: Speed percentage out of bounds (0-100)"));
        return;
    }

    // Convert percentage to actual speed (0-100% maps to 0-X_SPEED)
    float targetSpeed = (speedPercentage / 100.0) * X_SPEED;
    Serial.print(F("Calculated target speed: "));
    Serial.println(targetSpeed);

    // Log state manager status
    Serial.print(F("State Manager exists: "));
    Serial.println(stateManager ? "YES" : "NO");

    SystemState currentState = IDLE;  // Default state
    if (stateManager)
    {
        currentState = stateManager->getCurrentState();
        Serial.print(F("Current system state: "));
        Serial.println(currentState);
    }
    else
    {
        Serial.println(F("WARNING: State manager is null"));
    }

    bool isPatternActive =
        (currentState == EXECUTING_PATTERN || currentState == PAINTING_SIDE);
    Serial.print(F("Pattern active: "));
    Serial.println(isPatternActive ? "YES" : "NO");

    // Store speed for appropriate pattern
    Serial.print(F("Storing speed for pattern: "));
    Serial.println(pattern);

    if (pattern == "FRONT")
    {
        Serial.print(F("Previous front speed: "));
        Serial.println(frontSpeed);
        frontSpeed = targetSpeed;
        Serial.print(F("New front speed: "));
        Serial.println(frontSpeed);

        if (stateManager && isPatternActive)
        {
            Serial.println(F("Applying speed to X stepper (FRONT)"));
            stepperX.setMaxSpeed(targetSpeed);
            Serial.print(F("Verified X stepper speed: "));
            Serial.println(stepperX.maxSpeed());
        }
    }
    else if (pattern == "BACK")
    {
        Serial.print(F("Previous back speed: "));
        Serial.println(backSpeed);
        backSpeed = targetSpeed;
        Serial.print(F("New back speed: "));
        Serial.println(backSpeed);

        if (stateManager && isPatternActive)
        {
            Serial.println(F("Applying speed to X stepper (BACK)"));
            stepperX.setMaxSpeed(targetSpeed);
            Serial.print(F("Verified X stepper speed: "));
            Serial.println(stepperX.maxSpeed());
        }
    }
    else if (pattern == "LEFT")
    {
        Serial.print(F("Previous left speed: "));
        Serial.println(leftSpeed);
        leftSpeed = targetSpeed;
        Serial.print(F("New left speed: "));
        Serial.println(leftSpeed);

        if (stateManager && isPatternActive)
        {
            Serial.println(F("Applying speed to X stepper (LEFT)"));
            stepperX.setMaxSpeed(targetSpeed);
            Serial.print(F("Verified X stepper speed: "));
            Serial.println(stepperX.maxSpeed());
        }
    }
    else if (pattern == "RIGHT")
    {
        Serial.print(F("Previous right speed: "));
        Serial.println(rightSpeed);
        rightSpeed = targetSpeed;
        Serial.print(F("New right speed: "));
        Serial.println(rightSpeed);

        if (stateManager && isPatternActive)
        {
            Serial.println(F("Applying speed to X stepper (RIGHT)"));
            stepperX.setMaxSpeed(targetSpeed);
            Serial.print(F("Verified X stepper speed: "));
            Serial.println(stepperX.maxSpeed());
        }
    }
    else
    {
        Serial.print(F("ERROR: Invalid pattern received: "));
        Serial.println(pattern);
    }

    Serial.println(F("--- setPatternSpeed Debug End ---"));
}

void MovementController::applyPatternSpeed(const String& pattern)
{
    if (pattern == "FRONT")
    {
        stepperX.setMaxSpeed(frontSpeed);
    }
    else if (pattern == "BACK")
    {
        stepperX.setMaxSpeed(backSpeed);
    }
    else if (pattern == "LEFT")
    {
        stepperX.setMaxSpeed(leftSpeed);
    }
    else if (pattern == "RIGHT")
    {
        stepperX.setMaxSpeed(rightSpeed);
    }
}

void MovementController::resetToDefaultSpeed()
{
    stepperX.setMaxSpeed(X_SPEED);
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
    // // Add rotation command debug logging
    // if (cmd.type == 'R')
    // {
    //     Serial.println(F("=== Rotation Command Debug ==="));
    //     Serial.print(F("Current steps: "));
    //     Serial.println(getCurrentRotationSteps());
    //     Serial.print(F("Target value: "));
    //     Serial.println(cmd.value);
    //     Serial.print(F("Absolute mode: "));
    //     Serial.println(cmd.sprayOn ? "true" : "false");
    // }

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
            targetSteps = (cmd.value * 400) / 360;
            if (cmd.sprayOn)  // If absolute positioning
            {
                stepperRotation.moveTo(targetSteps);
                Serial.print(F("Moving to absolute position: "));
                Serial.println(targetSteps);
            }
            else
            {
                stepperRotation.move(targetSteps);
                Serial.print(F("Moving relative steps: "));
                Serial.println(targetSteps);
            }
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
        digitalWrite(PAINT_RELAY_PIN, cmd.sprayOn ? HIGH : LOW);
    }
    else if (cmd.sprayOn && (cmd.type == 'X' || cmd.type == 'Y' ||
                             cmd.type == 'M' || cmd.type == 'N'))
    {
        digitalWrite(PAINT_RELAY_PIN, HIGH);
    }
}

bool MovementController::isMoving() const { return motorsRunning; }

void MovementController::stop()
{
    stepperX.stop();
    stepperY.stop();
    stepperRotation.stop();
    digitalWrite(PAINT_RELAY_PIN, LOW);
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
    digitalWrite(PAINT_RELAY_PIN, LOW);
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

    // If motors just stopped running
    if (previouslyRunning && !motorsRunning)
    {
        // Turn off spray
        digitalWrite(PAINT_RELAY_PIN, LOW);

        // If we were in manual rotation mode, transition back to appropriate
        // state
        if (stateManager && stateManager->getCurrentState() == MANUAL_ROTATING)
        {
            // Return to previous state (IDLE or HOMED)
            if (stateManager->getPreviousState() == HOMED)
            {
                stateManager->setState(HOMED);
            }
            else
            {
                stateManager->setState(IDLE);
            }
        }
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

float MovementController::getCurrentXSpeed() { return stepperX.maxSpeed(); }