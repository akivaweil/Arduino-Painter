#include "HomingController.h"

#include "config.h"

HomingController::HomingController(MovementController& movement)
    : movementController(movement),
      xHomeSensor(),
      yHomeSensor(),
      homing(false),
      homeComplete(false),
      currentAxis(0)
{
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
    }
}

void HomingController::processXHoming()
{
    if (!xHomeSensor.read())  // Sensor triggered
    {
        movementController.stopMovement();   // Stop immediately
        movementController.setXPosition(0);  // Set current position as 0

        Serial.println(F("X-axis home position found"));
        currentAxis = 1;  // Move to Y-axis homing
    }
    else if (!movementController.isMoving())
    {
        // Create a command for continuous movement towards home
        Command homeCmd('M', -999999,
                        false);  // Use absolute move with large negative value
        movementController.executeCommand(homeCmd);
    }
}

void HomingController::processYHoming()
{
    if (!yHomeSensor.read())  // Sensor triggered
    {
        movementController.stopMovement();   // Stop immediately
        movementController.setYPosition(0);  // Set current position as 0

        Serial.println(F("Y-axis home position found"));
        homing = false;
        homeComplete = true;
    }
    else if (!movementController.isMoving())
    {
        // Create a command for continuous movement towards home
        Command homeCmd('N', -999999,
                        false);  // Use absolute move with large negative value
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
    }
}

bool HomingController::isHoming() const { return homing; }

bool HomingController::isHomed() const { return homeComplete; }