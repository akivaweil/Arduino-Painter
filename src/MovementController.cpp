#include "MovementController.h"

#include <Arduino.h>

#include "config.h"

const float MAX_X_TRAVEL_INCHES = 34.0;
const float MAX_Y_TRAVEL_INCHES = 36.0;
const float MIN_TRAVEL_INCHES = 1.0;

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
      rightSpeed(X_SPEED),
      xHomed(false),
      yHomed(false),
      lastPositionLog(0)
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
    // // Log input parameters
    // Serial.println(F("--- setPatternSpeed Debug Start ---"));
    // Serial.print(F("Pattern: "));
    // Serial.println(pattern);
    // Serial.print(F("Speed Percentage: "));
    // Serial.println(speedPercentage);

    // Validate input parameters
    if (speedPercentage < 0 || speedPercentage > 100)
    {
        // Serial.println(F("ERROR: Speed percentage out of bounds (0-100)"));
        return;
    }

    // Convert percentage to actual speed (0-100% maps to 0-X_SPEED)
    float targetSpeed = (speedPercentage / 100.0) * X_SPEED;
    // Serial.print(F("Calculated target speed: "));
    // Serial.println(targetSpeed);

    // // Log state manager status
    // Serial.print(F("State Manager exists: "));
    // Serial.println(stateManager ? "YES" : "NO");

    SystemState currentState = IDLE;  // Default state
    if (stateManager)
    {
        currentState = stateManager->getCurrentState();
        // Serial.print(F("Current system state: "));
        // Serial.println(currentState);
    }
    else
    {
        // Serial.println(F("WARNING: State manager is null"));
    }

    bool isPatternActive =
        (currentState == EXECUTING_PATTERN || currentState == PAINTING_SIDE);
    // Serial.print(F("Pattern active: "));
    // Serial.println(isPatternActive ? "YES" : "NO");

    // // Store speed for appropriate pattern
    // Serial.print(F("Storing speed for pattern: "));
    // Serial.println(pattern);

    if (pattern == "FRONT")
    {
        // Serial.print(F("Previous front speed: "));
        // Serial.println(frontSpeed);
        frontSpeed = targetSpeed;
        // Serial.print(F("New front speed: "));
        // Serial.println(frontSpeed);

        if (stateManager && isPatternActive)
        {
            // Serial.println(F("Applying speed to X stepper (FRONT)"));
            stepperX.setMaxSpeed(targetSpeed);
            // Serial.print(F("Verified X stepper speed: "));
            // Serial.println(stepperX.maxSpeed());
        }
    }
    else if (pattern == "BACK")
    {
        // Serial.print(F("Previous back speed: "));
        // Serial.println(backSpeed);
        backSpeed = targetSpeed;
        // Serial.print(F("New back speed: "));
        // Serial.println(backSpeed);

        if (stateManager && isPatternActive)
        {
            // Serial.println(F("Applying speed to X stepper (BACK)"));
            stepperX.setMaxSpeed(targetSpeed);
            // Serial.print(F("Verified X stepper speed: "));
            // Serial.println(stepperX.maxSpeed());
        }
    }
    else if (pattern == "LEFT")
    {
        // Serial.print(F("Previous left speed: "));
        // Serial.println(leftSpeed);
        leftSpeed = targetSpeed;
        // Serial.print(F("New left speed: "));
        // Serial.println(leftSpeed);

        if (stateManager && isPatternActive)
        {
            // Serial.println(F("Applying speed to X stepper (LEFT)"));
            stepperX.setMaxSpeed(targetSpeed);
            // Serial.print(F("Verified X stepper speed: "));
            // Serial.println(stepperX.maxSpeed());
        }
    }
    else if (pattern == "RIGHT")
    {
        // Serial.print(F("Previous right speed: "));
        // Serial.println(rightSpeed);
        rightSpeed = targetSpeed;
        // Serial.print(F("New right speed: "));
        // Serial.println(rightSpeed);

        if (stateManager && isPatternActive)
        {
            // Serial.println(F("Applying speed to X stepper (RIGHT)"));
            stepperX.setMaxSpeed(targetSpeed);
            // Serial.print(F("Verified X stepper speed: "));
            // Serial.println(stepperX.maxSpeed());
        }
    }
    else
    {
        // Serial.print(F("ERROR: Invalid pattern received: "));
        // Serial.println(pattern);
    }

    // Serial.println(F("--- setPatternSpeed Debug End ---"));
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
    updateSprayControl(cmd);

    // If it's a spray-only command, return true since updateSprayControl
    // handled it
    if (cmd.type == 'P')
    {
        return true;
    }

    long targetSteps = 0;
    float targetPosition = 0.0;

    switch (cmd.type)
    {
        case 'X':  // Relative X movement
            targetSteps = getCurrentXSteps() + (cmd.value * X_STEPS_PER_INCH);
            enforceXLimit(targetSteps);
            stepperX.moveTo(targetSteps);
            break;

        case 'Y':  // Relative Y movement
            targetSteps = getCurrentYSteps() + (cmd.value * Y_STEPS_PER_INCH);
            enforceYLimit(targetSteps);
            stepperY.moveTo(targetSteps);
            break;

        case 'M':  // Absolute X movement
            targetSteps = cmd.value * X_STEPS_PER_INCH;
            enforceXLimit(targetSteps);
            stepperX.moveTo(targetSteps);
            break;

        case 'N':  // Absolute Y movement
            targetSteps = cmd.value * Y_STEPS_PER_INCH;
            enforceYLimit(targetSteps);
            stepperY.moveTo(targetSteps);
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

        case 'S':  // Servo angle command
            if (servoController != nullptr)
            {
                Serial.println(F("=== Servo Command Debug ==="));
                Serial.print(F("Setting servo angle to: "));
                Serial.println(static_cast<int>(cmd.value));
                return servoController->setAngle(static_cast<int>(cmd.value));
            }
            Serial.println(F("ERROR: ServoController not initialized"));
            return false;

        default:
            Serial.println(F("ERROR: Invalid movement command type"));
            return false;
    }

    return true;
}

void MovementController::updateSprayControl(const Command& cmd)
{
    if (cmd.type == 'P')  // Change to 'P' for Paint/spray control
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
    // Ensure spray is turned off
    digitalWrite(PAINT_RELAY_PIN, HIGH);
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

    // Ensure spray is turned off
    digitalWrite(PAINT_RELAY_PIN, HIGH);

    motorsRunning = false;
    continuousMovementActive = false;
    continuousDiagonalActive = false;
}

bool MovementController::isManualMovement() const
{
    return continuousMovementActive || continuousDiagonalActive;
}

void MovementController::logPosition()
{
    float xInches = stepsToInches(getCurrentXSteps(), X_STEPS_PER_INCH);
    float yInches = stepsToInches(getCurrentYSteps(), Y_STEPS_PER_INCH);

    Serial.print(F("Position - X: "));
    Serial.print(xInches);
    Serial.print(F(" inches, Y: "));
    Serial.print(yInches);
    Serial.println(F(" inches"));
}

void MovementController::update()
{
    // Update all stepper positions
    stepperX.run();
    stepperY.run();
    stepperRotation.run();

    // Update position cache
    updatePositionCache();

    // Check for limit clears during any movement
    float currentXInches = stepsToInches(getCurrentXSteps(), X_STEPS_PER_INCH);
    float currentYInches = stepsToInches(getCurrentYSteps(), Y_STEPS_PER_INCH);

    static bool xWasAtLimit = false;
    static bool yWasAtLimit = false;

    // Check X axis limit clear
    bool xAtLimit = (currentXInches <= MIN_TRAVEL_INCHES ||
                     currentXInches >= MAX_X_TRAVEL_INCHES);
    if (xWasAtLimit && !xAtLimit)
    {
        Serial.println(F("LIMIT_CLEAR:X"));
    }
    xWasAtLimit = xAtLimit;

    // Check Y axis limit clear
    bool yAtLimit = (currentYInches <= MIN_TRAVEL_INCHES ||
                     currentYInches >= MAX_Y_TRAVEL_INCHES);
    if (yWasAtLimit && !yAtLimit)
    {
        Serial.println(F("LIMIT_CLEAR:Y"));
    }
    yWasAtLimit = yAtLimit;

    // Update motors running status
    bool previouslyRunning = motorsRunning;
    motorsRunning = stepperX.isRunning() || stepperY.isRunning() ||
                    stepperRotation.isRunning();

    // Log position during manual movement
    if (motorsRunning && isManualMovement())
    {
        unsigned long currentTime = millis();
        if (currentTime - lastPositionLog >= POSITION_LOG_INTERVAL)
        {
            logPosition();
            lastPositionLog = currentTime;
        }
    }

    // If motors just stopped running
    if (previouslyRunning && !motorsRunning)
    {
        // Log final position if it was a manual movement
        if (isManualMovement())
        {
            Serial.println(F("Movement complete. Final position:"));
            logPosition();
        }

        // Turn off spray only if not in maintenance mode
        if (stateManager && stateManager->getCurrentState() != PRIMING &&
            stateManager->getCurrentState() != CLEANING &&
            stateManager->getCurrentState() != BACK_WASHING)
        {
            digitalWrite(PAINT_RELAY_PIN, HIGH);
        }

        // Handle state transitions for manual rotation
        if (stateManager && stateManager->getCurrentState() == MANUAL_ROTATING)
        {
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

    // Handle continuous movement
    if (continuousMovementActive)
    {
        AccelStepper& stepper = continuousMovementIsX ? stepperX : stepperY;
        if (!stepper.isRunning())
        {
            // Check if we've hit the limit
            float currentInches =
                continuousMovementIsX
                    ? stepsToInches(getCurrentXSteps(), X_STEPS_PER_INCH)
                    : stepsToInches(getCurrentYSteps(), Y_STEPS_PER_INCH);

            float maxTravel = continuousMovementIsX ? MAX_X_TRAVEL_INCHES
                                                    : MAX_Y_TRAVEL_INCHES;

            // Add minimum travel check
            if (continuousMovementPositive && currentInches >= maxTravel)
            {
                // Log limit reached before position
                Serial.println(continuousMovementIsX ? F("LIMIT:X_MAX")
                                                     : F("LIMIT:Y_MAX"));
                Serial.println(
                    F("Continuous movement reached limit. Final position:"));
                logPosition();
                continuousMovementActive = false;
                return;
            }
            else if (!continuousMovementPositive &&
                     currentInches <= MIN_TRAVEL_INCHES)
            {
                // Log limit reached before position
                Serial.println(continuousMovementIsX ? F("LIMIT:X_MIN")
                                                     : F("LIMIT:Y_MIN"));
                Serial.println(
                    F("Continuous movement reached limit. Final position:"));
                logPosition();
                continuousMovementActive = false;
                return;
            }

            // Calculate target that respects both min and max limits
            float targetInches =
                continuousMovementPositive ? maxTravel : MIN_TRAVEL_INCHES;
            long stepsPerInch =
                continuousMovementIsX ? X_STEPS_PER_INCH : Y_STEPS_PER_INCH;
            long targetSteps = targetInches * stepsPerInch;

            stepper.moveTo(targetSteps);
        }
    }

    // Handle continuous diagonal movement
    if (continuousDiagonalActive)
    {
        if (!stepperX.isRunning() || !stepperY.isRunning())
        {
            // Check if we've hit any limits
            float currentXInches =
                stepsToInches(getCurrentXSteps(), X_STEPS_PER_INCH);
            float currentYInches =
                stepsToInches(getCurrentYSteps(), Y_STEPS_PER_INCH);

            // Clear limit flags if we've moved away from limits
            if (currentXInches > MIN_TRAVEL_INCHES &&
                currentXInches < MAX_X_TRAVEL_INCHES)
            {
                Serial.println(F("LIMIT_CLEAR:X"));
            }
            if (currentYInches > MIN_TRAVEL_INCHES &&
                currentYInches < MAX_Y_TRAVEL_INCHES)
            {
                Serial.println(F("LIMIT_CLEAR:Y"));
            }

            bool xAtLimit = (continuousDiagonalXPositive &&
                             currentXInches >= MAX_X_TRAVEL_INCHES) ||
                            (!continuousDiagonalXPositive &&
                             currentXInches <= MIN_TRAVEL_INCHES);
            bool yAtLimit = (continuousDiagonalYPositive &&
                             currentYInches >= MAX_Y_TRAVEL_INCHES) ||
                            (!continuousDiagonalYPositive &&
                             currentYInches <= MIN_TRAVEL_INCHES);

            if (xAtLimit || yAtLimit)
            {
                // Log specific limits that were reached
                if (xAtLimit)
                {
                    Serial.println(continuousDiagonalXPositive
                                       ? F("LIMIT:X_MAX")
                                       : F("LIMIT:X_MIN"));
                }
                if (yAtLimit)
                {
                    Serial.println(continuousDiagonalYPositive
                                       ? F("LIMIT:Y_MAX")
                                       : F("LIMIT:Y_MIN"));
                }

                Serial.println(
                    F("Diagonal movement reached limit. Final position:"));
                logPosition();
                continuousDiagonalActive = false;
                return;
            }

            // Set targets to appropriate limits
            float targetX = continuousDiagonalXPositive ? MAX_X_TRAVEL_INCHES
                                                        : MIN_TRAVEL_INCHES;
            float targetY = continuousDiagonalYPositive ? MAX_Y_TRAVEL_INCHES
                                                        : MIN_TRAVEL_INCHES;

            stepperX.moveTo(targetX * X_STEPS_PER_INCH);
            stepperY.moveTo(targetY * Y_STEPS_PER_INCH);
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

bool MovementController::startContinuousMovement(bool isXAxis, bool isPositive,
                                                 float speed,
                                                 float acceleration)
{
    // Stop any existing movement first
    stopMovement();

    // Get current position in inches
    float currentInches =
        isXAxis ? stepsToInches(getCurrentXSteps(), X_STEPS_PER_INCH)
                : stepsToInches(getCurrentYSteps(), Y_STEPS_PER_INCH);

    Serial.println(F("=== Starting Continuous Movement ==="));
    Serial.print(F("Current position (inches): "));
    Serial.println(currentInches);

    // Check if we're already at the limit in the requested direction
    float maxTravel = isXAxis ? MAX_X_TRAVEL_INCHES : MAX_Y_TRAVEL_INCHES;

    if ((isPositive && currentInches >= maxTravel) ||
        (!isPositive &&
         currentInches <= MIN_TRAVEL_INCHES))  // Changed from <= -maxTravel
    {
        Serial.println(F("Already at travel limit, movement blocked"));
        return false;
    }

    // Set movement parameters
    AccelStepper& stepper = isXAxis ? stepperX : stepperY;
    stepper.setMaxSpeed(speed);
    stepper.setAcceleration(acceleration);

    // Calculate target that won't exceed limits
    long stepsPerInch = isXAxis ? X_STEPS_PER_INCH : Y_STEPS_PER_INCH;
    long currentSteps = isXAxis ? getCurrentXSteps() : getCurrentYSteps();

    // Set target to the appropriate limit in the requested direction
    float targetInches =
        isPositive ? maxTravel : MIN_TRAVEL_INCHES;  // Changed from -maxTravel
    long targetSteps = targetInches * stepsPerInch;

    Serial.print(F("Setting target to (inches): "));
    Serial.println(targetInches);

    stepper.moveTo(targetSteps);

    // Update tracking variables
    continuousMovementActive = true;
    continuousMovementIsX = isXAxis;
    continuousMovementPositive = isPositive;

    return true;
}

bool MovementController::startContinuousDiagonalMovement(bool xPositive,
                                                         bool yPositive,
                                                         float speed,
                                                         float acceleration)
{
    // Stop any existing movement first
    stopMovement();

    // Get current positions
    float currentXInches = stepsToInches(getCurrentXSteps(), X_STEPS_PER_INCH);
    float currentYInches = stepsToInches(getCurrentYSteps(), Y_STEPS_PER_INCH);

    Serial.println(F("=== Starting Continuous Diagonal Movement ==="));
    Serial.print(F("Current X (inches): "));
    Serial.println(currentXInches);
    Serial.print(F("Current Y (inches): "));
    Serial.println(currentYInches);

    // Check if either axis is already at its limit
    if ((xPositive && currentXInches >= MAX_X_TRAVEL_INCHES) ||
        (!xPositive && currentXInches <= MIN_TRAVEL_INCHES) ||
        (yPositive && currentYInches >= MAX_Y_TRAVEL_INCHES) ||
        (!yPositive && currentYInches <= MIN_TRAVEL_INCHES))
    {
        Serial.println(F("Already at travel limit, movement blocked"));
        return false;
    }

    // Calculate the remaining travel distance for each axis
    float remainingX = xPositive ? (MAX_X_TRAVEL_INCHES - currentXInches)
                                 : (currentXInches - MIN_TRAVEL_INCHES);
    float remainingY = yPositive ? (MAX_Y_TRAVEL_INCHES - currentYInches)
                                 : (currentYInches - MIN_TRAVEL_INCHES);

    // Calculate speed ratios to maintain true diagonal movement
    float totalStepsX = remainingX * X_STEPS_PER_INCH;
    float totalStepsY = remainingY * Y_STEPS_PER_INCH;

    // Adjust speeds proportionally based on the number of steps each motor
    // needs to move
    float speedRatio = totalStepsX / totalStepsY;
    float xSpeed = speedRatio >= 1.0 ? speed : speed * speedRatio;
    float ySpeed = speedRatio >= 1.0 ? speed / speedRatio : speed;

    // Set movement parameters for both axes
    stepperX.setMaxSpeed(xSpeed);
    stepperY.setMaxSpeed(ySpeed);
    stepperX.setAcceleration(acceleration);
    stepperY.setAcceleration(acceleration);

    // Set targets to appropriate limits
    float targetX = xPositive ? MAX_X_TRAVEL_INCHES : MIN_TRAVEL_INCHES;
    float targetY = yPositive ? MAX_Y_TRAVEL_INCHES : MIN_TRAVEL_INCHES;

    Serial.print(F("Target X (inches): "));
    Serial.println(targetX);
    Serial.print(F("Target Y (inches): "));
    Serial.println(targetY);
    Serial.print(F("X Speed: "));
    Serial.println(xSpeed);
    Serial.print(F("Y Speed: "));
    Serial.println(ySpeed);

    stepperX.moveTo(targetX * X_STEPS_PER_INCH);
    stepperY.moveTo(targetY * Y_STEPS_PER_INCH);

    // Update tracking variables
    continuousDiagonalActive = true;
    continuousDiagonalXPositive = xPositive;
    continuousDiagonalYPositive = yPositive;
    continuousMovementActive = false;  // Disable single-axis movement

    return true;
}

void MovementController::toggleSpray(bool on)
{
    digitalWrite(PAINT_RELAY_PIN, on ? LOW : HIGH);
}

// Add these helper methods to enforce limits while allowing movement up to them
void MovementController::enforceXLimit(long& targetSteps)
{
    // During homing or if not yet homed, don't enforce lower limit
    if (!xHomed ||
        (stateManager && (stateManager->getCurrentState() == HOMING_X ||
                          stateManager->getCurrentState() == HOMING_Y ||
                          stateManager->getCurrentState() == HOMING_ROTATION)))
    {
        // Only enforce upper limit
        float currentInches =
            stepsToInches(getCurrentXSteps(), X_STEPS_PER_INCH);
        float targetInches = stepsToInches(targetSteps, X_STEPS_PER_INCH);
        float relativeMovement = targetInches - currentInches;

        if (relativeMovement > MAX_X_TRAVEL_INCHES)
        {
            float clampedRelative = MAX_X_TRAVEL_INCHES;
            targetSteps =
                getCurrentXSteps() + (clampedRelative * X_STEPS_PER_INCH);
        }
        return;
    }

    // Normal operation with both limits
    float currentInches = stepsToInches(getCurrentXSteps(), X_STEPS_PER_INCH);
    float targetInches = stepsToInches(targetSteps, X_STEPS_PER_INCH);
    float relativeMovement = targetInches - currentInches;

    if (relativeMovement <
        (MIN_TRAVEL_INCHES - currentInches))  // Would move below minimum
    {
        targetSteps = MIN_TRAVEL_INCHES * X_STEPS_PER_INCH;  // Clamp at minimum
    }
    else if (relativeMovement >
             (MAX_X_TRAVEL_INCHES - currentInches))  // Would exceed max
    {
        targetSteps = MAX_X_TRAVEL_INCHES * X_STEPS_PER_INCH;
    }
}

void MovementController::enforceYLimit(long& targetSteps)
{
    // During homing or if not yet homed, don't enforce lower limit
    if (!yHomed ||
        (stateManager && (stateManager->getCurrentState() == HOMING_X ||
                          stateManager->getCurrentState() == HOMING_Y ||
                          stateManager->getCurrentState() == HOMING_ROTATION)))
    {
        // Only enforce upper limit
        float currentInches =
            stepsToInches(getCurrentYSteps(), Y_STEPS_PER_INCH);
        float targetInches = stepsToInches(targetSteps, Y_STEPS_PER_INCH);
        float relativeMovement = targetInches - currentInches;

        if (relativeMovement > MAX_Y_TRAVEL_INCHES)
        {
            float clampedRelative = MAX_Y_TRAVEL_INCHES;
            targetSteps =
                getCurrentYSteps() + (clampedRelative * Y_STEPS_PER_INCH);
        }
        return;
    }

    // Normal operation with both limits
    float currentInches = stepsToInches(getCurrentYSteps(), Y_STEPS_PER_INCH);
    float targetInches = stepsToInches(targetSteps, Y_STEPS_PER_INCH);
    float relativeMovement = targetInches - currentInches;

    if (relativeMovement <
        (MIN_TRAVEL_INCHES - currentInches))  // Would move below minimum
    {
        targetSteps = MIN_TRAVEL_INCHES * Y_STEPS_PER_INCH;  // Clamp at minimum
    }
    else if (relativeMovement >
             (MAX_Y_TRAVEL_INCHES - currentInches))  // Would exceed max
    {
        targetSteps = MAX_Y_TRAVEL_INCHES * Y_STEPS_PER_INCH;
    }
}

bool MovementController::isPositionValid(long xSteps, long ySteps) const
{
    // Add your machine's specific limits here
    // Example:
    const long MAX_X_STEPS = 40 * X_STEPS_PER_INCH;  // 40 inches
    const long MAX_Y_STEPS = 40 * Y_STEPS_PER_INCH;  // 40 inches

    return (xSteps >= 0 && xSteps <= MAX_X_STEPS && ySteps >= 0 &&
            ySteps <= MAX_Y_STEPS);
}