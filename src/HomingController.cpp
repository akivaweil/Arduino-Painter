// HomingController.cpp
#include "HomingController.h"

#include "config.h"

HomingController::HomingController(MovementController& movement)
    : movementController(movement),
      stateManager(nullptr),
      xHomeSensor(),
      yHomeSensor(),
      rotationHomeSensor(),
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
    rotationHomeSensor.attach(ROTATION_HOME_SENSOR_PIN);
    xHomeSensor.interval(20);  // 20ms debounce time
    yHomeSensor.interval(20);
    rotationHomeSensor.interval(20);
}

void HomingController::update()
{
    // Update sensor readings
    xHomeSensor.update();
    yHomeSensor.update();
    rotationHomeSensor.update();

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
    if (!rotationHomeSensor.read())  // Sensor triggered
    {
        movementController.stopMovement();
        movementController.setRotationPosition(0);
        movementController.setRotationSpeed(
            ROTATION_SPEED);  // Restore normal speed

        Serial.println(F("Rotation home position found"));
        homing = false;
        homeComplete = true;

        // Force an immediate update to ensure the movement completes
        movementController.update();

        if (stateManager)
        {
            Serial.println(F("Homing sequence complete"));
            stateManager->setState(HOMED);
        }
    }
    else if (!movementController.isMoving())
    {
        // Move CCW slowly until home sensor is found
        movementController.setRotationSpeed(ROTATION_HOMING_SPEED);
        Command homeCmd('R', -360, false);  // Full rotation CCW
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