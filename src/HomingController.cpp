// HomingController.cpp
#include "HomingController.h"

#include "config.h"

HomingController::HomingController(MovementController& movement)
    : movementController(movement),
      stateManager(nullptr),
      xHomeSensor(),
      yHomeSensor(),
      homing(false),
      homeComplete(false),
      currentAxis(0),
      initialRotationPosition(0),
      initialPositionSet(false)
{
}

void HomingController::setStateManager(StateManager* manager)
{
    stateManager = manager;
}

void HomingController::setup()
{
    // Configure home sensors
    pinMode(X_HOME_SENSOR_PIN, INPUT_PULLUP);
    pinMode(Y_HOME_SENSOR_PIN, INPUT_PULLUP);

    xHomeSensor.attach(X_HOME_SENSOR_PIN);
    yHomeSensor.attach(Y_HOME_SENSOR_PIN);
    xHomeSensor.interval(20);  // 20ms debounce time
    yHomeSensor.interval(20);

    // Capture initial rotation position
    initialRotationPosition = movementController.getCurrentRotationSteps();
    initialPositionSet = true;
    Serial.print(F("Initial rotation position set to: "));
    Serial.println(initialRotationPosition);
}

void HomingController::update()
{
    // Update sensor readings
    xHomeSensor.update();
    yHomeSensor.update();

    if (!homing)
    {
        return;
    }

    // Process homing sequence based on current axis
    switch (currentAxis)
    {
        case 0:
            processXHoming();
            break;
        case 1:
            processYHoming();
            break;
        case 2:
            processRotationHoming();
            break;
    }
}

void HomingController::processXHoming()
{
    if (!xHomeSensor.read())  // Sensor triggered
    {
        movementController.stopMovement();
        movementController.setXPosition(0);
        movementController.setXSpeed(X_SPEED);  // Restore normal speed

        Serial.println(F("X-axis home position found"));
        currentAxis = 1;  // Move to Y-axis homing

        if (stateManager)
        {
            stateManager->setState(HOMING_Y);
        }
    }
    else if (!movementController.isMoving())
    {
        movementController.setXSpeed(HOMING_SPEED);
        Command homeCmd('M', -999999, false);
        movementController.executeCommand(homeCmd);
    }
}

void HomingController::processYHoming()
{
    if (!yHomeSensor.read())  // Sensor triggered
    {
        movementController.stopMovement();
        movementController.setYPosition(0);
        movementController.setYSpeed(Y_SPEED);

        Serial.println(F("Y-axis home position found"));
        currentAxis = 2;  // Move to rotation homing

        if (stateManager)
        {
            stateManager->setState(HOMING_ROTATION);
        }
    }
    else if (!movementController.isMoving())
    {
        movementController.setYSpeed(HOMING_SPEED);
        Command homeCmd('N', -999999, false);
        movementController.executeCommand(homeCmd);
    }
}

void HomingController::processRotationHoming()
{
    // If we haven't sent the rotation command yet
    if (homing)
    {
        Serial.println(F("=== Rotation Homing State Check ==="));
        Serial.print(F("Movement in progress: "));
        Serial.println(movementController.isMoving() ? "YES" : "NO");

        if (!movementController.isMoving())
        {
            if (!initialPositionSet)
            {
                initialRotationPosition =
                    movementController.getCurrentRotationSteps();
                initialPositionSet = true;
                Serial.print(F("Late initial rotation position set to: "));
                Serial.println(initialRotationPosition);
            }

            // Add debug logging
            Serial.println(F("=== Rotation Homing Command Preparation ==="));
            Serial.print(F("Current rotation steps: "));
            Serial.println(movementController.getCurrentRotationSteps());
            Serial.print(F("Initial rotation position: "));
            Serial.println(initialRotationPosition);
            Serial.println(F("Sending absolute rotation command..."));

            // Create command to return to initial position using absolute steps
            Command rotateCmd('R', initialRotationPosition, true);
            bool cmdSuccess = movementController.executeCommand(rotateCmd);

            Serial.print(F("Command execution success: "));
            Serial.println(cmdSuccess ? "YES" : "NO");

            Serial.println(F("Setting homing=false"));
            homing = false;

            // Set homeComplete immediately if we're already at the target
            // position
            if (!movementController.isMoving())
            {
                homeComplete = true;
                if (stateManager)
                {
                    Serial.println(
                        F("Already at home position, transitioning to HOMED"));
                    stateManager->setState(HOMED);
                }
            }
        }
    }
    // Check if rotation movement is complete
    else if (!homeComplete)  // Only process if we ha ven't completed homing yet
    {
        Serial.println(F("=== Post-Rotation Movement Check ==="));
        Serial.print(F("Movement in progress: "));
        Serial.println(movementController.isMoving() ? "YES" : "NO");

        if (!movementController.isMoving())
        {
            Serial.println(F("=== Final Homing State Check ==="));
            Serial.print(F("Current position: "));
            Serial.println(movementController.getCurrentRotationSteps());
            Serial.print(F("Target position: "));
            Serial.println(initialRotationPosition);
            Serial.print(F("Position difference: "));
            Serial.println(abs(movementController.getCurrentRotationSteps() -
                               initialRotationPosition));

            homeComplete = true;

            if (stateManager)
            {
                Serial.println(F("Attempting to set HOMED state"));
                stateManager->setState(HOMED);
                Serial.print(F("New state: "));
                Serial.println(stateManager->getCurrentState());
            }
        }
    }
}

void HomingController::startHoming()
{
    if (!homing)
    {
        homing = true;
        homeComplete = false;
        currentAxis = 0;
        Serial.println(F("Starting homing sequence"));

        if (stateManager)
        {
            Serial.println(F("Setting state to HOMING_X"));
            stateManager->setState(HOMING_X);
        }
        else
        {
            Serial.println(F("No state manager set"));
        }
    }
}

bool HomingController::isHoming() const { return homing; }

bool HomingController::isHomed() const { return homeComplete; }

long HomingController::getHomeRotationPosition() const
{
    return initialRotationPosition;
}