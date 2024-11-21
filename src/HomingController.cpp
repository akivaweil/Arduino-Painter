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
      currentAxis(0)
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

    // Process homing sequence
    if (currentAxis == 0)
    {
        processXHoming();
    }
    else
    {
        processYHoming();

        // If we just completed homing, ensure state transition
        if (homeComplete && stateManager)
        {
            stateManager->setState(HOMED);
        }
    }
}

void HomingController::processXHoming()
{
    if (!xHomeSensor.read())  // Sensor triggered
    {
        movementController.stopMovement();   // Stop immediately
        movementController.setXPosition(0);  // Set current position as 0
        movementController.setXSpeed(
            X_SPEED);  // Restore normal speed after homing complete

        Serial.println(F("X-axis home position found"));
        currentAxis = 1;  // Move to Y-axis homing

        if (stateManager)
        {
            stateManager->setState(HOMING_Y);
        }
    }
    else if (!movementController.isMoving())
    {
        // Create a command for continuous movement towards homwe at slower
        // speed
        movementController.setXSpeed(
            HOMING_SPEED);  // Use defined constant for homing
        Command homeCmd('M', -999999,
                        false);  // Use absolute move with large negative value
        movementController.executeCommand(homeCmd);
    }
}

void HomingController::processYHoming()
{
    if (!yHomeSensor.read())  // Sensor triggered
    {
        // First stop movement
        movementController.stopMovement();
        movementController.setYPosition(0);
        movementController.setYSpeed(
            Y_SPEED);  // Restore normal speed after homing complete

        // Update status before state transition
        Serial.println(F("Y-axis home position found"));
        homing = false;
        homeComplete = true;

        // Force an immediate update to ensure the movement completes
        movementController.update();

        // Now transition state and report completion
        if (stateManager)
        {
            Serial.println(F("Homing sequence complete"));
            stateManager->setState(HOMED);
        }
    }
    else if (!movementController.isMoving())
    {
        // Create a command for continuous movement towards home at slower speed
        movementController.setYSpeed(
            HOMING_SPEED);  // Use defined constant for homing
        Command homeCmd('N', -999999, false);
        movementController.executeCommand(homeCmd);
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
            stateManager->setState(HOMING_X);
        }
    }
}

bool HomingController::isHoming() const { return homing; }

bool HomingController::isHomed() const { return homeComplete; }